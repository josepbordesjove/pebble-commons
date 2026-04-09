#include "scroll_demo_window.h"
#include "../commons/scroll_text_layer.h"

static Window *s_window;
static ScrollTextLayer *s_scroll_short;
static ScrollTextLayer *s_scroll_long;
static ScrollTextLayer *s_scroll_fast;
static TextLayer *s_label_layer;
static TextLayer *s_label_short;
static TextLayer *s_label_long;
static TextLayer *s_label_fast;

static TextLayer *create_label(GRect frame, const char *text, Layer *parent) {
  TextLayer *label = text_layer_create(frame);
  text_layer_set_text(label, text);
  text_layer_set_font(label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(label, GColorClear);
  text_layer_set_text_color(label, GColorLightGray);
  layer_add_child(parent, text_layer_get_layer(label));
  return label;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDarkGray);
  #else
  window_set_background_color(window, GColorBlack);
  #endif

  int16_t pad = PBL_IF_ROUND_ELSE(30, 8);
  int16_t w = bounds.size.w - pad * 2;
  int16_t y = PBL_IF_ROUND_ELSE(24, 10);

  // Title
  s_label_layer = text_layer_create(GRect(0, y, bounds.size.w, 24));
  text_layer_set_text(s_label_layer, "Scroll Text Layer");
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_color(s_label_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
  y += 30;

  // Demo 1: Default speed
  s_label_short = create_label(GRect(pad, y, w, 16), "Default speed:", window_layer);
  y += 16;

  s_scroll_short = scroll_text_layer_create(GRect(pad, y, w, 20));
  scroll_text_layer_set_text_color(s_scroll_short, GColorWhite);
  scroll_text_layer_set_background_color(s_scroll_short, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
  scroll_text_layer_set_text(s_scroll_short, "Vueling Airlines — Flight VY6249");
  layer_add_child(window_layer, scroll_text_layer_get_layer(s_scroll_short));
  y += 26;

  // Demo 2: Long text
  s_label_long = create_label(GRect(pad, y, w, 16), "Long text:", window_layer);
  y += 16;

  s_scroll_long = scroll_text_layer_create(GRect(pad, y, w, 20));
  scroll_text_layer_set_text_color(s_scroll_long, GColorWhite);
  scroll_text_layer_set_background_color(s_scroll_long, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
  scroll_text_layer_set_font(s_scroll_long, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  scroll_text_layer_set_text(s_scroll_long, "Barcelona El Prat International Airport (BCN)");
  layer_add_child(window_layer, scroll_text_layer_get_layer(s_scroll_long));
  y += 30;

  // Demo 3: Fast speed
  s_label_fast = create_label(GRect(pad, y, w, 16), "Fast (speed 3):", window_layer);
  y += 16;

  s_scroll_fast = scroll_text_layer_create(GRect(pad, y, w, 20));
  scroll_text_layer_set_text_color(s_scroll_fast, PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite));
  scroll_text_layer_set_background_color(s_scroll_fast, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
  scroll_text_layer_set_speed(s_scroll_fast, 3);
  scroll_text_layer_set_pause_ticks(s_scroll_fast, 25);
  scroll_text_layer_set_text(s_scroll_fast, "This text scrolls much faster with a shorter pause at each end");
  layer_add_child(window_layer, scroll_text_layer_get_layer(s_scroll_fast));
}

static void window_unload(Window *window) {
  scroll_text_layer_destroy(s_scroll_short);
  scroll_text_layer_destroy(s_scroll_long);
  scroll_text_layer_destroy(s_scroll_fast);
  text_layer_destroy(s_label_layer);
  text_layer_destroy(s_label_short);
  text_layer_destroy(s_label_long);
  text_layer_destroy(s_label_fast);
  window_destroy(s_window);
  s_window = NULL;
}

void scroll_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
