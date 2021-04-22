#ifndef INTERCORE_COMM_H
#define INTERCORE_COMM_H

#include <assert.h>
#include <stdint.h>

typedef enum HSEM_ID
{
    HSEM_BOOT = 0, HSEM_START_AUDIO, HSEM_I2C4, HSEM_N
} HSEM_ID;

static_assert(HSEM_N < 32, "Too many HSEM_ID declared!");

int32_t lock_unlock_hsem(HSEM_ID hsem_id);
int32_t lock_hsem(HSEM_ID hsem_id);
void unlock_hsem(HSEM_ID hsem_id);

void lock_unlock_callback(uint32_t sem_mask);

#endif /* INTERCORE_COMM_H */
