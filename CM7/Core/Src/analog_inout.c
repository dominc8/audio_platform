/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shared_data.h"
#include "fft_hist.h"
#include "intercore_comm.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)128)
#define N_AUDIO_BLOCKS              ((uint32_t)4)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static uint16_t audio_buffer_in[AUDIO_BLOCK_SIZE]);
ALIGN_32BYTES(static uint16_t audio_buffer_out[AUDIO_BUFFER_SIZE]);
static volatile int32_t buf_out_idx = 0;
static volatile int32_t err_cnt;
static volatile int32_t race_cnt;

/* Private function prototypes -----------------------------------------------*/

/*----------------------------------------------------------------------------*/
void analog_inout_demo(void)
{
    int32_t buf_idx = 0;

    BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);

    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_IN_OUT_Init();
    unlock_hsem(HSEM_I2C4);

    err_cnt = 0;
    race_cnt = 0;
    buf_out_idx = 0;

    // GUI_DisplayStringAt(0, 40, (uint8_t*) "Start Demo", CENTER_MODE);

    while (lock_hsem(HSEM_I2C4))
        ;
    err_cnt += BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in));
    err_cnt += BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out));
    unlock_hsem(HSEM_I2C4);

    // GUI_DisplayStringAt(0, 70, (uint8_t *)&shared_audio_data[0], LEFT_MODE);

    while (start_audio == 1)
    {
        int32_t new_buf_idx = buf_out_idx;
        // if (buf_idx != new_buf_idx)
        if (0 == new_buf_idx)
        {
            buf_idx = 0;

            fft_16hist(&shared_fft_l[0], &shared_fft_r[0], &audio_buffer_out[buf_idx]);
            SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_l[0], sizeof(shared_fft_l));
            SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_r[0], sizeof(shared_fft_r));

            buf_idx = new_buf_idx;

            ++new_data_flag;
            SCB_CleanDCache_by_Addr((uint32_t*) &new_data_flag, sizeof(new_data_flag));
        }
    }
    while (lock_hsem(HSEM_I2C4))
        ;
    BSP_AUDIO_OUT_Stop(0);
    BSP_AUDIO_OUT_DeInit(0);
    BSP_AUDIO_IN_Stop(0);
    BSP_AUDIO_IN_DeInit(0);
    unlock_hsem(HSEM_I2C4);
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[0], sizeof(audio_buffer_in) / 2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[0], sizeof(audio_buffer_in) / 2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in) / 2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx], sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE / 2;
    }
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        err_cnt = 0;
    }
    int32_t buf_idx = buf_out_idx;
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in) / 2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in) / 2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
    // sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in) / 2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx],
    // sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE / 2;
        buf_out_idx %= AUDIO_BUFFER_SIZE;
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}
