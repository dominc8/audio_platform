#ifndef LOW_LATENCY_H
#define LOW_LATENCY_H

#include <stdint.h>

extern volatile uint8_t start_low_latency;

void low_latency(void);

#endif /* LOW_LATENCY_H */

