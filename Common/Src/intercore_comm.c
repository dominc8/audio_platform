#include "intercore_comm.h"
#include "shared_data.h"
#include "stm32h7xx_hal.h"

#define IS_HSEMID_IN_MASK(_HSEM_ID_, _SEM_MASK_) (_SEM_MASK_ & __HAL_HSEM_SEMID_TO_MASK(_HSEM_ID_))

int32_t lock_unlock_hsem(HSEM_ID hsem_id)
{
    int32_t ret_val = 0;
    if (HAL_OK != HAL_HSEM_FastTake(hsem_id))
    {
        ret_val = 1;
    }
    else
    {
        HAL_HSEM_Release(hsem_id, 0);
    }
    return ret_val;
}

int32_t lock_hsem(HSEM_ID hsem_id)
{
    int32_t ret_val = 0;
    if (HAL_OK != HAL_HSEM_FastTake(hsem_id))
    {
        ret_val = 1;
    }
    return ret_val;
}

void unlock_hsem(HSEM_ID hsem_id)
{
    HAL_HSEM_Release(hsem_id, 0);
}

#ifdef CORE_CM4
void lock_unlock_callback(uint32_t sem_mask)
{
    (void) sem_mask;
}
#endif

#ifdef CORE_CM7

void lock_unlock_callback(uint32_t sem_mask)
{
    if (IS_HSEMID_IN_MASK(HSEM_START_AUDIO, sem_mask))
    {
        start_audio = start_audio == 1 ? 0 : 1;
    }
}
#endif

