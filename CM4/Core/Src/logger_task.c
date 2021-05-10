#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include "event_queue.h"
#include <stddef.h>

int32_t logger_task_init(void)
{
    logger_init(115200);
    return scheduler_enqueue_task(&logger_task, NULL);
}

int32_t logger_task(void *arg)
{
    /* TODO: Handle log messages from M7 */
    event e;
    int32_t get_event_status;
    get_event_status = eq_m7_get_event(&e);

    if (get_event_status == 0)
    {
        logg(LOG_INF, "Got event (%d, %u)", e.id, e.val);
    }

    static int32_t i = 0;
    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "Logger task");
    }
    return scheduler_enqueue_task(&logger_task, NULL);
}

