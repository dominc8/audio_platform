#include "biquad.h"
#include "dsp_utils.h"

int32_t biquad_f32(biquad_f32_t *f, int32_t in)
{
    float out = 0.f;
    float in_interm = (float) in;
    float *coeff_ptr = &f->coeff[0];
    float *state_ptr = &f->state[0];
    uint32_t n = f->n_stage;
    float b0, b1, b2, a1, a2;
    float s1, s2;

    for (; n != 0; --n)
    {
        b0 = *coeff_ptr++;
        b1 = *coeff_ptr++;
        b2 = *coeff_ptr++;
        a1 = *coeff_ptr++;
        a2 = *coeff_ptr++;
        s1 = *state_ptr;
        s2 = *(state_ptr + 1);

        out = b0 * in_interm + s1;
        s1 = b1 * in_interm + a1 * out + s2;
        s2 = b2 * in_interm + a2 * out;
        in_interm = out;

        *state_ptr++ = s1;
        *state_ptr++ = s2;
    }

    return out;
}

int32_t biquad_q31(biquad_q31_t *f, int32_t in)
{
    int32_t out = 0;
    int32_t *coeff_ptr = &f->coeff[0];
    int32_t *state_ptr = &f->state[0];
    uint32_t n = f->n_stage;
    int32_t b0, b1, b2, a1, a2;
    int32_t x1, x2, y1, y2;

    for (; n != 0; --n)
    {
        b0 = *coeff_ptr++;
        b1 = *coeff_ptr++;
        b2 = *coeff_ptr++;
        a1 = *coeff_ptr++;
        a2 = *coeff_ptr++;
        x1 = *state_ptr;
        x2 = *(state_ptr + 1);
        y1 = *(state_ptr + 2);
        y2 = *(state_ptr + 3);

        out = smmulr(b0, in);
        out = smmlar(b1, x1, out);
        out = smmlar(b2, x2, out);
        out = smmlar(a1, y1, out);
        out = smmlar(a2, y2, out);
        out <<= 1;

        *state_ptr++ = in;
        *state_ptr++ = x1;
        *state_ptr++ = out;
        *state_ptr++ = y1;

        in = out;
    }

    return out;
}

