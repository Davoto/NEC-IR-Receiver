#ifndef IR_SIMPLE_REMOTE_H
#define IR_SIMPLE_REMOTE_H

#include "CleanRTOS.h"
#include "Arduino.h"
#include "cJSON.h"
#include "IR_Button_Handler.h"

// Signal Settings
#define JSON_SIGNAl_STRING      "id"
#define JSON_SIGNAL_SIZE_TAG    "n_bytes"

// Task Settings
#define SR_TASK_NAME           "IR_Simple_Remote"
#define SR_TASK_STACK_DEPTH    6000
#define SR_TASK_PRIORITY       2

class IR_Simple_Remote{
public:
    IR_Simple_Remote(const gpio_num_t& ButtonSendPin, const gpio_num_t& ButtonUpPin, const gpio_num_t& ButtonDownPin,
                     cJSON* JsonList, const uint8_t& Address, IR_Blaster& IR_Blaster) :

            ButtonSendPin(ButtonSendPin), ButtonUpPin(ButtonUpPin), ButtonDownPin(ButtonDownPin), JsonList(JsonList),
            Address(Address), IR_Blaster_(IR_Blaster) {};

    void begin(){
        ButtonSend.begin();
        ButtonUp.begin();
        ButtonDown.begin();
        xTaskCreate(Static_main, SR_TASK_NAME, SR_TASK_STACK_DEPTH, this, SR_TASK_PRIORITY, NULL);
    }

private:
    gpio_num_t ButtonSendPin, ButtonUpPin, ButtonDownPin;
    cJSON* JsonList;
    uint8_t Address;
    IR_Blaster IR_Blaster_;
    IR_Button_Handler ButtonSend = IR_Button_Handler(ButtonSendPin);
    IR_Button_Handler ButtonUp = IR_Button_Handler(ButtonUpPin);
    IR_Button_Handler ButtonDown = IR_Button_Handler(ButtonDownPin);
    int Coordinate = 0;
    cJSON* CurrentItem = cJSON_GetArrayItem(JsonList, Coordinate);

    void main(){
        while(true){
            if(ButtonSend.GetState()){
                ButtonSend.ResetButton();
                uint32_t Message = cJSON_GetObjectItemCaseSensitive(CurrentItem, JSON_SIGNAl_STRING)->valueint;
                uint8_t N_Bytes = cJSON_GetObjectItemCaseSensitive(CurrentItem, JSON_SIGNAL_SIZE_TAG)->valueint;
                IR_Blaster_.sendMessage(Message, Address, N_Bytes);
                Serial.println("Pressed Send, of this signal:");
                Serial.println(cJSON_Print(cJSON_GetArrayItem(JsonList, Coordinate)));
            }
            if(ButtonUp.GetState()){
                ButtonUp.ResetButton();
                if(Coordinate > 0) Coordinate--;
                CurrentItem = cJSON_GetArrayItem(JsonList, Coordinate);
                Serial.println("Pressed Up, now selected:");
                Serial.println(cJSON_Print(cJSON_GetArrayItem(JsonList, Coordinate)));
            }
            if(ButtonDown.GetState()){
                ButtonDown.ResetButton();
                if(Coordinate < (cJSON_GetArraySize(JsonList) - 1)) Coordinate++;
                CurrentItem = cJSON_GetArrayItem(JsonList, Coordinate);
                Serial.println("Pressed Down, now selected:");
                Serial.println(cJSON_Print(cJSON_GetArrayItem(JsonList, Coordinate)));
            }

            delay(200);
        }
    }

    static void Static_main(void* arg){
        IR_Simple_Remote* runner = (IR_Simple_Remote*)arg;
        runner->main();
    };
};

#endif
