#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include "event_queue.h"
#include <stddef.h>

static const char* event_to_str(EVENT_ID id);

int32_t logger_task_init(void)
{
    logger_init(115200);
    return scheduler_enqueue_task(&logger_task, NULL);
}

int32_t logger_task(void *arg)
{
    static int32_t i = 0;
    event e;
    int32_t get_event_status;
    (void) arg;
    get_event_status = eq_m7_get_event(&e);

    if (get_event_status == 0)
    {
        const char *event_name = event_to_str(e.id);
        if (event_name == NULL)
        {
            logg(LOG_INF, "M7: (%d, %u)", e.id, e.val);
        }
        else
        {
            logg(LOG_INF, "M7 %s: %u", event_name, e.val);
        }
    }

    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "Logger task");
    }
    return scheduler_enqueue_task(&logger_task, NULL);
}

static const char* event_to_str(EVENT_ID id)
{
    static const char *event_names[EVENT_N] =
    { "FFT", "DSP", "MDMA_CFG", "BM_EMPTY", "BM_ADD" };

    if ((id >= 0) && (id < EVENT_N))
    {
        return event_names[id];
    }
    else
    {
        return NULL;
    }
}

