#include "perf_meas.h"

extern uint32_t SystemCoreClock;

void ccnt_init()
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t ccnt_to_ms(uint32_t ccnt)
{
    return ccnt / (SystemCoreClock / 1000);
}

uint32_t ccnt_to_us(uint32_t ccnt)
{
    return ccnt / (SystemCoreClock / 1000000);
}
