/**
 * @brief Implementation of console module.
 */

#include "shell.h"

//=============================================================================
//                           Macro Definitions
//=============================================================================
#define PROMPT "> "

//=============================================================================
//                            Type Definitions
//=============================================================================
#define CONSOLE_CMD_BFR_SIZE 80

struct console_state {
    struct console_cfg cfg;
    char cmd_bfr[CONSOLE_CMD_BFR_SIZE];
    uint16_t num_cmd_bfr_chars;
    bool start_of_line;
};

//=============================================================================
//                       Private (static) variables
//=============================================================================
static struct console_state state;
static int32_t log_level = LOG_DEFAULT;

//=============================================================================
//                          Function Definitions
//=============================================================================
int32_t console_get_default_cfg(struct console_cfg* cfg)
{
    if (cfg == NULL)
        return SHELL_ERR_ARG;

    memset(cfg, 0, sizeof(struct console_cfg));
    cfg->ttys_instance_id = TTYS_INSTANCE_UART1;

    return 0;
}


int32_t console_init(struct console_cfg* cfg)
{
    if (cfg == NULL)
        return SHELL_ERR_ARG;

    log_debug("Initializing console...\n");
    memset(&state, 0, sizeof(struct console_state));
    state.cfg = *cfg;
    state.start_of_line = true;

    return 0;
}


int32_t console_run(void)
{
    char c;

    // Print the PROMPT character if we are in the start of line
    if (state.start_of_line) {
        state.start_of_line = false;
        printf(PROMPT);
    }

    while (ttys_getc(state.cfg.ttys_instance_id, &c)) {

        // Handle the processing of completed command line
        if (c == '\n' || c == '\r') {
            state.cmd_bfr[state.num_cmd_bfr_chars] = '\0';
            printf("\n");
            cmd_execute(state.cmd_bfr);
            state.num_cmd_bfr_chars = 0;
            state.start_of_line = true;
        }
        // Handle backspace/delete
        else if (c == '\b' || c == '\x7f') {
            if (state.num_cmd_bfr_chars > 0) {
                // Overwrite last character with a blank
                printf("\b \b");
                state.num_cmd_bfr_chars--;
            }
        }
        // Handle logging on/off toggle
        else if (c == LOG_TOGGLE_CHAR) {
            log_toggle_active();
            printf("\n<Logging %s>\n", log_is_active() ? "on" : "off");
        }
        // Echo the character back.
        else if (isprint(c)) {
            if (state.num_cmd_bfr_chars < (CONSOLE_CMD_BFR_SIZE-1)) {
                state.cmd_bfr[state.num_cmd_bfr_chars++] = c;
                printf("%c", c);
            } else {
                // No space in buffer for the character!
                printf("\a");
            }
        }
    }

    return 0;
}
