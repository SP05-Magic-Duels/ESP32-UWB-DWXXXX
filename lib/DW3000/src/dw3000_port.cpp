/*
 * dw3000_port.cpp
 *
 * Converted for ESP-IDF SPI API usage.
 */ 

#ifdef __cplusplus
extern "C" {
#endif

// --- ESP-IDF and Standard C Includes ---
#include "dw3000_port.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h" // For esp_rom_delay_us
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <stddef.h> // For size_t

static const char* TAG = "DW3000_PORT";

// --- Global Variables (Adapted for ESP-IDF) ---

// SPI device handle for the DW3000 chip
spi_device_handle_t dw3000_spi_device = NULL;

uint8_t _ss;
uint8_t _rst;
uint8_t _irq;

// SPI config parameters (ESP-IDF style)
#define SPI_HOST_ID       (SPI2_HOST) // Using SPI2 (VSPI) host
#define SPI_FAST_RATE_HZ  (8000000)
#define SPI_SLOW_RATE_HZ  (2000000)

// Placeholder GPIOs for SPI bus (DEFINE THESE IN YOUR PROJECT!)
// Example hardcoded values for clarity, but use sdkconfig or constants
// NOTE: Make sure these pins are correct for your chosen SPI_HOST_ID (SPI2_HOST)
#define CONFIG_SPI_MOSI_GPIO (GPIO_NUM_23) 
#define CONFIG_SPI_MISO_GPIO (GPIO_NUM_19)
#define CONFIG_SPI_SCLK_GPIO (GPIO_NUM_18)


// Register Caches (Now using uint8_t and constants from header)
uint8_t _syscfg[LEN_SYS_CFG];
uint8_t _sysctrl[LEN_SYS_CTRL];
uint8_t _sysstatus[LEN_SYS_STATUS];
uint8_t _txfctrl[LEN_TX_FCTRL];
uint8_t _sysmask[LEN_SYS_MASK];
uint8_t _chanctrl[LEN_CHAN_CTRL];

uint8_t _deviceMode;
uint8_t _vmeas3v3;
uint8_t _tmeas23C;
uint8_t _networkAndAddress[LEN_PANADR];

bool _debounceClockEnabled = false;

// --- Time and Utility Functions (Replaced Arduino functions) ---

void enableDebounceClock() {
    uint8_t pmscctrl0[LEN_PMSC_CTRL0];
    memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
    readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    pmscctrl0[GPDCE_BIT/8] |= (1 << (GPDCE_BIT%8)); // setBit(..., GPDCE_BIT, 1)
    pmscctrl0[KHZCLKEN_BIT/8] |= (1 << (KHZCLKEN_BIT%8)); // setBit(..., KHZCLKEN_BIT, 1)
    writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    _debounceClockEnabled = true;
}

void sleepms(uint32_t x)
{
  vTaskDelay(pdMS_TO_TICKS(x));
}

int sleepus(uint32_t x)
{
  esp_rom_delay_us(x);
  return 0;
}

void deca_sleep(uint8_t time_ms)
{
  sleepms(time_ms);
}

void deca_usleep(uint8_t time_us)
{
  sleepus(time_us);
}


// --- ESP-IDF SPI & GPIO Implementation ---

// Initializes the GPIOs and the SPI bus host
void spiBegin(uint8_t irq, uint8_t rst)
{
    _rst = rst;
    _irq = irq;

    // 1. Initialize GPIOs
    gpio_set_direction((gpio_num_t)irq, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)rst, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)rst, 1);
    
    // 2. Initialize the SPI bus (Host)
    const spi_bus_config_t buscfg = {
        // --- These must be in the exact order as declared in the C header ---
        .mosi_io_num = CONFIG_SPI_MOSI_GPIO, // (or data0_io_num)
        .miso_io_num = CONFIG_SPI_MISO_GPIO, // (or data1_io_num)
        .sclk_io_num = CONFIG_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,                 // (or data2_io_num)
        .quadhd_io_num = -1,                 // (or data3_io_num)
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .data_io_default_level = false, // <-- Fix: Corrected position and initialized
        .max_transfer_sz = 0,           // <-- Fix: Corrected position
        .flags = 0,                     // <-- Fix: Corrected position and initialized
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO, // <-- Fix: Corrected position and initialized
        .intr_flags = 0,                // <-- Fix: Corrected position and initialized
    };
    
    // Initialize the bus (Host ID)
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO));
    
    sleepms(5); // Initial delay
}

// Selects the slave device (called internally when adding the device)
void reselect(uint8_t ss) {
    _ss = ss;
    gpio_set_direction((gpio_num_t)_ss, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)_ss, 1); // CS starts HIGH (inactive)
}

