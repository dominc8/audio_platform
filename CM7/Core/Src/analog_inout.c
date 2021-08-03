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
#include "fir.h"
#include "biquad.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)2)
#define N_AUDIO_BLOCKS              ((uint32_t)32)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static int32_t audio_buffer_in[AUDIO_BLOCK_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t audio_buffer_out[AUDIO_BUFFER_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t audio_in[2]);
ALIGN_32BYTES(static int32_t dtcm_buffer_out[AUDIO_BUFFER_SIZE]);

static fir_f32_t fir_left_ch;
static fir_f32_t fir_right_ch;
static biquad_f32_t biquad_left_ch;
static biquad_f32_t biquad_right_ch;

static volatile int32_t buf_out_idx = 0;
static volatile int32_t err_cnt;
static volatile int32_t race_cnt;
static MDMA_HandleTypeDef hmdma;
static MDMA_LinkNodeTypeDef ll_node __attribute__ ((section(".AXI_SRAM")));

static void gather_and_log_fft_time(uint32_t fft_time)
{
    static uint32_t acc_fft_time = 0;
    static int32_t cnt = 0;
    acc_fft_time += fft_time;
    ++cnt;
    if (1 << 15 == cnt)
    {
        event e =
        { .id = EVENT_M7_FFT, .val = acc_fft_time >> 15 };
        eq_m7_add_event(e);
        acc_fft_time = 0;
        cnt = 0;
    }
}

static void gather_and_log_dsp_time(uint32_t dsp_time)
{
    static uint32_t acc_dsp_time = 0;
    static int32_t cnt = 0;
    acc_dsp_time += dsp_time;
    ++cnt;
    if (1 << 17 == cnt)
    {
        event e =
        { .id = EVENT_M7_DSP, .val = acc_dsp_time >> 15 };
        eq_m7_add_event(e);
        acc_dsp_time = 0;
        cnt = 0;
    }
}

static int32_t dsp_fir_left(int32_t input)
{
    return fir_f32(&fir_left_ch, input);
}

static int32_t dsp_fir_right(int32_t input)
{
    return fir_f32(&fir_right_ch, input);
}

static int32_t dsp_biquad_left(int32_t input)
{
    return biquad_f32(&biquad_left_ch, input);
}

static int32_t dsp_biquad_right(int32_t input)
{
    return biquad_f32(&biquad_right_ch, input);
}

static int32_t (*dsp_left)(int32_t input) = &dsp_fir_left;
static int32_t (*dsp_right)(int32_t input) = &dsp_fir_right;

