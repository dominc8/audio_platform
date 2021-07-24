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
#include <string.h>

static TS_MultiTouch_State_t ts_mt_state =
{ 0 };

static volatile int32_t button_state;
static volatile uint32_t JoyPinPressed = 0;
static uint32_t Joy_State;

static TS_Init_t hts_init;
static TS_Init_t *hts;
static TS_Gesture_Config_t gesture_conf;

static void button_init(void);
static void joystick_init(void);
static void lcd_init(void);
static void led_init(void);
static void handle_touch(void);
static void handle_joystick(void);
static void display_fft(void);
static void display_start_info(void);
static void setup_gui(void);
static void gather_and_log_fft_time(uint32_t fft_time);

int32_t ui_task_init(void)
{
    button_init();
    joystick_init();
    led_init();
    lcd_init();
    display_start_info();
    return scheduler_enqueue_task(&ui_task, NULL);
}

int32_t ui_task(void *arg)
{
    static uint32_t last_ccnt;
    uint32_t new_ccnt;
    uint32_t diff;

    do {
        new_ccnt = GET_CCNT();
        diff = DIFF_CCNT(last_ccnt, new_ccnt);
    } while ( ccnt_to_ms(diff) < 33);
    last_ccnt = new_ccnt;

    static int32_t i = 0;
    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "UI task");
    }
    handle_touch();
    handle_joystick();
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

static void joystick_init(void)
{
    BSP_JOY_Init(JOY1, JOY_MODE_EXTI, JOY_ALL);
    JoyPinPressed = 0;
    Joy_State = JOY_NONE;
}

static void led_init(void)
{
    /* LED_RED is reserved for ErrorHandler on CM7 */
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_BLUE);
    BSP_LED_Init(LED_ORANGE);
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
        logg(LOG_ERR, "lcd_init()<ts> failed with %d", ts_status);
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

