#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <assert.h>
#include <stdint.h>

// @formatter:off
typedef enum EVENT_ID
{
    EVENT_M7_FFT = 0,
    EVENT_M7_DSP,
    EVENT_M7_MDMA_CFG,
    EVENT_N_INFO,
    EVENT_BM_FIR_F32_CUSTOM = EVENT_N_INFO,
    EVENT_BM_FIR_Q31_CUSTOM,
    EVENT_BM_FIR_F32,
    EVENT_BM_FIR_I32,
    EVENT_BM_FIR_Q31,
    EVENT_BM_FIR_F32_CUSTOM_CACHE,
    EVENT_BM_FIR_Q31_CUSTOM_CACHE,
    EVENT_BM_FIR_F32_CACHE,
    EVENT_BM_FIR_I32_CACHE,
    EVENT_BM_FIR_Q31_CACHE,
    EVENT_BM_BIQUAD_F32_CUSTOM,
    EVENT_BM_BIQUAD_F64_CUSTOM,
    EVENT_BM_BIQUAD_Q31_CUSTOM,
    EVENT_BM_BIQUAD_F32,
    EVENT_BM_BIQUAD_I32,
    EVENT_BM_BIQUAD_Q31,
    EVENT_BM_BIQUAD_F32_CUSTOM_CACHE,
    EVENT_BM_BIQUAD_F64_CUSTOM_CACHE,
    EVENT_BM_BIQUAD_Q31_CUSTOM_CACHE,
    EVENT_BM_BIQUAD_F32_CACHE,
    EVENT_BM_BIQUAD_I32_CACHE,
    EVENT_BM_BIQUAD_Q31_CACHE,
    EVENT_BM_RFFT_F32,
    EVENT_BM_RFFT_Q31,
    EVENT_BM_CFFT_F32,
    EVENT_BM_CFFT_Q31,
    EVENT_BM_RFFT_F32_CACHE,
    EVENT_BM_RFFT_Q31_CACHE,
    EVENT_BM_CFFT_F32_CACHE,
    EVENT_BM_CFFT_Q31_CACHE,
    EVENT_N,
    EVENT_DBG = 0xFFFFFFFF
} EVENT_ID;
// @formatter:on
static_assert(sizeof(EVENT_ID) == 4, "EVENT_ID should be 4 bytes big as it is used for casting array of structs on uint8_t data buffer");

typedef struct event
{
    uint32_t val;
    EVENT_ID id;
} event;

void eq_m7_init(void);
int32_t eq_m7_get_size(void);
int32_t eq_m7_add_event(event e);
int32_t eq_m7_get_event(event *e);

#endif /* EVENT_QUEUE_H */

