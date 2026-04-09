// pebble-commons: scroll_text_layer
// SPDX-License-Identifier: MIT

#include "scroll_text_layer.h"

#define DEFAULT_SPEED 1
#define DEFAULT_PAUSE_TICKS 50
#define DEFAULT_INTERVAL_MS 40

typedef struct ScrollTextLayer {
  Layer *layer;
  const char *text;
  GFont font;
  GColor text_color;
  GColor bg_color;
  int speed;
  int pause_ticks;
  int interval_ms;
  int overflow;       // how many pixels the text exceeds the frame
  int offset;         // current scroll offset in pixels
  int direction;      // 1 = scrolling left, -1 = scrolling right
  int pause_remaining;
  AppTimer *timer;
  bool scrolling;
} ScrollTextLayer;

static void scroll_timer_callback(void *context);

static void recalculate_overflow(ScrollTextLayer *stl) {
  if (!stl->text || !stl->font) {
    stl->overflow = 0;
    return;
  }

  GRect bounds = layer_get_bounds(stl->layer);
  GSize text_size = graphics_text_layout_get_content_size(
      stl->text, stl->font,
      GRect(0, 0, 9999, bounds.size.h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);

  int overflow = text_size.w - bounds.size.w;
  stl->overflow = overflow > 0 ? overflow : 0;
}

static void update_proc(Layer *layer, GContext *ctx) {
  ScrollTextLayer *stl = *(ScrollTextLayer **)layer_get_data(layer);
  if (!stl->text || !stl->font) return;

  GRect bounds = layer_get_bounds(layer);

  // Measure full text width for drawing
  GSize text_size = graphics_text_layout_get_content_size(
      stl->text, stl->font,
      GRect(0, 0, 9999, bounds.size.h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);

  // Draw text at current scroll offset
  graphics_context_set_text_color(ctx, stl->text_color);
  int draw_w = text_size.w + 4; // small buffer to avoid clipping
  GRect text_rect = GRect(-stl->offset, 0, draw_w, bounds.size.h);
  graphics_draw_text(ctx, stl->text, stl->font,
                     text_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  // Mask overflow with background color
  if (stl->overflow > 0 && !gcolor_equal(stl->bg_color, GColorClear)) {
    graphics_context_set_fill_color(ctx, stl->bg_color);

    // Mask right edge
    if (stl->offset < stl->overflow) {
      int mask_x = bounds.size.w;
      graphics_fill_rect(ctx, GRect(mask_x, 0, draw_w, bounds.size.h), 0, GCornerNone);
    }

    // Mask left edge when scrolled
    if (stl->offset > 0) {
      graphics_fill_rect(ctx, GRect(-draw_w, 0, draw_w, bounds.size.h), 0, GCornerNone);
    }
  }
}

static void scroll_timer_callback(void *context) {
  ScrollTextLayer *stl = (ScrollTextLayer *)context;
  stl->timer = NULL;

  if (stl->overflow <= 0) {
    stl->scrolling = false;
    return;
  }

  if (stl->pause_remaining > 0) {
    stl->pause_remaining--;
  } else {
    stl->offset += stl->direction * stl->speed;

    if (stl->offset >= stl->overflow) {
      stl->offset = stl->overflow;
      stl->direction = -1;
      stl->pause_remaining = stl->pause_ticks;
    } else if (stl->offset <= 0) {
      stl->offset = 0;
      stl->direction = 1;
      stl->pause_remaining = stl->pause_ticks;
    }
  }

  layer_mark_dirty(stl->layer);
  stl->timer = app_timer_register(stl->interval_ms, scroll_timer_callback, stl);
}

// ── Public API ──────────────────────────────────────────────────────

ScrollTextLayer *scroll_text_layer_create(GRect frame) {
  ScrollTextLayer *stl = malloc(sizeof(ScrollTextLayer));
  if (!stl) return NULL;

  memset(stl, 0, sizeof(ScrollTextLayer));
  stl->layer = layer_create_with_data(frame, sizeof(ScrollTextLayer *));

  // Store pointer to stl in layer data so update_proc can access it
  ScrollTextLayer **data = (ScrollTextLayer **)layer_get_data(stl->layer);
  *data = stl;

  stl->font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  stl->text_color = GColorBlack;
  stl->bg_color = GColorWhite;
  stl->speed = DEFAULT_SPEED;
  stl->pause_ticks = DEFAULT_PAUSE_TICKS;
  stl->interval_ms = DEFAULT_INTERVAL_MS;
  stl->direction = 1;

  layer_set_update_proc(stl->layer, update_proc);
  layer_set_clips(stl->layer, true);

  return stl;
}

void scroll_text_layer_destroy(ScrollTextLayer *layer) {
  if (!layer) return;
  scroll_text_layer_stop(layer);
  layer_destroy(layer->layer);
  free(layer);
}

Layer *scroll_text_layer_get_layer(ScrollTextLayer *layer) {
  return layer->layer;
}

void scroll_text_layer_set_text(ScrollTextLayer *layer, const char *text) {
  layer->text = text;
  recalculate_overflow(layer);

  if (layer->overflow > 0) {
    scroll_text_layer_start(layer);
  } else {
    scroll_text_layer_stop(layer);
    layer_mark_dirty(layer->layer);
  }
}

void scroll_text_layer_set_font(ScrollTextLayer *layer, GFont font) {
  layer->font = font;
  recalculate_overflow(layer);
  layer_mark_dirty(layer->layer);
}

void scroll_text_layer_set_text_color(ScrollTextLayer *layer, GColor color) {
  layer->text_color = color;
  layer_mark_dirty(layer->layer);
}

void scroll_text_layer_set_background_color(ScrollTextLayer *layer, GColor color) {
  layer->bg_color = color;
  layer_mark_dirty(layer->layer);
}

void scroll_text_layer_set_speed(ScrollTextLayer *layer, int speed) {
  layer->speed = speed > 0 ? speed : 1;
}

void scroll_text_layer_set_pause_ticks(ScrollTextLayer *layer, int ticks) {
  layer->pause_ticks = ticks > 0 ? ticks : 0;
}

void scroll_text_layer_set_interval(ScrollTextLayer *layer, int interval_ms) {
  layer->interval_ms = interval_ms > 0 ? interval_ms : DEFAULT_INTERVAL_MS;
}

void scroll_text_layer_start(ScrollTextLayer *layer) {
  layer->offset = 0;
  layer->direction = 1;
  layer->pause_remaining = layer->pause_ticks;
  layer->scrolling = true;

  if (!layer->timer) {
    layer->timer = app_timer_register(layer->interval_ms, scroll_timer_callback, layer);
  }
}

void scroll_text_layer_stop(ScrollTextLayer *layer) {
  if (layer->timer) {
    app_timer_cancel(layer->timer);
    layer->timer = NULL;
  }
  layer->offset = 0;
  layer->scrolling = false;
}

bool scroll_text_layer_is_scrolling(ScrollTextLayer *layer) {
  return layer->scrolling;
}
