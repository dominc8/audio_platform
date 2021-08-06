#include "shared_data.h"

SHARED(volatile uint16_t shared_fft_l[SHARED_FFT_SIZE]);
SHARED(volatile uint16_t shared_fft_r[SHARED_FFT_SIZE]);
SHARED(volatile uint16_t start_audio);
SHARED(volatile int32_t new_data_flag);
SHARED_A32(volatile uint8_t m7_eq_buf[M7_EQ_BUF_SIZE]);
SHARED(volatile float fir_coeffs[2][MAX_FIR_ORDER + 1]);
SHARED(volatile int32_t fir_orders[2]);
SHARED(volatile float biquad_coeffs[2][N_COEFF_IN_STAGE * MAX_BIQUAD_STAGES]);
SHARED(volatile int32_t biquad_stages[2]);
SHARED(volatile uint32_t dsp_update_mask);
