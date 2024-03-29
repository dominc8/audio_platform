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

typedef struct biquad_f64_t
{
    double coeff[N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES];
    double state[MAX_BIQUAD_STAGES * 2];
    int32_t n_stage;
} biquad_f64_t;

typedef struct biquad_q31_t
{
    int32_t coeff[N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES];
    int32_t state[MAX_BIQUAD_STAGES * 4];
    int32_t n_stage;
} biquad_q31_t;

#if defined(CORE_CM4)
int32_t biquad_f32(biquad_f32_t *f, int32_t in); // __attribute__((section(".RAM_EXEC")));
int32_t biquad_f64(biquad_f64_t *f, int32_t in); // __attribute__((section(".RAM_EXEC")));
int32_t biquad_q31(biquad_q31_t *f, int32_t in); // __attribute__((section(".RAM_EXEC")));
#elif defined(CORE_CM7)
int32_t biquad_f32(biquad_f32_t *f, int32_t in); // __attribute__((section(".ITCM_RAM")));
int32_t biquad_f64(biquad_f64_t *f, int32_t in); // __attribute__((section(".ITCM_RAM")));
int32_t biquad_q31(biquad_q31_t *f, int32_t in); // __attribute__((section(".ITCM_RAM")));
#else
int32_t biquad_f32(biquad_f32_t *f, int32_t in);
int32_t biquad_f64(biquad_f64_t *f, int32_t in);
int32_t biquad_q31(biquad_q31_t *f, int32_t in);
#endif

#endif /* BIQUAD_H */

