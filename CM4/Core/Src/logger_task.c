#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include "event_queue.h"
#include "shared_data.h"
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
        if (EVENT_BM_FIR == e.id)
        {
            logg(LOG_INF, "M7 FIR: ");
            for (int32_t i = 0; i < 30; ++i)
            {
                volatile fir_meas *meas = &fir_measurements[i];
                logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", meas->n_taps, meas->block_size, meas->cycles);
            }
        }
        else
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
    { "FFT", "DSP", "MDMA_CFG", "BM_EMPTY", "BM_ADD", "BM_FIR" };

    if ((id >= 0) && (id < EVENT_N))
    {
        return event_names[id];
    }
    else
    {
        return NULL;
    }
}

