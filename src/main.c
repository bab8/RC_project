#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_cmpr.h"

#define LED 2
#define FOWARD_DRIVER_PIN 22
#define REVERSE_DRIVER_PIN 23

void motor_driver_timer_init(){
    //MCPWM Timer
    //1000000/20000 = 50Hz
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .intr_priority = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_DOWN,
        .period_ticks = 20000,
        .flags.update_period_on_empty = 1
    };
    

    //Obtain pointer and return

}

motor_driver_operator_init(){
    //MCPWM Operator
    mcpwm_operator_config_t op_config = {
        .group_id = 0,
        .intr_priority = 0,
        .flags.update_gen_action_on_tez = 1
    };

    //Obtain pointer and return

}

motor_driver_comparator_init(){
    //MCPWM Comperator
    mcpwm_comparator_config_t compr_config = {
        .intr_priority = 0,
        .flags.update_cmp_on_tez = 1
    };

    //Obtain pointer and return
    
}

motor_driver_gen_init(int driver_pin){
    //MCPWM Generator
    mcpwm_generator_config_t gen_config;
    gen_config.gen_gpio_num = driver_pin;

    //Obtain pointer and return

}

void motor_reverse(){
    //drive forward pin low, disable gen
    //set duty cycle and enable reverse pin gen

    //disable after period (FOR TESTING MOTOR)
}

void motor_forward(){
    //drive reverse pin low, disable gen
    //set duty cycle and enable forward pin gen

    //disable after period (FOR TESTING MOTOR)
}

void motor_stop(){
    //disbale both gens
}

void app_main() {
    
    // gpio_reset_pin(LED);
    // gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    while(1){

        /* LED blink 
        gpio_set_level(LED, 1);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        gpio_set_level(LED, 0);
        vTaskDelay(5000/portTICK_PERIOD_MS); */

        //Enable Timer

        //Set Duty Cycle
    }
}