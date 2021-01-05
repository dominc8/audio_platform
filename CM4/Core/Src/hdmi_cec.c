/**
  ******************************************************************************
  * File Name          : HDMI_CEC.c
  * Description        : This file provides code for the configuration
  *                      of the HDMI_CEC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hdmi_cec.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CEC_HandleTypeDef hcec;
uint8_t cec_receive_buffer[16];

/* HDMI_CEC init function */
void MX_HDMI_CEC_Init(void)
{

  hcec.Instance = CEC;
  hcec.Init.SignalFreeTime = CEC_DEFAULT_SFT;
  hcec.Init.Tolerance = CEC_STANDARD_TOLERANCE;
  hcec.Init.BRERxStop = CEC_RX_STOP_ON_BRE;
  hcec.Init.BREErrorBitGen = CEC_BRE_ERRORBIT_NO_GENERATION;
  hcec.Init.LBPEErrorBitGen = CEC_LBPE_ERRORBIT_NO_GENERATION;
  hcec.Init.BroadcastMsgNoErrorBitGen = CEC_BROADCASTERROR_ERRORBIT_GENERATION;
  hcec.Init.SignalFreeTimeOption = CEC_SFT_START_ON_TXSOM;
  hcec.Init.ListenMode = CEC_FULL_LISTENING_MODE;
  hcec.Init.OwnAddress = CEC_OWN_ADDRESS_NONE;
  hcec.Init.RxBuffer = cec_receive_buffer;
  if (HAL_CEC_Init(&hcec) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CEC_MspInit(CEC_HandleTypeDef* cecHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(cecHandle->Instance==CEC)
  {
  /* USER CODE BEGIN CEC_MspInit 0 */

  /* USER CODE END CEC_MspInit 0 */
    /* CEC clock enable */
    __HAL_RCC_CEC_CLK_ENABLE();
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**HDMI_CEC GPIO Configuration    
    PB6     ------> CEC 
    */
    GPIO_InitStruct.Pin = HDMI_CEC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_CEC;
    HAL_GPIO_Init(HDMI_CEC_GPIO_Port, &GPIO_InitStruct);

    /* CEC interrupt Init */
    HAL_NVIC_SetPriority(CEC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CEC_IRQn);
  /* USER CODE BEGIN CEC_MspInit 1 */

  /* USER CODE END CEC_MspInit 1 */
  }
}

void HAL_CEC_MspDeInit(CEC_HandleTypeDef* cecHandle)
{

  if(cecHandle->Instance==CEC)
  {
  /* USER CODE BEGIN CEC_MspDeInit 0 */

  /* USER CODE END CEC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CEC_CLK_DISABLE();
  
    /**HDMI_CEC GPIO Configuration    
    PB6     ------> CEC 
    */
    HAL_GPIO_DeInit(HDMI_CEC_GPIO_Port, HDMI_CEC_Pin);

    /* CEC interrupt Deinit */
    HAL_NVIC_DisableIRQ(CEC_IRQn);
  /* USER CODE BEGIN CEC_MspDeInit 1 */

  /* USER CODE END CEC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
