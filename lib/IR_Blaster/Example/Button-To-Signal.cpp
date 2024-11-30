#include "Arduino.h"

#include "IR_Blaster.h"

IR_Blaster IR_Blaster(33);
uint8_t SendButton = 21;

void setup(){
    IR_Blaster.begin();
    Serial.begin(115200);
    pinMode(SendButton, INPUT_PULLDOWN);
}

void loop(){
    if(digitalRead(SendButton)) {
        Serial.println("IR blaster go brr...");
        IR_Blaster.sendMessage(0x45);
    }
}