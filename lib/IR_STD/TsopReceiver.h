#ifndef TSOP_RECEIVER_H
#define TSOP_RECEIVER_H

#include <Arduino.h>
#include "CleanRTOS.h"

class TsopReceiver{
public:
    explicit TsopReceiver(const gpio_num_t& Pin) : Pin(Pin) {};

    void begin(){
        gpio_set_direction(Pin, GPIO_MODE_INPUT);
        gpio_set_pull_mode(Pin, GPIO_PULLUP_ONLY);
    }

	bool isSignalPresent(){
        return !gpio_get_level(Pin);
	};
private:
    gpio_num_t Pin;
};

#endif