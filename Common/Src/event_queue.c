#include "event_queue.h"
#include "shared_data.h"
#ifdef CORE_CM4
#include "logger.h"
#endif
#ifdef CORE_CM7
#include "stm32h747xx.h"
#endif

/* 8 is size reserved for both counters */
#define EQ_M7_HEADER_SIZE       8
#define EQ_M7_SIZE ((M7_EQ_BUF_SIZE - EQ_M7_HEADER_SIZE)/sizeof(event))

typedef volatile struct eq_m7
{
    event events[EQ_M7_SIZE];
} eq_m7;

static eq_m7 *eq_m7_ptr;
static int32_t *head_ptr;
static int32_t *tail_ptr;

static inline int32_t is_queue_full(int32_t head, int32_t tail)
{
    if (tail == 0)
    {
        tail = EQ_M7_SIZE;
    }
    return (head + 1 == tail);
}

static inline int32_t is_queue_empty(int32_t head, int32_t tail)
{
    return head == tail;
}


void eq_m7_init(void)
{
    eq_m7_ptr = (void*)&m7_eq_buf[EQ_M7_HEADER_SIZE];
    head_ptr = (void*)&m7_eq_buf[0];
    *head_ptr = 0;
    tail_ptr = (void*)&m7_eq_buf[4];
    *tail_ptr = 0;
}

int32_t eq_m7_get_size(void)
{
    return EQ_M7_SIZE - 1;
}

#ifdef CORE_CM7
int32_t eq_m7_add_event(event e)
{
    int32_t ret_val = 0;
    int32_t eq_m7_head = *head_ptr;
    int32_t eq_m7_tail = *tail_ptr;
    if (is_queue_full(eq_m7_head, eq_m7_tail))
    {
        ret_val = -1;
    }
    else
    {
        eq_m7_ptr->events[eq_m7_head] = e;
        if (++eq_m7_head == EQ_M7_SIZE)
        {
            eq_m7_head = 0;
        }
        *head_ptr = eq_m7_head;
    }
    return ret_val;
}
#endif

#ifdef CORE_CM4
int32_t eq_m7_get_event(event *e)
{
    int32_t ret_val = 0;
    int32_t eq_m7_head = *head_ptr;
    int32_t eq_m7_tail = *tail_ptr;
    if (is_queue_empty(eq_m7_head, eq_m7_tail))
    {
        ret_val = -1;
    }
    else
    {
        *e = eq_m7_ptr->events[eq_m7_tail];
        if (++eq_m7_tail == EQ_M7_SIZE)
        {
            eq_m7_tail = 0;
        }
        *tail_ptr = eq_m7_tail;
    }
    return ret_val;
}
#endif

