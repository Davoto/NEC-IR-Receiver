#include <Arduino.h>
#include "CleanRTOS.h"
#include "NecReceiver.h"

const gpio_num_t tsopPin = GPIO_NUM_33;
NecReceiver necReceiver(tsopPin);

void setup(){
	Serial.begin(115200);
    necReceiver.begin();

	vTaskDelay(1000);
}

void loop(){

}
