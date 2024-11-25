#ifndef NEC_RECEIVER_H
#define NEC_RECEIVER_H

#include "crt_CleanRTOS.h"
#include "TsopReceiver.h"
#include "MessageToSerial.h"
#include "SignalPauseDetector.h"

namespace crt{
class NecReceiver: public Task{
    enum states{
        WaitingForLeadSignal,
        WaitingForLeadPause,
        WaitingForBitPause
    };
public:
	NecReceiver(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber) :
		Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber), signalChannel(this), pauseChannel(this) {
			start();
		};

	void signalDetected(uint32_t t_us){
		if(t_us > T_LEADSIGNAL_MIN_US) {
            signalChannel.clear();
            pauseChannel.clear();
        };
		signalChannel.write(t_us);
	};

	void pauseDetected(uint32_t t_us){
		pauseChannel.write(t_us);
	};
private:
	uint16_t T_LEADSIGNAL_MIN_US = 7000;
	uint16_t T_LEADSIGNAL_MAX_US = 11000;
	uint16_t T_LEADPAUSE_MIN_US = 3000;
	uint16_t T_LEADPAUSE_MAX_US = 6000;
	uint16_t T_BITPAUSE_MIN_US = 200;
	uint16_t T_BITPAUSE_MAX_US = 2000;
	uint16_t T_BITPAUSE_THRESHOLD_ZERO_ONE = 1100;

	states state = WaitingForLeadSignal;
	Queue<uint32_t, 5> signalChannel;
	Queue<uint32_t, 5> pauseChannel;
	uint32_t t_signalUs{};
	uint32_t t_pauseUs{};

	uint32_t n{}, m{};
	uint64_t msg{};
	uint8_t nofBytes{};

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
		vTaskDelay(1000);

		while(true){
			switch(state){
				case WaitingForLeadSignal:
					signalChannel.read(t_signalUs);

					if(t_signalUs > T_LEADSIGNAL_MIN_US && t_signalUs < T_LEADSIGNAL_MAX_US) state = WaitingForLeadPause;

					break;
				case WaitingForLeadPause:
					pauseChannel.read(t_pauseUs);

					if(t_pauseUs > T_LEADPAUSE_MIN_US && t_pauseUs < T_LEADPAUSE_MAX_US){
						state = WaitingForBitPause;
						n = 0;
						m = 0;
					}else state = WaitingForLeadSignal;

					break;
				case WaitingForBitPause:
					pauseChannel.read(t_pauseUs);

					if(t_pauseUs > T_BITPAUSE_MIN_US && t_pauseUs < T_BITPAUSE_MAX_US){
						m <<= 1;

						if(t_pauseUs > T_BITPAUSE_THRESHOLD_ZERO_ONE) m |= 1;

						n++;
					}else {
						extractMessage(msg, nofBytes, m, n);
						MessageToSerial::messageReceived(msg, nofBytes);
						state = WaitingForLeadSignal;
					};

					break;
			}
		};
	};
};
};

#endif