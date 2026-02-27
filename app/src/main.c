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

#define CANVAS_WIDTH  240
#define CANVAS_HEIGHT 200

// Canvas buffer the size of the screen
static uint8_t canvas_buf[CANVAS_WIDTH * CANVAS_HEIGHT];

static lv_indev_t * touch_indev;
static lv_point_t last_point;
static bool drawing = false;
static lv_obj_t * canvas;

static void draw_line(int x1, int y1, int x2, int y2)
{
    // Create a temporary "layer" to draw on the canvas (LVGL v9 style)
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    // Configure the line settings
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    
    dsc.p1.x = x1;
    dsc.p1.y = y1;
    dsc.p2.x = x2;
    dsc.p2.y = y2;
    
    dsc.color = lv_color_hex(0x000000); // Black line
    dsc.width = 3;

    // Perform the draw operation onto the layer
    lv_draw_line(&layer, &dsc);

    // Apply the layer changes back to the canvas buffer
    lv_canvas_finish_layer(canvas, &layer);
}

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

  canvas = lv_canvas_create(screen);
  lv_canvas_set_buffer(canvas,
                      canvas_buf,
                      CANVAS_WIDTH,
                      CANVAS_HEIGHT,
                      LV_COLOR_FORMAT_L8);

  lv_obj_center(canvas);

  lv_canvas_fill_bg(canvas,
                  lv_color_hex(0xFFFFFF),
                  LV_OPA_COVER);

  display_blanking_off(display_dev);

  touch_indev = lv_indev_get_next(NULL);

  while (1) {
    lv_timer_handler();

    if (touch_indev) {
        lv_point_t point;
        lv_indev_state_t state =
            lv_indev_get_state(touch_indev);

        lv_indev_get_point(touch_indev, &point);

        if (state == LV_INDEV_STATE_PRESSED) {

            if (!drawing) {
                drawing = true;
                last_point = point;
            } else {
                draw_line(last_point.x,
                          last_point.y,
                          point.x,
                          point.y);

                last_point = point;
            }
        } else {
            drawing = false;
        }
    }

    k_msleep(SLEEP_MS);
  }
  return 0;
}
