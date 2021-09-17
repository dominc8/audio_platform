#include "scheduler.h"
#include <stddef.h>

#define QUEUE_SIZE 8  /* Must be power of 2 */

typedef struct task_obj
{
    task t;
    void *arg;
} task_obj;

typedef struct scheduler_context
{
    int32_t size;
    int32_t n_used;
    int32_t begin;
    int32_t end;
    task_obj queue[QUEUE_SIZE];
} scheduler_context;

static scheduler_context scheduler_ctx;

void scheduler_init(void)
{
    scheduler_ctx.size = QUEUE_SIZE;
    scheduler_ctx.n_used = 0;
    scheduler_ctx.begin = 0;
    scheduler_ctx.end = 0;
    for (int32_t i = 0; i < QUEUE_SIZE; ++i)
    {
        scheduler_ctx.queue[i].t = NULL;
        scheduler_ctx.queue[i].arg = NULL;
    }
}

int32_t scheduler_get_queue_size()
{
    return QUEUE_SIZE;
}

int32_t scheduler_enqueue_task(task t, void *arg)
{
    if (scheduler_ctx.n_used == scheduler_ctx.size)
    {
        return -1;
    }

    int32_t end = scheduler_ctx.end;
    scheduler_ctx.queue[end].t = t;
    scheduler_ctx.queue[end].arg = arg;
    scheduler_ctx.end = (end + 1) & (QUEUE_SIZE - 1);
    scheduler_ctx.n_used++;

    return 0;
}

int32_t scheduler_dequeue_task(void)
{
    if (scheduler_ctx.n_used == 0)
    {
        return -1;
    }

    int32_t begin = scheduler_ctx.begin;
    int32_t ret_val = scheduler_ctx.queue[begin].t(scheduler_ctx.queue[begin].arg);
    scheduler_ctx.begin = (begin + 1) & (QUEUE_SIZE - 1);
    scheduler_ctx.n_used--;
    return ret_val;
}

