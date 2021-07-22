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
ALIGN_32BYTES(static int32_t audio_in[2]);
ALIGN_32BYTES(static int32_t dtcm_buffer_out[AUDIO_BUFFER_SIZE]);
ALIGN_32BYTES(static int32_t test_buffer1[4]);
ALIGN_32BYTES(static int32_t test_buffer2[4]);

typedef struct fir5 {
    float coeff[3];
    float samples[6];
} fir5;

typedef struct fir {
    float coeff[10];
    float samples[20];
    int32_t n_coeff;
} fir;

static fir5 lowpass_02;
static fir5 highpass_025;

static volatile uint32_t acc_time;
static volatile uint32_t n_acc_time;
static volatile uint32_t prev_time;

static volatile int32_t buf_out_idx = 0;
static volatile int32_t err_cnt;
static volatile int32_t race_cnt;
static MDMA_HandleTypeDef hmdma;
static MDMA_LinkNodeTypeDef node1 __attribute__ ((section(".AXI_SRAM")));

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
    }

}

static int32_t fir5_apply(fir5 *f, int32_t in)
{
    float out = 0.f;
    float *sample_ptr = &f->samples[0];
    for (int32_t i = 0; i < 3; ++i)
    {
        out += f->coeff[i] * (*sample_ptr);
        ++sample_ptr;
    }
    for (int32_t i = 2; i >= 3; --i)
    {
        out += f->coeff[i] * (*sample_ptr);
        ++sample_ptr;
    }
    for (int32_t i = 0; i < 5; ++i)
    {
        f->samples[i] = f->samples[i + 1];
    }
    f->samples[5] = (float)in;
    return out;
}

static int32_t fir5_apply_highpass(fir5 *f, int32_t in)
{
    float out = 0.f;
    float *sample_ptr = &f->samples[0];
    for (int32_t i = 0; i < 3; ++i)
    {
        out += f->coeff[i] * (*sample_ptr);
        ++sample_ptr;
    }
    for (int32_t i = 2; i >= 3; --i)
    {
        out -= f->coeff[i] * (*sample_ptr);
        ++sample_ptr;
    }
    for (int32_t i = 0; i < 5; ++i)
    {
        f->samples[i] = f->samples[i + 1];
    }
    f->samples[5] = (float)in;
    return out;
}

static int32_t low_pass(int32_t in)
{
    const float a = 0.2f;
    static float out;
    static int32_t prev_in;

    out = (1.f - a) * out + a * ((in >> 1) + prev_in);
    prev_in = in >> 1;
    return (int32_t)out;
}

static int32_t high_pass(int32_t in)
{
    const float a[3] = { 0.79482f, -1.58968f, 0.79489f };
    const float b[2] = { -1.52677f, 0.65259f };
    static float state[2];

    float out = a[0] * in + state[0];
    state[0] = a[1] * in - b[0] * out + state[1];
    state[1] = a[2] * in - b[1] * out;

    return (int32_t)out;
}

void mdma_callback(MDMA_HandleTypeDef *_hmdma)
{
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    int32_t out;
//    out = low_pass(mdma_left[0]);
//    out = fir5_apply(&lowpass_02, mdma_left[0]);
//    out = audio_in[0];
//    out = fir5_apply(&lowpass_02, audio_in[0]);
    out = low_pass(audio_in[0]);
    audio_buffer_out[buf_idx] = out;
    dtcm_buffer_out[buf_idx] = out;
//    out = high_pass(mdma_right[0]);
//    out = fir5_apply_highpass(&highpass_025, mdma_right[0]);
//    out = audio_in[1];
    out = fir5_apply_highpass(&highpass_025, audio_in[1]);
    audio_buffer_out[buf_idx + 1] = out;
    dtcm_buffer_out[buf_idx + 1] = out;
    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in));

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE;
        buf_out_idx %= AUDIO_BUFFER_SIZE;
    }
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
    buf_out_idx = AUDIO_BUFFER_SIZE / 2;
    acc_time = 0;
    n_acc_time = 0;
    prev_time = 0;
    lowpass_02.coeff[0] = 0.0955f;
    lowpass_02.coeff[1] = 0.1949f;
    lowpass_02.coeff[2] = 0.2624f;
    memset(&lowpass_02.samples[0], 0, sizeof(lowpass_02.samples));
    highpass_025.coeff[0] =  0.0217f;
    highpass_025.coeff[1] = -0.1054f;
    highpass_025.coeff[2] = -0.5978f;
    memset(&highpass_025.samples[0], 0, sizeof(highpass_025.samples));

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
                    &dtcm_buffer_out[0]);
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
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
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
    hmdma.Init.BufferTransferLength = 8;
    hmdma.Init.DataAlignment = MDMA_DATAALIGN_RIGHT;;
    hmdma.Init.DestBlockAddressOffset = 0;
    hmdma.Init.DestBurst = MDMA_DEST_BURST_2BEATS;
    hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
    hmdma.Init.DestinationInc = MDMA_DEST_INC_WORD;
    hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    hmdma.Init.Request = MDMA_REQUEST_DMA2_Stream4_TC;
    hmdma.Init.SourceBlockAddressOffset = 0;
    hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_2BEATS;
    hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    hmdma.Init.SourceInc = MDMA_SRC_INC_WORD;
    hmdma.Init.TransferTriggerMode = MDMA_REPEAT_BLOCK_TRANSFER;
    status = HAL_MDMA_Init(&hmdma);
    e.val = status << 0;
    eq_m7_add_event(e);
    status = HAL_MDMA_RegisterCallback(&hmdma, HAL_MDMA_XFER_REPBLOCKCPLT_CB_ID, &mdma_callback);
    e.val = status << 2;
    eq_m7_add_event(e);

    HAL_MDMA_ConfigPostRequestMask(&hmdma, (uint32_t)&(DMA2->HIFCR), DMA_HIFCR_CTCIF4);

    memcpy(&node_conf.Init, &hmdma.Init, sizeof(hmdma.Init));
    node_conf.PostRequestMaskAddress = (uint32_t)&(DMA2->HIFCR);
    node_conf.PostRequestMaskData = DMA_HIFCR_CTCIF4;
    node_conf.BlockCount = 1;
    node_conf.BlockDataLength = 8;

    node_conf.DstAddress = (uint32_t)&audio_in[0];
    node_conf.SrcAddress = (uint32_t)&audio_buffer_in[0];

    status = HAL_MDMA_LinkedList_CreateNode(&node1, &node_conf);
    e.val = status << 4;
    eq_m7_add_event(e);
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &node1, 0);
    e.val = status << 6;
    eq_m7_add_event(e);

    status = HAL_MDMA_LinkedList_EnableCircularMode(&hmdma);
    e.val = status << 12;
    eq_m7_add_event(e);

    SCB_CleanDCache_by_Addr((uint32_t *)&node1, sizeof(node1));

    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(&hmdma);
}