// Helper to remove the existing device and re-add it with a new speed
static void change_spi_speed(int speed_hz) {
    if (dw3000_spi_device != NULL) {
        spi_bus_remove_device(dw3000_spi_device);
        dw3000_spi_device = NULL;
    }
    
    const spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0, // SPI_MODE0
        .clock_source = SPI_CLK_SRC_DEFAULT,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = speed_hz,
        .input_delay_ns = 0,
        .sample_point = SPI_SAMPLING_POINT_PHASE_0,    // <-- Fix: Corrected position
        .spics_io_num = (int)_ss,
        .flags = 0,
        .queue_size = 7,
        .pre_cb = NULL,       // <-- Fix: Callbacks must come before sample_point
        .post_cb = NULL,      // <-- Fix: Callbacks must come before sample_point
    };
    
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_ID, &devcfg, &dw3000_spi_device));
}

void readBytes(uint8_t cmd, uint16_t offset, uint8_t data[], uint16_t n) {
    uint8_t header[3];
    uint8_t headerLen = 1;
    
    // Build SPI header
    if(offset == NO_SUB) {
        header[0] = READ | cmd;
    } else {
        header[0] = READ_SUB | cmd;
        if(offset < 128) {
            header[1] = (uint8_t)offset;
            headerLen++;
        } else {
            header[1] = RW_SUB_EXT | (uint8_t)offset;
            header[2] = (uint8_t)(offset >> 7);
            headerLen += 2;
        }
    }

    // 1. Send Header (Fix: Explicitly initialize all fields)
    spi_transaction_t header_t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)headerLen * 8, 
        .rxlength = 0,
        .override_freq_hz = 0,  // <-- Fix: Added missing field and positioned correctly
        .user = NULL,           // <-- Fix: Moved user field up
        .tx_buffer = header,
        .rx_buffer = NULL,
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &header_t));

    // 2. Read Data (Fix: Explicitly initialize all fields)
    spi_transaction_t data_t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)n * 8,          // Fix: Narrowing conversion
        .rxlength = (size_t)n * 8,        // Fix: Narrowing conversion
        .override_freq_hz = 0,
        .user = NULL,
        .tx_buffer = NULL,
        .rx_buffer = data,
    };
    
    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &data_t));
    esp_rom_delay_us(5); // Replaces delayMicroseconds(5)
}

void readBytesOTP(uint16_t address, uint8_t data[]) {
  uint8_t addressBytes[LEN_OTP_ADDR];
  
  // p60 - 6.3.3 Reading a value from OTP memory
  // bytes of address
  addressBytes[0] = (address & 0xFF);
  addressBytes[1] = ((address >> 8) & 0xFF);
  // set address
  writeBytes(OTP_IF, OTP_ADDR_SUB, addressBytes, LEN_OTP_ADDR);
  // switch into read mode
  writeByte(OTP_IF, OTP_CTRL_SUB, 0x03); // OTPRDEN | OTPREAD
  writeByte(OTP_IF, OTP_CTRL_SUB, 0x01); // OTPRDEN
  // read value/block - 4 bytes
  readBytes(OTP_IF, OTP_RDAT_SUB, data, LEN_OTP_RDAT);
  // end read mode
  writeByte(OTP_IF, OTP_CTRL_SUB, 0x00);
}

void writeByte(uint8_t cmd, uint16_t offset, uint8_t data) {
  writeBytes(cmd, offset, &data, 1);
}

void writeBytes(uint8_t cmd, uint16_t offset, uint8_t data[], uint16_t data_size) {
    uint8_t header[3];
    uint8_t headerLen = 1;
    
    // Build SPI header
    if(offset == NO_SUB) {
        header[0] = WRITE | cmd;
    } else {
        header[0] = WRITE_SUB | cmd;
        if(offset < 128) {
            header[1] = (uint8_t)offset;
            headerLen++;
        } else {
            header[1] = RW_SUB_EXT | (uint8_t)offset;
            header[2] = (uint8_t)(offset >> 7);
            headerLen += 2;
        }
    }

    // Combine header and data into a single buffer
    uint8_t tx_buffer[headerLen + data_size];
    memcpy(tx_buffer, header, headerLen);
    memcpy(tx_buffer + headerLen, data, data_size);

    // Fix: Explicitly initialize all fields
    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)(headerLen + data_size) * 8, // Fix: Narrowing conversion
        .rxlength = 0,
        .override_freq_hz = 0,
        .user = NULL,
        .tx_buffer = tx_buffer,
        .rx_buffer = NULL,
        
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &t));
    esp_rom_delay_us(5); // Replaces delayMicroseconds(5)
}

