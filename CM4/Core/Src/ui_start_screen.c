#include "ui_states.h"
#include "basic_gui.h"
#include "stm32h747i_discovery_lcd.h"
#include "agh_logo.h"

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);
static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin);

void init_ui_start_screen(ui_state_t *ui_state)
{
    ui_state->f_handle_ui = &handle_ui_init;
}

static UI_STATE handle_ui(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
{
    UI_STATE next_state = UI_STATE_START_SCREEN;
    if (button_state == 1)
    {
        init_ui_start_screen(self);
        next_state = UI_STATE_AUDIO_VISUALIZATION;
    }
    return next_state;
}

static UI_STATE handle_ui_init(ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
        int32_t button_state, JOYPin_TypeDef joy_pin)
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

    self->f_handle_ui = &handle_ui;
    return UI_STATE_START_SCREEN;
}
