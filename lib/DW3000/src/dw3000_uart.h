/*
 * UART.h
 *
 * Created: 9/10/2021 12:32:28 PM
 *  Author: Emin Eminof
 */ 

#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dw3000.h"

#define UART_PORT_NUM      UART_NUM_0   // Default UART for serial monitor
#define UART_BAUD_RATE     115200
#define UART_TX_PIN        1            // GPIO1 (TX0)
#define UART_RX_PIN        3            // GPIO3 (RX0)

void UART_init(void);
void UART_puts(char* s);

void test_run_info(unsigned char * s);

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
