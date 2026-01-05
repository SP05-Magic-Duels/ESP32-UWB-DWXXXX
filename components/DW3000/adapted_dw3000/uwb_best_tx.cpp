#include "dw3000.h"

#define APP_NAME "UWB BEST TX v1.0"

// Pins (match other examples)
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

/* Default DW3000 config - sensible defaults for ranging and short frames */
static dwt_config_t config = {
    5, DWT_PLEN_128, DWT_PAC8, 9, 9, 1,
    DWT_BR_6M8, DWT_PHRMODE_STD, DWT_PHRRATE_STD,
    (129 + 8 - 8), DWT_STS_MODE_OFF, DWT_STS_LEN_64, DWT_PDOA_M0
};

/* Antenna delay defaults (calibrate per-device for production) */
#define TX_ANT_DLY 16385

extern dwt_txconfig_t txconfig_options;

// Simple blink/payload
static uint8_t tx_msg[] = {0xC5, 0, 'U', 'W', 'B', '_', 'T', 'X'};
#define TX_SN_IDX 1
#define FRAME_LEN (sizeof(tx_msg) + FCS_LEN)

// Robust transmit helper with timeout and optional response flag
static bool uwb_transmit(const uint8_t *data, uint16_t len, bool expect_response = false)
{
    uint32_t start_ms = millis();
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(len - FCS_LEN, (uint8_t *)data, 0);
    dwt_writetxfctrl(len, 0, expect_response ? 1 : 0);

    uint32_t mode = DWT_START_TX_IMMEDIATE | (expect_response ? DWT_RESPONSE_EXPECTED : 0);
    dwt_starttx(mode);

    // wait for TXFRS or timeout (2000 ms)
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK)) {
        if ((millis() - start_ms) > 2000) {
            // timeout
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
            return false;
        }
    }
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    return true;
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
    dwt_configuretxrf(&txconfig_options);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
}

void loop()
{
    tx_msg[TX_SN_IDX]++;
    bool ok = uwb_transmit(tx_msg, FRAME_LEN, false);
    if (ok) test_run_info((unsigned char *)"TX OK"); else test_run_info((unsigned char *)"TX TIMEOUT");
    Sleep(500);
}
