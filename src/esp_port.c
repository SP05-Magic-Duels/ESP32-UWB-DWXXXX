#include "driver/gpio.h"
#include "esp_port.h"
#include "freertos/FreeRTOS.h"

void hal_reset_dw1000(void) {
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS); // in PlatformIO for FreeRTOS?
    gpio_set_level(PIN_RST, 1);

    // MULoc sets the pin to an input, but I'm not sure if that's necessary
}

