#include <pebble.h>

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })

// Options should be set in wscript via ctx.define("$1", 1) under configure
// Customization options available:
// INVERT_COLORS
// SHOW_DATE
// SHOW_SECONDS
// POP_NUMBERS

// Code goes here

#ifndef INVERT_COLORS
#define COLOR_FOREGROUND GColorBlack
#define COLOR_BACKGROUND GColorWhite
#else
#define COLOR_FOREGROUND GColorWhite
#define COLOR_BACKGROUND GColorBlack
#endif


#ifdef SHOW_DATE 
#ifdef SHOW_SECONDS
// Show both
#define DATE_V_OFFSET 13
#define TIME_V_OFFSET 60
#define SECS_V_OFFSET 114
#else
// Just date
#define DATE_V_OFFSET 37
#define TIME_V_OFFSET 84
#define SECS_V_OFFSET -1
#endif
#elif defined(SHOW_SECONDS)
// Just seconds
#define DATE_V_OFFSET -1
#define TIME_V_OFFSET 37
#define SECS_V_OFFSET 91
#else
// Just time
#define DATE_V_OFFSET -1
#define TIME_V_OFFSET 60
#define SECS_V_OFFSET -1
#endif

short DATE_H_OFFSET = 0;
short TIME_H_OFFSET = 0;
short SECS_H_OFFSET = 0;

Window *window;

Layer *time_layer;

// Definition of the shapes of the numbers

#define NUM_LENGTH 11
#define NUM_WIDTH 21

#define REFRESH_RATE 25

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
  // empty
  {
    -1
  }
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
  float x, y;
  float dx, dy;
  int active;
} Particle;

#define MAX_PARTICLES 100

typedef struct {
  Particle contents[MAX_PARTICLES];
  int front;
  int count;
} ParticleQ;

ParticleQ particles;

AppTimer* particle_timer = NULL;

#define COOKIE_PARTICLE_TIMER 1

void handle_timer(void *data);

void particle_q_push(Particle p) {

  int new_index;

  if(particles.count >= MAX_PARTICLES) {
    return;
  }

  if(p.active == 0) {
    p.active = 2;
  }

#ifdef POP_NUMBERS
  int LO = -5;
  int HI = 3;

  p.dx = LO + (float)rand()/((float)RAND_MAX/(HI-LO));
  p.dy = LO + (float)rand()/((float)RAND_MAX/(HI-LO));
#endif

  new_index = (particles.front + particles.count) % MAX_PARTICLES;
  particles.contents[new_index] = p;
  particles.count++;

  if(particle_timer == NULL) {
    particle_timer = app_timer_register(REFRESH_RATE, handle_timer, NULL);
  }

}

void push_popped_particles(unsigned short x, unsigned short y, short prevNumber, short currentNumber) {

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Popping %u to %u", prevNumber, currentNumber);

  for(int i=0; i<NUM_WIDTH && numbers[prevNumber][i] != -1; i++) {

    bool found = false;
    for(int j=0; j<NUM_WIDTH && numbers[currentNumber][j] != -1; j++) {
      if(numbers[prevNumber][i] == numbers[currentNumber][j]) {
        found = true;
        break;
      }
    }

    if(!found) {
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "%u is missing", numbers[prevNumber][i]);

      int dx = x + (numbers[prevNumber][i] % 4) * DOT_PITCH;
      int dy = y + (numbers[prevNumber][i] / 4) * DOT_PITCH;

      particle_q_push((Particle) {.x = dx, .y = dy});

    }
  }
}

unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}

// Pebble drawing callbacks

struct tm *stored_time_no_flickering;

// The different widths require a little fudge to ensure they're centered
// Time format     - Columns - Total width - Offset to center
// 24h             - 26 7 26 7 5 7 26 7 26 - 137 - 3
// 1               - 5 7 5 7 26 7 26 - 83 - 30
// 2 3 4 5 6 7 8 9 - 26 7 5 7 26 7 26 - 104 - 20
// 10 11 12        - 5 7 26 7 5 26 7 26 - 116 - 14

void nudge_h_offset() {

  short display_hour = get_display_hour(stored_time_no_flickering->tm_hour);
  if(clock_is_24h_style()) {
    if(TIME_H_OFFSET > 0) TIME_H_OFFSET--;
    if(TIME_H_OFFSET < 0) TIME_H_OFFSET++;
  } else if(display_hour == 1) {
    if(TIME_H_OFFSET > -27) TIME_H_OFFSET--;
    if(TIME_H_OFFSET < -27) TIME_H_OFFSET++;
  } else if(display_hour < 10) {
    if(TIME_H_OFFSET > -17) TIME_H_OFFSET--;
    if(TIME_H_OFFSET < -17) TIME_H_OFFSET++;
  } else {
    if(TIME_H_OFFSET > -11) TIME_H_OFFSET--;
    if(TIME_H_OFFSET < -11) TIME_H_OFFSET++;
  }

}

void set_h_offset() {

  short display_hour = get_display_hour(stored_time_no_flickering->tm_hour);
  if(clock_is_24h_style()) {
    TIME_H_OFFSET = 0;
  } else if(display_hour == 1) {
    TIME_H_OFFSET = -27;
  } else if(display_hour < 10) {
    TIME_H_OFFSET = -17;
  } else {
    TIME_H_OFFSET = -11;
  }

}

