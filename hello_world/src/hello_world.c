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

// Definition of the shapes of the numbers

#define NUM_LENGTH 10
#define NUM_WIDTH 21

const short numbers[NUM_LENGTH][NUM_WIDTH] = {
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

// Drawing primitives on the screen

#define CIRCLE_RADIUS 2

#define CIRCLE_RADIUS_2 3

#define DOT_PITCH 7

void draw_dot(GContext *ctx, unsigned short x, unsigned short y) {

  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);

  graphics_fill_circle(ctx, GPoint(x + CIRCLE_RADIUS_2, y + CIRCLE_RADIUS_2), CIRCLE_RADIUS);

}

void draw_num(GContext *ctx, unsigned short x, unsigned short y, unsigned short number) {

  if(number < NUM_LENGTH) {

    for(int i=0; i<NUM_WIDTH && numbers[number][i] != -1; i++) {
      int dx = x + (numbers[number][i] % 4) * DOT_PITCH;
      int dy = y + (numbers[number][i] / 4) * DOT_PITCH;

      draw_dot(ctx, dx, dy);
    }

  }
}

// Handling of the particles that get generated

typedef struct {
  short x, y;
  short dx, dy;
} Particle;

#define MAX_PARTICLES 100

typedef struct {
  Particle contents[MAX_PARTICLES];
  int front;
  int count;
} ParticleQ;

ParticleQ particles;

Animation particle_animation;

void particle_q_push(Particle p) {

  int new_index;

  if(particles.count >= MAX_PARTICLES) {
    return;
  }

  new_index = (particles.front + particles.count) % MAX_PARTICLES;
  particles.contents[new_index] = p;
  particles.count++;

  // TODO: This doesn't work... why?
  // animation_schedule(&particle_animation);

}

void particle_p_pop() {

  if(particles.count <= 0) {
    return;
  }

  particles.front++;
  particles.count--;

  if(particles.count == 0) {
    // TODO: This doesn't work... why?
    // animation_unschedule(&particle_animation);
  }

}

// Particle animation

void particle_animation_setup(struct Animation *animation) {
}

void particle_animation_teardown(struct Animation *animation) {
  // At the end of animation, clear the Q
  particles.count = 0;
}

void particle_animation_update(struct Animation *animation, const uint32_t time_normalized) {
  for(int i=0; i<particles.count; i++) {
    Particle *p = &particles.contents[(i + particles.front) % MAX_PARTICLES];
    p->x += p->dx;
    p->y += p->dy;
  }

}

// Pebble drawing callbacks

void animation_layer_update_callback(Layer *me, GContext* ctx) {

  for(int i=0; i<particles.count; i++) {
    Particle *p = &particles.contents[(i + particles.front) % MAX_PARTICLES];
    draw_dot(ctx, p->x, p->y);
  }

}

void time_layer_update_callback(Layer *me, GContext* ctx) {

  PblTm t;

  get_time(&t);

  draw_num(ctx, 3, 60, t.tm_hour/10);
  draw_num(ctx, 36, 60, t.tm_hour%10);

  draw_num(ctx, 81, 60, t.tm_min/10);
  draw_num(ctx, 114, 60, t.tm_min%10);

  if(t.tm_sec%2==0) {
    draw_dot(ctx, 69, 74);
    draw_dot(ctx, 69, 88);
  } else {
    particle_q_push((Particle) {69, 74, rand()%4+1, rand()%4+1});
    particle_q_push((Particle) {69, 88, rand()%4+1, rand()%4+1});
  }

  draw_num(ctx, 42, 114, t.tm_sec/10);
  draw_num(ctx, 75, 114, t.tm_sec%10);

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

  layer_init(&animation_layer, window.layer.frame);
  animation_layer.update_proc = &animation_layer_update_callback;
  layer_add_child(&window.layer, &animation_layer);

  AnimationImplementation anImp = {
    .setup = &particle_animation_setup,
    .teardown = &particle_animation_teardown,
    .update = &particle_animation_update
  };

  animation_init(&particle_animation);
  animation_set_duration(&particle_animation, 1000);
  animation_set_implementation(&particle_animation, &anImp);

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
