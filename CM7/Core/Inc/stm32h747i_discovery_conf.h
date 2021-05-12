/**
 ******************************************************************************
 * @file    stm32H747i_discovery_conf.h
 * @author  MCD Application Team
 * @brief   STM32H747I_Discovery board configuration file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#ifndef STM32H747I_DISCOVERY_CONF_H
#define STM32H747I_DISCOVERY_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Audio codecs defines */
#define USE_AUDIO_CODEC_WM8994              1U

/* Default Audio IN internal buffer size */
#define DEFAULT_AUDIO_IN_BUFFER_SIZE        64U

/* IRQ priorities */
#define BSP_AUDIO_OUT_IT_PRIORITY           14U
#define BSP_AUDIO_IN_IT_PRIORITY            15U
#define BSP_SDRAM_IT_PRIORITY               15U

#ifdef __cplusplus
}
#endif

#endif /* STM32H747I_DISCOVERY_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