void enableClock(uint8_t clock) {
    uint8_t pmscctrl0[LEN_PMSC_CTRL0];
    memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
    readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
    
    if(clock == AUTO_CLOCK || clock == PLL_CLOCK) {
        change_spi_speed(SPI_FAST_RATE_HZ);
        pmscctrl0[0] &= 0xFC;
        pmscctrl0[0] |= PLL_CLOCK;
    } else if(clock == XTI_CLOCK) {
        change_spi_speed(SPI_SLOW_RATE_HZ);
        pmscctrl0[0] &= 0xFC;
        pmscctrl0[0] |= XTI_CLOCK;
    } else {
        ESP_LOGW(TAG, "Unknown clock setting: %d", clock);
    }
    writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
}

void reset() {
  if(_rst == 0xff) {
    softReset();
  } else {
    // dw1000 data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
    gpio_set_direction((gpio_num_t)_rst, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)_rst, 0);
    sleepms(2);
    gpio_set_direction((gpio_num_t)_rst, GPIO_MODE_INPUT); // leave floating
    sleepms(10);
    idle();
  }
}

void softReset() {
  uint8_t pmscctrl0[LEN_PMSC_CTRL0];
  readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
  pmscctrl0[0] = 0x01;
  writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
  pmscctrl0[3] = 0x00;
  writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
  sleepms(10);
  pmscctrl0[0] = 0x00;
  pmscctrl0[3] = 0xF0;
  writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
  idle();
}

// Replaced Arduino bitSet macro
void setBit(uint8_t data[], uint16_t n, uint16_t bit, bool val) {
  uint16_t idx;
  uint8_t shift;
  
  idx = bit/8;
  if(idx >= n) {
    return;
  }
  uint8_t* targetByte = &data[idx];
  shift = bit%8;
  
  if(val) {
    *targetByte |= (1 << shift); // Replaced bitSet
  } else {
    *targetByte &= ~(1 << shift); // Replaced bitClear
  }
}

// Replaced Arduino bitRead macro
bool getBit(uint8_t data[], uint16_t n, uint16_t bit) {
  uint16_t idx;
  uint8_t  shift;
  
  idx = bit/8;
  if(idx >= n) {
    return false;
  }
  uint8_t targetByte = data[idx];
  shift = bit%8;
  
  return (targetByte >> shift) & 0x01; // Replaced bitRead
}

void writeValueToBytes(uint8_t data[], int32_t val, uint16_t n) {
  uint16_t i;
  for(i = 0; i < n; i++) {
    data[i] = ((val >> (i*8)) & 0xFF);
  }
}

