#include "fir.h"
#include "dsp_utils.h"

int32_t fir_f32(fir_f32_t *f, int32_t in)
{
    float out = 0.f;
    float *coeff_ptr = &f->coeff[0];
    float *sample_ptr = &f->samples[0];
    uint32_t n = f->order;
    float a;
    float s;

    for (; n != 0; n--, ++coeff_ptr, ++sample_ptr)
    {
        a = *coeff_ptr;
        s = *(sample_ptr + 1);
        out += a * s;
        *sample_ptr = s;
    }
    a = *coeff_ptr;
    *sample_ptr = in;
    out += a * in;

    return out;
}

int32_t fir_q31(fir_q31_t *f, int32_t in)
{
    int32_t out = 0;
    int32_t *coeff_ptr = &f->coeff[0];
    int32_t *sample_ptr = &f->samples[0];
    uint32_t n = f->order;
    int32_t a;
    int32_t s;

    for (; n != 0; --n, ++coeff_ptr, ++sample_ptr)
    {
        a = *coeff_ptr;
        s = *(sample_ptr + 1);
        out = smmlar(a, s, out);

        *sample_ptr = s;
    }
    a = *coeff_ptr;
    *sample_ptr = in;
    out = smmlar(a, in, out);

    return out << 1;

}

