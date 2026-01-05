#include "dw3000.h"
#include "dw3000_mac_802_15_4.h"

#define APP_NAME "UWB BEST RX SECURE v1.0"

// Pins
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

/* Default DW3000 config */
static dwt_config_t config = {
    5, DWT_PLEN_128, DWT_PAC8, 9, 9, 1,
    DWT_BR_6M8, DWT_PHRMODE_STD, DWT_PHRRATE_STD,
    (129 + 8 - 8), DWT_STS_MODE_OFF, DWT_STS_LEN_64, DWT_PDOA_M0
};

/* Antenna delays */
#define RX_ANT_DLY 16385

/* AES setup */
static dwt_aes_config_t aes_config = {
  AES_key_RAM,
  AES_core_type_CCM,
  MIC_0,
  AES_KEY_Src_Register,
  AES_KEY_Load,
  0,
  AES_KEY_128bit,
  AES_Encrypt
};

static dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS] = {
    {0x00010203,0x04050607,0x08090A0B,0x0C0D0E0F,0,0,0,0},
    {0x11223344,0x55667788,0x99AABBCC,0xDDEEFF00,0,0,0,0},
};

#define RESPONDER_KEY_INDEX 1

/* receive buffer */
#define RX_BUF_LEN 127
static uint8_t rx_buffer[RX_BUF_LEN];

/* AES job for RX */
dwt_aes_job_t aes_job_rx;

// Helper: receive and attempt AES decryption; returns payload length or 0 on fail
static uint16_t secure_receive_and_decrypt(uint8_t *out_buf, uint16_t buf_size, uint32_t timeout_ms)
{
    memset(out_buf, 0, buf_size);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    uint32_t start = millis();
    uint32_t status;
    while (!((status = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
        if ((millis() - start) > timeout_ms) {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            return 0;
        }
    }

    if (status & SYS_STATUS_RXFCG_BIT_MASK) {
        uint16_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (frame_len > sizeof(rx_buffer)) {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
            return 0;
        }
        /* read MHR first into rx_buffer so mac helpers can parse it */
        dwt_readrxdata(rx_buffer, frame_len, 0);

        /* prepare AES job for decrypting from RX buffer to out_buf */
        aes_job_rx.mode = AES_Decrypt;
        aes_job_rx.src_port = AES_Src_Rx_buf_0;
        aes_job_rx.dst_port = AES_Dst_Rx_buf_0;
        aes_job_rx.nonce = NULL; /* rx helper will derive nonce from MHR */
        aes_job_rx.header = (uint8_t *)MHR_802_15_4_PTR(&mac_frame_802_15_4_format_t{}); /* placeholder, rx helper will inspect rx_buffer */
        aes_job_rx.header_len = 0; /* will be set by rx_aes_802_15_4 */
        aes_job_rx.payload = out_buf;
        aes_job_rx.payload_len = buf_size;

        /* set key in AES engine (responder key index assumed) */
        dwt_set_keyreg_128(&keys_options[RESPONDER_KEY_INDEX-1]);
        aes_config.mode = AES_Decrypt;
        dwt_configure_aes(&aes_config);

        /* Use rx_aes_802_15_4 helper which will parse header, nonce and decrypt into rx_buffer/out_buf */
        int8_t res = rx_aes_802_15_4((mac_frame_802_15_4_format_t *)rx_buffer, frame_len, &aes_job_rx, buf_size, keys_options, 0x1122334455667788ULL, 0x8877665544332211ULL, &aes_config);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        if (res == AES_RES_OK) {
            return aes_job_rx.payload_len;
        }
        return 0;
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
    uint8_t out[64];
    uint16_t plen = secure_receive_and_decrypt(out, sizeof(out), 2000);
    if (plen) {
        test_run_info((unsigned char *)"SECURE RX OK");
        // print payload
        test_run_info((unsigned char *)out);
    } else {
        test_run_info((unsigned char *)"SECURE RX TIMEOUT/ERR");
    }
    Sleep(200);
}
