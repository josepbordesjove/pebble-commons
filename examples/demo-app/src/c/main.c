#include <pebble.h>
#include "windows/options_demo_window.h"
#include "windows/progress_demo_window.h"
#include "windows/scroll_demo_window.h"
#include "windows/pager_demo_window.h"
#include "windows/icons_demo_window.h"

#define NUM_DEMOS 5

static Window *s_window;
static MenuLayer *s_menu_layer;

typedef struct {
  const char *title;
  const char *subtitle;
  void (*push_fn)(void);
} DemoEntry;

static const DemoEntry s_demos[NUM_DEMOS] = {
  { "Options Menu",   "Animated menu with accent", options_demo_window_push },
  { "Progress Layer", "Configurable progress bar", progress_demo_window_push },
  { "Scroll Text",    "Auto-scrolling marquee",    scroll_demo_window_push },
  { "Card Pager",     "Horizontal cards + dots",   pager_demo_window_push },
  { "Weather Icons",  "Bundled icon resources",    icons_demo_window_push },
};

static uint16_t get_num_rows(MenuLayer *menu, uint16_t section, void *data) {
  return NUM_DEMOS;
}

static int16_t get_header_height(MenuLayer *menu, uint16_t section, void *data) {
  return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
}

static void draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorVividCerulean);
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif

  GRect text_rect = GRect(PBL_IF_ROUND_ELSE(0, 5),
                           (bounds.size.h - 20) / 2,
                           bounds.size.w - 10, 20);
  graphics_draw_text(ctx, "pebble-commons",
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     text_rect, GTextOverflowModeTrailingEllipsis,
                     PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *index, void *data) {
  menu_cell_basic_draw(ctx, cell_layer,
                       s_demos[index->row].title,
                       s_demos[index->row].subtitle,
                       NULL);
}

static void select_click(MenuLayer *menu, MenuIndex *index, void *data) {
  if (index->row < NUM_DEMOS) {
    s_demos[index->row].push_fn();
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows,
    .get_header_height = get_header_height,
    .draw_header = draw_header,
    .draw_row = draw_row,
    .select_click = select_click,
  });

  #ifdef PBL_ROUND
  menu_layer_set_center_focused(s_menu_layer, true);
  #endif

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
