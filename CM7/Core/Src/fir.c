#include "fir.h"

int32_t fir_f32(fir_f32_t *f, int32_t in)
{
    float out = 0.f;
    float *coeff_ptr = &f->coeff[0];
    float *sample_ptr = &f->samples[0];
    int32_t n = f->order;

    for (int32_t i = 0; i < n; ++i, ++coeff_ptr, ++sample_ptr)
    {
        out += *coeff_ptr * *(sample_ptr + 1);
        *sample_ptr = *(sample_ptr + 1);
    }
    *sample_ptr = (float) in;
    out += *coeff_ptr * in;

    return out;
}

