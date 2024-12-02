#include <Arduino.h>
#include "NecReceiver.h"
#include "TsopReceiver.h"

gpio_num_t tsopPin = GPIO_NUM_33;


TsopReceiver TsopReceiver(tsopPin);
NecReceiver NecReceiver(TsopReceiver);

void setup(){
	Serial.begin(115200);
    TsopReceiver.begin();
    NecReceiver.begin();

	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
}

void loop(){
    if(TsopReceiver.isSignalPresent()) Serial.println("Beep...");
}
