#ifndef UI_STATES_H
#define UI_STATES_H

#include "stdint.h"
#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_ts.h"

typedef enum UI_STATE
{
    UI_STATE_START_SCREEN = 0, UI_STATE_AUDIO_VISUALIZATION, UI_STATE_FIR_ADJ, UI_STATE_N
} UI_STATE;

typedef struct ui_state_t
{
    UI_STATE (*f_handle_ui)(struct ui_state_t *self, const TS_MultiTouch_State_t *touch_state,
            int32_t button_state, JOYPin_TypeDef joy_pin);
} ui_state_t;

void init_ui_start_screen(ui_state_t *ui_state);
void init_ui_fft(ui_state_t *ui_state);
void init_ui_fir_adj(ui_state_t *ui_state);
void init_ui_biquad_adj(ui_state_t *ui_state);

#endif /* UI_STATES_H */
