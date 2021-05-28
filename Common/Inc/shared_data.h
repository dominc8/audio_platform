#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "stdint.h"

/* Exported macro ------------------------------------------------------------*/
#define SHARED(data)  data __attribute__ ((section(".RAM3_SHARED")))
#define SHARED_A32(data)  data __attribute__ ((section(".RAM3_SHARED"))) __attribute__ ((aligned (32)))

/* Exported define -----------------------------------------------------------*/
#define SHARED_AUDIO_DATA_SIZE      (512)
#define SHARED_FFT_SIZE             (16)
#define M7_EQ_BUF_SIZE              (512)

/* Exported variables --------------------------------------------------------*/
extern volatile uint16_t shared_audio_data[SHARED_AUDIO_DATA_SIZE];
extern volatile uint16_t shared_fft_l[SHARED_FFT_SIZE];
extern volatile uint16_t shared_fft_r[SHARED_FFT_SIZE];
extern volatile uint16_t start_audio;
extern volatile int32_t new_data_flag;
extern volatile uint8_t m7_eq_buf[M7_EQ_BUF_SIZE];

#endif /* SHARED_DATA_H */

