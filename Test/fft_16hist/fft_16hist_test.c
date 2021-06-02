#include "fft_hist.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"

#include <stdio.h>

#define X_SIZE 64
#define Y_SIZE 16

void fft_16hist_test(void)
{
    int32_t x[X_SIZE];
    int16_t yl_ref[Y_SIZE];
    int16_t yr_ref[Y_SIZE];
    int16_t yl[Y_SIZE];
    int16_t yr[Y_SIZE];

    load_array(x, sizeof(*x), X_SIZE, "x.dat");
    load_array(yl_ref, sizeof(*yl_ref), Y_SIZE, "yl.dat");
    load_array(yr_ref, sizeof(*yr_ref), Y_SIZE, "yr.dat");

    fft_16hist(yl, yr, x);

    int16_t tol = 1;
    for (int32_t i = 0; i < Y_SIZE; ++i)
    {
        ASSERT_EQUAL_WITH_TOL(yl[i], yl_ref[i], tol);
        ASSERT_EQUAL_WITH_TOL(yr[i], yr_ref[i], tol);
    }
}

#define TESTS           \
    CUNIT_CI_TEST(fft_16hist_test)

CUNIT_CI_RUN("fft_16hist_test", TESTS);

