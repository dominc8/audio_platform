#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "stdint.h"


/* Exported macro ------------------------------------------------------------*/
#define SHARED(data)  data __attribute__ ((section(".RAM3_SHARED")))

/* Exported define -----------------------------------------------------------*/
#define SHARED_AUDIO_DATA_SIZE      256

/* Exported variables --------------------------------------------------------*/
extern uint16_t shared_audio_data[SHARED_AUDIO_DATA_SIZE];


#endif /* SHARED_DATA_H */

