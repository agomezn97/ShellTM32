#ifndef _DIO_H_
#define _DIO_H_

/**
 * @brief Interface declaration of dio module.
 *
 * This module provides access to discrete inputs and outputs, sometimes
 * referred to as digital inputs and outputs.
 *
 * During configuration, the user must specify the set of inputs and outputs and
 * their characteristics.
 *
 * The following console commands are provided:
 * > dio status
 * > dio get
 * > dio set
 * See code for details.
 *
 * Currently, definitions from the STMicroelectronics Low Level (LL) device
 * library are used for some configuration parameters. A future enhancement would
 * be to define all configuration parameters in this module, so that user code
 * is more portable.
 */

#include <stdint.h>

#include "stm32f7xx_ll_gpio.h"

/**
 * Guide to defining dio inputs and outputs.
 * - An array of dio_in_info and dio_out_info structures is created for inputs
 *   and outputs.
 * - Pointers to these arrays are passed to the dio module, and the dio module
 *   stores the pointers, and accesses the arrays during normal operation.
 *
 * Fields common for inputs and outputs:
 *   - name : A readable name for the input/output.
 *   - port : One of DIO_PORT_A, DIO_PORT_B, ...
 *   - pin : One of:
 *     + DIO_PIN_0
 *     + DIO_PIN_1
 *          :
 *     + DIO_PIN_15
 *   - pull : One of:
 *     + DIO_PULL_NO
 *     + DIO_PULL_UP
 *     + DIO_PULL_DOWN
 *   - invert : True to invert the signal value.
 *
 * Fields for outputs only:
 *   - init_value : 0 or 1
 *   - speed : One of:
 *     + DIO_SPEED_FREQ_LOW
 *     + DIO_SPEED_FREQ_MEDIUM
 *     + DIO_SPEED_FREQ_HIGH
 *     + DIO_SPEED_FREQ_VERY_HIGH
 *   - output_type : One of:
 *     + DIO_OUTPUT_PUSHPULL
 *     + DIO_OUTPUT_OPENDRAIN
 */
//=============================================================================
//                         Preprocessor Constants
//=============================================================================
#define DIO_PORT_A               (GPIOA)
#define DIO_PORT_B               (GPIOB)
#define DIO_PORT_C               (GPIOC)
#define DIO_PORT_D               (GPIOD)
#define DIO_PORT_E               (GPIOE)
#define DIO_PORT_F               (GPIOF)
#define DIO_PORT_G               (GPIOG)
#define DIO_PORT_H               (GPIOH)
#define DIO_PORT_I               (GPIOI)
#define DIO_PORT_J               (GPIOJ)
#define DIO_PORT_K               (GPIOK)

#define DIO_PIN_0                (GPIO_PIN_0)
#define DIO_PIN_1                (GPIO_PIN_1)
#define DIO_PIN_2                (GPIO_PIN_2)
#define DIO_PIN_3                (GPIO_PIN_3)
#define DIO_PIN_4                (GPIO_PIN_4)
#define DIO_PIN_5                (GPIO_PIN_5)
#define DIO_PIN_6                (GPIO_PIN_6)
#define DIO_PIN_7                (GPIO_PIN_7)
#define DIO_PIN_8                (GPIO_PIN_8)
#define DIO_PIN_9                (GPIO_PIN_9)
#define DIO_PIN_10               (GPIO_PIN_10)
#define DIO_PIN_11               (GPIO_PIN_11)
#define DIO_PIN_12               (GPIO_PIN_12)
#define DIO_PIN_13               (GPIO_PIN_13)
#define DIO_PIN_14               (GPIO_PIN_14)
#define DIO_PIN_15               (GPIO_PIN_15)

#define DIO_PULL_NO              (LL_GPIO_PULL_NO)
#define DIO_PULL_UP              (LL_GPIO_PULL_UP)
#define DIO_PULL_DOWN            (LL_GPIO_PULL_DOWN)

#define DIO_SPEED_FREQ_LOW       (LL_GPIO_SPEED_FREQ_LOW)
#define DIO_SPEED_FREQ_MEDIUM    (LL_GPIO_SPEED_FREQ_MEDIUM)
#define DIO_SPEED_FREQ_HIGH      (LL_GPIO_SPEED_FREQ_HIGH)
#define DIO_SPEED_FREQ_VERY_HIGH (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define DIO_OUTPUT_PUSHPULL      (LL_GPIO_OUTPUT_PUSHPULL)
#define DIO_OUTPUT_OPENDRAIN     (LL_GPIO_OUTPUT_OPENDRAIN)

//=============================================================================
//                            Type Definitions
//=============================================================================
struct dio_in_info {
    const char* const name;
    GPIO_TypeDef* const port;
    const uint32_t pin;
    const uint32_t pull;
    const uint8_t invert;
};

struct dio_out_info {
    const char* const name;
    GPIO_TypeDef* const port;
    const uint32_t pin;
    const uint32_t pull;
    const uint8_t invert;
    const uint8_t init_value;
    const uint32_t speed;
    const uint32_t output_type;
};

struct dio_cfg
{
    const uint32_t num_inputs;
    const struct dio_in_info* const inputs;
    const uint32_t num_outputs;
    const struct dio_out_info* const outputs;
};

//=============================================================================
//                         DIO interface functions
//=============================================================================
/**
 * @brief Initialize dio module instance.
 *
 * @param[in] cfg The dio configuration.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * This function initializes the dio singleton module. dio_init() keeps a
 * copy of the cfg pointer.
 */
int32_t dio_init(struct dio_cfg* cfg);

/**
 * @brief Get value of discrete input.
 *
 * @param[in] din_idx Discrete input index per module configuration.
 *
 * @return Input state (0/1), else a "ERR" value (< 0). See code for
 *         details.
 */
int32_t dio_get(uint32_t din_idx);

/**
 * @brief Get value of discrete output.
 *
 * @param[in] dout_idx Discrete output index per module configuration.
 *
 * @return Output state (0/1), else a "ERR" value (< 0). See code for
 *         details.
 */
int32_t dio_get_out(uint32_t dout_idx);

/**
 * @brief Set value of discrete output.
 *
 * @param[in] dout_idx Discrete output index per module configuration.
 * @param[in] value Output value 0/1.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 */
int32_t dio_set(uint32_t dout_idx, uint32_t value);

/**
 * @brief Get number of discrete inputs.
 *
 * @return Return number of inputs (non-negative) for success, else a "ERR"
 *         value. See code for details.
 */
int32_t dio_get_num_in(void);

/**
 * @brief Get number of discrete output.
 *
 * @return Return number of outputs (non-negative) for success, else a "ERR"
 *         value. See code for details.
 */
int32_t dio_get_num_out(void);

#endif /*_DIO_H_ */
