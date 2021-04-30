#ifndef PERF_MEAS_H
#define PERF_MEAS_H

#include "stm32h7xx.h"

#define GET_CCNT()                  DWT->CYCCNT
#define DIFF_CCNT(start, stop)      (stop > start ? (stop - start) : start - stop)

void init_cycle_counter();

#endif /* PERF_MEAS_H */
