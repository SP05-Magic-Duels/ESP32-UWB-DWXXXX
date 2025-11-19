/*
 * UART.c
 *
 * Created: 9/10/2021 12:32:14 PM
 *  Author: Emin Eminof
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include "dw3000_uart.h"

void UART_init(void)
{
  // Configure UART parameters
  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };

  // Install UART driver
  uart_driver_install(UART_PORT_NUM, 1024, 0, 0, NULL, 0);
  uart_param_config(UART_PORT_NUM, &uart_config);
  uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Send text to serial monitor
  const char *msg = "Hello from ESP32 via UART!\n";
  uart_write_bytes(UART_PORT_NUM, msg, strlen(msg));

  // You can also use ESP-IDF logging
  ESP_LOGI("UART_EXAMPLE", "This is printed via ESP_LOGI");
}

void UART_puts(char* s)
{
  uart_write_bytes(UART_PORT_NUM, s, strlen(s));
}

void test_run_info(unsigned char * s)
{
    UART_puts((char *)s);
    UART_puts("\r\n");
}

#ifdef __cplusplus
}
#endif
