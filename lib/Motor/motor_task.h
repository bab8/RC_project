#ifndef _MOTOR_TASK_H_

#define TASK_STACK_SIZE 1024
#define TASK_SWAP_DELAY_MS 500

enum events {
    STOP,
    FORWARD,
    REVERSE
};

extern QueueHandle_t evQueueHandle;

void motorTask();

#endif