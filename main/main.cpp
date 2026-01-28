/*
PUT THIS BACK IN EXAMPLE AND REVERT CMAKE TO LOOK FOR main.c ?
uwb_tag.cpp

For ESP32 UWB or ESP32 UWB Pro
Adapted from Makerfabs -> example/tag/uwb_tag described in Caroline's notes

*/
#include <stdint.h>

// #include "esp_log.h"

#include "DW1000Ranging.h"
#include "DW1000.h"

// Indicate which device configuration to compile and flash
#define DEVICE_TAG 1
#define DEVICE_ANCHOR 0

// #define SPI_SCK 18
// #define SPI_MISO 19
// #define SPI_MOSI 23
// #define SPI_CS 4

#if DEVICE_TAG
#define MAIN_TAG "TAG_MAIN"
#elif DEVICE_ANCHOR
#define MAIN_TAG "ANCHOR_MAIN"
#else
#error "Must define either DEVICE_TAG or DEVICE_ANCHOR"
#endif

// Connection pins
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

extern "C" void newRange()
{
    ESP_LOGI(MAIN_TAG, "from: ");
    ESP_LOGI(MAIN_TAG, "%x", DW1000Ranging.getDistantDevice()->getShortAddress());
    ESP_LOGI(MAIN_TAG, "\t Range: ");
    ESP_LOGI(MAIN_TAG, "%lf", DW1000Ranging.getDistantDevice()->getRange());
    ESP_LOGI(MAIN_TAG, " m");
    ESP_LOGI(MAIN_TAG, "\t RX power: ");
    ESP_LOGI(MAIN_TAG, "%lf", DW1000Ranging.getDistantDevice()->getRXPower());
    ESP_LOGI(MAIN_TAG, " dBm");
}

extern "C" void newDevice(DW1000Device *device)
{
    ESP_LOGI(MAIN_TAG, "ranging init; 1 device added ! -> ");
    ESP_LOGI(MAIN_TAG, " short:");
    ESP_LOGI(MAIN_TAG, "%x", device->getShortAddress());
}

extern "C" void inactiveDevice(DW1000Device *device)
{
    ESP_LOGI(MAIN_TAG, "delete inactive device: ");
    ESP_LOGI(MAIN_TAG, "%x", device->getShortAddress());
}

extern "C" void setup(void)
{
    vTaskDelay(pdMS_TO_TICKS(1));
    // init the configuration
    //  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); // Reset, CS, IRQ pin
    // define the sketch as anchor. It will be great to dynamically change the type of module

    // DW1000Ranging.attachNewRange(newRange);
    // DW1000Ranging.attachNewDevice(newDevice);
    // DW1000Ranging.attachInactiveDevice(inactiveDevice);

    // Enable the filter to smooth the distance
    // DW1000Ranging.useRangeFilter(true);

    // we start the module as a tag
    char *tag_address = "7D:00:22:EA:82:60:3B:9C";
    // DW1000Ranging.startAsTag(tag_address, DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
}

/**
 *
 * TAG
 *
 */

#if DEVICE_TAG

extern "C" void tag_loop(void *pvParameters)
{
    while (1)
    {
        DW1000Ranging.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void app_main(void)
{
    setup();

    // xTaskCreate(&tag_loop,  // Function/ task
    //             "tag_loop", // Name of the task (for human readability)
    //             2048,       // Stack size (bytes)
    //             NULL,       // &ucParameterToPass
    //             1,          // Priority
    //             NULL        // &xHandle
    // );
    return;
}
#endif

/**
 *
 * ANCHOR
 *
 */

#if DEVICE_ANCHOR

extern "C" void app_main(void)
{
    setup();

    while (1)
    {
        DW1000Ranging.loop();
    }

    return;
}
#endif