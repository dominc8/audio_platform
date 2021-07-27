#include "shared_data.h"

SHARED(volatile uint16_t shared_audio_data[SHARED_AUDIO_DATA_SIZE]);
SHARED(volatile uint16_t shared_fft_l[SHARED_FFT_SIZE]);
SHARED(volatile uint16_t shared_fft_r[SHARED_FFT_SIZE]);
SHARED(volatile uint16_t start_audio);
SHARED(volatile int32_t new_data_flag);
SHARED_A32(volatile uint8_t m7_eq_buf[M7_EQ_BUF_SIZE]);
SHARED(volatile float fir_left_coeff[MAX_FIR_ORDER + 1]);
SHARED(volatile float fir_right_coeff[MAX_FIR_ORDER + 1]);
SHARED(volatile int32_t fir_left_order);
SHARED(volatile int32_t fir_right_order);
