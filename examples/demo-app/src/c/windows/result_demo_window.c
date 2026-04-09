#include "result_demo_window.h"
#include <pebble.h>

#define NUM_MATCHES 3
#define CELL_HEIGHT PBL_IF_ROUND_ELSE(68, 58)
#define ACCENT_COLOR PBL_IF_COLOR_ELSE(GColorMayGreen, GColorBlack)

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
} MatchData;

static const MatchData s_matches[NUM_MATCHES] = {
  { "BAR", "RMA", "FC Barcelona", "Real Madrid CF",
    3, 1, "FT", "Camp Nou", "Del Cerro Grande", false },
  { "ATM", "SEV", "Atletico Madrid", "Sevilla FC",
    2, 2, "LIVE 67'", "Metropolitano", "Mateu Lahoz", true },
  { "VIL", "BET", "Villarreal CF", "Real Betis",
    0, 0, "15:30", "La Ceramica", "Gil Manzano", false },
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
  GTextAlignment center = GTextAlignmentCenter;

  GFont big_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont name_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  #ifdef PBL_COLOR
  GColor text_main = GColorWhite;
  GColor text_dim = GColorLightGray;
  GColor text_accent = m->is_live ? GColorRed : GColorMayGreen;
  #else
  GColor text_main = GColorBlack;
  GColor text_dim = GColorBlack;
  GColor text_accent = GColorBlack;
  #endif

  // Status at top
  int16_t y = PBL_IF_ROUND_ELSE(14, 6);
  graphics_context_set_text_color(ctx, text_accent);
  graphics_draw_text(ctx, m->status, small_font,
                     GRect(pad, y, w - pad * 2, 16), GTextOverflowModeTrailingEllipsis,
                     center, NULL);

  // Home team name
  y += 20;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->home_full, small_font,
                     GRect(pad, y, w - pad * 2, 18), GTextOverflowModeTrailingEllipsis,
                     center, NULL);

  // Big score: "3 - 1"
  y += 16;
  char score_buf[12];
  snprintf(score_buf, sizeof(score_buf), "%d - %d", m->home_score, m->away_score);
  graphics_context_set_text_color(ctx, text_main);
  graphics_draw_text(ctx, score_buf, big_font,
                     GRect(0, y, w, 34), GTextOverflowModeTrailingEllipsis,
                     center, NULL);

  // Away team name
  y += 34;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->away_full, small_font,
                     GRect(pad, y, w - pad * 2, 18), GTextOverflowModeTrailingEllipsis,
                     center, NULL);

  // Separator
  y += 24;
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  #else
  graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  graphics_draw_line(ctx, GPoint(pad + 10, y), GPoint(w - pad - 10, y));

  // Venue
  y += 6;
  graphics_context_set_text_color(ctx, text_dim);
  graphics_draw_text(ctx, m->venue, small_font,
                     GRect(pad, y, w - pad * 2, 18), GTextOverflowModeTrailingEllipsis,
                     center, NULL);

  // Referee
  y += 16;
  graphics_draw_text(ctx, m->referee, small_font,
                     GRect(pad, y, w - pad * 2, 18), GTextOverflowModeTrailingEllipsis,
                     center, NULL);
}

static void detail_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
  window_set_background_color(window, GColorDarkGreen);
  #else
  window_set_background_color(window, GColorWhite);
  #endif

  s_detail_canvas = layer_create(bounds);
  layer_set_update_proc(s_detail_canvas, detail_canvas_draw);
  layer_add_child(window_layer, s_detail_canvas);
}

static void detail_window_unload(Window *window) {
  layer_destroy(s_detail_canvas);
  window_destroy(s_detail_window);
  s_detail_window = NULL;
}

static void show_match_detail(int index) {
  s_detail_match = &s_matches[index];
  s_detail_window = window_create();
  window_set_window_handlers(s_detail_window, (WindowHandlers) {
    .load = detail_window_load,
    .unload = detail_window_unload,
  });
  window_stack_push(s_detail_window, true);
}

// ── Match List Window ───────────────────────────────────────────────

static Window *s_window;
static MenuLayer *s_menu_layer;
static AppTimer *s_pulse_timer;
static bool s_dot_visible = true;

static void pulse_timer_callback(void *context) {
  s_dot_visible = !s_dot_visible;
  if (s_menu_layer) menu_layer_reload_data(s_menu_layer);
  s_pulse_timer = app_timer_register(500, pulse_timer_callback, NULL);
}

