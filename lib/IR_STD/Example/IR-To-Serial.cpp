#include <Arduino.h>
#include "SignalPauseDetector.h"
#include "NecReceiver.h"
#include "TsopReceiver.h"

uint8_t tsopPin = 32;

namespace crt {
	// MainInits mainInits;            // Initialize CleanRTOS.
	NecReceiver necReceiver("Ruben", 2, 4000, 1);
	TsopReceiver tsopReceiver("Richard", 2, 4000, 1, tsopPin);
	SignalPauseDetector signalPauseDetector("Phillip", 2, 4000, 1, tsopReceiver, necReceiver);
}

void setup(){
	Serial.begin(115200);
	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
};

void loop(){
	vTaskDelay(1);
};
