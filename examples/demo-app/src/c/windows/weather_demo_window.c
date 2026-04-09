#include "weather_demo_window.h"
#include <pebble.h>

static Window *s_window;
static Layer *s_canvas_layer;
static GBitmap *s_weather_bitmap;

static void canvas_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int16_t pad = PBL_IF_ROUND_ELSE(30, 10);
  int16_t mid_x = w / 2;

  GFont big_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // ── City name ──
  int16_t title_y = PBL_IF_ROUND_ELSE(16, 6);
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorWhite);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_text(ctx, "Barcelona", title_font,
                     GRect(0, title_y, w, 22),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // ── Two-column data ──
  int16_t data_y = title_y + 28;
  int16_t col_w = mid_x - pad - 4;

  // Vertical divider
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, GColorWhite);
  #else
  graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  graphics_draw_line(ctx, GPoint(mid_x, data_y + 2),
                     GPoint(mid_x, data_y + 50));

  // Left column: Temperature
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorWhite);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_text(ctx, "23°C", big_font,
                     GRect(pad, data_y, col_w, 32),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorLightGray);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_text(ctx, "Sunny", small_font,
                     GRect(pad, data_y + 30, col_w, 18),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // Right column: Wind
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorWhite);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_text(ctx, "NE", big_font,
                     GRect(mid_x + 4, data_y, col_w, 32),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorLightGray);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_text(ctx, "12 km/h", small_font,
                     GRect(mid_x + 4, data_y + 30, col_w, 18),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // ── Large weather icon (centered) ──
  if (s_weather_bitmap) {
    GSize icon_size = gbitmap_get_bounds(s_weather_bitmap).size;
    int16_t icon_x = (w - icon_size.w) / 2;
    int16_t icon_y = data_y + 58;

    #ifdef PBL_COLOR
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    #else
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    #endif
    graphics_draw_bitmap_in_rect(ctx, s_weather_bitmap,
                                 GRect(icon_x, icon_y, icon_size.w, icon_size.h));
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorPictonBlue);
  #else
  window_set_background_color(window, GColorWhite);
  #endif

  s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_SUNNY);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_draw);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  if (s_weather_bitmap) {
    gbitmap_destroy(s_weather_bitmap);
    s_weather_bitmap = NULL;
  }
  window_destroy(s_window);
  s_window = NULL;
}

void weather_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
