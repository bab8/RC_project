#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "motor.h"
#include "bsp.h"

void app_main() {
    /*Init GPIO*/
    gpio_init();

    /*Init motor driver*/
    Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);

    while(1){
        /*Run Motor*/
        motor_forward(motor,FORWARD_PIN,FOR_BUTTON,.9);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        motor_reverse(motor,REVERSE_PIN, REV_BUTTON,.8);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}