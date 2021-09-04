#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "intercore_comm.h"
#include "agh_logo.h"
#include "ui_utils.h"

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui_init_from_fft(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static void toggle_audio_on_m7(HSEM_ID dsp_type);

static ui_button_t ui_button_low_lat;
static ui_button_t ui_button_dsp_block;
static ui_button_t ui_button_benchmark;

static HSEM_ID last_dsp_type = HSEM_LOW_LATENCY;

void init_ui_start_screen(ui_state_t *ui_state)
{
    ui_state->f_handle_ui = &handle_ui_init;
}

static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    UI_STATE next_state = UI_STATE_START_SCREEN;
    (void) button_state;
    (void) joy_pin;

    if (is_ui_button_touched(&ui_button_low_lat, touch_state) == 1)
    {
        self->f_handle_ui = &handle_ui_init_from_fft;
        next_state = UI_STATE_AUDIO_VISUALIZATION;
        toggle_audio_on_m7(HSEM_LOW_LATENCY);
        last_dsp_type = HSEM_LOW_LATENCY;
    }
    else if (is_ui_button_touched(&ui_button_dsp_block, touch_state) == 1)
    {
        self->f_handle_ui = &handle_ui_init_from_fft;
        next_state = UI_STATE_AUDIO_VISUALIZATION;
        toggle_audio_on_m7(HSEM_DSP_BLOCKING);
        last_dsp_type = HSEM_DSP_BLOCKING;
    }
    else if (is_ui_button_touched(&ui_button_benchmark, touch_state) == 1)
    {
        self->f_handle_ui = &handle_ui_init;
        lock_unlock_hsem(HSEM_BENCHMARK);
        next_state = UI_STATE_BENCHMARK;
    }
    return next_state;
}

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    uint32_t x_size;
    uint32_t y_size;
    (void) touch_state;
    (void) button_state;
    (void) joy_pin;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_BLACK);

    GUI_FillRect(0, 46, x_size, 2, GUI_COLOR_RED);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLACK);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 12, (uint8_t*) "Audio DSP Platform", CENTER_MODE);
    GUI_DrawBitmap(x_size - 30, 0, (uint8_t*) aghlogo);

    ui_button_low_lat.x0 = 2;
    ui_button_low_lat.x1 = x_size / 4;
    ui_button_low_lat.y0 = 50;
    ui_button_low_lat.y1 = y_size / 3;
    ui_button_low_lat.color = 0xFF19784E;
    ui_button_low_lat.font = &Font24;
    ui_button_low_lat.text = "Low latency";
    set_x0_text_centered(&ui_button_low_lat);
    set_y0_text_centered(&ui_button_low_lat);
    draw_ui_button(&ui_button_low_lat);

    ui_button_dsp_block.x0 = 3 * x_size / 4;
    ui_button_dsp_block.x1 = x_size - 2;
    ui_button_dsp_block.y0 = 50;
    ui_button_dsp_block.y1 = y_size / 3;
    ui_button_dsp_block.color = 0xFF19784E;
    ui_button_dsp_block.font = &Font24;
    ui_button_dsp_block.text = "DSP blocks";
    set_x0_text_centered(&ui_button_dsp_block);
    set_y0_text_centered(&ui_button_dsp_block);
    draw_ui_button(&ui_button_dsp_block);

    ui_button_benchmark.x0 = 3 * x_size / 8;
    ui_button_benchmark.x1 = 5 * x_size / 8;
    ui_button_benchmark.y0 = 50;
    ui_button_benchmark.y1 = y_size / 3;
    ui_button_benchmark.color = 0xFF19784E;
    ui_button_benchmark.font = &Font24;
    ui_button_benchmark.text = "Benchmark";
    set_x0_text_centered(&ui_button_benchmark);
    set_y0_text_centered(&ui_button_benchmark);
    draw_ui_button(&ui_button_benchmark);

    self->f_handle_ui = &handle_ui;
    return UI_STATE_START_SCREEN;
}

static UI_STATE handle_ui_init_from_fft(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    UI_STATE ret_val = handle_ui_init(self, touch_state, button_state, joy_pin);
    toggle_audio_on_m7(last_dsp_type);
    return ret_val;
}

static void toggle_audio_on_m7(HSEM_ID dsp_type)
{
    if (0 == lock_unlock_hsem(dsp_type))
    {
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        BSP_LED_On(LED_ORANGE);
    }
}

