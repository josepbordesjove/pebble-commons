// pebble-commons: splash window template
// A minimal splash screen pattern for Pebble apps.
// Copy this file into your project and customize the marked sections.
//
// SPDX-License-Identifier: MIT

#include <pebble.h>

// CUSTOMIZE: Include your next window's header
// #include "main_window.h"

// CUSTOMIZE: Splash duration in milliseconds
#define SPLASH_DURATION_MS 2500

static Window *s_window;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap;
static TextLayer *s_title_layer;
static AppTimer *s_timer;

static void timer_callback(void *context) {
  s_timer = NULL;

  // CUSTOMIZE: Push your next window here
  // main_window_push();

  // Remove splash from stack so back button exits the app
  window_stack_remove(s_window, false);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // CUSTOMIZE: Set your app's splash background color
  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDukeBlue);
  #else
  window_set_background_color(window, GColorWhite);
  #endif

  // CUSTOMIZE: Replace RESOURCE_ID_SPLASH_ICON with your icon resource
  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPLASH_ICON);
  GSize icon_size = gbitmap_get_bounds(s_icon_bitmap).size;

  int16_t icon_x = (bounds.size.w - icon_size.w) / 2;
  int16_t icon_y = bounds.size.h / 2 - icon_size.h - 6;

  s_icon_layer = bitmap_layer_create(GRect(icon_x, icon_y, icon_size.w, icon_size.h));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  // CUSTOMIZE: Set your app's title text
  int16_t title_y = icon_y + icon_size.h + 8;
  s_title_layer = text_layer_create(GRect(0, title_y, bounds.size.w, 30));
  text_layer_set_text(s_title_layer, "My App");
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_title_layer, GColorClear);

  #ifdef PBL_COLOR
  text_layer_set_text_color(s_title_layer, GColorWhite);
  #else
  text_layer_set_text_color(s_title_layer, GColorBlack);
  #endif

  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  // Start auto-advance timer
  s_timer = app_timer_register(SPLASH_DURATION_MS, timer_callback, NULL);
}

static void window_unload(Window *window) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }

  bitmap_layer_destroy(s_icon_layer);
  gbitmap_destroy(s_icon_bitmap);
  text_layer_destroy(s_title_layer);
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
