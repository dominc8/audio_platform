#include "shared_data.h"

SHARED(volatile uint16_t shared_audio_data[SHARED_AUDIO_DATA_SIZE]);
SHARED(volatile uint16_t shared_fft_l[SHARED_FFT_SIZE]);
SHARED(volatile uint16_t shared_fft_r[SHARED_FFT_SIZE]);
SHARED(uint16_t start_audio);
SHARED(volatile int32_t new_data_flag);
