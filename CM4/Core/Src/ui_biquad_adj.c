#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "shared_data.h"
#include "perf_meas.h"
#include "logger.h"
#include "ui_utils.h"
#include <stdio.h>

static float get_coeff_from_slider(const ui_slider_t *s)
{
    float coeff = 2 * s->val - s->width;
    coeff = 2.F * coeff / s->width;
    return coeff;
}

static void put_coeff_to_slider(ui_slider_t *s, float coeff)
{
    coeff = coeff * s->width / 2.F + s->width;
    s->val = coeff / 2;
}

static void print_biquad_slider_val(const ui_slider_t *s)
{
    float coeff = get_coeff_from_slider(s);

    sFONT *font = GUI_GetFont();
    GUI_SetFont(&Font16);
    char str[8];
    snprintf(&str[0], sizeof(str), "%+.4f", coeff);
    str[7] = '\0';
    uint32_t x0 = s->x0 + s->width / 2 - Font16.Width * 3;
    uint32_t y0 = s->y0 + s->height / 2 - Font16.Height / 2;
    GUI_DisplayStringAt(x0, y0, (uint8_t*) &str[0], LEFT_MODE);
    GUI_SetFont(font);
}

static void debug_biquad_coeff(void)
{
    logg(LOG_INF, "BIQUAD coeffs:");
    for (int32_t ch = 0; ch < 2; ++ch)
    {
        for (int32_t i = 0; i < N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES; ++i)
        {
            logg(LOG_INF, "[%d][%d]: %.4f", ch, i, biquad_coeffs[ch][i]);
        }
    }
}

static ui_slider_t biquad_slider;
static ui_button_t biquad_stage_up;
static ui_button_t biquad_stage_down;
static ui_button_t biquad_force_coeffs;

static int32_t biquad_coeff_idx;
static int32_t biquad_channel;

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);

static void handle_joy(JOYPin_TypeDef joy_pin);
static void save_biquad_coeff(void);
static void update_biquad_coeff(void);
static void print_biquad_info(void);
static void force_coeffs_in_range(void);

void init_ui_biquad_adj(ui_state_t *ui_state)
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
        logg(LOG_DBG, "handle_ui in ui_biquad_adj");
    }
    UI_STATE next_state = UI_STATE_BIQUAD_ADJ;
    if (joy_pin != JOY_NONE)
    {
        handle_joy(joy_pin);
        print_biquad_info();
    }
    else if (button_state == 1)
    {
        init_ui_biquad_adj(self);
        next_state = UI_STATE_AUDIO_VISUALIZATION;
        debug_biquad_coeff();
    }
    else
    {
        update_ui_slider(&biquad_slider, touch_state);
        draw_ui_slider(&biquad_slider);

        if (is_ui_button_touched(&biquad_stage_up, touch_state) == 1)
        {
            if (biquad_stages[biquad_channel] < MAX_BIQUAD_STAGES)
            {
                biquad_stages[biquad_channel]++;
            }
            print_biquad_info();
        }
        if (is_ui_button_touched(&biquad_stage_down, touch_state) == 1)
        {
            if (biquad_stages[biquad_channel] > 1)
            {
                biquad_stages[biquad_channel]--;
            }
            print_biquad_info();
        }
        if (is_ui_button_touched(&biquad_force_coeffs, touch_state) == 1)
        {
            force_coeffs_in_range();
            print_biquad_info();
        }
    }
    return next_state;
}

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    uint32_t x_size, y_size;
    (void) touch_state;
    (void) button_state;
    (void) joy_pin;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_BLACK);

    GUI_FillRect(0, 38, x_size, 2, GUI_COLOR_RED);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLACK);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 10, (uint8_t*) "BIQUAD Adjustment", CENTER_MODE);

    biquad_channel = 0;
    biquad_coeff_idx = 0;

    biquad_slider.x0 = 10;
    biquad_slider.y0 = y_size / 2;
    biquad_slider.width = (x_size - 20) & 0x00000FFE;
    biquad_slider.height = 40;
    biquad_slider.color = GUI_COLOR_GRAY;
    biquad_slider.print_val = &print_biquad_slider_val;
    put_coeff_to_slider(&biquad_slider, biquad_coeffs[0][0]);
    draw_ui_slider(&biquad_slider);

    print_biquad_info();

    biquad_stage_up.x0 = 3 * x_size / 4;
    biquad_stage_up.y0 = y_size / 8;
    biquad_stage_up.x1 = x_size;
    biquad_stage_up.y1 = 3 * y_size / 8;
    biquad_stage_up.text = "Stage++";
    biquad_stage_up.font = &Font24;
    biquad_stage_up.color = 0xFF19784E;
    biquad_stage_down.x0 = 0;
    biquad_stage_down.y0 = y_size / 8;
    biquad_stage_down.x1 = x_size / 4;
    biquad_stage_down.y1 = 3 * y_size / 8;
    biquad_stage_down.text = "Stage--";
    biquad_stage_down.font = &Font24;
    biquad_stage_down.color = 0xFFAF2F44;
    biquad_force_coeffs.x0 = 3 * x_size / 8;
    biquad_force_coeffs.y0 = y_size / 8;
    biquad_force_coeffs.x1 = 5 * x_size / 8;
    biquad_force_coeffs.y1 = 3 * y_size / 8;
    biquad_force_coeffs.text = "Fix coeffs";
    biquad_force_coeffs.font = &Font24;
    biquad_force_coeffs.color = 0xFF2E89FF;
    set_x0_text_centered(&biquad_stage_up);
    set_y0_text_centered(&biquad_stage_up);
    set_x0_text_centered(&biquad_stage_down);
    set_y0_text_centered(&biquad_stage_down);
    set_x0_text_centered(&biquad_force_coeffs);
    set_y0_text_centered(&biquad_force_coeffs);
    draw_ui_button(&biquad_stage_up);
    draw_ui_button(&biquad_stage_down);
    draw_ui_button(&biquad_force_coeffs);

    BSP_LED_Off(LED_ORANGE);
    BSP_LED_On(LED_BLUE);

    self->f_handle_ui = &handle_ui;
    logg(LOG_DBG, "handle_ui_init in ui_biquad_adj");
    return UI_STATE_BIQUAD_ADJ;
}

