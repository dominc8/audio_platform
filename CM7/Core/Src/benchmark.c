#include "benchmark.h"
#include "perf_meas.h"
#include "event_queue.h"
#include "arm_math.h"
#include "shared_data.h"

#define N_BENCHMARKS    3

static uint32_t benchmark_empty(void)
{
    uint32_t start, stop;
    uint32_t acc = 0;
    int32_t n = 1 << 15;

    while (n-- > 0)
    {
        start = GET_CCNT();
        stop = GET_CCNT();
        acc += DIFF_CCNT(start, stop);
    }

    acc = acc >> 15;
    return acc;
}

static uint32_t benchmark_add(void)
{
    uint32_t start, stop;
    volatile uint32_t acc = 0;
    int32_t n = 1 << 15;

    while (n-- > 0)
    {
        start = GET_CCNT();
        acc += 10;
        stop = GET_CCNT();
        acc += DIFF_CCNT(start, stop);
    }

    acc -= 10 << 15;
    acc = acc >> 15;
    return acc;
}

static uint32_t benchmark_fir(void)
{
//    static int32_t dtcm_in[256];
//    static int32_t dtcm_out[256];
    static float dtcm_in[256];
    static float dtcm_out[256];
    static float coeff[100];
    static float state[100 + 256 - 1];
    static const int32_t taps[] = { 5, 10, 20, 50, 100 };
    static const int32_t block_size[] = { 8, 16, 32, 64, 128, 256 };
    const int32_t n_taps = 5;
    const int32_t n_block_size = 6;
    arm_fir_instance_f32 fir_f32_inst;
    int32_t fir_meas_idx = 0;


    for (int32_t n_tap_idx = 0; n_tap_idx < n_taps; ++n_tap_idx)
    {
        int32_t num_taps = taps[n_tap_idx];
        for (int32_t n_block_size_idx = 0; n_block_size_idx < n_block_size; ++n_block_size_idx)
        {
            int32_t b_size = block_size[n_block_size_idx];
            arm_fir_init_f32(&fir_f32_inst, num_taps, &coeff[0], &state[0], b_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 15;

            while (n-- > 0)
            {
                start = GET_CCNT();
//                arm_fir_f32_int(&fir_f32_inst, &dtcm_in[0], &dtcm_out[0], b_size);
                arm_fir_f32(&fir_f32_inst, &dtcm_in[0], &dtcm_out[0], b_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 15;
            fir_measurements[fir_meas_idx].n_taps = num_taps;
            fir_measurements[fir_meas_idx].block_size = b_size;
            fir_measurements[fir_meas_idx].cycles = acc;
            ++fir_meas_idx;
        }
    }
    return 0;
}

static uint32_t (*benchmarks[N_BENCHMARKS])(void) =
{
    &benchmark_empty,
    &benchmark_add,
    &benchmark_fir,
};

static EVENT_ID bm_events[N_BENCHMARKS] =
{
    EVENT_BM_EMPTY,
    EVENT_BM_ADD,
    EVENT_BM_FIR,
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
