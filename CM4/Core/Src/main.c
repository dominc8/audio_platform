#include "intercore_comm.h"
#include "perf_meas.h"
#include "logger.h"
#include "scheduler.h"
#include "logger_task.h"
#include "ui_task.h"
#include "event_queue.h"

int main(void)
{
    /* HW semaphore Clock enable */
    __HAL_RCC_HSEM_CLK_ENABLE();
    /* Activate HSEM notification for Cortex-M4*/
    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_BOOT));

    /*
     Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
     perform system initialization (system clock config, external memory configuration.. )
     */
    HAL_PWREx_ClearPendingEvent();
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
    /* Clear HSEM flag */
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_BOOT));

    HAL_Init();
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_LOW_LATENCY));
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_DSP_BLOCKING));

    ccnt_init();
    scheduler_init();
    logger_task_init();
    ui_task_init();
    eq_m7_init();

    while (1)
    {
        int32_t scheduler_status = scheduler_dequeue_task();
        if (scheduler_status != 0)
        {
            logg(LOG_WRN, "Scheduler dequeue returned %d", scheduler_status);
        }

    }
}

