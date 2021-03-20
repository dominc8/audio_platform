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
static volatile int32_t buf_out_idx = 0;
static volatile int32_t err_cnt;
static volatile int32_t race_cnt;

/* Private function prototypes -----------------------------------------------*/
static inline void fft_4p(float *out, float *in);
static inline void fft_4p_init(float *out, int16_t *in);
static inline void fft_16p(float *out, int16_t *in);
static void fft_16hist(int16_t *out_l, int16_t *out_r, int16_t *in);
static inline int16_t abs_comp(float *x);
static inline void mul_comp(float *out, float *a, float *b);

/*----------------------------------------------------------------------------*/
void analog_inout_demo(void)
{
    int32_t buf_idx = 0;

    BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
    BSP_AUDIO_IN_OUT_Init();
    err_cnt = 0;
    race_cnt = 0;
    buf_out_idx = 0;

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
    BSP_AUDIO_OUT_Stop(0);
    BSP_AUDIO_OUT_DeInit(0);
    BSP_AUDIO_IN_Stop(0);
    BSP_AUDIO_IN_DeInit(0);
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
    SCB_InvalidateDCache_by_Addr((uint32_t*) &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[0], sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx], sizeof(audio_buffer_in)/2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx], sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE/2;
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
            sizeof(audio_buffer_in)/2);

    memcpy(&audio_buffer_out[buf_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            sizeof(audio_buffer_in)/2);
    // memcpy(&shared_audio_data[buf_out_idx], &audio_buffer_in[AUDIO_BLOCK_SIZE / 2],
            // sizeof(audio_buffer_in)/2);

    SCB_CleanDCache_by_Addr((uint32_t*) &audio_buffer_out[buf_idx],
            sizeof(audio_buffer_in)/2);
    // SCB_CleanDCache_by_Addr((uint32_t*) &shared_audio_data[buf_out_idx],
            // sizeof(audio_buffer_in)/2);

    if (buf_idx != buf_out_idx)
    {
        race_cnt++;
    }
    else
    {
        buf_out_idx += AUDIO_BLOCK_SIZE/2;
        buf_out_idx %= AUDIO_BUFFER_SIZE;
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}

static inline void fft_4p_init(float *out, int16_t *in)
{
    int32_t ar = in[0] + in[2];
    int32_t br = in[0] - in[2];
    int32_t cr = in[1] + in[3];
    int32_t di = in[3] - in[1];

    out[0] = ar + cr;
    out[1] = 0;
    out[2] = br;
    out[3] = di;
    out[4] = ar - cr;
    out[5] = 0;
    out[6] = br;
    out[7] = -di;
}

static inline void fft_4p(float *out, float *in)
{
    float ar = in[0] + in[4];
    float ai = in[1] + in[5];
    float br = in[0] - in[4];
    float bi = in[1] - in[5];
    float cr = in[2] + in[6];
    float ci = in[3] + in[7];
    float dr = in[3] - in[7];
    float di = in[6] - in[2];

    out[0] = ar + cr;
    out[1] = ai + ci;
    out[2] = br + dr;
    out[3] = bi + di;
    out[4] = ar - cr;
    out[5] = ai - ci;
    out[6] = br - dr;
    out[7] = bi - di;
}


const float w1[2] = {0.9239f, -0.3827f};
const float w2[2] = {0.7071f, -0.7071f};
const float w3[2] = {0.3827f, -0.9239f};

static inline void fft_16p(float *out, int16_t *in)
{
    float a[8], b[8], c[8], d[8];
    float s[8], t[8], u[8], v[8];
    int16_t tmp[4];

    tmp[0] = in[0];
    tmp[1] = in[4];
    tmp[2] = in[8];
    tmp[3] = in[12];
    fft_4p_init(&a[0], &tmp[0]);
    tmp[0] = in[1];
    tmp[1] = in[5];
    tmp[2] = in[9];
    tmp[3] = in[13];
    fft_4p_init(&b[0], &tmp[0]);
    tmp[0] = in[2];
    tmp[1] = in[6];
    tmp[2] = in[10];
    tmp[3] = in[14];
    fft_4p_init(&c[0], &tmp[0]);
    tmp[0] = in[3];
    tmp[1] = in[7]; tmp[2] = in[11];
    tmp[3] = in[15];
    fft_4p_init(&d[0], &tmp[0]);

    s[0] = a[0];
    s[1] = a[1];
    s[2] = c[0];
    s[3] = c[1];
    s[4] = b[0];
    s[5] = b[1];
    s[6] = d[0];
    s[7] = d[1];

    t[0] = a[2];
    t[1] = a[3];
    t[2] = c[2] * w1[0] - c[3] * w1[1];
    t[3] = c[3] * w1[0] + c[2] * w1[1];
    t[4] = b[2] * w2[0] - b[3] * w2[1];
    t[5] = b[3] * w2[0] + b[2] * w2[1];
    t[6] = d[2] * w3[0] - d[3] * w3[1];
    t[7] = d[3] * w3[0] + d[2] * w3[1];

    u[0] = a[4];
    u[1] = a[5];
    u[2] = c[4] * w2[0] - c[5] * w2[1];
    u[3] = c[5] * w2[0] + c[4] * w2[1];
    u[4] = b[5];
    u[5] = -b[4];
    u[6] = d[5] * w2[0] + d[4] * w2[1];
    u[7] = -d[4] * w2[0] + d[5] * w2[1];

    v[0] = a[6];
    v[1] = a[7];
    v[2] = c[6] * w3[0] - c[7] * w3[1];
    v[3] = c[7] * w3[0] + c[6] * w3[1];
    v[4] = b[5] * w2[0] + b[4] * w2[1];
    v[5] = -b[4] * w2[0] + b[5] * w2[1];
    v[6] = -d[6] * w1[0] + d[7] * w1[1];
    v[7] = -c[7] * w1[0] - c[6] * w1[1];

    fft_4p(&a[0], &s[0]);
    fft_4p(&b[0], &t[0]);
    fft_4p(&c[0], &u[0]);
    fft_4p(&d[0], &v[0]);

    out[0] += a[0];
    out[1] += a[1];
    out[2] += b[0];
    out[3] += b[1];
    out[4] += c[0];
    out[5] += c[1];
    out[6] += d[0];
    out[7] += d[1];

    out[8] += a[2];
    out[9] += a[3];
    out[10] += b[2];
    out[11] += b[3];
    out[12] += c[2];
    out[13] += c[3];
    out[14] += d[2];
    out[15] += d[3];

    out[16] += a[4];
    out[17] += a[5];
    out[18] += b[4];
    out[19] += b[5];
    out[20] += c[4];
    out[21] += c[5];
    out[22] += d[4];
    out[23] += d[5];

    out[24] += a[6];
    out[25] += a[7];
    out[26] += b[6];
    out[27] += b[7];
    out[28] += c[6];
    out[29] += c[7];
    out[30] += d[6];
    out[31] += d[7];
}

static void fft_16hist(int16_t *out_l, int16_t *out_r, int16_t *in)
{
    int16_t left1[16];
    int16_t left2[16];
    int16_t right1[16];
    int16_t right2[16];
    for (int32_t i = 0; i < 16; ++i)
    {
        left1[i] = in[4*i];
        left2[i] = in[4*i+1];
        right1[i] = in[4*i+2];
        right2[i] = in[4*i+3];
    }
    float left[32] = {0};
    fft_16p(&left[0], &left2[0]);

    float temp[2];

    temp[0] = 0.9808f;
    temp[1] = -0.1951f;
    mul_comp(&left[2], &left[2], &temp[0]);
    temp[0] = w1[0];
    temp[1] = w1[1];
    mul_comp(&left[4], &left[4], &temp[0]);
    temp[0] = 0.8315f;
    temp[1] = -0.5556f;
    mul_comp(&left[6], &left[6], &temp[0]);
    temp[0] = w2[0];
    temp[1] = w2[1];
    mul_comp(&left[8], &left[8], &temp[0]);
    temp[0] = 0.5556f;
    temp[1] = -0.8315f;
    mul_comp(&left[10], &left[10], &temp[0]);
    temp[0] = -w1[1];
    temp[1] = -w1[0];
    mul_comp(&left[12], &left[12], &temp[0]);
    temp[0] = 0.1951f;
    temp[1] = -0.9808f;
    mul_comp(&left[14], &left[14], &temp[0]);

    temp[0] = left[16];;
    left[16] = left[17];
    left[17] = temp[0];

    temp[0] = -0.1951f;
    temp[1] = -0.9808f;
    mul_comp(&left[18], &left[18], &temp[0]);
    temp[0] = w1[1];
    temp[1] = -w1[0];
    mul_comp(&left[20], &left[20], &temp[0]);
    temp[0] = -0.5556f;
    temp[1] = -0.8315f;
    mul_comp(&left[22], &left[22], &temp[0]);
    temp[0] = w2[1];
    temp[1] = w2[1];
    mul_comp(&left[24], &left[24], &temp[0]);
    temp[0] = -0.8315f;
    temp[1] = -0.5556f;
    mul_comp(&left[26], &left[26], &temp[0]);
    temp[0] = -w1[0];
    temp[1] = w1[1];
    mul_comp(&left[28], &left[28], &temp[0]);
    temp[0] = -0.9808f;
    temp[1] = -0.1951f;
    mul_comp(&left[30], &left[30], &temp[0]);

    fft_16p(&left[0], &left1[0]);




    float right[32] = {0};
    fft_16p(&right[0], &right2[0]);

    temp[0] = 0.9808f;
    temp[1] = -0.1951f;
    mul_comp(&right[2], &right[2], &temp[0]);
    temp[0] = w1[0];
    temp[1] = w1[1];
    mul_comp(&right[4], &right[4], &temp[0]);
    temp[0] = 0.8315f;
    temp[1] = -0.5556f;
    mul_comp(&right[6], &right[6], &temp[0]);
    temp[0] = w2[0];
    temp[1] = w2[1];
    mul_comp(&right[8], &right[8], &temp[0]);
    temp[0] = 0.5556f;
    temp[1] = -0.8315f;
    mul_comp(&right[10], &right[10], &temp[0]);
    temp[0] = -w1[1];
    temp[1] = -w1[0];
    mul_comp(&right[12], &right[12], &temp[0]);
    temp[0] = 0.1951f;
    temp[1] = -0.9808f;
    mul_comp(&right[14], &right[14], &temp[0]);

    temp[0] = right[16];;
    right[16] = right[17];
    right[17] = temp[0];

    temp[0] = -0.1951f;
    temp[1] = -0.9808f;
    mul_comp(&right[18], &right[18], &temp[0]);
    temp[0] = w1[1];
    temp[1] = -w1[0];
    mul_comp(&right[20], &right[20], &temp[0]);
    temp[0] = -0.5556f;
    temp[1] = -0.8315f;
    mul_comp(&right[22], &right[22], &temp[0]);
    temp[0] = w2[1];
    temp[1] = w2[1];
    mul_comp(&right[24], &right[24], &temp[0]);
    temp[0] = -0.8315f;
    temp[1] = -0.5556f;
    mul_comp(&right[26], &right[26], &temp[0]);
    temp[0] = -w1[0];
    temp[1] = w1[1];
    mul_comp(&right[28], &right[28], &temp[0]);
    temp[0] = -0.9808f;
    temp[1] = -0.1951f;
    mul_comp(&right[30], &right[30], &temp[0]);

    fft_16p(&right[0], &right1[0]);

    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        out_l[i] = abs_comp(&left[2*i]);
    }
    for (int32_t i = 0; i < SHARED_FFT_SIZE; ++i)
    {
        out_r[i] = abs_comp(&right[2*i]);
    }
}

static inline int16_t abs_comp(float *x)
{
    return fabsf(x[0] * x[0] + x[1] * x[1]) / 64;
}

static inline void mul_comp(float *out, float *a, float *b)
{
    out[0] = a[0] * b[0] - a[1] * b[1];
    out[1] = a[0] * b[1] + a[1] * b[0];
}




