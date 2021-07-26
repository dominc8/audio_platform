#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "intercore_comm.h"
#include "shared_data.h"
#include "perf_meas.h"
#include "logger.h"

#define N_FFT_BIN   133

extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef hlcd_ltdc;
extern BSP_LCD_Ctx_t Lcd_Ctx[LCD_INSTANCES_NBR];

static uint32_t fft_bin[N_FFT_BIN];

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);

static void display_fft(void);
static void gather_and_log_fft_time(uint32_t fft_time);
static void init_fft_bin(void);
static void toggle_audio_on_m7(void);

void init_ui_fft(ui_state_t *ui_state)
{
    ui_state->f_handle_ui = &handle_ui_init;
}

static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    static int32_t cnt = 0;
    if (++cnt > 1000)
    {
        cnt = 0;
        logg(LOG_DBG, "handle_ui in ui_fft");
    }
    UI_STATE next_state = UI_STATE_AUDIO_VISUALIZATION;
    if (button_state == 1)
    {
        init_ui_fft(self);
        next_state = UI_STATE_START_SCREEN;
        toggle_audio_on_m7();
    }
    else
    {
        if (new_data_flag != 0)
        {
            uint32_t start = GET_CCNT();
            display_fft();
            uint32_t stop = GET_CCNT();
            uint32_t fft_time = DIFF_CCNT(start, stop);
            gather_and_log_fft_time(ccnt_to_us(fft_time));
        }
    }
    return next_state;
}

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
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

    BSP_LED_Off(LED_ORANGE);
    BSP_LED_On(LED_BLUE);

    new_data_flag = 0;
    toggle_audio_on_m7();
    init_fft_bin();

    self->f_handle_ui = &handle_ui;
    logg(LOG_DBG, "handle_ui_init in ui_fft");
    return UI_STATE_AUDIO_VISUALIZATION;
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
                    * (Lcd_Ctx[0].XSize * (y_offset + (N_FFT_BIN - val)) + x_offset
                            + bin_width * bin));
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

static void toggle_audio_on_m7(void)
{
    if (0 == lock_unlock_hsem(HSEM_START_AUDIO))
    {
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        BSP_LED_On(LED_ORANGE);
    }
}
