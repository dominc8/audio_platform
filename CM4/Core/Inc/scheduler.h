#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef int32_t (*task)(void *arg);

void scheduler_init(void);
int32_t scheduler_get_queue_size(void);
int32_t scheduler_enqueue_task(task t, void *arg);
int32_t scheduler_dequeue_task(void);



#endif /* SCHEDULER_H */

