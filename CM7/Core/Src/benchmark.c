#include "benchmark.h"
#include "perf_meas.h"
#include "event_queue.h"
#include "arm_math.h"
#include "shared_data.h"

#define N_BENCHMARKS        3
#define TAPS_LEN            5
#define BLOCK_SIZE_LEN      6

static int32_t dtcm_in_i32[256] __attribute__ ((aligned (32)));
static int32_t dtcm_out_i32[256] __attribute__ ((aligned (32)));
static int32_t coeff_q31[100] __attribute__ ((aligned (32)));
static int32_t state_q31[100 + 256 - 1] __attribute__ ((aligned (32)));
static float dtcm_in_f32[256] __attribute__ ((aligned (32)));
static float dtcm_out_f32[256] __attribute__ ((aligned (32)));
static float coeff_f32[100] __attribute__ ((aligned (32)));
static float state_f32[100 + 256 - 1] __attribute__ ((aligned (32)));

static const int32_t taps_arr[TAPS_LEN] = { 5, 10, 20, 50, 100 };
static const int32_t block_size_arr[BLOCK_SIZE_LEN] = { 8, 16, 32, 64, 128, 256 };


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
            arm_fir_init_f32(&fir_inst, n_taps, &coeff_f32[0], &state_f32[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32(&fir_inst, &dtcm_in_f32[0], &dtcm_out_f32[0], block_size);
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
            arm_fir_init_f32(&fir_inst, n_taps, &coeff_f32[0], &state_f32[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32_int(&fir_inst, &dtcm_in_i32[0], &dtcm_out_i32[0], block_size);
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
            arm_fir_init_q31(&fir_inst, n_taps, &coeff_q31[0], &state_q31[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_fast_q31(&fir_inst, &dtcm_in_i32[0], &dtcm_out_i32[0], block_size);
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

static uint32_t (*benchmarks[N_BENCHMARKS])(void) =
{
    &benchmark_fir_f32,
    &benchmark_fir_i32,
    &benchmark_fir_q31,
};

static EVENT_ID bm_events[N_BENCHMARKS] =
{
    EVENT_BM_FIR_F32,
    EVENT_BM_FIR_I32,
    EVENT_BM_FIR_Q31,
};


void benchmark(void)
{
    for (int32_t bm = 0; bm < N_BENCHMARKS; ++bm)
    {
        uint32_t result = benchmarks[bm]();
        event e = { .id = bm_events[bm], .val = result };
        eq_m7_add_event(e);
    }
}
