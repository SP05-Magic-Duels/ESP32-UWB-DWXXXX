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


void SPI_ConfigFastRate(uint16_t scalingfactor)
{
	// which SPI to use?

    // Call de-init?

   


    // Initialize the SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
  
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret); // Not sure what exactly this does

    // Add DW1000 as a device

    spi_device_interface_config_t devcfg = {
        // .command_bits = 10,
        .clock_speed_hz = EEPROM_CLK_FREQ,
        .mode = 0,          //SPI mode 0
        /*
         * The timing requirements to read the busy signal from the EEPROM cannot be easily emulated
         * by SPI transactions. We need to control CS pin by SW to check the busy signal manually.
         */
        .spics_io_num = -1,
        .queue_size = 1,
        // .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_POSITIVE_CS,
        // .pre_cb = cs_high,
        // .post_cb = cs_low,
        // .input_delay_ns = EEPROM_INPUT_DELAY_NS,  //the EEPROM output the data half a SPI clock behind.
    };

    //Attach the EEPROM to the SPI bus
    err = spi_bus_add_device(/* ctx->cfg.host, &devcfg, &ctx->spi */);
    

    
    // SPI_InitTypeDef SPI_InitStructure;

	// SPI_I2S_DeInit(SPIx);


	// // SPIx Mode setup
	// SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	// SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	// SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	// SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	 //
	// //SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //
	// SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	// //SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //
	// //SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	// SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	// SPI_InitStructure.SPI_BaudRatePrescaler = scalingfactor; //sets BR[2:0] bits - baudrate in SPI_CR1 reg bits 4-6
	// SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	// SPI_InitStructure.SPI_CRCPolynomial = 7;

	// SPI_Init(SPIx, &SPI_InitStructure);

	// // Enable SPIx
	// SPI_Cmd(SPIx, ENABLE);
}