static uint16_t get_num_rows(MenuLayer *menu, uint16_t section, void *data) {
  return NUM_MATCHES;
}

static int16_t get_cell_height(MenuLayer *menu, MenuIndex *index, void *data) {
  return CELL_HEIGHT;
}

static int16_t get_header_height(MenuLayer *menu, uint16_t section, void *data) {
  return PBL_IF_ROUND_ELSE(0, 22);
}

static void draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section, void *data) {
  #if !PBL_ROUND
  GRect bounds = layer_get_bounds(cell_layer);

  graphics_context_set_fill_color(ctx, ACCENT_COLOR);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_text_color(ctx, GColorWhite);
  GRect text_rect = GRect(6, 1, bounds.size.w - 12, 20);
  graphics_draw_text(ctx, "La Liga - Matchday 30",
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     text_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);
  #endif
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  const MatchData *m = &s_matches[index->row];
  bool selected = menu_layer_get_selected_index(s_menu_layer).row == index->row;

  int16_t pad = PBL_IF_ROUND_ELSE(24, 6);
  int16_t y = PBL_IF_ROUND_ELSE(8, 4);
  int16_t w = bounds.size.w - pad * 2;
  GTextAlignment align = PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft);

  GFont team_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont status_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  #ifdef PBL_COLOR
  GColor text_color = GColorBlack;
  #else
  GColor text_color = selected ? GColorWhite : GColorBlack;
  #endif

  // Home team + score
  char home_line[24];
  snprintf(home_line, sizeof(home_line), "%s    %d", m->home, m->home_score);
  graphics_context_set_text_color(ctx, text_color);
  graphics_draw_text(ctx, home_line, team_font,
                     GRect(pad, y, w, 20),
                     GTextOverflowModeTrailingEllipsis, align, NULL);

  // Away team + score
  y += 18;
  char away_line[24];
  snprintf(away_line, sizeof(away_line), "%s    %d", m->away, m->away_score);
  graphics_draw_text(ctx, away_line, team_font,
                     GRect(pad, y, w, 20),
                     GTextOverflowModeTrailingEllipsis, align, NULL);

  // Status line
  y += 18;
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, m->is_live ? GColorRed : GColorDarkGray);
  #else
  graphics_context_set_text_color(ctx, text_color);
  #endif
  graphics_draw_text(ctx, m->status, status_font,
                     GRect(pad, y, w, 16),
                     GTextOverflowModeTrailingEllipsis, align, NULL);

  // Pulsing live dot
  if (m->is_live && s_dot_visible) {
    GSize status_size = graphics_text_layout_get_content_size(
        m->status, status_font, GRect(0, 0, w, 16),
        GTextOverflowModeTrailingEllipsis, align);
    int dot_x;
    #if PBL_ROUND
    dot_x = bounds.size.w / 2 + status_size.w / 2 + 8;
    #else
    dot_x = pad + status_size.w + 8;
    #endif
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
    #else
    graphics_context_set_fill_color(ctx, selected ? GColorWhite : GColorBlack);
    #endif
    graphics_fill_circle(ctx, GPoint(dot_x, y + 8), 3);
  }

  // Bottom separator
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  #else
  graphics_context_set_stroke_color(ctx, selected ? GColorWhite : GColorBlack);
  #endif
  graphics_draw_line(ctx, GPoint(pad, bounds.size.h - 1),
                     GPoint(bounds.size.w - pad, bounds.size.h - 1));
}

static void select_click(MenuLayer *menu, MenuIndex *index, void *data) {
  show_match_detail(index->row);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorWhite);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows,
    .get_cell_height = get_cell_height,
    .get_header_height = get_header_height,
    .draw_header = draw_header,
    .draw_row = draw_row,
    .select_click = select_click,
  });

  menu_layer_set_normal_colors(s_menu_layer, GColorWhite, GColorBlack);
  #ifdef PBL_COLOR
  menu_layer_set_highlight_colors(s_menu_layer, GColorMintGreen, GColorBlack);
  #else
  menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, GColorWhite);
  #endif

  #ifdef PBL_ROUND
  menu_layer_set_center_focused(s_menu_layer, true);
  #endif

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_pulse_timer = app_timer_register(500, pulse_timer_callback, NULL);
}

static void window_unload(Window *window) {
  if (s_pulse_timer) {
    app_timer_cancel(s_pulse_timer);
    s_pulse_timer = NULL;
  }
  menu_layer_destroy(s_menu_layer);
  window_destroy(s_window);
  s_window = NULL;
}

void result_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
