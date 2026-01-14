/*
 * UART.c
 *
 * Created: 9/10/2021 12:32:14 PM
 *  Author: Emin Eminof
 */
#include "esp_log.h"
#include "dw3000_uart.h"

static const char *UART0_TAG = "UART0_ESPLOG";

void UART_init(void)
{
	return;
}

void UART_putc(char data)
{
	ESP_LOGI(UART0_TAG, "%c", data);
}

void UART_puts(char *s)
{
	ESP_LOGI(UART0_TAG, "%s", s);
}

void test_run_info(unsigned char *s)
{
	UART_puts((char *)s);
	UART_puts("\r\n");
}
