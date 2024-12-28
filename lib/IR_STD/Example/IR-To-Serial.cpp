#include <Arduino.h>
#include "NecReceiver.h"

uint8_t tsopPin = GPIO_NUM_33;

NecReceiver necReceiver(tsopPin);

void setup(){
	Serial.begin(115200);
    necReceiver.begin();

	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
}

void loop(){
}
