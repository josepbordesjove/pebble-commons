// pebble-commons: progress_layer
// SPDX-License-Identifier: MIT

#include "progress_layer.h"

typedef struct {
  int16_t progress_percent;
  uint16_t corner_radius;
  GColor foreground_color;
  GColor background_color;
} ProgressLayerData;

static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);

  #ifdef PBL_COLOR
  // Draw background fill on color displays
  graphics_context_set_fill_color(ctx, data->background_color);
  graphics_fill_rect(ctx, bounds, data->corner_radius, GCornersAll);
  #endif

  // Draw progress bar fill
  int progress_width = (data->progress_percent * bounds.size.w) / 100;
  if (progress_width > 0) {
    GRect progress_rect = GRect(bounds.origin.x, bounds.origin.y,
                                 progress_width, bounds.size.h);
    graphics_context_set_fill_color(ctx, data->foreground_color);
    graphics_fill_rect(ctx, progress_rect, data->corner_radius, GCornersAll);
  }

  // Draw border outline (ensures clean look on both color and B&W)
  graphics_context_set_stroke_color(ctx, data->foreground_color);
  graphics_draw_round_rect(ctx, bounds, data->corner_radius);
}

ProgressLayer *progress_layer_create(GRect frame) {
  Layer *layer = layer_create_with_data(frame, sizeof(ProgressLayerData));
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);

  data->progress_percent = 0;
  data->corner_radius = 2;
  data->foreground_color = GColorBlack;
  data->background_color = GColorWhite;

  layer_set_update_proc(layer, progress_layer_update_proc);

  return layer;
}

void progress_layer_destroy(ProgressLayer *layer) {
  if (layer) {
    layer_destroy(layer);
  }
}

Layer *progress_layer_get_layer(ProgressLayer *layer) {
  return layer;
}

void progress_layer_set_progress(ProgressLayer *layer, int16_t progress) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  data->progress_percent = progress > 100 ? 100 : (progress < 0 ? 0 : progress);
  layer_mark_dirty(layer);
}

void progress_layer_increment_progress(ProgressLayer *layer, int16_t amount) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  int16_t new_progress = data->progress_percent + amount;
  progress_layer_set_progress(layer, new_progress);
}

void progress_layer_set_corner_radius(ProgressLayer *layer, uint16_t radius) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  data->corner_radius = radius;
  layer_mark_dirty(layer);
}

void progress_layer_set_foreground_color(ProgressLayer *layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  data->foreground_color = color;
  layer_mark_dirty(layer);
}

void progress_layer_set_background_color(ProgressLayer *layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(layer);
  data->background_color = color;
  layer_mark_dirty(layer);
}
