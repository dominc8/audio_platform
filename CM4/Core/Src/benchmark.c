#include "benchmark.h"
#include "perf_meas.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "logger.h"
#include "fir.h"
#include "biquad.h"

#define N_FIR_BM            5
#define N_BIQUAD_BM         5
#define N_FFT_BM            4
#define TAPS_LEN            6
#define STAGES_LEN          5
#define BLOCK_SIZE_LEN      6
#define FFT_SIZE_LEN        5

static const int32_t taps_arr[TAPS_LEN] =
{ 5, 10, 20, 50, 100, 128 };
static const int32_t stages_arr[STAGES_LEN] =
{ 1, 4, 8, 12, 16 };
static const int32_t block_size_arr[BLOCK_SIZE_LEN] =
{ 8, 16, 32, 64, 128, 256 };
static const int32_t fft_size_arr[FFT_SIZE_LEN] =
{ 64, 128, 256, 512, 1024 };

static int32_t data_in[2048] __attribute__ ((aligned (32)));
static int32_t data_out[2048] __attribute__ ((aligned (32)));
static int32_t coeff[128] __attribute__ ((aligned (32)));
static int32_t state[128 + 256 - 1] __attribute__ ((aligned (32)));

static fir_f32_t fir_f32_inst;
static fir_q31_t fir_q31_inst;
static biquad_f32_t biquad_f32_inst;
static biquad_q31_t biquad_q31_inst;

static void benchmark_fir_f32_custom(void)
{
    logg(LOG_INF, "M4 FIR F32 CUSTOM:");
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        fir_f32_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            data_out[0] = fir_f32(&fir_f32_inst, data_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", n_taps, 1, acc);
    }
}

static void benchmark_fir_q31_custom(void)
{
    logg(LOG_INF, "M4 FIR Q31 CUSTOM:");
    for (int32_t i = 0; i < TAPS_LEN; ++i)
    {
        int32_t n_taps = taps_arr[i];
        fir_q31_inst.order = n_taps - 1;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            data_out[0] = fir_q31(&fir_q31_inst, data_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", n_taps, 1, acc);
    }
}

static void benchmark_fir_f32(void)
{
    arm_fir_instance_f32 fir_inst;

    logg(LOG_INF, "M4 FIR F32:");

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &coeff[0], (float*) &state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32(&fir_inst, (float*) &data_in[0], (float*) &data_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", n_taps, block_size, acc);
        }
    }
}

static void benchmark_fir_i32(void)
{
    arm_fir_instance_f32 fir_inst;

    logg(LOG_INF, "M4 FIR I32:");

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_f32(&fir_inst, n_taps, (float*) &coeff[0], (float*) &state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_f32_int(&fir_inst, &data_in[0], &data_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", n_taps, block_size, acc);
        }
    }
}

