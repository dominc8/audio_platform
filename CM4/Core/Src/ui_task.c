#include "ui_task.h"
#include "logger.h"
#include "scheduler.h"
#include "intercore_comm.h"
#include "perf_meas.h"
#include "ui_states.h"

#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_lcd.h"
#include "stm32h747i_discovery_ts.h"
#include "basic_gui.h"

static ui_state_t ui_states[UI_STATE_N];

static TS_MultiTouch_State_t ts_mt_state =
{ 0 };

static volatile int32_t button_state;
static volatile uint32_t joy_pin_pressed = 0;
static JOYPin_TypeDef joy_state;
static JOYPin_TypeDef joy_old_state;

static TS_Init_t hts_init;
static TS_Init_t *hts;
static TS_Gesture_Config_t gesture_conf;

static void button_init(void);
static void joystick_init(void);
static void lcd_init(void);
static void led_init(void);
static void init_ui_states(void);
static void update_joy_state(void);
static int32_t get_button_state(void);

int32_t ui_task_init(void)
{
    button_init();
    joystick_init();
    led_init();
    lcd_init();
    init_ui_states();
    return scheduler_enqueue_task(&ui_task, NULL);
}

int32_t ui_task(void *arg)
{
    static uint32_t last_ccnt;
    uint32_t new_ccnt;
    uint32_t diff;

    do
    {
        new_ccnt = GET_CCNT();
        diff = DIFF_CCNT(last_ccnt, new_ccnt);
    } while (ccnt_to_ms(diff) < 33);
    last_ccnt = new_ccnt;

    static int32_t i = 0;
    if (++i > 10000)
    {
        i = 0;
        logg(LOG_DBG, "UI task");
    }

    ui_state_t *state = (ui_state_t*) arg;
    UI_STATE next_state = UI_STATE_START_SCREEN;
    if (state == NULL)
    {
        logg(LOG_ERR, "UI State == NULL!");
        state = &ui_states[UI_STATE_START_SCREEN];
    }

    while (lock_hsem(HSEM_I2C4))
        ;
    int32_t ts_status = BSP_TS_Get_MultiTouchState(0, &ts_mt_state);
    unlock_hsem(HSEM_I2C4);
    if (ts_status != BSP_ERROR_NONE)
    {
        ts_mt_state.TouchDetected = 0;
    }
    update_joy_state();

    if (state->f_handle_ui != NULL)
    {
        next_state = state->f_handle_ui(state, &ts_mt_state, get_button_state(),
                joy_state & (joy_state ^ joy_old_state));
    }
    joy_old_state = joy_state;
    if (next_state < UI_STATE_START_SCREEN || next_state >= UI_STATE_N)
    {
        next_state = UI_STATE_START_SCREEN;
    }
    return scheduler_enqueue_task(&ui_task, &ui_states[next_state]);
}

static void button_init(void)
{
    button_state = 0;
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);
}

static void joystick_init(void)
{
    BSP_JOY_Init(JOY1, JOY_MODE_EXTI, JOY_ALL);
    joy_pin_pressed = 0;
    joy_state = JOY_NONE;
    joy_old_state = JOY_NONE;
}

static void led_init(void)
{
    /* LED_RED is reserved for ErrorHandler on CM7 */
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_BLUE);
    BSP_LED_Init(LED_ORANGE);
}

static void lcd_init(void)
{
    uint32_t x_size, y_size;
    uint32_t ts_status = BSP_ERROR_NONE;

    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
    GUI_SetFuncDriver(&LCD_Driver);
    GUI_SetFont(&GUI_DEFAULT_FONT);

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    hts = &hts_init;
    hts->Width = x_size;
    hts->Height = y_size;
    hts->Orientation = TS_SWAP_XY | TS_SWAP_Y;
    hts->Accuracy = 5;

    while (lock_hsem(HSEM_I2C4))
        ;

    ts_status = BSP_TS_Init(0, hts);
    ts_status = BSP_TS_GestureConfig(0, &gesture_conf);

    unlock_hsem(HSEM_I2C4);

    if (BSP_ERROR_NONE != ts_status)
    {
        logg(LOG_ERR, "lcd_init()<ts> failed with %d", ts_status);
    }
    logg(LOG_DBG, "lcd resolution: %u %u", x_size, y_size);
}

static void init_ui_states(void)
{
    init_ui_start_screen(&ui_states[UI_STATE_START_SCREEN]);
    init_ui_fft(&ui_states[UI_STATE_AUDIO_VISUALIZATION]);
    init_ui_fir_adj(&ui_states[UI_STATE_FIR_ADJ]);
    init_ui_biquad_adj(&ui_states[UI_STATE_BIQUAD_ADJ]);
    init_ui_benchmark(&ui_states[UI_STATE_BENCHMARK]);
}

static void update_joy_state(void)
{
    joy_state ^= joy_pin_pressed;
    joy_pin_pressed = 0;
}

static int32_t get_button_state(void)
{
    int32_t ret_val = button_state;
    button_state = 0;
    return ret_val;
}

void BSP_PB_Callback(Button_TypeDef button)
{
    if (button == BUTTON_WAKEUP)
    {
        button_state = 1;
    }
}

void BSP_JOY_Callback(JOY_TypeDef JOY, uint32_t JoyPin)
{
    (void) JOY;
    joy_pin_pressed = JoyPin;
}
