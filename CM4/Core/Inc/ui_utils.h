#ifndef UI_UTILS_H
#define UI_UTILS_H

#include "stm32h747i_discovery_ts.h"
#include "basic_gui.h"

typedef struct ui_button_t
{
    int16_t x0, y0;
    int16_t x1, y1;
    uint32_t color;
    int16_t x0_text, y0_text;
    const char *text;
    sFONT *font;
} ui_button_t;

typedef struct ui_slider_t
{
    int16_t x0, y0;
    int16_t width, height;
    uint32_t color;
    int16_t val;
    void (*print_val)(const struct ui_slider_t *s);
} ui_slider_t;

void draw_ui_button(const ui_button_t *b);
void draw_ui_slider(const ui_slider_t *s);
int32_t is_ui_button_touched(const ui_button_t *b, const TS_MultiTouch_State_t *state);
int32_t update_ui_slider(ui_slider_t *s, const TS_MultiTouch_State_t *state);

#endif /* UI_UTILS_H */
