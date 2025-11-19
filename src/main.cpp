#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <dw3000.h>

#ifdef __cplusplus // Checks if the file is being compiled as C++
extern "C" {
#endif

    void app_main(void) {}
    //     static const char *TAG = "SS_TWR_RESP";

    // // ----------------- DW3000 Pins ------------------
    // #define PIN_RST 27
    // #define PIN_IRQ 34
    // #define PIN_SS 4

    //     // ----------------- DW3000 Config -----------------
    //     static dwt_config_t config = {
    //         5,               // Channel
    //         DWT_PLEN_128,    // Preamble length
    //         DWT_PAC8,        // PAC size
    //         9,               // TX preamble code
    //         9,               // RX preamble code
    //         1,               // SFD mode
    //         DWT_BR_6M8,      // Data rate
    //         DWT_PHRMODE_STD, // PHR mode
    //         DWT_PHRRATE_STD, // PHR rate
    //         (129 + 8 - 8),   // SFD timeout
    //         DWT_STS_MODE_OFF,
    //         DWT_STS_LEN_64,
    //         DWT_PDOA_M0};

    // #define TX_ANT_DLY 16385
    // #define RX_ANT_DLY 16385

    //     // ----------------- Messages ----------------------
    //     static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
    //     static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1,
    //                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // #define ALL_MSG_COMMON_LEN 10
    // #define ALL_MSG_SN_IDX 2
    // #define RESP_MSG_POLL_RX_TS_IDX 10
    // #define RESP_MSG_RESP_TX_TS_IDX 14
    // #define RESP_MSG_TS_LEN 4

    //     static uint8_t frame_seq_nb = 0;

    // // ----------------- Buffers -----------------------
    // #define RX_BUF_LEN 12
    //     static uint8_t rx_buffer[RX_BUF_LEN];

    // #define POLL_RX_TO_RESP_TX_DLY_UUS 450

    //     // timestamps
    //     static uint64_t poll_rx_ts;
    //     static uint64_t resp_tx_ts;

    //     // Provided by DW3000 API
    //     extern dwt_txconfig_t txconfig_options;

    //     // ----------------- Helpers -----------------------
    //     void resp_msg_set_ts(uint8_t *ts_field, uint64_t ts)
    //     {
    //         for (int i = 0; i < RESP_MSG_TS_LEN; i++)
    //             ts_field[i] = (uint8_t)(ts >> (i * 8));
    //     }

    //     // -----------------------------------------------
    //     //       Ranging Responder FreeRTOS Task
    //     // -----------------------------------------------
    //     void uwb_resp_task(void *arg)
    //     {
    //         uint32_t status;

    //         ESP_LOGI(TAG, "Starting SS-TWR Responder Task");

    //         // Enable RX
    //         dwt_rxenable(DWT_START_RX_IMMEDIATE);

    //         while (1)
    //         {
    //             // Wait for good RX or error
    //             do
    //             {
    //                 status = dwt_read32bitreg(SYS_STATUS_ID);
    //             } while (!(status & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR)));

    //             // ---------------- GOOD RX ----------------
    //             if (status & SYS_STATUS_RXFCG_BIT_MASK)
    //             {

    //                 dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    //                 uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

    //                 if (frame_len <= RX_BUF_LEN)
    //                 {
    //                     dwt_readrxdata(rx_buffer, frame_len, 0);

    //                     // Clear sequence number for compare
    //                     rx_buffer[ALL_MSG_SN_IDX] = 0;

    //                     if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
    //                     {

    //                         poll_rx_ts = get_rx_timestamp_u64();

    //                         uint32_t resp_tx_time =
    //                             (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

    //                         dwt_setdelayedtrxtime(resp_tx_time);

    //                         resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

    //                         // Fill timestamps
    //                         resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
    //                         resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);

    //                         // Send response
    //                         tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    //                         dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
    //                         dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

    //                         int ret = dwt_starttx(DWT_START_TX_DELAYED);

    //                         if (ret == DWT_SUCCESS)
    //                         {
    //                             while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
    //                             {
    //                                 ;
    //                             }
    //                             dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    //                             frame_seq_nb++;
    //                         }
    //                     }
    //                 }
    //             }
    //             // ---------------- RX ERROR ----------------
    //             else
    //             {
    //                 dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    //                 ESP_LOGW(TAG, "RX error occurred");
    //             }

    //             // restart RX
    //             dwt_rxenable(DWT_START_RX_IMMEDIATE);
    //         }
    //     }

    //     // -----------------------------------------------
    //     //                DW3000 INIT
    //     // -----------------------------------------------
    //     void init_dw3000(void)
    //     {
    //         ESP_LOGI(TAG, "Initializing DW3000...");

    //         // Reset pin
    //         gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    //         gpio_set_level(PIN_RST, 0);
    //         vTaskDelay(pdMS_TO_TICKS(2));
    //         gpio_set_level(PIN_RST, 1);

    //         vTaskDelay(pdMS_TO_TICKS(5));

    //         if (!dwt_checkidlerc())
    //         {
    //             ESP_LOGE(TAG, "DW3000 failed to enter IDLE_RC");
    //             while (1)
    //             {
    //             }
    //         }

    //         if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    //         {
    //             ESP_LOGE(TAG, "DW3000 init failed");
    //             while (1)
    //             {
    //             }
    //         }

    //         dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    //         if (dwt_configure(&config))
    //         {
    //             ESP_LOGE(TAG, "DW3000 config failed");
    //             while (1)
    //             {
    //             }
    //         }

    //         dwt_configuretxrf(&txconfig_options);

    //         dwt_setrxantennadelay(RX_ANT_DLY);
    //         dwt_settxantennadelay(TX_ANT_DLY);

    //         dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    //     }

    //     // -----------------------------------------------
    //     //                  app_main
    //     // -----------------------------------------------
    //     void app_main(void)
    //     {
    //         ESP_LOGI(TAG, "Starting SS-TWR Responder");

    //         // Initialize SPI (done inside the DW driver if needed)
    //         // you may need spi_bus_initialize() and spi_bus_add_device()

    //         init_dw3000();

    //         xTaskCreatePinnedToCore(
    //             uwb_resp_task,
    //             "uwb_resp_task",
    //             4096,
    //             NULL,
    //             5,
    //             NULL,
    //             1);
    //     }

#ifdef __cplusplus
}
#endif
