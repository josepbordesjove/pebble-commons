// pebble-commons: options_menu
// SPDX-License-Identifier: MIT

#include "options_menu.h"
#include <pebble.h>

static Window *s_window;
static OptionsMenuConfig s_config;

// ── Rectangular (non-round) implementation ──────────────────────────

#if !PBL_ROUND

#define STRIPE_WIDTH 14
#define CIRCLE_RADIUS 4
#define CELL_HEIGHT 44
#define TEXT_PAD 6

static MenuLayer *s_menu_layer;
static Layer *s_stripe_layer;
static AppTimer *s_stripe_timer = NULL;

static uint16_t get_num_rows(MenuLayer *menu, uint16_t section, void *data) {
  return (uint16_t)s_config.num_items;
}

static int16_t get_cell_height(MenuLayer *menu, MenuIndex *index, void *data) {
  return CELL_HEIGHT;
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  const char *title = s_config.items[index->row].title;
  if (!title) return;

  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  int text_w = bounds.size.w - TEXT_PAD - 4;

  GSize text_size = graphics_text_layout_get_content_size(
      title, font, GRect(0, 0, text_w, bounds.size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft);
  int text_y = (bounds.size.h - text_size.h) / 2;

  #ifdef PBL_COLOR
  bool selected = menu_layer_get_selected_index(s_menu_layer).row == index->row;
  graphics_context_set_text_color(ctx, selected ? GColorWhite : GColorLightGray);
  #else
  // B&W: white text on black bg; system inversion makes it black on white when selected
  graphics_context_set_text_color(ctx, GColorWhite);
  #endif

  GRect text_rect = GRect(TEXT_PAD, text_y, text_w, text_size.h);
  graphics_draw_text(ctx, title, font,
                     text_rect, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void select_click(MenuLayer *menu, MenuIndex *index, void *data) {
  if (index->row < s_config.num_items && s_config.items[index->row].callback) {
    s_config.items[index->row].callback(index->row, s_config.callback_context);
  }
}

static void stripe_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  #ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, s_config.accent_color);
  #else
  graphics_context_set_fill_color(ctx, GColorWhite);
  #endif
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (s_menu_layer) {
    MenuIndex idx = menu_layer_get_selected_index(s_menu_layer);
    GPoint scroll_offset = scroll_layer_get_content_offset(
        menu_layer_get_scroll_layer(s_menu_layer));
    int circle_y = idx.row * CELL_HEIGHT + CELL_HEIGHT / 2 + scroll_offset.y;

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, circle_y), CIRCLE_RADIUS);
  }
}

static void stripe_timer_callback(void *context) {
  s_stripe_timer = NULL;
  if (s_stripe_layer) {
    layer_mark_dirty(s_stripe_layer);
  }
  s_stripe_timer = app_timer_register(50, stripe_timer_callback, NULL);
}

#endif // !PBL_ROUND

// ── Round (Chalk/Gabbro) implementation ─────────────────────────────

#if PBL_ROUND

#define ANIM_FRAMES 8
#define ANIM_INTERVAL_MS 30
#define ARC_THICKNESS 12
#define ARC_SPAN_DEG 160
#define DOT_RADIUS 4

// Scale item spacing to screen size (44 on chalk 180px, ~56 on gabbro 228px)
#ifdef PBL_PLATFORM_GABBRO
#define ROUND_ITEM_SPACING 56
#else
#define ROUND_ITEM_SPACING 44
#endif

static Layer *s_round_canvas;
static AppTimer *s_anim_timer = NULL;
static int s_selected_row = 0;
static int s_prev_selected_row = 0;
static int s_anim_frame = 0;
static bool s_animating = false;

static int lerp(int from, int to, int frame, int total) {
  return from + ((to - from) * frame) / total;
}

// Convert a Y position to the angle on the left arc where a dot should sit
static int y_to_arc_angle(int y, int cy, int r) {
  int dy = cy - y; // positive = above center
  if (dy >= r) return 360;   // top
  if (dy <= -r) return 180;  // bottom
  int dx_sq = r * r - dy * dy;
  int dx = 1;
  while (dx * dx < dx_sq) dx++;
  if (dx * dx > dx_sq) dx--;
  int32_t angle_trig = atan2_lookup(dy, -dx);
  return (int)((angle_trig * 360) / TRIG_MAX_ANGLE);
}

