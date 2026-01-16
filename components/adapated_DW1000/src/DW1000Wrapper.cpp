
#include "DW1000.h"
#include "DW1000Ranging.h"

static void DW1000Ranging_initCommunication(uint8_t myRST = DEFAULT_RST_PIN, uint8_t mySS = DEFAULT_SPI_SS_PIN, uint8_t myIRQ = 2)
{
    DW1000Ranging.initCommunication(myRST, mySS, myIRQ);
};
static void DW1000Ranging_configureNetwork(uint16_t deviceAddress, uint16_t networkId, const uint8_t mode[])
{
    DW1000Ranging.configureNetwork(deviceAddress, networkId, mode);
};
static void DW1000Ranging_generalStart()
{
    DW1000Ranging.generalStart();
};
static void DW1000Ranging_startAsAnchor(char address[], const uint8_t mode[], const bool randomShortAddress = true)
{
    DW1000Ranging.startAsAnchor(address, mode, randomShortAddress);
};
static void DW1000Ranging_startAsTag(char address[], const uint8_t mode[], const bool randomShortAddress = true)
{
    DW1000Ranging.startAsTag(address, mode, randomShortAddress);
};
static uint8_t DW1000Ranging_addNetworkDevices(DW1000Device *device, uint8_t shortAddress)
{
    return DW1000Ranging.addNetworkDevices(device, shortAddress);
};
static uint8_t DW1000Ranging_addNetworkDevices(DW1000Device *device)
{
    return DW1000Ranging.addNetworkDevices(device);
};
static void DW1000Ranging_removeNetworkDevices(int16_t index)
{
    DW1000Ranging.removeNetworkDevices(index);
};

// setters
static void DW1000Ranging_setReplyTime(uint16_t replyDelayTimeUs)
{
    DW1000Ranging.setReplyTime(replyDelayTimeUs);
};
static void DW1000Ranging_setResetPeriod(uint32_t resetPeriod)
{
    DW1000Ranging.setResetPeriod(resetPeriod);
};

// getters
static uint8_t *DW1000Ranging_getCurrentAddress() { return DW1000Ranging.getCurrentAddress(); };

static uint8_t *DW1000Ranging_getCurrentShortAddress() { return DW1000Ranging.getCurrentShortAddress(); };

static uint8_t DW1000Ranging_getNetworkDevicesNumber() { return DW1000Ranging.getNetworkDevicesNumber(); };

// ranging functions
static int16_t DW1000Ranging_detectMessageType(uint8_t datas[])
{
    return DW1000Ranging.detectMessageType(datas);
}; // TODO check return type
static void DW1000Ranging_loop()
{
    return DW1000Ranging.loop();
};
static void DW1000Ranging_useRangeFilter(uint8_t enabled)
{
    return DW1000Ranging.useRangeFilter(enabled);
};
// Used for the smoothing algorithm (Exponential Moving Average). newValue must be >= 2. Default 15.
static void DW1000Ranging_setRangeFilterValue(uint16_t newValue)
{
    return DW1000Ranging.setRangeFilterValue(newValue);
};

// Handlers:
static void DW1000Ranging_attachNewRange(void (*handleNewRange)(void)) { DW1000Ranging.attachNewRange(handleNewRange); };

static void DW1000Ranging_attachBlinkDevice(void (*handleBlinkDevice)(DW1000Device *)) { DW1000Ranging.attachBlinkDevice(handleBlinkDevice); };

static void DW1000Ranging_attachNewDevice(void (*handleNewDevice)(DW1000Device *)) { DW1000Ranging.attachNewDevice(handleNewDevice); };
static void DW1000Ranging_attachInactiveDevice(void (*handleInactiveDevice)(DW1000Device *)) { DW1000Ranging.attachInactiveDevice(handleInactiveDevice); };
static DW1000Device *DW1000Ranging_getDistantDevice() { return DW1000Ranging.getDistantDevice(); };
static DW1000Device *DW1000Ranging_searchDistantDevice(uint8_t shortAddress[]) { return DW1000Ranging.searchDistantDevice(shortAddress); };

// FOR DEBUGGING
static void DW1000Ranging_visualizeDatas(uint8_t datas[])
{
    DW1000Ranging.visualizeDatas(datas);
};

// static void DW1000Ranging_initCommunication(uint8_t myRST = DEFAULT_RST_PIN, uint8_t mySS = DEFAULT_SPI_SS_PIN, uint8_t myIRQ = 2) {};
// static void DW1000Ranging_configureNetwork(uint16_t deviceAddress, uint16_t networkId, const uint8_t mode[]);
// static void DW1000Ranging_generalStart();
// static void DW1000Ranging_startAsAnchor(char address[], const uint8_t mode[], const bool randomShortAddress = true);
// static void DW1000Ranging_startAsTag(char address[], const uint8_t mode[], const bool randomShortAddress = true);
// static uint8_t DW1000Ranging_addNetworkDevices(DW1000Device *device, uint8_t shortAddress);
// static uint8_t DW1000Ranging_addNetworkDevices(DW1000Device *device);
// static void DW1000Ranging_removeNetworkDevices(int16_t index);

// // setters
// static void DW1000Ranging_setReplyTime(uint16_t replyDelayTimeUs);
// static void DW1000Ranging_setResetPeriod(uint32_t resetPeriod);

// // getters
// static uint8_t *DW1000Ranging_getCurrentAddress() { return _currentAddress; };

// static uint8_t *DW1000Ranging_getCurrentShortAddress() { return _currentShortAddress; };

// static uint8_t DW1000Ranging_getNetworkDevicesNumber() { return _networkDevicesNumber; };

// // ranging functions
// static int16_t DW1000Ranging_detectMessageType(uint8_t datas[]); // TODO check return type
// static void DW1000Ranging_loop();
// static void DW1000Ranging_useRangeFilter(uint8_t enabled);
// // Used for the smoothing algorithm (Exponential Moving Average). newValue must be >= 2. Default 15.
// static void DW1000Ranging_setRangeFilterValue(uint16_t newValue);

// // Handlers:
// static void DW1000Ranging_attachNewRange(void (*handleNewRange)(void)) { _handleNewRange = handleNewRange; };

// static void DW1000Ranging_attachBlinkDevice(void (*handleBlinkDevice)(DW1000Device *)) { _handleBlinkDevice = handleBlinkDevice; };

// static void DW1000Ranging_attachNewDevice(void (*handleNewDevice)(DW1000Device *)) { _handleNewDevice = handleNewDevice; };

// static void DW1000Ranging_attachInactiveDevice(void (*handleInactiveDevice)(DW1000Device *)) { _handleInactiveDevice = handleInactiveDevice; };
// static DW1000Device *DW1000Ranging_getDistantDevice();
// static DW1000Device *DW1000Ranging_searchDistantDevice(uint8_t shortAddress[]);

// // FOR DEBUGGING
// static void DW1000Ranging_visualizeDatas(uint8_t datas[]);
