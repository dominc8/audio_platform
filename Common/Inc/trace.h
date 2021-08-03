#ifndef TRACE_H
#define TRACE_H

#include "stm32h7xx.h"

#define N_TRACE                     4
#define TRACE_PIN_MASK              (3U << 8) | (3U << 14)
#define TRACE_PORT                  GPIOB

#define TRACE_START0                (GPIOB->BSRR = (1U << 8))
#define TRACE_STOP0                 (GPIOB->BSRR = (1U << 24))
#define TRACE_START1                (GPIOB->BSRR = (1U << 9))
#define TRACE_STOP1                 (GPIOB->BSRR = (1U << 25))
#define TRACE_START2                (GPIOB->BSRR = (1U << 14))
#define TRACE_STOP2                 (GPIOB->BSRR = (1U << 30))
#define TRACE_START3                (GPIOB->BSRR = (1U << 15))
#define TRACE_STOP3                 (GPIOB->BSRR = (1U << 31))

void trace_init();

#endif /* TRACE_H */

