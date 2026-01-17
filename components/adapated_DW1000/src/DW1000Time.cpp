/**
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Copyright (c) 2016 by Ludwig Grill (www.rotzbua.de); refactored class
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
 * @file DW1000Time.cpp
 * Arduino driver library timestamp wrapper (source file) for the Decawave
 * DW1000 UWB transceiver IC.
 */

#include "DW1000Time.h"

/**
 * Initiates DW1000Time with 0
 */
extern "C" DW1000Time::DW1000Time()
{
	_timestamp = 0;
}

/**
 * Initiates DW1000Time with timestamp
 * @param time timestamp with intervall 1 is approx. 15ps
 */
extern "C" DW1000Time::DW1000Time(int64_t time)
{
	setTimestamp(time);
}

/**
 * Initiates DW1000Time with timestamp
 * @param data timestamp as uint8_t array
 */
extern "C" DW1000Time::DW1000Time(uint8_t data[])
{
	setTimestamp(data);
}

/**
 * Initiates DW100Time with another instance
 * @param copy other instance
 */
extern "C" DW1000Time::DW1000Time(const DW1000Time &copy)
{
	setTimestamp(copy);
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
extern "C" DW1000Time::DW1000Time(float timeUs)
{
	setTime(timeUs);
}

/**
 * Initiates DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
extern "C" DW1000Time::DW1000Time(int32_t value, float factorUs)
{
	setTime(value, factorUs);
}

/**
 * Empty
 */
extern "C" DW1000Time::~DW1000Time() {}

/**
 * Set timestamp
 * @param value - timestamp with intervall 1 is approx. 15ps
 */
extern "C" void DW1000Time::setTimestamp(int64_t value)
{
	_timestamp = value;
}

/**
 * Set timestamp
 * @param data timestamp as uint8_t array
 */
extern "C" void DW1000Time::setTimestamp(uint8_t data[])
{
	_timestamp = 0;
	for (uint8_t i = 0; i < LENGTH_TIMESTAMP; i++)
	{
		_timestamp |= ((int64_t)data[i] << (i * 8));
	}
}

/**
 * Set timestamp from other instance
 * @param copy instance where the timestamp should be copied
 */
extern "C" void DW1000Time::setTimestamp(const DW1000Time &copy)
{
	_timestamp = copy.getTimestamp();
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
extern "C" void DW1000Time::setTime(float timeUs)
{
	_timestamp = (int64_t)(timeUs * TIME_RES_INV);
	//	_timestamp %= TIME_OVERFLOW; // clean overflow
}

/**
 * Set DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
extern "C" void DW1000Time::setTime(int32_t value, float factorUs)
{
	// float tsValue = value*factorUs;
	// tsValue = fmod(tsValue, TIME_OVERFLOW);
	// setTime(tsValue);
	setTime(value * factorUs);
}

/**
 * Get timestamp as integer
 * @return timestamp as integer
 */
extern "C" int64_t DW1000Time::getTimestamp() const
{
	return _timestamp;
}

/**
 * Get timestamp as uint8_t array
 * @param data var where data should be written
 */
extern "C" void DW1000Time::getTimestamp(uint8_t data[]) const
{
	memset(data, 0, LENGTH_TIMESTAMP);
	for (uint8_t i = 0; i < LENGTH_TIMESTAMP; i++)
	{
		data[i] = (uint8_t)((_timestamp >> (i * 8)) & 0xFF);
	}
}

/**
 * Return real time in micro seconds
 * @return time in micro seconds
 * @deprecated use getAsMicroSeconds()
 */
extern "C" float DW1000Time::getAsFloat() const
{
	// return fmod((float)_timestamp, TIME_OVERFLOW)*TIME_RES;
	return getAsMicroSeconds();
}

/**
 * Return real time in micro seconds
 * @return time in micro seconds
 */
extern "C" float DW1000Time::getAsMicroSeconds() const
{
	return (_timestamp % TIME_OVERFLOW) * TIME_RES;
}

/**
 * Return time as distance in meter, d=c*t
 * this is useful for e.g. time of flight
 * @return distance in meters
 */
extern "C" float DW1000Time::getAsMeters() const
{
	// return fmod((float)_timestamp, TIME_OVERFLOW)*DISTANCE_OF_RADIO;
	return (_timestamp % TIME_OVERFLOW) * DISTANCE_OF_RADIO;
}

/**
 * Converts negative values due overflow of one node to correct value
 * @example:
 * Maximum timesamp is 1000.
 * Node N1 sends 999 as timesamp. N2 recieves and sends delayed and increased timestamp back.
 * Delay is 10, so timestamp would be 1009, but due overflow 009 is sent back.
 * Now calculate TOF: 009 - 999 = -990 -> not correct time, so wrap()
 * Wrap calculation: -990 + 1000 = 10 -> correct time
 * @return
 */
extern "C" DW1000Time &DW1000Time::wrap()
{
	if (_timestamp < 0)
	{
		_timestamp += TIME_OVERFLOW;
	}
	return *this;
}

/**
 * Check if timestamp is valid for usage with DW1000 device
 * @return true if valid, false if negative or overflow (maybe after calculation)
 */
extern "C" bool DW1000Time::isValidTimestamp()
{
	return (0 <= _timestamp && _timestamp <= TIME_MAX);
}

// assign
extern "C" DW1000Time &DW1000Time::operator=(const DW1000Time &assign)
{
	if (this == &assign)
	{
		return *this;
	}
	_timestamp = assign.getTimestamp();
	return *this;
}

// add
extern "C" DW1000Time &DW1000Time::operator+=(const DW1000Time &add)
{
	_timestamp += add.getTimestamp();
	return *this;
}

extern "C" DW1000Time DW1000Time::operator+(const DW1000Time &add) const
{
	return DW1000Time(*this) += add;
}

// subtract
extern "C" DW1000Time &DW1000Time::operator-=(const DW1000Time &sub)
{
	_timestamp -= sub.getTimestamp();
	return *this;
}

extern "C" DW1000Time DW1000Time::operator-(const DW1000Time &sub) const
{
	return DW1000Time(*this) -= sub;
}

// multiply
extern "C" DW1000Time &DW1000Time::operator*=(float factor)
{
	// float tsValue = (float)_timestamp*factor;
	//_timestamp = (int64_t)tsValue;
	_timestamp *= factor;
	return *this;
}

extern "C" DW1000Time DW1000Time::operator*(float factor) const
{
	return DW1000Time(*this) *= factor;
}

extern "C" DW1000Time &DW1000Time::operator*=(const DW1000Time &factor)
{
	_timestamp *= factor.getTimestamp();
	return *this;
}

extern "C" DW1000Time DW1000Time::operator*(const DW1000Time &factor) const
{
	return DW1000Time(*this) *= factor;
}

// divide
extern "C" DW1000Time &DW1000Time::operator/=(float factor)
{
	//_timestamp *= (1.0f/factor);
	_timestamp /= factor;
	return *this;
}

extern "C" DW1000Time DW1000Time::operator/(float factor) const
{
	return DW1000Time(*this) /= factor;
}

extern "C" DW1000Time &DW1000Time::operator/=(const DW1000Time &factor)
{
	_timestamp /= factor.getTimestamp();
	return *this;
}

extern "C" DW1000Time DW1000Time::operator/(const DW1000Time &factor) const
{
	return DW1000Time(*this) /= factor;
}

// compare
extern "C" uint8_t DW1000Time::operator==(const DW1000Time &cmp) const
{
	return _timestamp == cmp.getTimestamp();
}

extern "C" uint8_t DW1000Time::operator!=(const DW1000Time &cmp) const
{
	// return !(*this == cmp); // seems not as intended
	return _timestamp != cmp.getTimestamp();
}