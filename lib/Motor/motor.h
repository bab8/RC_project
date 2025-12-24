#ifndef MOTOR_H_
#define MOTOR_H_

#include "bsp.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_cmpr.h"

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

void motor_stop(Motor motor, int forward_pin, int reverse_pin);
void motor_reverse(Motor motor, int forward_pin, int forward_button, double percent);
void motor_forward(Motor motor, int reverse_pin, int reverse_button, double percent);
Motor motor_driver_init(int forward_pin, int reverse_pin);

#endif