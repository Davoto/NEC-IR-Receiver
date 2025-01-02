#include <Arduino.h>
#include "NecReceiver.h"
#include "SignalPauseDetector.h"

uint8_t tsopPin = GPIO_NUM_33;

NecReceiver necReceiver = NecReceiver();
SignalPauseDetector signalPauseDetector(tsopPin, necReceiver);

void setup(){
	Serial.begin(115200);
    necReceiver.begin();
    signalPauseDetector.begin();

	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
}

void loop(){
}
