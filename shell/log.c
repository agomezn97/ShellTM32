/**
 * @brief Implementation of log module.
 *
 */

#include "shell.h"

//=============================================================================
//                         Public (global) variables
//=============================================================================
bool _log_active = true;

//=============================================================================
//                         Public (global) functions
//=============================================================================
void log_toggle_active(void)
{
    _log_active = _log_active ? false : true;
}


bool log_is_active(void)
{
    return _log_active;
}


void log_printf(const char* fmt, ...)
{
    va_list args;
    //uint32_t ms = tmr_get_ms();

    //printf("%lu.%03lu ", ms / 1000U, ms % 1000U);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
