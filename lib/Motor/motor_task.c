#include "motor_task.h"
#include "motor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

ESP_EVENT_DECLARE_BASE(MOTOR_EVENT);

static const char* TAG = "MOTOR_TASK";

void motorEventHandler(void* handler_arg,
                                esp_event_base_t base,
                                int32_t id,
                                void* event_data){
    if(base == MOTOR_EVENT){
        Motor motor = (Motor)*(Motor*)event_data; //dumb hack investigates
        switch(id){
            case MOTOR_EVENT_FORWARD:
                ESP_LOGI(TAG, "Forward Event Received");
                motor_forward(motor, FORWARD_PIN, FOR_BUTTON,.9);
                break;
            case MOTOR_EVENT_REVERSE:
                ESP_LOGI(TAG, "Reverse Event Received");
                motor_reverse(motor, REVERSE_PIN, REV_BUTTON,.95);
                break;
            case MOTOR_EVENT_STOP:
                ESP_LOGI(TAG, "Stop Event Received");
                motor_stop(motor, REVERSE_PIN, REV_BUTTON);
                break;
            default:
                ESP_LOGI(TAG,"Unknown Motor Event Id: %ld", id);
        }
    }
}
