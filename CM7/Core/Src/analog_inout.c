/* Includes ------------------------------------------------------------------*/
#include <error_handler.h>
#include "stm32h747i_discovery_audio.h"
#include "stm32h7xx_hal_mdma.h"
#include "string.h"
#include "shared_data.h"
#include "fft_hist.h"
#include "intercore_comm.h"
#include "event_queue.h"
#include "perf_meas.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)2)
#define N_AUDIO_BLOCKS              ((uint32_t)32)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static int32_t audio_buffer_in[AUDIO_BLOCK_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t audio_buffer_out[AUDIO_BUFFER_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t mdma_buffer1[4]);
ALIGN_32BYTES(static int32_t mdma_buffer2[4]);
ALIGN_32BYTES(static int32_t test_buffer1[4]);
ALIGN_32BYTES(static int32_t test_buffer2[4]);

static volatile uint32_t acc_time;
static volatile uint32_t n_acc_time;
static volatile uint32_t prev_time;

static volatile int32_t buf_out_idx = 0;
static volatile int32_t err_cnt;
static volatile int32_t race_cnt;
static volatile int32_t mdma_cnt;
static MDMA_HandleTypeDef hmdma;
static MDMA_LinkNodeTypeDef node1 __attribute__ ((section(".AXI_SRAM")));
static MDMA_LinkNodeTypeDef node2 __attribute__ ((section(".AXI_SRAM")));

static void gather_and_log_fft_time(uint32_t fft_time)
{
    static uint32_t acc_fft_time = 0;
    static int32_t cnt = 0;
    acc_fft_time += fft_time;
    ++cnt;
    if (1 << 15 == cnt)
    {
        event e =
        { .id = EVENT_M7_TRACE, .val = acc_fft_time >> 15 };
        eq_m7_add_event(e);
        acc_fft_time = 0;
        cnt = 0;

//        e.id = EVENT_DBG;
//        e.val = mdma_cnt;
//        eq_m7_add_event(e);
    }

}

void mdma_callback(MDMA_HandleTypeDef *_hmdma)
{

    uint32_t curr_time = GET_CCNT();
    acc_time += DIFF_CCNT(prev_time, curr_time);
    prev_time = curr_time;
    ++n_acc_time;

    if (n_acc_time >= 1 << 15)
    {
        event e =
        { .id = EVENT_DBG, .val = acc_time >> 15 };
        eq_m7_add_event(e);
        n_acc_time = 0;
        acc_time = 0;
    }
//    ++mdma_cnt;
}


/* Private function prototypes -----------------------------------------------*/
static void config_MDMA();

/*----------------------------------------------------------------------------*/
void analog_inout(void)
{
    int32_t buf_idx = 0;
    const uint32_t audio_freq = 48000;
    const uint32_t audio_resolution = 32;

    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_IN_OUT_Init(audio_freq, audio_resolution);
    unlock_hsem(HSEM_I2C4);

    memset(&test_buffer1[0], 0xAB, sizeof(test_buffer1));
    memset(&test_buffer2[0], 0, sizeof(test_buffer2));

    config_MDMA();

    err_cnt = 0;
    race_cnt = 0;
    buf_out_idx = 0;
    mdma_cnt = 0;
    acc_time = 0;
    n_acc_time = 0;
    prev_time = 0;

    while (lock_hsem(HSEM_I2C4))
        ;
    err_cnt += BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in));
    err_cnt += BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out));
    unlock_hsem(HSEM_I2C4);

    // Useless as a transfer because it is not in the linked list, but function call needed to trigger start of transfer,
    // Can be copy of any of linked list nodes to not create random memory buffers
    HAL_StatusTypeDef status = HAL_MDMA_Start_IT(&hmdma, (uint32_t)&audio_buffer_out[0], (uint32_t)&test_buffer2[0], sizeof(uint32_t), 3);
    event e;
    e.id = EVENT_MDMA_CFG;
    e.val = status << 14;
    eq_m7_add_event(e);


    while (start_audio == 1)
    {
        int32_t new_buf_idx = buf_out_idx;
        // if (buf_idx != new_buf_idx)
        if (0 == (new_buf_idx % AUDIO_BUFFER_SIZE))
        {
            buf_idx = 0;

            uint32_t start = GET_CCNT();
            fft_16hist((int16_t*) &shared_fft_l[0], (int16_t*) &shared_fft_r[0],
                    &audio_buffer_out[buf_idx]);
            uint32_t stop = GET_CCNT();

            buf_idx = new_buf_idx;

            ++new_data_flag;

            gather_and_log_fft_time(DIFF_CCNT(start, stop));
        }
    }
    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_OUT_Stop(0);
    BSP_AUDIO_OUT_DeInit(0);
    BSP_AUDIO_IN_Stop(0);
    BSP_AUDIO_IN_DeInit(0);
    unlock_hsem(HSEM_I2C4);

    HAL_MDMA_Abort_IT(&hmdma);
    HAL_MDMA_UnRegisterCallback(&hmdma, HAL_MDMA_XFER_CPLT_CB_ID);
    HAL_MDMA_DeInit(&hmdma);

    e.id = EVENT_DBG;
    e.val = mdma_cnt;
    eq_m7_add_event(e);
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[0], sizeof(audio_buffer_in) / 2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[0], sizeof(audio_buffer_in) / 2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in) / 2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx], sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE / 2;
    }
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in) / 2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in) / 2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
    // sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in) / 2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx],
    // sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE / 2;
        buf_out_idx %= AUDIO_BUFFER_SIZE;
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}

