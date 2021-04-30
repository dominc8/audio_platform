#include "main.h"
#include "stdio.h"
#include "agh_logo.h"
#include "shared_data.h"
#include "intercore_comm.h"
#include "perf_meas.h"
#include "logger.h"

#define HSEM_ID_0 (0U) /* HW semaphore 0*/

TS_State_t ts_state =
{ 0 };
TS_MultiTouch_State_t ts_mt_state =
{ 0 };

static TS_Init_t hts_init;
TS_Init_t *hts;
TS_Gesture_Config_t gesture_conf;

__IO uint32_t ButtonState = 0;
static void display_start_info(void);
static void setup_gui(void);
static void display_data(void);
static void display_fft(void);
static void button_init(void);
static void led_init(void);
static void lcd_init(void);
static void handle_touch(void);

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
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_DATA));

    int32_t counter = 0;
    new_data_flag = 0;
    uint32_t fft_time;

    button_init();
    led_init();
    lcd_init();
    logger_init(115200);
    ccnt_init();

    display_start_info();
    BSP_LED_On(LED_ORANGE);

    while (1)
    {
        if (ButtonState == 1)
        {
            ButtonState = 0;
            BSP_LED_Off(LED_ORANGE);
            BSP_LED_On(LED_BLUE);

            setup_gui();
            new_data_flag = 0;
            if (0 == lock_unlock_hsem(HSEM_START_AUDIO))
            {
                BSP_LED_On(LED_GREEN);
            }
            else
            {
                BSP_LED_On(LED_ORANGE);
            }
        }
        else
        {
            if (new_data_flag != 0)
            {
                new_data_flag = 0;
                // display_data();
                uint32_t start = GET_CCNT();
                display_fft();
                uint32_t stop = GET_CCNT();
                fft_time = DIFF_CCNT(start, stop);
            }
            else
            {
                handle_touch();
            }
        }
        if (++counter > 2000)
        {
            static LOG_LEVEL log_lvl = LOG_ERR;
            counter = 0;
            BSP_LED_Off(LED_ORANGE);
            BSP_LED_Off(LED_BLUE);
            BSP_LED_Off(LED_GREEN);
            logg(log_lvl, "FFT display time: %u ms", ccnt_to_ms(fft_time));
            if (++log_lvl == LOG_N)
                log_lvl = LOG_ERR;
        }
    }
}

/**
 * @brief  Display main demo messages
 * @param  None
 * @retval None
 */
static void display_start_info(void)
{
    uint32_t x_size;
    uint32_t y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_SetFont(&GUI_DEFAULT_FONT);

    GUI_SetBackColor(GUI_COLOR_WHITE);
    GUI_Clear(GUI_COLOR_WHITE);

    GUI_SetTextColor(GUI_COLOR_DARKBLUE);

    GUI_DisplayStringAt(0, 10, (uint8_t*) "Start screen", CENTER_MODE);

    GUI_DrawBitmap(x_size / 2 - 15, y_size - 80, (uint8_t*) aghlogo);

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

static void setup_gui(void)
{
    char temp[64];
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_WHITE);

    GUI_FillRect(0, 0, x_size, 90, GUI_COLOR_BLUE);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLUE);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 0, (uint8_t*) "Analog input/output demo", CENTER_MODE);
    sprintf(temp, "addr: %p", &new_data_flag);
    GUI_DisplayStringAt(0, 40, (uint8_t*) temp, CENTER_MODE);
    GUI_SetFont(&Font16);

    GUI_DrawRect(10, 100, x_size - 20, y_size - 110, GUI_COLOR_BLUE);
    GUI_DrawRect(11, 101, x_size - 22, y_size - 112, GUI_COLOR_BLUE);
}

static inline int32_t limit_val(int32_t val, int32_t val_max)
{
    if (val < 0)
        return 0;
    else if (val > val_max)
        return val_max;
    else
        return val;
}

static void display_data(void)
{
    const int32_t group_size = 4;
    const int32_t n_channels = 2;
    const int32_t n_groups = 512 / (group_size * n_channels);
    const int32_t x0 = 20;
    const int32_t dx = 10;
    const int32_t y_left = 150;
    const int32_t y_right = 300;
    const int32_t ymax = 128;
    const int32_t ymax_shift = 7;
    const int32_t val_shift = 13 - ymax_shift; // theoretically should be 16..18 - ymax_shift

    for (int32_t i = 0; i < n_groups; ++i)
    {
        int32_t val_left = 0;
        int32_t val_right = 0;
        int32_t idx = i * group_size;
        ;
        for (int32_t j = 0; j < group_size; ++j)
        {
            val_left += (int16_t) shared_audio_data[idx];
            val_right += (int16_t) shared_audio_data[idx + 1];
            idx += n_channels;
        }
        val_left = limit_val(val_left >> (val_shift), ymax);
        val_right = limit_val(val_right >> (val_shift), ymax);

        GUI_FillRect(x0 + i * dx, y_left + ymax - val_left, dx, val_left, GUI_COLOR_GREEN);
        GUI_FillRect(x0 + i * dx, y_left, dx, ymax - val_left, GUI_COLOR_WHITE);
        GUI_FillRect(x0 + i * dx, y_right + ymax - val_right, dx, val_right, GUI_COLOR_GREEN);
        GUI_FillRect(x0 + i * dx, y_right, dx, ymax - val_right, GUI_COLOR_WHITE);
    }
}

