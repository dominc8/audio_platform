#include "ui_utils.h"
#include "logger.h"

int32_t is_ui_button_touched(const ui_button_t *b, const TS_MultiTouch_State_t *state)
{
    int32_t ret = 0;
    for (int32_t i = 0; i < state->TouchDetected; ++i)
    {
        int32_t x = state->TouchX[i];
        int32_t y = state->TouchY[i];
        logg(LOG_DBG, "ui_button (%d %d), touch_evt[%d]: (%d %d)", b->x0, b->y0, i, x, y);
        if ((x >= b->x0) && (x <= b->x1) && (y >= b->y0) && (y <= b->y1))
        {
            ret = 1;
        }
    }
    return ret;
}

void draw_ui_button(const ui_button_t *b)
{
    GUI_FillRect(b->x0, b->y0, b->x1 - b->x0, b->y1 - b->y0, b->color);

    if ((b->font != NULL) && (b->text != NULL))
    {
        sFONT *font = GUI_GetFont();
        uint32_t back_color = GUI_GetBackColor();
        GUI_SetBackColor(b->color);
        GUI_SetFont(b->font);
        GUI_DisplayStringAt(b->x0_text, b->y0_text, b->text, LEFT_MODE);
        GUI_SetFont(font);
        GUI_SetBackColor(back_color);
    }
}

void draw_ui_slider(const ui_slider_t *s)
{
    GUI_FillRect(s->x0, s->y0, s->width, s->height, s->color);
    if (s->print_val != NULL)
    {
        s->print_val(s);
    }
}

int32_t update_ui_slider(ui_slider_t *s, const TS_MultiTouch_State_t *state)
{
    int32_t ret = 0;
    for (int32_t i = 0; i < state->TouchDetected; ++i)
    {
        int32_t x = state->TouchX[i];
        int32_t y = state->TouchY[i];
        if ((x >= s->x0) && (x <= (s->x0 + s->width)) && (y >= s->y0) && (y <= (s->y0 + s->height)))
        {
            logg(LOG_DBG, "ui_slider (%d %d), touch_evt[%d]: (%d %d)", s->x0, s->y0, i, x, y);
            ret = 1;
            s->val = x - s->x0;
        }
    }
    return ret;
}
