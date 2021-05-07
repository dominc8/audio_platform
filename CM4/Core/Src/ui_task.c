#include "ui_task.h"
#include "logger.h"
#include "scheduler.h"
#include "intercore_comm.h"
#include "shared_data.h"
#include "perf_meas.h"
#include "agh_logo.h"

#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_lcd.h"
#include "stm32h747i_discovery_ts.h"
#include "basic_gui.h"
#include <stddef.h>
#include <stdio.h>

static TS_MultiTouch_State_t ts_mt_state =
{ 0 };

static volatile int32_t button_state;

static TS_Init_t hts_init;
static TS_Init_t *hts;
static TS_Gesture_Config_t gesture_conf;

static void button_init(void);
static void lcd_init(void);
static void led_init(void);
static void handle_touch(void);
static void display_fft(void);
static void display_start_info(void);
static void setup_gui(void);
static void gather_and_log_fft_time(uint32_t fft_time);

int32_t ui_task_init(void)
{
    button_init();
    led_init();
    lcd_init();
    display_start_info();
    return scheduler_enqueue_task(&ui_task, NULL);
}

int32_t ui_task(void *arg)
{
    static int32_t i = 0;
    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "UI task");
    }
    handle_touch();
    if (ui_get_button_state() == 1)
    {
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
    if (new_data_flag != 0)
    {
        uint32_t start = GET_CCNT();
        display_fft();
        uint32_t stop = GET_CCNT();
        uint32_t fft_time = DIFF_CCNT(start, stop);
        gather_and_log_fft_time(ccnt_to_us(fft_time));
    }
    return scheduler_enqueue_task(&ui_task, NULL);
}

int32_t ui_peek_button_state()
{
    return button_state;
}

int32_t ui_get_button_state()
{
    int32_t ret_val = button_state;
    button_state = 0;
    return ret_val;
}

static void button_init(void)
{
    button_state = 0;
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
    uint32_t ts_status = BSP_ERROR_NONE;
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    /* Check in polling mode in touch screen the touch status and coordinates */
    /* of touches if touch occurred                                           */

    while (lock_hsem(HSEM_I2C4))
        ;
    ts_status = BSP_TS_Get_MultiTouchState(0, &ts_mt_state);
    unlock_hsem(HSEM_I2C4);

    if (BSP_ERROR_NONE == ts_status && ts_mt_state.TouchDetected)
    {
        /* One or dual touch have been detected  */

        /* Get X and Y position of the first touch post calibrated */
        x = saturate_u16(ts_mt_state.TouchX[0], circle_radius, x_size - circle_radius);
        y = saturate_u16(ts_mt_state.TouchY[0], circle_radius, y_size - circle_radius);

        GUI_FillCircle(x, y, circle_radius, GUI_COLOR_ORANGE);

        logg(LOG_DBG, "x1 = %u, y1 = %u, Event = %lu, Weight = %lu", x, y,
                ts_mt_state.TouchEvent[0], ts_mt_state.TouchWeight[0]);

        if (ts_mt_state.TouchDetected > 1)
        {
            /* Get X and Y position of the second touch post calibrated */
            x = saturate_u16(ts_mt_state.TouchX[1], circle_radius, x_size - circle_radius);
            y = saturate_u16(ts_mt_state.TouchY[1], circle_radius, y_size - circle_radius);

            GUI_FillCircle(x, y, circle_radius, GUI_COLOR_ORANGE);

            logg(LOG_DBG, "x2 = %u, y2 = %u, Event = %lu, Weight = %lu", x, y,
                    ts_mt_state.TouchEvent[0], ts_mt_state.TouchWeight[0]);

            return;
            while (lock_hsem(HSEM_I2C4))
                ;
            ts_status = BSP_TS_GetGestureId(0, &gesture_id);
            unlock_hsem(HSEM_I2C4);

            if (BSP_ERROR_NONE == ts_status)
            {
                logg(LOG_DBG, "Gesture Id = %lu", gesture_id);
            }
        }

    }
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

static void setup_gui(void)
{
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_WHITE);

    GUI_FillRect(0, 0, x_size, 90, GUI_COLOR_BLUE);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLUE);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 0, (uint8_t*) "Analog input/output demo", CENTER_MODE);

    GUI_DrawRect(10, 100, x_size - 20, y_size - 110, GUI_COLOR_BLUE);
    GUI_DrawRect(11, 101, x_size - 22, y_size - 112, GUI_COLOR_BLUE);
}


static void gather_and_log_fft_time(uint32_t fft_time)
{
    static uint32_t acc_fft_time = 0;
    static int32_t cnt = 0;
    acc_fft_time += fft_time;
    ++cnt;
    if (1024 == cnt)
    {
        logg(LOG_DBG, "Avg FFT display time: %u us", acc_fft_time >> 10);
        acc_fft_time = 0;
        cnt = 0;
    }
}


void BSP_PB_Callback(Button_TypeDef button)
{
    if (button == BUTTON_WAKEUP)
    {
        button_state = 1;
    }
}
