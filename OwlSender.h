/*
 * OwlSender library (C) 2017 tazounet
*/

#include "Arduino.h"
#include "fastpin.h"

class OwlSender {
	
public:
	OwlSender();
	OwlSender(byte txpin, byte sensorId);
	void setup(byte txpin, byte sensorId);
	void setTxPin(byte txPin);
	void setSensorId(byte sensorId);
	void send(int rtCons, long accuCons); // W, Wh

private:
	void sendZero(void);
	void sendOne(void);
	void sendData(byte *data, byte size);
	void sendOregon(byte *data, byte size);
	void sendPreamble(void);
	void sendPostamble(void);
	void sendSync(void);
	void setType(byte *data);
	void setId(byte *data, byte ID);
	void setConsumption(byte *data, int rtCons, long accuCons);
	int Sum(byte count, const byte* data);
	void calculateAndSetChecksum(byte* data);

	// Buffer for Oregon message
	byte _oregonMessageBuffer[13];
	OutputPin* _pin;

};