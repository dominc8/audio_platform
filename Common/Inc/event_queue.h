#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <assert.h>
#include <stdint.h>

typedef enum EVENT_ID
{
    EVENT_M7_FFT = 0, EVENT_M7_DSP, EVENT_M7_MDMA_CFG,
    EVENT_BM_EMPTY, EVENT_BM_ADD, EVENT_BM_FIR, EVENT_N, EVENT_DBG = 0xFFFFFFFF
} EVENT_ID;

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

