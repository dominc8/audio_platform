#ifndef FFT_HIST_H
#define FFT_HIST_H

#include "stdint.h"

void fft_16hist(int16_t *out_l, int16_t *out_r, int32_t *in);
void fft_16p(float *out, int32_t *in);

#endif /* FFT_HIST_H */

