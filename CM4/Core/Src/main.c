#include "main.h"
#include "stlogo.h"
#include "shared_data.h"

#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#define HSEM_DATA (1U) /* HW semaphore for signaling new data */

__IO uint32_t ButtonState = 0;
static void Display_DemoDescription(void);

int main(void)
{
/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /* Activate HSEM notification for Cortex-M4*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

  /*
  Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
  perform system initialization (system clock config, external memory configuration.. )
  */
  HAL_PWREx_ClearPendingEvent();
  HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  /* Clear HSEM flag */
  __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Shared data test start */
  for (int32_t i = 0; i < SHARED_AUDIO_DATA_SIZE; ++i)
  {
      shared_audio_data[i] = (' ' << 8) + (i % ('z' - 'a' + 1)) + 'a';
  }
  shared_audio_data[SHARED_AUDIO_DATA_SIZE - 1] = '\0';
  /* Shared data test end */


  int32_t led_idx = 0;

    /* Initialize Button and LEDs */
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);

    /* Initialize the LCD */
    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
    GUI_SetFuncDriver(&LCD_Driver);
    GUI_SetFont(&GUI_DEFAULT_FONT);
    Display_DemoDescription();
    BSP_LED_On(led_idx);

  while (1)
  {
      if(ButtonState == 1)
      {
          ButtonState = 0;
          BSP_LED_Off(led_idx);
          led_idx = (led_idx + 1) & 0x3;
          BSP_LED_On(led_idx);
      }
  }
}

/**
 * @brief  Display main demo messages
 * @param  None
 * @retval None
 */
static void Display_DemoDescription(void)
{
    char desc[64];
    uint32_t x_size;
    uint32_t y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    /* Set LCD Foreground Layer  */
    GUI_SetFont(&GUI_DEFAULT_FONT);

    /* Clear the LCD */
    GUI_SetBackColor(GUI_COLOR_WHITE);
    GUI_Clear(GUI_COLOR_WHITE);

    /* Set the LCD Text Color */
    GUI_SetTextColor(GUI_COLOR_DARKBLUE);

    /* Display LCD messages */
    GUI_DisplayStringAt(0, 10, (uint8_t*) "STM32H747I BSP", CENTER_MODE);
    GUI_DisplayStringAt(0, 35, (uint8_t*) "Drivers examples", CENTER_MODE);

    /* Draw Bitmap */
    GUI_DrawBitmap((x_size - 80) / 2, 65, (uint8_t*) stlogo);

    GUI_SetFont(&Font12);
    GUI_DisplayStringAt(0, y_size - 20, (uint8_t*) "Copyright (c) STMicroelectronics 2018",
            CENTER_MODE);

    GUI_SetFont(&Font16);
    BSP_LCD_FillRect(0, 0, y_size / 2 + 15, x_size, 60, GUI_COLOR_BLUE);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLUE);
    GUI_DisplayStringAt(0, y_size / 2 + 30, (uint8_t*) "Press Wakeup button to start :",
            CENTER_MODE);
}

/**
 * @brief  Check for user input
 * @param  None
 * @retval Input state (1 : active / 0 : Inactive)
 */
uint8_t CheckForUserInput(void)
{
    return ButtonState;
}

/**
 * @brief  Button Callback
 * @param  Button Specifies the pin connected EXTI line
 * @retval None
 */
void BSP_PB_Callback(Button_TypeDef Button)
{
    if (Button == BUTTON_WAKEUP)
    {

        ButtonState = 1;
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
