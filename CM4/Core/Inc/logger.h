#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdint.h>

typedef enum LOG_LEVEL
{
    LOG_ERR = 0, LOG_WRN, LOG_INF, LOG_DBG, LOG_N
} LOG_LEVEL;

int32_t logger_init(uint32_t baudrate);
void logger_set_level(LOG_LEVEL log_lvl);
int32_t logg(LOG_LEVEL log_lvl, const char *fmt, ...);

#endif /* LOGGER_H */

