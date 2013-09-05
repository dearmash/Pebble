#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x90, 0x55, 0xDE, 0x3D, 0x8F, 0x8B, 0x43, 0x74, 0x94, 0x34, 0x1D, 0x46, 0xC9, 0xBE, 0xFF, 0xBD }
PBL_APP_INFO(MY_UUID,
             "Hello world", "qni Enterprises",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define INVERT_COLORS

#ifndef INVERT_COLORS
#define COLOR_FOREGROUND GColorBlack
#define COLOR_BACKGROUND GColorWhite
#else
#define COLOR_FOREGROUND GColorWhite
#define COLOR_BACKGROUND GColorBlack
#endif


Window window;

TextLayer hello_layer;

Layer time_layer;

Layer animation_layer;

#define NUM_LENGTH 10
#define NUM_WIDTH 21

const unsigned short numbers[NUM_LENGTH][NUM_WIDTH] = {
            // 0
            {
                    0, 1, 2, 3, 4, 7, 8, 11, 12, 15, 16, 19, 20, 23, 24, 25, 26, 27, -1
            },
            // 1
            {
                    3, 7, 11, 15, 19, 23, 27, -1
            },
            // 2
            {
                    0, 1, 2, 3, 7, 11, 12, 13, 14, 15, 16, 20, 24, 25, 26, 27, -1
            },
            // 3
            {
                    0, 1, 2, 3, 7, 11, 12, 13, 14, 15, 19, 23, 24, 25, 26, 27, -1
            },
            // 4
            {
                    0, 3, 4, 7, 8, 11, 12, 13, 14, 15, 19, 23, 27, -1
            },
            // 5
            {
                    0, 1, 2, 3, 4, 8, 12, 13, 14, 15, 19, 23, 24, 25, 26, 27, -1
            },
            // 6
            {
                    0, 1, 2, 3, 4, 8, 12, 13, 14, 15, 16, 19, 20, 23, 24, 25, 26, 27, -1
            },
            // 7
            {
                    0, 1, 2, 3, 7, 11, 15, 19, 23, 27, -1
            },
            // 8
            {
                    0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15, 16, 19, 20, 23, 24, 25, 26, 27, -1
            },
            // 9
            {
                    0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15, 19, 23, 24, 25, 26, 27, -1
            },
    };


#define CIRCLE_RADIUS 2

#define CIRCLE_RADIUS_2 3

#define DOT_PITCH 7

void draw_dot(GContext *ctx, unsigned short x, unsigned short y) {

  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);

  graphics_fill_circle(ctx, GPoint(x + CIRCLE_RADIUS_2, y + CIRCLE_RADIUS_2), CIRCLE_RADIUS);

}

void draw_num(GContext *ctx, unsigned short x, unsigned short y, unsigned short number) {

  if(number < NUM_LENGTH) {

    for(int i=0; i<NUM_WIDTH; i++) {
      int dx = x + (numbers[number][i] % 4) * DOT_PITCH;
      int dy = y + (numbers[number][i] / 4) * DOT_PITCH;

      draw_dot(ctx, dx, dy);
    }

  }
}

void time_layer_update_callback(Layer *me, GContext* ctx) {

  draw_num(ctx, 3, 60, 8);
  draw_num(ctx, 36, 60, 8);
  draw_num(ctx, 81, 60, 8);
  draw_num(ctx, 114, 60, 8);

  draw_dot(ctx, 69, 74);
  draw_dot(ctx, 69, 88);

}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {

  layer_mark_dirty(&time_layer);

}

void handle_init(AppContextRef ctx) {

  window_init(&window, "Hello world");
  window_stack_push(&window, true /* Animated */);

  window_set_background_color(&window, COLOR_BACKGROUND);

  text_layer_init(&hello_layer, GRect(0, 0, 144, 30));
  text_layer_set_text_alignment(&hello_layer, GTextAlignmentCenter);
  text_layer_set_text_color(&hello_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(&hello_layer, GColorClear);
  text_layer_set_text(&hello_layer, "Hello world");
  text_layer_set_font(&hello_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(&window.layer, &hello_layer.layer);

  layer_init(&time_layer, window.layer.frame);
  time_layer.update_proc = &time_layer_update_callback;
  layer_add_child(&window.layer, &time_layer);

}

void pbl_main(void *params) {

  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
	  }
  };
  app_event_loop(params, &handlers);

}
