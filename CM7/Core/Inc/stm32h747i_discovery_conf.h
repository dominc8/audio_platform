#ifndef STM32H747I_DISCOVERY_CONF_H
#define STM32H747I_DISCOVERY_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

/* Audio codecs defines */
#define USE_AUDIO_CODEC_WM8994              1U

/* IRQ priorities */
#define BSP_AUDIO_OUT_IT_PRIORITY           14U
#define BSP_AUDIO_IN_IT_PRIORITY            15U
#define BSP_SDRAM_IT_PRIORITY               15U

#ifdef __cplusplus
}
#endif

#endif /* STM32H747I_DISCOVERY_CONF_H */

