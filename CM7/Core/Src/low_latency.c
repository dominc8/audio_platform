/* Includes ------------------------------------------------------------------*/
#include <error_handler.h>
#include "stm32h747i_discovery_audio.h"
#include "stm32h7xx_it.h"
#include "string.h"
#include "shared_data.h"
#include "fft_hist.h"
#include "intercore_comm.h"
#include "event_queue.h"
#include "perf_meas.h"
#include "fir.h"
#include "biquad.h"
#include "arm_math.h"
#include "trace.h"
#include "math.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)2)
#define N_AUDIO_BLOCKS              ((uint32_t)8)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))
#define FFT_N_SAMPLES               ((uint32_t)256)
#define FFT_GAMMA_SHIFT             (3)

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static int32_t audio_buffer_in[AUDIO_BLOCK_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t audio_buffer_out[AUDIO_BUFFER_SIZE]) __attribute__ ((section(".AXI_SRAM")));
ALIGN_32BYTES(static int32_t audio_in[2]);
ALIGN_32BYTES(static float audio_tmp[2][FFT_N_SAMPLES]);
ALIGN_32BYTES(static float audio_fft[FFT_N_SAMPLES]);
ALIGN_32BYTES(static uint16_t last_fft_bins[2][SHARED_FFT_SIZE]);

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
    if (1 << 15 == cnt)
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

static void mdma_callback(MDMA_HandleTypeDef *_hmdma)
{
    (void) _hmdma;
    int32_t out;

    TRACE_START1;

    int32_t buf_idx = buf_out_idx;
    uint32_t start = GET_CCNT();

    out = dsp_left(audio_in[0]);
    audio_buffer_out[buf_idx] = out;
    out = dsp_right(audio_in[1]);
    audio_buffer_out[buf_idx + 1] = out;

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in));

    uint32_t stop = GET_CCNT();
    gather_and_log_dsp_time(DIFF_CCNT(start, stop));

    buf_out_idx += AUDIO_BLOCK_SIZE;
    buf_out_idx %= AUDIO_BUFFER_SIZE;

    TRACE_STOP1;
}

/* Private function prototypes -----------------------------------------------*/
static void init_mdma(void);
static void deinit_mdma(void);
static void setup_n_freq_in_bins(int32_t *n_freq_in_bins, int32_t n_freqs);
static void setup_filters(void);
static void sync_dsp_filters(uint32_t dsp_mask);
static void update_fft_bins_channel(float *new_fft, const int32_t *n_freq_in_bins, int32_t channel);
static void apply_gamma_fft_bins_channel(int32_t channel);
static inline float fft_power(float re, float im);