static void display_fft(void)
{
    const int32_t x0 = 20;
    const int32_t dx = 40;
    const int32_t y_left = 150;
    const int32_t y_right = 300;
    const int32_t ymax = 128;
    const int32_t ymax_shift = 16;
    const int32_t val_shift = 16 - ymax_shift; // theoretically should be 16..18 - ymax_shift

    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        int32_t val_left = limit_val(shared_fft_l[i] >> (val_shift), ymax);
        int32_t val_right = limit_val(shared_fft_r[i] >> (val_shift), ymax);

        GUI_FillRect(x0 + i * dx, y_left + ymax - val_left, dx, val_left, GUI_COLOR_GREEN);
        GUI_FillRect(x0 + i * dx, y_left, dx, ymax - val_left, GUI_COLOR_WHITE);
        GUI_FillRect(x0 + i * dx, y_right + ymax - val_right, dx, val_right, GUI_COLOR_GREEN);
        GUI_FillRect(x0 + i * dx, y_right, dx, ymax - val_right, GUI_COLOR_WHITE);
    }
}

static void button_init(void)
{
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);
}

static void led_init(void)
{
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);
}

static void lcd_init(void)
{
    uint32_t x_size, y_size;
    uint32_t ts_status = BSP_ERROR_NONE;

    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
    GUI_SetFuncDriver(&LCD_Driver);
    GUI_SetFont(&GUI_DEFAULT_FONT);

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    hts = &hts_init;
    hts->Width = x_size;
    hts->Height = y_size;
    hts->Orientation = TS_SWAP_XY | TS_SWAP_Y;
    hts->Accuracy = 5;

    while (lock_hsem(HSEM_I2C4))
        ;

    ts_status = BSP_TS_Init(0, hts);
    ts_status = BSP_TS_GestureConfig(0, &gesture_conf);

    unlock_hsem(HSEM_I2C4);

    if (BSP_ERROR_NONE != ts_status)
    {
        BSP_LED_On(LED_RED);
    }
}

static uint16_t saturate_u16(uint16_t x, uint16_t min, uint16_t max)
{
    if (x < min)
    {
        x = min;
    }
    else if (x > max)
    {
        x = max;
    }
    return x;
}

static void handle_touch(void)
{
    const uint16_t circle_radius = 10;
    uint32_t gesture_id = GESTURE_ID_NO_GESTURE;
    uint16_t x = 0;
    uint16_t y = 0;
    uint32_t drawTouch1 = 0; /* activate/deactivate draw of footprint of touch 1 */
    uint32_t drawTouch2 = 0; /* activate/deactivate draw of footprint of touch 2 */
    uint32_t ts_status = BSP_ERROR_NONE;
    uint32_t x_size, y_size;
    char lcd_string[70] = "";

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    /* Check in polling mode in touch screen the touch status and coordinates */
    /* of touches if touch occurred                                           */

    while (lock_hsem(HSEM_I2C4))
        ;
    ts_status = BSP_TS_Get_MultiTouchState(0, &ts_mt_state);
    unlock_hsem(HSEM_I2C4);

    if (ts_mt_state.TouchDetected)
    {
        GUI_FillRect(0, y_size - 60, x_size, 60, GUI_COLOR_BLUE);

        /* One or dual touch have been detected  */

        /* Desactivate drawing footprint of touch 1 and touch 2 until validated against boundaries of touch pad values */
        drawTouch1 = drawTouch2 = 0;

        /* Get X and Y position of the first touch post calibrated */
        x = saturate_u16(ts_mt_state.TouchX[0], circle_radius, x_size - circle_radius);
        y = saturate_u16(ts_mt_state.TouchY[0], circle_radius, y_size - circle_radius);

        GUI_FillCircle(x, y, circle_radius, GUI_COLOR_ORANGE);

        GUI_SetFont(&Font12);
        sprintf((char*) lcd_string, "x1 = %u, y1 = %u, Event = %lu, Weight = %lu", x, y,
                ts_mt_state.TouchEvent[0], ts_mt_state.TouchWeight[0]);
        GUI_DisplayStringAt(0, y_size - 45, lcd_string, CENTER_MODE);

        if (ts_mt_state.TouchDetected > 1)
        {
            /* Get X and Y position of the second touch post calibrated */
            x = saturate_u16(ts_mt_state.TouchX[1], circle_radius, x_size - circle_radius);
            y = saturate_u16(ts_mt_state.TouchY[1], circle_radius, y_size - circle_radius);

            GUI_FillCircle(x, y, circle_radius, GUI_COLOR_ORANGE);

            GUI_SetFont(&Font12);
            sprintf((char*) lcd_string, "x2 = %u, y2 = %u, Event = %lu, Weight = %lu", x, y,
                    ts_mt_state.TouchEvent[0], ts_mt_state.TouchWeight[0]);
            GUI_DisplayStringAt(0, y_size - 35, lcd_string, CENTER_MODE);

            return;
            while (lock_hsem(HSEM_I2C4))
                ;
            ts_status = BSP_TS_GetGestureId(0, &gesture_id);
            unlock_hsem(HSEM_I2C4);

            sprintf((char*) lcd_string, "Gesture Id = %lu", gesture_id);
            GUI_DisplayStringAt(0, y_size - 25, lcd_string, CENTER_MODE);
        }

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