void readSystemConfigurationRegister() {
  readBytes(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
}

void writeSystemConfigurationRegister() {
  writeBytes(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
}

void readSystemEventStatusRegister() {
  readBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void readNetworkIdAndDeviceAddress() {
  readBytes(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
}

void writeNetworkIdAndDeviceAddress() {
  writeBytes(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
}

void readSystemEventMaskRegister() {
  readBytes(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
}

void writeSystemEventMaskRegister() {
  writeBytes(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
}

void readChannelControlRegister() {
  readBytes(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
}

void writeChannelControlRegister() {
  writeBytes(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
}

void readTransmitFrameControlRegister() {
  readBytes(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
}

void writeTransmitFrameControlRegister() {
  writeBytes(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
}

void idle() {
  memset(_sysctrl, 0, LEN_SYS_CTRL);
  setBit(_sysctrl, LEN_SYS_CTRL, TRXOFF_BIT, true);
  _deviceMode = IDLE_MODE;
  writeBytes(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
}

void setDoubleBuffering(bool val) {
  setBit(_syscfg, LEN_SYS_CFG, DIS_DRXB_BIT, !val);
}

void setInterruptPolarity(bool val) {
  setBit(_syscfg, LEN_SYS_CFG, HIRQ_POL_BIT, val);
}

void clearInterrupts() {
  memset(_sysmask, 0, LEN_SYS_MASK);
}

void manageLDE() {
  // ... (implementation remains the same, assuming setBit/readBytes/writeBytes are now correct) ...
  uint8_t ldoTune[LEN_OTP_RDAT];
  readBytesOTP(0x04, ldoTune); // TODO #define
  if(ldoTune[0] != 0) {
    // TODO tuning available, copy over to RAM: use OTP_LDO bit
  }
  // tell the chip to load the LDE microcode
  uint8_t pmscctrl0[LEN_PMSC_CTRL0];
  uint8_t otpctrl[LEN_OTP_CTRL];
  memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
  memset(otpctrl, 0, LEN_OTP_CTRL);
  readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
  readBytes(OTP_IF, OTP_CTRL_SUB, otpctrl, LEN_OTP_CTRL);
  pmscctrl0[0] = 0x01;
  pmscctrl0[1] = 0x03;
  otpctrl[0]   = 0x00;
  otpctrl[1]   = 0x80;
  writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
  writeBytes(OTP_IF, OTP_CTRL_SUB, otpctrl, 2);
  sleepms(5);
  pmscctrl0[0] = 0x00;
  pmscctrl0[1] &= 0x02;
  writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
}

void Sleep(uint32_t d) {
    sleepms(d);
}

void spiSelect(uint8_t ss) {
  reselect(ss);
  enableClock(AUTO_CLOCK);
  sleepms(5);
  if(_rst != 0xff) {
    gpio_set_direction((gpio_num_t)_rst, GPIO_MODE_INPUT);
  }
  reset();
  
  writeValueToBytes(_networkAndAddress, 0xFF, LEN_PANADR);
  writeNetworkIdAndDeviceAddress();
  
  memset(_syscfg, 0, LEN_SYS_CFG);
  setDoubleBuffering(false);
  setInterruptPolarity(true);
  writeSystemConfigurationRegister();
  
  clearInterrupts();
  writeSystemEventMaskRegister();
  
  enableClock(XTI_CLOCK);
  sleepms(5);
  manageLDE();
  sleepms(5);
  enableClock(AUTO_CLOCK);
  sleepms(5);
  
  uint8_t buf_otp[4];
  readBytesOTP(0x008, buf_otp);
  _vmeas3v3 = buf_otp[0];
  readBytesOTP(0x009, buf_otp);
  _tmeas23C = buf_otp[0];
}

int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readLength, uint8_t *readBuffer)
{
    // 1. Send Header
    spi_transaction_t header_t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)headerLength * 8, // Fix: Narrowing conversion
        .rxlength = 0,
        .override_freq_hz = 0,
        .user = NULL,
        .tx_buffer = headerBuffer,
        .rx_buffer = NULL,
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &header_t));

    // 2. Read Body
    spi_transaction_t read_t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)readLength * 8,       // Fix: Narrowing conversion
        .rxlength = (size_t)readLength * 8,     // Fix: Narrowing conversion
        .override_freq_hz = 0,
        .user = NULL,
        .tx_buffer = NULL,
        .rx_buffer = readBuffer,
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &read_t));
    esp_rom_delay_us(5);
    return 0;
}

int writetospi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t bodyLength, uint8_t *bodyBuffer)
{
    // Combine header and body into a single transaction buffer
    uint8_t tx_buffer[headerLength + bodyLength];
    memcpy(tx_buffer, headerBuffer, headerLength);
    memcpy(tx_buffer + headerLength, bodyBuffer, bodyLength);

    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size_t)(headerLength + bodyLength) * 8, // Fix: Narrowing conversion
        .rxlength = 0,
        .override_freq_hz = 0,
        .user = NULL,
        .tx_buffer = tx_buffer,
        .rx_buffer = NULL,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(dw3000_spi_device, &t));
    esp_rom_delay_us(5);
    return 0;
}

void wakeup_device_with_io() {
    // In ESP-IDF, we control CS via spi_device_polling_transmit, but this function
    // is often used to toggle CS manually to wake the chip.
    // We temporarily use GPIO calls for this specific sequence.
    gpio_set_level((gpio_num_t)_ss, 0);
    sleepms(2);
    gpio_set_level((gpio_num_t)_ss, 1);
    if (_debounceClockEnabled){
        enableDebounceClock();
    }
}


void port_set_dw_ic_spi_fastrate(uint8_t irq, uint8_t rst, uint8_t ss) {
    spiBegin(irq, rst);
    reselect(ss);
    change_spi_speed(SPI_FAST_RATE_HZ);
}

// --- Placeholder Interrupt Functions (Require ESP-IDF ISR setup) ---
// NOTE: These function definitions require returning a value (decaIrqStatus_t / uint32_t)

uint32_t port_GetEXT_IRQStatus(void) {
    return 0; // Placeholder
}

uint32_t port_CheckEXT_IRQ(void) {
    return 0; // Placeholder
}

void port_DisableEXT_IRQ(void) {
    // Requires ESP-IDF gpio_intr_disable()
}

void port_EnableEXT_IRQ(void) {
    // Requires ESP-IDF gpio_intr_enable()
}

/* DW IC IRQ handler definition. */
static port_dwic_isr_t port_dwic_isr = NULL;

void port_set_dwic_isr(port_dwic_isr_t dwic_isr)
{
    portDISABLE_INTERRUPTS(); // FreeRTOS macro to disable
    port_dwic_isr = dwic_isr;
    portENABLE_INTERRUPTS(); // FreeRTOS macro to enable
}

#ifdef __cplusplus
}
#endif
