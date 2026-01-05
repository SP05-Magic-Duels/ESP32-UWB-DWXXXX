// UART ESP32 implementation using standard output
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/spi_master/lcd/main/spi_master_example_main.c
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/spi_slave/receiver/main/app_main.c
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/spi_slave/sender/main/app_main.c

// https://krutarthpurohit.medium.com/implementing-spi-protocol-on-esp32-idf-5-1-version-6f2383af1c22

#include "dw3000_port.h"

static const char *TAG = "dw3000_port_idf";

static spi_device_handle_t s_spi = nullptr;
static int s_ss_pin = -1;
static int s_irq_pin = -1;
static int s_rst_pin = -1;

// Helper: perform a single SPI transaction with a tx buffer and read into rx (both optional)
static esp_err_t spi_txrx(const uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len)
{
    size_t total = tx_len + rx_len;
    if (total == 0) return ESP_OK;

    uint8_t *buf = (uint8_t *)heap_caps_malloc(total, MALLOC_CAP_DMA);
    uint8_t *rxbuf = nullptr;
    if (!buf) return ESP_ERR_NO_MEM;
    if (tx) memcpy(buf, tx, tx_len);
    if (rx_len) memset(buf + tx_len, 0x00, rx_len);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = total * 8; // bits
    t.tx_buffer = buf;
    rxbuf = (uint8_t *)heap_caps_malloc(total, MALLOC_CAP_DMA);
    if (!rxbuf) {
        heap_caps_free(buf);
        return ESP_ERR_NO_MEM;
    }
    t.rx_buffer = rxbuf;

    esp_err_t err = spi_device_transmit(s_spi, &t);
    if (err == ESP_OK && rx && rx_len) {
        // copy only rx portion (after header tx_len)
        memcpy(rx, rxbuf + tx_len, rx_len);
    }

    heap_caps_free(buf);
    heap_caps_free(rxbuf);
    return err;
}

void spiBegin(uint8_t irq, uint8_t rst, uint8_t ss)
{
    gpio_set_direction((gpio_num_t)irq, GPIO_MODE_INPUT);
    (void)rst;

    port_set_dw_ic_spi_fastrate(irq, rst, ss);
}

void port_set_dw_ic_spi_fastrate(uint8_t irq, uint8_t rst, uint8_t ss)
{
    // initialize SPI bus and add device using reasonable defaults for ESP32
    spi_bus_config_t buscfg = {
        .miso_io_num = DEFAULT_MISO,
        .mosi_io_num = DEFAULT_MOSI,
        .sclk_io_num = DEFAULT_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000, // 20 MHz safe default
        .mode = 0,
        .spics_io_num = ss,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI1_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI1_HOST, &devcfg, &s_spi));
    s_ss_pin = ss;
}

void reselect(uint8_t ss)
{
    s_ss_pin = ss;
    gpio_set_direction((gpio_num_t)s_ss_pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)s_ss_pin, 1);
}

void spiSelect(uint8_t ss)
{
    reselect(ss);
    // minimal reset/init flow retained from original: reset then basic config
    reset();
}

// Simplified readBytes: build header then perform one transaction returning the payload bytes
void readBytes(uint8_t cmd, uint16_t offset, uint8_t data[], uint16_t n)
{
    uint8_t header[3];
    uint8_t headerLen = 1;

    if (offset == NO_SUB) {
        header[0] = READ | cmd;
    } else {
        header[0] = READ_SUB | cmd;
        if (offset < 128) {
            header[1] = (uint8_t)offset;
            headerLen = 2;
        } else {
            header[1] = RW_SUB_EXT | (uint8_t)offset;
            header[2] = (uint8_t)(offset >> 7);
            headerLen = 3;
        }
    }

    spi_txrx(header, headerLen, data, n);
}

void writeBytes(uint8_t cmd, uint16_t offset, uint8_t data[], uint16_t data_size)
{
    uint8_t header[3];
    uint8_t headerLen = 1;
    if (offset == NO_SUB) {
        header[0] = WRITE | cmd;
    } else {
        header[0] = WRITE_SUB | cmd;
        if (offset < 128) {
            header[1] = (uint8_t)offset;
            headerLen = 2;
        } else {
            header[1] = RW_SUB_EXT | (uint8_t)offset;
            header[2] = (uint8_t)(offset >> 7);
            headerLen = 3;
        }
    }

    // build tx buffer header + data and send
    size_t total = headerLen + data_size;
    uint8_t *tx = (uint8_t *)malloc(total);
    if (!tx) return;
    memcpy(tx, header, headerLen);
    memcpy(tx + headerLen, data, data_size);
    spi_txrx(tx, total, NULL, 0);
    free(tx);
}

