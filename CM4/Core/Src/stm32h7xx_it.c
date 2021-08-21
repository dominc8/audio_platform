#include "stm32h7xx_it.h"
#include "stm32h747i_discovery.h"

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

void EXTI15_10_IRQHandler(void)
{
    BSP_PB_IRQHandler(BUTTON_WAKEUP);
}

void EXTI2_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_SEL);
}

void EXTI3_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_DOWN);
}

void EXTI4_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_LEFT);
}

void EXTI9_5_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_RIGHT);
    BSP_JOY_IRQHandler(JOY1, JOY_UP);
}

