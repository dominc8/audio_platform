/**
  ******************************************************************************
  * @file    BSP/CM7/Src/joystick.c
  * @author  MCD Application Team
  * @brief   This example code shows how to use the joystick feature in the
  *          stm32h747i_discovery driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HEADBAND_HEIGHT                 80
 __IO uint32_t JoyPinPressed = 0;
__IO uint32_t Joy_State ;
__IO uint32_t PreviousPinState=0;
  uint32_t x_size, y_size;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint32_t JoyStickDemo = 0;
/* Private function prototypes -----------------------------------------------*/
static void Joystick_SetHint(void);
static void Joystick_SetCursorPosition(void);
/* Private functions ---------------------------------------------------------*/

/**
* @brief  Joystick demo
* @param  None
* @retval None
*/
void Joystick_demo (void)
{
  JoyStickDemo = 1;
  Joystick_SetHint();
  BSP_JOY_Init(JOY1, JOY_MODE_EXTI,JOY_ALL);

   JoyPinPressed = 0;
  while (1)
  {
          switch(JoyPinPressed)
      {
      case 0x01U:
        Joy_State = JOY_SEL;
        break;

      case 0x02U:
        Joy_State = JOY_DOWN;
        break;

      case 0x04U:
        Joy_State = JOY_LEFT;
        break;

      case 0x08U:
        Joy_State = JOY_RIGHT;
        break;

      case 0x10U:
        Joy_State = JOY_UP;
        break;
      default:
        Joy_State = JOY_NONE;
        break;
      }
    Joystick_SetCursorPosition();
    JoyPinPressed = 0;
    if(CheckForUserInput() > 0)
    {

      BSP_JOY_DeInit(JOY1, JOY_ALL);
      ButtonState = 0;
      JoyStickDemo = 0;
      return;
    }
     HAL_Delay(6);
  }
}

void BSP_JOY_Callback(JOY_TypeDef JOY, uint32_t JoyPin)
{
    JoyPinPressed = JoyPin;
}
/**
* @brief  Joystick cursor position
* @param  None
* @retval None
*/
static void Joystick_SetCursorPosition()
{
  static uint16_t xPtr = 12;
  static uint16_t yPtr = HEADBAND_HEIGHT + 12;
  static uint16_t old_xPtr = 12;
  static uint16_t old_yPtr = HEADBAND_HEIGHT + 12;


  switch(Joy_State)
  {
  case JOY_UP:
    if(yPtr > HEADBAND_HEIGHT + 12)
    {
      yPtr--;
    }
    break;
  case JOY_DOWN:
    if(yPtr < (y_size - 12 - 11))
    {
      yPtr++;
    }
    break;
  case JOY_LEFT:
    if(xPtr > 12)
    {
      xPtr--;
    }
    break;
  case JOY_RIGHT:
    if(xPtr < (y_size - 8 - 11))
    {
      xPtr++;
    }
    break;
  default:

    break;
  }

  GUI_SetBackColor(GUI_COLOR_WHITE);
  GUI_SetTextColor(GUI_COLOR_BLUE);

  if(Joy_State == JOY_SEL)
  {
    GUI_SetTextColor(GUI_COLOR_RED);
    GUI_DisplayChar(xPtr, yPtr, 'X');

  }
  else if(Joy_State == JOY_NONE)
  {
    GUI_SetTextColor(GUI_COLOR_BLUE);
    GUI_DisplayChar(xPtr, yPtr, 'X');
  }
  else
  {
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_DisplayChar(old_xPtr, old_yPtr, 'X');
    GUI_SetTextColor(GUI_COLOR_BLUE);
    GUI_DisplayChar(xPtr, yPtr, 'X');

    old_xPtr = xPtr;
    old_yPtr = yPtr;
  }
}

/**
* @brief  Display joystick demo hint
* @param  None
* @retval None
*/
static void Joystick_SetHint(void)
{
  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  /* Clear the LCD */
  GUI_Clear(GUI_COLOR_WHITE);

  /* Set Joystick Demo description */
  GUI_FillRect(0, 0, x_size, 80, GUI_COLOR_BLUE);
  GUI_SetTextColor(GUI_COLOR_WHITE);
  GUI_SetBackColor(GUI_COLOR_BLUE);
  GUI_SetFont(&Font24);
  GUI_DisplayStringAt(0, 0, (uint8_t *)"Joystick", CENTER_MODE);
  GUI_SetFont(&Font12);
  GUI_DisplayStringAt(0, 30, (uint8_t *)"Please use the joystick to move", CENTER_MODE);
  GUI_DisplayStringAt(0, 45, (uint8_t *)"the pointer inside the rectangle", CENTER_MODE);
  GUI_DisplayStringAt(0, 60, (uint8_t *)"Press Tamper push-button to switch to next menu", CENTER_MODE);

  /* Set the LCD Text Color */
  GUI_DrawRect(10, 90, x_size - 20, y_size- 100, GUI_COLOR_BLUE);
  GUI_DrawRect(11, 91, x_size - 22, y_size- 102, GUI_COLOR_BLUE);
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
