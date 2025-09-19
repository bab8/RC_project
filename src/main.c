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

#define FORWARD_PIN 22
#define REVERSE_PIN 23
#define TIMER_RES_HZ 1000000 // 1 MHz = 1 tick = 1 mircosec
#define PWM_PERIOD_TICKS 20000

typedef struct {
    mcpwm_timer_handle_t timer;
    mcpwm_oper_handle_t oper;
    mcpwm_cmpr_handle_t for_cmpr;
    mcpwm_cmpr_handle_t rev_cmpr;
    mcpwm_gen_handle_t for_gen;
    mcpwm_gen_handle_t rev_gen;
    mcpwm_timer_config_t timer_config;
    mcpwm_operator_config_t op_config;
    mcpwm_comparator_config_t compr_config;
    mcpwm_generator_config_t rev_gen_config;
    mcpwm_generator_config_t for_gen_config;
} Motor;


void motor_reverse(Motor motor){
    //drive forward pin low, disable gen
    //set duty cycle and enable reverse pin gen
    gpio_set_level(FORWARD_PIN, 0);
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.rev_gen,
    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor.rev_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor.rev_cmpr,MCPWM_GEN_ACTION_LOW)));
    
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor.rev_cmpr,(.75*PWM_PERIOD_TICKS)));

    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_NO_STOP));

    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_STOP_EMPTY));
}

void motor_forward(Motor motor){
    //drive reverse pin low, disable gen
    //set duty cycle and enable forward pin gen
    gpio_set_level(REVERSE_PIN, 0);
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.for_gen,
    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor.for_gen, 
    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor.for_cmpr,MCPWM_GEN_ACTION_LOW)));
    
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor.for_cmpr,(.75*PWM_PERIOD_TICKS)));

    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_NO_STOP));
    //ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_STOP_FULL));
}

void motor_stop(){
    //disbale both gens
    gpio_set_level(REVERSE_PIN, 1);
    gpio_set_level(FORWARD_PIN, 1);
}

Motor motor_driver_init(int forward_pin, int reverse_pin){
    Motor motor = { 0 };
    //initialize timer
    motor.timer_config.group_id = 0;
    motor.timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    motor.timer_config.resolution_hz = TIMER_RES_HZ;
    motor.timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
    motor.timer_config.period_ticks = PWM_PERIOD_TICKS; //1000000/20000 = 50Hz
    motor.timer_config.flags.update_period_on_empty = 1;
    
    //create timer
    ESP_ERROR_CHECK(mcpwm_new_timer(&motor.timer_config, &motor.timer));

    //initialize operator
    motor.op_config.group_id = 0;

    //create operator
    ESP_ERROR_CHECK(mcpwm_new_operator(&motor.op_config, &motor.oper));

    //Connect operator and timer
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(motor.oper, motor.timer));

    //initialize comparator
    motor.compr_config.flags.update_cmp_on_tez = 1;

    //create comparators
    ESP_ERROR_CHECK(mcpwm_new_comparator(motor.oper, &motor.compr_config, &motor.for_cmpr));
    ESP_ERROR_CHECK(mcpwm_new_comparator(motor.oper, &motor.compr_config, &motor.rev_cmpr));
    
    //configure genrators
    motor.for_gen_config.gen_gpio_num = forward_pin;
    motor.rev_gen_config.gen_gpio_num = reverse_pin;

    //creat generators
    ESP_ERROR_CHECK(mcpwm_new_generator(motor.oper, &motor.for_gen_config, &motor.for_gen));
    ESP_ERROR_CHECK(mcpwm_new_generator(motor.oper, &motor.rev_gen_config, &motor.rev_gen));

    //start timer
    ESP_ERROR_CHECK(mcpwm_timer_enable(motor.timer));

    return motor;
}

void app_main() {

    gpio_reset_pin(FORWARD_PIN);
    gpio_set_direction(FORWARD_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(REVERSE_PIN);
    gpio_set_direction(REVERSE_PIN, GPIO_MODE_OUTPUT);

    /*Init motor driver*/
    
    Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);

    while(1){
        /*Run Motor*/
        //Set direction R
        motor_forward(motor);
        //vTaskDelay(5000/portTICK_PERIOD_MS);
        
        //motor_reverse(motor);
        //ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor1.rev_cmpr,(.75*PWM_PERIOD_TICKS)));

    }
}