#ifndef NEC_RECEIVER_H
#define NEC_RECEIVER_H

#include "CleanRTOS.h"

#define NR_TASK_NAME           "NecReceiver"
#define NR_TASK_STACK_DEPTH    6000
#define NR_TASK_PRIORITY       2

#define QUEUE_SIZE 8

#define T_LEADSIGNAL_MIN_US             7000
#define T_LEADSIGNAL_MAX_US             11000
#define T_LEADPAUSE_MIN_US              3000
#define T_LEADPAUSE_MAX_US              6000
#define T_BITPAUSE_MIN_US               200
#define T_BITPAUSE_MAX_US               2000
#define T_BITPAUSE_THRESHOLD_ZERO_ONE   1100

class NecReceiver{
    enum Receiver_State{
        Start,
        Address,
        Message,
    };
public:
    explicit NecReceiver(const gpio_num_t& IR_Pin) : IR_Pin(IR_Pin), signalChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))),
    pauseChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))) {};

    void begin(){
        xTaskCreate(Static_Main_Loop, NR_TASK_NAME, NR_TASK_STACK_DEPTH, this, NR_TASK_PRIORITY, NULL);
    }

    uint16_t getMessage(){
        return 1;
    }
private:
    gpio_num_t IR_Pin;
    bool Interrupt_State = false;
    Receiver_State State = Start;

    uint32_t Time_Buffer = 0;

    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

    void Main_Loop(){
        while(true){
            if(1){
                switch (State) {
                    case Start:
                        break;
                    case Address:
                        break;
                    case Message:
                        break;
                }
            }
        }
    }

    static void Static_Main_Loop(void* arg){
        NecReceiver* runner = (NecReceiver*)arg;
        runner->Main_Loop();
    }
};

#endif //NEC_RECEIVER_H