void mdma_callback(MDMA_HandleTypeDef *_hmdma)
{
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    int32_t out;

    uint32_t start = GET_CCNT();

    out = dsp_left(audio_in[0]);
    audio_buffer_out[buf_idx] = out;
    dtcm_buffer_out[buf_idx] = out;
    out = dsp_right(audio_in[1]);
    audio_buffer_out[buf_idx + 1] = out;
    dtcm_buffer_out[buf_idx + 1] = out;

    uint32_t stop = GET_CCNT();
    gather_and_log_dsp_time(DIFF_CCNT(start, stop));

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
static void init_mdma(void);
static void deinit_mdma(void);
static void sync_dsp_filters(uint32_t dsp_mask);

/*----------------------------------------------------------------------------*/
void analog_inout(void)
{
    const uint32_t audio_freq = 48000;
    const uint32_t audio_resolution = 32;

    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_IN_OUT_Init(audio_freq, audio_resolution);
    unlock_hsem(HSEM_I2C4);

    init_mdma();

    err_cnt = 0;
    race_cnt = 0;
    buf_out_idx = AUDIO_BUFFER_SIZE / 2;
    dsp_update_mask = 0;

    // setup FIR filters
    if (fir_orders[0] <= 0 || fir_orders[0] > MAX_FIR_ORDER)
    {
        memset(&fir_coeffs[0][0], 0, sizeof(fir_coeffs[0]));
        fir_orders[0] = 5;
        fir_coeffs[0][0] = 0.0955f;
        fir_coeffs[0][1] = 0.1949f;
        fir_coeffs[0][2] = 0.2624f;
        fir_coeffs[0][3] = 0.2624f;
        fir_coeffs[0][4] = 0.1949f;
        fir_coeffs[0][5] = 0.0955f;
        sync_dsp_filters(0x01U);
    }
    if (fir_orders[1] <= 0 || fir_orders[1] > MAX_FIR_ORDER)
    {
        memset(&fir_coeffs[1][0], 0, sizeof(fir_coeffs[1]));
        fir_orders[1] = 5;
        fir_coeffs[1][0] = 0.0217f;
        fir_coeffs[1][1] = -0.1054f;
        fir_coeffs[1][2] = -0.5978f;
        fir_coeffs[1][3] = 0.5978f;
        fir_coeffs[1][4] = 0.1054f;
        fir_coeffs[1][5] = -0.0217f;
        sync_dsp_filters(0x02U);
    }
    if (biquad_stages[0] <= 0 || biquad_stages[0] > MAX_BIQUAD_STAGES)
    {
        memset(&biquad_coeffs[0][0], 0, sizeof(biquad_coeffs[0]));
        biquad_stages[0] = 2;
        biquad_coeffs[0][0] = 0.0472f;
        biquad_coeffs[0][1] = 0.0198f;
        biquad_coeffs[0][2] = 0.0472f;
        biquad_coeffs[0][3] = 0.4060f;
        biquad_coeffs[0][4] = -0.9598f;
        biquad_coeffs[0][5] = 0.4434f;
        biquad_coeffs[0][6] = 0.5240f;
        biquad_coeffs[0][7] = 0.4434f;
        biquad_coeffs[0][8] = 1.0785f;
        biquad_coeffs[0][9] = -0.8450f;
        sync_dsp_filters(0x04U);
    }
    if (biquad_stages[1] <= 0 || biquad_stages[1] > MAX_BIQUAD_STAGES)
    {
        memset(&biquad_coeffs[1][0], 0, sizeof(biquad_coeffs[1]));
        biquad_stages[1] = 2;
        biquad_coeffs[1][0] = 0.0472f;
        biquad_coeffs[1][1] = 0.0198f;
        biquad_coeffs[1][2] = 0.0472f;
        biquad_coeffs[1][3] = 0.4060f;
        biquad_coeffs[1][4] = -0.9598f;
        biquad_coeffs[1][5] = 0.4434f;
        biquad_coeffs[1][6] = 0.5240f;
        biquad_coeffs[1][7] = 0.4434f;
        biquad_coeffs[1][8] = 1.0785f;
        biquad_coeffs[1][9] = -0.8450f;
        sync_dsp_filters(0x08U);
    }

    while (lock_hsem(HSEM_I2C4))
        ;
    err_cnt += BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in));
    err_cnt += BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out));
    unlock_hsem(HSEM_I2C4);

    while (start_audio == 1)
    {
        if (0 == (buf_out_idx % AUDIO_BUFFER_SIZE))
        {
            uint32_t start = GET_CCNT();
            fft_16hist((int16_t*) &shared_fft_l[0], (int16_t*) &shared_fft_r[0],
                    &dtcm_buffer_out[0]);
            uint32_t stop = GET_CCNT();
            gather_and_log_fft_time(DIFF_CCNT(start, stop));

            ++new_data_flag;
        }
        uint32_t dsp_mask = dsp_update_mask;
        if (dsp_mask != 0)
        {
            dsp_update_mask &= ~dsp_mask;
            sync_dsp_filters(dsp_mask);
        }
    }
    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_OUT_Stop(0);
    BSP_AUDIO_OUT_DeInit(0);
    BSP_AUDIO_IN_Stop(0);
    BSP_AUDIO_IN_DeInit(0);
    unlock_hsem(HSEM_I2C4);

    deinit_mdma();

}

