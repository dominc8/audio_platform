#include "error_handler.h"
#include "stm32h747i_discovery.h"
#include "stdint.h"

void Error_Handler(void)
{
    Led_TypeDef error_led;
#ifdef CORE_CM7
    error_led = LED_RED;
#endif
#ifdef CORE_CM4
    error_led = LED_ORANGE;
#endif

    while (1)
    {
        for (volatile int32_t i = 0; i < 1000000; ++i)
        {
        }
        BSP_LED_Toggle(error_led);
    }
}