void time_layer_update_callback(Layer *me, GContext *ctx) {

  struct tm *t = stored_time_no_flickering;

  unsigned short display_hour = get_display_hour(t->tm_hour);

  if(display_hour/10 > 0 || clock_is_24h_style()) {
    draw_num(ctx, 3 + TIME_H_OFFSET, TIME_V_OFFSET, display_hour/10);
  }
  draw_num(ctx, 36 + TIME_H_OFFSET, TIME_V_OFFSET, display_hour%10);

  draw_num(ctx, 81 + TIME_H_OFFSET, TIME_V_OFFSET, t->tm_min/10);
  draw_num(ctx, 114 + TIME_H_OFFSET, TIME_V_OFFSET, t->tm_min%10);

#ifdef SHOW_SECONDS
  draw_num(ctx, 42, SECS_V_OFFSET, t->tm_sec/10);
  draw_num(ctx, 75, SECS_V_OFFSET, t->tm_sec%10);
#endif

  if(t->tm_sec % 4 < 2) {
    draw_dot(ctx, 69 + TIME_H_OFFSET, TIME_V_OFFSET + 14);
    draw_dot(ctx, 69 + TIME_H_OFFSET, TIME_V_OFFSET + 28);
  }

  for(int i=0; i<particles.count; i++) {
    Particle *p = &particles.contents[(i + particles.front) % MAX_PARTICLES];
    draw_dot(ctx, p->x, p->y);
  }  

}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

  time_t now = time(NULL);
  stored_time_no_flickering = localtime(&now);

  if(stored_time_no_flickering->tm_sec % 4 == 2) {
    particle_q_push((Particle) {.x = 69 + TIME_H_OFFSET, .y = TIME_V_OFFSET + 14});
    particle_q_push((Particle) {.x = 69 + TIME_H_OFFSET, .y = TIME_V_OFFSET + 28});
  }

#ifdef SHOW_SECONDS
  push_popped_particles(75, SECS_V_OFFSET, (tick_time->tm_sec+9)%10, tick_time->tm_sec%10);
#endif

  if(tick_time->tm_sec%10 != 0) {
    goto end;
  }

#ifdef SHOW_SECONDS
  push_popped_particles(42, SECS_V_OFFSET, (tick_time->tm_sec/10 + 9)%10, tick_time->tm_sec/10);
#endif

  if(tick_time->tm_sec/10 != 0) {
    goto end;
  }
  push_popped_particles(114 + TIME_H_OFFSET, TIME_V_OFFSET, (tick_time->tm_min+9)%10, tick_time->tm_min%10);

  if(tick_time->tm_min%10 != 0) {
    goto end;
  }
  push_popped_particles(81 + TIME_H_OFFSET, TIME_V_OFFSET, (tick_time->tm_min/10 + 9)%10, tick_time->tm_min/10);

  if(tick_time->tm_min/10 != 0) {
    goto end;
  }

  unsigned short display_hour = get_display_hour(tick_time->tm_hour);          

  if(clock_is_24h_style() || display_hour != 1) {
    push_popped_particles(36 + TIME_H_OFFSET, TIME_V_OFFSET, (display_hour+9)%10, display_hour%10);
  } else {
    push_popped_particles(36 + TIME_H_OFFSET, TIME_V_OFFSET, 2, 1);
  }

  if(clock_is_24h_style() && tick_time->tm_hour%10 != 0) {
    goto end;
  }

  if(!clock_is_24h_style() && display_hour != 1) {
    goto end;
  }

  if(clock_is_24h_style()) {
    push_popped_particles(3 + TIME_H_OFFSET, TIME_V_OFFSET, (tick_time->tm_hour/10+9)%10, tick_time->tm_hour/10);
  } else {
    push_popped_particles(3 + TIME_H_OFFSET, TIME_V_OFFSET, 1, 10);  // Hack to get the full digit to drop
  }

end:
  layer_mark_dirty(time_layer);

}

void handle_timer(void *data) {

  bool keepGoing = false;
  for(int i=0; i<particles.count; i++) {
    Particle *p = &particles.contents[(i + particles.front) % MAX_PARTICLES];

    if(p->active > 0) {
      p->active--;
      keepGoing = true;
      continue;
    }

    p->x += p->dx;
    p->y += p->dy;

    if(p->x < 0 || p->x >= 144 - DOT_PITCH) {
      p->x = max(0, min(p->x, 144 - DOT_PITCH));
      p->dx = -p->dx;
    }

    p->y = max(0, p->y);

    p->dy += 0.3;

    if(p->y < 168) {
      keepGoing = true;
    }
  }

  nudge_h_offset();

  layer_mark_dirty(time_layer);

  if(keepGoing) {
    particle_timer = app_timer_register(REFRESH_RATE, handle_timer, NULL);
  } else {
    particle_timer = NULL;
    particles.count = 0;
  }

}

static void window_load(Window *w) {

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  time_layer = layer_create(bounds);
  layer_set_update_proc(time_layer, time_layer_update_callback);
  layer_add_child(window_layer, time_layer);

}

static void window_unload(Window *w) {

  layer_destroy(time_layer);

}

static void init(void) {

  time_t now = time(NULL);
  stored_time_no_flickering = localtime(&now);
  set_h_offset();

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_set_background_color(window, COLOR_BACKGROUND);

  window_stack_push(window, true /* Animated */);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);  

}

static void deinit(void) {

  tick_timer_service_unsubscribe();
  window_destroy(window);

}

int main(void) {

  init();
  app_event_loop();
  deinit();

}
