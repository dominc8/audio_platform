#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "intercore_comm.h"
#include "ui_utils.h"

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);

void init_ui_benchmark(ui_state_t *ui_state)
{
    ui_state->f_handle_ui = &handle_ui_init;
}

static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    UI_STATE next_state = UI_STATE_BENCHMARK;
    (void) touch_state;
    (void) joy_pin;

    if (1 == button_state)
    {
        self->f_handle_ui = &handle_ui_init;
        next_state = UI_STATE_START_SCREEN;
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

    GUI_FillRect(0, 50, x_size, y_size - 50, GUI_COLOR_BLACK);
    GUI_DisplayStringAt(0, y_size / 2, (uint8_t*) "Running benchmarks", CENTER_MODE);

    self->f_handle_ui = &handle_ui;
    return UI_STATE_BENCHMARK;
}

