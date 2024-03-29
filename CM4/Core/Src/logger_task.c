#include "logger_task.h"
#include "logger.h"
#include "scheduler.h"
#include "event_queue.h"
#include "shared_data.h"
#include <stddef.h>

static const char* event_to_str(EVENT_ID id);
static void print_fir_meas(volatile bm_meas *meas, int32_t n);
static void print_biquad_meas(volatile bm_meas *meas, int32_t n);
static void print_fft_meas(volatile bm_meas *meas, int32_t n);

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
        i = 0;
        const char *event_name = event_to_str(e.id);
        if (event_name == NULL)
        {
            switch (e.id)
            {
                case EVENT_BM_FIR_F32_CUSTOM:
                    logg(LOG_INF, "M7 FIR F32 CUSTOM:");
                    print_fir_meas(&fir_measurements_custom[0], 6);
                    break;
                case EVENT_BM_FIR_Q31_CUSTOM:
                    logg(LOG_INF, "M7 FIR Q31 CUSTOM:");
                    print_fir_meas(&fir_measurements_custom[0], 6);
                    break;
                case EVENT_BM_FIR_F32:
                    logg(LOG_INF, "M7 FIR F32:");
                    print_fir_meas(&fir_measurements_f32[0], 36);
                    break;
                case EVENT_BM_FIR_I32:
                    logg(LOG_INF, "M7 FIR I32:");
                    print_fir_meas(&fir_measurements_i32[0], 36);
                    break;
                case EVENT_BM_FIR_Q31:
                    logg(LOG_INF, "M7 FIR Q31:");
                    print_fir_meas(&fir_measurements_q31[0], 36);
                    break;
                case EVENT_BM_FIR_F32_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 FIR F32 CUSTOM CACHE:");
                    print_fir_meas(&fir_measurements_custom[0], 6);
                    break;
                case EVENT_BM_FIR_Q31_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 FIR Q31 CUSTOM CACHE:");
                    print_fir_meas(&fir_measurements_custom[0], 6);
                    break;
                case EVENT_BM_FIR_F32_CACHE:
                    logg(LOG_INF, "M7 FIR F32 CACHE:");
                    print_fir_meas(&fir_measurements_f32[0], 36);
                    break;
                case EVENT_BM_FIR_I32_CACHE:
                    logg(LOG_INF, "M7 FIR I32 CACHE:");
                    print_fir_meas(&fir_measurements_i32[0], 36);
                    break;
                case EVENT_BM_FIR_Q31_CACHE:
                    logg(LOG_INF, "M7 FIR Q31 CACHE:");
                    print_fir_meas(&fir_measurements_q31[0], 36);
                    break;
                case EVENT_BM_BIQUAD_F32_CUSTOM:
                    logg(LOG_INF, "M7 BIQUAD F32 CUSTOM:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_F64_CUSTOM:
                    logg(LOG_INF, "M7 BIQUAD F64 CUSTOM:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_Q31_CUSTOM:
                    logg(LOG_INF, "M7 BIQUAD Q31 CUSTOM:");
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
                case EVENT_BM_BIQUAD_F32_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 BIQUAD F32 CUSTOM CACHE:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_F64_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 BIQUAD F64 CUSTOM CACHE:");
                    print_biquad_meas(&biquad_measurements_custom[0], 5);
                    break;
                case EVENT_BM_BIQUAD_Q31_CUSTOM_CACHE:
                    logg(LOG_INF, "M7 BIQUAD Q31 CUSTOM CACHE:");
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
                case EVENT_BM_RFFT_F32:
                    logg(LOG_INF, "M7 RFFT F32:");
                    print_fft_meas(&rfft_measurements_f32[0], 5);
                    break;
                case EVENT_BM_RFFT_Q31:
                    logg(LOG_INF, "M7 RFFT Q31:");
                    print_fft_meas(&rfft_measurements_q31[0], 5);
                    break;
                case EVENT_BM_CFFT_F32:
                    logg(LOG_INF, "M7 CFFT F32:");
                    print_fft_meas(&cfft_measurements_f32[0], 5);
                    break;
                case EVENT_BM_CFFT_Q31:
                    logg(LOG_INF, "M7 CFFT Q31:");
                    print_fft_meas(&cfft_measurements_q31[0], 5);
                    break;
                case EVENT_BM_RFFT_F32_CACHE:
                    logg(LOG_INF, "M7 RFFT F32 CACHE:");
                    print_fft_meas(&rfft_measurements_f32[0], 5);
                    break;
                case EVENT_BM_RFFT_Q31_CACHE:
                    logg(LOG_INF, "M7 RFFT Q31 CACHE:");
                    print_fft_meas(&rfft_measurements_q31[0], 5);
                    break;
                case EVENT_BM_CFFT_F32_CACHE:
                    logg(LOG_INF, "M7 CFFT F32 CACHE:");
                    print_fft_meas(&cfft_measurements_f32[0], 5);
                    break;
                case EVENT_BM_CFFT_Q31_CACHE:
                    logg(LOG_INF, "M7 CFFT Q31 CACHE:");
                    print_fft_meas(&cfft_measurements_q31[0], 5);
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

    if (++i > 1000000)
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

static void print_fir_meas(volatile bm_meas *meas, int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        logg(LOG_INF, "n_taps=%u, block_size=%u, cycles=%u", meas[i].n_taps, meas[i].block_size,
                meas[i].cycles);
    }
}

static void print_biquad_meas(volatile bm_meas *meas, int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        logg(LOG_INF, "n_stages=%u, block_size=%u, cycles=%u", meas[i].n_taps, meas[i].block_size,
                meas[i].cycles);
    }
}

static void print_fft_meas(volatile bm_meas *meas, int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        logg(LOG_INF, "fft_size=%u, cycles=%u", meas[i].block_size, meas[i].cycles);
    }
}

