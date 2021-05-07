#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <assert.h>
#include <stdint.h>

typedef enum EVENT_ID
{
    EVENT_M7_TRACE = 0, EVENT_N
} EVENT_ID;

static_assert(sizeof(EVENT_ID) == 4, "EVENT_ID should be 4 bytes big as it is used for casting array of structs on uint8_t data buffer");

typedef struct event
{
    EVENT_ID id;
    uint32_t val;
} event;

void eq_m7_init(void);
int32_t eq_m7_add_event(event e);
int32_t eq_m7_get_event(event *e);


#endif /* EVENT_QUEUE_H */

