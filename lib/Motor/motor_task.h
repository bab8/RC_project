#ifndef _MOTOR_TASK_H_

#define TASK_STACK_SIZE 4096
#define TASK_SWAP_DELAY_MS 500

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef enum { STOP, FORWARD, REVERSE } EVENT;

typedef struct {
    uint16_t sig;
} Events;

extern QueueHandle_t evQueueHandle;

void motorTask();

#endif