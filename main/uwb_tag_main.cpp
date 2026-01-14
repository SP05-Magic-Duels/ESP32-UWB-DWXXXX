/*
PUT THIS BACK IN EXAMPLE AND REVERT CMAKE TO LOOK FOR main.c ?
uwb_tag.cpp

For ESP32 UWB or ESP32 UWB Pro
Adapted from Makerfabs -> example/tag/uwb_tag described in Caroline's notes

*/

// #include <SPI.h>
// #include "DW1000Ranging.h"
#include "..\components\adapated_DW1000\src\DW1000.h"
#include "..\components\adapated_DW1000\src\DW1000Ranging.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

#define MAIN_TAG "MAIN"

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin



void newRange()
{
    ESP_LOGI(MAIN_TAG, "from: ");
    ESP_LOGI(MAIN_TAG, "%X", DW1000Ranging.getDistantDevice()->getShortAddress());
    ESP_LOGI(MAIN_TAG, "\t Range: ");
    ESP_LOGI(MAIN_TAG, "%D", DW1000Ranging.getDistantDevice()->getRange());
    ESP_LOGI(MAIN_TAG, " m");
    ESP_LOGI(MAIN_TAG, "\t RX power: ");
    ESP_LOGI(MAIN_TAG, "%D", DW1000Ranging.getDistantDevice()->getRXPower());
    ESP_LOGI(MAIN_TAG, " dBm");
}

void newDevice(DW1000Device *device)
{
    ESP_LOGI(MAIN_TAG, "ranging init; 1 device added ! -> ");
    ESP_LOGI(MAIN_TAG, " short:");
    ESP_LOGI(MAIN_TAG, "%X", device->getShortAddress());
}

void inactiveDevice(DW1000Device *device)
{
    ESP_LOGI(MAIN_TAG, "delete inactive device: ");
    ESP_LOGI(MAIN_TAG, "%X", device->getShortAddress());
}

void setup()
{
    // Serial.begin(115200);
    // delay(1000);      
    vTaskDelay(pdMS_TO_TICKS(1000));
    //init the configuration
    // SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
    //define the sketch as anchor. It will be great to dynamically change the type of module
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    //Enable the filter to smooth the distance
    //DW1000Ranging.useRangeFilter(true);

    //we start the module as a tag
    char* tag_address = "7D:00:22:EA:82:60:3B:9C";
    DW1000Ranging.startAsTag(tag_address, DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
}

void app_main()
{
    setup();

    while(1) 
    {
        DW1000Ranging.loop();
    }
}


#ifdef __cplusplus
}
#endif
