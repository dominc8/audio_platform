#include "biquad.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"
#include "arm_math.h"

#include <stdio.h>
#include <string.h>

#define N 128

void biquad_test(void)
{
    int32_t n_coeff;
    int32_t x[N];
    int32_t y_ref[N];
    int32_t y[N];
    biquad_f32_t f;
    memset(&f.state[0], 0, sizeof(f.state));

    load_array(x, sizeof(*x), N, "x.dat");
    load_array(y_ref, sizeof(*y_ref), N, "y.dat");
    n_coeff = load_array(&f.coeff[0], sizeof(*(f.coeff)), MAX_BIQUAD_STAGES * N_COEFF_IN_STAGE, "coeff.dat");

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

#if 0
void biquad_test_arm(void)
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
#endif


#define TESTS           \
    CUNIT_CI_TEST(biquad_test)

CUNIT_CI_RUN("biquad_test", TESTS);

