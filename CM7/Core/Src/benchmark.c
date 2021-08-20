#include "benchmark.h"
#include "perf_meas.h"
#include "event_queue.h"
#include "arm_math.h"
#include "shared_data.h"
#include "fir.h"

#define INCLUDE_CACHE_OP

#define N_BENCHMARKS        5
#define TAPS_LEN            5
#define BLOCK_SIZE_LEN      6

static const int32_t taps_arr[TAPS_LEN] = { 5, 10, 20, 50, 100 };
static const int32_t block_size_arr[BLOCK_SIZE_LEN] = { 8, 16, 32, 64, 128, 256 };

static int32_t dtcm_in[256] __attribute__ ((aligned (32)));
static int32_t dtcm_out[256] __attribute__ ((aligned (32)));
static int32_t dtcm_coeff[100] __attribute__ ((aligned (32)));
static int32_t dtcm_state[100 + 256 - 1] __attribute__ ((aligned (32)));

static int32_t cached_in[256] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_out[256] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_coeff[100] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));
static int32_t cached_state[100 + 256 - 1] __attribute__ ((aligned (32))) __attribute__ ((section(".AXI_SRAM")));

static fir_f32_t dtcm_fir_inst;
static fir_f32_t cached_fir_inst __attribute__ ((section(".AXI_SRAM")));

static uint32_t benchmark_fir_custom(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            dtcm_out[0] = fir_f32(&dtcm_fir_inst, dtcm_in[0]);
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
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*)&dtcm_coeff[0], (float*)&dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32(&fir_inst, (float*)&dtcm_in[0], (float*)&dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_f32[fir_meas_idx].n_taps = n_taps;
            fir_measurements_f32[fir_meas_idx].block_size = block_size;
            fir_measurements_f32[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_i32(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*)&dtcm_coeff[0], (float*)&dtcm_state[0], block_size);

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
            fir_measurements_i32[fir_meas_idx].n_taps = n_taps;
            fir_measurements_i32[fir_meas_idx].block_size = block_size;
            fir_measurements_i32[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_q31(void)
{
    arm_fir_instance_q31 fir_inst;
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &dtcm_coeff[0], &dtcm_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_fast_q31(&fir_inst, &dtcm_in[0], &dtcm_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_q31[fir_meas_idx].n_taps = n_taps;
            fir_measurements_q31[fir_meas_idx].block_size = block_size;
            fir_measurements_q31[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_custom_cache(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        cached_fir_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr(&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_f32(&cached_fir_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr(&cached_out[0], sizeof(cached_out[0]));
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
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*)&cached_coeff[0], (float*)&cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr(&cached_in[0], sizeof(cached_in));
#endif
                arm_fir_f32(&fir_inst, (float*)&cached_in[0], (float*)&cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr(&cached_out[0], sizeof(cached_out));
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_f32[fir_meas_idx].n_taps = n_taps;
            fir_measurements_f32[fir_meas_idx].block_size = block_size;
            fir_measurements_f32[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_i32_cache(void)
{
    arm_fir_instance_f32 fir_inst;
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*)&cached_coeff[0], (float*)&cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr(&cached_in[0], sizeof(cached_in));
#endif
                arm_fir_f32_int(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr(&cached_out[0], sizeof(cached_out));
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_i32[fir_meas_idx].n_taps = n_taps;
            fir_measurements_i32[fir_meas_idx].block_size = block_size;
            fir_measurements_i32[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_q31_cache(void)
{
    arm_fir_instance_q31 fir_inst;
    int32_t fir_meas_idx = 0;

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &cached_coeff[0], &cached_state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
                SCB_InvalidateDCache_by_Addr(&cached_in[0], sizeof(cached_in));
#endif
                arm_fir_fast_q31(&fir_inst, &cached_in[0], &cached_out[0], block_size);
#ifdef INCLUDE_CACHE_OP
                SCB_CleanDCache_by_Addr(&cached_out[0], sizeof(cached_out));
#endif
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements_q31[fir_meas_idx].n_taps = n_taps;
            fir_measurements_q31[fir_meas_idx].block_size = block_size;
            fir_measurements_q31[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t benchmark_fir_custom_cache_data_only(void)
{
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        dtcm_fir_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
#ifdef INCLUDE_CACHE_OP
            SCB_InvalidateDCache_by_Addr(&cached_in[0], sizeof(cached_in[0]));
#endif
            cached_out[0] = fir_f32(&dtcm_fir_inst, cached_in[0]);
#ifdef INCLUDE_CACHE_OP
            SCB_CleanDCache_by_Addr(&cached_out[0], sizeof(cached_out[0]));
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



static uint32_t (*benchmarks[N_BENCHMARKS])(void) =
{
    &benchmark_fir_custom,
    &benchmark_fir_f32,
//    &benchmark_fir_i32,
//    &benchmark_fir_q31,
    &benchmark_fir_custom_cache,
    &benchmark_fir_f32_cache,
//    &benchmark_fir_i32_cache,
//    &benchmark_fir_q31_cache,
    &benchmark_fir_custom_cache_data_only,
};

static EVENT_ID bm_events[N_BENCHMARKS] =
{
    EVENT_BM_FIR_CUSTOM,
    EVENT_BM_FIR_F32,
//    EVENT_BM_FIR_I32,
//    EVENT_BM_FIR_Q31,
    EVENT_BM_FIR_CUSTOM_CACHE,
    EVENT_BM_FIR_F32_CACHE,
//    EVENT_BM_FIR_I32_CACHE,
//    EVENT_BM_FIR_Q31_CACHE,
    EVENT_BM_FIR_CUSTOM_CACHE_DATA_ONLY,
};


void benchmark(void)
{
    n_m7_bm_left = N_BENCHMARKS;
    for (int32_t bm = 0; bm < N_BENCHMARKS; ++bm, --n_m7_bm_left)
    {
        uint32_t result = benchmarks[bm]();
        event e = { .id = bm_events[bm], .val = result };
        eq_m7_add_event(e);
    }
}
