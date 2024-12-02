#include "Arduino.h"

#include "IR_Blaster.h"
#include "IR_Button_Handler.h"

gpio_num_t Button = GPIO_NUM_21;

IR_Blaster IR_Blaster(33);
IR_Button_Handler IR_Button_Handler(Button);

void setup(){
    IR_Blaster.begin();
    IR_Button_Handler.begin();
    Serial.begin(115200);
}

void loop(){
    if(IR_Button_Handler.GetState()) {
        IR_Button_Handler.ResetButton();
        Serial.println("IR blaster go brr...");
        IR_Blaster.sendMessage(0x45);
    }
}