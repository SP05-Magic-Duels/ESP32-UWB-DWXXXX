#include "dw3000.h"
#include "dw3000_mac_802_15_4.h"

#define APP_NAME "UWB BEST TX SECURE v1.0"

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
#define TX_ANT_DLY 16385

/* AES setup copied/simplified from example */
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

/* Optional keys table */
static dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS] = {
    {0x00010203,0x04050607,0x08090A0B,0x0C0D0E0F,0,0,0,0},
    {0x11223344,0x55667788,0x99AABBCC,0xDDEEFF00,0,0,0,0},
};

#define INITIATOR_KEY_INDEX 1

/* A simple MAC frame template used for AES header (only MHR fields needed) */
mac_frame_802_15_4_format_t mac_frame = {
  {{0x09,0xEC}, 0x00, {0x21,0x43}, {0}, {0}, {0x0F, {0,0,0,0}, 0 }},
  0x00
};

/* Payload */
static uint8_t tx_payload[] = { 'S','E','C','U','R','E','T','X' };

/* AES job */
dwt_aes_job_t aes_job_tx;
uint8_t nonce[13];

extern dwt_txconfig_t txconfig_options;

static bool secure_tx_once()
{
    int8_t status;

    /* program key */
    dwt_set_keyreg_128(&keys_options[INITIATOR_KEY_INDEX-1]);
    MAC_FRAME_AUX_KEY_IDENTIFY_802_15_4(&mac_frame) = INITIATOR_KEY_INDEX;

    /* prepare MHR fields (PAN/addresses) and derive nonce */
    mac_frame_set_pan_ids_and_addresses_802_15_4(&mac_frame, 0x4321, 0x1122334455667788ULL, 0x8877665544332211ULL);
    mac_frame_get_nonce(&mac_frame, nonce);

    /* build AES job */
    aes_job_tx.mode = AES_Encrypt;
    aes_job_tx.src_port = AES_Src_Tx_buf;
    aes_job_tx.dst_port = AES_Dst_Tx_buf;
    aes_job_tx.nonce = nonce;
    aes_job_tx.header = (uint8_t *)MHR_802_15_4_PTR(&mac_frame);
    aes_job_tx.header_len = MAC_FRAME_HEADER_SIZE(&mac_frame);
    aes_job_tx.payload = tx_payload;
    aes_job_tx.payload_len = sizeof(tx_payload);
    aes_job_tx.mic_size = mac_frame_get_aux_mic_size(&mac_frame);

    /* configure AES engine */
    aes_config.mode = AES_Encrypt;
    aes_config.mic = dwt_mic_size_from_bytes(aes_job_tx.mic_size);
    dwt_configure_aes(&aes_config);

    /* perform AES job -> encrypted payload copied to TX buffer by driver */
    status = dwt_do_aes(&aes_job_tx, aes_config.aes_core_type);
    if (status < 0 || (status & AES_ERRORS)) return false;

    /* Write total frame length and start TX (no ranging reply expected here) */
    dwt_writetxfctrl(aes_job_tx.header_len + aes_job_tx.payload_len + aes_job_tx.mic_size + FCS_LEN, 0, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE);

    /* wait TXFRS */
    uint32_t t0 = millis();
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK)) {
        if ((millis() - t0) > 2000) return false;
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
}

void loop()
{
    bool ok = secure_tx_once();
    if (ok) test_run_info((unsigned char *)"SECURE TX OK"); else test_run_info((unsigned char *)"SECURE TX ERR");
    Sleep(500);
}
