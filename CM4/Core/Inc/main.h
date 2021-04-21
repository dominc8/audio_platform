#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_lcd.h"
#include "stm32h747i_discovery_ts.h"
#include "stm32h747i_discovery_conf.h"
#include "basic_gui.h"

#define LED_GREEN       LED1
#define LED_ORANGE      LED2
#define LED_RED         LED3
#define LED_BLUE        LED4
extern __IO uint32_t ButtonState;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
