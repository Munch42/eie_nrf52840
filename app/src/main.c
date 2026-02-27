/**
 * @file main.c
 */

// FOR LCD BUILD WITH:
// west build -b nrf52840dk/nrf52840 --shield=adafruit_2_8_tft_touch_v2 app

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>

#include "BTN.h"
#include "LED.h"
#include "lv_data_obj.h"

#define SLEEP_MS 1

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *screen = NULL; 

void lv_button_callback(lv_event_t *event) {
  lv_obj_t *data_obj = (lv_obj_t *)lv_event_get_user_data(event);
  led_id led = *(led_id *)lv_data_obj_get_data_ptr(data_obj);

  LED_toggle(led);
}

static lv_indev_t* touch_indev;
static lv_obj_t* circle = NULL;

int main(void) {
  if (!device_is_ready(display_dev)) {
    return 0;
  }

  screen = lv_screen_active();
  if (screen == NULL) {
    return 0;
  }

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  /*lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Hello World!");

  display_blanking_off(display_dev);*/
  
  /*for (uint8_t i = 0; i < NUM_LEDS; i++) {
    lv_obj_t *ui_btn = lv_button_create(screen);
    // Place the buttons in a 2x2 grid in the center of the screen
    // matching the orientations of the LEDS on the board
    lv_obj_align(ui_btn, LV_ALIGN_CENTER, 50 * (i % 2 ? 1 : -1), 20 * (i < 2 ? -1 : 1));
    lv_obj_t *button_label = lv_label_create(ui_btn);
    char label_text[10];
    snprintf(label_text, 10, "LED %d", i);
    lv_label_set_text(button_label, label_text);
    lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);

    led_id led = (led_id)i;
    lv_obj_t *data_obj = lv_data_obj_create_alloc_assign(ui_btn, &led, sizeof(led_id));
    lv_obj_add_event_cb(ui_btn, lv_button_callback, LV_EVENT_CLICKED, data_obj);
  }*/

  display_blanking_off(display_dev);

  touch_indev = lv_indev_get_next(NULL);

  while (1) {
    lv_timer_handler();

    if (touch_indev) {
        lv_point_t point;
        lv_indev_state_t state = lv_indev_get_state(touch_indev);

        if (state == LV_INDEV_STATE_PRESSED) {
            lv_indev_get_point(touch_indev, &point);

            if (!circle) {
                circle = lv_obj_create(screen);
                lv_obj_set_size(circle, 15, 15);
                lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(circle,
                lv_color_hex(0xFF0000), 0);
            }
            
            lv_obj_set_pos(circle, point.x - 7, point.y - 7);
        } else {
            if (circle) {
                lv_obj_del(circle);
                circle = NULL;
            }
        }
    }

    k_msleep(SLEEP_MS);
  }
  return 0;
}
