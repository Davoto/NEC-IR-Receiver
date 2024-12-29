#ifndef SIGNAL_PAUSE_DETECTOR_H
#define SIGNAL_PAUSE_DETECTOR_H

#include <Arduino.h>

class SignalPauseDetector{
public:
    /**
     * Constructor for SignalPauseDetector class.
     * @param tsopReceiver Pin used by necReceiver for reading out a tsop sensor, tested with a VS1838B.
     * @param signalChannel Queue used by this class to insert the latest signal on time.
     * @param pauseChannel Queue used by this class to insert the latest signal off time.
     */
	SignalPauseDetector(const uint8_t& tsopReceiver, QueueHandle_t& signalChannel, QueueHandle_t& pauseChannel)
                        : tsopReceiver(tsopReceiver), signalChannel(signalChannel), pauseChannel(pauseChannel) {};

    /**
     * Begin function to setup the tsopReceiver, configure the timer and start the task.
     */
    void begin(){
        pinMode(tsopReceiver, INPUT);
        timerEventGroup = xEventGroupCreate();

        timer_args.name = SPD_TIMER_NAME;
        timer_args.callback = Timer_ISR;
        timer_args.arg = this;
        timer_args.dispatch_method = ESP_TIMER_ISR;
        esp_timer_create(&timer_args, &timer);

        xTaskCreate(Static_main, SPD_TASK_NAME, SPD_TASK_STACK_DEPTH, this, SPD_TASK_PRIORITY, &Task);
    }

    /**
     * Stop function to stop the timer and task.
     */
    void stop(){
        esp_timer_stop(timer);
        vTaskDelete(&Task);
    }
private:
    /**
     * Settings used for the NecReceiver Task.
     */
    const char SPD_TASK_NAME[20] = "SignalPauseDetector";
    const uint16_t SPD_TASK_STACK_DEPTH = 6000;
    const uint8_t  SPD_TASK_PRIORITY    = 2;
    TaskHandle_t Task;

    /**
     * Timing settings for NEC-Protocol.
     */
    const uint16_t T_LEADSIGNAL_MIN_US  = 7000;
    const uint16_t T_MAX_PAUSE_US       = 6000;

    /**
     * Pin used to read out tsopReceiver.
     */
    uint8_t tsopReceiver;

    /**
     * States used by the task of this class.
     */
    enum states{
        WaitingForPause,
        WaitingForSignal
    };
    states state = WaitingForSignal;

    /**
     * Queue's used by this class.
     */
    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

    /**
     * Configuration used by hardware timer.
     */
    const char SPD_TIMER_NAME[9] = "SPDTimer";
    EventGroupHandle_t timerEventGroup;
    uint32_t timerBitMask = 1;
    esp_timer_create_args_t timer_args;
    esp_timer_handle_t timer;

    /**
     * Variables that are used to hold the latest signal data for queue.
     */
	uint32_t t_signalUs = 0;
	uint32_t t_pauseUs = 0;

    /**
     * Function to add to signal queue when signal is above "T_LEADSIGNAL_MIN_US", this also clears both queue's first
     * to start a new message.
     * @param t_us Time in microseconds there was a on signal transmitted to "tsopReceiver".
     */
    void signalDetected(uint32_t t_us){
        if(t_us > T_LEADSIGNAL_MIN_US) {
            clearQueue(signalChannel);
            clearQueue(pauseChannel);
            xQueueSend(signalChannel, &t_us, 0);
        }
    };

    /**
     * Function to send how long in microseconds there has been no signal from "tsopReceiver".
     * @param t_us Time in microseconds.
     */
    void pauseDetected(uint32_t t_us){
        xQueueSend(pauseChannel, &t_us, 0);
    };

    /**
     * Function to clear a queue.
     * @param Queue Queue to be cleared.
     */
    static void clearQueue(QueueHandle_t Queue){
        uint32_t dummy;
        while(uxQueueMessagesWaiting(Queue) > 0){
            xQueueReceive(Queue, &dummy, portMAX_DELAY);
        }
    }

    /**
     * <b>Function that runs as task. Starts a timer that sets a event-bit every 100 microseconds and then switches
     * between these states:</b><br><br>
     *
     * <b>WaitingForPause<b/>: Waits for bit in "timerEventGroup" to flip, than reads "tsopReceiver". If the reading
     * from tsopReceiver is "low" it adds 100 microseconds to "t_signalUs", when "high" it uses signalDetected() with
     * "t_signalUs", resets "t_pauseUs" to 0 and changes state to WaitingForSignal.<br><br>
     *
     * <b>WaitingForSignal<b/>: Waits for bit in "timerEventGroup" to flip, than reads "tsopReceiver". If the reading
     * from tsopReceiver is "high" it adds 100 microseconds to "t_pauseUs", when "low" or when t_pauseUs exceeds
     * "T_MAX_PAUSE_US" it uses pauseDetected() with "t_pauseUs", resets "t_signalUs" to 0 and changes state to
     * WaitingForPause.<br><br>
     */
	void main(){
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

    /**
     * Function to run a non-static function in a Task, through pointing 🫵.
     */
    static void Static_main(void* arg){
        SignalPauseDetector* runner = (SignalPauseDetector*)arg;
        runner->main();
    }

    /**
     * This function is called when the timer callback function is called, it sets a bit in the "timerEventGroup".
     * @param arg This should be a pointer to the class instance itself.
     */
    static void Timer_ISR(void* arg){
        SignalPauseDetector* runner = (SignalPauseDetector*)arg;
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken = pdFALSE;

        xResult = xEventGroupSetBitsFromISR(runner->timerEventGroup, runner->timerBitMask, &xHigherPriorityTaskWoken);

        if(xResult == pdPASS) portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
};

#endif