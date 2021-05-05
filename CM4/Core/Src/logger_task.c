#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include <stddef.h>

int32_t logger_task_init(void)
{
    logger_init(115200);
    return scheduler_enqueue_task(&logger_task, NULL);
}

int32_t logger_task(void *arg)
{
    /* TODO: Handle log messages from M7 */
    static int32_t i = 0;
    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "Logger task");
    }
    return scheduler_enqueue_task(&logger_task, NULL);
}

