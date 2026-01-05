cmake_minimum_required(VERSION 3.16)

# Selectable examples for adapted_dw3000. Set ADAPTED_DW3000_EXAMPLE to one
# of the names below (without .cpp) to build that example. Use `all` to build
# all examples (may fail if multiple mains are present).

set(ADAPTED_DW3000_EXAMPLES
    ex_00a_reading_dev_id
    ex_01a_simple_tx
    ex_01b_tx_sleep
    ex_01c_tx_sleep_auto
    ex_02a_simple_rx
    ex_06a_ss_twr_initiator
    ex_06a_ss_twr_initiator_sts
    ex_06a_ss_twr_initiator_sts_no_data
    ex_06b_ss_twr_responder
    ex_06b_ss_twr_responder_sts
    ex_06b_ss_twr_responder_sts_no_data
    ex_06e_AES_ss_twr_initiator
    ex_06f_AES_ss_twr_responder
    uwb_best_tx
    uwb_best_rx
    uwb_best_tx_secure
    uwb_best_rx_secure
)

set(ADAPTED_DW3000_EXAMPLE "ex_06a_ss_twr_initiator" CACHE STRING "Example from adapted_dw3000 to build (use 'all' to build all)")

if(ADAPTED_DW3000_EXAMPLE STREQUAL "all")
    set(SRCS_CPP)
    foreach(f ${ADAPTED_DW3000_EXAMPLES})
        list(APPEND SRCS_CPP "${f}.cpp")
    endforeach()
    idf_component_register(SRCS ${SRCS_CPP})
else()
    list(FIND ADAPTED_DW3000_EXAMPLES ${ADAPTED_DW3000_EXAMPLE} _idx)
    if(_idx EQUAL -1)
        message(FATAL_ERROR "Unknown ADAPTED_DW3000_EXAMPLE='${ADAPTED_DW3000_EXAMPLE}'. Available: ${ADAPTED_DW3000_EXAMPLES}")
    endif()
    idf_component_register(SRCS "${ADAPTED_DW3000_EXAMPLE}.cpp")
endif()

target_include_directories(${COMPONENT_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${COMPONENT_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
