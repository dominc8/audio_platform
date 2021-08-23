#include "benchmark.h"
#include "perf_meas.h"
#include "event_queue.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "shared_data.h"
#include "fir.h"
#include "biquad.h"

#define INCLUDE_CACHE_OP

#define N_FIR_BM            15
#define N_BIQUAD_BM         15
#define N_FFT_BM            8
#define TAPS_LEN            5
#define STAGES_LEN          5
#define BLOCK_SIZE_LEN      6
#define FFT_SIZE_LEN        5

static const int32_t taps_arr[TAPS_LEN] =
{ 5, 10, 20, 50, 100 };
static const int32_t stages_arr[STAGES_LEN] =
{ 1, 4, 8, 12, 16 };
static const int32_t block_size_arr[BLOCK_SIZE_LEN] =
{ 8, 16, 32, 64, 128, 256 };
static const int32_t fft_size_arr[FFT_SIZE_LEN] =
{ 64, 128, 256, 512, 1024 };

static int32_t dtcm_in[2048] __attribute__ ((aligned (32)));
static int32_t dtcm_out[2048] __attribute__ ((aligned (32)));
static int32_t dtcm_coeff[100] __attribute__ ((aligned (32)));
static int32_t dtcm_state[100 + 256 - 1] __attribute__ ((aligned (32)));

static int32_t cached_in[2048] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_out[2048] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_coeff[100] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_state[100 + 256 - 1] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));

static fir_q31_t dtcm_fir_q31_inst;
static fir_q31_t cached_fir_q31_inst __attribute__ ((section(".AXI_SRAM")));
static fir_f32_t dtcm_fir_f32_inst;
static fir_f32_t cached_fir_f32_inst __attribute__ ((section(".AXI_SRAM")));
static biquad_f32_t dtcm_biquad_f32_inst;
static biquad_f32_t cached_biquad_f32_inst __attribute__ ((section(".AXI_SRAM")));
static biquad_q31_t dtcm_biquad_q31_inst;
static biquad_q31_t cached_biquad_q31_inst __attribute__ ((section(".AXI_SRAM")));

