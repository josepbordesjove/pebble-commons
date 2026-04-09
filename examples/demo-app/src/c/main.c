#include <pebble.h>
#include "windows/splash_window.h"

#define NUM_DEMOS 5

static Window *s_window;
static MenuLayer *s_menu_layer;
static GBitmap *s_logo_bitmap;

// Forward declarations — defined after splash pushes main
static void options_demo_push(void);
static void progress_demo_push(void);
static void scroll_demo_push(void);
static void pager_demo_push(void);
static void icons_demo_push(void);

typedef struct {
  const char *title;
  const char *subtitle;
  void (*push_fn)(void);
} DemoEntry;

static const DemoEntry s_demos[NUM_DEMOS] = {
  { "Options Menu",   "Animated menu with accent", options_demo_push },
  { "Progress Layer", "Configurable progress bar", progress_demo_push },
  { "Scroll Text",    "Auto-scrolling marquee",    scroll_demo_push },
  { "Card Pager",     "Horizontal cards + dots",   pager_demo_push },
  { "Weather Icons",  "Bundled icon resources",    icons_demo_push },
};

// Include demo window headers here (after forward decls)
#include "windows/options_demo_window.h"
#include "windows/progress_demo_window.h"
#include "windows/scroll_demo_window.h"
#include "windows/pager_demo_window.h"
#include "windows/icons_demo_window.h"

static void options_demo_push(void) { options_demo_window_push(); }
static void progress_demo_push(void) { progress_demo_window_push(); }
static void scroll_demo_push(void) { scroll_demo_window_push(); }
static void pager_demo_push(void) { pager_demo_window_push(); }
static void icons_demo_push(void) { icons_demo_window_push(); }

static uint16_t get_num_rows(MenuLayer *menu, uint16_t section, void *data) {
  return NUM_DEMOS;
}

static int16_t get_header_height(MenuLayer *menu, uint16_t section, void *data) {
  return 48;
}

static void draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section, void *data) {
  if (!s_logo_bitmap) return;
  GRect bounds = layer_get_bounds(cell_layer);
  GSize logo_size = gbitmap_get_bounds(s_logo_bitmap).size;

  int16_t x = (bounds.size.w - logo_size.w) / 2;
  int16_t y = (bounds.size.h - logo_size.h) / 2;

  #ifdef PBL_COLOR
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  #else
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  #endif
  graphics_draw_bitmap_in_rect(ctx, s_logo_bitmap, GRect(x, y, logo_size.w, logo_size.h));
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

  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PEBBLE_LOGO);

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
  if (s_logo_bitmap) {
    gbitmap_destroy(s_logo_bitmap);
    s_logo_bitmap = NULL;
  }
}

void main_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void init(void) {
  splash_window_push();
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
