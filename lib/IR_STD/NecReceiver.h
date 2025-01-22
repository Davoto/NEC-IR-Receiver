#ifndef NEC_RECEIVER_H
#define NEC_RECEIVER_H

class NecReceiver {
public:
    struct MessageData {
        uint64_t msg = 0;
        uint8_t nofBytes = 0;
        int64_t timeOfExtractionUs = 0;
    };

    /**
     * Constructor for NecReceiver-class, initializes the signalChannel- and pauseChannel-Queue.
     */
    explicit NecReceiver()
            : signalChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))),
              pauseChannel(xQueueCreate(QUEUE_SIZE, sizeof(uint32_t))) {};

    /**
     * Begin function to start the task of NecReceiver.
     */
    void begin() {
        xTaskCreate(Static_main, NECR_TASK_NAME, NECR_TASK_STACK_DEPTH, this, NECR_TASK_PRIORITY, &Task);
    }

    /**
     * Stop function to stop the NecReceiver task.
     */
    void stop() {
        vTaskDelete(&Task);
    }

    /**
 * Function to add to signal queue when signal is above "T_LEADSIGNAL_MIN_US", this also clears both queue's first
 * to start a new message.
 * @param t_us Time in microseconds there was a on signal transmitted to "tsopReceiver".
 */
    void signalDetected(uint32_t t_us) {
        if (t_us > T_LEADSIGNAL_MIN_US) {
            clearQueue(signalChannel);
            clearQueue(pauseChannel);
            xQueueSend(signalChannel, &t_us, 0);
        }
    };

    /**
     * Function to send how long in microseconds there has been no signal from "tsopReceiver".
     * @param t_us Time in microseconds.
     */
    void pauseDetected(uint32_t t_us) {
        xQueueSend(pauseChannel, &t_us, 0);
    };

    /**
     * Function to get the last received message, number of bytes and timestamp of extraction.
     * @param msgHolder Variable to get latest message.
     * @param nofBytesHolder Variable to get latest number of bytes.
     * @param timeHolder Variable to get last time of Extraction.
     */
    MessageData latestRawMsgData() {
        return messageData;
    }

    bool verifyMessage(MessageData &data) {
        uint64_t msgHolder = data.msg;
        uint8_t nofBytesHolder = data.nofBytes / 2;
        uint64_t newMsg = 0;
        for (uint8_t i = 0; i < nofBytesHolder; i++) {
            uint8_t byte = msgHolder >> (i * 16);
            uint8_t reverseByte = ~msgHolder >> (i * 16 + 8);
            if (byte == reverseByte) {
                newMsg = (newMsg << i * 8) | byte;
            } else {
                ESP_LOGE(NECR_TASK_NAME, "Byte: %X reverseByte: %X", byte, reverseByte);
                ESP_LOGE(NECR_TASK_NAME, "Could not verify message.");
                return false;
            }
        }
        data.msg = newMsg;
        data.nofBytes = nofBytesHolder;
        return true;
    }

