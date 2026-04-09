// pebble-commons: scroll_text_layer
// A text layer that automatically scrolls (marquee-style) when the text
// overflows its bounds. The text bounces back and forth with configurable
// speed and pause duration at each end.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <pebble.h>

typedef struct ScrollTextLayer ScrollTextLayer;

// Create a scroll text layer with the given frame
ScrollTextLayer *scroll_text_layer_create(GRect frame);

// Destroy the scroll text layer
void scroll_text_layer_destroy(ScrollTextLayer *layer);

// Get the underlying Layer for adding to a parent
Layer *scroll_text_layer_get_layer(ScrollTextLayer *layer);

// Set the text to display. The string must remain valid while displayed.
// Automatically starts scrolling if text overflows the frame width.
void scroll_text_layer_set_text(ScrollTextLayer *layer, const char *text);

// Set the font (default: FONT_KEY_GOTHIC_14_BOLD)
void scroll_text_layer_set_font(ScrollTextLayer *layer, GFont font);

// Set text color (default: GColorBlack)
void scroll_text_layer_set_text_color(ScrollTextLayer *layer, GColor color);

// Set background color used to mask overflow (default: GColorWhite).
// Set to GColorClear to disable masking (caller must handle clipping).
void scroll_text_layer_set_background_color(ScrollTextLayer *layer, GColor color);

// Set scroll speed in pixels per tick (default: 1)
void scroll_text_layer_set_speed(ScrollTextLayer *layer, int speed);

// Set pause duration at each end in ticks (default: 50, ~2s at 40ms interval)
void scroll_text_layer_set_pause_ticks(ScrollTextLayer *layer, int ticks);

// Set animation interval in milliseconds (default: 40, ~25fps)
void scroll_text_layer_set_interval(ScrollTextLayer *layer, int interval_ms);

// Start or restart scrolling (called automatically by set_text if needed)
void scroll_text_layer_start(ScrollTextLayer *layer);

// Stop scrolling and reset to the beginning
void scroll_text_layer_stop(ScrollTextLayer *layer);

// Returns true if the text overflows and scrolling is active
bool scroll_text_layer_is_scrolling(ScrollTextLayer *layer);
