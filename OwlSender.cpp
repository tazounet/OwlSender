/*
 * connectingStuff, Oregon Scientific v2.1 Emitter
 * http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/
 *
 * Copyright (C) 2013 olivier.lebrun@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
/*
 * OwlSender library (C) 2017 tazounet
*/

#include "OwlSender.h"

const unsigned long TIME = 488;
const unsigned long TWOTIME = TIME*2;
 
#define SEND_HIGH() _pin->hi();
#define SEND_LOW()  _pin->lo();
 
/**
 * \brief    Send logical "0" over RF
 * \details  a zero bit be represented by an off-to-on transition
 * \         of the RF signal at the middle of a clock period.
 */
inline void OwlSender::sendZero(void) 
{
  SEND_LOW();
  delayMicroseconds(TIME);
  SEND_HIGH();
  delayMicroseconds(TIME);
}
 
/**
 * \brief    Send logical "1" over RF
 * \details  a one bit be represented by an on-to-off transition
 * \         of the RF signal at the middle of a clock period.
 */
inline void OwlSender::sendOne(void) 
{
   SEND_HIGH();
   delayMicroseconds(TIME);
   SEND_LOW();
   delayMicroseconds(TIME);
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Send a buffer over RF
 * \param    data   Data to send
 * \param    size   size of data to send
 */
void OwlSender::sendData(byte *data, byte size)
{
  for(byte i = 0; i < size; ++i)
  {
    (bitRead(data[i], 0)) ? sendOne() : sendZero();
    (bitRead(data[i], 1)) ? sendOne() : sendZero();
    (bitRead(data[i], 2)) ? sendOne() : sendZero();
    (bitRead(data[i], 3)) ? sendOne() : sendZero();
    (bitRead(data[i], 4)) ? sendOne() : sendZero();
    (bitRead(data[i], 5)) ? sendOne() : sendZero();
    (bitRead(data[i], 6)) ? sendOne() : sendZero();
    (bitRead(data[i], 7)) ? sendOne() : sendZero();
  }
}
 
/**
 * \brief    Send an Oregon message
 * \param    data   The Oregon message
 */
void OwlSender::sendOregon(byte *data, byte size)
{
    sendPreamble();
    sendData(data, size);
    sendPostamble();
}
 
/**
 * \brief    Send preamble
 * \details  The preamble consists of 10 "1" bits (minimum)
 */
inline void OwlSender::sendPreamble(void)
{
  for(byte i = 0; i < 10; ++i)
  {
    sendOne();
  }
}
 
/**
 * \brief    Send postamble
 * \details  The postamble consists of 4 "0" bits
 */
inline void OwlSender::sendPostamble(void)
{
  for(byte i = 0; i < 4; ++i)
  {
    sendZero() ;
  }
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/

/**
 * \brief    Set the sensor type
 * \param    data       Oregon message
 * \param    type       Sensor type
 */
inline void OwlSender::setType(byte *data) 
{
  data[0] = 0x62;
  data[1] = 0x80;
}

/**
 * \brief    Set the sensor ID
 * \param    data       Oregon message
 * \param    ID         Sensor unique ID
 */
inline void OwlSender::setId(byte *data, byte ID) 
{
  data[2] = ID;
}

/**
 * \brief    Set the consumption
 * \param    data       Oregon message
 * \param    rtCons     the realtime consumption (W)
 * \param    accuCons   the accumulative consumption (Wh)
 */
void OwlSender::setConsumption(byte *data, int rtCons, long accuCons)
{
	unsigned long rtCons2 = long(long(rtCons) * 497L / 500L / 16L);
	unsigned long long accuCons2 = accuCons * 223666LL / 1000LL;

	data[3] = (rtCons2 & 0x0F) << 4;
	data[4] = (rtCons2 >> 4) & 0xFF;
	data[5] = ((rtCons2 >> 12) & 0x0F) + ((accuCons2 & 0x0F) << 4);
	data[6] = (accuCons2 >> 4) & 0xFF;
	data[7] = (accuCons2 >> 12) & 0xFF;
	data[8] = (accuCons2 >> 20) & 0xFF;
	data[9] = (accuCons2 >> 28) & 0xFF; 
	data[10] = (accuCons2 >> 36) & 0xFF; 
}
 
/**
 * \brief    Sum data for checksum
 * \param    count      number of bit to sum
 * \param    data       Oregon message
 */
int OwlSender::Sum(byte count, const byte* data)
{
  int s = 0;
 
  for(byte i = 0; i<count;i++)
  {
    s += long(data[i]&0x0F) + long(data[i]>>4);
  }
 
  s -= 2;
 
  return s;
}
 
/**
 * \brief    Calculate checksum
 * \param    data       Oregon message
 */
void OwlSender::calculateAndSetChecksum(byte* data)
{
  int s = Sum(11, data);
 
  data[11] = ((s&0x0F)<<4) + ((s>>8)&0x0F);
  data[12] = (int(s>>4)&0x0F);
}
 
/******************************************************************/
/******************************************************************/
/****************0**************************************************/
 
OwlSender::OwlSender()
{
}

OwlSender::OwlSender(byte txPin, byte sensorId)
{
	setup(txPin, sensorId);
}

void OwlSender::setup(byte txPin, byte sensorId)
{
	setTxPin(txPin);
	setType(_oregonMessageBuffer);
	setSensorId(sensorId);
}

void OwlSender::setTxPin(byte txPin)
{
	_pin = new OutputPin(txPin);
}

void OwlSender::setSensorId(byte sensorId)
{
	setId(_oregonMessageBuffer, sensorId);
}

void OwlSender::send(int rtCons, long accuCons)
{
	setConsumption(_oregonMessageBuffer, rtCons, accuCons);
 
	// Calculate the checksum
	calculateAndSetChecksum(_oregonMessageBuffer);
 
	// Show the Oregon Message
	//for (byte i = 0; i < sizeof(_oregonMessageBuffer); ++i) {
	//	Serial.print(_oregonMessageBuffer[i] >> 4, HEX);
	//	Serial.print(_oregonMessageBuffer[i] & 0x0F, HEX);
	//}
 
	// Send the Message over RF
	sendOregon(_oregonMessageBuffer, 13);
 
	SEND_LOW();
}