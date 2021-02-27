/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BLOCK_SIZE            ((uint32_t)128)
#define N_AUDIO_BLOCKS              ((uint32_t)4)
#define AUDIO_BUFFER_SIZE           ((uint32_t)(AUDIO_BLOCK_SIZE * N_AUDIO_BLOCKS))

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static uint16_t audio_buffer_in[AUDIO_BLOCK_SIZE]);
ALIGN_32BYTES(static uint16_t audio_buffer_out[AUDIO_BUFFER_SIZE]);
static int32_t buf_out_idx = 0;

/* Private function prototypes -----------------------------------------------*/
static void setup_gui(void);

/*----------------------------------------------------------------------------*/
void analog_inout_demo(void)
{
    setup_gui();

    BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
    BSP_AUDIO_IN_OUT_Init();

    GUI_DisplayStringAt(0, 190, (uint8_t*) "Start Recording ", CENTER_MODE);
    if (BSP_AUDIO_IN_Record(0, (uint8_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)))
    {
        GUI_DisplayStringAt(0, 240, (uint8_t*) "Record error!", CENTER_MODE);
    }

    if (BSP_AUDIO_OUT_Play(0, (uint8_t*) &audio_buffer_out[0], sizeof(audio_buffer_out)))
    {
        GUI_DisplayStringAt(0, 260, (uint8_t*) "Play error!", CENTER_MODE);
    }

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
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}

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
