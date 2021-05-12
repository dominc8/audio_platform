#include "main.h"
#include "stm32h7xx_it.h"

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

/**
 * @brief  This function handles External lines 15 to 10 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI15_10_IRQHandler(void)
{
    BSP_PB_IRQHandler(BUTTON_WAKEUP);
}

/**
 * @brief  This function handles External line 2 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI2_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_SEL);
}

/**
 * @brief  This function handles External line 3 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI3_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_DOWN);
}

/**
 * @brief  This function handles External line 4 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI4_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_LEFT);
}

/**
 * @brief  This function handles External lines 9 to 5 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI9_5_IRQHandler(void)
{
    BSP_JOY_IRQHandler(JOY1, JOY_RIGHT);
    BSP_JOY_IRQHandler(JOY1, JOY_UP);
}