// Integer square root using Newton's method
static int isqrt(int n) {
  if (n <= 0) return 0;
  int x = n, y = (x + 1) / 2;
  while (y < x) { x = y; y = (x + n / x) / 2; }
  return x;
}

// Compute left padding so text at (center_y ± half_h) clears the round edge + arc.
// Uses the outermost Y of the text (corner furthest from screen center) to ensure
// the text never clips into the curved screen edge, even at the top/bottom.
static int round_pad_at_y(int center_y, int half_h, int cy, int r, int arc_inset) {
  // Use the edge of the text furthest from center for worst-case padding
  int dy_outer = center_y - cy;
  if (dy_outer < 0) dy_outer = -dy_outer;
  dy_outer += half_h; // extend to the outer edge of the text
  if (dy_outer >= r) return r;
  int edge_x = r - isqrt(r * r - dy_outer * dy_outer);
  return edge_x + arc_inset + 8;
}

static void round_canvas_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int cx = bounds.size.w / 2;
  int cy = bounds.size.h / 2;
  int r = cx; // screen radius

  // Pebble Gothic fonts have ~3px internal top padding before the glyphs.
  // To visually center text at a target Y, we offset the rect upward.
  #define FONT_TOP_PAD 3

  // ── Fixed accent arc (left side of circle) ──
  int32_t arc_start = DEG_TO_TRIGANGLE(270 - ARC_SPAN_DEG / 2);
  int32_t arc_end = DEG_TO_TRIGANGLE(270 + ARC_SPAN_DEG / 2);

  graphics_context_set_fill_color(ctx, s_config.accent_color);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle,
                       ARC_THICKNESS, arc_start, arc_end);

  // ── Scroll offset: selected item scrolls to center ──
  int target_scroll = cy - s_selected_row * ROUND_ITEM_SPACING;
  int prev_scroll = cy - s_prev_selected_row * ROUND_ITEM_SPACING;
  int scroll;
  if (s_animating) {
    scroll = lerp(prev_scroll, target_scroll, s_anim_frame, ANIM_FRAMES);
  } else {
    scroll = target_scroll;
  }

  // ── Menu items ──
  for (int i = 0; i < s_config.num_items; i++) {
    if (!s_config.items[i].title) continue;

    // item_y = visual center of this item
    int item_y = scroll + i * ROUND_ITEM_SPACING;

    // Distance from screen center (for font/color selection)
    int dy = item_y - cy;
    if (dy < 0) dy = -dy;

    // Font: selected (near center) gets larger bold font
    bool near_center = (dy < ROUND_ITEM_SPACING / 2);
    GFont font = fonts_get_system_font(
        near_center ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_14_BOLD);
    int font_h = near_center ? 18 : 14;

    // Fade items based on distance from center
    if (dy < ROUND_ITEM_SPACING / 3) {
      graphics_context_set_text_color(ctx, GColorWhite);
    } else if (dy < ROUND_ITEM_SPACING) {
      graphics_context_set_text_color(ctx, GColorLightGray);
    } else {
      graphics_context_set_text_color(ctx, GColorDarkGray);
    }

    // Position text so its visual center aligns with item_y.
    // Text rect top = item_y - font_h/2 - FONT_TOP_PAD
    int text_top = item_y - font_h / 2 - FONT_TOP_PAD;
    int text_h = font_h + FONT_TOP_PAD * 2 + 4;

    // Padding: compute using the outer edge of the text for worst-case clearance
    int pad = round_pad_at_y(item_y, text_h / 2, cy, r, ARC_THICKNESS);
    int text_w = bounds.size.w - pad - pad / 2; // right padding proportional

    GRect text_rect = GRect(pad, text_top, text_w, text_h);
    graphics_draw_text(ctx, s_config.items[i].title, font,
                       text_rect, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
  }

  // ── Dot indicator — sits on the arc at the selected item's Y ──
  int dot_y_pos = scroll + s_selected_row * ROUND_ITEM_SPACING;
  if (s_animating) {
    dot_y_pos = cy;
  }
  int arc_r = cx - ARC_THICKNESS / 2;
  int dot_angle = y_to_arc_angle(dot_y_pos, cy, arc_r);

  int32_t dot_trig = DEG_TO_TRIGANGLE(dot_angle);
  int dot_x = cx + (arc_r * cos_lookup(dot_trig)) / TRIG_MAX_RATIO;
  int dot_y = cy + (-arc_r * sin_lookup(dot_trig)) / TRIG_MAX_RATIO;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(dot_x, dot_y), DOT_RADIUS);
}

