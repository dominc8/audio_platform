#include "logger.h"
#include "stm32h747i_discovery.h"
#include <stdio.h>
#include <string.h>


#define BUF_LEN             50
#define LOG_HEADER_LEN      5

static LOG_LEVEL log_level = LOG_DBG;

static char* log_headers[LOG_N] = {
        "ERR: ",
        "WRN: ",
        "INF: ",
        "DBG: "
};

#ifdef CORE_CM4

extern UART_HandleTypeDef hcom_uart[COMn];

int32_t logger_init(uint32_t baudrate)
{
    COM_InitTypeDef COM_Init = { .BaudRate = baudrate,
                                 .WordLength = COM_WORDLENGTH_8B,
                                 .StopBits = UART_STOPBITS_1,
                                 .Parity = UART_PARITY_NONE,
                                 .HwFlowCtl = UART_HWCONTROL_NONE
                               };

    return BSP_COM_Init(COM1, &COM_Init);
}

void logger_set_level(LOG_LEVEL log_lvl)
{
    log_level = log_lvl;
}

int32_t logg(LOG_LEVEL log_lvl, const char *format, ...)
{
    char buf[BUF_LEN + LOG_HEADER_LEN + 1];
    int32_t ret_val = -1;
    int32_t n_char;

    if (log_lvl <= log_level)
    {
        memcpy(&buf[0], log_headers[log_lvl], LOG_HEADER_LEN);

        va_list args;
        va_start (args, format);
        ret_val = vsnprintf(&buf[LOG_HEADER_LEN], BUF_LEN + 1, format, args);
        va_end(args);

        if (ret_val < BUF_LEN)
        {
            n_char = ret_val + LOG_HEADER_LEN;
        }
        else
        {
            n_char = BUF_LEN + LOG_HEADER_LEN;
        }
        buf[n_char] = '\n';

        if (HAL_OK != HAL_UART_Transmit (&hcom_uart [COM1], (uint8_t *) &buf[0], n_char + 1, COM_POLL_TIMEOUT))
        {
            ret_val = 0;
        }
    }
    return ret_val;
}

#endif

#ifdef CORE_CM7

int32_t logger_init(uint32_t baudrate)
{
    /* Queue request to CM4 or ignore */
    return 0;
}

void logger_set_level(LOG_LEVEL log_lvl)
{
    /* Queue request to CM4 or ignore */
}

int32_t logg(LOG_LEVEL log_lvl, const char *fmt, ...)
{
    /* Queue request to CM4 */
    return 0;
}

#endif

