#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define ABS_DIFF(x, y) ((x) > (y) ? ((x) - (y)) : ((y) - (x)))

#define ASSERT_EQUAL_WITH_TOL(a, b, tol) CU_ASSERT_TRUE(ABS_DIFF(a, b) <= tol)

int32_t load_array(void *arr, int32_t el_size, int32_t n, const char* filename);
void print_farray(float *arr, int32_t size);
void print_i16array(int16_t *arr, int32_t size);
void print_i32array(int32_t *arr, int32_t size);

#endif /* UTILS_H */

