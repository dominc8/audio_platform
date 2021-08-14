#ifndef ANALOG_INOUT_H
#define ANALOG_INOUT_H

#include <stdint.h>

extern volatile uint8_t start_low_latency;
void analog_inout(void);

#endif /* ANALOG_INOUT_H */

