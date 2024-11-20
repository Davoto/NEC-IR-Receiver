#include <Arduino.h>
#include "nvs_flash.h" // nodig voor WIFI functionaliteit via Arduino IDE

#include "IR-To-Serial.cpp"

extern "C" {
	void app_main();
}

void app_main(void)
{
	// Initialisatie van NVS, nodig voor WIFI functionaliteit
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());  // Wis de NVS-partitie en probeer opnieuw
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	setup();
	for(;;)
	{
		loop();
		vTaskDelay(1);  // prevent the watchdog timer to kick in for this thread.
	}
}