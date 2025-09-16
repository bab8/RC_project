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
#define FOWARD_PIN 22
#define REVERSE_PIN 23

typedef struct {
    mcpwm_timer_handle_t timer;
    mcpwm_oper_handle_t oper;
    mcpwm_cmpr_handle_t cmpr;
    mcpwm_gen_handle_t for_gen;
    mcpwm_gen_handle_t rev_gen;
} Motor;

void motor_driver_timer_init(mcpwm_timer_handle_t timer_handler){
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
    
    //Obtain pointer 
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer_handler));
}

void motor_driver_operator_init(mcpwm_oper_handle_t op_handler){
    //MCPWM Operator
    mcpwm_operator_config_t op_config = {
        .group_id = 0,
        .intr_priority = 0,
        .flags.update_gen_action_on_tez = 1
    };

    //Obtain pointer 
    ESP_ERROR_CHECK(mcpwm_new_operator(&op_config, &op_handler));
}

void motor_driver_comparator_init(mcpwm_cmpr_handle_t cmpr_handle, mcpwm_oper_handle_t op_handler){
    //MCPWM Comperator
    mcpwm_comparator_config_t compr_config = {
        .intr_priority = 0,
        .flags.update_cmp_on_tez = 1
    };

    //Obtain pointer 
    ESP_ERROR_CHECK(mcpwm_new_comparator(op_handler, &compr_config, &cmpr_handle));
}

void motor_driver_generator_init(int driver_pin, mcpwm_gen_handle_t gen_handle, mcpwm_oper_handle_t op_handler){
    //MCPWM Generator
    mcpwm_generator_config_t gen_config;
    gen_config.gen_gpio_num = driver_pin;

    //Obtain pointer
    ESP_ERROR_CHECK(mcpwm_new_generator(op_handler, &gen_config, &gen_handle));
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

void motor_driver_init(){
    Motor motor1;
    //initialize timer, operator, comparator
    motor_driver_timer_init(motor1.timer);
    motor_driver_operator_init(motor1.oper);
    motor_driver_comparator_init(motor1.cmpr, motor1.oper);

    //Connect operator and timer
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(motor1.oper, motor1.timer));

    //Set comparator events
    

    //Start timer(try utilizing timer only in start and stop functions for now)
    //ESP_ERROR_CHECK(mcpwm_timer_enable(motor1.timer));
    
    //configure genrators, initalize and set timer event actions
    motor_driver_generator_init(REVERSE_PIN, motor1.rev_gen, motor1.oper);
    

    motor_driver_generator_init(FOWARD_PIN, motor1.for_gen, motor1.oper);
   ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor1.for_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor1.cmpr,MCPWM_GEN_ACTION_HIGH)))
}

void app_main() {
    
    // gpio_reset_pin(LED);
    // gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    /*Init motor driver*/

    /*Run Motor*/
    //Set direction

    while(1){

        /* LED blink 
        gpio_set_level(LED, 1);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        gpio_set_level(LED, 0);
        vTaskDelay(5000/portTICK_PERIOD_MS); */

    }
}