static void round_anim_timer_callback(void *context) {
  s_anim_timer = NULL;
  if (s_animating) {
    s_anim_frame++;
    if (s_anim_frame >= ANIM_FRAMES) {
      s_animating = false;
    }
  }
  if (s_round_canvas) {
    layer_mark_dirty(s_round_canvas);
  }
  s_anim_timer = app_timer_register(ANIM_INTERVAL_MS, round_anim_timer_callback, NULL);
}

static void round_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_animating) return;
  if (s_selected_row > 0) {
    s_prev_selected_row = s_selected_row;
    s_selected_row--;
    s_anim_frame = 0;
    s_animating = true;
  }
}

static void round_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_animating) return;
  if (s_selected_row < s_config.num_items - 1) {
    s_prev_selected_row = s_selected_row;
    s_selected_row++;
    s_anim_frame = 0;
    s_animating = true;
  }
}

static void round_select_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_animating) return;
  if (s_selected_row < s_config.num_items && s_config.items[s_selected_row].callback) {
    s_config.items[s_selected_row].callback(s_selected_row, s_config.callback_context);
  }
}

static void round_click_config(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, round_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, round_down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, round_select_handler);
}

#endif // PBL_ROUND

// ── Window handlers ─────────────────────────────────────────────────

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

#if PBL_ROUND
  s_round_canvas = layer_create(bounds);
  layer_set_update_proc(s_round_canvas, round_canvas_draw);
  layer_add_child(window_layer, s_round_canvas);

  s_selected_row = 0;
  s_prev_selected_row = 0;
  s_animating = false;
  s_anim_frame = 0;

  window_set_click_config_provider(window, round_click_config);
  s_anim_timer = app_timer_register(ANIM_INTERVAL_MS, round_anim_timer_callback, NULL);
#else
  GRect menu_bounds = GRect(STRIPE_WIDTH, 0,
                            bounds.size.w - STRIPE_WIDTH, bounds.size.h);
  s_menu_layer = menu_layer_create(menu_bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows,
    .get_cell_height = get_cell_height,
    .draw_row = draw_row,
    .select_click = select_click,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  #ifdef PBL_COLOR
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, GColorWhite);
  #else
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorWhite, GColorBlack);
  #endif

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_stripe_layer = layer_create(GRect(0, 0, STRIPE_WIDTH, bounds.size.h));
  layer_set_update_proc(s_stripe_layer, stripe_draw);
  layer_add_child(window_layer, s_stripe_layer);

  s_stripe_timer = app_timer_register(50, stripe_timer_callback, NULL);
#endif
}

static void window_unload(Window *window) {
#if PBL_ROUND
  if (s_anim_timer) {
    app_timer_cancel(s_anim_timer);
    s_anim_timer = NULL;
  }
  layer_destroy(s_round_canvas);
  s_round_canvas = NULL;
#else
  if (s_stripe_timer) {
    app_timer_cancel(s_stripe_timer);
    s_stripe_timer = NULL;
  }
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  layer_destroy(s_stripe_layer);
  s_stripe_layer = NULL;
#endif
  window_destroy(s_window);
  s_window = NULL;
}

// ── Public API ──────────────────────────────────────────────────────

void options_menu_push(OptionsMenuConfig *config) {
  s_config = *config;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void options_menu_update_title(int index, const char *title) {
  if (index >= 0 && index < s_config.num_items) {
    s_config.items[index].title = title;
#if PBL_ROUND
    if (s_round_canvas) {
      layer_mark_dirty(s_round_canvas);
    }
#else
    if (s_menu_layer) {
      menu_layer_reload_data(s_menu_layer);
    }
#endif
  }
}
