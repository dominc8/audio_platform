#include "biquad.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"
#include "arm_math.h"

#include <stdio.h>
#include <string.h>

#define N 128

void biquad_f32_test(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    biquad_f32_t f;
    memset(&f.state[0], 0, sizeof(f.state));

    load_array(x, sizeof(*x), N, "x.dat");
    load_array(y_ref, sizeof(*y_ref), N, "y.dat");
    n_coeff = load_array(&f.coeff[0], sizeof(*(f.coeff)), MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE,
            "coeff.dat");

    f.n_stage = n_coeff / N_COEFF_IN_STAGE;

    for (int32_t i = 0; i < N; ++i)
    {
        y[i] = biquad_f32(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

void biquad_f32_test_arm(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    float arm_coeff[MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE];
    float arm_state[MAX_BIQUAD_STAGES * 2];
    biquad_f32_t f;
    arm_biquad_cascade_df2T_instance_f32 arm_f;

    load_array(x, sizeof(*x), N, "x.dat");

    memset(&f.state[0], 0, sizeof(f.state));
    n_coeff = load_array(&f.coeff[0], sizeof(*(f.coeff)), MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE,
            "coeff.dat");
    f.n_stage = n_coeff / N_COEFF_IN_STAGE;

    memcpy(&arm_coeff[0], &f.coeff[0], sizeof(arm_coeff));
    arm_biquad_cascade_df2T_init_f32(&arm_f, f.n_stage, &arm_coeff[0], &arm_state[0]);

    for (int32_t i = 0; i < N; ++i)
    {
        float src = (float) x[i];
        float dst;
        arm_biquad_cascade_df2T_f32(&arm_f, &src, &dst, 1);
        y_ref[i] = (int32_t) dst;
        y[i] = biquad_f32(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

void biquad_q31_test_arm(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    float temp_coeff[MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE];
    int32_t arm_coeff[MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE];
    int32_t arm_state[MAX_BIQUAD_STAGES * 4];
    biquad_q31_t f;
    arm_biquad_casd_df1_inst_q31 arm_f;

    load_array(x, sizeof(*x), N, "x.dat");

    memset(&f.state[0], 0, sizeof(f.state));
    n_coeff = load_array(&temp_coeff[0], sizeof(*(temp_coeff)),
            MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE, "coeff.dat");
#if 1
    f.n_stage = n_coeff / N_COEFF_IN_STAGE;

    for (int32_t i = 0; i < n_coeff; ++i)
    {
        int32_t q_coeff = temp_coeff[i] * (1 << 29);
        f.coeff[i] = q_coeff;
        arm_coeff[i] = q_coeff;
    }
#else
    n_coeff = 3 * N_COEFF_IN_STAGE;
    f.n_stage = 3;

    for (int32_t i = 0; i < n_coeff; ++i)
    {
        int32_t q_coeff = temp_coeff[i] * (1 << 30);
        f.coeff[i] = q_coeff;
        arm_coeff[i] = q_coeff;
    }
#endif
    arm_biquad_cascade_df1_init_q31(&arm_f, f.n_stage, &arm_coeff[0], &arm_state[0], 0);

    for (int32_t i = 0; i < N; ++i)
    {
        int32_t src = x[i];
        int32_t dst;
        arm_biquad_cascade_df1_fast_q31(&arm_f, &src, &dst, 1);
        y_ref[i] = dst;
        y[i] = biquad_q31(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }

}

#define TESTS           \
    CUNIT_CI_TEST(biquad_f32_test), \
    CUNIT_CI_TEST(biquad_f32_test_arm), \
    CUNIT_CI_TEST(biquad_q31_test_arm)

CUNIT_CI_RUN("biquad_test", TESTS);

