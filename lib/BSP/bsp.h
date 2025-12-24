#ifndef BSP_H_
#define BSP_H

#include "stdio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define FORWARD_PIN 22
#define REVERSE_PIN 23
#define REV_BUTTON 18
#define FOR_BUTTON 19

void gpio_init();

#endif