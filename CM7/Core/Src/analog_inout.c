/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shared_data.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)128)
#define N_AUDIO_BLOCKS              ((uint32_t)4)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static uint16_t audio_buffer_in[AUDIO_BLOCK_SIZE]);
ALIGN_32BYTES(static uint16_t audio_buffer_out[AUDIO_BUFFER_SIZE]);
static int32_t buf_out_idx = 0;
volatile static int32_t new_data_flag = 0;

/* Private function prototypes -----------------------------------------------*/
static void setup_gui(void);
static void display_data(void);

/*----------------------------------------------------------------------------*/
void analog_inout_demo(void)
{
    setup_gui();

    BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
    BSP_AUDIO_IN_OUT_Init();

    GUI_DisplayStringAt(0, 40, (uint8_t*) "Start Demo", CENTER_MODE);

    if (BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)))
    {
        GUI_DisplayStringAt(0, 60, (uint8_t*) "Record error!", CENTER_MODE);
    }

    if (BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out)))
    {
        GUI_DisplayStringAt(0, 70, (uint8_t*) "Play error!", CENTER_MODE);
    }

    GUI_DisplayStringAt(0, 70, (uint8_t *)&shared_audio_data[0], LEFT_MODE);

    while (1)
    {
        if (CheckForUserInput() > 0)
        {
            ButtonState = 0;
            BSP_AUDIO_OUT_Stop(0);
            BSP_AUDIO_OUT_DeInit(0);
            BSP_AUDIO_IN_Stop(0);
            BSP_AUDIO_IN_DeInit(0);
            break;
        }
        else
        {
            if(new_data_flag > 0)
            {
                display_data();
                new_data_flag = 0;
            }
        }
    }
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_out_idx], sizeof(audio_buffer_in)/2);

    buf_out_idx += AUDIO_BLOCK_SIZE/2;
}


void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    UNUSED(Instance);
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_out_idx],
            sizeof(audio_buffer_in)/2);

    buf_out_idx += AUDIO_BLOCK_SIZE/2;
    buf_out_idx %= AUDIO_BUFFER_SIZE;
    if (buf_out_idx == 0)
        ++new_data_flag;
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

static inline int32_t limit_val(int32_t val, int32_t val_max)
{
    if (val < 0)
        return 0;
    else if (val > val_max)
        return val_max;
    else
        return val;
}

static void display_data(void)
{
    const int32_t group_size = 4;
    const int32_t n_channels = 2;
    const int32_t n_groups = AUDIO_BUFFER_SIZE/(group_size * n_channels);
    const int32_t x0 = 20;
    const int32_t dx = 10;
    const int32_t y_left = 150;
    const int32_t y_right = 300;
    const int32_t ymax = 128;
    const int32_t ymax_shift = 7;
    const int32_t val_shift = 13 - ymax_shift; // theoretically should be 16..18 - ymax_shift


    for (int32_t i = 0; i < n_groups; ++i)
    {
        int32_t val_left = 0;
        int32_t val_right = 0;
        int32_t idx = i*group_size;;
        for (int32_t j = 0; j < group_size; ++j)
        {
            val_left += (int16_t)audio_buffer_out[idx];
            val_right += (int16_t)audio_buffer_out[idx+1];
            idx += n_channels;
        }
        val_left = limit_val(val_left>>(val_shift), ymax);
        val_right = limit_val(val_right>>(val_shift), ymax);

        GUI_FillRect(x0+i*dx, y_left+ymax-val_left, dx, val_left, GUI_COLOR_GREEN);
        GUI_FillRect(x0+i*dx, y_left, dx, ymax-val_left, GUI_COLOR_WHITE);
        GUI_FillRect(x0+i*dx, y_right+ymax-val_right, dx, val_right, GUI_COLOR_GREEN);
        GUI_FillRect(x0+i*dx, y_right, dx, ymax-val_right, GUI_COLOR_WHITE);
    }
}