static void benchmark_fir_q31(void)
{
    arm_fir_instance_q31 fir_inst;

    logg(LOG_INF, "M4 FIR Q31:");

    for (int32_t tap_idx = 0; tap_idx < TAPS_LEN; ++tap_idx)
    {
        int32_t n_taps = taps_arr[tap_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_fir_init_q31(&fir_inst, n_taps, &coeff[0], &state[0], block_size);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_fir_fast_q31(&fir_inst, &data_in[0], &data_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", n_taps, block_size, acc);
        }
    }
}

static void benchmark_biquad_f32_custom(void)
{
    logg(LOG_INF, "M4 BIQUAD F32 CUSTOM:");
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        biquad_f32_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            data_out[0] = biquad_f32(&biquad_f32_inst, data_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", n_stages, 1, acc);
    }
}

static void benchmark_biquad_q31_custom(void)
{
    logg(LOG_INF, "M4 BIQUAD Q31 CUSTOM:");
    for (int32_t i = 0; i < STAGES_LEN; ++i)
    {
        int32_t n_stages = stages_arr[i];
        biquad_q31_inst.n_stage = n_stages;

        uint32_t start, stop;
        uint32_t acc = 0;
        int32_t n = 1 << 17;

        while (n-- > 0)
        {
            start = GET_CCNT();
            data_out[0] = biquad_q31(&biquad_q31_inst, data_in[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 17;

        logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", n_stages, 1, acc);
    }
}

static void benchmark_biquad_f32(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;

    logg(LOG_INF, "M4 BIQUAD F32:");

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &coeff[0],
                    (float*) &state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df2T_f32(&biquad_inst, (float*) &data_in[0],
                        (float*) &data_out[0], block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", n_stages, block_size, acc);
        }
    }
}

static void benchmark_biquad_i32(void)
{
    arm_biquad_cascade_df2T_instance_f32 biquad_inst;

    logg(LOG_INF, "M4 BIQUAD I32:");

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df2T_init_f32(&biquad_inst, n_stages, (float*) &coeff[0],
                    (float*) &state[0]);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df2T_f32_int(&biquad_inst, &data_in[0], &data_out[0],
                        block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", n_stages, block_size, acc);
        }
    }
}

static void benchmark_biquad_q31(void)
{
    arm_biquad_casd_df1_inst_q31 biquad_inst;

    logg(LOG_INF, "M4 BIQUAD Q31:");

    for (int32_t stage_idx = 0; stage_idx < STAGES_LEN; ++stage_idx)
    {
        int32_t n_stages = stages_arr[stage_idx];
        for (int32_t b_size_idx = 0; b_size_idx < BLOCK_SIZE_LEN; ++b_size_idx)
        {
            int32_t block_size = block_size_arr[b_size_idx];
            arm_biquad_cascade_df1_init_q31(&biquad_inst, n_stages, &coeff[0], &state[0], 1);

            uint32_t start, stop;
            uint32_t acc = 0;
            int32_t n = 1 << 13;

            while (n-- > 0)
            {
                start = GET_CCNT();
                arm_biquad_cascade_df1_fast_q31(&biquad_inst, &data_in[0], &data_out[0],
                        block_size);
                stop = GET_CCNT();
                acc += DIFF_CCNT(start, stop);
            }
            acc = acc >> 13;
            logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", n_stages, block_size, acc);
        }
    }
}

static void benchmark_rfft_f32(void)
{
    arm_rfft_fast_instance_f32 arm_rfft;

    logg(LOG_INF, "M4 RFFT F32:");

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
            arm_rfft_fast_f32(&arm_rfft, (float*) &data_in[0], (float*) &data_out[0], 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        logg(LOG_INF, "fft_size=%u, cycles=%u", fft_size, acc);
    }
}

static void benchmark_rfft_q31(void)
{
    arm_rfft_instance_q31 arm_rfft;

    logg(LOG_INF, "M4 RFFT Q31:");

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
            arm_rfft_q31(&arm_rfft, &data_in[0], &data_out[0]);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        logg(LOG_INF, "fft_size=%u, cycles=%u", fft_size, acc);
    }
}

static void benchmark_cfft_f32(void)
{
    const arm_cfft_instance_f32 *arm_cfft;

    logg(LOG_INF, "M4 CFFT F32:");

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
            default:
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
            arm_cfft_f32(arm_cfft, (float*) &data_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        logg(LOG_INF, "fft_size=%u, cycles=%u", fft_size, acc);
    }
}

static void benchmark_cfft_q31(void)
{
    const arm_cfft_instance_q31 *arm_cfft;

    logg(LOG_INF, "M4 CFFT Q31:");

    for (int32_t i = 0; i < FFT_SIZE_LEN; ++i)
    {
        int32_t fft_size = fft_size_arr[i];
        switch (fft_size)
        {
            case 64:
            default:
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
            arm_cfft_q31(arm_cfft, &data_in[0], 0, 0);
            stop = GET_CCNT();
            acc += DIFF_CCNT(start, stop);
        }
        acc = acc >> 12;

        logg(LOG_INF, "fft_size=%u, cycles=%u", fft_size, acc);
    }
}

static void (*fir_benchmarks[N_FIR_BM])(void) =
{
    &benchmark_fir_f32_custom,
    &benchmark_fir_q31_custom,
    &benchmark_fir_f32,
    &benchmark_fir_i32,
    &benchmark_fir_q31,
};

static void (*biquad_benchmarks[N_BIQUAD_BM])(void) =
{
    &benchmark_biquad_f32_custom,
    &benchmark_biquad_q31_custom,
    &benchmark_biquad_f32,
    &benchmark_biquad_i32,
    &benchmark_biquad_q31,
};

static void (*fft_benchmarks[N_FFT_BM])(void) =
{
    &benchmark_rfft_f32,
    &benchmark_rfft_q31,
    &benchmark_cfft_f32,
    &benchmark_cfft_q31,
};

void benchmark(void)
{
    logg(LOG_INF, "Running M4 benchmarks ...");
    for (int32_t bm = 0; bm < N_FIR_BM; ++bm)
    {
        fir_benchmarks[bm]();
    }
    for (int32_t bm = 0; bm < N_BIQUAD_BM; ++bm)
    {
        biquad_benchmarks[bm]();
    }
    for (int32_t bm = 0; bm < N_FFT_BM; ++bm)
    {
        fft_benchmarks[bm]();
    }
}

