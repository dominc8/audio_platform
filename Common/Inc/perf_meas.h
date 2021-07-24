#ifndef PERF_MEAS_H
#define PERF_MEAS_H

#include "stm32h7xx.h"

#define GET_CCNT()                  DWT->CYCCNT
#define DIFF_CCNT(start, stop)      (stop - start)
//(stop > start ? (stop - start) : start - stop)

void ccnt_init();
uint32_t ccnt_to_ms(uint32_t ccnt);
uint32_t ccnt_to_us(uint32_t ccnt);

#endif /* PERF_MEAS_H */
