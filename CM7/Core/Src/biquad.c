#include "biquad.h"

int32_t biquad_f32(biquad_f32_t *f, int32_t in)
{
    float out = 0.f;
    float in_interm = (float) in;
    float *coeff_ptr = &f->coeff[0];
    float *state_ptr = &f->state[0];
    int32_t n = f->n_stage;

    for (int32_t i = 0; i < n; ++i)
    {
        out = *coeff_ptr * in_interm + *state_ptr;
        ++coeff_ptr;
        *state_ptr = *coeff_ptr * in_interm + *(coeff_ptr + 2) * out + *(state_ptr + 1);
        ++coeff_ptr;
        ++state_ptr;
        *state_ptr = *coeff_ptr * in_interm + *(coeff_ptr + 2) * out;
        coeff_ptr += 3;
        ++state_ptr;
        in_interm = out;
    }

    return out;
}

