#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "stdint.h"


/* Exported macro ------------------------------------------------------------*/
#define SHARED(data)  data __attribute__ ((section(".RAM3_SHARED")))

/* Exported define -----------------------------------------------------------*/
#define SHARED_AUDIO_DATA_SIZE      (512)
#define HSEM_DATA                   (1U)        /* HW semaphore for signaling new data */

/* Exported variables --------------------------------------------------------*/
extern volatile uint16_t shared_audio_data[SHARED_AUDIO_DATA_SIZE];
extern uint16_t start_audio;
extern volatile int32_t new_data_flag;


#endif /* SHARED_DATA_H */

