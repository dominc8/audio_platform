#include "benchmark.h"
#include "fir.h"
#include "perf_meas.h"
// #include "arm_math.h"
#include "logger.h"

#define N_BENCHMARKS    1
#define TAPS_LEN        5
#define BLOCK_SIZE_LEN  6

static int32_t data_in[256] __attribute__ ((aligned (32)));
static int32_t data_out[256] __attribute__ ((aligned (32)));
static const int32_t taps_arr[TAPS_LEN] = { 5, 10, 20, 50, 100 };
static const int32_t block_size_arr[BLOCK_SIZE_LEN] = { 8, 16, 32, 64, 128, 256 };

static fir_f32_t fir_inst;

static void benchmark_fir_custom(void)
{
    logg(LOG_INF, "M4 FIR CUSTOM:");
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        fir_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            data_out[0] = fir_f32(&fir_inst, data_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        logg(LOG_INF, "n_taps=%u, cycles=%u", n_taps, acc);
    }
}



static void (*benchmarks[N_BENCHMARKS])(void) =
{
    &benchmark_fir_custom,
};

void benchmark(void)
{
    logg(LOG_INF, "Running M4 benchmarks ...");
    for (int32_t bm = 0; bm < N_BENCHMARKS; ++bm)
    {
        benchmarks[bm]();
    }
}

