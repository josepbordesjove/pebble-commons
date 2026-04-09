// pebble-commons: card_pager_layer
// SPDX-License-Identifier: MIT

#include "card_pager_layer.h"

#define ANIM_FRAMES 6
#define ANIM_FRAME_MS 33  // ~200ms total

#define DOT_RADIUS 3
#define DOT_GAP 10
#define DOT_BOTTOM_MARGIN 8

struct CardPagerLayer {
  Layer *root_layer;     // container: hosts both canvas and dots
  Layer *canvas_layer;   // page content (animated horizontally)
  Layer *dots_layer;     // page indicator dots (static)
  CardPagerDrawCallback draw_cb;
  void *draw_context;
  int num_pages;
  int current_page;
  int anim_direction;    // -1 = sliding left (next), +1 = sliding right (prev)
  int anim_frame;
  int anim_offset_x;
  AppTimer *anim_timer;
  GColor active_dot_color;
  GColor inactive_dot_color;
};

// ── Drawing ─────────────────────────────────────────────────────────

static void draw_dots(GContext *ctx, GRect bounds, int current, int total,
                      GColor active, GColor inactive) {
  int total_w = total * (DOT_RADIUS * 2) + (total - 1) * (DOT_GAP - DOT_RADIUS * 2);
  int start_x = (bounds.size.w - total_w) / 2;
  int y = bounds.size.h - DOT_BOTTOM_MARGIN;

  for (int i = 0; i < total; i++) {
    int cx = start_x + i * DOT_GAP + DOT_RADIUS;
    if (i == current) {
      graphics_context_set_fill_color(ctx, active);
      graphics_fill_circle(ctx, GPoint(cx, y), DOT_RADIUS);
    } else {
      graphics_context_set_fill_color(ctx, inactive);
      graphics_fill_circle(ctx, GPoint(cx, y), DOT_RADIUS - 1);
    }
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  CardPagerLayer **ptr = (CardPagerLayer **)layer_get_data(layer);
  CardPagerLayer *pager = *ptr;

  if (pager->draw_cb) {
    GRect bounds = layer_get_bounds(layer);
    pager->draw_cb(ctx, bounds, pager->current_page, pager->draw_context);
  }
}

static void dots_update_proc(Layer *layer, GContext *ctx) {
  CardPagerLayer **ptr = (CardPagerLayer **)layer_get_data(layer);
  CardPagerLayer *pager = *ptr;

  GRect bounds = layer_get_bounds(layer);
  draw_dots(ctx, bounds, pager->current_page, pager->num_pages,
            pager->active_dot_color, pager->inactive_dot_color);
}

// ── Animation ───────────────────────────────────────────────────────

static void anim_timer_callback(void *context) {
  CardPagerLayer *pager = (CardPagerLayer *)context;
  pager->anim_timer = NULL;
  pager->anim_frame++;

  if (pager->anim_frame >= ANIM_FRAMES) {
    // Animation complete
    pager->anim_offset_x = 0;
    pager->anim_direction = 0;
  } else {
    // Ease-out: offset shrinks each frame
    GRect frame = layer_get_frame(pager->canvas_layer);
    int width = frame.size.w;
    int remaining = ANIM_FRAMES - pager->anim_frame;
    pager->anim_offset_x = pager->anim_direction * (width * remaining / ANIM_FRAMES);
    pager->anim_timer = app_timer_register(ANIM_FRAME_MS, anim_timer_callback, pager);
  }

  // Move canvas layer horizontally
  GRect frame = layer_get_frame(pager->canvas_layer);
  frame.origin.x = pager->anim_offset_x;
  layer_set_frame(pager->canvas_layer, frame);
  layer_mark_dirty(pager->canvas_layer);
  layer_mark_dirty(pager->dots_layer);
}

static void start_animation(CardPagerLayer *pager, int direction) {
  if (pager->anim_timer) {
    app_timer_cancel(pager->anim_timer);
    pager->anim_timer = NULL;
  }

  pager->anim_direction = direction;
  pager->anim_frame = 0;

  GRect frame = layer_get_frame(pager->canvas_layer);
  pager->anim_offset_x = direction * frame.size.w;
  frame.origin.x = pager->anim_offset_x;
  layer_set_frame(pager->canvas_layer, frame);

  pager->anim_timer = app_timer_register(ANIM_FRAME_MS, anim_timer_callback, pager);
  layer_mark_dirty(pager->canvas_layer);
  layer_mark_dirty(pager->dots_layer);
}

// ── Public API ──────────────────────────────────────────────────────

CardPagerLayer *card_pager_layer_create(GRect frame, int num_pages) {
  CardPagerLayer *pager = malloc(sizeof(CardPagerLayer));
  if (!pager) return NULL;
  memset(pager, 0, sizeof(CardPagerLayer));

  pager->num_pages = num_pages > 0 ? num_pages : 1;
  pager->active_dot_color = GColorBlack;
  #ifdef PBL_COLOR
  pager->inactive_dot_color = GColorDarkGray;
  #else
  pager->inactive_dot_color = GColorBlack;
  #endif

  // Root layer: container
  pager->root_layer = layer_create(frame);

  // Canvas layer: draws page content, animated horizontally
  GRect inner = GRect(0, 0, frame.size.w, frame.size.h);
  pager->canvas_layer = layer_create_with_data(inner, sizeof(CardPagerLayer *));
  CardPagerLayer **canvas_data = (CardPagerLayer **)layer_get_data(pager->canvas_layer);
  *canvas_data = pager;
  layer_set_update_proc(pager->canvas_layer, canvas_update_proc);
  layer_set_clips(pager->canvas_layer, true);
  layer_add_child(pager->root_layer, pager->canvas_layer);

  // Dots layer: on top, not animated
  pager->dots_layer = layer_create_with_data(inner, sizeof(CardPagerLayer *));
  CardPagerLayer **dots_data = (CardPagerLayer **)layer_get_data(pager->dots_layer);
  *dots_data = pager;
  layer_set_update_proc(pager->dots_layer, dots_update_proc);
  layer_add_child(pager->root_layer, pager->dots_layer);

  return pager;
}

void card_pager_layer_destroy(CardPagerLayer *pager) {
  if (!pager) return;
  if (pager->anim_timer) {
    app_timer_cancel(pager->anim_timer);
    pager->anim_timer = NULL;
  }
  layer_destroy(pager->dots_layer);
  layer_destroy(pager->canvas_layer);
  layer_destroy(pager->root_layer);
  free(pager);
}

Layer *card_pager_layer_get_layer(CardPagerLayer *pager) {
  return pager->root_layer;
}

void card_pager_layer_set_draw_callback(CardPagerLayer *pager,
                                        CardPagerDrawCallback callback,
                                        void *context) {
  pager->draw_cb = callback;
  pager->draw_context = context;
  layer_mark_dirty(pager->canvas_layer);
}

void card_pager_layer_set_num_pages(CardPagerLayer *pager, int num_pages) {
  pager->num_pages = num_pages > 0 ? num_pages : 1;
  pager->current_page = 0;
  pager->anim_offset_x = 0;
  pager->anim_direction = 0;
  if (pager->anim_timer) {
    app_timer_cancel(pager->anim_timer);
    pager->anim_timer = NULL;
  }
  layer_mark_dirty(pager->canvas_layer);
  layer_mark_dirty(pager->dots_layer);
}

int card_pager_layer_get_current_page(CardPagerLayer *pager) {
  return pager->current_page;
}

void card_pager_layer_set_current_page(CardPagerLayer *pager, int page, bool animated) {
  if (page < 0 || page >= pager->num_pages) return;
  if (page == pager->current_page) return;

  int direction = (page > pager->current_page) ? 1 : -1;
  pager->current_page = page;

  if (animated) {
    start_animation(pager, direction);
  } else {
    pager->anim_offset_x = 0;
    pager->anim_direction = 0;
    GRect frame = layer_get_frame(pager->canvas_layer);
    frame.origin.x = 0;
    layer_set_frame(pager->canvas_layer, frame);
    layer_mark_dirty(pager->canvas_layer);
    layer_mark_dirty(pager->dots_layer);
  }
}

bool card_pager_layer_next(CardPagerLayer *pager) {
  if (pager->anim_timer) return false;
  if (pager->current_page >= pager->num_pages - 1) return false;
  pager->current_page++;
  start_animation(pager, 1);
  return true;
}

bool card_pager_layer_previous(CardPagerLayer *pager) {
  if (pager->anim_timer) return false;
  if (pager->current_page <= 0) return false;
  pager->current_page--;
  start_animation(pager, -1);
  return true;
}

void card_pager_layer_set_active_dot_color(CardPagerLayer *pager, GColor color) {
  pager->active_dot_color = color;
  layer_mark_dirty(pager->dots_layer);
}

void card_pager_layer_set_inactive_dot_color(CardPagerLayer *pager, GColor color) {
  pager->inactive_dot_color = color;
  layer_mark_dirty(pager->dots_layer);
}

void card_pager_layer_mark_dirty(CardPagerLayer *pager) {
  layer_mark_dirty(pager->canvas_layer);
}

bool card_pager_layer_is_animating(CardPagerLayer *pager) {
  return pager->anim_timer != NULL;
}
