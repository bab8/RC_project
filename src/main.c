#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "motor.h"
#include "bsp.h"
#include "motor_task.h"

#include "TCPwifi.h"
#include "secrets.h"

static uint8_t sRetryNum = 0;

static TaskHandle_t xMotorHandle = NULL;
static uint8_t ucMotorParameters;

static TaskHandle_t xWifiHandle = NULL;
static uint8_t ucWifiParameters;

static EventGroupHandle_t wifiEventGroup;

QueueHandle_t evQueueHandle = NULL;

static const char* TAG = "MAIN_MODULE";

void wifiTask();

static void wifiEventHandler(void* arg, esp_event_base_t eventBase, 
                            int32_t eventId, void* eventData){
    if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START){
        ESP_LOGI(TAG, "Connecting");
        esp_wifi_connect();
    } else if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED){
        if(sRetryNum < MAX_FAILURES){
            ESP_LOGI(TAG, "Reconnecting");
            esp_wifi_connect();
            sRetryNum++;
        } else{
            xEventGroupSetBits(wifiEventGroup, WIFI_FAILURE);
        }
    }
}

static void ipEventHandler(void* arg, esp_event_base_t eventBase,
                            int32_t eventId, void* eventData){
    if(eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        sRetryNum = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_SUCCESS);
    }
}

esp_err_t connectWifi(){
    int status = WIFI_FAILURE;

    //init esp network
    ESP_ERROR_CHECK(esp_netif_init());

    //init default esp 
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //create wifi station in the driver
    esp_netif_create_default_wifi_sta();

    //init wifi config
    wifi_init_config_t wConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wConfig));

    //Event loop
    wifiEventGroup = xEventGroupCreate();

    //Wifi event handlersS
    esp_event_handler_instance_t wifiHandleEventInstance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifiEventHandler,
                                                        NULL,
                                                        &wifiHandleEventInstance));

    esp_event_handler_instance_t ipHandleEventInstance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ipEventHandler,
                                                        NULL,
                                                        &ipHandleEventInstance));

    //Config wifi (define this elsewhere to hide)
    wifi_config_t wifiConfig = {
        .sta = {
            .ssid = {SSID},  //ssid and password should be strings
            .password = {PASSWORD},
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, //wifi auth
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    //set wifi mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    //attach wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));

    //start wifi
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wifi init complete");

    //Wait for event flags
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                            WIFI_SUCCESS | WIFI_FAILURE,
                                            pdFALSE,
                                            pdFALSE,
                                            portMAX_DELAY);

    //Check event
    if(bits & WIFI_SUCCESS){
        ESP_LOGI(TAG,"Connected to Wifi");
        status = WIFI_SUCCESS;
    } else if(bits & WIFI_FAILURE){
        ESP_LOGI(TAG,"Failed to connect");
        status = WIFI_FAILURE;
    } else{
        ESP_LOGI(TAG,"UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    //unregister events once connected
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ipHandleEventInstance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiHandleEventInstance));
    vEventGroupDelete(wifiEventGroup);

    return status;
}

esp_err_t startTCPClient(int *socket_ptr){
    //setup TCP client
    struct sockaddr_in serverInfo = {0};

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = IPADDR;
    serverInfo.sin_port = htons(PORT);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        ESP_LOGE(TAG, "Failed to init socket");
        return TCP_FAILURE;
    }
    *socket_ptr = sock;

    if(connect(sock, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) != 0){
        ESP_LOGE(TAG, "Failed to connect to %s", inet_ntoa(serverInfo.sin_addr.s_addr));
        close(sock);
        return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "Connected to TCP server");

    return TCP_SUCCESS;
}

void wifi(){
    esp_err_t status = WIFI_FAILURE; 

    //init nvs
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    status = connectWifi();
    if(WIFI_SUCCESS != status){
        ESP_LOGI(TAG, "Failed to connect to wifi, terminating");
        return;
    }

   
    int socket;
    status = startTCPClient(&socket);
    if(TCP_SUCCESS != status){
        ESP_LOGI(TAG, "Failed to connect to TCP server");
        return;
    }

    //when connected read from buffer
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
                ESP_LOGI(TAG, "FORWARD EVENT");
            }
        }
    }
}

void app_main() {
    /*Init GPIO*/
    ESP_LOGI(TAG, "Init GPIO");
    gpio_init();
    ESP_LOGI(TAG, "Init GPIO finished");

    wifi();

    /*Init motor driver*/
    //ESP_LOGI(TAG, "Init Motor");
    //Motor motor = motor_driver_init(FORWARD_PIN,REVERSE_PIN);
    //ESP_LOGI(TAG, "Init Motor finished");

    //TODO: create wifi task(should connect to wifi, setup tcp server and listen to update events)
    /* evQueueHandle = xQueueCreate(3, sizeof( Events ));
    if(evQueueHandle == NULL){
        ESP_LOGI(TAG, "failed to create queue");
    }
    xTaskCreate(wifiTask,"Wifi Task", TASK_STACK_SIZE,&ucWifiParameters,2,&xWifiHandle);
    if(xWifiHandle == NULL){
        ESP_LOGI(TAG, "failed to create wifi task");
        vTaskDelete(xWifiHandle);
    }
    //TODO: create motor task(should init motors and run tehm based on events passed to freertos queue by TCP)
    xTaskCreate(motorTask,"Motor Task", TASK_STACK_SIZE,&ucMotorParameters,2,&xMotorHandle);
    if(xMotorHandle == NULL){
        ESP_LOGI(TAG, "failed to create motor task");
        vTaskDelete(xMotorHandle);
    } */

    //TODO: Port wifi code to wifiTask
    //TODO: cleanup files
    //TODO: change motor init to just a function, passs motor like socket
    //TODO: port motor loop to a handler, place in wifi task
    //TODO: create camera task
    //TODO: create idle task(for when connection is dead, investiagte low power mode)


    while(1){vTaskDelay(2000/portTICK_PERIOD_MS);}
    /*while(1){
        // Run Motor
        motor_forward(motor,FORWARD_PIN,FOR_BUTTON,.9);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        motor_reverse(motor,REVERSE_PIN, REV_BUTTON,.8);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }*/
    
}


void wifiTask(){
    Events ev;
    ESP_LOGI(TAG, "Entered Wifi event loop");
    while(1){
        //cycle through events
        ESP_LOGI(TAG, "Send Forward Event");
        ev.sig = FORWARD;
        xQueueSend(evQueueHandle,  &ev, (TickType_t) 0);
        vTaskDelay(2000/portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Send Reverse Event");
        ev.sig = REVERSE;
        xQueueSend(evQueueHandle, &ev, (TickType_t) 0);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}
