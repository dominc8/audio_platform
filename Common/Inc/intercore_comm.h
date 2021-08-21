#ifndef INTERCORE_COMM_H
#define INTERCORE_COMM_H

#include <assert.h>
#include <stdint.h>

typedef enum
{
    M7_IDLE, M7_LOW_LATENCY, M7_DSP_BLOCKING, M7_BENCHMARK, M7_N_STATES
} M7_STATE;

extern volatile M7_STATE m7_state;

typedef enum
{
    HSEM_BOOT = 0, HSEM_LOW_LATENCY, HSEM_DSP_BLOCKING, HSEM_BENCHMARK, HSEM_I2C4, HSEM_N
} HSEM_ID;

static_assert(HSEM_N < 32, "Too many HSEM_ID declared!");

int32_t lock_unlock_hsem(HSEM_ID hsem_id);
int32_t lock_hsem(HSEM_ID hsem_id);
void unlock_hsem(HSEM_ID hsem_id);

void lock_unlock_callback(uint32_t sem_mask);

#endif /* INTERCORE_COMM_H */

