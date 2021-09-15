#include "fir.h"
#include "dsp_utils.h"

__attribute__(( optimize("-O3") ))
int32_t fir_f32(fir_f32_t *f, int32_t in)
{
    float *coeff_ptr = &f->coeff[0];
    float *sample_ptr = &f->samples[0];
    uint32_t n = f->order;
    float acc1 = 0.f, acc2 = 0.f, acc3 = 0.f, acc4 = 0.f;
    float a1, a2, a3, a4;
    float s1, s2, s3, s4;
    uint32_t n_block = n >> 2;
    n = n & 0x3;

    for (; n_block != 0; n_block--)
    {
        a1 = *coeff_ptr++; // vldr
        a2 = *coeff_ptr++;
        a3 = *coeff_ptr++;
        a4 = *coeff_ptr++;
        s1 = *(sample_ptr + 1);
        s2 = *(sample_ptr + 2);
        s3 = *(sample_ptr + 3);
        s4 = *(sample_ptr + 4);

        acc1 += a1 * s1; // vfma.f32
        acc2 += a2 * s2;
        acc3 += a3 * s3;
        acc4 += a4 * s4;

        *sample_ptr++ = s1; // vstr
        *sample_ptr++ = s2;
        *sample_ptr++ = s3;
        *sample_ptr++ = s4;
    }

    for (; n != 0; n--, ++coeff_ptr, ++sample_ptr)
    {
        a1 = *coeff_ptr;
        s1 = *(sample_ptr + 1);
        acc1 += a1 * s1;
        *sample_ptr = s1;
    }

    acc1 += acc2;
    acc3 += acc4;
    a2 = *coeff_ptr;
    *sample_ptr = in;
    acc3 += a2 * in;
    acc1 += acc3;

    return acc1;
}

__attribute__(( optimize("-O2") ))
int32_t fir_q31(fir_q31_t *f, int32_t in)
{
    int32_t *coeff_ptr = &f->coeff[0];
    int32_t *sample_ptr = &f->samples[0];
    uint32_t n = f->order;
    int32_t a1, a2;
    int32_t s1, s2, s3, s4;
    int32_t acc1 = 0, acc2 = 0;
    uint32_t n_block = n >> 2;
    n = n & 0x3;

    for (; n_block != 0; n_block--)
    {
        a1 = *coeff_ptr++; // ldr
        a2 = *coeff_ptr++;
        s1 = *(sample_ptr + 1);
        s2 = *(sample_ptr + 2);
        s3 = *(sample_ptr + 3);
        s4 = *(sample_ptr + 4);

        acc1 = smmlar(a1, s1, acc1); // smmlar
        acc2 = smmlar(a2, s2, acc2);

        a1 = *coeff_ptr++;
        a2 = *coeff_ptr++;

        acc1 = smmlar(a1, s3, acc1);
        acc2 = smmlar(a2, s4, acc2);

        *sample_ptr++ = s1; // str
        *sample_ptr++ = s2;
        *sample_ptr++ = s3;
        *sample_ptr++ = s4;
    }

    for (; n != 0; --n, ++coeff_ptr, ++sample_ptr)
    {
        a1 = *coeff_ptr;
        s1 = *(sample_ptr + 1);
        acc1 = smmlar(a1, s1, acc1);
        *sample_ptr = s1;
    }

    acc1 += acc2;
    a2 = *coeff_ptr;
    *sample_ptr = in;
    acc1 = smmlar(a2, in, acc1);

    return acc1 << 1;
}

