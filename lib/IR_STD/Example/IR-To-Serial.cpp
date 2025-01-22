#include <Arduino.h>
#include "NecReceiver.h"
#include "SignalPauseDetector.h"

uint8_t tsopPin = GPIO_NUM_33;
const char MAIN[] = "MAIN";

NecReceiver necReceiver = NecReceiver();
SignalPauseDetector signalPauseDetector(tsopPin, necReceiver);
NecReceiver::MessageData messageData;

void setup(){
	Serial.begin(115200);
    necReceiver.begin();
    signalPauseDetector.begin();

	ESP_LOGI("checkpoint", "start of main");
	
	vTaskDelay(1000);
}

void loop(){
    NecReceiver::MessageData newMessageData = necReceiver.latestRawMsgData();
    if(messageData.timeOfExtractionUs != newMessageData.timeOfExtractionUs) {
        messageData = newMessageData;
        ESP_LOGI(MAIN, "Raw Message received:");
        ESP_LOGI(MAIN, "nofBytes: %" PRIx32, messageData.nofBytes);
        ESP_LOGI(MAIN, "%" PRIx64, messageData.msg);
        if(necReceiver.verifyMessage(messageData)){
           ESP_LOGI(MAIN, "Verified Message received:");
           ESP_LOGI(MAIN, "nofBytes: %" PRIx32, messageData.nofBytes);
           ESP_LOGI(MAIN, "%" PRIx64, messageData.msg);
        }
    }
}
