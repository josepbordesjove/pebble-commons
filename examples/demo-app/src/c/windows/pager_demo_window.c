#include "pager_demo_window.h"
#include "../commons/src/card_pager_layer.h"

#define NUM_PAGES 3

static Window *s_window;
static CardPagerLayer *s_pager;

static void draw_page(GContext *ctx, GRect bounds, int page, void *context) {
  int cx = bounds.size.w / 2;
  int cy = bounds.size.h / 2 - 10;

  // Page-specific accent color
  #ifdef PBL_COLOR
  GColor colors[] = { GColorVividCerulean, GColorMayGreen, GColorSunsetOrange };
  GColor accent = colors[page];
  #else
  GColor accent = GColorBlack;
  (void)page;
  #endif

  // Draw a centered icon-like circle
  int circle_r = PBL_IF_ROUND_ELSE(30, 25);
  #ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, accent);
  #else
  graphics_context_set_fill_color(ctx, GColorBlack);
  #endif
  graphics_fill_circle(ctx, GPoint(cx, cy - 10), circle_r);

  // Draw page number inside circle
  char num_buf[4];
  snprintf(num_buf, sizeof(num_buf), "%d", page + 1);
  graphics_context_set_text_color(ctx, GColorWhite);
  GRect num_rect = GRect(cx - 15, cy - 10 - 16, 30, 30);
  graphics_draw_text(ctx, num_buf,
                     fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                     num_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // Title below circle
  const char *titles[] = { "Route Info", "Weather", "Details" };
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, accent);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  GRect title_rect = GRect(0, cy + circle_r - 4, bounds.size.w, 28);
  graphics_draw_text(ctx, titles[page],
                     fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                     title_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // Subtitle
  const char *subs[] = {
    "UP/DOWN to navigate",
    "Slide animation",
    "Dot indicators below"
  };
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorDarkGray);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  GRect sub_rect = GRect(PBL_IF_ROUND_ELSE(30, 10), cy + circle_r + 22,
                          bounds.size.w - PBL_IF_ROUND_ELSE(60, 20), 20);
  graphics_draw_text(ctx, subs[page],
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     sub_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);
}

static void up_handler(ClickRecognizerRef recognizer, void *context) {
  card_pager_layer_previous(s_pager);
}

static void down_handler(ClickRecognizerRef recognizer, void *context) {
  card_pager_layer_next(s_pager);
}

static void click_config(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorWhite);

  s_pager = card_pager_layer_create(bounds, NUM_PAGES);
  card_pager_layer_set_draw_callback(s_pager, draw_page, NULL);

  #ifdef PBL_COLOR
  card_pager_layer_set_active_dot_color(s_pager, GColorVividCerulean);
  card_pager_layer_set_inactive_dot_color(s_pager, GColorLightGray);
  #endif

  layer_add_child(window_layer, card_pager_layer_get_layer(s_pager));
  window_set_click_config_provider(window, click_config);
}

static void window_unload(Window *window) {
  card_pager_layer_destroy(s_pager);
  window_destroy(s_window);
  s_window = NULL;
}

void pager_demo_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}
