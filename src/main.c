#include "stdio.h"

#include "motor.h"
#include "bsp.h"
#include "motor_task.h"

#include "TCPwifi.h"
#include "secrets.h"

ESP_EVENT_DEFINE_BASE(MOTOR_EVENT);

static const char* TAG = "MAIN_MODULE";

void app_main() {
    /*Init GPIO*/
    ESP_LOGI(TAG, "Init GPIO");
    gpio_init();
    ESP_LOGI(TAG, "Init GPIO finished");

    //motor init
    Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);

    //wifi init
    int socket = wifi();

    //motor event loop init
    static esp_event_loop_args_t loop_args = {
        .queue_size = 3,
        .task_name = "MOTOR TASK",
        .task_priority = 1,
        .task_stack_size = TASK_STACK_SIZE,
        .task_core_id = 1,
    };

    static esp_event_loop_handle_t motorHandle;

    esp_event_loop_create(&loop_args, &motorHandle);

    ESP_ERROR_CHECK(esp_event_handler_register_with(motorHandle, MOTOR_EVENT, ESP_EVENT_ANY_ID,
                                                &motorEventHandler, NULL));

    //when connected to wifi read from buffer
    char readBuffer[1024] = {0};
    bzero(readBuffer, sizeof(readBuffer));
    while(1){
        int r = recv(socket, readBuffer, sizeof(readBuffer) - 1, 0);
        if(r < 0){
            ESP_LOGE(TAG, "recv failed");
            break;
        } else{
            //if buffer contains valid command post event
            if(strcmp(readBuffer, "FORWARD") == 0){
                esp_event_post_to(motorHandle, MOTOR_EVENT, MOTOR_EVENT_FORWARD,
                                &motor, sizeof(motor), portMAX_DELAY);
                ESP_LOGI(TAG, "FORWARD EVENT");
                bzero(readBuffer, sizeof(readBuffer));
            }
            if(strcmp(readBuffer, "REVERSE") == 0){
                esp_event_post_to(motorHandle, MOTOR_EVENT, MOTOR_EVENT_REVERSE,
                                &motor, sizeof(motor), portMAX_DELAY);
                ESP_LOGI(TAG, "REVERSE EVENT");
                bzero(readBuffer, sizeof(readBuffer));
            }
            if(strcmp(readBuffer, "STOP") == 0){
                esp_event_post_to(motorHandle, MOTOR_EVENT, MOTOR_EVENT_STOP,
                                &motor, sizeof(motor), portMAX_DELAY);
                ESP_LOGI(TAG, "STOP EVENT");
                bzero(readBuffer, sizeof(readBuffer));
            }
        }
    }

    //TODO: create camera task
    //TODO: create idle task(for when connection is dead, investiagte low power mode)
 
}

