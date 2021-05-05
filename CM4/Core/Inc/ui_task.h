#ifndef UI_TASK_H
#define UI_TASK_H

#include <stdint.h>

int32_t ui_task_init(void);
int32_t ui_task(void *arg);
int32_t ui_peek_button_state();
int32_t ui_get_button_state();

#endif /* UI_TASK_H */