static uint32_t benchmark_fir_f32_custom(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_f32_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            dtcm_out[0] = fir_f32(&dtcm_fir_f32_inst, dtcm_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_fir_f32(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &dtcm_coeff[0], (float*) &dtcm_state[0],
                    block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32(&fir_inst, (float*) &dtcm_in[0], (float*) &dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_f32[bm_idx].n_taps = n_taps;
            fir_measurements_f32[bm_idx].block_size = block_size;
            fir_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_i32(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &dtcm_coeff[0], (float*) &dtcm_state[0],
                    block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32_int(&fir_inst, &dtcm_in[0], &dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_i32[bm_idx].n_taps = n_taps;
            fir_measurements_i32[bm_idx].block_size = block_size;
            fir_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_q31(void)
{
    arm_fir_instance_q31 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &dtcm_coeff[0], &dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_fast_q31(&fir_inst, &dtcm_in[0], &dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            fir_measurements_q31[bm_idx].n_taps = n_taps;
            fir_measurements_q31[bm_idx].block_size = block_size;
            fir_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_f32_custom_cache(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        cached_fir_f32_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_f32(&cached_fir_f32_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_fir_f32_cache(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &cached_coeff[0],
                    (float*) &cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_f32(&fir_inst, (float*) &cached_in[0], (float*) &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_f32[bm_idx].n_taps = n_taps;
            fir_measurements_f32[bm_idx].block_size = block_size;
            fir_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_i32_cache(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &cached_coeff[0],
                    (float*) &cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_f32_int(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_i32[bm_idx].n_taps = n_taps;
            fir_measurements_i32[bm_idx].block_size = block_size;
            fir_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_q31_cache(void)
{
    arm_fir_instance_q31 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &cached_coeff[0], &cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_fast_q31(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            fir_measurements_q31[bm_idx].n_taps = n_taps;
            fir_measurements_q31[bm_idx].block_size = block_size;
            fir_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_f32_custom_cache_data_only(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_f32_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_f32(&dtcm_fir_f32_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_fir_f32_cache_data_only(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_f32(&fir_inst, (float*) &cached_in[0], (float*) &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_f32[bm_idx].n_taps = n_taps;
            fir_measurements_f32[bm_idx].block_size = block_size;
            fir_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_i32_cache_data_only(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_f32_int(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_i32[bm_idx].n_taps = n_taps;
            fir_measurements_i32[bm_idx].block_size = block_size;
            fir_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_q31_cache_data_only(void)
{
    arm_fir_instance_q31 fir_inst;
    int32_t bm_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &dtcm_coeff[0], &dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_fir_fast_q31(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            fir_measurements_q31[bm_idx].n_taps = n_taps;
            fir_measurements_q31[bm_idx].block_size = block_size;
            fir_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}


static uint32_t benchmark_fir_q31_custom(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_q31_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            dtcm_out[0] = fir_q31(&dtcm_fir_q31_inst, dtcm_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_fir_q31_custom_cache(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        cached_fir_q31_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_q31(&cached_fir_q31_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_fir_q31_custom_cache_data_only(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_q31_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_q31(&dtcm_fir_q31_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        fir_measurements_custom[i].n_taps = n_taps;
        fir_measurements_custom[i].block_size = 1;
        fir_measurements_custom[i].cycles = acc;
    }
    return 0;
}



static uint32_t benchmark_biquad_f32_custom(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        dtcm_biquad_f32_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            dtcm_out[0] = biquad_f32(&dtcm_biquad_f32_inst, dtcm_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_biquad_f32(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df2T_f32(&biquad_inst, (float*) &dtcm_in[0],
                        (float*) &dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_f32[bm_idx].n_taps = n_stages;
            biquad_measurements_f32[bm_idx].block_size = block_size;
            biquad_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_i32(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df2T_f32_int(&biquad_inst, &dtcm_in[0], &dtcm_out[0],
                        block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_i32[bm_idx].n_taps = n_stages;
            biquad_measurements_i32[bm_idx].block_size = block_size;
            biquad_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_q31(void)
{
    arm_biquad_casd_df1_inst_q31 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df1_init_q31(&biquad_inst, n_stages, &dtcm_coeff[0], &dtcm_state[0],
                    1);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 14;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df1_fast_q31(&biquad_inst, &dtcm_in[0], &dtcm_out[0],
                        block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 14;
            biquad_measurements_q31[bm_idx].n_taps = n_stages;
            biquad_measurements_q31[bm_idx].block_size = block_size;
            biquad_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_f32_custom_cache(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        cached_biquad_f32_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = biquad_f32(&cached_biquad_f32_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_biquad_f32_cache(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &cached_coeff[0],
                    (float*) &cached_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df2T_f32(&biquad_inst, (float*) &cached_in[0],
                        (float*) &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_f32[bm_idx].n_taps = n_stages;
            biquad_measurements_f32[bm_idx].block_size = block_size;
            biquad_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_i32_cache(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &cached_coeff[0],
                    (float*) &cached_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df2T_f32_int(&biquad_inst, &cached_in[0], &cached_out[0],
                        block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_i32[bm_idx].n_taps = n_stages;
            biquad_measurements_i32[bm_idx].block_size = block_size;
            biquad_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_q31_cache(void)
{
    arm_biquad_casd_df1_inst_q31 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df1_init_q31(&biquad_inst, n_stages, &cached_coeff[0],
                    &cached_state[0], 1);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 14;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df1_fast_q31(&biquad_inst, &cached_in[0], &cached_out[0],
                        block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 14;
            biquad_measurements_q31[bm_idx].n_taps = n_stages;
            biquad_measurements_q31[bm_idx].block_size = block_size;
            biquad_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_f32_custom_cache_data_only(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        dtcm_biquad_f32_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = biquad_f32(&dtcm_biquad_f32_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_biquad_f32_cache_data_only(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df2T_f32(&biquad_inst, (float*) &cached_in[0],
                        (float*) &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_f32[bm_idx].n_taps = n_stages;
            biquad_measurements_f32[bm_idx].block_size = block_size;
            biquad_measurements_f32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_i32_cache_data_only(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &dtcm_coeff[0],
                    (float*) &dtcm_state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df2T_f32_int(&biquad_inst, &cached_in[0], &cached_out[0],
                        block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            biquad_measurements_i32[bm_idx].n_taps = n_stages;
            biquad_measurements_i32[bm_idx].block_size = block_size;
            biquad_measurements_i32[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_q31_cache_data_only(void)
{
    arm_biquad_casd_df1_inst_q31 biquad_inst;
    int32_t bm_idx = 0;

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df1_init_q31(&biquad_inst, n_stages, &dtcm_coeff[0],
                    &dtcm_state[0], 1);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 14;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]) * block_size);
#endif
                arm_biquad_cascade_df1_fast_q31(&biquad_inst, &cached_in[0], &cached_out[0],
                        block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]) * block_size);
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 14;
            biquad_measurements_q31[bm_idx].n_taps = n_stages;
            biquad_measurements_q31[bm_idx].block_size = block_size;
            biquad_measurements_q31[bm_idx].cycles = acc;
            ++bm_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_biquad_q31_custom(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        dtcm_biquad_q31_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            dtcm_out[0] = biquad_q31(&dtcm_biquad_q31_inst, dtcm_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_biquad_q31_custom_cache(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        cached_biquad_q31_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = biquad_q31(&cached_biquad_q31_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_biquad_q31_custom_cache_data_only(void)
{
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        dtcm_biquad_q31_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr((uint32_t *)&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = biquad_q31(&dtcm_biquad_q31_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr((uint32_t *)&cached_out[0], sizeof(cached_out[0]));
#endif
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        biquad_measurements_custom[i].n_taps = n_stages;
        biquad_measurements_custom[i].block_size = 1;
        biquad_measurements_custom[i].cycles = acc;
    }
    return 0;
}



static uint32_t benchmark_rfft_f32(void)
{
    arm_rfft_fast_instance_f32 arm_rfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        arm_rfft_fast_init_f32(&arm_rfft, fft_size);
        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_rfft_fast_f32(&arm_rfft, (float*) &dtcm_in[0], (float*) &dtcm_out[0], 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        rfft_measurements_f32[i].block_size = fft_size;
        rfft_measurements_f32[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_rfft_q31(void)
{
    arm_rfft_instance_q31 arm_rfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        arm_rfft_init_q31(&arm_rfft, fft_size, 0, 0);
        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_rfft_q31(&arm_rfft, &dtcm_in[0], &dtcm_out[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        rfft_measurements_q31[i].block_size = fft_size;
        rfft_measurements_q31[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_cfft_f32(void)
{
    const arm_cfft_instance_f32 *arm_cfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
                arm_cfft = &arm_cfft_sR_f32_len64;
                break;
            case 128:
                arm_cfft = &arm_cfft_sR_f32_len128;
                break;
            case 256:
                arm_cfft = &arm_cfft_sR_f32_len256;
                break;
            case 512:
                arm_cfft = &arm_cfft_sR_f32_len512;
                break;
            case 1024:
                arm_cfft = &arm_cfft_sR_f32_len1024;
                break;
        }

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_cfft_f32(arm_cfft, (float*) &dtcm_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        cfft_measurements_f32[i].block_size = fft_size;
        cfft_measurements_f32[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_cfft_q31(void)
{
    const arm_cfft_instance_q31 *arm_cfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
                arm_cfft = &arm_cfft_sR_q31_len64;
                break;
            case 128:
                arm_cfft = &arm_cfft_sR_q31_len128;
                break;
            case 256:
                arm_cfft = &arm_cfft_sR_q31_len256;
                break;
            case 512:
                arm_cfft = &arm_cfft_sR_q31_len512;
                break;
            case 1024:
                arm_cfft = &arm_cfft_sR_q31_len1024;
                break;
        }

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_cfft_q31(arm_cfft, &dtcm_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        cfft_measurements_q31[i].block_size = fft_size;
        cfft_measurements_q31[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_rfft_f32_cache(void)
{
    arm_rfft_fast_instance_f32 arm_rfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        arm_rfft_fast_init_f32(&arm_rfft, fft_size);
        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_rfft_fast_f32(&arm_rfft, (float*) &cached_in[0], (float*) &cached_out[0], 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        rfft_measurements_f32[i].block_size = fft_size;
        rfft_measurements_f32[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_rfft_q31_cache(void)
{
    arm_rfft_instance_q31 arm_rfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        arm_rfft_init_q31(&arm_rfft, fft_size, 0, 0);
        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_rfft_q31(&arm_rfft, &cached_in[0], &cached_out[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        rfft_measurements_q31[i].block_size = fft_size;
        rfft_measurements_q31[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_cfft_f32_cache(void)
{
    const arm_cfft_instance_f32 *arm_cfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
                arm_cfft = &arm_cfft_sR_f32_len64;
                break;
            case 128:
                arm_cfft = &arm_cfft_sR_f32_len128;
                break;
            case 256:
                arm_cfft = &arm_cfft_sR_f32_len256;
                break;
            case 512:
                arm_cfft = &arm_cfft_sR_f32_len512;
                break;
            case 1024:
                arm_cfft = &arm_cfft_sR_f32_len1024;
                break;
        }

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_cfft_f32(arm_cfft, (float*) &cached_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        cfft_measurements_f32[i].block_size = fft_size;
        cfft_measurements_f32[i].cycles = acc;
    }
    return 0;
}

static uint32_t benchmark_cfft_q31_cache(void)
{
    const arm_cfft_instance_q31 *arm_cfft;

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
                arm_cfft = &arm_cfft_sR_q31_len64;
                break;
            case 128:
                arm_cfft = &arm_cfft_sR_q31_len128;
                break;
            case 256:
                arm_cfft = &arm_cfft_sR_q31_len256;
                break;
            case 512:
                arm_cfft = &arm_cfft_sR_q31_len512;
                break;
            case 1024:
                arm_cfft = &arm_cfft_sR_q31_len1024;
                break;
        }

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 12;

        while (n-- > 0)
        {
            start = GET_CCNT();
            arm_cfft_q31(arm_cfft, &cached_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        cfft_measurements_q31[i].block_size = fft_size;
        cfft_measurements_q31[i].cycles = acc;
    }
    return 0;
}

static uint32_t (*fir_benchmarks[N_FIR_BM])(void) =
{
    &benchmark_fir_f32_custom,
    &benchmark_fir_f32,
    &benchmark_fir_q31_custom,
    &benchmark_fir_q31,
    &benchmark_fir_i32,
    &benchmark_fir_f32_custom_cache,
    &benchmark_fir_f32_cache,
    &benchmark_fir_q31_custom_cache,
    &benchmark_fir_q31_cache,
    &benchmark_fir_i32_cache,
    &benchmark_fir_f32_custom_cache_data_only,
    &benchmark_fir_f32_cache_data_only,
    &benchmark_fir_q31_custom_cache_data_only,
    &benchmark_fir_q31_cache_data_only,
    &benchmark_fir_i32_cache_data_only,
};

static uint32_t (*biquad_benchmarks[N_BIQUAD_BM])(void) =
{
    &benchmark_biquad_f32_custom,
    &benchmark_biquad_f32,
    &benchmark_biquad_q31_custom,
    &benchmark_biquad_q31,
    &benchmark_biquad_i32,
    &benchmark_biquad_f32_custom_cache,
    &benchmark_biquad_f32_cache,
    &benchmark_biquad_q31_custom_cache,
    &benchmark_biquad_q31_cache,
    &benchmark_biquad_i32_cache,
    &benchmark_biquad_f32_custom_cache_data_only,
    &benchmark_biquad_f32_cache_data_only,
    &benchmark_biquad_q31_custom_cache_data_only,
    &benchmark_biquad_q31_cache_data_only,
    &benchmark_biquad_i32_cache_data_only,
};

static uint32_t (*fft_benchmarks[N_FFT_BM])(void) =
{
    &benchmark_rfft_f32,
    &benchmark_rfft_q31,
    &benchmark_cfft_f32,
    &benchmark_cfft_q31,
    &benchmark_rfft_f32_cache,
    &benchmark_rfft_q31_cache,
    &benchmark_cfft_f32_cache,
    &benchmark_cfft_q31_cache,
};

static EVENT_ID bm_fir_events[N_FIR_BM] =
{
    EVENT_BM_FIR_F32_CUSTOM,
    EVENT_BM_FIR_F32,
    EVENT_BM_FIR_Q31_CUSTOM,
    EVENT_BM_FIR_Q31,
    EVENT_BM_FIR_I32,
    EVENT_BM_FIR_F32_CUSTOM_CACHE,
    EVENT_BM_FIR_F32_CACHE,
    EVENT_BM_FIR_Q31_CUSTOM_CACHE,
    EVENT_BM_FIR_Q31_CACHE,
    EVENT_BM_FIR_I32_CACHE,
    EVENT_BM_FIR_F32_CUSTOM_CACHE_DATA_ONLY,
    EVENT_BM_FIR_F32_CACHE_DATA_ONLY,
    EVENT_BM_FIR_Q31_CUSTOM_CACHE_DATA_ONLY,
    EVENT_BM_FIR_Q31_CACHE_DATA_ONLY,
    EVENT_BM_FIR_I32_CACHE_DATA_ONLY,
};

static EVENT_ID bm_biquad_events[N_BIQUAD_BM] =
{
    EVENT_BM_BIQUAD_F32_CUSTOM,
    EVENT_BM_BIQUAD_F32,
    EVENT_BM_BIQUAD_Q31_CUSTOM,
    EVENT_BM_BIQUAD_Q31,
    EVENT_BM_BIQUAD_I32,
    EVENT_BM_BIQUAD_F32_CUSTOM_CACHE,
    EVENT_BM_BIQUAD_F32_CACHE,
    EVENT_BM_BIQUAD_Q31_CUSTOM_CACHE,
    EVENT_BM_BIQUAD_Q31_CACHE,
    EVENT_BM_BIQUAD_I32_CACHE,
    EVENT_BM_BIQUAD_F32_CUSTOM_CACHE_DATA_ONLY,
    EVENT_BM_BIQUAD_F32_CACHE_DATA_ONLY,
    EVENT_BM_BIQUAD_Q31_CUSTOM_CACHE_DATA_ONLY,
    EVENT_BM_BIQUAD_Q31_CACHE_DATA_ONLY,
    EVENT_BM_BIQUAD_I32_CACHE_DATA_ONLY,
};

static EVENT_ID bm_fft_events[N_FFT_BM] =
{
    EVENT_BM_RFFT_F32,
    EVENT_BM_RFFT_Q31,
    EVENT_BM_CFFT_F32,
    EVENT_BM_CFFT_Q31,
    EVENT_BM_RFFT_F32_CACHE,
    EVENT_BM_RFFT_Q31_CACHE,
    EVENT_BM_CFFT_F32_CACHE,
    EVENT_BM_CFFT_Q31_CACHE,
};

void benchmark(void)
{
    n_m7_bm_left = N_FIR_BM + N_BIQUAD_BM + N_FFT_BM;
    for (int32_t bm = 0; bm < N_FIR_BM; ++bm, --n_m7_bm_left)
    {
        uint32_t result = fir_benchmarks[bm]();
        event e =
        { .id = bm_fir_events[bm], .val = result };
        eq_m7_add_event(e);
    }
    for (int32_t bm = 0; bm < N_BIQUAD_BM; ++bm, --n_m7_bm_left)
    {
        uint32_t result = biquad_benchmarks[bm]();
        event e =
        { .id = bm_biquad_events[bm], .val = result };
        eq_m7_add_event(e);
    }
    for (int32_t bm = 0; bm < N_FFT_BM; ++bm, --n_m7_bm_left)
    {
        uint32_t result = fft_benchmarks[bm]();
        event e =
        { .id = bm_fft_events[bm], .val = result };
        eq_m7_add_event(e);
    }
}

