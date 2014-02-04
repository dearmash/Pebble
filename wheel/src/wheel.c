#include <pebble.h>

#include "bitmap.h"

#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84

typedef enum { DIGIT_NORMAL, DIGIT_BIG, DIGIT_SMALL } digit_size_t;

#define NUM_DIGITS 10
#define NUM_IMAGES 30
const int digitId[NUM_IMAGES] = {
  // 20 x 20
  RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2, RESOURCE_ID_IMAGE_3,
  RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5, RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7,
  RESOURCE_ID_IMAGE_8, RESOURCE_ID_IMAGE_9,
  // 30 x 30
  RESOURCE_ID_IMAGE_0_BIG, RESOURCE_ID_IMAGE_1_BIG, RESOURCE_ID_IMAGE_2_BIG, RESOURCE_ID_IMAGE_3_BIG,
  RESOURCE_ID_IMAGE_4_BIG, RESOURCE_ID_IMAGE_5_BIG, RESOURCE_ID_IMAGE_6_BIG, RESOURCE_ID_IMAGE_7_BIG,
  RESOURCE_ID_IMAGE_8_BIG, RESOURCE_ID_IMAGE_9_BIG,
  // 15 x 15
  RESOURCE_ID_IMAGE_0_SMALL, RESOURCE_ID_IMAGE_1_SMALL, RESOURCE_ID_IMAGE_2_SMALL, RESOURCE_ID_IMAGE_3_SMALL,
  RESOURCE_ID_IMAGE_4_SMALL, RESOURCE_ID_IMAGE_5_SMALL, RESOURCE_ID_IMAGE_6_SMALL, RESOURCE_ID_IMAGE_7_SMALL,
  RESOURCE_ID_IMAGE_8_SMALL, RESOURCE_ID_IMAGE_9_SMALL
};

GBitmap *digitBmp[NUM_IMAGES];
GBitmap *backgroundBmp;
GBitmap *hoursBmp;
GBitmap *minutesBmp;
GBitmap *foregroundBmp;
GBitmap *dateBmp;

static Window *window;
static BitmapLayer *background;
static BitmapLayer *hours;
static BitmapLayer *minutes;
static BitmapLayer *foreground;
static BitmapLayer *date;

static int number = 0;

static void wheel_draw(GBitmap *dest, int number, int angle, int angleSpacing, digit_size_t size) {



}

static int add_char_to_buffer(GBitmap *dst, GBitmap *src, int offset, int padding) {

  GRect from = src->bounds;
  // TODO: bounds checking
  GPoint to = GPoint(offset, dst->bounds.size.h/2 - src->bounds.size.h/2);

  bmpSub(src, dst, from, to);

  return offset + src->bounds.size.w + padding;
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

  bmpFill(hoursBmp, GColorBlack);
  layer_mark_dirty((Layer*)hours);

}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {

  gbitmap_destroy(hoursBmp);
  hoursBmp = gbitmap_create_with_resource(RESOURCE_ID_HOURS);
  bitmap_layer_set_bitmap(hours, hoursBmp);
  layer_mark_dirty((Layer*)hours);

  number = (number+1)%NUM_IMAGES;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  number = (number-1+NUM_IMAGES)%NUM_IMAGES;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  background = bitmap_layer_create(GRect(0, 0, SCREENW, SCREENH));
  bitmap_layer_set_bitmap(background, backgroundBmp);
  layer_add_child(window_layer, (Layer*)background);

  hours = bitmap_layer_create(GRect(0, 0, SCREENW, SCREENH));
  bitmap_layer_set_bitmap(hours, hoursBmp);
  bitmap_layer_set_compositing_mode(hours, GCompOpOr);
  layer_add_child(window_layer, (Layer*)hours);

  minutes = bitmap_layer_create(GRect(0, 0, SCREENW, SCREENH));
  bitmap_layer_set_bitmap(minutes, minutesBmp);
  bitmap_layer_set_compositing_mode(minutes, GCompOpOr);
  layer_add_child(window_layer, (Layer*)minutes);

  foreground = bitmap_layer_create(GRect(0, 0, SCREENW, SCREENH));
  bitmap_layer_set_bitmap(foreground, foregroundBmp);
  bitmap_layer_set_compositing_mode(foreground, GCompOpAnd);
  layer_add_child(window_layer, (Layer*)foreground);

  date = bitmap_layer_create(GRect(0, 0, SCREENW, SCREENH));
  bitmap_layer_set_bitmap(date, dateBmp);
  bitmap_layer_set_compositing_mode(date, GCompOpOr);
  layer_add_child(window_layer, (Layer*)date);
}

static void window_unload(Window *window) {

  bitmap_layer_destroy(background);
  bitmap_layer_destroy(hours);
  bitmap_layer_destroy(minutes);
  bitmap_layer_destroy(foreground);
  bitmap_layer_destroy(date);

}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  for(int i=0; i<NUM_IMAGES; i++) {
    digitBmp[i] = gbitmap_create_with_resource(digitId[i]);
  }

  backgroundBmp = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  hoursBmp = gbitmap_create_with_resource(RESOURCE_ID_HOURS);
  minutesBmp = gbitmap_create_with_resource(RESOURCE_ID_MINUTES);
  foregroundBmp = gbitmap_create_with_resource(RESOURCE_ID_FOREGROUND);
  dateBmp = gbitmap_create_with_resource(RESOURCE_ID_DATE);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  
  for(int i=0; i<NUM_IMAGES; i++) {
    gbitmap_destroy(digitBmp[i]);
  }

  gbitmap_destroy(backgroundBmp);
  gbitmap_destroy(hoursBmp);
  gbitmap_destroy(minutesBmp);
  gbitmap_destroy(foregroundBmp);
  gbitmap_destroy(dateBmp);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
