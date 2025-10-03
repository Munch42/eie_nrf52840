/*
* main.c
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// DT I think means device tree.
// Therefore we are getting the device tree alias for led0
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

// Then, with the device tree alias for led0, we use the function
// to get the specs for that alias.
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

int main(void) {
    int ret0;
    int ret1;
    int ret2;
    int ret3;

    // I think if it is not ready, it returns from main
    // which in this context just restarts the board
    if (!gpio_is_ready_dt(&led0) && !gpio_is_ready_dt(&led1)
         && !gpio_is_ready_dt(&led2) && !gpio_is_ready_dt(&led3)){
        return -1;;
    }

    // I think this function returns a status code
    // If it is 0 it is good probably
    // Then negative numbers for errors. In which case we
    // return that error code which restarts main/the board
    ret0 = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret0 < 0) {
        return ret0;
    }
    ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret1 < 0) {return ret1;}
    ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    if (ret2 < 0) {return ret2;}
    ret3 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
    if (ret3 < 0) {return ret3;}
    
    while(1) {
        gpio_pin_toggle_dt(&led0); // Toggle the led0
        k_msleep(500);
        gpio_pin_toggle_dt(&led0); // Toggle the led0 again to be twice as fast
        gpio_pin_toggle_dt(&led1); // Toggle the led1
        gpio_pin_toggle_dt(&led2); // Toggle the led2
        gpio_pin_toggle_dt(&led3); // Toggle the led3

        k_msleep(500);
    }

    return 0;
}