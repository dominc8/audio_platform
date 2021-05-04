#include "main.h"
#include "intercore_comm.h"
#include "perf_meas.h"
#include "logger.h"
#include "scheduler.h"
#include "logger_task.h"
#include "ui_task.h"

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

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_START_AUDIO));

    ccnt_init();
    scheduler_init();
    logger_task_init();
    ui_task_init();

    BSP_LED_On(LED_ORANGE);

    while (1)
    {
        int32_t scheduler_status = scheduler_dequeue_task();
        if (scheduler_status != 0)
        {
            logg(LOG_WRN, "Scheduler dequeue returned %d", scheduler_status);
        }

    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void Error_Handler(void)
{
    /* Turn LED REDon */
    volatile int32_t i = 0;
    while (1)
    {
        for (i = 0; i < 1000000; ++i)
        {
        }
        BSP_LED_On(LED_RED);
        for (i = 0; i < 1000000; ++i)
        {
        }
        BSP_LED_Off(LED_RED);
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
