#ifndef SIGNALPAUSEDETECTOR
#define SIGNALPAUSEDETECTOR

#include <Arduino.h>
#include "NecReceiver.hpp"
#include "TsopReceiver.hpp"
#include "crt_CleanRTOS.h"

namespace crt{
class SignalPauseDetector: public Task{
	enum states{
		WaitingForPause,
		WaitingForSignal
	};
public:
	SignalPauseDetector(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber, TsopReceiver& tsopReceiver, NecReceiver& necReceiver) : 
		Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber), tsopReceiver(tsopReceiver), necReceiver(necReceiver) {
			start();
		};
private:
	TsopReceiver tsopReceiver;
    NecReceiver necReceiver;

    uint32_t T_MAX_PAUSE_US = 6000;

	uint32_t t_signalUs = 0;
	uint32_t t_pauseUs;	
	
	states state = WaitingForPause;
	
	void main(){
		vTaskDelay(1000);
		
		while(true){
			switch(state){
				case WaitingForPause:
					delayMicroseconds(100);
					
					if(tsopReceiver.isSignalPresent()){ 
						t_signalUs+=100;
					}else{
						necReceiver.signalDetected(t_signalUs);
						t_pauseUs = 0;
						state = WaitingForSignal;
					};
					
					break;
				case WaitingForSignal:
					delayMicroseconds(100);
					
					if(!tsopReceiver.isSignalPresent()){
						t_pauseUs+=100;
						
						if(t_pauseUs > T_MAX_PAUSE_US) t_pauseUs = 0;
					}else{
						necReceiver.pauseDetected(t_pauseUs);
						
						t_signalUs = 0;
						state = WaitingForPause;
					};
					
					break;
			};
		};
	};
};
};

#endif