#ifndef STM32H7XX_IT_H
#define STM32H7XX_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void AUDIO_IN_SAI_PDMx_DMAx_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream4_IRQHandler(void);
void MDMA_IRQHandler(void);
void HSEM1_IRQHandler(void);
void HSEM2_IRQHandler(void);

#endif /* STM32H7XX_IT_H */

