#include "motor.h"

static const char* TAG = "MOTOR_MODULE";

/*Unused for now*/
void motor_stop(Motor motor, int forward_pin, int reverse_pin){
    ESP_LOGI(TAG, "Motor stopping");
    //disbale both gens
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.for_gen,
    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_STOP_EMPTY));
    
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.rev_gen,
    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_STOP_EMPTY));
}

/*Motor reverse*/
void motor_reverse(Motor motor, int forward_pin, int forward_button, double percent){
    ESP_LOGI(TAG, "Motor reversing");
    //drive forward pin low
    //set duty cycle and enable reverse pin gen
    if(percent < 1.0){
        gpio_set_level(forward_pin, 0);
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.rev_gen,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor.rev_gen, 
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor.rev_cmpr,MCPWM_GEN_ACTION_LOW)));
        
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor.rev_cmpr,(percent*PWM_PERIOD_TICKS)));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_NO_STOP));
    } else{
        //disable gen for forward pin 
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.rev_gen,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_STOP_EMPTY));
    }

}

void motor_forward(Motor motor, int reverse_pin, int reverse_button, double percent){
    ESP_LOGI(TAG, "Motor forward");
    //drive reverse pin low
    //set duty cycle and enable forward pin gen
    if(percent < 1.0){
        gpio_set_level(reverse_pin, 0);
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.for_gen,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motor.for_gen, 
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,motor.for_cmpr,MCPWM_GEN_ACTION_LOW)));
        
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motor.for_cmpr,(percent*PWM_PERIOD_TICKS)));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_NO_STOP));
    } else{
        //disable gen for forward pin 
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motor.for_gen,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(motor.timer,MCPWM_TIMER_START_STOP_EMPTY));
    }

}

Motor motor_driver_init(int forward_pin, int reverse_pin){
    ESP_LOGI(TAG, "Motor init entered");
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
    ESP_LOGI(TAG, "Motor init exit");

    return motor;
}