static void sync_dsp_filters(uint32_t dsp_mask)
{
    if (dsp_mask & 0x01U)
    {
        fir_left_ch.order = fir_orders[0];
        memcpy(&fir_left_ch.coeff[0], &fir_coeffs[0][0], (fir_left_ch.order + 1) * sizeof(float));
        dsp_left = &dsp_fir_left;
    }
    if (dsp_mask & 0x02U)
    {
        fir_right_ch.order = fir_orders[1];
        memcpy(&fir_right_ch.coeff[0], &fir_coeffs[1][0], (fir_right_ch.order + 1) * sizeof(float));
        dsp_right = &dsp_fir_right;
    }
    if (dsp_mask & 0x04U)
    {
        biquad_left_ch.n_stage = biquad_stages[0];
        memcpy(&biquad_left_ch.coeff[0], &biquad_coeffs[0][0],
                (biquad_left_ch.n_stage * N_COEFF_IN_STAGE) * sizeof(float));
        dsp_left = &dsp_biquad_left;
    }
    if (dsp_mask & 0x08U)
    {
        biquad_right_ch.n_stage = biquad_stages[1];
        memcpy(&biquad_right_ch.coeff[0], &biquad_coeffs[1][0],
                (biquad_right_ch.n_stage * N_COEFF_IN_STAGE) * sizeof(float));
        dsp_right = &dsp_biquad_right;
    }
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

static void init_mdma(void)
{
    HAL_StatusTypeDef status;
    MDMA_LinkNodeConfTypeDef node_conf;
    event e =
    { .id = EVENT_M7_MDMA_CFG, .val = 0U };

    __HAL_RCC_MDMA_CLK_ENABLE();

    hmdma.Instance = MDMA_Channel1;
    hmdma.Init.BufferTransferLength = 8;
    hmdma.Init.DataAlignment = MDMA_DATAALIGN_RIGHT;
    ;
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
    e.val += status << 0;
    status = HAL_MDMA_RegisterCallback(&hmdma, HAL_MDMA_XFER_REPBLOCKCPLT_CB_ID, &mdma_callback);
    e.val += status << 2;

    HAL_MDMA_ConfigPostRequestMask(&hmdma, (uint32_t) &(DMA2->HIFCR), DMA_HIFCR_CTCIF4);

    memcpy(&node_conf.Init, &hmdma.Init, sizeof(hmdma.Init));
    node_conf.PostRequestMaskAddress = (uint32_t) &(DMA2->HIFCR);
    node_conf.PostRequestMaskData = DMA_HIFCR_CTCIF4;
    node_conf.BlockCount = 1;
    node_conf.BlockDataLength = 8;

    node_conf.DstAddress = (uint32_t) &audio_in[0];
    node_conf.SrcAddress = (uint32_t) &audio_buffer_in[0];

    status = HAL_MDMA_LinkedList_CreateNode(&ll_node, &node_conf);
    e.val += status << 4;
    status = HAL_MDMA_LinkedList_AddNode(&hmdma, &ll_node, 0);
    e.val += status << 6;

    status = HAL_MDMA_LinkedList_EnableCircularMode(&hmdma);
    e.val += status << 8;

    SCB_CleanDCache_by_Addr((uint32_t*) &ll_node, sizeof(ll_node));

    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);

    // Useless parameters, function called to start MDMA in Interrupt Mode
    status = HAL_MDMA_Start_IT(&hmdma, (uint32_t) &audio_buffer_out[0],
            (uint32_t) &dtcm_buffer_out[0], sizeof(uint32_t), 1);
    e.val += status << 10;
    eq_m7_add_event(e);
}

static void deinit_mdma(void)
{
    HAL_MDMA_Abort_IT(&hmdma);
    HAL_MDMA_UnRegisterCallback(&hmdma, HAL_MDMA_XFER_CPLT_CB_ID);
    HAL_MDMA_DeInit(&hmdma);
}

void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(&hmdma);
}

