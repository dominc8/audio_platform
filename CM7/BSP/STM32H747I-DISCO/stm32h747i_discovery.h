/**
  ******************************************************************************
  * @file    stm32h747i_discovery.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32H747I_DISCO:
  *          LEDs
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H747I_DISCO_H
#define STM32H747I_DISCO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h747i_discovery_conf.h"
#include "stm32h747i_discovery_errno.h"

/* Exported Types ------------------------------------------------------------*/

typedef enum
{
  LED1 = 0U,
  LED_GREEN = LED1,
  LED2 = 1U,
  LED_ORANGE = LED2,
  LED3 = 2U,
  LED_RED = LED3,
  LED4 = 3U,
  LED_BLUE = LED4,
  LEDn
} Led_TypeDef;

/* Exported Constants --------------------------------------------------------*/

#if !defined (USE_STM32H747I_DISCO)
 #define USE_STM32H747I_DISCO
#endif
/**
 * @brief STM32H747I Discovery BSP Driver version number V2.0.0
 */
#define STM32H747I_DISCO_BSP_VERSION_MAIN   (0x02) /*!< [31:24] main version */
#define STM32H747I_DISCO_BSP_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define STM32H747I_DISCO_BSP_VERSION_SUB2   (0x00) /*!< [15:8]  sub2 version */
#define STM32H747I_DISCO_BSP_VERSION_RC     (0x00) /*!< [7:0]  release candidate */
#define STM32H747I_DISCO_BSP_VERSION        ((STM32H747I_DISCO_BSP_VERSION_MAIN << 24)\
                                            |(STM32H747I_DISCO_BSP_VERSION_SUB1 << 16)\
                                            |(STM32H747I_DISCO_BSP_VERSION_SUB2 << 8 )\
                                            |(STM32H747I_DISCO_BSP_VERSION_RC))
#define STM32H747I_DISCO_BSP_BOARD_NAME  "STM32H747I-DISCO";
#define STM32H747I_DISCO_BSP_BOARD_ID    "MB12481D";

#define LED1_GPIO_PORT                   GPIOI
#define LED1_PIN                         GPIO_PIN_12

#define LED2_GPIO_PORT                   GPIOI
#define LED2_PIN                         GPIO_PIN_13

#define LED3_GPIO_PORT                   GPIOI
#define LED3_PIN                         GPIO_PIN_14

#define LED4_GPIO_PORT                   GPIOI
#define LED4_PIN                         GPIO_PIN_15

#define LEDx_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define LEDx_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()

int32_t  BSP_LED_Init(Led_TypeDef Led);
int32_t  BSP_LED_DeInit(Led_TypeDef Led);
int32_t  BSP_LED_On(Led_TypeDef Led);
int32_t  BSP_LED_Off(Led_TypeDef Led);
int32_t  BSP_LED_Toggle(Led_TypeDef Led);
int32_t  BSP_LED_GetState (Led_TypeDef Led);

#ifdef __cplusplus
}
#endif

#endif /* STM32H747I_DISCO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
