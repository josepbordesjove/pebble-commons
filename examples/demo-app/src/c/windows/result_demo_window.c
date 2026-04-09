#include "result_demo_window.h"
#include <pebble.h>

#define NUM_MATCHES 3
#define CELL_HEIGHT 58
#define CELL_HEIGHT_ROUND 64
#define ACCENT_COLOR GColorMayGreen

// Pulse animation
#define PULSE_INTERVAL_MS 80
#define PULSE_STEP 15
#define PULSE_MIN 80
#define PULSE_MAX 255

typedef struct {
  const char *home;
  const char *away;
  const char *home_full;
  const char *away_full;
  int home_score;
  int away_score;
  const char *status;
  const char *venue;
  const char *referee;
  bool is_live;
  bool is_finished;
} MatchData;

static const MatchData s_matches[NUM_MATCHES] = {
  { "BAR", "RMA", "FC Barcelona", "Real Madrid CF",
    3, 1, "FT", "Camp Nou", "Del Cerro Grande", false, true },
  { "ATM", "SEV", "Atletico Madrid", "Sevilla FC",
    2, 2, "67'", "Metropolitano", "Mateu Lahoz", true, false },
  { "VIL", "BET", "Villarreal CF", "Real Betis",
    0, 0, "15:30", "La Ceramica", "Gil Manzano", false, false },
};

// ── Match Detail Window ─────────────────────────────────────────────

static Window *s_detail_window;
static Layer *s_detail_canvas;
static const MatchData *s_detail_match;

static void detail_canvas_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const MatchData *m = s_detail_match;
  if (!m) return;

  int16_t w = bounds.size.w;
  int16_t pad = PBL_IF_ROUND_ELSE(20, 8);

  GFont big_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont name_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  #ifdef PBL_COLOR
  GColor text_main = GColorWhite;
  GColor text_dim = GColorCeleste;
  GColor text_accent = m->is_live ? GColorSunsetOrange : GColorWhite;
  #else
  GColor text_main = GColorBlack;
  GColor text_dim = GColorBlack;
  GColor text_accent = GColorBlack;
  #endif

  // Status at top
  int16_t y = PBL_IF_ROUND_ELSE(14, 6);
  graphics_context_set_text_color(ctx, text_accent);
  graphics_draw_text(ctx, m->status, name_font,
                     GRect(pad, y, w - pad * 2, 22),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Home team name
  y += 24;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->home_full, small_font,
                     GRect(pad, y, w - pad * 2, 18),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Big score
  y += 16;
  char score_buf[12];
  snprintf(score_buf, sizeof(score_buf), "%d - %d", m->home_score, m->away_score);
  graphics_context_set_text_color(ctx, text_main);
  graphics_draw_text(ctx, score_buf, big_font,
                     GRect(0, y, w, 34),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Away team name
  y += 34;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->away_full, small_font,
                     GRect(pad, y, w - pad * 2, 18),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Separator
  y += 24;
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, GColorMintGreen);
  #else
  graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  graphics_draw_line(ctx, GPoint(pad + 10, y), GPoint(w - pad - 10, y));

  // Venue
  y += 8;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->venue, small_font,
                     GRect(pad, y, w - pad * 2, 18),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Referee
  y += 16;
  graphics_draw_text(ctx, m->referee, small_font,
                     GRect(pad, y, w - pad * 2, 18),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void detail_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDarkGreen);
  #else
  window_set_background_color(window, GColorWhite);
  #endif

  s_detail_canvas = layer_create(bounds);
  layer_set_update_proc(s_detail_canvas, detail_canvas_draw);
  layer_add_child(root, s_detail_canvas);
}

static void detail_unload(Window *window) {
  layer_destroy(s_detail_canvas);
  window_destroy(s_detail_window);
  s_detail_window = NULL;
}

static void show_detail(int index) {
  s_detail_match = &s_matches[index];
  s_detail_window = window_create();
  window_set_window_handlers(s_detail_window, (WindowHandlers) {
    .load = detail_load, .unload = detail_unload,
  });
  window_stack_push(s_detail_window, true);
}

// ── Match List Window ───────────────────────────────────────────────

static Window *s_window;
static MenuLayer *s_menu_layer;
static AppTimer *s_pulse_timer;
static uint8_t s_pulse_alpha = PULSE_MAX;
static bool s_pulse_increasing = false;

#if !PBL_ROUND
static TextLayer *s_header_layer;
#endif

static bool has_live(void) {
  for (int i = 0; i < NUM_MATCHES; i++) if (s_matches[i].is_live) return true;
  return false;
}

static void pulse_callback(void *context) {
  if (s_pulse_increasing) {
    s_pulse_alpha += PULSE_STEP;
    if (s_pulse_alpha >= PULSE_MAX) { s_pulse_alpha = PULSE_MAX; s_pulse_increasing = false; }
  } else {
    s_pulse_alpha -= PULSE_STEP;
    if (s_pulse_alpha <= PULSE_MIN) { s_pulse_alpha = PULSE_MIN; s_pulse_increasing = true; }
  }
  if (s_menu_layer) layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  if (has_live()) {
    s_pulse_timer = app_timer_register(PULSE_INTERVAL_MS, pulse_callback, NULL);
  } else {
    s_pulse_timer = NULL;
  }
}

