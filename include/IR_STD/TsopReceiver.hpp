#ifndef TSOPRECEIVER
#define TSOPRECEIVER

#include <Arduino.h>
#include "crt_CleanRTOS.h"

class TsopReceiver: public crt::Task{
public:
	bool isSignalPresent(){
		return Signal;
	};
private:
	void main(){
		vTaskDelay(1000);
		
		pinMode(Pin, INPUT);
		while(true){
			delayMicroseconds(100);
			Signal = digitalRead(Pin);
		};
	};
	
	int Pin;
	bool Signal;
public:
	TsopReceiver(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber, const uint8_t& Pin) : 
		Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber), Pin(Pin) {
			start();
		};
};

#endif