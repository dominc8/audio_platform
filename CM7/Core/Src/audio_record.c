/**
  ******************************************************************************
  * @file    BSP/CM7/Src/audio_record.c
  * @author  MCD Application Team
  * @brief   This example describes how to use DFSDM HAL API to realize
  *          audio recording.
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

/** @addtogroup BSP_Examples
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Audio frequency */
extern AUDIO_ErrorTypeDef AUDIO_Start(uint32_t audio_start_address, uint32_t audio_file_size);
#define AUDIO_FREQUENCY            16000U
#define AUDIO_IN_PDM_BUFFER_SIZE  (uint32_t)(128*AUDIO_FREQUENCY/16000*2)
#define AUDIO_IN_BUFFER_SIZE  (uint32_t)(256)
#define AUDIO_NB_BLOCKS    ((uint32_t)4)
#define AUDIO_BLOCK_SIZE   ((uint32_t)0xFFFE)
/* Size of the recorder buffer */
#define RECORD_BUFFER_SIZE        4096
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Define record Buf at D3SRAM @0x38000000 since the BDMA for SAI4 use only this memory */
#if defined ( __CC_ARM )  /* !< ARM Compiler */
  ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));

#elif defined ( __ICCARM__ )  /* !< ICCARM Compiler */
  #pragma location=0x38000000
ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]);
#elif defined ( __GNUC__ )  /* !< GNU Compiler */
  ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));
