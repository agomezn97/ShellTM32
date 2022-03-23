/**
 * @brief Implementation of ttys module.
 */

#include "shell.h"

//=============================================================================
//                              Common Macros
//=============================================================================
// This module integrates into the C language stdio system. The ttys "device
// files" can be viewed as always "open", with the following file
// descriptors. These are then mapped to FILE streams.
//
// Note that one of the UARTs is mapped to stdout (file descriptor 1), which
// is thus the one used for printf() and friends.

#define UART1_FD 1    // stdout
// #define UARTx_FD   // uncomment for available UARTs
#define UART5_FD 3
#define UART6_FD 4

//=============================================================================
//                            Type Definitions
//=============================================================================
/**
 * Per-instance ttys state information
 */
struct ttys_state {
    struct ttys_cfg cfg;
    FILE* stream;
    int fd;
    USART_TypeDef* uart_reg_base;
    uint16_t rx_buf_get_idx;
    uint16_t rx_buf_put_idx;
    uint16_t tx_buf_get_idx;
    uint16_t tx_buf_put_idx;
    char tx_buf[TTYS_TX_BUF_SIZE];
    char rx_buf[TTYS_RX_BUF_SIZE];
};

//=============================================================================
//                  Private (static) function declarations
//=============================================================================
static void ttys_interrupt(enum ttys_instance_id instance_id,
                           IRQn_Type irq_type);

//=============================================================================
//                        Private (static) variables
//=============================================================================
static struct ttys_state ttys_states[TTYS_NUM_INSTANCES];

//=============================================================================
//                        Public (global) functions
//=============================================================================
int32_t ttys_get_default_cfg(enum ttys_instance_id instance_id, struct ttys_cfg* cfg)
{
    if (cfg == NULL)
        return SHELL_ERR_ARG;

    memset(cfg, 0, sizeof(struct ttys_cfg));
    cfg->create_stream = true;
    cfg->send_cr_after_nl = true;

    return 0;
}


