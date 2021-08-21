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

typedef struct fir_q31_t
{
    int32_t coeff[MAX_FIR_ORDER + 1];
    int32_t samples[MAX_FIR_ORDER + 1];
    int32_t order;
} fir_q31_t;

static inline int32_t smmlar(int32_t in1, int32_t in2, int32_t acc)
{
    int32_t out;
    __asm ("SMMLAR %[r_dest], %[r_in1], %[r_in2], %[r_acc]"
        : [r_dest] "=r" (out)
        : [r_in1] "r" (in1), [r_in2] "r" (in2), [r_acc] "r" (acc)
      );
    return out;
}

#ifdef CORE_CM4
int32_t fir_f32(fir_f32_t *f, int32_t in);// __attribute__((section(".RAM_EXEC")));
int32_t fir_q31(fir_q31_t *f, int32_t in);// __attribute__((section(".RAM_EXEC")));
#endif

#ifdef CORE_CM7
int32_t fir_f32(fir_f32_t *f, int32_t in);// __attribute__((section(".ITCM_RAM")));
int32_t fir_q31(fir_q31_t *f, int32_t in);// __attribute__((section(".ITCM_RAM")));
#endif

#endif /* FIR_H */

