#include "progress_demo_window.h"
#include "../commons/src/progress_layer.h"

static Window *s_window;
static ProgressLayer *s_progress;
static TextLayer *s_label_layer;
static TextLayer *s_value_layer;
static AppTimer *s_auto_timer;
static char s_value_buf[8];
static int16_t s_current_progress;
static bool s_auto_running;

static void update_value_text(void) {
  snprintf(s_value_buf, sizeof(s_value_buf), "%d%%", s_current_progress);
  text_layer_set_text(s_value_layer, s_value_buf);
}

static void auto_timer_callback(void *context) {
  s_auto_timer = NULL;
  s_current_progress += 2;
  if (s_current_progress > 100) {
    s_current_progress = 0;
  }
  progress_layer_set_progress(s_progress, s_current_progress);
  update_value_text();
  s_auto_timer = app_timer_register(80, auto_timer_callback, NULL);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_auto_running) return;
  s_current_progress += 10;
  if (s_current_progress > 100) s_current_progress = 100;
  progress_layer_set_progress(s_progress, s_current_progress);
  update_value_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_auto_running) return;
  s_current_progress -= 10;
  if (s_current_progress < 0) s_current_progress = 0;
  progress_layer_set_progress(s_progress, s_current_progress);
  update_value_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_auto_running = !s_auto_running;
  if (s_auto_running) {
    s_auto_timer = app_timer_register(80, auto_timer_callback, NULL);
  } else {
    if (s_auto_timer) {
      app_timer_cancel(s_auto_timer);
      s_auto_timer = NULL;
    }
  }
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 150, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 150, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDarkGray);
  #else
  window_set_background_color(window, GColorBlack);
  #endif

  // Title label
  int16_t label_y = PBL_IF_ROUND_ELSE(30, 20);
  s_label_layer = text_layer_create(GRect(0, label_y, bounds.size.w, 30));
  text_layer_set_text(s_label_layer, "Progress Layer");
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_color(s_label_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  // Progress bar
  int16_t bar_w = PBL_IF_ROUND_ELSE(120, bounds.size.w - 40);
  int16_t bar_x = (bounds.size.w - bar_w) / 2;
  int16_t bar_y = bounds.size.h / 2 - 4;

  s_progress = progress_layer_create(GRect(bar_x, bar_y, bar_w, 8));
  progress_layer_set_corner_radius(s_progress, 4);
  #ifdef PBL_COLOR
  progress_layer_set_foreground_color(s_progress, GColorVividCerulean);
  progress_layer_set_background_color(s_progress, GColorLightGray);
  #else
  progress_layer_set_foreground_color(s_progress, GColorWhite);
  progress_layer_set_background_color(s_progress, GColorBlack);
  #endif
  layer_add_child(window_layer, progress_layer_get_layer(s_progress));

  // Value text
  int16_t value_y = bar_y + 16;
  s_value_layer = text_layer_create(GRect(0, value_y, bounds.size.w, 30));
  text_layer_set_text_alignment(s_value_layer, GTextAlignmentCenter);
  text_layer_set_font(s_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_value_layer, GColorClear);
  text_layer_set_text_color(s_value_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_value_layer));

  s_current_progress = 0;
  s_auto_running = false;
  progress_layer_set_progress(s_progress, s_current_progress);
  update_value_text();

  window_set_click_config_provider(window, click_config_provider);
}

static void window_unload(Window *window) {
  if (s_auto_timer) {
    app_timer_cancel(s_auto_timer);
    s_auto_timer = NULL;
  }
  progress_layer_destroy(s_progress);
  text_layer_destroy(s_label_layer);
  text_layer_destroy(s_value_layer);
  window_destroy(s_window);
  s_window = NULL;
}

void progress_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
