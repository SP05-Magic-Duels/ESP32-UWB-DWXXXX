/*
 * prgms_dw3000.h
 * Convenience header that includes the example .cpp files in this folder.
 * NOTE: Including .cpp files is intentional here to allow building these
 * example translation units from a single include, matching some Arduino-style
 * build expectations. Use with care.
 */

#ifndef PRGMS_DW3000_H
#define PRGMS_DW3000_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ex_00a_reading_dev_id.cpp"
#include "ex_01a_simple_tx.cpp"
#include "ex_01b_tx_sleep.cpp"
#include "ex_01c_tx_sleep_auto.cpp"
#include "ex_02a_simple_rx.cpp"
#include "ex_06a_ss_twr_initiator.cpp"
#include "ex_06a_ss_twr_initiator_sts.cpp"
#include "ex_06a_ss_twr_initiator_sts_no_data.cpp"
#include "ex_06b_ss_twr_responder.cpp"
#include "ex_06b_ss_twr_responder_sts.cpp"
#include "ex_06b_ss_twr_responder_sts_no_data.cpp"
#include "ex_06e_AES_ss_twr_initiator.cpp"
#include "ex_06f_AES_ss_twr_responder.cpp"

#ifdef __cplusplus
}
#endif

#endif // PRGMS_DW3000_H
