#include "bsp.h"

void gpio_init(){
    gpio_reset_pin(FORWARD_PIN);
    gpio_set_direction(FORWARD_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(REVERSE_PIN);
    gpio_set_direction(REVERSE_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(FOR_BUTTON);
    gpio_set_direction(FOR_BUTTON, GPIO_MODE_INPUT);
    gpio_pulldown_en(FOR_BUTTON);

    gpio_reset_pin(REV_BUTTON);
    gpio_set_direction(REV_BUTTON, GPIO_MODE_INPUT);
    gpio_pulldown_en(REV_BUTTON);
}
