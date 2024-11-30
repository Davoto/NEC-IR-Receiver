#ifndef IR_SIMPLE_REMOTE_H
#define IR_SIMPLE_REMOTE_H

#include "Arduino.h"
#include "CleanRTOS.h"
#include "cJSON.h"

#define JSON_SIGNAl_STRING  "id"
#define MESSAGE_N_BYTES     1
#define TASK_NAME           "IR_Simple_Remote"
#define TASK_STACK_DEPTH    6000
#define TASK_PRIORITY       2

volatile bool ButtonSend_Pressed = false;
volatile bool ButtonUp_Pressed = false;
volatile bool ButtonDown_Pressed = false;

class IR_Simple_Remote{
public:
    IR_Simple_Remote(const gpio_num_t& ButtonSend, const gpio_num_t& ButtonUp, const gpio_num_t& ButtonDown,
                     cJSON* JsonList, const uint8_t& Address, IR_Blaster& IR_Blaster) :

            ButtonSend(ButtonSend), ButtonUp(ButtonUp), ButtonDown(ButtonDown), JsonList(JsonList),
            Address(Address), IR_Blaster(IR_Blaster) {};

    void begin(){
        // initialize all pins to act as interrupts.
        gpio_install_isr_service(0);

        gpio_reset_pin(ButtonSend);
        gpio_reset_pin(ButtonUp);
        gpio_reset_pin(ButtonDown);

        gpio_set_direction(ButtonSend, GPIO_MODE_INPUT);
        gpio_set_direction(ButtonUp, GPIO_MODE_INPUT);
        gpio_set_direction(ButtonDown, GPIO_MODE_INPUT);

        gpio_pullup_en(ButtonSend);
        gpio_pullup_en(ButtonUp);
        gpio_pullup_en(ButtonDown);

        gpio_set_intr_type(ButtonSend, GPIO_INTR_POSEDGE);
        gpio_set_intr_type(ButtonUp, GPIO_INTR_POSEDGE);
        gpio_set_intr_type(ButtonDown, GPIO_INTR_POSEDGE);

        gpio_isr_handler_add(ButtonSend, ButtonSend_Intr_Handler, NULL);
        gpio_isr_handler_add(ButtonUp, ButtonUp_Intr_Handler, NULL);
        gpio_isr_handler_add(ButtonDown, ButtonDown_Intr_Handler, NULL);

        xTaskCreate(Static_main, TASK_NAME, TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL);
    }

    void main(){
        while(true){
            if(ButtonSend_Pressed){
                ButtonSend_Pressed = false;
                uint32_t Message = cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(JsonList, Address), JSON_SIGNAl_STRING));
                IR_Blaster.sendMessage(Message, Address, MESSAGE_N_BYTES);
                Serial.println("Pressed Send, of this signal:");
                Serial.println(cJSON_Print(JsonList));
            }
            if(ButtonUp_Pressed){
                ButtonUp_Pressed = false;
                if(Coordinate > 0) Coordinate--;
                Serial.println("Pressed Up, now selected:");
                Serial.println(cJSON_Print(cJSON_GetArrayItem(JsonList, Address)));
            }
            if(ButtonDown_Pressed){
                ButtonDown_Pressed = false;
                if(Coordinate > (cJSON_GetArraySize(JsonList) - 1)) Coordinate++;
                Serial.println("Pressed Down, now selected:");
                Serial.println(cJSON_Print(cJSON_GetArrayItem(JsonList, Address)));
            }

            delay(10);
        }
    }

    static void Static_main(void* arg){
        IR_Simple_Remote* runner = (IR_Simple_Remote*)arg;
        runner->main();
    };
private:
    gpio_num_t ButtonSend, ButtonUp, ButtonDown;
    cJSON* JsonList;
    uint32_t Coordinate = 0;
    uint8_t Address;
    IR_Blaster IR_Blaster;

    static void ButtonSend_Intr_Handler(void* arg){
        ButtonSend_Pressed = true;
    }

    static void ButtonUp_Intr_Handler(void* arg){
        ButtonUp_Pressed = true;
    }

    static void ButtonDown_Intr_Handler(void* arg){
        ButtonDown_Pressed = true;
    }
};

#endif
