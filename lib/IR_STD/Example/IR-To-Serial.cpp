#include <Arduino.h>
#include "NecReceiver.h"
#include "TsopReceiver.h"

gpio_num_t tsopPin = GPIO_NUM_33;


TsopReceiver tsopReceiver(tsopPin);
NecReceiver necReceiver(tsopReceiver);

void setup(){
	Serial.begin(115200);
    tsopReceiver.begin();
    necReceiver.begin();

	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
}

void loop(){
}
