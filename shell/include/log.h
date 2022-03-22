#ifndef _SHELL_LOG_H_
#define _SHELL_LOG_H_

/**
 * @brief Interface declaration of log module.
 *
 * This module provides a simple logging utility. Most of the API is implemented
 * via macros found in this header file. Output is written to a ttys instance
 * specified during module creation.
 *
 * A client that uses the log API macros must have a private/static variable
 * called "log_level" to control the amount of logging. Typically, the client
 * provides a pointer to this variable to the cmd module. The cmd module then
 * allows the console user to display and set the client's logging level.
 *
 * There is also a global variable, log_active, that can be used to inhibit
 * log output. The console module toggles this variable on/off based on a
 * input key (ctrl-L).
 */

#include "shell.h"

//=============================================================================
//                         Preprocessor Constants
//=============================================================================
/**
 * The log toggle char at the console is ctrl-L which is form feed, or 0x0c
 */
#define LOG_TOGGLE_CHAR '\x0c'

#define LOG_LEVEL_NAMES "off, error, warning, info, debug, trace"
#define LOG_LEVEL_NAMES_CSV "off", "error", "warning", "info", "debug", "trace"

//=============================================================================
//                            Type Definitions
//=============================================================================
enum log_level {
    LOG_OFF = 0,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
    LOG_DEFAULT = LOG_INFO
};

//=============================================================================
//                                   API
//=============================================================================
/**
 * @brief Toggle state of "log active".
 */
void log_toggle_active(void);

/**
 * @brief Get state of "log active".
 *
 * @return Value of log active.
 */
bool log_is_active(void);

/**
 * @brief Base "printf" style function for logging.
 *
 * @param[in] fmt Format string
 */
void log_printf(const char* fmt, ...);

//=============================================================================
//                         Preprocessor Macros
//=============================================================================
#define log_error(fmt, ...) do { if (_log_active && log_level >= LOG_ERROR) \
            log_printf("ERR  " fmt, ##__VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (_log_active && log_level >= LOG_WARNING) \
            log_printf("WARN " fmt, ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...) do { if (_log_active && log_level >= LOG_INFO) \
            log_printf("INFO " fmt, ##__VA_ARGS__); } while (0)
#define log_debug(fmt, ...) do { if (_log_active && log_level >= LOG_DEBUG) \
            log_printf("DBG  " fmt, ##__VA_ARGS__); } while (0)
#define log_trace(fmt, ...) do { if (_log_active && log_level >= LOG_TRACE) \
            log_printf("TRC  " fmt, ##__VA_ARGS__); } while (0)

// Following variable is global to allow efficient access by macros,
// but is considered private.
extern bool _log_active;

#endif /* _SHELL_LOG_H_ */
