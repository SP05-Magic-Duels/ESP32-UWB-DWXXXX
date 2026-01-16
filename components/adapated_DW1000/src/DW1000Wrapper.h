/************************************* C API Wrapper **************************************/
#ifndef DW1000WRAPPER_H
#define DW1000WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "DW1000.h"
#include "DW1000Ranging.h"

    static void initCommunication(uint8_t myRST = DEFAULT_RST_PIN, uint8_t mySS = DEFAULT_SPI_SS_PIN, uint8_t myIRQ = 2);
    static void configureNetwork(uint16_t deviceAddress, uint16_t networkId, const uint8_t mode[]);
    static void generalStart();
    static void startAsAnchor(char address[], const uint8_t mode[], const bool randomShortAddress = true);
    static void startAsTag(char address[], const uint8_t mode[], const bool randomShortAddress = true);
    static uint8_t addNetworkDevices(DW1000Device *device, uint8_t shortAddress);
    static uint8_t addNetworkDevices(DW1000Device *device);
    static void removeNetworkDevices(int16_t index);

    // setters
    static void setReplyTime(uint16_t replyDelayTimeUs);
    static void setResetPeriod(uint32_t resetPeriod);

    // getters
    static uint8_t *getCurrentAddress() { return _currentAddress; };

    static uint8_t *getCurrentShortAddress() { return _currentShortAddress; };

    static uint8_t getNetworkDevicesNumber() { return _networkDevicesNumber; };

    // ranging functions
    static int16_t detectMessageType(uint8_t datas[]); // TODO check return type
    static void loop();
    static void useRangeFilter(uint8_t enabled);
    // Used for the smoothing algorithm (Exponential Moving Average). newValue must be >= 2. Default 15.
    static void setRangeFilterValue(uint16_t newValue);

    // Handlers:
    static void attachNewRange(void (*handleNewRange)(void)) { _handleNewRange = handleNewRange; };

    static void attachBlinkDevice(void (*handleBlinkDevice)(DW1000Device *)) { _handleBlinkDevice = handleBlinkDevice; };

    static void attachNewDevice(void (*handleNewDevice)(DW1000Device *)) { _handleNewDevice = handleNewDevice; };

    static void attachInactiveDevice(void (*handleInactiveDevice)(DW1000Device *)) { _handleInactiveDevice = handleInactiveDevice; };

    static DW1000Device *getDistantDevice();
    static DW1000Device *searchDistantDevice(uint8_t shortAddress[]);

    // FOR DEBUGGING
    static void visualizeDatas(uint8_t datas[]);

#ifdef __cplusplus
}
#endif

#endif // DW1000WRAPPER_H
