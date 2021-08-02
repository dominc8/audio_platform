#ifndef BIQUAD_H
#define BIQUAD_H

#include <stdint.h>

#define MAX_BIQUAD_STAGES       19
#define BIQUAD_COEFF_MAX        2.F
#define BIQUAD_COEFF_MIN       -2.F
#define N_COEFF_IN_STAGE        5

typedef struct biquad_f32_t
{
    float coeff[N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES];
    float state[MAX_BIQUAD_STAGES * 2];
    int32_t n_stage;
} biquad_f32_t;

int32_t biquad_f32(biquad_f32_t *f, int32_t in);


#endif /* BIQUAD_H */

