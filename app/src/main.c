/**
 * @file main.c
 */

// FOR LCD BUILD WITH:
// west build -b nrf52840dk/nrf52840 --shield=adafruit_2_8_tft_touch_v2 app

#include <inttypes.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>

#include "BTN.h"
#include "LED.h"
#include "lv_data_obj.h"

#define SLEEP_MS 1
#define DOT_SIZE 30

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *screen = NULL; 

void lv_button_callback(lv_event_t *event) {
  lv_obj_t *data_obj = (lv_obj_t *)lv_event_get_user_data(event);
  led_id led = *(led_id *)lv_data_obj_get_data_ptr(data_obj);

  LED_toggle(led);
}

static lv_indev_t * touch_indev;

static lv_obj_t* dot;
static bool pressed = false; 

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

  // Create the dot object
  dot = lv_obj_create(screen);

  // Make it a circle
  lv_obj_set_size(dot, DOT_SIZE, DOT_SIZE);
  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);

  lv_obj_set_style_pad_all(dot, 0, 0);
  lv_obj_set_style_border_width(dot, 0, 0);
  lv_obj_set_style_outline_width(dot, 0, 0);

  // Style it red initially
  lv_obj_set_style_bg_color(dot, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);

  // Put it at a default position
  lv_obj_set_pos(dot, 0, 0);

  display_blanking_off(display_dev);

  touch_indev = lv_indev_get_next(NULL);

  while (1) {
    lv_timer_handler();

    if (touch_indev) {
        lv_indev_state_t state =
            lv_indev_get_state(touch_indev);
        lv_point_t touch_point;
        lv_indev_get_point(touch_indev, &touch_point); // Get the coordinates of where they pressed in an object.

        if (state == LV_INDEV_STATE_PRESSED && !pressed) {
          // Current coords:
          int32_t dot_x = lv_obj_get_x(dot);
          int32_t dot_y = lv_obj_get_y(dot);

          int buffer = 10;
          if (touch_point.x >= (dot_x - buffer) && touch_point.x <= (dot_x + DOT_SIZE + buffer) &&
              touch_point.y >= (dot_y - buffer) && touch_point.y <= (dot_y + DOT_SIZE + buffer)) {
            // Then the dot has been pressed within the buffer so count it as a hit.
            pressed = true;

            int32_t screen_w = lv_display_get_horizontal_resolution(NULL);
            int32_t screen_h = lv_display_get_vertical_resolution(NULL);

            // Generate coordinates safely
            int rand_x = rand() % (screen_w - DOT_SIZE);
            int rand_y = rand() % (screen_h - DOT_SIZE);

            // Change the colour randomly
            lv_color_t random_color = lv_color_make(rand() % 256, rand() % 256, rand() % 256);
            lv_obj_set_style_bg_color(dot, random_color, 0);

            //printk("Screen: %dx%d | New Dot X:%d, Y:%d\n", screen_w, screen_h, rand_x, rand_y);
            
            // Move the object
            lv_obj_set_pos(dot, rand_x, rand_y);
          }
        } else if (state == LV_INDEV_STATE_RELEASED) {
          pressed = false;
        }
    }

    k_msleep(SLEEP_MS);
  }
  return 0;
}
