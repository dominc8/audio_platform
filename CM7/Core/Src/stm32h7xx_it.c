#include "stm32h7xx_it.h"
#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_audio.h"
#include "intercore_comm.h"

static MDMA_HandleTypeDef *p_hmdma;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void AUDIO_IN_SAI_PDMx_DMAx_IRQHandler(void)
{
    BSP_AUDIO_IN_IRQHandler(1, AUDIO_IN_DEVICE_DIGITAL_MIC);
}

void DMA2_Stream1_IRQHandler(void)
{
    BSP_AUDIO_OUT_IRQHandler(0);
}

void DMA2_Stream4_IRQHandler(void)
{
    BSP_AUDIO_IN_IRQHandler(0, AUDIO_IN_DEVICE_ANALOG_MIC);
}

void HSEM1_IRQHandler(void)
{
    uint32_t status_reg = HSEM_COMMON->MISR;
    HSEM_COMMON->ICR = (status_reg);
    NVIC_ClearPendingIRQ(HSEM1_IRQn);

    lock_unlock_callback(status_reg);
}

void HSEM2_IRQHandler(void)
{
    // sem_unlock_callback(statusreg);
}

void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(p_hmdma);
}

void set_mdma_handler(void *hmdma)
{
    p_hmdma = hmdma;
}

