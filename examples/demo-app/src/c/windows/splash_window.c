#include "splash_window.h"
#include "../commons/progress_layer.h"
#include <pebble.h>

#define SPLASH_DURATION_MS 2500
#define PROGRESS_INTERVAL_MS 50
#define PROGRESS_STEP 2

// Defined in main.c
extern void main_window_push(void);

static Window *s_window;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;
static TextLayer *s_subtitle_layer;
static ProgressLayer *s_progress;
static AppTimer *s_splash_timer;
static AppTimer *s_progress_timer;
static int16_t s_progress_value;

static void splash_timer_callback(void *context) {
  s_splash_timer = NULL;
  main_window_push();
  window_stack_remove(s_window, false);
}

static void progress_timer_callback(void *context) {
  s_progress_timer = NULL;
  s_progress_value += PROGRESS_STEP;
  if (s_progress_value > 100) s_progress_value = 100;
  progress_layer_set_progress(s_progress, s_progress_value);
  if (s_progress_value < 100) {
    s_progress_timer = app_timer_register(PROGRESS_INTERVAL_MS, progress_timer_callback, NULL);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorBlack);
  #else
  window_set_background_color(window, GColorWhite);
  #endif

  // Logo centered vertically, slightly above center
  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PEBBLE_LOGO);
  GSize logo_size = gbitmap_get_bounds(s_logo_bitmap).size;

  int16_t logo_x = (bounds.size.w - logo_size.w) / 2;
  int16_t logo_y = bounds.size.h / 2 - logo_size.h - 4;

  s_logo_layer = bitmap_layer_create(GRect(logo_x, logo_y, logo_size.w, logo_size.h));
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  #ifdef PBL_COLOR
  bitmap_layer_set_compositing_mode(s_logo_layer, GCompOpSet);
  #else
  bitmap_layer_set_compositing_mode(s_logo_layer, GCompOpAssign);
  #endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));

  // "commons" subtitle below logo
  int16_t subtitle_y = logo_y + logo_size.h + 2;
  s_subtitle_layer = text_layer_create(GRect(0, subtitle_y, bounds.size.w, 24));
  text_layer_set_text(s_subtitle_layer, "commons");
  text_layer_set_text_alignment(s_subtitle_layer, GTextAlignmentCenter);
  text_layer_set_font(s_subtitle_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_subtitle_layer, GColorClear);
  #ifdef PBL_COLOR
  text_layer_set_text_color(s_subtitle_layer, GColorLightGray);
  #else
  text_layer_set_text_color(s_subtitle_layer, GColorBlack);
  #endif
  layer_add_child(window_layer, text_layer_get_layer(s_subtitle_layer));

  // Progress bar near the bottom
  int16_t bar_w = PBL_IF_ROUND_ELSE(100, 80);
  int16_t bar_x = (bounds.size.w - bar_w) / 2;
  int16_t bar_y = subtitle_y + 34;

  s_progress = progress_layer_create(GRect(bar_x, bar_y, bar_w, 4));
  progress_layer_set_corner_radius(s_progress, 2);
  #ifdef PBL_COLOR
  progress_layer_set_foreground_color(s_progress, GColorWhite);
  progress_layer_set_background_color(s_progress, GColorDarkGray);
  #else
  progress_layer_set_foreground_color(s_progress, GColorBlack);
  progress_layer_set_background_color(s_progress, GColorWhite);
  #endif
  layer_add_child(window_layer, progress_layer_get_layer(s_progress));

  // Start progress animation and splash timer
  s_progress_value = 0;
  s_progress_timer = app_timer_register(PROGRESS_INTERVAL_MS, progress_timer_callback, NULL);
  s_splash_timer = app_timer_register(SPLASH_DURATION_MS, splash_timer_callback, NULL);
}

static void window_unload(Window *window) {
  if (s_splash_timer) {
    app_timer_cancel(s_splash_timer);
    s_splash_timer = NULL;
  }
  if (s_progress_timer) {
    app_timer_cancel(s_progress_timer);
    s_progress_timer = NULL;
  }
  progress_layer_destroy(s_progress);
  bitmap_layer_destroy(s_logo_layer);
  gbitmap_destroy(s_logo_bitmap);
  text_layer_destroy(s_subtitle_layer);
  window_destroy(s_window);
  s_window = NULL;
}

void splash_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
