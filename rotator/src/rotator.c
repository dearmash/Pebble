#include <pebble.h>

static Window *window = NULL;

static Layer *window_layer;

static AppTimer* animator = NULL;

static int angle = 0;

static const GPathInfo ZERO_PATH = {
  .num_points = 20,
  .points = (GPoint[]) {
    {0, 26}, 
    {0, 5}, {5, 0}, {21, 0}, {26, 5}, {26, 35}, {21, 40}, {5, 40}, {0, 35},
    {0, 12}, {8, 12},
    {8, 30}, {10, 32}, {16, 32}, {18, 30}, {18, 9}, {16, 7}, {10, 7}, {8, 9},
    {8, 26}
  }
};

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void window_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(window_layer);

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_rect(ctx, bounds, 0, 0);

  graphics_context_set_fill_color(ctx, GColorBlack);

  GPath *pathPtr = gpath_create(&ZERO_PATH);

//  graphics_draw_circle(ctx, GPoint(72, 268), 238);

//  graphics_draw_circle(ctx, GPoint(72, 268), 198);

  gpath_move_to(pathPtr, GPoint(72, 268));

  for(int itr = -1; itr <= 2; itr++) {

    gpath_rotate_to(pathPtr, (0x10000 / 20) * itr - angle);

    for(uint32_t i=0; i<pathPtr->num_points; i++) {
      pathPtr->points[i].x += 3;
      pathPtr->points[i].y -= 238;
    }

    gpath_draw_filled(ctx, pathPtr);

    for(uint32_t i=0; i<pathPtr->num_points; i++) {
      pathPtr->points[i].x -= 32;
    }

    gpath_draw_filled(ctx, pathPtr);

    for(uint32_t i=0; i<pathPtr->num_points; i++) {
      pathPtr->points[i].x += 29;
      pathPtr->points[i].y += 238;
    }
  }

//  graphics_draw_circle(ctx, GPoint(72, 268), 178);

//  graphics_draw_circle(ctx, GPoint(72, 268), 148);  

  gpath_destroy(pathPtr);

}

static void animator_tick() {
  if(window != NULL) {

    angle += (0x10000/20)/(1000/50);

    if(angle > 0x10000/20) {
      angle = 1;
      animator = app_timer_register(1000, animator_tick, NULL);
    } else {
      animator = app_timer_register(50, animator_tick, NULL);
    }

    layer_mark_dirty(window_layer);
/*
    if(angle % (0x10000 / 11) == 0)
      animator = app_timer_register(1000, animator_tick, NULL);
    else 
      animator = app_timer_register(40, animator_tick, NULL);
      */
  }
}

static void window_load(Window *window) {
  window_layer = window_get_root_layer(window);

  layer_set_update_proc(window_layer, window_update_proc);

  animator = app_timer_register(20, animator_tick, NULL);
}

static void window_unload(Window *window) {

  app_timer_cancel(animator);
}

static void init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Do init");

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  window = NULL;
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
