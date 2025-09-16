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
#define TIMER_RES_HZ 1000000 // 1 MHz = 1 tick = 1 mircosec
#define PWM_FREQ_HZ 50 // 50 Hz
#define PWM_PERIOD_TICKS (TIMER_RES_HZ/PWM_FREQ_HZ) 

typedef struct {
    mcpwm_timer_handle_t timer;
    mcpwm_oper_handle_t oper;
    mcpwm_cmpr_handle_t for_cmpr;
    mcpwm_cmpr_handle_t rev_cmpr;
    mcpwm_gen_handle_t for_gen;
    mcpwm_gen_handle_t rev_gen;
} Motor;

void motor_driver_timer_init(mcpwm_timer_handle_t timer_handler){
    //MCPWM Timer
    
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .intr_priority = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = TIMER_RES_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = PWM_PERIOD_TICKS //1000000/20000 = 50Hz
    };
    
    //Obtain pointer 
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer_handler));
}

void motor_driver_operator_init(mcpwm_oper_handle_t op_handler){
    //MCPWM Operator
    mcpwm_operator_config_t op_config = {
        .group_id = 0,
        .intr_priority = 0
    };

    //Obtain pointer 
    ESP_ERROR_CHECK(mcpwm_new_operator(&op_config, &op_handler));
}

void motor_driver_comparator_init(mcpwm_cmpr_handle_t cmpr_handle, mcpwm_oper_handle_t op_handler){
    //MCPWM Comperator
    mcpwm_comparator_config_t compr_config = {
        .intr_priority = 0
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

void motor_reverse(Motor *motor){
    //drive forward pin low, disable gen
    //set duty cycle and enable reverse pin gen
    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor->timer,MCPWM_TIMER_START_NO_STOP));
    gpio_set_level(FOWARD_PIN, 0);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor->rev_cmpr,(.5*PWM_PERIOD_TICKS)));
    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor->timer,MCPWM_TIMER_STOP_FULL));
    
}

void motor_forward(Motor *motor){
    //drive reverse pin low, disable gen
    //set duty cycle and enable forward pin gen
    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor->timer,MCPWM_TIMER_START_NO_STOP));
    //gpio_set_level(REVERSE_PIN, 0);
    gpio_set_level(FOWARD_PIN, 1);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor->for_cmpr,(1*PWM_PERIOD_TICKS)));
    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor->timer,MCPWM_TIMER_STOP_FULL));
}

void motor_stop(){
    //disbale both gens
    gpio_set_level(REVERSE_PIN, 1);
    gpio_set_level(FOWARD_PIN, 1);
}

void motor_driver_init(Motor *motor){

    //initialize timer, operator, comparator
    motor_driver_timer_init(motor->timer);
    motor_driver_operator_init(motor->oper);
    motor_driver_comparator_init(motor->for_cmpr, motor->oper);
    motor_driver_comparator_init(motor->rev_cmpr, motor->oper);

    //Connect operator and timer
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(motor->oper, motor->timer));

    //Start timer(try utilizing timer only in start and stop functions for now)
    ESP_ERROR_CHECK(mcpwm_timer_enable(motor->timer));
    
    //configure genrators, initalize and set comparator event actions
    motor_driver_generator_init(REVERSE_PIN, motor->rev_gen, motor->oper);
    motor_driver_generator_init(FOWARD_PIN, motor->for_gen, motor->oper);
    //Reverse comaprator events
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor->for_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor->rev_cmpr,MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor->rev_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor->rev_cmpr,MCPWM_GEN_ACTION_HIGH)));
    
    //Foward comparator events
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor->for_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor->for_cmpr,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor->rev_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor->for_cmpr,MCPWM_GEN_ACTION_LOW)));

    //start timer
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor->timer,MCPWM_TIMER_START_NO_STOP));
}

void app_main() {
    
    gpio_reset_pin(FOWARD_PIN);
    gpio_set_direction(FOWARD_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(REVERSE_PIN);
    gpio_set_direction(REVERSE_PIN, GPIO_MODE_OUTPUT);

    /*Init motor driver*/
    Motor *motor1 = { 0 };
    motor_driver_init(motor1);

    while(1){
        /*Run Motor*/
        //Set direction foward
        motor_forward(motor1);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        motor_reverse(motor1);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        
        
        /* LED blink 
        gpio_set_level(LED, 1);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        gpio_set_level(LED, 0);
        vTaskDelay(5000/portTICK_PERIOD_MS); */

    }
}