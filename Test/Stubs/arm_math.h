#ifndef ARM_MATH_H
#define ARM_MATH_H

#define ARM_MATH_CM7
#define IAR_ONLY_LOW_OPTIMIZATION_ENTER
#define LOW_OPTIMIZATION_ENTER
#define IAR_ONLY_LOW_OPTIMIZATION_EXIT
#define LOW_OPTIMIZATION_EXIT

#define multAcc_32x32_keep32_R(a, x, y) \
    a = (q31_t) (((((q63_t) a) << 32) + ((q63_t) x * y) + 0x80000000LL ) >> 32)

#define mult_32x32_keep32_R(a, x, y) \
    a = (q31_t) (((q63_t) x * y + 0x80000000LL ) >> 32)

#include "math.h"
#include "stdint.h"
#include "string.h"

typedef float float32_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

typedef struct
{
uint16_t numTaps;     /**< number of filter coefficients in the filter. */
float32_t *pState;    /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
float32_t *pCoeffs;   /**< points to the coefficient array. The array is of length numTaps. */
} arm_fir_instance_f32;

typedef struct
{
uint16_t numTaps;         /**< number of filter coefficients in the filter. */
q31_t *pState;            /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
q31_t *pCoeffs;           /**< points to the coefficient array. The array is of length numTaps. */
} arm_fir_instance_q31;

typedef struct
{
uint8_t numStages;         /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
float32_t *pState;         /**< points to the array of state coefficients.  The array is of length 2*numStages. */
float32_t *pCoeffs;        /**< points to the array of coefficients.  The array is of length 5*numStages. */
} arm_biquad_cascade_df2T_instance_f32;

typedef struct
{
uint32_t numStages;      /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
q31_t *pState;           /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
q31_t *pCoeffs;          /**< Points to the array of coefficients.  The array is of length 5*numStages. */
uint8_t postShift;       /**< Additional shift, in bits, applied to each output sample. */
} arm_biquad_casd_df1_inst_q31;

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

void arm_fir_fast_q31(
const arm_fir_instance_q31 * S,
q31_t * pSrc,
q31_t * pDst,
uint32_t blockSize);

void arm_fir_init_q31(
arm_fir_instance_q31 * S,
uint16_t numTaps,
q31_t * pCoeffs,
q31_t * pState,
uint32_t blockSize);

void arm_biquad_cascade_df2T_f32(
const arm_biquad_cascade_df2T_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize);

void arm_biquad_cascade_df2T_init_f32(
arm_biquad_cascade_df2T_instance_f32 * S,
uint8_t numStages,
float32_t * pCoeffs,
float32_t * pState);

void arm_biquad_cascade_df1_fast_q31(
const arm_biquad_casd_df1_inst_q31 * S,
q31_t * pSrc,
q31_t * pDst,
uint32_t blockSize);

void arm_biquad_cascade_df1_init_q31(
arm_biquad_casd_df1_inst_q31 * S,
uint8_t numStages,
q31_t * pCoeffs,
q31_t * pState,
int8_t postShift);

#endif /* ARM_MATH_H */

