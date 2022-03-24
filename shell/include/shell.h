#ifndef _SHELL_H_
#define _SHELL_H_

/**
 * @brief TODO
 *
 */

//=============================================================================
//                            Included Files
//=============================================================================
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "log.h"
#include "ttys.h"
#include "console.h"
#include "cmd.h"
#include "stm32f7xx_hal.h"

//=============================================================================
//                           Macro Definitions
//=============================================================================
/**
 * Error codes
 */
#define SHELL_ERR_ARG          -1
#define SHELL_ERR_RESOURCE     -2
#define SHELL_ERR_STATE        -3
#define SHELL_ERR_BAD_CMD      -4
#define SHELL_ERR_BUF_OVERRUN  -5
#define SHELL_ERR_BAD_INSTANCE -6

//=============================================================================
//                           Macro Definitions
//=============================================================================
// Get size of an array.
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * @brief Shell initialization function
 *
 * @param[in] ttys_instance The ttys instance used for transmission. See the
 * enum for more details.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 */
uint32_t shell_init(enum ttys_instance_id ttys_instance);

#endif /* _SHELL_H_ */
