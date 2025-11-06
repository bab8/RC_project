#include "motor_task.h"
#include "motor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char* MOTOR_TASK_TAG = "MOTOR_TASK";

void motorTask(){
    ESP_LOGI(MOTOR_TASK_TAG, "Init Motor");
    Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);
    ESP_LOGI(MOTOR_TASK_TAG, "Init Motor finished");

    enum events event;
    
    //Motor event loop
    ESP_LOGI(MOTOR_TASK_TAG, "Entered Event loop");
    while(1){
        //check queue for event
        if(xQueueReceive(evQueueHandle, &event, 500)){
            //process event
            switch (event)
            {
            case FORWARD:
                motor_forward(motor,FORWARD_PIN,FOR_BUTTON,.9);
                break;
            case REVERSE:
                motor_reverse(motor,REVERSE_PIN, REV_BUTTON,.8);
                break;
            default:
                break;
            }
        }  
    }
}