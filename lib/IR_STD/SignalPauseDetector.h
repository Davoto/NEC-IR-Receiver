#ifndef SIGNAL_PAUSE_DETECTOR_H
#define SIGNAL_PAUSE_DETECTOR_H

#include <Arduino.h>

#define T_MAX_PAUSE_US 6000

#define SPD_TASK_NAME           "SignalPauseDetector"
#define SPD_TASK_STACK_DEPTH    6000
#define SPD_TASK_PRIORITY       2

#define SPD_TIMER_NAME          "SPDTimer"

#define T_LEADSIGNAL_MIN_US     7000


class SignalPauseDetector{
	enum states{
		WaitingForPause,
		WaitingForSignal
	};
public:
	SignalPauseDetector(const uint8_t& tsopReceiver, QueueHandle_t& signalChannel, QueueHandle_t& pauseChannel)
                        : tsopReceiver(tsopReceiver), signalChannel(signalChannel), pauseChannel(pauseChannel) {};

    void begin(){
        pinMode(tsopReceiver, INPUT);
        timerEventGroup = xEventGroupCreate();

        timer_args.name = SPD_TIMER_NAME;
        timer_args.callback = Timer_ISR;
        timer_args.arg = this;
        timer_args.dispatch_method = ESP_TIMER_ISR;
        esp_timer_create(&timer_args, &timer);

        xTaskCreate(Static_main, SPD_TASK_NAME, SPD_TASK_STACK_DEPTH, this, SPD_TASK_PRIORITY, NULL);
    }
private:
	uint8_t tsopReceiver;

    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

    EventGroupHandle_t timerEventGroup;

    uint32_t timerBitMask = 1;
    esp_timer_create_args_t timer_args;
    esp_timer_handle_t timer;

	uint32_t t_signalUs = 0;
	uint32_t t_pauseUs = 0;
	states state = WaitingForSignal;

    void signalDetected(uint32_t t_us){
        if(t_us > T_LEADSIGNAL_MIN_US) {
            clearQueue(signalChannel);
            clearQueue(pauseChannel);
        }
        xQueueSend(signalChannel, &t_us, 0);
    };

    void pauseDetected(uint32_t t_us){
        xQueueSend(pauseChannel, &t_us, 0);
    };

    static void clearQueue(QueueHandle_t Queue){
        uint32_t dummy;
        while(uxQueueMessagesWaiting(Queue) > 0){
            xQueueReceive(Queue, &dummy, portMAX_DELAY);
        }
    }

	void main(){
		vTaskDelay(1);
        ESP_LOGI(SPD_TASK_NAME, "Start Task.");

        esp_timer_start_periodic(timer, 100);
        while(true){
			switch(state){
				case WaitingForPause:
                    xEventGroupWaitBits(timerEventGroup, timerBitMask, pdTRUE,
                                        pdTRUE, portMAX_DELAY);

					if(!digitalRead(tsopReceiver)){
						t_signalUs+=100;
					}else{
                        // Serial.print(t_signalUs); used for debugging.
						signalDetected(t_signalUs);
						t_pauseUs = 0;
						state = WaitingForSignal;
					}
					
					break;
				case WaitingForSignal:
                    xEventGroupWaitBits(timerEventGroup, timerBitMask, pdTRUE,
                                        pdTRUE, portMAX_DELAY);

					if(digitalRead(tsopReceiver)){
						t_pauseUs+=100;
						
						if(t_pauseUs > T_MAX_PAUSE_US){
                            pauseDetected(t_pauseUs);

                            t_signalUs = 0;
                            state = WaitingForPause;
                        }
					}else{
						pauseDetected(t_pauseUs);
						
						t_signalUs = 0;
						state = WaitingForPause;
					}
					
					break;
			}
		}
	};

    static void Static_main(void* arg){
        SignalPauseDetector* runner = (SignalPauseDetector*)arg;
        runner->main();
    }

    static void Timer_ISR(void* arg){
        SignalPauseDetector* runner = (SignalPauseDetector*)arg;
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken = pdFALSE;

        xResult = xEventGroupSetBitsFromISR(runner->timerEventGroup, runner->timerBitMask, &xHigherPriorityTaskWoken);

        if(xResult == pdPASS) portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
};

#endif