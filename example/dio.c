/**
 * @brief Implementation of dio module.
 *
 */

#include "shell.h"
#include "dio.h"

//=============================================================================
//                   Private (static) function declarations
//=============================================================================
static int32_t cmd_dio_status(int32_t argc, const char** argv);
static int32_t cmd_dio_get(int32_t argc, const char** argv);
static int32_t cmd_dio_set(int32_t argc, const char** argv);

//=============================================================================
//                       Private (static) variables
//=============================================================================
static struct dio_cfg* cfg;

static struct cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_dio_status,
        .help = "Get module status, usage: dio status",
    },
    {
        .name = "get",
        .func = cmd_dio_get,
        .help = "Get input value, usage: dio get <input-name>",
    },
    {
        .name = "set",
        .func = cmd_dio_set,
        .help = "Set output value, usage: dio set <output-name> {0|1}",
    },
};

static int32_t log_level = LOG_DEFAULT;

static struct cmd_client_info cmd_info = {
    .name = "dio",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};

//=============================================================================
//                       Public (global) functions
//=============================================================================
int32_t dio_init(struct dio_cfg* _cfg)
{
    uint32_t idx;
    const struct dio_in_info*  dii;
    const struct dio_out_info* doi;

    cfg = _cfg;

    // Configure inputs
    for (idx = 0; idx < cfg->num_inputs; idx++) {
        dii = &cfg->inputs[idx];
        LL_GPIO_SetPinPull(dii->port, dii->pin, dii->pull);
        LL_GPIO_SetPinMode(dii->port, dii->pin, LL_GPIO_MODE_INPUT);
    }

    // Configure outputs
    for (idx = 0; idx < cfg->num_outputs; idx++) {
        doi = &cfg->outputs[idx];
        LL_GPIO_SetPinSpeed(doi->port, doi->pin, doi->speed);
        LL_GPIO_SetPinOutputType(doi->port, doi->pin,  doi->output_type);
        LL_GPIO_SetPinPull(doi->port, doi->pin, doi->pull);
        LL_GPIO_SetPinMode(doi->port, doi->pin, LL_GPIO_MODE_OUTPUT);
    }

    // Register the commands in the cmd module
    int32_t result = cmd_register(&cmd_info);
    if (result < 0) {
	    log_error("dio_start: cmd error %d\n", result);
	    return SHELL_ERR_RESOURCE;
    }

    return 0;
}


int32_t dio_get(uint32_t din_idx)
{
    if (din_idx >= cfg->num_inputs)
        return SHELL_ERR_ARG;

    return LL_GPIO_IsInputPinSet(cfg->inputs[din_idx].port,
                                 cfg->inputs[din_idx].pin) ^
           cfg->inputs[din_idx].invert;
}


int32_t dio_get_out(uint32_t dout_idx)
{
    if (dout_idx >= cfg->num_outputs)
        return SHELL_ERR_ARG;

    return LL_GPIO_IsOutputPinSet(cfg->outputs[dout_idx].port,
                                  cfg->outputs[dout_idx].pin) ^
           cfg->outputs[dout_idx].invert;
}


int32_t dio_set(uint32_t dout_idx, uint32_t value)
{
    if (dout_idx >= cfg->num_outputs)
        return SHELL_ERR_ARG;

    if (value ^ cfg->outputs[dout_idx].invert) {
        LL_GPIO_SetOutputPin(cfg->outputs[dout_idx].port,
                             cfg->outputs[dout_idx].pin);
    } else {
        LL_GPIO_ResetOutputPin(cfg->outputs[dout_idx].port,
                               cfg->outputs[dout_idx].pin);
    }

    return 0;
}


int32_t dio_get_num_in(void)
{
    return cfg == NULL ? SHELL_ERR_RESOURCE : cfg->num_inputs;
}


int32_t dio_get_num_out(void)
{
    return cfg == NULL ? SHELL_ERR_RESOURCE : cfg->num_outputs;
}

//=============================================================================
//                         Private (static) functions
//=============================================================================
/**
 * @brief Console command function for "dio status".
 *
 * @param[in] argc Number of arguments, including "dio".
 * @param[in] argv Argument values, including "dio".
 *
 * @return 0 for success.
 *
 * Command usage: dio status
 */
static int32_t cmd_dio_status(int32_t argc, const char** argv)
{
    uint32_t idx;

    printf("Inputs:\n");
    for (idx = 0; idx < cfg->num_inputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->inputs[idx].name, dio_get(idx));


    printf("Outputs:\n");
    for (idx = 0; idx < cfg->num_outputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->outputs[idx].name,
               dio_get_out(idx));

    return 0;
}

/**
 * @brief Console command function for "dio get".
 *
 * @param[in] argc Number of arguments, including "dio".
 * @param[in] argv Argument values, including "dio".
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * Command usage: dio get <input-name>
 */
static int32_t cmd_dio_get(int32_t argc, const char** argv)
{
    uint32_t idx;
    struct cmd_arg_val arg_vals[1];

    if (cmd_parse_args(argc-2, argv+2, "s", arg_vals) != 1)
        return SHELL_ERR_BAD_CMD;

    for (idx = 0; idx < cfg->num_inputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->inputs[idx].name) == 0)
            break;
    if (idx < cfg->num_inputs) {
        printf("%s = %ld\n", cfg->inputs[idx].name, dio_get(idx));
        return 0;
    }

    for (idx = 0; idx < cfg->num_outputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0)
            break;
    if (idx < cfg->num_outputs) {
        printf("%s %ld\n", cfg->outputs[idx].name, dio_get_out(idx));
        return 0;
    }

    printf("Invalid dio input/output name '%s'\n", arg_vals[0].val.s);
    return SHELL_ERR_ARG;
}

/**
 * @brief Console command function for "dio set".
 *
 * @param[in] argc Number of arguments, including "dio".
 * @param[in] argv Argument values, including "dio".
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * Command usage: dio set <output-name> {0|1}
 */
static int32_t cmd_dio_set(int32_t argc, const char** argv)
{
    uint32_t idx;
    struct cmd_arg_val arg_vals[2];
    uint32_t value;

    if (cmd_parse_args(argc-2, argv+2, "su", arg_vals) != 2)
        return SHELL_ERR_BAD_CMD;

    for (idx = 0; idx < cfg->num_outputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0)
            break;
    if (idx >= cfg->num_outputs) {
        printf("Invalid dio name '%s'\n", arg_vals[0].val.s);
        return SHELL_ERR_ARG;
    }

    value = arg_vals[1].val.u;
    if (value != 0 && value != 1) {
        printf("Invalid value '%s'\n", argv[3]);
        return SHELL_ERR_ARG;
    }

    return dio_set(idx, value);
}
