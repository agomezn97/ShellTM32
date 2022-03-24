# ShellTM32

ShellTM32 is a console-based command-line shell that allows users to interface with STM32 devices. In order to do that, user's modules can register their own set of commands to the shell. The shell's job is to create a serial connection with the host PC in order to accept commands, interpret them, and issue them to the appropriate module. The user can optionally assign a log level to each module and make use of the shell's logging capability. The shell is integrated in the stdio API, meaning that you can use printf to print messages in the console.

![console](https://user-images.githubusercontent.com/76229651/159970916-b0cc099a-019c-4ed7-b184-de59f812c1d2.png)

The option of communicating with the shell over Ethernet is on the way.


## Usage
Using ShellTM32 is very easy! You can check the example folder to see the integration of a dio (digital I/O) module into the shell.

### In main.c

1. To begin, copy the 'shell' folder in your project directory and `#include "shell.h"` in your main.c file. Make sure to add 'shell/include' to your include path as well.
2. After that, you have to initialize the shell by calling before your infinite loop:
```C
shell_init(TTYS_INSTANCE_UARTx);
```
You must specify the UART you are using for serial transmission when calling `shell_init()` (if you are using an evaluation board, this will generally be the UART used for VirtualCOM).
IMPORTANT: it is the user's job to correctly initialize the UART instance before calling `shell_init()`. You can achieve this using CubeMX's auto-generated code.

3. Later in your infinite loop you simply call:
```C
while(1)
{
        console_run();
        ...
}
```
### In your module
You must do the following in order to add your custom commands to the shell:
1. Add `#include "shell.h"` in your module.c file.
2. Declare an array of cmd_info structs:
```C
static struct cmd_info cmds[] = {
    {
        .name = "command1_name",
        .func = cmd1_function,
        .help = "help string",
    },
    {
        .name = ""command2_name"",
        .func = cmd2_function,
        .help = "help string",
    },
    {
        .name = ""command3_name"",
        .func = cmd3_function,
        .help = "help string",
    },
};
```
3. (Optionally) Declare a log_level variable:
```C
static int32_t log_level = LOG_DEFAULT;
```
The possible values are LOG_OFF, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_TRACE, LOG_DEFAULT = LOG_INFO.

4. Declare a cmd_client_info struct:
```C
static struct cmd_client_info client_info = {
    .name = "module_name",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};
```

5. In your `module_init()` function, register the commands in the shell:
```C
int32_t result = cmd_register(&client_info);
if (result < 0) {
    log_error("module_name: cmd error %d\n", result);
    return SHELL_ERR_RESOURCE;
}
```

## Author
Antonio Gomez Navarro - agomez@emberity.com

Check my [website](www.emberity.com)!
