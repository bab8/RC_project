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

    Events event;
    BaseType_t eventFlag;
    //Motor event loop
    while(1){
        eventFlag = xQueueReceive(evQueueHandle, &(event), (TickType_t) 10);
        ESP_LOGI(MOTOR_TASK_TAG, "Entered Event loop");
        //check queue for event
        if(eventFlag == pdPASS){
            //process event
            switch (event.sig)
            {
                case FORWARD:
                    ESP_LOGI(MOTOR_TASK_TAG, "Forward Event Received");
                    motor_forward(motor,FORWARD_PIN,FOR_BUTTON,.9);
                    break;
                case REVERSE:
                    ESP_LOGI(MOTOR_TASK_TAG, "Reverse Event Received");
                    motor_reverse(motor,REVERSE_PIN, REV_BUTTON,.8);
                    break;
                default:
                    ESP_LOGI(MOTOR_TASK_TAG, "No Event Received");
                    break;
            }
            vTaskDelay(1000/portTICK_PERIOD_MS);
        } 
        vTaskDelay(1000/portTICK_PERIOD_MS);   
    }  
}
