/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net> and Leopold Sayous <leosayous@gmail.com>
 * Decawave DW1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file DW1000Device.h
 * Arduino global library (header file) working with the DW1000 library
 * for the Decawave DW1000 UWB transceiver IC.
 *
 * @todo complete this class
 */

#ifndef _DW1000Device_H_INCLUDED
#define _DW1000Device_H_INCLUDED

#define INACTIVITY_TIME 1000

#include <time.h>
#include <math.h>

#include "esp_timer.h"

#include "DW1000Time.h"
#include "DW1000Mac.h"

#ifdef __cplusplus

// Wrapper for millis() function
static uint32_t millis();

class DW1000Mac;

class DW1000Device;

class DW1000Device
{
public:
	// Constructor and destructor
	DW1000Device();
	DW1000Device(uint8_t address[], uint8_t shortAddress[]);
	DW1000Device(uint8_t address[], uint8_t shortOne = false);
	~DW1000Device();

	// setters:
	void setReplyTime(uint16_t replyDelayTimeUs);
	void setAddress(char address[]);
	void setAddress(uint8_t *address);
	void setShortAddress(uint8_t address[]);

	void setRange(float range);
	void setRXPower(float power);
	void setFPPower(float power);
	void setQuality(float quality);

	void setReplyDelayTime(uint16_t time) { _replyDelayTimeUS = time; }

	void setIndex(int8_t index) { _index = index; }

	// getters
	uint16_t getReplyTime() { return _replyDelayTimeUS; }

	uint8_t *getByteAddress();

	int8_t getIndex() { return _index; }

	// String getAddress();
	uint8_t *getByteShortAddress();
	uint16_t getShortAddress();
	// String getShortAddress();

	float getRange();
	float getRXPower();
	float getFPPower();
	float getQuality();

	uint8_t isAddressEqual(DW1000Device *device);
	uint8_t isShortAddressEqual(DW1000Device *device);

	// functions which contains the date: (easier to put as public)
	//  timestamps to remember
	DW1000Time timePollSent;
	DW1000Time timePollReceived;
	DW1000Time timePollAckSent;
	DW1000Time timePollAckReceived;
	DW1000Time timeRangeSent;
	DW1000Time timeRangeReceived;

	void noteActivity();
	uint8_t isInactive();

private:
	// device ID
	uint8_t _ownAddress[8];
	uint8_t _shortAddress[2];
	int32_t _activity;
	uint16_t _replyDelayTimeUS;
	int8_t _index; // not used

	int16_t _range;
	int16_t _RXPower;
	int16_t _FPPower;
	int16_t _quality;

	void randomShortAddress();
};

#endif // __cplusplus

#endif // _DW1000Device_H_INCLUDED
