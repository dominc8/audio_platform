#ifndef DSP_BLOCKING_H
#define DSP_BLOCKING_H

#include <stdint.h>
/* Use this define to copy out buffers from DTCM to AXI_SRAM with MDMA transfer,
 * seems to be comparable or faster for AUDIO_BLOCK_SIZE >= 32 */
#define DSP_MDMA_OUT

extern volatile uint8_t start_dsp_blocking;

void dsp_blocking(void);

#endif /* DSP_BLOCKING_H */

