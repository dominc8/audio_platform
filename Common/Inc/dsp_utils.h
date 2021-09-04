#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#include <stdint.h>

#if defined(CORE_CM4) || defined(CORE_CM7)
static inline int32_t smmlar(int32_t in1, int32_t in2, int32_t acc)
{
    int32_t out;
    __asm ("SMMLAR %[r_dest], %[r_in1], %[r_in2], %[r_acc]"
            : [r_dest] "=r" (out)
            : [r_in1] "r" (in1), [r_in2] "r" (in2), [r_acc] "r" (acc)
    );
    return out;
}
static inline int32_t smmulr(int32_t in1, int32_t in2)
{
    int32_t out;
    __asm ("SMMULR %[r_dest], %[r_in1], %[r_in2]"
            : [r_dest] "=r" (out)
            : [r_in1] "r" (in1), [r_in2] "r" (in2)
    );
    return out;
}
#else
static inline int32_t smmlar(int32_t in1, int32_t in2, int32_t acc)
{
    return (int32_t) (((((int64_t) acc) << 32) + ((int64_t) in1 * in2) + 0x80000000LL ) >> 32);
}
static inline int32_t smmulr(int32_t in1, int32_t in2)
{
    return (int32_t) ((((int64_t) in1 * in2) + 0x80000000LL ) >> 32);
}
#endif

#endif /* DSP_UTILS_H */

