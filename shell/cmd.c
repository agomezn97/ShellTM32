/**
 * @brief Implementation of cmd module.
 */

#include "shell.h"

//=============================================================================
//                         Preprocessor Constants
//=============================================================================
#define MAX_CMD_TOKENS 10

//=============================================================================
//                  Private (static) function declarations
//=============================================================================
static const char* log_level_str(int32_t level);
static int32_t log_level_int(const char* level_name);

//=============================================================================
//                       Private (static) variables
//=============================================================================
static const struct cmd_client_info* client_info[CMD_MAX_CLIENTS];

static int32_t log_level = LOG_DEFAULT;
static const char* log_level_names[] = { LOG_LEVEL_NAMES_CSV };

//=============================================================================
//                       Public (global) functions
//=============================================================================
int32_t cmd_init(struct cmd_cfg* cfg)
{
    memset(client_info, 0, sizeof(client_info));
    return 0;
}


int32_t cmd_register(const struct cmd_client_info* _client_info)
{
    for (int32_t idx = 0; idx < CMD_MAX_CLIENTS; idx++) {
        if (client_info[idx] == NULL ||
            strcasecmp(client_info[idx]->name, _client_info->name) == 0) {
            client_info[idx] = _client_info;
            return 0;
        }
    }

    return SHELL_ERR_RESOURCE;
}


// TODO: Refactor by spliting in smaller functions!!
int32_t cmd_execute(char* bfr)
{
    int32_t num_tokens = 0;
    const char* tokens[MAX_CMD_TOKENS];
    char* p = bfr;
    int32_t idx;
    int32_t idx2;
    const struct cmd_client_info* ci;
    const struct cmd_info* cmdi;

    // Tokenize the command line in-place.
    while (1) {

        // Find start of token.
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0') {
            // Found end of line.
            break;
        } else {
            if (num_tokens >= MAX_CMD_TOKENS) {
                printf("Too many arguments\n");
                return SHELL_ERR_BAD_CMD;
            }
            // Record pointer to token and find its end.
            tokens[num_tokens++] = p;
            while (*p && !isspace((unsigned char)*p))
                p++;
            if (*p) {
                // Terminate token.
                *p++ = '\0';
            } else {
                // Found end of line.
                break;
            }
        }
    }

    // If there are no tokens, nothing to do.
    if (num_tokens == 0)
        return 0;

    // Handle wild card commands
    if (strcmp("*", tokens[0]) == 0) {
        if (num_tokens < 2) {
            printf("Wildcard missing command\n");
            return SHELL_ERR_BAD_CMD;
        }
        if (strcasecmp(tokens[1], "log") == 0) {
            int32_t log_level = 0;
            if (num_tokens == 3) {
                log_level = log_level_int(tokens[2]);
                if (log_level < 0) {
                    printf("Invalid log level: %s\n", tokens[2]);
                    return SHELL_ERR_ARG;
                }
            } else if (num_tokens > 3) {
                printf("Invalid arguments\n");
                return SHELL_ERR_ARG;
            }
            for (idx = 0;
                 idx < CMD_MAX_CLIENTS && client_info[idx] != NULL;
                 idx++) {
                ci = client_info[idx];
                if (ci->log_level_ptr != NULL) {
                    if (num_tokens == 3) {
                        *ci->log_level_ptr = log_level;
                    } else {
                        printf("Log level for %s = %s\n", ci->name,
                               log_level_str(*ci->log_level_ptr));
                    }
                }
            }
        }
        return 0;
    }

    // Handle top-level help.
    if (strcasecmp("help", tokens[0]) == 0 ||
        strcasecmp("?", tokens[0]) == 0) {
        for (idx = 0;
             idx < CMD_MAX_CLIENTS && client_info[idx] != NULL;
             idx++) {
            ci = client_info[idx];
            if (ci->num_cmds == 0)
                continue;
            printf("%s (", ci->name);
            for (idx2 = 0; idx2 < ci->num_cmds; idx2++) {
                cmdi = &ci->cmds[idx2];
                printf("%s%s", idx2 == 0 ? "" : ", ", cmdi->name);
            }
            // If client provided log level, include log command.
            if (ci->log_level_ptr)
                printf("%s%s", idx2 == 0 ? "" : ", ", "log");

            printf(")\n");
        }
        printf("\nLog levels are: %s\n", LOG_LEVEL_NAMES);
        return 0;
    }

    // Find and execute the command.
    for (idx = 0;
         idx < CMD_MAX_CLIENTS && client_info[idx] != NULL;
         idx++) {
        ci = client_info[idx];
        if (strcasecmp(tokens[0], ci->name) != 0)
            continue;

        // If there is no command, create a dummy.
        if (num_tokens == 1)
            tokens[1] = "";

        // Handle help command directly.
        if (strcasecmp(tokens[1], "help") == 0 ||
            strcasecmp(tokens[1], "?") == 0) {
            log_debug("Handle client help\n");
            for (idx2 = 0; idx2 < ci->num_cmds; idx2++) {
                cmdi = &ci->cmds[idx2];
                printf("%s %s: %s\n", ci->name, cmdi->name, cmdi->help);
            }

            // If client provided log level, print help for log command.
            if (ci->log_level_ptr) {
                printf("%s log: set or get log level, args: [level]\n",
                       ci->name);
            }

            if (ci->log_level_ptr)
                printf("\nLog levels are: %s\n", LOG_LEVEL_NAMES);

            return 0;
        }

        // Handle log command directly.
        if (strcasecmp(tokens[1], "log") == 0) {
            log_debug("Handle command log\n");
            if (ci->log_level_ptr) {
                if (num_tokens < 3) {
                    printf("Log level for %s = %s\n", ci->name,
                           log_level_str(*ci->log_level_ptr));
                } else {
                    int32_t log_level = log_level_int(tokens[2]);
                    if (log_level < 0) {
                        printf("Invalid log level: %s\n", tokens[2]);
                        return SHELL_ERR_ARG;
                    }
                    *ci->log_level_ptr = log_level;
                }
            }
            return 0;
        }

        // Find the command
        for (idx2 = 0; idx2 < ci->num_cmds; idx2++) {
            if (strcasecmp(tokens[1], ci->cmds[idx2].name) == 0) {
                log_debug("Handle command\n");
                ci->cmds[idx2].func(num_tokens, tokens);
                return 0;
            }
        }

        printf("No such command (%s %s)\n", tokens[0], tokens[1]);
        return SHELL_ERR_BAD_CMD;
    }

    printf("No such command (%s)\n", tokens[0]);
    return SHELL_ERR_BAD_CMD;
}


