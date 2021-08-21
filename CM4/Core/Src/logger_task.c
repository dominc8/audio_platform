#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include "event_queue.h"
#include "shared_data.h"
#include <stddef.h>

static const char* event_to_str(EVENT_ID id);
static void print_fir_meas(volatile bm_meas* meas, int32_t n);
static void print_biquad_meas(volatile bm_meas* meas, int32_t n);

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
            switch (e.id)
            {
                case EVENT_BM_FIR_CUSTOM:
                    logg(LOG_INF, "M7 FIR CUSTOM:");
                    print_fir_meas(&fir_measurements_custom[0], 5);
                    break;
                case EVENT_BM_FIR_F32:
                    logg(LOG_INF, "M7 FIR F32:");
                    print_fir_meas(&fir_measurements_f32[0], 30);
                    break;
                case EVENT_BM_FIR_I32:
                    logg(LOG_INF, "M7 FIR I32:");
                    print_fir_meas(&fir_measurements_i32[0], 30);
                    break;
                case EVENT_BM_FIR_Q31:
                    logg(LOG_INF, "M7 FIR Q31:");
                    print_fir_meas(&fir_measurements_q31[0], 30);
                    break;
                case EVENT_BM_FIR_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 FIR CUSTOM CACHE:");
                    print_fir_meas(&fir_measurements_custom[0], 5);
                    break;
                case EVENT_BM_FIR_F32_CACHE:
                    logg(LOG_INF, "M7 FIR F32 CACHE:");
                    print_fir_meas(&fir_measurements_f32[0], 30);
                    break;
                case EVENT_BM_FIR_I32_CACHE:
                    logg(LOG_INF, "M7 FIR I32 CACHE:");
                    print_fir_meas(&fir_measurements_i32[0], 30);
                    break;
                case EVENT_BM_FIR_Q31_CACHE:
                    logg(LOG_INF, "M7 FIR Q31 CACHE:");
                    print_fir_meas(&fir_measurements_q31[0], 30);
                    break;
                case EVENT_BM_FIR_CUSTOM_CACHE_DATA_ONLY:
                    logg(LOG_INF, "M7 FIR CUSTOM CACHE DATA ONLY:");
                    print_fir_meas(&fir_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_CUSTOM:
                    logg(LOG_INF, "M7 BIQUAD CUSTOM:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_F32:
                    logg(LOG_INF, "M7 BIQUAD F32:");
                    print_biquad_meas(&biquad_measurements_f32[0], 30);
                    break;
                case EVENT_BM_BIQUAD_I32:
                    logg(LOG_INF, "M7 BIQUAD I32:");
                    print_biquad_meas(&biquad_measurements_i32[0], 30);
                    break;
                case EVENT_BM_BIQUAD_Q31:
                    logg(LOG_INF, "M7 BIQUAD Q31:");
                    print_biquad_meas(&biquad_measurements_q31[0], 30);
                    break;
                case EVENT_BM_BIQUAD_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 BIQUAD CUSTOM CACHE:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_F32_CACHE:
                    logg(LOG_INF, "M7 BIQUAD F32 CACHE:");
                    print_biquad_meas(&biquad_measurements_f32[0], 30);
                    break;
                case EVENT_BM_BIQUAD_I32_CACHE:
                    logg(LOG_INF, "M7 BIQUAD I32 CACHE:");
                    print_biquad_meas(&biquad_measurements_i32[0], 30);
                    break;
                case EVENT_BM_BIQUAD_Q31_CACHE:
                    logg(LOG_INF, "M7 BIQUAD Q31 CACHE:");
                    print_biquad_meas(&biquad_measurements_q31[0], 30);
                    break;
                case EVENT_BM_BIQUAD_CUSTOM_CACHE_DATA_ONLY:
                    logg(LOG_INF, "M7 BIQUAD CUSTOM CACHE DATA ONLY:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                default:
                    logg(LOG_INF, "M7: (%d, %u)", e.id, e.val);
                    break;
            }
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
    static const char *event_names[EVENT_N_INFO] =
    { "FFT", "DSP", "MDMA_CFG" };

    if ((id >= 0) && (id < EVENT_N_INFO))
    {
        return event_names[id];
    }
    else
    {
        return NULL;
    }
}

static void print_fir_meas(volatile bm_meas* meas, int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", meas[i].n_taps, meas[i].block_size, meas[i].cycles);
    }
}

static void print_biquad_meas(volatile bm_meas* meas, int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", meas[i].n_taps, meas[i].block_size, meas[i].cycles);
    }
}