/*----------------------------------------------------------------------------*/
void low_latency(void)
{
    int32_t n_freq_in_bins[SHARED_FFT_SIZE];

    const uint32_t audio_freq = 48000;
    const uint32_t audio_resolution = 32;
    int32_t tmp_idx = 0;
    int32_t already_copied = 0;
    arm_rfft_fast_instance_f32 arm_rfft;
    arm_rfft_fast_init_f32(&arm_rfft, FFT_N_SAMPLES);
    setup_n_freq_in_bins(&n_freq_in_bins[0], FFT_N_SAMPLES / 2);

    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_IN_OUT_Init(audio_freq, audio_resolution);
    unlock_hsem(HSEM_I2C4);

    init_mdma();

    err_cnt = 0;
    race_cnt = 0;
    buf_out_idx = AUDIO_BUFFER_SIZE / 2;
    dsp_update_mask = 0;

    setup_filters();

    while (lock_hsem(HSEM_I2C4))
        ;
    err_cnt += BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in));
    err_cnt += BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out));
    unlock_hsem(HSEM_I2C4);

    while (M7_LOW_LATENCY == m7_state)
    {
        if (0 == (buf_out_idx % AUDIO_BUFFER_SIZE))
        {
            if (already_copied == 1)
                continue;
            already_copied = 1;
            uint32_t start = GET_CCNT();
            float *audio_left = &audio_tmp[0][tmp_idx];
            float *audio_right = &audio_tmp[1][tmp_idx];
            int32_t *audio_out = &audio_buffer_out[0];
            int32_t i;
            int32_t n_samples = (AUDIO_BUFFER_SIZE > (2 * FFT_N_SAMPLES) ?
                            (2 * FFT_N_SAMPLES) : AUDIO_BUFFER_SIZE);

            for (i = n_samples; i > 0; i -= 2)
            {
                /* >> 8 because of 24bit samples being positioned that way,
                 * prevents potential overflows */
                *audio_left++ = *audio_out++ >> 8;
                *audio_right++ = *audio_out++ >> 8;
            }
            tmp_idx += n_samples / 2;
            tmp_idx %= FFT_N_SAMPLES;

            if (0 == tmp_idx && new_data_flag < SHARED_FFT_SLICE_RATE)
            {
                if (0 == new_data_flag)
                {
                    memcpy(&last_fft_bins[0][0], (const void*) &shared_fft[0][0],
                            sizeof(shared_fft));
                }
                /* CMSIS DSP rfft for each frequency puts real and complex parts
                 * next to each other. For f=0 and f=fs/2 there are no complex parts
                 * so first 2 values are not real and complex for f=0 but real values
                 * for f=0 and f=fs/2. Because of that for bin=0 value is additionally
                 * divided by 2. Also it seems that audio codec on this board has
                 * much bigger gain for low frequencies (possibly to be fixed with
                 * some default filter).
                 * */

                // left
                arm_rfft_fast_f32(&arm_rfft, &audio_tmp[0][0], &audio_fft[0], 0);
                audio_fft[0] *= 0.5F;
                audio_fft[1] *= 0.5F;
                update_fft_bins_channel(&audio_fft[0], &n_freq_in_bins[0], 0);

                // right
                arm_rfft_fast_f32(&arm_rfft, &audio_tmp[1][0], &audio_fft[0], 0);
                audio_fft[0] *= 0.5F;
                audio_fft[1] *= 0.5F;
                update_fft_bins_channel(&audio_fft[0], &n_freq_in_bins[0], 1);

                ++new_data_flag;

                if (new_data_flag == SHARED_FFT_SLICE_RATE)
                {
                    apply_gamma_fft_bins_channel(0);
                    apply_gamma_fft_bins_channel(1);
                }
            }
            uint32_t stop = GET_CCNT();
            gather_and_log_fft_time(DIFF_CCNT(start, stop));

        }
        else
        {
            already_copied = 0;
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

static void update_fft_bins_channel(float *new_fft, const int32_t *n_freq_in_bins, int32_t channel)
{
    int32_t i = 0;
    for (int32_t bin = 0; bin < SHARED_FFT_SIZE; ++bin)
    {
        float peak_in_bin = 0.F;
        for (int32_t freq_idx = 0; freq_idx < n_freq_in_bins[bin]; ++freq_idx)
        {
            float freq_power = fft_power(new_fft[i], new_fft[i + 1]);
            if (freq_power > peak_in_bin)
            {
                peak_in_bin = freq_power;
            }
            i += 2;
        }
        uint32_t val = peak_in_bin;
        val = val >> 17;
        if (0 == new_data_flag || (new_data_flag > 0 && shared_fft[channel][bin] < val))
        {
            shared_fft[channel][bin] = val;
        }
    }
}

static void apply_gamma_fft_bins_channel(int32_t channel)
{
    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        uint32_t curr = shared_fft[channel][i];
        uint32_t prev = last_fft_bins[channel][i];
        curr = curr - (curr >> FFT_GAMMA_SHIFT);
        prev = prev >> FFT_GAMMA_SHIFT;
        shared_fft[channel][i] = curr + prev;
    }
}

static inline float fft_power(float re, float im)
{
    return __builtin_sqrtf(re * re + im * im / (1 << 24));
}

static void setup_n_freq_in_bins(int32_t *n_freq_in_bins, int32_t n_freqs)
{
    const int32_t n_bins = SHARED_FFT_SIZE;
	int32_t f_start = 0;

	for (int32_t i = 0; i < n_bins; i++) {
		int32_t f_end = (powf(((float)(i + 1)) /
			(float)n_bins, 2) * n_freqs);
		int f_width;

		if (f_end > n_freqs)
			f_end = n_freqs;

		f_width = f_end - f_start;
		if (f_width <= 0)
        {
			f_width = 1;
            f_end = f_start + 1;
        }
        n_freq_in_bins[i] = f_width;
		f_start = f_end;
	}
}

static void setup_filters(void)
{
    if (fir_orders[0] <= 0 || fir_orders[0] > MAX_FIR_ORDER)
    {
        memset((void*) &fir_coeffs[0][0], 0, sizeof(fir_coeffs[0]));
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
        memset((void*) &fir_coeffs[1][0], 0, sizeof(fir_coeffs[1]));
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
        memset((void*) &biquad_coeffs[0][0], 0, sizeof(biquad_coeffs[0]));
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
        memset((void*) &biquad_coeffs[1][0], 0, sizeof(biquad_coeffs[1]));
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
}

static void sync_dsp_filters(uint32_t dsp_mask)
{
    if (dsp_mask & 0x01U)
    {
        fir_left_ch.order = fir_orders[0];
        memcpy((void*) &fir_left_ch.coeff[0], (void*) &fir_coeffs[0][0],
                (fir_left_ch.order + 1) * sizeof(float));
        dsp_left = &dsp_fir_left;
    }
    if (dsp_mask & 0x02U)
    {
        fir_right_ch.order = fir_orders[1];
        memcpy((void*) &fir_right_ch.coeff[0], (void*) &fir_coeffs[1][0],
                (fir_right_ch.order + 1) * sizeof(float));
        dsp_right = &dsp_fir_right;
    }
    if (dsp_mask & 0x04U)
    {
        biquad_left_ch.n_stage = biquad_stages[0];
        memcpy((void*) &biquad_left_ch.coeff[0], (void*) &biquad_coeffs[0][0],
                (biquad_left_ch.n_stage * N_COEFF_IN_STAGE) * sizeof(float));
        dsp_left = &dsp_biquad_left;
    }
    if (dsp_mask & 0x08U)
    {
        biquad_right_ch.n_stage = biquad_stages[1];
        memcpy((void*) &biquad_right_ch.coeff[0], (void*) &biquad_coeffs[1][0],
                (biquad_right_ch.n_stage * N_COEFF_IN_STAGE) * sizeof(float));
        dsp_right = &dsp_biquad_right;
    }
}
#if 0

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
    UNUSED(Instance);
    Error_Handler();
}
#endif

static void init_mdma(void)
{
    HAL_StatusTypeDef status;
    MDMA_LinkNodeConfTypeDef node_conf;
    event e =
    { .id = EVENT_M7_MDMA_CFG, .val = 0U };

    __HAL_RCC_MDMA_CLK_ENABLE();
    set_mdma_handler(&hmdma);

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
    status = HAL_MDMA_Start_IT(&hmdma, (uint32_t) &audio_buffer_out[0], (uint32_t) &audio_tmp[0],
            sizeof(uint32_t), 1);
    e.val += status << 10;
    eq_m7_add_event(e);
}

static void deinit_mdma(void)
{
    HAL_MDMA_Abort_IT(&hmdma);
    HAL_MDMA_UnRegisterCallback(&hmdma, HAL_MDMA_XFER_CPLT_CB_ID);
    HAL_MDMA_DeInit(&hmdma);
}