static void handle_joystick(void)
{
    switch (JoyPinPressed)
    {
        case 0x01U:
            Joy_State = JOY_SEL;
            break;

        case 0x02U:
            Joy_State = JOY_DOWN;
            break;

        case 0x04U:
            Joy_State = JOY_LEFT;
            break;

        case 0x08U:
            Joy_State = JOY_RIGHT;
            break;

        case 0x10U:
            Joy_State = JOY_UP;
            break;
        default:
            Joy_State = JOY_NONE;
            break;
    }
    if (JoyPinPressed != 0)
    {
        logg(LOG_DBG, "Joystick %u", JoyPinPressed);
    }
    JoyPinPressed = 0;
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

extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef  hlcd_ltdc;
extern BSP_LCD_Ctx_t       Lcd_Ctx[1];

static uint32_t fft_bin[128] =
{
    0xFFFF0000, 0xFFFF0400, 0xFFFF0800, 0xFFFF0C00, 0xFFFF1000, 0xFFFF1400, 0xFFFF1800, 0xFFFF1C00,
    0xFFFF2000, 0xFFFF2400, 0xFFFF2800, 0xFFFF2C00, 0xFFFF3000, 0xFFFF3400, 0xFFFF3800, 0xFFFF3C00,
    0xFFFF4000, 0xFFFF4400, 0xFFFF4800, 0xFFFF4C00, 0xFFFF5000, 0xFFFF5400, 0xFFFF5800, 0xFFFF5C00,
    0xFFFF6000, 0xFFFF6400, 0xFFFF6800, 0xFFFF6C00, 0xFFFF7000, 0xFFFF7400, 0xFFFF7800, 0xFFFF7C00,
    0xFFFF8000, 0xFFFF8400, 0xFFFF8800, 0xFFFF8C00, 0xFFFF9000, 0xFFFF9400, 0xFFFF9800, 0xFFFF9C00,
    0xFFFFA000, 0xFFFFA400, 0xFFFFA800, 0xFFFFAC00, 0xFFFFB000, 0xFFFFB400, 0xFFFFB800, 0xFFFFBC00,
    0xFFFFC000, 0xFFFFC400, 0xFFFFC800, 0xFFFFCC00, 0xFFFFD000, 0xFFFFD400, 0xFFFFD800, 0xFFFFDC00,
    0xFFFFE000, 0xFFFFE400, 0xFFFFE800, 0xFFFFEC00, 0xFFFFF000, 0xFFFFF400, 0xFFFFF800, 0xFFFFFC00,

    0xFFFCFF00, 0xFFF8FF00, 0xFFF4FF00, 0xFFF0FF00, 0xFFECFF00, 0xFFE8FF00, 0xFFE4FF00, 0xFFE0FF00,
    0xFFDCFF00, 0xFFD8FF00, 0xFFD4FF00, 0xFFD0FF00, 0xFFCCFF00, 0xFFC8FF00, 0xFFC4FF00, 0xFFC0FF00,
    0xFFBCFF00, 0xFFB8FF00, 0xFFB4FF00, 0xFFB0FF00, 0xFFACFF00, 0xFFA8FF00, 0xFFA4FF00, 0xFFA0FF00,
    0xFF9CFF00, 0xFF98FF00, 0xFF94FF00, 0xFF90FF00, 0xFF8CFF00, 0xFF88FF00, 0xFF84FF00, 0xFF80FF00,
    0xFF7CFF00, 0xFF78FF00, 0xFF74FF00, 0xFF70FF00, 0xFF6CFF00, 0xFF68FF00, 0xFF64FF00, 0xFF60FF00,
    0xFF5CFF00, 0xFF58FF00, 0xFF54FF00, 0xFF50FF00, 0xFF4CFF00, 0xFF48FF00, 0xFF44FF00, 0xFF40FF00,
    0xFF3CFF00, 0xFF38FF00, 0xFF34FF00, 0xFF30FF00, 0xFF2CFF00, 0xFF28FF00, 0xFF24FF00, 0xFF20FF00,
    0xFF1CFF00, 0xFF18FF00, 0xFF14FF00, 0xFF10FF00, 0xFF0CFF00, 0xFF08FF00, 0xFF04FF00, 0xFF00FF00,
};

static void draw_bin(int32_t val, int32_t bin, int32_t y_offset)
{
    if (val <= 0)
    {
        return;
    }

  uint32_t *pSrc = &fft_bin[128 - val];
  uint32_t pDst = (hlcd_ltdc.LayerCfg[Lcd_Ctx[0].ActiveLayer].FBStartAdress) + (Lcd_Ctx[0].BppFactor*(Lcd_Ctx[0].XSize*(y_offset + (128 - val)) + 20 + 40*bin));
  uint32_t line_dx = Lcd_Ctx[0].BppFactor;

  hlcd_dma2d.Init.Mode         = DMA2D_M2M_PFC;
  hlcd_dma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  hlcd_dma2d.Init.OutputOffset = Lcd_Ctx[0].XSize - 1;

  /* Foreground Configuration */
  hlcd_dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hlcd_dma2d.LayerCfg[1].InputAlpha = 0xFF;
  hlcd_dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hlcd_dma2d.LayerCfg[1].InputOffset = 0;

  hlcd_dma2d.Instance = DMA2D;


  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hlcd_dma2d) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) == HAL_OK)
    {
        for(int32_t i = 0; i < 40; ++i)
        {
              if (HAL_DMA2D_Start(&hlcd_dma2d, (uint32_t)pSrc, (uint32_t)pDst, 1, val) == HAL_OK)
              {
                /* Polling For DMA transfer */
                (void)HAL_DMA2D_PollForTransfer(&hlcd_dma2d, 50);
              }
              pDst += line_dx;
        }
    }
  }

}

static void display_fft(void)
{
    const int32_t x0 = 20;
    const int32_t dx = 40;
    const int32_t y_left = 150;
    const int32_t y_right = 300;
    const int32_t ymax = 128;

    GUI_FillRect(x0, y_left, dx * SHARED_FFT_SIZE, ymax, GUI_COLOR_WHITE);

    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        int32_t val_left = limit_val(shared_fft_l[i], ymax);
        draw_bin(val_left, i, y_left);
//        GUI_FillRect(x0 + i * dx, y_left + ymax - val_left, dx, val_left, GUI_COLOR_GREEN);
//        GUI_FillRect(x0 + i * dx, y_left, dx, ymax - val_left, GUI_COLOR_WHITE);
    }
    GUI_FillRect(x0, y_right, dx * SHARED_FFT_SIZE, ymax, GUI_COLOR_WHITE);
    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        int32_t val_right = limit_val(shared_fft_r[i], ymax);
        draw_bin(val_right, i, y_right);
//        GUI_FillRect(x0 + i * dx, y_right + ymax - val_right, dx, val_right, GUI_COLOR_GREEN);
//        GUI_FillRect(x0 + i * dx, y_right, dx, ymax - val_right, GUI_COLOR_WHITE);
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

void BSP_JOY_Callback(JOY_TypeDef JOY, uint32_t JoyPin)
{
    JoyPinPressed = JoyPin;
}
