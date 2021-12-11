#ifndef IS42S32800J_CONF_H
#define IS42S32800J_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#define REFRESH_COUNT                   ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz SD clock) */

#define IS42S32800J_TIMEOUT             ((uint32_t)0xFFFF)

#ifdef __cplusplus
}
#endif

#endif /* IS42S32800J_CONF_H */

