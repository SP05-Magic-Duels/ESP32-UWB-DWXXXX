#include "dw3000.h"

#define APP_NAME "UWB BEST RX v1.0"

// Pins
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

/* Default DW3000 config - sensible defaults for receiving short frames */
static dwt_config_t config = {
    5, DWT_PLEN_128, DWT_PAC8, 9, 9, 1,
    DWT_BR_6M8, DWT_PHRMODE_STD, DWT_PHRRATE_STD,
    (129 + 8 - 8), DWT_STS_MODE_OFF, DWT_STS_LEN_64, DWT_PDOA_M0
};

/* Antenna delay defaults (calibrate per-device for production) */
#define RX_ANT_DLY 16385

#define RX_BUF_LEN 128
static uint8_t rx_buffer[RX_BUF_LEN];

// Receive helper that returns frame length or 0 on timeout/error
static uint16_t uwb_receive(uint8_t *out_buf, uint16_t buf_size, uint32_t timeout_ms)
{
    memset(out_buf, 0, buf_size);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    uint32_t start = millis();
    uint32_t status;
    while (!((status = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
        if ((millis() - start) > timeout_ms) {
            // timeout
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            return 0;
        }
    }

    if (status & SYS_STATUS_RXFCG_BIT_MASK) {
        uint16_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
        if (frame_len <= buf_size) {
            dwt_readrxdata(out_buf, frame_len - FCS_LEN, 0);
        }
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        return frame_len;
    } else {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return 0;
    }
}

void setup()
{
    UART_init();
    test_run_info((unsigned char *)APP_NAME);

    spiBegin(PIN_IRQ, PIN_RST);
    spiSelect(PIN_SS);
    delay(2);

    while (!dwt_checkidlerc()) { UART_puts("IDLE FAILED\r\n"); }
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) { UART_puts("INIT FAILED\r\n"); while (1); }
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    if (dwt_configure(&config)) { UART_puts("CONFIG FAILED\r\n"); while (1); }
    dwt_setrxantennadelay(RX_ANT_DLY);
}

void loop()
{
    uint16_t rlen = uwb_receive(rx_buffer, sizeof(rx_buffer), 2000);
    if (rlen) {
        test_run_info((unsigned char *)"RX OK");
        // Optionally, read RX timestamp for precise timing
        uint32_t rx_ts = dwt_readrxtimestamplo32();
        // Print short info
        char info[64];
        snprintf(info, sizeof(info), "LEN:%u TS:0x%08X", rlen, rx_ts);
        test_run_info((unsigned char *)info);
    } else {
        test_run_info((unsigned char *)"RX TIMEOUT/ERR");
    }
    Sleep(100);
}
