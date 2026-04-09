#include "options_demo_window.h"
#include "../commons/options_menu.h"

static bool s_dark_mode = false;

static void on_action_one(int index, void *context) {
  window_stack_pop(true);
}

static void on_action_two(int index, void *context) {
  window_stack_pop(true);
}

static void on_toggle_dark_mode(int index, void *context) {
  s_dark_mode = !s_dark_mode;
  options_menu_update_title(index, s_dark_mode ? "Dark mode: ON" : "Dark mode: OFF");
}

static void on_about(int index, void *context) {
  window_stack_pop(true);
}

void options_demo_window_push(void) {
  OptionsMenuConfig config = {
    .items = {
      { .title = "Refresh data",    .callback = on_action_one },
      { .title = "New search",      .callback = on_action_two },
      { .title = "Dark mode: OFF",  .callback = on_toggle_dark_mode },
      { .title = "About",           .callback = on_about },
    },
    .num_items = 4,
    .accent_color = PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite),
    .callback_context = NULL,
  };
  options_menu_push(&config);
}
