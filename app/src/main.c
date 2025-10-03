/*
* main.c
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// DT I think means device tree.
// Therefore we are getting the device tree alias for led0
#define LED0_NODE DT_ALIAS(led0)

// Then, with the device tree alias for led0, we use the function
// to get the specs for that alias.
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void) {
    int ret;

    if (!gpio_is_ready_dt(&led0)){
        return -1;;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return ret;
    }
    
    while(1) {
        
    }

    return 0;
}