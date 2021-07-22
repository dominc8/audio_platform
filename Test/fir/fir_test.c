#include "fir.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

#define N 128

void fir_test(void)
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

    int32_t tol = 2;
    for (int32_t i = 0; i < N; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(y_ref[i], y[i], tol);
    }
}

#define TESTS           \
    CUNIT_CI_TEST(fir_test)

CUNIT_CI_RUN("fir_test", TESTS);

