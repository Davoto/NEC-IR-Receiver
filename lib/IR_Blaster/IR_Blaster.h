#ifndef IR_BLASTER_H
#define IR_BLASTER_H

// Values for proper PWM bursts
#define BURST_ON         50
#define BURST_OFF        0

// Delays in microseconds for different bits.
#define START_POSITIVE   9000
#define START_NEGATIVE   4500
#define ONE_POSITIVE     560
#define ONE_NEGATIVE     1690
#define ZERO_POSITIVE    560
#define ZERO_NEGATIVE    560

// Standard ledcSetup() settings.
#define PWM_CHANNEL      0
#define NEC_KHZ          38000
#define PWM_RESOLUTION   8

class IR_Blaster {
public:
    explicit IR_Blaster(uint8_t Pin) : Pin(Pin) {};

    void begin() const {
        ledcSetup(PWM_CHANNEL, NEC_KHZ, PWM_RESOLUTION);
        ledcAttachPin(Pin, PWM_CHANNEL);
    };

    void sendMessage(uint32_t Message, uint8_t Adress = 0x00, uint8_t N_Bytes = 1) {
        sendStartBit();
        sendByte(Adress);

        switch (N_Bytes) {
            case 1:
                sendByte(Message);
                break;
            case 2:
                sendByte(Message);
                sendByte(Message >> 8);
                break;
            case 3:
                sendByte(Message);
                sendByte(Message >> 8);
                sendByte(Message >> 16);
                break;
            case 4:
                sendByte(Message);
                sendByte(Message >> 8);
                sendByte(Message >> 16);
                sendByte(Message >> 24);
                break;
        }

        sendZeroBit();
    };

    void sendByte(const uint8_t& Message) {
        uint8_t PositiveMsg = Message;
        uint8_t NegativeMsg = ~Message;

        // Send positive version of byte.
        for (int j = 0; j < 8; j++) {
            if (PositiveMsg % 2) {
                sendOneBit();
            } else sendZeroBit();

            PositiveMsg >>= 1;
        }

        // Send negative version of byte as check.
        for (int k = 0; k < 8; k++) {
            if (NegativeMsg % 2) {
                sendOneBit();
            } else sendZeroBit();

            NegativeMsg >>= 1;
        }
    }

    void sendStartBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(START_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(START_NEGATIVE);
    };

private:
    uint8_t Pin;

    void sendOneBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ONE_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ONE_NEGATIVE);
    };

    void sendZeroBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ZERO_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ZERO_NEGATIVE);
    };
};

#endif