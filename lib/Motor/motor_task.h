#ifndef _MOTOR_TASK_H_

#define TASK_STACK_SIZE 4096
#define TASK_SWAP_DELAY_MS 500

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "bsp.h"

typedef enum {
    MOTOR_EVENT_FORWARD,
    MOTOR_EVENT_REVERSE,
    MOTOR_EVENT_STOP
} motor_event_id_t;

void motorTask();

void motorEventHandler(void* handler_arg,
                                esp_event_base_t base,
                                int32_t id,
                                void* event_data);

#endif