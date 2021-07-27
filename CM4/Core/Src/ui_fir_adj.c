#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "intercore_comm.h"
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
    if (coeff < -1.f || coeff > 1.f)
    {
        logg(LOG_WRN, "coeff is out of range (%f), changing to 0.f", coeff);
        coeff = 0.f;
    }
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
    GUI_DisplayStringAt(x0, y0, &str[0], LEFT_MODE);
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
        update_fir_coeff();
        debug_fir_coeff();
        // signalize updated fir coeff
    }
    else
    {
        update_ui_slider(&fir_slider, touch_state);
        draw_ui_slider(&fir_slider);

        // handle fir adjustment
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
    char str[20];

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    GUI_FillRect(0, y_size - 40, x_size, 40, GUI_COLOR_BLACK);
    snprintf(&str[0], sizeof(str), "ch: %ld, idx: %ld", fir_channel, fir_coeff_idx);
    GUI_DisplayStringAt(0, y_size - 32, (uint8_t*) &str[0], CENTER_MODE);

}

static void save_fir_coeff(void)
{
    float coeff = get_coeff_from_slider(&fir_slider);
    fir_coeffs[fir_channel][fir_coeff_idx] = coeff;
}

static void update_fir_coeff(void)
{
    if (0 != lock_unlock_hsem(HSEM_FIR_UPDATE))
    {
        logg(LOG_WRN, "HSEM FIR update failed");
    }
    else
    {
        logg(LOG_INF, "HSEM FIR update succedeed");
    }
}