static void handle_joy(JOYPin_TypeDef joy_pin)
{
    switch (joy_pin)
    {
        case JOY_SEL:
            save_biquad_coeff();
            update_biquad_coeff();
            break;
        case JOY_DOWN:
            biquad_channel = 0;
            break;
        case JOY_UP:
            biquad_channel = 1;
            break;
        case JOY_LEFT:
            if (biquad_coeff_idx > 0)
            {
                biquad_coeff_idx--;
            }
            break;
        case JOY_RIGHT:
            if (biquad_coeff_idx < (N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES - 1))
            {
                biquad_coeff_idx++;
            }
            break;
        default:
            break;
    }
    put_coeff_to_slider(&biquad_slider, biquad_coeffs[biquad_channel][biquad_coeff_idx]);

}

static void print_biquad_info(void)
{
    uint32_t x_size, y_size;
    char str[32];

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    GUI_FillRect(0, y_size - 40, x_size, 40, GUI_COLOR_BLACK);
    snprintf(&str[0], sizeof(str), "ch: %ld, idx: %ld, stages: %ld", biquad_channel,
            biquad_coeff_idx, biquad_stages[biquad_channel]);
    GUI_DisplayStringAt(0, y_size - 32, (uint8_t*) &str[0], CENTER_MODE);
}

static void save_biquad_coeff(void)
{
    float coeff = get_coeff_from_slider(&biquad_slider);
    biquad_coeffs[biquad_channel][biquad_coeff_idx] = coeff;
}

static void update_biquad_coeff(void)
{
    dsp_update_mask = 0x04U << biquad_channel;
    logg(LOG_INF, "BIQUAD %ld channel update", biquad_channel);
}

static void force_coeffs_in_range(void)
{
    int32_t n_reset = 0;
    for (int32_t ch = 0; ch < 2; ++ch)
    {
        for (int32_t idx = 0; idx < N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES; ++idx)
        {
            float coeff = biquad_coeffs[ch][idx];
            if ((coeff < BIQUAD_COEFF_MIN) || (coeff > BIQUAD_COEFF_MAX))
            {
                biquad_coeffs[ch][idx] = 0.F;
                ++n_reset;
            }
        }

    }
    update_biquad_coeff();
    logg(LOG_INF, "Reset %ld BIQUAD out-of-range coeffs", n_reset);
}

