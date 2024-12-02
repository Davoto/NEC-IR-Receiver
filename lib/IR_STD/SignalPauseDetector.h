#ifndef SIGNAL_PAUSE_DETECTOR_H
#define SIGNAL_PAUSE_DETECTOR_H

#include <Arduino.h>
#include "NecReceiver.h"
#include "crt_CleanRTOS.h"

#define T_MAX_PAUSE_US 6000

#define SPD_TASK_NAME           "SignalPauseDetector"
#define SPD_TASK_STACK_DEPTH    6000
#define SPD_TASK_PRIORITY       2

class SignalPauseDetector{
	enum states{
		WaitingForPause,
		WaitingForSignal
	};
public:
	SignalPauseDetector(TsopReceiver& tsopReceiver, NecReceiver& necReceiver) : tsopReceiver(tsopReceiver), necReceiver(necReceiver) {};

    void begin(){
        xTaskCreate(Static_main, SPD_TASK_NAME, SPD_TASK_STACK_DEPTH, this, SPD_TASK_PRIORITY, NULL);
    }
private:
	TsopReceiver tsopReceiver;
    NecReceiver necReceiver;
	uint32_t t_signalUs = 0;
	uint32_t t_pauseUs;
	states state = WaitingForPause;
	
	void main(){
		vTaskDelay(1);
		
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

    static void Static_main(void* arg){
        SignalPauseDetector* runner = (SignalPauseDetector*)arg;
        runner->main();
    }
};

#endif