#ifndef _SHELL_CONSOLE_H_
#define _SHELL_CONSOLE_H_

/**
 * @brief Interface declaration of console module.
 *
 * This module supports a CLI console on a ttys. Upon receiving a command it
 * passes the command string to the cmd module for execution.
 *
 * This module provides simple line discipline functions:
 * - Echoing received characters.
 * - Handling backspace/delete.
 *
 * This module recognizes a special character to enable/disable logging output
 * (i.e. toggle). This is handy to temporarily stop logging output when running
 * commands.
 */

#include "ttys.h"

//=============================================================================
//                            Type Definitions
//=============================================================================
struct console_cfg {
    enum ttys_instance_id ttys_instance_id;
};

//=============================================================================
//                    Console module interface functions
//=============================================================================
/**
 * @brief Get default console configuration.
 *
 * @param[out] cfg The console configuration with defaults filled in.
 *
 * @return 0 for success, else a "SHELL_ERR_ARG" value (see ShellStatus enum).
 */
int32_t console_get_default_cfg(struct console_cfg* cfg);

/**
 * @brief Initialize the console module instance.
 *
 * @param[in] cfg The console configuration.
 *
 * @return 0 for success, else a "SHELL_ERR_ARG" value (see ShellStatus enum).
 *
 * This function initializes the console singleton module. Generally, it should
 * not access other modules as they might not have been initialized yet. An
 * exception is the log module.
 */
int32_t console_init(struct console_cfg* cfg);

/**
 * @brief Run console instance.
 *
 * @return 0 for success.
 *
 * @note This function should not block.
 *
 * This function runs the console singleton module, during normal operation.
 */
int32_t console_run(void);


#endif /* _SHELL_CONSOLE_H_ */
