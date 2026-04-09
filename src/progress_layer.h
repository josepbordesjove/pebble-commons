// pebble-commons: progress_layer
// Configurable progress bar widget for Pebble smartwatch apps.
// Supports custom colors, corner radius, and progress tracking (0-100).
//
// SPDX-License-Identifier: MIT

#pragma once

#include <pebble.h>

// Progress layer type
typedef Layer ProgressLayer;

// Create a new progress layer with the given frame
ProgressLayer *progress_layer_create(GRect frame);

// Destroy the progress layer
void progress_layer_destroy(ProgressLayer *layer);

// Get the underlying Layer for adding to a parent layer
Layer *progress_layer_get_layer(ProgressLayer *layer);

// Set progress value (clamped to 0-100)
void progress_layer_set_progress(ProgressLayer *layer, int16_t progress);

// Increment progress by the given amount (clamped to 0-100)
void progress_layer_increment_progress(ProgressLayer *layer, int16_t amount);

// Set corner radius (default: 2)
void progress_layer_set_corner_radius(ProgressLayer *layer, uint16_t radius);

// Set foreground (fill) color (default: GColorBlack)
void progress_layer_set_foreground_color(ProgressLayer *layer, GColor color);

// Set background color (default: GColorWhite, only visible on color displays)
void progress_layer_set_background_color(ProgressLayer *layer, GColor color);
