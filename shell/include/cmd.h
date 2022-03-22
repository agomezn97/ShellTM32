#ifndef _SHELL_CMD_H_
#define _SHELL_CMD_H_

/**
 * @brief Interface declaration of cmd module.
 *
 * This module is a utility used by clients (e.g. other modules) to provide
 * console commands. It works like this:
 * - Client register a set of console commands during startup.
 * - When the console module receives a command line string, it passes it to
 *   this (cmd) module which parses the string, and handles it directly, or
 *   calls a client command handler function.
 * - This module also provides some common commands automatically, on behalf
 *   of the clients, as described below.
 *
 * Generally, a command line string consists of a client name, then the command
 * name, and then optional command arguments.
 *
 * For example, say the tmr module has two commands, "status" and "test". The
 * tmr module would register these commands, with this (cmd) module. It would
 * specify a client name of "tmr". It would provide information for the commands
 * "status" and "test". Then, to invoke these commands, the console user would
 * enter commands like this.
 *
 * > tmr status
 * > tmr test get 0
 *
 * This module would parse the command line, and call the appropriate command
 * handler function specified by the tmr client (as part of registration).
 *
 * All command line tokens (i.e. the client name, the command name, and any
 * arguments) are passed to the command handler using the (int32_t argc, char**
 * argv) pattern. It is up to the command handler function to validate the
 * arguments.
 *
 * The cmd module provides several commands automatically on behalf of the
 * clients:
 * - A help command. For example, if the console user enters "tmr help" then a
 *   list of tmr commands is printed.
 * - A command to get and set the client's logging level, if the client provided
 *   access to its logging level variable (as part of registration). For
 *   example, the console user could enter "tmr log" to get the current log
 *   level, or "tmr log debug" or "tmr log off" to set the log level.
 * - A command to get or clear the client's performance measurements (typically
 *   counters), if the client provided access to these measurements (as part of
 *   registration). For example, the console user could enter "ttys pm" to get
 *   the current performance measurement values, or "ttys pm clear" to clear
 *   them.
 *
 * The cmd module provides a global "help" command to list the commands of all
 * clients. The token "?" can be used in place of help.
 *
 * The cmd module provides "wild card" commands which are executed for all
 * clients. In this case, the first token in the command line is "*" rather than
 * a client name. The following wild card commands are supported:
 *
 * > * log
 * > * log <new-level>
 */

#include <stdint.h>

//=============================================================================
//                         Preprocessor Constants
//=============================================================================
/**
 * Maximum number of clients (modules) supported
 */
#define CMD_MAX_CLIENTS  10

//=============================================================================
//                            Type Definitions
//=============================================================================
/**
 * Function signature for a command handler function
 */
typedef int32_t (*cmd_func)(int32_t argc, const char** argv);

/**
 * Information about a single command, provided by the client
 */
struct cmd_info {
    const char* const name;  /**< Name of command     */
    const char* const help;  /**< Command help string */
    const cmd_func func;     /**< Command function    */
};

/**
 * Information provided by the client:
 * - Command base name
 * - Command set info
 * - Pointer to log level variable (optional)
 */
struct cmd_client_info {
    const char* const name;                  /**< Client name (first command line token)  */
    const int32_t num_cmds;                  /**< Number of commands                      */
    const struct cmd_info* const cmds;       /**< Pointer to array of command info struct */
    int32_t* const log_level_ptr;            /**< Pointer to log level variable (or NULL) */
};

/**
 * Structure containing a parsed argument value
 */
struct cmd_arg_val {
    char type;
    union {
        void*       p;
        uint8_t*    p8;
        uint16_t*   p16;
        uint32_t*   p32;
        int32_t     i;
        uint32_t    u;
        const char* s;
    } val;
};

struct cmd_cfg {
    // FUTURE
};

//=============================================================================
//                       CMD module interface functions
//=============================================================================
/**
 * @brief Initialize cmd instance.
 *
 * @param[in] cfg The cmd configuration. (FUTURE)
 *
 * @return 0 for success, else a "ERR" value.
 *
 * This function initializes the cmd singleton module. Generally, it should not
 * access other modules as they might not have been initialized yet. An
 * exception is the log module.
 */
int32_t cmd_init(struct cmd_cfg* cfg);

/**
 * @brief Register a client.
 *
 * @param[in] cmd_client_info The client's configuration (eg. command metadata)
 *
 * @return 0 for success, else a "SHELL_ERR_RESOURCE" value.
 *
 * @note This function keeps a copy of the cmd_client_info pointer.
 */
int32_t cmd_register(const struct cmd_client_info* cmd_client_info);

/**
 * @brief Execute a command line
 *
 * @param[in] bfr The buffer containing the command line arguments.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * This function parses the command line and then executes the command,
 * typically by running a command function handler for a client.
 *
 * @note The user may not need to call this function as it's implicitly
 * called by the console module.
 */
int32_t cmd_execute(char* bfr);

/**
 * @brief Parse and validate command arguments
 *
 * @param[in]  argc The number of arguments to be parsed.
 * @param[in]  argv Pointer string array of arguments to be be parsed.
 * @param[in]  fmt String indicating expected argument types (format)
 * @param[out] arg_vals Pointer to array of parsed argument values
 *
 * @return Number of parsed arguments, negative value if error.
 *
 * @note In case of error, an error message is always printed to the
 * console.
 *
 * This function provides a common method to parse and validate command
 * arguments. In particular, it does string to numeric conversions, with error
 * checking.
 *
 * A format string is used to guide the parsing. The format string contains a
 * letter for each expected argument. The supported letters are:
 * - i Integer value, in either decimal, octal, or hex formats. Octal values
 *     must start with 0, and hex values must start with 0x.
 * - u Unsigned value, in either decimal, octal, or hex formats. Octal values
 *     must start with 0, and hex values must start with 0x.
 * - p Pointer, in hex format. No leading 0x is necessary (but allowed).
 * - s String
 *
 * In addition:
 * - [ indicates that remaining arguments are optional. However,
 *   if one optional argument is present, then subsequent arguments
 *   are required.
 * - ] is ignored; it can be put into the fmt string for readability,
 *   to match brackets.
 *
 * Examples:
 *   "up" - Requires exactly one unsigned and one pointer argument.
 *   "ii" - Requires exactly two integer arguments.
 *   "i[i" - Requires either one or two integer arguments.
 *   "i[i]" - Same as above (matched brackets).
 *   "i[i[i" - Requires either one, two, or three integer arguments.
 *   "i[i[i]]" - Same as above (matched brackets).
 *   "i[ii" - Requires either one or three integer arguments.
 *   "i[ii]" - Same as above (matched brackets).
 *
 * @return On success, the number of arguments present (>=0), a "ERR" value
 *         (<0). See code for details.
 *
 */
int32_t cmd_parse_args(int32_t argc, const char** argv, const char* fmt,
                       struct cmd_arg_val* arg_vals);

#endif /* _SHELL_CMD_H_ */
