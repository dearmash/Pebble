#include <pebble.h>

#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84

typedef enum { DIGIT_NORMAL, DIGIT_BIG, DIGIT_SMALL } digit_size_t;

#define NUM_DIGITS 10
#define NUM_IMAGES 30
const int digitId[NUM_IMAGES] = {
  RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2, RESOURCE_ID_IMAGE_3,
  RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5, RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7,
  RESOURCE_ID_IMAGE_8, RESOURCE_ID_IMAGE_9,
  RESOURCE_ID_IMAGE_0_BIG, RESOURCE_ID_IMAGE_1_BIG, RESOURCE_ID_IMAGE_2_BIG, RESOURCE_ID_IMAGE_3_BIG,
  RESOURCE_ID_IMAGE_4_BIG, RESOURCE_ID_IMAGE_5_BIG, RESOURCE_ID_IMAGE_6_BIG, RESOURCE_ID_IMAGE_7_BIG,
  RESOURCE_ID_IMAGE_8_BIG, RESOURCE_ID_IMAGE_9_BIG,
  RESOURCE_ID_IMAGE_0_SMALL, RESOURCE_ID_IMAGE_1_SMALL, RESOURCE_ID_IMAGE_2_SMALL, RESOURCE_ID_IMAGE_3_SMALL,
  RESOURCE_ID_IMAGE_4_SMALL, RESOURCE_ID_IMAGE_5_SMALL, RESOURCE_ID_IMAGE_6_SMALL, RESOURCE_ID_IMAGE_7_SMALL,
  RESOURCE_ID_IMAGE_8_SMALL, RESOURCE_ID_IMAGE_9_SMALL
};

GBitmap *digitBmp[NUM_IMAGES];
GBitmap *backgroundBmp;
GBitmap *foregroundBmp;
GBitmap *hoursBmp;
GBitmap *minutesBmp;

static Window *window;
static BitmapLayer *background;
static BitmapLayer *foreground;
static BitmapLayer *hours;
static BitmapLayer *minutes;

static int number = 0;

static void wheel_draw(GContext *ctx, int number, int angle, int angleSpacing, digit_size_t size) {



}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
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
}

static void window_unload(Window *window) {
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
  foregroundBmp = gbitmap_create_with_resource(RESOURCE_ID_FOREGROUND);
  hoursBmp = gbitmap_create_with_resource(RESOURCE_ID_HOURS);
  minutesBmp = gbitmap_create_with_resource(RESOURCE_ID_MINUTES);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  
  for(int i=0; i<NUM_IMAGES; i++) {
    gbitmap_destroy(digitBmp[i]);
  }

  gbitmap_destroy(backgroundBmp);
  gbitmap_destroy(foregroundBmp);
  gbitmap_destroy(hoursBmp);
  gbitmap_destroy(minutesBmp);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
