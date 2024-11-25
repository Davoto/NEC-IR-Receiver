#include <Arduino.h>

#include "Button-To-Signal.cpp"

extern "C" {
	void app_main();
}

void app_main(void)
{
	setup();
	for(;;)
	{
		loop();
		vTaskDelay(1);  // prevent the watchdog timer to kick in for this thread.
	}
}