static uint16_t get_num_rows(MenuLayer *ml, uint16_t section, void *data) {
  return NUM_MATCHES;
}

static int16_t get_cell_height(MenuLayer *ml, MenuIndex *idx, void *data) {
  return PBL_IF_ROUND_ELSE(CELL_HEIGHT_ROUND, CELL_HEIGHT);
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  const MatchData *m = &s_matches[idx->row];
  bool selected = menu_layer_is_index_selected(s_menu_layer, idx);

  // Selection fills entire cell with accent color
  #ifdef PBL_COLOR
  GColor bg = selected ? ACCENT_COLOR : GColorWhite;
  GColor text = selected ? GColorWhite : GColorBlack;
  GColor dim = selected ? GColorWhite : GColorDarkGray;
  #else
  GColor bg = selected ? GColorBlack : GColorWhite;
  GColor text = selected ? GColorWhite : GColorBlack;
  GColor dim = text;
  #endif

  graphics_context_set_fill_color(ctx, bg);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  int margin = PBL_IF_ROUND_ELSE(12, 6);
  int content_w = bounds.size.w - margin * 2;
  int y = 4;

  GFont team_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont status_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  // Line 1: Home TLA (left) + score (right)
  char score_str[8];
  snprintf(score_str, sizeof(score_str), m->is_finished || m->is_live ? "%d" : "-", m->home_score);
  graphics_context_set_text_color(ctx, text);
  GRect row1 = GRect(margin, y, content_w, 20);
  graphics_draw_text(ctx, m->home, team_font, row1,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, score_str, team_font, row1,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Line 2: Away TLA (left) + score (right)
  y += 18;
  snprintf(score_str, sizeof(score_str), m->is_finished || m->is_live ? "%d" : "-", m->away_score);
  GRect row2 = GRect(margin, y, content_w, 20);
  graphics_draw_text(ctx, m->away, team_font, row2,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, score_str, team_font, row2,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Line 3: Status with optional live dot
  y += 18;
  graphics_context_set_text_color(ctx, dim);

  if (m->is_live) {
    // Pulsing dot left of status text
    int dot_x = margin + 4;
    int dot_y = y + 8;
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, s_pulse_alpha > 180 ?
      (selected ? GColorWhite : GColorRed) :
      (selected ? GColorCeleste : GColorDarkCandyAppleRed));
    graphics_fill_circle(ctx, GPoint(dot_x, dot_y), 3);
    #else
    if (s_pulse_alpha > 160) {
      graphics_context_set_fill_color(ctx, text);
      graphics_fill_circle(ctx, GPoint(dot_x, dot_y), 3);
    }
    #endif
    GRect status_rect = GRect(margin + 12, y, content_w - 12, 16);
    graphics_draw_text(ctx, m->status, status_font, status_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  } else {
    GRect row3 = GRect(margin, y, content_w, 16);
    graphics_draw_text(ctx, m->status, status_font, row3,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  // Separator only on unselected rows
  if (!selected) {
    #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
    #endif
    graphics_draw_line(ctx, GPoint(margin, bounds.size.h - 1),
                       GPoint(bounds.size.w - margin, bounds.size.h - 1));
  }
}

static void select_click(MenuLayer *ml, MenuIndex *idx, void *data) {
  show_detail(idx->row);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  window_set_background_color(window, GColorWhite);

  int menu_top = 0;

  #if !PBL_ROUND
  int header_h = 20;
  s_header_layer = text_layer_create(GRect(0, 0, bounds.size.w, header_h));
  text_layer_set_text(s_header_layer, "La Liga - Matchday 30");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  #ifdef PBL_COLOR
  text_layer_set_background_color(s_header_layer, ACCENT_COLOR);
  #else
  text_layer_set_background_color(s_header_layer, GColorBlack);
  #endif
  text_layer_set_text_color(s_header_layer, GColorWhite);
  layer_add_child(root, text_layer_get_layer(s_header_layer));
  menu_top = header_h;
  #endif

  GRect menu_bounds = GRect(0, menu_top, bounds.size.w, bounds.size.h - menu_top);
  s_menu_layer = menu_layer_create(menu_bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows,
    .get_cell_height = get_cell_height,
    .draw_row = draw_row,
    .select_click = select_click,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  #ifdef PBL_COLOR
  menu_layer_set_highlight_colors(s_menu_layer, ACCENT_COLOR, GColorWhite);
  #endif

  #if PBL_ROUND
  menu_layer_set_center_focused(s_menu_layer, true);
  #endif

  layer_add_child(root, menu_layer_get_layer(s_menu_layer));

  if (has_live()) {
    s_pulse_alpha = PULSE_MAX;
    s_pulse_increasing = false;
    s_pulse_timer = app_timer_register(PULSE_INTERVAL_MS, pulse_callback, NULL);
  }
}

static void window_unload(Window *window) {
  if (s_pulse_timer) { app_timer_cancel(s_pulse_timer); s_pulse_timer = NULL; }
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  #if !PBL_ROUND
  text_layer_destroy(s_header_layer);
  #endif
  window_destroy(s_window);
  s_window = NULL;
}

void result_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load, .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