void writeByte(uint8_t cmd, uint16_t offset, uint8_t data)
{
    writeBytes(cmd, offset, &data, 1);
}

// OTP read simplified: reuse writeBytes/readBytes sequence from original but with IDF SPI
void readBytesOTP(uint16_t address, uint8_t data[])
{
    uint8_t addressBytes[LEN_OTP_ADDR];
    addressBytes[0] = address & 0xFF;
    addressBytes[1] = (address >> 8) & 0xFF;
    writeBytes(OTP_IF, OTP_ADDR_SUB, addressBytes, LEN_OTP_ADDR);
    writeByte(OTP_IF, OTP_CTRL_SUB, 0x03);
    writeByte(OTP_IF, OTP_CTRL_SUB, 0x01);
    readBytes(OTP_IF, OTP_RDAT_SUB, data, LEN_OTP_RDAT);
    writeByte(OTP_IF, OTP_CTRL_SUB, 0x00);
}

// Simple wrappers used by higher-level code
int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readLength, uint8_t *readBuffer)
{
    return spi_txrx(headerBuffer, headerLength, readBuffer, readLength) == ESP_OK ? 0 : -1;
}

int writetospi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t bodyLength, uint8_t *bodyBuffer)
{
    // combine header and body in one tx
    size_t total = headerLength + bodyLength;
    uint8_t *tx = (uint8_t *)malloc(total);
    if (!tx) return -1;
    memcpy(tx, headerBuffer, headerLength);
    memcpy(tx + headerLength, bodyBuffer, bodyLength);
    esp_err_t err = spi_txrx(tx, total, NULL, 0);
    free(tx);
    return err == ESP_OK ? 0 : -1;
}

void reset()
{
    // If RST pin is defined, toggle it. Otherwise try softReset
    if (DD_RESET_PIN >= 0) {
        gpio_set_direction((gpio_num_t)DD_RESET_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)DD_RESET_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(2));
        gpio_set_level((gpio_num_t)DD_RESET_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else {
        softReset();
    }
}

void softReset()
{
    uint8_t pmscctrl0[LEN_PMSC_CTRL0];
    memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
    readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    pmscctrl0[0] = 0x01;
    writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    pmscctrl0[3] = 0x00;
    writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    vTaskDelay(pdMS_TO_TICKS(10));
    pmscctrl0[0] = 0x00;
    pmscctrl0[3] = 0xF0;
    writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    idle();
}

void idle()
{
    uint8_t buf[LEN_SYS_CTRL];
    memset(buf, 0, LEN_SYS_CTRL);
    setBit(buf, LEN_SYS_CTRL, TRXOFF_BIT, true);
    writeBytes(SYS_CTRL, NO_SUB, buf, LEN_SYS_CTRL);
}

void Sleep(uint32_t d)
{
    vTaskDelay(pdMS_TO_TICKS(d));
}

// Retain other utility functions in header - simple implementations
void setBit(uint8_t data[], uint16_t n, uint16_t bit, uint8_t val)
{
    uint16_t idx = bit / 8;
    if (idx >= n) return;
    uint8_t shift = bit % 8;
    if (val) data[idx] |= (1 << shift); else data[idx] &= ~(1 << shift);
}

uint8_t getBit(uint8_t data[], uint16_t n, uint16_t bit)
{
    uint16_t idx = bit / 8;
    if (idx >= n) return 0;
    uint8_t shift = bit % 8;
    return (data[idx] >> shift) & 0x1;
}

void writeValueToBytes(uint8_t data[], int32_t val, uint16_t n)
{
    for (uint16_t i = 0; i < n; ++i) data[i] = (val >> (i * 8)) & 0xFF;
}

// IRQ management stubs (user should implement if needed)
uint32_t port_GetEXT_IRQStatus(void) { return 0; }
uint32_t port_CheckEXT_IRQ(void) { return 0; }
void port_DisableEXT_IRQ(void) { }
void port_EnableEXT_IRQ(void) { }
