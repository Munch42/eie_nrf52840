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
#define DOT_CLICKABLE_BUFFER 10
#define MIN_DOT_SIZE 5

#define BG_RED_CHANNEL 191
#define BG_GREEN_CHANNEL 33
#define BG_BLUE_CHANNEL 186
#define BG_DIFF_THRESHOLD 50

#define MAX_DOT_SCORE 300
#define TIMER_PERIOD 250

static lv_timer_t * shrink_timer;
static uint32_t current_timer_period = TIMER_PERIOD;

static lv_obj_t * score_label;
static int score = 0;
static int current_thousand_barrier = 0;
static int nextScore = MAX_DOT_SCORE;
static lv_obj_t * game_over_label;

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *screen = NULL; 

void lv_button_callback(lv_event_t *event) {
  lv_obj_t *data_obj = (lv_obj_t *)lv_event_get_user_data(event);
  led_id led = *(led_id *)lv_data_obj_get_data_ptr(data_obj);

  LED_toggle(led);
}

// The event to call when the dot has been clicked
static void dot_event_cb(lv_event_t * e) {
    lv_obj_t * target = lv_event_get_target(e); // The dot that was clicked

    if (!lv_obj_has_flag(game_over_label, LV_OBJ_FLAG_HIDDEN)) {
        score = 0;
        current_timer_period = TIMER_PERIOD;
        current_thousand_barrier = 0;
        
        lv_timer_set_period(shrink_timer, TIMER_PERIOD);
        lv_timer_resume(shrink_timer);
        lv_obj_add_flag(game_over_label, LV_OBJ_FLAG_HIDDEN);
    }

    // Get screen dimensions
    int32_t screen_w = lv_display_get_horizontal_resolution(NULL);
    int32_t screen_h = lv_display_get_vertical_resolution(NULL);

    // Since the dot size is reset to max when it moves, just use DOT_SIZE for it instead of 
    // int32_t dot_size = lv_obj_get_width(target);
    // Calculate new random position
    int rand_x = rand() % (screen_w - DOT_SIZE);
    int rand_y = rand() % (screen_h - DOT_SIZE);

    lv_obj_set_size(target, DOT_SIZE, DOT_SIZE);

    // Move and change the colour
    lv_obj_set_pos(target, rand_x, rand_y);

    uint8_t red_val, green_val, blue_val;
    uint8_t red_diff, green_diff, blue_diff;
    do {
      red_val = rand() % 256;
      green_val = rand() % 256;
      blue_val = rand() % 256;

      red_diff = abs(BG_RED_CHANNEL - red_val);
      green_diff = abs(BG_GREEN_CHANNEL - green_val);
      blue_diff = abs(BG_BLUE_CHANNEL- blue_val);
    } while ((red_diff + green_diff + blue_diff) < BG_DIFF_THRESHOLD);

    score += nextScore;
    nextScore = MAX_DOT_SCORE;
  
    lv_obj_set_style_bg_color(target, lv_color_make(red_val, green_val, blue_val), 0);
    lv_label_set_text_fmt(score_label, "Score: %d", score);
}

static void shrink_timer_cb(lv_timer_t * timer) {
    lv_obj_t * obj = (lv_obj_t *)lv_timer_get_user_data(timer);
  
    int32_t curr_size = lv_obj_get_width(obj);

    if (curr_size > MIN_DOT_SIZE) {
        int32_t new_size = curr_size - 1;
        lv_obj_set_size(obj, new_size, new_size);
        nextScore -= MAX_DOT_SCORE / (DOT_SIZE - MIN_DOT_SIZE);
    } else {
      lv_obj_set_style_bg_color(obj, lv_color_hex(0x555555), 0); // Turn grey if "dead"

      // Show the game over message
      lv_obj_remove_flag(game_over_label, LV_OBJ_FLAG_HIDDEN);
        
      // Pause the timer so it stops running the callback
      lv_timer_pause(timer);
    }

    if (score >= (current_thousand_barrier + 1000) && current_timer_period > 50) {
      current_thousand_barrier += 1000;
      current_timer_period -= 25;
      lv_timer_set_period(timer, current_timer_period);
    }
}

//static lv_indev_t * touch_indev;

static lv_obj_t* dot;
//static bool pressed = false; 

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

  game_over_label = lv_label_create(screen);
  lv_label_set_text(game_over_label, "GAME OVER\nClick the grey dot to restart!");
  lv_obj_set_style_text_align(game_over_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(game_over_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(game_over_label, &lv_font_montserrat_14, 0); // Slightly larger font

  // Place it exactly in the center
  lv_obj_center(game_over_label);

  // Hide it until death
  lv_obj_add_flag(game_over_label, LV_OBJ_FLAG_HIDDEN);

  // Set the screen background colour
  lv_obj_set_style_bg_color(screen, lv_color_make(BG_RED_CHANNEL, BG_GREEN_CHANNEL, BG_BLUE_CHANNEL), 0);

  // Create a label on the active screen
  score_label = lv_label_create(screen);

  // Set the initial text
  lv_label_set_text(score_label, "Score: 0");

  // Style it so it's readable against your purple background
  lv_obj_set_style_text_color(score_label, lv_color_hex(0xFFFFFF), 0); // White text
  lv_obj_set_style_text_font(score_label, &lv_font_montserrat_14, 0);   // Standard font

  // Align it to the top center with a 10px margin from the top
  lv_obj_align(score_label, LV_ALIGN_TOP_MID, 0, 10);

  // Create the dot object
  dot = lv_obj_create(screen);

  // Create the timer to run every 250ms
  shrink_timer = lv_timer_create(shrink_timer_cb, TIMER_PERIOD, dot);

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
  lv_obj_set_pos(dot, 30, 60);

  // Extend the area around the dot that is clickable to add a buffer for game smoothness
  // This allows the native event callback functionality to see that there is a "buffer" around it
  lv_obj_set_ext_click_area(dot, DOT_CLICKABLE_BUFFER);

  // Add the clicked callback
  lv_obj_add_event_cb(dot, dot_event_cb, LV_EVENT_CLICKED, NULL);

  display_blanking_off(display_dev);

  //touch_indev = lv_indev_get_next(NULL);

  while (1) {
    lv_timer_handler();

    /*if (touch_indev) {
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
    }*/

    k_msleep(SLEEP_MS);
  }
  return 0;
}
