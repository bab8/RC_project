#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "motor.h"
#include "bsp.h"
#include "motor_task.h"

static TaskHandle_t xMotorHandle = NULL;
static uint8_t ucMotorParameters;

static TaskHandle_t xWifiHandle = NULL;
static uint8_t ucWifiParameters;

evQueueHandle = NULL;

static const char* TAG = "MAIN_MODULE";

void app_main() {
    /*Init GPIO*/
    ESP_LOGI(TAG, "Init GPIO");
    gpio_init();
    ESP_LOGI(TAG, "Init GPIO finished");

    /*Init motor driver*/
    //ESP_LOGI(TAG, "Init Motor");
    //Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);
    //ESP_LOGI(TAG, "Init Motor finished");

    //TODO: create wifi task(should connect to wifi, setup tcp server and listen to update events)
    evQueueHandle = xQueueCreate(5, sizeof(enum events));
    xTaskCreate(wifiTask,"Wifi Task", TASK_STACK_SIZE,&ucWifiParameters,2,xWifiHandle);
    if(xWifiHandle == NULL){
        ESP_LOGI(TAG, "failed to create wifi task");
        vTaskDelete(xWifiHandle);
    }
    //TODO: create motor task(should init motors and run tehm based on events passed to freertos queue by TCP)
    xTaskCreate(motorTask,"Motor Task", TASK_STACK_SIZE,&ucMotorParameters,2,xMotorHandle);
    if(xMotorHandle == NULL){
        ESP_LOGI(TAG, "failed to create motor task");
        vTaskDelete(xMotorHandle);
    }
    //TODO: create idle task(for when connection is dead, investiagte low power mode)
    //TODO: create camera task, will handle camera module

    while(1){}
    /*while(1){
        // Run Motor
        motor_forward(motor,FORWARD_PIN,FOR_BUTTON,.9);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        motor_reverse(motor,REVERSE_PIN, REV_BUTTON,.8);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }*/
    
}


void wifiTask(){
    enum events ev;
    ESP_LOGI(TAG, "Entered Wifi event loop");
    while(1){
        //cycle through events
        ev = FORWARD;
        xQueueSend(evQueueHandle, &ev, 500);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        ev = REVERSE;
        xQueueSend(evQueueHandle, &ev, 500);
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}
