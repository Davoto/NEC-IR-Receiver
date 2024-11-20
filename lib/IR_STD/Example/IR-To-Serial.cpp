#include <Arduino.h>
#include "SignalPauseDetector.hpp"

uint8_t tsopPin = 32;

namespace crt {
	MainInits mainInits;            // Initialize CleanRTOS.
	NecReceiver necReceiver("Ruben", 2, 4000, ARDUINO_RUNNING_CORE);
	TsopReceiver tsopReceiver("Richard", 2, 4000, ARDUINO_RUNNING_CORE, tsopPin);
	SignalPauseDetector signalPauseDetector("Phillip", 2, 4000, ARDUINO_RUNNING_CORE, tsopReceiver, necReceiver);
}

void setup(){
	Serial.begin(115200);
	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
};

void loop(){
	vTaskDelay(1);
};