int32_t ttys_init(enum ttys_instance_id instance_id, struct ttys_cfg* cfg)
{
    IRQn_Type irq_type;

    // Input checking:
    if (instance_id >= TTYS_NUM_INSTANCES)
        return SHELL_ERR_BAD_INSTANCE;

    if (cfg == NULL)
        return SHELL_ERR_ARG;

    // Get corresponding state instance and initialize all values to zero:
    struct ttys_state* state = &ttys_states[instance_id];
    memset(state, 0, sizeof(struct ttys_state));

    // Initialize all non-zero variables in the state structure:
    state->cfg = *cfg;

    switch (instance_id) {
        case TTYS_INSTANCE_UART1:
            state->uart_reg_base = USART1;
            state->fd = UART1_FD;
            irq_type = USART1_IRQn;
            break;
//        case TTYS_INSTANCE_UARTx:
//            state->uart_reg_base = USARTx;
//            state->fd = UARTx_FD;
//            irq_type = USARTx_IRQn;
//            break;
        case TTYS_INSTANCE_UART5:
            state->uart_reg_base = UART5;
            state->fd = UART5_FD;
            irq_type = UART5_IRQn;
            break;
        case TTYS_INSTANCE_UART6:
            state->uart_reg_base = USART6;
            state->fd = UART6_FD;
            irq_type = USART6_IRQn;
            break;
        default:
            return SHELL_ERR_BAD_INSTANCE;
    }

    if (state->cfg.create_stream) {
        state->stream = fdopen(state->fd, "r+");
        if (state->stream != NULL)
            setvbuf(state->stream, NULL, _IONBF, 0);
    } else {
        state->stream = NULL;
    }

    // Disable I/O buffering for STDOUT stream, so that
    // chars are sent out as soon as they are printed
    setvbuf(stdout, NULL, _IONBF, 0);

    // Enable interrupts
    ATOMIC_SET_BIT(state->uart_reg_base->CR1, USART_CR1_RXNEIE);
    ATOMIC_SET_BIT(state->uart_reg_base->CR1, USART_CR1_TXEIE);

    NVIC_SetPriority(irq_type,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(irq_type);

    return 0;
}


int32_t ttys_putc(enum ttys_instance_id instance_id, char c)
{
    if (instance_id >= TTYS_NUM_INSTANCES)
        return SHELL_ERR_BAD_INSTANCE;

    struct ttys_state* state = &ttys_states[instance_id];

    // Calculate the new TX buffer put index
    uint16_t next_put_idx = state->tx_buf_put_idx + 1;
    if (next_put_idx >= TTYS_TX_BUF_SIZE)
        next_put_idx = 0;

    // If buffer is full, then return error
    if (next_put_idx == state->tx_buf_get_idx)
        return SHELL_ERR_BUF_OVERRUN;

    // Put the char in the TX buffer
    state->tx_buf[state->tx_buf_put_idx] = c;
    state->tx_buf_put_idx = next_put_idx;

    // Ensure the TX interrupt is enabled
    if (ttys_states[instance_id].uart_reg_base != NULL)
	ATOMIC_SET_BIT(state->uart_reg_base->CR1, USART_CR1_TXEIE);

    return 0;
}


int32_t ttys_getc(enum ttys_instance_id instance_id, char* c)
{
    if (instance_id >= TTYS_NUM_INSTANCES)
        return SHELL_ERR_BAD_INSTANCE;

    struct ttys_state* state = &ttys_states[instance_id];

    // Check if buffer is empty
    if (state->rx_buf_get_idx == state->rx_buf_put_idx)
        return 0;

    // Get a character and advance get index
    *c = state->rx_buf[state->rx_buf_get_idx];

    state->rx_buf_get_idx++;
    if (state->rx_buf_get_idx >= TTYS_RX_BUF_SIZE)
    	state->rx_buf_get_idx = 0;

    return 1;
}


int ttys_get_fd(enum ttys_instance_id instance_id)
{
    if (instance_id >= TTYS_NUM_INSTANCES)
        return SHELL_ERR_ARG;

    if (ttys_states[instance_id].fd >= 0)
        return ttys_states[instance_id].fd;

    return SHELL_ERR_RESOURCE;
}


FILE* ttys_get_stream(enum ttys_instance_id instance_id)
{
    if (instance_id >= TTYS_NUM_INSTANCES)
        return NULL;

    return ttys_states[instance_id].stream;
}

//=============================================================================
//                    USART Interrupt Service Routines
//=============================================================================
// The following interrupt handler functions override the default handlers,
// which are "weak" symbols.

void USART1_IRQHandler(void)
{
    ttys_interrupt(TTYS_INSTANCE_UART1, USART1_IRQn);
}

// Uncomment and modify for all the available USARTs:
//
// void USARTx_IRQHandler(void)
// {
//     ttys_interrupt(TTYS_INSTANCE_UARTx, USARTx_IRQn);
// }

//=============================================================================
//                    Private (static) functions
//=============================================================================
static void ttys_interrupt(enum ttys_instance_id instance_id,
                           IRQn_Type irq_type)
{
    struct ttys_state* state = &ttys_states[instance_id];

    uint32_t isr = state->uart_reg_base->ISR;

    if (isr & USART_ISR_RXNE) {
        // Got an incoming character.
    	char rx_data = state->uart_reg_base->RDR;

	// Check RX buffer isn't full
	if ((state->rx_buf_put_idx + 1) != state->rx_buf_get_idx) {
	    state->rx_buf[state->rx_buf_put_idx] = rx_data;
	    state->rx_buf_put_idx++;
	    if (state->rx_buf_put_idx >= TTYS_RX_BUF_SIZE)
		state->rx_buf_put_idx = 0;
	} else {
	    Error_Handler();
	}
    } else if (isr & USART_ISR_TXE) {
        // Can send a character.
        if (state->tx_buf_get_idx == state->tx_buf_put_idx) {
            // No characters to send, disable the interrrupt
            ATOMIC_CLEAR_BIT(state->uart_reg_base->CR1, USART_CR1_TXEIE);
        } else {
            state->uart_reg_base->TDR = state->tx_buf[state->tx_buf_get_idx];
            state->tx_buf_get_idx++;
            if (state->tx_buf_get_idx >= TTYS_TX_BUF_SIZE)
                state->tx_buf_get_idx = 0;
        }
    } else if (isr & (USART_ISR_ORE | USART_ISR_NE |
    		      USART_ISR_FE | USART_ISR_PE)) {
        // Error conditions. To clear the bit, we need to read the data
        // register, but we don't use it.
        (void)state->uart_reg_base->RDR;
    }
}

////////////////////////////////////////////////////////////////////////////////
// The following functions are used to integrate this module into the C
// language stdio system. This is largely based on overriding of the default
// "system call functions" _write and _read. The default functions use "weak"
// symbols.
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Map file descriptor to ttys instance.
 *
 * @param[in] fd File descriptor.
 *
 * @return Instance ID corresponding to file instance, or TTYS_NUM_INSTANCES if
 *         no match.
 */
static enum ttys_instance_id fd_to_instance(int fd)
{
    enum ttys_instance_id instance_id = TTYS_NUM_INSTANCES;

    switch (fd) {
        case UART1_FD:
            instance_id = TTYS_INSTANCE_UART1;
            break;
//        case UARTx_FD:
//			instance_id = TTYS_INSTANCE_UARTx;
//			break;
        case UART5_FD:
            instance_id = TTYS_INSTANCE_UART5;
            break;
        case UART6_FD:
            instance_id = TTYS_INSTANCE_UART6;
            break;
    }

    return instance_id;
}

/**
 * @brief System call function for write().
 *
 * @param[in] file File descriptor.
 * @param[in] ptr Data to be written.
 * @param[in] len Length of data.
 *
 * @return Number of characters written, or -1 for error.
 *
 * @note Characters might be dropped due to buffer overrun, and this is not
 *       reflected the return value. An alternative would be to return -1 and
 *       errno=EWOULDBLOCK.
 */
int _write(int file, char* ptr, int len)
{
    int idx;
    enum ttys_instance_id instance_id = fd_to_instance(file);

    if (instance_id >= TTYS_NUM_INSTANCES) {
        errno = EBADF;
        return -1;
    }

    for (idx = 0; idx < len; idx++) {
        char c = *ptr++;
        ttys_putc(instance_id, c);
        if (c == '\n' && ttys_states[instance_id].cfg.send_cr_after_nl) {
            ttys_putc(instance_id, '\r');
        }
    }

    return len;
}

/**
 * @brief System call function for read().
 *
 * @param[in] file File descriptor.
 * @param[in] ptr Location of buffer to place characters.
 * @param[in] len Length of buffer.
 *
 * @return Number of characters written, or -1 for error.
 *
 * @note Assumes non-blocking operation.
 */
int _read(int file, char* ptr, int len)
{
    int rc = 0;
    char c;
    enum ttys_instance_id instance_id = fd_to_instance(file);

    if (instance_id >= TTYS_NUM_INSTANCES) {
        errno = EBADF;
        return -1;
    }

    if (ttys_states[instance_id].rx_buf_get_idx ==
        ttys_states[instance_id].rx_buf_put_idx) {
        errno = EAGAIN;
        rc = -1;
    } else {
        while (rc < len && ttys_getc(instance_id, &c)) {
            *ptr++ = c;
            rc++;
        }
    }

    return rc;
}