static void config_MDMA()
{
    HAL_StatusTypeDef status;
    MDMA_LinkNodeConfTypeDef node_conf;
    event e =
    { .id = EVENT_MDMA_CFG};

    __HAL_RCC_MDMA_CLK_ENABLE();

    hmdma.Instance = MDMA_Channel1;
    hmdma.Init.BufferTransferLength = 4;
    hmdma.Init.DataAlignment = MDMA_DATAALIGN_LEFT;;
    hmdma.Init.DestBlockAddressOffset = 0;
    hmdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
    hmdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    hmdma.Init.Request = MDMA_REQUEST_DMA2_Stream4_TC;
    hmdma.Init.SourceBlockAddressOffset = 0;
    hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    hmdma.Init.SourceInc = MDMA_SRC_INC_DISABLE;
    hmdma.Init.TransferTriggerMode = MDMA_REPEAT_BLOCK_TRANSFER;
    status = HAL_MDMA_Init(&hmdma);
    e.val = status << 0;
    eq_m7_add_event(e);
    status = HAL_MDMA_RegisterCallback(&hmdma, HAL_MDMA_XFER_REPBLOCKCPLT_CB_ID, &mdma_callback);
    e.val = status << 2;
    eq_m7_add_event(e);

    HAL_MDMA_ConfigPostRequestMask(&hmdma, (uint32_t)&(DMA2->HIFCR), DMA_HIFCR_CTCIF4);

#if 1

    memcpy(&node_conf.Init, &hmdma.Init, sizeof(hmdma.Init));
    node_conf.PostRequestMaskAddress = (uint32_t)&(DMA2->HIFCR);
    node_conf.PostRequestMaskData = DMA_HIFCR_CTCIF4;
    node_conf.BlockCount = 1;
    node_conf.BlockDataLength = 4;

    node_conf.DstAddress = (uint32_t)&mdma_buffer1[0];
    node_conf.SrcAddress = (uint32_t)&audio_buffer_in[0];

    status = HAL_MDMA_LinkedList_CreateNode(&node1, &node_conf);
    e.val = status << 4;
    eq_m7_add_event(e);
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &node1, 0);
    e.val = status << 6;
    eq_m7_add_event(e);

    node_conf.DstAddress = (uint32_t)&mdma_buffer2[0];
    node_conf.SrcAddress = (uint32_t)&audio_buffer_in[1];

    status = HAL_MDMA_LinkedList_CreateNode(&node2, &node_conf);
    e.val = status << 8;
    eq_m7_add_event(e);
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &node2, 0);
    e.val = status << 10;
    eq_m7_add_event(e);

#else
    memcpy(&node_conf.Init, &hmdma.Init, sizeof(hmdma.Init));
    node_conf.PostRequestMaskAddress = 0;
    node_conf.PostRequestMaskData = 0;
    node_conf.BlockDataLength = 4;
    node_conf.BlockCount = 1;
    node_conf.SrcAddress = (uint32_t)&test_buffer1[0];
    node_conf.DstAddress = (uint32_t)&mdma_buffer1[0];

    status = HAL_MDMA_LinkedList_CreateNode(&node1, &node_conf);
    e.val = status << 2;
    eq_m7_add_event(e);
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &node1, 0);
    e.val = status << 4;
    eq_m7_add_event(e);

    node_conf.BlockCount = 4;
    node_conf.DstAddress = (uint32_t)&mdma_buffer2[0];

    status = HAL_MDMA_LinkedList_CreateNode(&node2, &node_conf);
    e.val = status << 6;
    eq_m7_add_event(e);
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &node2, 0);
    e.val = status << 8;
    eq_m7_add_event(e);

#endif
    status = HAL_MDMA_LinkedList_EnableCircularMode(&hmdma);
    e.val = status << 12;
    eq_m7_add_event(e);

    SCB_CleanDCache_by_Addr((uint32_t *)&node1, sizeof(node1));
    SCB_CleanDCache_by_Addr((uint32_t *)&node2, sizeof(node2));


    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

void MDMA_IRQHandler(void)
{
//    hmdma.Instance->CIFCR = 0x0000001F;
//    hmdma.Instance->CLAR = (uint32_t)hmdma.FirstLinkedListNodeAddress;
//    hmdma.Instance->CBNDTR = 2;
//    __HAL_MDMA_ENABLE(&hmdma);
    HAL_MDMA_IRQHandler(&hmdma);
//    HAL_MDMA_Start_IT(&hmdma, (uint32_t)&audio_buffer_out[0], (uint32_t)&test_buffer2[0], sizeof(uint32_t), 3);
}

