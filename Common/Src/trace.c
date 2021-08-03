#include "trace.h"
#include "stm32h7xx_hal.h"

void trace_init()
{
    GPIO_InitTypeDef gpio_init =
    {
        .Pin = TRACE_PIN_MASK,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH
    };
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_Init(TRACE_PORT, &gpio_init);
    HAL_GPIO_WritePin(TRACE_PORT, TRACE_PIN_MASK, GPIO_PIN_RESET);
}

