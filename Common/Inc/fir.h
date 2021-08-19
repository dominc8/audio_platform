#ifndef FIR_H
#define FIR_H

#include <stdint.h>

#define MAX_FIR_ORDER       127
#define FIR_COEFF_MAX       1.F
#define FIR_COEFF_MIN       -1.F

typedef struct fir_f32_t
{
    float coeff[MAX_FIR_ORDER + 1];
    float samples[MAX_FIR_ORDER + 1];
    int32_t order;
} fir_f32_t;

int32_t fir_f32(fir_f32_t *f, int32_t in) __attribute__((section("ITCM_RAM")));

#endif /* FIR_H */

