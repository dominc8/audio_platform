/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shared_data.h"
#include "arm_math.h"


/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)128)
#define N_AUDIO_BLOCKS              ((uint32_t)4)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static uint16_t audio_buffer_in[AUDIO_BLOCK_SIZE]);
ALIGN_32BYTES(static uint16_t audio_buffer_out[AUDIO_BUFFER_SIZE]);
static int32_t buf_out_idx = 0;
volatile static int32_t err_cnt;

/* Private function prototypes -----------------------------------------------*/
static void setup_gui(void);
static void display_data(void);





extern const arm_rfft_instance_q15 arm_rfft_sR_q15_len32;

/*----------------------------------------------------------------------------*/
void analog_inout_demo(void)
{
    // setup_gui();

    BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
    BSP_AUDIO_IN_OUT_Init();
    err_cnt = 0;

    // GUI_DisplayStringAt(0, 40, (uint8_t*) "Start Demo", CENTER_MODE);

    if (BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)))
    {
        err_cnt = 1;
        // GUI_DisplayStringAt(0, 60, (uint8_t*) "Record error!", CENTER_MODE);
    }

    if (BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out)))
    {
        err_cnt = 2;
        // GUI_DisplayStringAt(0, 70, (uint8_t*) "Play error!", CENTER_MODE);
    }

    // GUI_DisplayStringAt(0, 70, (uint8_t *)&shared_audio_data[0], LEFT_MODE);
    volatile int32_t * volatile flag_ptr = &new_data_flag;

    while (start_audio == 1)
    {
    }
    err_cnt = 0;
    BSP_AUDIO_OUT_Stop(0);
    BSP_AUDIO_OUT_DeInit(0);
    BSP_AUDIO_IN_Stop(0);
    BSP_AUDIO_IN_DeInit(0);
}

static int16_t fft_l[SHARED_FFT_SIZE * 2];
static int16_t fft_r[SHARED_FFT_SIZE * 2];

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        buf_out_idx = 0;
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_l[0], &shared_fft_l[0]);
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_r[0], &shared_fft_r[0]);
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_l[0], sizeof(shared_fft_l));
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_r[0], sizeof(shared_fft_r));
        memset(fft_l, 0, sizeof(fft_l));
        memset(fft_r, 0, sizeof(fft_l));
        memcpy(&shared_fft_l[0], &fft_l[0], sizeof(shared_fft_l));
        memcpy(&shared_fft_r[0], &fft_r[0], sizeof(shared_fft_r));
        ++new_data_flag;
        SCB_CleanDCache_by_Addr((uint32_t*) &new_data_flag, sizeof(new_data_flag));
    }
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    for (int32_t i = 0; i < 2*SHARED_FFT_SIZE; i+=2)
    {
        fft_l[i] += audio_buffer_in[i] >> 2;
        fft_r[i] += audio_buffer_in[i + 1] >> 2;
    }

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_out_idx], sizeof(audio_buffer_in)/2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx], sizeof(audio_buffer_in)/2);

    buf_out_idx += AUDIO_BLOCK_SIZE/2;
}


void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    if (buf_out_idx < 0 || buf_out_idx >= AUDIO_BUFFER_SIZE)
    {
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_l[0], &shared_fft_l[0]);
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_r[0], &shared_fft_r[0]);
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_l[0], sizeof(shared_fft_l));
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_r[0], sizeof(shared_fft_r));
        memset(fft_l, 0, sizeof(fft_l));
        memset(fft_r, 0, sizeof(fft_l));
        memcpy(&shared_fft_l[0], &fft_l[0], sizeof(shared_fft_l));
        memcpy(&shared_fft_r[0], &fft_r[0], sizeof(shared_fft_r));
        ++new_data_flag;
        SCB_CleanDCache_by_Addr((uint32_t*) &new_data_flag, sizeof(new_data_flag));
        buf_out_idx = 0;
    }
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in)/2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            // sizeof(audio_buffer_in)/2);

    for (int32_t i = 0; i < 2*SHARED_FFT_SIZE; i+=2)
    {
        fft_l[i] += audio_buffer_in[i] >> 2;
        fft_r[i] += audio_buffer_in[i + 1] >> 2;
    }

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_out_idx],
            sizeof(audio_buffer_in)/2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx],
            // sizeof(audio_buffer_in)/2);

    buf_out_idx += AUDIO_BLOCK_SIZE/2;
    buf_out_idx %= AUDIO_BUFFER_SIZE;
    if (buf_out_idx == 0)
    {
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_l[0], &shared_fft_l[0]);
        arm_rfft_q15(&arm_rfft_sR_q15_len32, &fft_r[0], &shared_fft_r[0]);
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_l[0], sizeof(shared_fft_l));
        SCB_CleanDCache_by_Addr((uint32_t*) &shared_fft_r[0], sizeof(shared_fft_r));
        memset(fft_l, 0, sizeof(fft_l));
        memset(fft_r, 0, sizeof(fft_l));
        memcpy(&shared_fft_l[0], &fft_l[0], sizeof(shared_fft_l));
        memcpy(&shared_fft_r[0], &fft_r[0], sizeof(shared_fft_r));
        ++new_data_flag;
        SCB_CleanDCache_by_Addr((uint32_t*) &new_data_flag, sizeof(new_data_flag));
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}

/* Private functions ---------------------------------------------------------*/

static void setup_gui(void)
{
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    GUI_Clear(GUI_COLOR_WHITE);

    GUI_FillRect(0, 0, x_size, 90, GUI_COLOR_BLUE);
    GUI_SetTextColor(GUI_COLOR_WHITE);
    GUI_SetBackColor(GUI_COLOR_BLUE);
    GUI_SetFont(&Font24);
    GUI_DisplayStringAt(0, 0, (uint8_t*) "Analog input/output demo", CENTER_MODE);
    GUI_SetFont(&Font16);

    GUI_DrawRect(10, 100, x_size - 20, y_size - 110, GUI_COLOR_BLUE);
    GUI_DrawRect(11, 101, x_size - 22, y_size - 112, GUI_COLOR_BLUE);
}

