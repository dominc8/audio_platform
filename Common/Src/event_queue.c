#include <stdio.h>
#include "event_queue.h"
#include "shared_data.h"

static int32_t eq_m7_head;
static int32_t eq_m7_tail;

/* 8 is size reserved for both counters */
#define EQ_M7_HEADER_SIZE       8
#define EQ_M7_SIZE ((M7_EQ_BUF_SIZE - EQ_M7_HEADER_SIZE)/sizeof(event))

typedef volatile struct eq_m7
{
    event events[EQ_M7_SIZE];
} eq_m7;

static eq_m7 *eq_m7_ptr;

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
    eq_m7_head = 0;
    eq_m7_tail = 0;
}

int32_t eq_m7_get_size(void)
{
    return EQ_M7_SIZE - 1;
}

int32_t eq_m7_add_event(event e)
{
    int32_t ret_val = 0;
    printf("add_event: head(%d), tail(%d), ", eq_m7_head, eq_m7_tail);
    if (is_queue_full(eq_m7_head, eq_m7_tail))
    {
        printf("queue is full\n");
        ret_val = -1;
    }
    else
    {
        printf("queue is not full(adding event), ");
        eq_m7_ptr->events[eq_m7_head] = e;
        if (++eq_m7_head == EQ_M7_SIZE)
        {
            eq_m7_head = 0;
        }
        printf("new head: %d\n", eq_m7_head);
    }
    return ret_val;
}

int32_t eq_m7_get_event(event *e)
{
    int32_t ret_val = 0;
    printf("get_event: head(%d), tail(%d), ", eq_m7_head, eq_m7_tail);
    if (is_queue_empty(eq_m7_head, eq_m7_tail))
    {
        printf("queue is empty\n");
        ret_val = -1;
    }
    else
    {
        printf("queue is not empty(getting event), ");
        *e = eq_m7_ptr->events[eq_m7_tail];
        if (++eq_m7_tail == EQ_M7_SIZE)
        {
            eq_m7_tail = 0;
        }
        printf("new tail: %d\n", eq_m7_tail);
    }
    return ret_val;
}

