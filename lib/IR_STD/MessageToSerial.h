#ifndef MESSAGE_TO_SERIAL_H
#define MESSAGE_TO_SERIAL_H

#include <Arduino.h>

class MessageToSerial{
public:	
	static void messageReceived(uint32_t msg, uint8_t nofBytes){
		uint32_t buffer = msg;
		uint8_t byte = buffer;
		
		Serial.print(char(byte));
		
		for(int i = 1; i < nofBytes; i++){
			buffer >>= 8;
			byte = buffer;
			
			Serial.print(char(byte));
		};
		
		Serial.println();
	};
};

#endif