private:
    /**
     * Settings used for the NecReceiver Task.
     */
    const char NECR_TASK_NAME[12] = "NecReceiver";
    const uint16_t NECR_TASK_STACK_DEPTH = 6000;
    const uint8_t NECR_TASK_PRIORITY = 2;
    TaskHandle_t Task;

    /**
     * Setting used for Queue's within this class.
     */
    const uint8_t QUEUE_SIZE = 8;

    /**
     * Timing settings for NEC-protocol.
     */
    const uint16_t T_LEADSIGNAL_MIN_US = 7000;
    const uint16_t T_LEADSIGNAL_MAX_US = 11000;
    const uint16_t T_LEADPAUSE_MIN_US = 3000;
    const uint16_t T_LEADPAUSE_MAX_US = 6000;
    const uint8_t T_BITPAUSE_MIN_US = 200;
    const uint16_t T_BITPAUSE_MAX_US = 2000;
    const uint16_t T_BITPAUSE_THRESHOLD_ZERO_ONE = 1100;

    /**
     * Different states of this Task's main-loop.
     */
    enum states {
        WaitingForLeadSignal,
        WaitingForLeadPause,
        WaitingForBitPause
    };
    states state = WaitingForLeadSignal;

    /**
     * Queue's used in this class.
     */
    QueueHandle_t signalChannel;
    QueueHandle_t pauseChannel;

    /**
     * Variables that are used to hold the latest Queue-data.
     */
    uint32_t t_signalUs = 0;
    uint32_t t_pauseUs = 0;

    /**
     * Variables used to extract and hold messages received using this task.
     */
    uint32_t nofBits = 0;
    uint64_t rawMsg = 0;
    MessageData messageData;

    /**
     * Function to convert raw bits filtered from the NecReceiver task to a properly oriented message.
     * @param msg Holds final message after usage.
     * @param nofBytes Holds final amount of bytes after usage.
     * @param m Holds raw message data.
     * @param n Holds raw number of bits.
     */
    static void extractMessage(uint64_t &msg, uint8_t &nofBytes, uint64_t m, unsigned int n, int64_t &time) {
        // revert bits:
        msg = 0;
        uint64_t mloc = m;
        time = esp_timer_get_time();

        for (int i = 0; i < n; i++) {
            msg <<= 1;
            msg |= (mloc & 1);
            mloc = mloc >> 1;
        };
        nofBytes = n / 8;
    };

    /**
     * Function to clear a queue.
     * @param Queue Queue to be cleared.
     */
    static void clearQueue(QueueHandle_t Queue) {
        uint32_t dummy;
        while (uxQueueMessagesWaiting(Queue) > 0) {
            xQueueReceive(Queue, &dummy, portMAX_DELAY);
        }
    }

    /**
     * <b>Function that runs as task. Switches between three states:</b><br><br>
     *
     * <b>WaitingForLeadSignal</b>: Waits for a queue item in "signalChannel" that's between "T_LEADSIGNAL_MIN_US" and
     * "T_LEADSIGNAL_MAX_US" to change state to WaitingForLeadPause.<br><br>
     *
     * <b>WaitingForLeadPause</b>: Waits for a queue item in "pauseChannel" that's between "T_LEADPAUSE_MIN_US" and
     * "T_LEADPAUSE_MAX_US" if the frontmost item in the queue is between these values variable's "nofBits" and "rawMsg"
     * are set to 0 and the state Changes to WaitingForBitPause, if the item from the queue is not between these first
     * values the state will revert to WaitingForLeadSignal.<br><br>
     *
     * <b>WaitingForBitPause</b>: Waits for a queue item in "pauseChannel" that's between "T_BITPAUSE_MIN_US" and
     * "T_BITPAUSE_MAX_US" if the queue item is between these values "rawMsg" will be shifted to the left, nofBits will
     * add 1 and depending on if the queue item is above "T_BITPAUSE_THRESHOLD_ZERO_ONE" the least significant bit in
     * "rawMsg" will be flipped positive. If the queue item is not between the two first two values it will extract the
     * "rawMsg" and print the changed "msg" and "nofBytes" before changing the state back to WaitingForLeadSignal.
     */
    void main() {
        ESP_LOGI(NECR_TASK_NAME, "Start Task.");

        while (true) {
            switch (state) {
                case WaitingForLeadSignal:
                    // ESP_LOGI(NECR_TASK_NAME, "Lead"); used for debugging.
                    xQueueReceive(signalChannel, &t_signalUs, portMAX_DELAY);

                    if ((t_signalUs > T_LEADSIGNAL_MIN_US) && (t_signalUs < T_LEADSIGNAL_MAX_US))
                        state = WaitingForLeadPause;
                    break;
                case WaitingForLeadPause:
                    xQueueReceive(pauseChannel, &t_pauseUs, portMAX_DELAY);

                    if ((t_pauseUs > T_LEADPAUSE_MIN_US) && (t_pauseUs < T_LEADPAUSE_MAX_US)) {
                        state = WaitingForBitPause;
                        nofBits = 0;
                        rawMsg = 0;
                    } else state = WaitingForLeadSignal;
                    break;
                case WaitingForBitPause:
                    xQueueReceive(pauseChannel, &t_pauseUs, portMAX_DELAY);

                    if ((t_pauseUs > T_BITPAUSE_MIN_US) && (t_pauseUs < T_BITPAUSE_MAX_US)) {
                        rawMsg <<= 1;

                        if (t_pauseUs > T_BITPAUSE_THRESHOLD_ZERO_ONE) rawMsg |= 1;

                        nofBits++;
                    } else {
                        extractMessage(messageData.msg, messageData.nofBytes, rawMsg, nofBits,
                                       messageData.timeOfExtractionUs);
                        // Debug for printing;
                        // ESP_LOGI(NECR_TASK_NAME, "Raw Message received:");
                        // ESP_LOGI(NECR_TASK_NAME, "nofBytes: %" PRIx32, messageData.nofBytes);
                        // ESP_LOGI(NECR_TASK_NAME, "%" PRIx64, messageData.msg);
                        // if(verifyMessage(messageData)){
                        //     ESP_LOGI(NECR_TASK_NAME, "Verified Message received:");
                        //     ESP_LOGI(NECR_TASK_NAME, "nofBytes: %" PRIx32, messageData.nofBytes);
                        //     ESP_LOGI(NECR_TASK_NAME, "%" PRIx64, messageData.msg);
                        // }else ESP_LOGE(NECR_TASK_NAME, "Message received incomplete.");
                        state = WaitingForLeadSignal;
                    }
                    break;
            }
        }
    };

    /**
     * Function to run a non-static function in a Task, through pointing 🫵.
     */
    static void Static_main(void *arg) {
        NecReceiver *runner = (NecReceiver *) arg;
        runner->main();
    }
};

#endif