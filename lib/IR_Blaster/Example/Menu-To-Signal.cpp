#include "Arduino.h"

#include "IR_Blaster.h"
#include "IR_Simple_Remote.h"
#include "cJSON.h"

gpio_num_t SendButton = GPIO_NUM_21;
gpio_num_t UpButton = GPIO_NUM_18;
gpio_num_t DownButton = GPIO_NUM_19;
uint8_t Address = 0x00;
cJSON* JsonList = cJSON_Parse(R"([{"name": "Richard", "id": 69}, {"name": "Boudewijn", "id": 70}, {"name": "Willemijn", "id": 71}])");

IR_Blaster IR_Blaster(33);
IR_Simple_Remote IR_Simple_Remote(SendButton, UpButton, DownButton, JsonList, Address, IR_Blaster);

void setup(){
    IR_Blaster.begin();
    Serial.begin(115200);
}

void loop(){
    if(digitalRead(SendButton)) {
        Serial.println("IR blaster go brr...");
        IR_Blaster.sendMessage(0x45);
    }
}