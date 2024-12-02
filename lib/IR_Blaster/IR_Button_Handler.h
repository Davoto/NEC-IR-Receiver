#ifndef IR_BUTTON_HANDLER_H
#define IR_BUTTON_HANDLER_H

#include "Arduino.h"

class IR_Button_Handler{
public:
    explicit IR_Button_Handler(const gpio_num_t& Button) : Button(Button) {}

    void begin(){
        gpio_install_isr_service(0);
        gpio_reset_pin(Button);
        gpio_set_direction(Button, GPIO_MODE_INPUT);
        gpio_pullup_en(Button);
        gpio_set_intr_type(Button, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(Button, Button_Intr_Handler, NULL);
    }

    bool GetState(){
        return ButtonPressed;
    }

    void ResetButton(){
        ButtonPressed = false;
    }
private:
    gpio_num_t Button;
    volatile bool ButtonPressed = false;

    static void Button_Intr_Handler(void* arg){
        IR_Button_Handler* runner = (IR_Button_Handler*)arg;
        runner->ButtonPress();
    }

    void ButtonPress(){
        ButtonPressed = true;
    }
};



#endif //IR_BUTTON_HANDLER_H
