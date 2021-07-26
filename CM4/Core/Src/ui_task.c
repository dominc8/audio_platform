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

#define N_FFT_BIN   133

extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef hlcd_ltdc;
extern BSP_LCD_Ctx_t Lcd_Ctx[1];

static TS_MultiTouch_State_t ts_mt_state =
{ 0 };

static volatile int32_t button_state;
static volatile uint32_t JoyPinPressed = 0;
static uint32_t Joy_State;
static uint32_t fft_bin[N_FFT_BIN];

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
static void init_fft_bin(void);

int32_t ui_task_init(void)
{
    button_init();
    joystick_init();
    led_init();
    lcd_init();
    display_start_info();
    init_fft_bin();
    return scheduler_enqueue_task(&ui_task, NULL);
}

int32_t ui_task(void *arg)
{
    static uint32_t last_ccnt;
    uint32_t new_ccnt;
    uint32_t diff;

    do
    {
        new_ccnt = GET_CCNT();
        diff = DIFF_CCNT(last_ccnt, new_ccnt);
    } while (ccnt_to_ms(diff) < 33);
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

static void draw_bin(int32_t val, int32_t bin, int32_t bin_width, int32_t y_offset)
{
    if (val <= 0)
    {
        return;
    }

    int32_t x_offset = bin_width >> 1;

    uint32_t *pSrc = &fft_bin[N_FFT_BIN - val];
    uint32_t pDst = (hlcd_ltdc.LayerCfg[Lcd_Ctx[0].ActiveLayer].FBStartAdress)
            + (Lcd_Ctx[0].BppFactor
                    * (Lcd_Ctx[0].XSize * (y_offset + (N_FFT_BIN - val)) + x_offset + bin_width
                            + bin));
    uint32_t line_dx = Lcd_Ctx[0].BppFactor;

    hlcd_dma2d.Init.Mode = DMA2D_M2M_PFC;
    hlcd_dma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hlcd_dma2d.Init.OutputOffset = Lcd_Ctx[0].XSize - 1;

    /* Foreground Configuration */
    hlcd_dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hlcd_dma2d.LayerCfg[1].InputAlpha = 0xFF;
    hlcd_dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hlcd_dma2d.LayerCfg[1].InputOffset = 0;

    hlcd_dma2d.Instance = DMA2D;

    /* DMA2D Initialization */
    if (HAL_DMA2D_Init(&hlcd_dma2d) == HAL_OK)
    {
        if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) == HAL_OK)
        {
            for (int32_t i = 0; i < bin_width; ++i)
            {
                if (HAL_DMA2D_Start(&hlcd_dma2d, (uint32_t) pSrc, (uint32_t) pDst, 1, val)
                        == HAL_OK)
                {
                    /* Polling For DMA transfer */
                    (void) HAL_DMA2D_PollForTransfer(&hlcd_dma2d, 50);
                }
                pDst += line_dx;
            }
        }
    }

}

static void display_fft(void)
{
    const int32_t bin_width = 40;
    const int32_t y_left = 150;
    const int32_t y_right = 300;
    const int32_t ymax = N_FFT_BIN;

    GUI_FillRect(bin_width >> 1, y_left, bin_width * SHARED_FFT_SIZE, ymax, GUI_COLOR_BLACK);

    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        int32_t val_left = limit_val(shared_fft_l[i], ymax);
        draw_bin(val_left, i, bin_width, y_left);
    }
    GUI_FillRect(bin_width >> 1, y_right, bin_width * SHARED_FFT_SIZE, ymax, GUI_COLOR_BLACK);
    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        int32_t val_right = limit_val(shared_fft_r[i], ymax);
        draw_bin(val_right, i, bin_width, y_right);
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

    GUI_DisplayStringAt(0, 10, (uint8_t*) "Audio DSP Platform", CENTER_MODE);

    GUI_DrawBitmap(x_size / 2 - 15, y_size - 80, (uint8_t*) aghlogo);

    GUI_SetFont(&Font16);
    BSP_LCD_FillRect(0, 0, y_size / 2 + 15, x_size, 60, GUI_COLOR_BLUE);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLUE);
    GUI_DisplayStringAt(0, y_size / 2 + 30, (uint8_t*) "Press button to start:", CENTER_MODE);
}

static void setup_gui(void)
{
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_BLACK);

    GUI_FillRect(0, 38, x_size, 2, GUI_COLOR_RED);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLACK);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 10, (uint8_t*) "Audio visualization", CENTER_MODE);
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

static void init_fft_bin(void)
{
    // @formatter:off
    uint32_t colors[15] =
    {
        0xFFC01010,
        0xFFF01010,
        0xFFFF1515,
        0xFFFF4525,
        0xFFFF9825,
        0xFFFFB830,
        0xFFFFE035,
        0xFFFFFF40,
        0xFFE0FF35,
        0xFFB8FF30,
        0xFF98FF25,
        0xFF65FF25,
        0xFF25F020,
        0xFF20C015,
        0xFF10A010
    };
    // @formatter:on
    for (int32_t i = 0; i < N_FFT_BIN;)
    {
        for (int32_t j = 0; j < 15 && i < N_FFT_BIN; ++j)
        {
            for (int32_t k = 0; k < 7 && i < N_FFT_BIN; ++k)
            {
                fft_bin[i++] = colors[j];
            }
            for (int32_t k = 0; k < 2 && i < N_FFT_BIN; ++k)
            {
                fft_bin[i++] = 0xFF000000;
            }
        }
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
