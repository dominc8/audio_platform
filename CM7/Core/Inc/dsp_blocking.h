#ifndef DSP_BLOCKING_H
#define DSP_BLOCKING_H

/* Use this define to copy out buffers from DTCM to AXI_SRAM with MDMA transfer,
 * seems to be comparable or faster for AUDIO_BLOCK_SIZE >= 32 */
#define DSP_MDMA_OUT

void dsp_blocking(void);

#endif /* DSP_BLOCKING_H */

