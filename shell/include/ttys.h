#ifndef _SHELL_TTYS_H_
#define _SHELL_TTYS_H_

/**
 * @brief Interface declaration of ttys module.
 *
 * This module provides a simple "TTY serial" interface for MCU UARTs.
 * Main features:
 * - Buffering on output to prevent blocking (overrun is possible)
 * - Buffering on input to avoid loss of input characters (overrun is possible)
 * - Integrate into the C standard library streams I/O (to support printf and
     friends)
 *
 * This library makes use of the STMicroelectronics HAL device library.
 *
 * Currently, this module does not perform hardware initialization of the UART
 * and associated hardware (e.g. GPIO), except for the interrupt controller (see
 * below). It is expected that the driver library has been used for
 * initialization (e.g. via generated IDE code). This avoids having to deal with
 * the issue of inconsistent driver libraries among MCUs.
 *
 * This module enables USART interrupts on the Nested Vector Interrupt
 * Controller (NVIC). It also overrides the (weak) USART interrupt handler
 * functions (USARTx_IRQHandler). Thus, in the IDE device configuration tool,
 * the "USARTx global interrupt" should NOT be chosen or you will get a
 * duplicate symbol at link time.
 *
 * A future feature is to perform full hardware initialization in this library,
 * and allowing at least some UART parameters to be set (e.g. baud).
 * Also allow communication to the host PC by methods other than UART,
 * like Ethernet.
 */

//=============================================================================
//                             Included Files
//=============================================================================
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//=============================================================================
//                         Preprocessor Constants
//=============================================================================
#define TTYS_RX_BUF_SIZE 80
#define TTYS_TX_BUF_SIZE 1024

//=============================================================================
//                            Type Definitions
//=============================================================================
/**
 * UART numbering based on the MCU hardware definition
 */
enum ttys_instance_id {
    TTYS_INSTANCE_UART1,
    TTYS_INSTANCE_UART5,
    TTYS_INSTANCE_UART6,

    TTYS_NUM_INSTANCES
};

/**
 * TTYS configuration struct:
 * - create_stream:    if set to TRUE, stdio stream is created for the UART, which
 *                     allows you to use stream IO API like printf(). Creating the
 *                     stream uses some heap memory.
 * - send_cr_after_nl: determines if a carriage return is automatically sent
 *                     after a new line.
 */
struct ttys_cfg {
    bool create_stream;
    bool send_cr_after_nl;
};

//=============================================================================
//                       TTYS core interface functions
//=============================================================================
/**
 * @brief Get default ttys configuration.
 *
 * @param[out] cfg The ttys configuration with defaults filled in.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 */
int32_t ttys_get_default_cfg(enum ttys_instance_id instance_id,
		                     struct ttys_cfg* cfg);

/**
 * @brief Initialize ttys module instance.
 *
 * @param[in] instance_id Identifies the ttys instance.
 * @param[in] cfg The ttys module configuration (FUTURE)
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * This function initializes a ttys module instance and enable
 * its interrupts.
 */
int32_t ttys_init(enum ttys_instance_id instance_id, struct ttys_cfg* cfg);

//=============================================================================
//                                Other APIs
//=============================================================================
/**
 * @brief Put a character in the transmission buffer.
 *
 * @param[in] instance_id Identifies the ttys instance.
 * @param[in] c Character to transmit.
 *
 * @return 0 for success, else a "ERR" value. See code for details.
 *
 * @note Before this module is started, the UART is not known, but the user can
 *       still put chars in the TX buffer that will be transmitted if and when
 *       the module is started.
 */
int32_t ttys_putc(enum ttys_instance_id instance_id, char c);

/**
 * @brief Get character from the receive buffer.
 *
 * @param[in] instance_id Identifies the ttys instance.
 * @param[out] c Received character.
 *
 * @return Number of characters returned (0 or 1)
 */
int32_t ttys_getc(enum ttys_instance_id instance_id, char* c);

/**
 * @brief Get file descriptor for a ttys instance.
 *
 * @param[in] instance_id Identifies the ttys instance.
 *
 * @return File descriptor (>= 0) for success, else a "ERR" value (<0). See
 *          code for details.
 */
int ttys_get_fd(enum ttys_instance_id instance_id);

/**
 * @brief Get FILE stream for a ttys instance.
 *
 * @param[in] instance_id Identifies the ttys instance.
 *
 * @return FILE stream pointer, or NULL if error.
 */
FILE* ttys_get_stream(enum ttys_instance_id instance_id);

#endif /* _SHELL_TTYS_H_ */
