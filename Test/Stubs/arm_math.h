#ifndef ARM_MATH_H
#define ARM_MATH_H

#include "math.h"
#include "stdint.h"
#include "string.h"

typedef float float32_t;

typedef struct
{
uint16_t numTaps;     /**< number of filter coefficients in the filter. */
float32_t *pState;    /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
float32_t *pCoeffs;   /**< points to the coefficient array. The array is of length numTaps. */
} arm_fir_instance_f32;

void arm_fir_f32(
const arm_fir_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize);

void arm_fir_init_f32(
arm_fir_instance_f32 * S,
uint16_t numTaps,
float32_t * pCoeffs,
float32_t * pState,
uint32_t blockSize);

#endif /* ARM_MATH_H */