int32_t cmd_parse_args(int32_t argc, const char** argv, const char* fmt,
                       struct cmd_arg_val* arg_vals)
{
    int32_t arg_cnt = 0;
    char* endptr;
    bool opt_args = false;

    while (*fmt) {
        if (*fmt == '[') {
            opt_args = true;
            fmt++;
            continue;
        }
        if (*fmt == ']') {
            fmt++;
            continue;
        }

        if (arg_cnt >= argc) {
            if (opt_args) {
                return arg_cnt;
            }
            printf("Insufficient arguments\n");
            return SHELL_ERR_BAD_CMD;
        }

        // These error conditions should not occur,
        // but we check them for safety
        if (*argv == NULL || **argv == '\0') {
            printf("Invalid empty arguments\n");
            return SHELL_ERR_BAD_CMD;
        }

        switch (*fmt) {
            case 'i':
                arg_vals->val.i = strtol(*argv, &endptr, 0);
                if (*endptr) {
                    printf("Argument '%s' not a valid integer\n", *argv);
                    return SHELL_ERR_ARG;
                }
                break;
            case 'u':
                arg_vals->val.u = strtoul(*argv, &endptr, 0);
                if (*endptr) {
                    printf("Argument '%s' not a valid unsigned integer\n", *argv);
                    return SHELL_ERR_ARG;
                }
                break;
            case 'p':
                arg_vals->val.p = (void*)strtoul(*argv, &endptr, 16);
                if (*endptr) {
                    printf("Argument '%s' not a valid pointer\n", *argv);
                    return SHELL_ERR_ARG;
                }
                break;
            case 's':
                arg_vals->val.s = *argv;
                break;
            default:
                printf("Bad argument format '%c'\n", *fmt);
                return SHELL_ERR_ARG;
        }
        arg_vals->type = *fmt;
        arg_vals++;
        arg_cnt++;
        argv++;
        fmt++;
        opt_args = false;
    }
    if (arg_cnt < argc) {
        printf("Too many arguments\n");
        return SHELL_ERR_BAD_CMD;
    }

    return arg_cnt;
}

//=============================================================================
//                       Private (static) functions
//=============================================================================
/**
 * @brief Convert integer log level to a string.
 *
 * @param[in] level The log level as an integer.
 *
 * @return Log level as a string (or "INVALID" on error)
 */
static const char* log_level_str(int32_t level)
{
    if (level < ARRAY_SIZE(log_level_names))
        return log_level_names[level];

    return "INVALID";
}


/**
 * @brief Convert log level string to an int.
 *
 * @param[in] level_name The log level as a string.
 *
 * @return Log level as an int, or -1 on error.
 */
static int32_t log_level_int(const char* level_name)
{
    int32_t rc = -1;

    for (int32_t level = 0; level < ARRAY_SIZE(log_level_names); level++) {
        if (strcasecmp(level_name, log_level_names[level]) == 0) {
            rc = level;
            break;
        }
    }

    return rc;
}
