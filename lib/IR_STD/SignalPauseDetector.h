#ifndef SIGNAL_PAUSE_DETECTOR_H
#define SIGNAL_PAUSE_DETECTOR_H

#include <Arduino.h>
#include "crt_CleanRTOS.h"

#define T_MAX_PAUSE_US 6000

#define SPD_TASK_NAME           "SignalPauseDetector"
#define SPD_TASK_STACK_DEPTH    6000
#define SPD_TASK_PRIORITY       2

#define T_LEADSIGNAL_MIN_US             7000


class SignalPauseDetector{
	enum states{
		WaitingForPause,
		WaitingForSignal
	};
public:
	SignalPauseDetector(TsopReceiver& tsopReceiver, QueueHandle_t& signalChannel, QueueHandle_t& pauseChannel)
                        : tsopReceiver(tsopReceiver), signalChannel(signalChannel), pauseChannel(pauseChannel) {};

    void begin(){
        xTaskCreate(Static_main, SPD_TASK_NAME, SPD_TASK_STACK_DEPTH, this, SPD_TASK_PRIORITY, NULL);
    }
private:
	TsopReceiver tsopReceiver;

    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

	uint32_t t_signalUs = 0;
	uint32_t t_pauseUs = 0;
	states state = WaitingForPause;

    void signalDetected(uint32_t t_us){
        if(t_us > T_LEADSIGNAL_MIN_US) {
            ClearQueue(signalChannel);
            ClearQueue(pauseChannel);
        }
        xQueueSend(signalChannel, &t_us, 0);
    };

    void pauseDetected(uint32_t t_us){
        xQueueSend(pauseChannel, &t_us, 0);
    };

    static void ClearQueue(QueueHandle_t Queue){
        uint32_t dummy;
        while(uxQueueMessagesWaiting(Queue) > 0){
            xQueueReceive(Queue, &dummy, portMAX_DELAY);
        }
    }

	void main(){
		vTaskDelay(1);
		
		while(true){
			switch(state){
				case WaitingForPause:
					delayMicroseconds(100);
					
					if(tsopReceiver.isSignalPresent()){ 
						t_signalUs+=100;
					}else{
						signalDetected(t_signalUs);
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
						pauseDetected(t_pauseUs);
						
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