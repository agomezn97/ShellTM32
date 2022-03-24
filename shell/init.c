#include "shell.h"

uint32_t shell_init(enum ttys_instance_id ttys_instance)
{
    struct console_cfg console_cfg;
    struct ttys_cfg ttys_cfg;
    uint32_t result;

    // ttys init
	ttys_get_default_cfg(ttys_instance, &ttys_cfg);
    result = ttys_init(ttys_instance, &ttys_cfg);
    if (result < 0)
        return result;

    // cmd init
    cmd_init(NULL);

    // console init
    console_get_default_cfg(&console_cfg);
    console_init(&console_cfg);

    return 0;
}
