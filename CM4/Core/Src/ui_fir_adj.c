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
    coeff = coeff / s->width;
    return coeff;
}

static void put_coeff_to_slider(ui_slider_t *s, float coeff)
{
    coeff = coeff * s->width + s->width;
    s->val = coeff / 2;
}

static void print_fir_slider_val(const ui_slider_t *s)
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

static void debug_fir_coeff(void)
{
    logg(LOG_INF, "FIR coeffs:");
    for (int32_t ch = 0; ch < 2; ++ch)
    {
        for (int32_t i = 0; i <= MAX_FIR_ORDER; ++i)
        {
            logg(LOG_INF, "[%d][%d]: %.4f", ch, i, fir_coeffs[ch][i]);
        }
    }
}

static ui_slider_t fir_slider;
static ui_button_t fir_order_up;
static ui_button_t fir_order_down;
static ui_button_t fir_force_coeffs;

static int32_t fir_coeff_idx;
static int32_t fir_channel;

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);

static void handle_joy(JOYPin_TypeDef joy_pin);
static void save_fir_coeff(void);
static void update_fir_coeff(void);
static void print_fir_info(void);
static void force_coeffs_in_range(void);

void init_ui_fir_adj(ui_state_t *ui_state)
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
        logg(LOG_DBG, "handle_ui in ui_fir_adj");
    }
    UI_STATE next_state = UI_STATE_FIR_ADJ;
    if (joy_pin != JOY_NONE)
    {
        handle_joy(joy_pin);
        print_fir_info();
    }
    else if (button_state == 1)
    {
        init_ui_fir_adj(self);
        next_state = UI_STATE_AUDIO_VISUALIZATION;
        debug_fir_coeff();
    }
    else
    {
        update_ui_slider(&fir_slider, touch_state);
        draw_ui_slider(&fir_slider);

        if (is_ui_button_touched(&fir_order_up, touch_state) == 1)
        {
            if (fir_orders[fir_channel] < MAX_FIR_ORDER)
            {
                fir_orders[fir_channel]++;
            }
            print_fir_info();
        }
        if (is_ui_button_touched(&fir_order_down, touch_state) == 1)
        {
            if (fir_orders[fir_channel] > 0)
            {
                fir_orders[fir_channel]--;
            }
            print_fir_info();
        }
        if (is_ui_button_touched(&fir_force_coeffs, touch_state) == 1)
        {
            force_coeffs_in_range();
            print_fir_info();
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
    GUI_DisplayStringAt(0, 10, (uint8_t*) "FIR Adjustment", CENTER_MODE);

    fir_channel = 0;
    fir_coeff_idx = 0;

    fir_slider.x0 = 10;
    fir_slider.y0 = y_size / 2;
    fir_slider.width = (x_size - 20) & 0x00000FFE;
    fir_slider.height = 40;
    fir_slider.color = GUI_COLOR_GRAY;
    fir_slider.print_val = &print_fir_slider_val;
    put_coeff_to_slider(&fir_slider, fir_coeffs[0][0]);
    draw_ui_slider(&fir_slider);

    print_fir_info();

    fir_order_up.x0 = 3 * x_size / 4;
    fir_order_up.y0 = y_size / 8;
    fir_order_up.x1 = x_size;
    fir_order_up.y1 = 3 * y_size / 8;
    fir_order_up.text = "Order++";
    fir_order_up.font = &Font24;
    fir_order_up.color = 0xFF19784E;
    fir_order_down.x0 = 0;
    fir_order_down.y0 = y_size / 8;
    fir_order_down.x1 = x_size / 4;
    fir_order_down.y1 = 3 * y_size / 8;
    fir_order_down.text = "Order--";
    fir_order_down.font = &Font24;
    fir_order_down.color = 0xFFAF2F44;
    fir_force_coeffs.x0 = 3 * x_size / 8;
    fir_force_coeffs.y0 = y_size / 8;
    fir_force_coeffs.x1 = 5 * x_size / 8;
    fir_force_coeffs.y1 = 3 * y_size / 8;
    fir_force_coeffs.text = "Fix coeffs";
    fir_force_coeffs.font = &Font24;
    fir_force_coeffs.color = 0xFF2E89FF;
    set_x0_text_centered(&fir_order_up);
    set_y0_text_centered(&fir_order_up);
    set_x0_text_centered(&fir_order_down);
    set_y0_text_centered(&fir_order_down);
    set_x0_text_centered(&fir_force_coeffs);
    set_y0_text_centered(&fir_force_coeffs);
    draw_ui_button(&fir_order_up);
    draw_ui_button(&fir_order_down);
    draw_ui_button(&fir_force_coeffs);

    BSP_LED_Off(LED_ORANGE);
    BSP_LED_On(LED_BLUE);

    self->f_handle_ui = &handle_ui;
    logg(LOG_DBG, "handle_ui_init in ui_fir_adj");
    return UI_STATE_FIR_ADJ;
}

static void handle_joy(JOYPin_TypeDef joy_pin)
{
    switch (joy_pin)
    {
        case JOY_SEL:
            save_fir_coeff();
            update_fir_coeff();
            break;
        case JOY_DOWN:
            fir_channel = 0;
            break;
        case JOY_UP:
            fir_channel = 1;
            break;
        case JOY_LEFT:
            if (fir_coeff_idx > 0)
            {
                fir_coeff_idx--;
            }
            break;
        case JOY_RIGHT:
            if (fir_coeff_idx < MAX_FIR_ORDER)
            {
                fir_coeff_idx++;
            }
            break;
        default:
            break;
    }
    put_coeff_to_slider(&fir_slider, fir_coeffs[fir_channel][fir_coeff_idx]);

}

static void print_fir_info(void)
{
    uint32_t x_size, y_size;
    char str[32];

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    GUI_FillRect(0, y_size - 40, x_size, 40, GUI_COLOR_BLACK);
    snprintf(&str[0], sizeof(str), "ch: %ld, idx: %ld, order: %ld", fir_channel, fir_coeff_idx,
            fir_orders[fir_channel]);
    GUI_DisplayStringAt(0, y_size - 32, (uint8_t*) &str[0], CENTER_MODE);
}

static void save_fir_coeff(void)
{
    float coeff = get_coeff_from_slider(&fir_slider);
    fir_coeffs[fir_channel][fir_coeff_idx] = coeff;
}

static void update_fir_coeff(void)
{
    dsp_update_mask = 0x01U << fir_channel;
    logg(LOG_INF, "FIR %ld channel update", fir_channel);
}

static void force_coeffs_in_range(void)
{
    int32_t n_reset = 0;
    for (int32_t ch = 0; ch < 2; ++ch)
    {
        for (int32_t idx = 0; idx <= MAX_FIR_ORDER; ++idx)
        {
            float coeff = fir_coeffs[ch][idx];
            if ((coeff < FIR_COEFF_MIN) || (coeff > FIR_COEFF_MAX))
            {
                fir_coeffs[ch][idx] = 0.F;
                ++n_reset;
            }
        }

    }
    update_fir_coeff();
    logg(LOG_INF, "Reset %ld FIR out-of-range coeffs", n_reset);
}
