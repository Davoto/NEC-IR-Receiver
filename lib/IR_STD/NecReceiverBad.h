#ifndef NEC_RECEIVER_BAD_H
#define NEC_RECEIVER_BAD_H

#include "CleanRTOS.h"
#include "TsopReceiver.h"
#include "MessageToSerial.h"
#include "SignalPauseDetector.h"

#define QUEUE_SIZE 8

#define T_LEADSIGNAL_MIN_US             7000
#define T_LEADSIGNAL_MAX_US             11000
#define T_LEADPAUSE_MIN_US              3000
#define T_LEADPAUSE_MAX_US              6000
#define T_BITPAUSE_MIN_US               200
#define T_BITPAUSE_MAX_US               2000
#define T_BITPAUSE_THRESHOLD_ZERO_ONE   1100

#define NECR_TASK_NAME           "NecReceiver"
#define NECR_TASK_STACK_DEPTH    6000
#define NECR_TASK_PRIORITY       2

class NecReceiverBad{
    enum states{
        WaitingForLeadSignal,
        WaitingForLeadPause,
        WaitingForBitPause
    };
public:
	explicit NecReceiverBad(TsopReceiver& tsopReceiver)
      : signalChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))),
        pauseChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))),
        signalPauseDetector(tsopReceiver, signalChannel, pauseChannel) {};

    void begin(){
        xTaskCreate(Static_main, NECR_TASK_NAME, NECR_TASK_STACK_DEPTH, this, NECR_TASK_PRIORITY, NULL);
    }
private:
	states state = WaitingForLeadSignal;

    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

    SignalPauseDetector signalPauseDetector;

    uint32_t t_signalUs = 0;
	uint32_t t_pauseUs = 0;

	uint32_t n = 0, m = 0;
	uint64_t msg = 0;
	uint8_t nofBytes = 0;

	static void extractMessage(uint64_t& msg, uint8_t& nofBytes, uint64_t m, unsigned int n){
		// revert bits:
		msg = 0;
		uint64_t mloc = m;

		for (int i=0; i<n; i++){
        msg<<=(mloc&1);
        mloc=mloc>>1;
		};
		nofBytes = n/8;
	};

	void main(){
		while(true){
			switch(state){
				case WaitingForLeadSignal:
                    xQueueReceive(signalChannel, &t_signalUs, portMAX_DELAY);

					if(t_signalUs > T_LEADSIGNAL_MIN_US && t_signalUs < T_LEADSIGNAL_MAX_US) state = WaitingForLeadPause;

					break;
				case WaitingForLeadPause:
                    xQueueReceive(pauseChannel, &t_pauseUs, portMAX_DELAY);

					if(t_pauseUs > T_LEADPAUSE_MIN_US && t_pauseUs < T_LEADPAUSE_MAX_US){
						state = WaitingForBitPause;
						n = 0;
						m = 0;
					}else state = WaitingForLeadSignal;

					break;
				case WaitingForBitPause:
                    xQueueReceive(pauseChannel, &t_pauseUs, portMAX_DELAY);

					if(t_pauseUs > T_BITPAUSE_MIN_US && t_pauseUs < T_BITPAUSE_MAX_US){
						m <<= 1;

						if(t_pauseUs > T_BITPAUSE_THRESHOLD_ZERO_ONE) m |= 1;

						n++;
					}else {
						extractMessage(msg, nofBytes, m, n);
						MessageToSerial::messageReceived(msg, nofBytes);
						state = WaitingForLeadSignal;
					}

					break;
			}
        }
	};

    static void Static_main(void* arg){
        NecReceiverBad* runner = (NecReceiverBad*)arg;
        runner->main();
    }
};

#endif