#endif
ALIGN_32BYTES (uint16_t  AudioBuffer[AUDIO_IN_BUFFER_SIZE]);
ALIGN_32BYTES (uint16_t  AudioBuffer2[AUDIO_IN_BUFFER_SIZE]);
ALIGN_32BYTES (uint16_t  testBuffer[32]);
uint32_t VolumeLevel = 100;
uint32_t  InState = 0;
uint32_t  OutState = 0;
// uint16_t playbackBuf[RECORD_BUFFER_SIZE*2];
BSP_AUDIO_Init_t  AudioInInit;
BSP_AUDIO_Init_t  AudioOutInit;
/* Pointer to record_data */
uint32_t playbackPtr;
uint32_t AudioBufferOffset;
/* Private function prototypes -----------------------------------------------*/
typedef enum {
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF,
  BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief Test Audio record.
  *   The main objective of this test is to check the hardware connection of the
  *   Audio peripheral.
  * @param  None
  * @retval None
*/
volatile retvals[5] = {0};
void AudioRecord_demo(void)
{
   uint32_t channel_nbr = 2;

  uint32_t x_size, y_size;

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  /* Clear the LCD */
  GUI_Clear(GUI_COLOR_WHITE);
  /* Set Audio Demo description */
  GUI_FillRect(0, 0, x_size, 90, GUI_COLOR_BLUE);
  GUI_SetTextColor(GUI_COLOR_WHITE);
  GUI_SetBackColor(GUI_COLOR_BLUE);
  GUI_SetFont(&Font24);
  GUI_DisplayStringAt(0, 0, (uint8_t *)"AUDIO RECORD SAI PDM EXAMPLE", CENTER_MODE);
  GUI_SetFont(&Font16);
  GUI_DisplayStringAt(0, 24, (uint8_t *)"Make sure the SW2 is in position PDM ", CENTER_MODE);
  GUI_DisplayStringAt(0, 40,  (uint8_t *)"Press User button for next menu", CENTER_MODE);
  /* Set the LCD Text Color */
  GUI_DrawRect(10, 100, x_size - 20, y_size - 110, GUI_COLOR_BLUE);
  GUI_DrawRect(11, 101, x_size - 22, y_size - 112, GUI_COLOR_BLUE);


  AudioOutInit.Device = AUDIO_OUT_DEVICE_AUTO;
  AudioOutInit.ChannelsNbr = channel_nbr;
  AudioOutInit.SampleRate = AUDIO_FREQUENCY;
  AudioOutInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioOutInit.Volume = VolumeLevel;

  AudioInInit.Device = AUDIO_IN_DEVICE_ANALOG_MIC;
  AudioInInit.ChannelsNbr = channel_nbr;
  AudioInInit.SampleRate = AUDIO_FREQUENCY;
  AudioInInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioInInit.Volume = VolumeLevel;

  BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
  BSP_AUDIO_IN_OUT_Init();

  /* Initialize Audio Recorder with 2 channels to be used */
  // BSP_AUDIO_IN_Init(0, &AudioInInit);
  // BSP_AUDIO_IN_GetState(0, &InState);

  //BSP_AUDIO_OUT_Init(0, &AudioOutInit);

  //BSP_AUDIO_OUT_SetDevice(0, AUDIO_OUT_DEVICE_HEADPHONE);

  /* Start Recording */
  GUI_DisplayStringAt(0, 190, (uint8_t *)"Start Recording ", CENTER_MODE);
  //BSP_AUDIO_IN_RecordPDM(1, (uint8_t*)&recordPDMBuf, 2*AUDIO_IN_PDM_BUFFER_SIZE);
  if (BSP_AUDIO_IN_Record(0, (uint8_t*)&AudioBuffer[0], AUDIO_IN_BUFFER_SIZE*2))
  {
      GUI_DisplayStringAt(0, 240, (uint8_t *)"Record error!", CENTER_MODE);
  }
  // BSP_AUDIO_OUT_SetDevice(0, AUDIO_OUT_DEVICE_HEADPHONE);

  // memset(&testBuffer[0], 0x8a, 32*2);
  //if (BSP_AUDIO_OUT_Play(0, (uint8_t*)&testBuffer[0], 32*2))
  if (BSP_AUDIO_OUT_Play(0, (uint8_t*)&AudioBuffer2[0], AUDIO_IN_BUFFER_SIZE*2))
  {
      GUI_DisplayStringAt(0, 260, (uint8_t *)"Play error!", CENTER_MODE);
  }

  while (1)
  {
    if (CheckForUserInput() > 0)
    {
      ButtonState = 0;
      BSP_AUDIO_OUT_Stop(0);
      BSP_AUDIO_OUT_DeInit(0);
      BSP_AUDIO_IN_Stop(0);
      BSP_AUDIO_IN_DeInit(0);
      break;
    }
  }
  BSP_AUDIO_OUT_Init(0, &AudioOutInit);

  BSP_AUDIO_OUT_SetDevice(0, AUDIO_OUT_DEVICE_HEADPHONE);

  GUI_DisplayStringAt(0, 220, (uint8_t *)"Play the recorded buffer... ", CENTER_MODE);
  BSP_AUDIO_OUT_Play(0, (uint8_t*)&AudioBuffer[0], AUDIO_IN_BUFFER_SIZE*2);
  while (1)
  {
    if (CheckForUserInput() > 0)
    {
      ButtonState = 0;
      BSP_AUDIO_OUT_Stop(0);
      BSP_AUDIO_OUT_DeInit(0);
      BSP_AUDIO_IN_Stop(0);
      BSP_AUDIO_IN_DeInit(0);
      return;
    }
  }
}

/**
  * @brief Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void  BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *)&AudioBuffer[AUDIO_IN_BUFFER_SIZE/2], AUDIO_IN_BUFFER_SIZE*2/2);

    memcpy(&AudioBuffer2[AUDIO_IN_BUFFER_SIZE/2], &AudioBuffer[AUDIO_IN_BUFFER_SIZE/2], AUDIO_IN_BUFFER_SIZE*2/2);

    SCB_CleanDCache_by_Addr((uint32_t*)&AudioBuffer2[AUDIO_IN_BUFFER_SIZE/2], AUDIO_IN_BUFFER_SIZE*2/2);
    if(Instance == 1U)
  {
    /* Invalidate Data Cache to get the updated content of the SRAM*/
#if 0
    SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE/2], AUDIO_IN_PDM_BUFFER_SIZE*2);

    BSP_AUDIO_IN_PDMToPCM(Instance, (uint16_t*)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE/2], &RecPlayback[playbackPtr]);

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)&RecPlayback[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE/4);

    playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE/4/2;
    if(playbackPtr >= RECORD_BUFFER_SIZE)
      playbackPtr = 0;
#endif
  }
  else
  {
    AudioBufferOffset = BUFFER_OFFSET_FULL;
  }

}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *)&AudioBuffer[0], AUDIO_IN_BUFFER_SIZE*2/2);

    memcpy(&AudioBuffer2[0], &AudioBuffer[0], AUDIO_IN_BUFFER_SIZE*2/2);

    SCB_CleanDCache_by_Addr((uint32_t*)&AudioBuffer2[0], AUDIO_IN_BUFFER_SIZE*2/2);
    if(Instance == 1U)
  {
#if 0
    /* Invalidate Data Cache to get the updated content of the SRAM*/
    SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[0], AUDIO_IN_PDM_BUFFER_SIZE*2);

    BSP_AUDIO_IN_PDMToPCM(Instance, (uint16_t*)&recordPDMBuf[0], &RecPlayback[playbackPtr]);

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)&RecPlayback[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE/4);

    playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE/4/2;
    if(playbackPtr >= RECORD_BUFFER_SIZE)
    {
      playbackPtr = 0;
    }
#endif
  }
  else
  {
    AudioBufferOffset = BUFFER_OFFSET_HALF;
  }

}

/**
  * @brief  Audio IN Error callback function
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
  /* Stop the program with an infinite loop */
  Error_Handler();
}
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
