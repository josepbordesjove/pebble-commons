#include "icons_demo_window.h"
#include <pebble.h>

#define NUM_ICONS 8
#define ICONS_PER_ROW 4

static Window *s_window;
static GBitmap *s_bitmaps[NUM_ICONS];
static BitmapLayer *s_bitmap_layers[NUM_ICONS];
static TextLayer *s_label_layers[NUM_ICONS];
static TextLayer *s_title_layer;

static const uint32_t s_icon_resources[NUM_ICONS] = {
  RESOURCE_ID_WEATHER_SUNNY,
  RESOURCE_ID_WEATHER_PARTLY_CLOUDY,
  RESOURCE_ID_WEATHER_CLOUDY,
  RESOURCE_ID_WEATHER_LIGHT_RAIN,
  RESOURCE_ID_WEATHER_HEAVY_RAIN,
  RESOURCE_ID_WEATHER_SNOW,
  RESOURCE_ID_WEATHER_RAIN_SNOW,
  RESOURCE_ID_WEATHER_GENERIC,
};

static const char *s_icon_labels[NUM_ICONS] = {
  "Sunny",
  "Partial",
  "Cloudy",
  "Light",
  "Heavy",
  "Snow",
  "Mix",
  "Other",
};

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDarkGray);
  #else
  window_set_background_color(window, GColorBlack);
  #endif

  // Title
  int16_t title_y = PBL_IF_ROUND_ELSE(10, 4);
  s_title_layer = text_layer_create(GRect(0, title_y, bounds.size.w, 24));
  text_layer_set_text(s_title_layer, "Weather Icons");
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  // Grid layout: 4x2 grid of icons with labels
  int16_t grid_start_y = title_y + 28;
  int16_t pad_x = PBL_IF_ROUND_ELSE(20, 6);
  int16_t cell_w = (bounds.size.w - pad_x * 2) / ICONS_PER_ROW;
  int16_t cell_h = (bounds.size.h - grid_start_y - PBL_IF_ROUND_ELSE(16, 4)) / 2;
  int16_t icon_size = 30;

  for (int i = 0; i < NUM_ICONS; i++) {
    int row = i / ICONS_PER_ROW;
    int col = i % ICONS_PER_ROW;
    int16_t cell_x = pad_x + col * cell_w;
    int16_t cell_y = grid_start_y + row * cell_h;

    // Center icon in cell
    int16_t icon_x = cell_x + (cell_w - icon_size) / 2;
    int16_t icon_y = cell_y + 2;

    s_bitmaps[i] = gbitmap_create_with_resource(s_icon_resources[i]);
    s_bitmap_layers[i] = bitmap_layer_create(GRect(icon_x, icon_y, icon_size, icon_size));
    bitmap_layer_set_bitmap(s_bitmap_layers[i], s_bitmaps[i]);
    #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_bitmap_layers[i], GCompOpSet);
    #else
    bitmap_layer_set_compositing_mode(s_bitmap_layers[i], GCompOpAssign);
    #endif
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layers[i]));

    // Label below icon
    int16_t label_y = icon_y + icon_size + 1;
    s_label_layers[i] = text_layer_create(GRect(cell_x, label_y, cell_w, 14));
    text_layer_set_text(s_label_layers[i], s_icon_labels[i]);
    text_layer_set_text_alignment(s_label_layers[i], GTextAlignmentCenter);
    text_layer_set_font(s_label_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_09));
    text_layer_set_background_color(s_label_layers[i], GColorClear);
    text_layer_set_text_color(s_label_layers[i], GColorLightGray);
    layer_add_child(window_layer, text_layer_get_layer(s_label_layers[i]));
  }
}

static void window_unload(Window *window) {
  for (int i = 0; i < NUM_ICONS; i++) {
    bitmap_layer_destroy(s_bitmap_layers[i]);
    gbitmap_destroy(s_bitmaps[i]);
    text_layer_destroy(s_label_layers[i]);
  }
  text_layer_destroy(s_title_layer);
  window_destroy(s_window);
  s_window = NULL;
}

void icons_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
