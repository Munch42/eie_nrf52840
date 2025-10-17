/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS        1

void set_counter_leds(uint8_t counter);

//#define SW0_NODE        DT_ALIAS(sw0)
//static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

int main(void) {
  // In serial monitor. If you don't see anything try the other COM port.
  if (0 > BTN_init()) {
    return 0;
  }

  if (0 > LED_init()) {
    return 0;
  }

  uint8_t counter = 0;

  while (1) {
    if (BTN_check_clear_pressed(BTN0)) {
      counter++;
      set_counter_leds(counter);
    }
    k_msleep(SLEEP_MS);
  }

  return 0;
}

void set_counter_leds(uint8_t counter) {
  bool led4_MSB = (counter >> 3) & 1;
  bool led3 = (counter >> 2) & 1;
  bool led2 = (counter >> 1) & 1;
  bool led1_LSB = (counter >> 0) & 1;

  LED_set(LED3, led4_MSB);
  LED_set(LED2, led3);
  LED_set(LED1, led2);
  LED_set(LED0, led1_LSB);
}
