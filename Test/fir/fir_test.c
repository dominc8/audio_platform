#include "fir.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"
#include "arm_math.h"

#include <stdio.h>
#include <string.h>

#define N 128

void fir_f32_test(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    fir_f32_t f;
    memset(&f.samples[0], 0, sizeof(f.samples));

    load_array(x, sizeof(*x), N, "x.dat");
    load_array(y_ref, sizeof(*y_ref), N, "y.dat");
    n_coeff = load_array(&f.coeff[0], sizeof(*(f.coeff)), MAX_FIR_ORDER + 1, "coeff.dat");

    f.order = n_coeff - 1;

    for (int32_t i = 0; i < N; ++i)
    {
        y[i] = fir_f32(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

void fir_f32_test_arm(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    float arm_coeff[MAX_FIR_ORDER + 1];
    float arm_state[MAX_FIR_ORDER + 1];
    fir_f32_t f;
    arm_fir_instance_f32 arm_f;

    load_array(x, sizeof(*x), N, "x.dat");

    memset(&f.samples[0], 0, sizeof(f.samples));
    n_coeff = load_array(&f.coeff[0], sizeof(*(f.coeff)), MAX_FIR_ORDER + 1, "coeff.dat");
    f.order = n_coeff - 1;

    for (int32_t i = 0; i < n_coeff; ++i)
    {
        arm_coeff[i] = f.coeff[n_coeff - 1 - i];
    }
    arm_fir_init_f32(&arm_f, n_coeff, &arm_coeff[0], &arm_state[0], 1);

    for (int32_t i = 0; i < N; ++i)
    {
        float src = (float) x[i];
        float dst;
        arm_fir_f32(&arm_f, &src, &dst, 1);
        y_ref[i] = (int32_t) dst;
        y[i] = fir_f32(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

void fir_q31_test_arm(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    float temp_coeff[MAX_FIR_ORDER + 1];
    int32_t arm_coeff[MAX_FIR_ORDER + 1];
    int32_t arm_state[MAX_FIR_ORDER + 1];
    fir_q31_t f;
    arm_fir_instance_q31 arm_f;

    load_array(x, sizeof(*x), N, "x.dat");

    memset(&f.samples[0], 0, sizeof(f.samples));
    n_coeff = load_array(&temp_coeff[0], sizeof(*(temp_coeff)), MAX_FIR_ORDER + 1, "coeff.dat");
    f.order = n_coeff - 1;

    for (int32_t i = 0; i < n_coeff; ++i)
    {
        int32_t q_coeff = temp_coeff[i] * (1 << 29);
        f.coeff[i] = q_coeff;
        arm_coeff[n_coeff - 1 - i] = q_coeff;
    }
    arm_fir_init_q31(&arm_f, n_coeff, &arm_coeff[0], &arm_state[0], 1);

    for (int32_t i = 0; i < N; ++i)
    {
        int32_t src = x[i];
        int32_t dst;
        arm_fir_fast_q31(&arm_f, &src, &dst, 1);
        y_ref[i] = dst;
        y[i] = fir_q31(&f, x[i]);
    }

    int32_t tol = 1;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

#define TESTS           \
    CUNIT_CI_TEST(fir_f32_test), \
    CUNIT_CI_TEST(fir_f32_test_arm), \
    CUNIT_CI_TEST(fir_q31_test_arm)

CUNIT_CI_RUN("fir_test", TESTS);

