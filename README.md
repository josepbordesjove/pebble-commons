# pebble-commons

Shared UI components and resources for [Pebble](https://developer.repebble.com/) smartwatch apps. Built for Pebble SDK v3 and compatible with all six hardware platforms.

## Modules

### Options Menu

A platform-adaptive options menu with a polished, native feel.

- **Rectangular displays** (Aplite, Basalt, Diorite, Emery): Uses `MenuLayer` with a colored accent stripe and animated dot indicator
- **Round displays** (Chalk, Gabbro): Custom canvas with animated scrolling, arc accent, proximity-based text sizing, and dot tracking
- Supports up to 8 menu items
- Configurable accent color
- Dynamic title updates (useful for toggles)

### Progress Layer

A lightweight, configurable progress bar widget.

- Percentage-based progress (0-100) with clamping
- Customizable foreground/background colors and corner radius
- Border outline for clean rendering on both color and B&W displays
- Increment support for step-based progress

### Scroll Text Layer

A text layer that automatically scrolls (marquee-style) when the text overflows its frame.

- Bounce-scroll animation: text slides left to reveal the end, pauses, then slides back
- Configurable scroll speed, pause duration, and animation interval
- Automatic overflow detection — static text when it fits, scrolling when it doesn't
- Built-in background masking at both edges
- Start/stop control for visibility-dependent scrolling

### Card Pager Layer

A horizontal paged card view with dot indicators and slide-in/slide-out transitions.

- UP/DOWN button navigation between pages
- Ease-out slide animation (~200ms) with the content layer moving horizontally
- Static page indicator dots centered at the bottom (current dot larger, inactive dots smaller)
- Configurable dot colors, page count, and draw callback
- Content drawn via a user-provided callback per page index

### Resources

Pre-made weather icons in two sizes:

| Directory | Size | Platforms | Description |
|-----------|------|-----------|-------------|
| `resources/images/30x30/` | 30x30 | Basalt, Chalk, Emery, Gabbro | Color weather icons |
| `resources/images/30x30_bw/` | 30x30 | Aplite, Diorite | B&W weather icons |

Available icons: `weather_sunny`, `weather_partly_cloudy`, `weather_cloudy`, `weather_light_rain`, `weather_heavy_rain`, `weather_snow`, `weather_rain_snow`, `weather_generic`.

## Installation

Add pebble-commons as a git submodule inside your project's `src/c/` directory:

```bash
cd your-pebble-app/
git submodule add git@github.com:josepbordesjove/pebble-commons.git src/c/commons
```

The default `wscript` glob (`src/c/**/*.c`) will automatically compile the commons source files. No build configuration changes needed.

### Including in your code

From any file under `src/c/`, use relative includes:

```c
// From src/c/modules/my_module.c
#include "../commons/options_menu.h"
#include "../commons/progress_layer.h"

// From src/c/main.c
#include "commons/options_menu.h"
#include "commons/progress_layer.h"
```

### Using weather icons

Copy the icons you need into your project's `resources/images/` directory, then add entries to `package.json`:

```json
{
  "pebble": {
    "resources": {
      "media": [
        {
          "type": "png",
          "name": "WEATHER_SUNNY",
          "file": "images/weather_sunny.png",
          "targetPlatforms": ["basalt", "chalk", "emery", "gabbro"]
        },
        {
          "type": "png",
          "name": "WEATHER_SUNNY",
          "file": "images/weather_sunny_bw.png",
          "targetPlatforms": ["aplite", "diorite"]
        }
      ]
    }
  }
}
```

### Cloning a project that uses pebble-commons

```bash
git clone --recurse-submodules <your-app-repo>
# Or if already cloned:
git submodule update --init --recursive
```

## API Reference

### Options Menu

```c
#include "commons/options_menu.h"
```

**Types:**

```c
// Callback signature for menu item selection
typedef void (*OptionsMenuSelectCallback)(int index, void *context);

// A single menu item
typedef struct {
  const char *title;                    // Display text
  OptionsMenuSelectCallback callback;   // Called on selection
} OptionsMenuItem;

// Menu configuration
typedef struct {
  OptionsMenuItem items[OPTIONS_MENU_MAX_ITEMS];  // Up to 8 items
  int num_items;                                   // Number of active items
  GColor accent_color;                             // Stripe/arc color
  void *callback_context;                          // Passed to callbacks
} OptionsMenuConfig;
```

**Functions:**

| Function | Description |
|----------|-------------|
| `void options_menu_push(OptionsMenuConfig *config)` | Push the options menu onto the window stack |
| `void options_menu_update_title(int index, const char *title)` | Update an item's title after the menu is displayed |

**Example:**

```c
static void on_refresh(int index, void *context) {
  // Handle refresh action
  window_stack_pop(true);
}

static void on_settings(int index, void *context) {
  // Handle settings action
  window_stack_pop(true);
}

static void show_options(void) {
  OptionsMenuConfig config = {
    .items = {
      { .title = "Refresh data", .callback = on_refresh },
      { .title = "Settings",     .callback = on_settings },
    },
    .num_items = 2,
    .accent_color = GColorVividCerulean,
    .callback_context = NULL,
  };
  options_menu_push(&config);
}
```

### Progress Layer

```c
#include "commons/progress_layer.h"
```

**Functions:**

| Function | Description |
|----------|-------------|
| `ProgressLayer *progress_layer_create(GRect frame)` | Create a progress layer with the given frame |
| `void progress_layer_destroy(ProgressLayer *layer)` | Destroy the progress layer |
| `Layer *progress_layer_get_layer(ProgressLayer *layer)` | Get the underlying `Layer` for adding to a parent |
| `void progress_layer_set_progress(ProgressLayer *layer, int16_t progress)` | Set progress (0-100, clamped) |
| `void progress_layer_increment_progress(ProgressLayer *layer, int16_t amount)` | Increment progress by the given amount |
| `void progress_layer_set_corner_radius(ProgressLayer *layer, uint16_t radius)` | Set corner radius (default: 2) |
| `void progress_layer_set_foreground_color(ProgressLayer *layer, GColor color)` | Set the fill color (default: `GColorBlack`) |
| `void progress_layer_set_background_color(ProgressLayer *layer, GColor color)` | Set the background color (default: `GColorWhite`) |

**Example:**

```c
static ProgressLayer *s_progress;

// In window_load:
s_progress = progress_layer_create(GRect(10, 140, 124, 6));
progress_layer_set_foreground_color(s_progress, GColorVividCerulean);
progress_layer_set_corner_radius(s_progress, 3);
layer_add_child(window_layer, progress_layer_get_layer(s_progress));

// Update progress:
progress_layer_set_progress(s_progress, 75);

// In window_unload:
progress_layer_destroy(s_progress);
```

### Scroll Text Layer

```c
#include "commons/scroll_text_layer.h"
```

**Functions:**

| Function | Description |
|----------|-------------|
| `ScrollTextLayer *scroll_text_layer_create(GRect frame)` | Create a scroll text layer with the given frame |
| `void scroll_text_layer_destroy(ScrollTextLayer *layer)` | Destroy the scroll text layer |
| `Layer *scroll_text_layer_get_layer(ScrollTextLayer *layer)` | Get the underlying `Layer` for adding to a parent |
| `void scroll_text_layer_set_text(ScrollTextLayer *layer, const char *text)` | Set text (auto-starts scrolling if it overflows) |
| `void scroll_text_layer_set_font(ScrollTextLayer *layer, GFont font)` | Set font (default: `FONT_KEY_GOTHIC_14_BOLD`) |
| `void scroll_text_layer_set_text_color(ScrollTextLayer *layer, GColor color)` | Set text color (default: `GColorBlack`) |
| `void scroll_text_layer_set_background_color(ScrollTextLayer *layer, GColor color)` | Set background mask color (default: `GColorWhite`) |
| `void scroll_text_layer_set_speed(ScrollTextLayer *layer, int speed)` | Set scroll speed in px/tick (default: 1) |
| `void scroll_text_layer_set_pause_ticks(ScrollTextLayer *layer, int ticks)` | Set pause at each end in ticks (default: 50, ~2s) |
| `void scroll_text_layer_set_interval(ScrollTextLayer *layer, int interval_ms)` | Set timer interval in ms (default: 40, ~25fps) |
| `void scroll_text_layer_start(ScrollTextLayer *layer)` | Start or restart scrolling |
| `void scroll_text_layer_stop(ScrollTextLayer *layer)` | Stop scrolling and reset to start |
| `bool scroll_text_layer_is_scrolling(ScrollTextLayer *layer)` | Check if the layer is currently scrolling |

**Example:**

```c
static ScrollTextLayer *s_scroll;

// In window_load:
s_scroll = scroll_text_layer_create(GRect(10, 50, 124, 20));
scroll_text_layer_set_font(s_scroll, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
scroll_text_layer_set_text_color(s_scroll, GColorWhite);
scroll_text_layer_set_background_color(s_scroll, GColorBlack);
scroll_text_layer_set_text(s_scroll, "This is a very long airline name that overflows");
layer_add_child(window_layer, scroll_text_layer_get_layer(s_scroll));

// In window_unload:
scroll_text_layer_destroy(s_scroll);
```

### Card Pager Layer

```c
#include "commons/card_pager_layer.h"
```

**Types:**

```c
// Callback to draw a page
typedef void (*CardPagerDrawCallback)(GContext *ctx, GRect bounds, int page, void *context);
```

**Functions:**

| Function | Description |
|----------|-------------|
| `CardPagerLayer *card_pager_layer_create(GRect frame, int num_pages)` | Create a pager with the given frame and page count |
| `void card_pager_layer_destroy(CardPagerLayer *pager)` | Destroy the pager |
| `Layer *card_pager_layer_get_layer(CardPagerLayer *pager)` | Get the underlying `Layer` for adding to a parent |
| `void card_pager_layer_set_draw_callback(CardPagerLayer *pager, CardPagerDrawCallback cb, void *ctx)` | Set the page draw callback |
| `void card_pager_layer_set_num_pages(CardPagerLayer *pager, int num_pages)` | Set page count (resets to page 0) |
| `int card_pager_layer_get_current_page(CardPagerLayer *pager)` | Get current page index |
| `void card_pager_layer_set_current_page(CardPagerLayer *pager, int page, bool animated)` | Jump to a page |
| `bool card_pager_layer_next(CardPagerLayer *pager)` | Go to the next page (animated) |
| `bool card_pager_layer_previous(CardPagerLayer *pager)` | Go to the previous page (animated) |
| `void card_pager_layer_set_active_dot_color(CardPagerLayer *pager, GColor color)` | Set current-page dot color |
| `void card_pager_layer_set_inactive_dot_color(CardPagerLayer *pager, GColor color)` | Set other dots' color |
| `void card_pager_layer_mark_dirty(CardPagerLayer *pager)` | Trigger a content redraw |
| `bool card_pager_layer_is_animating(CardPagerLayer *pager)` | Check if a transition is running |

**Example:**

```c
static CardPagerLayer *s_pager;

static void draw_page(GContext *ctx, GRect bounds, int page, void *context) {
  const char *titles[] = { "Route", "Weather", "Details" };
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, titles[page],
                     fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                     GRect(0, 50, bounds.size.w, 30),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);
}

// In window_load:
s_pager = card_pager_layer_create(bounds, 3);
card_pager_layer_set_draw_callback(s_pager, draw_page, NULL);
layer_add_child(window_layer, card_pager_layer_get_layer(s_pager));

// In button handlers:
// card_pager_layer_next(s_pager);
// card_pager_layer_previous(s_pager);

// In window_unload:
card_pager_layer_destroy(s_pager);
```

## Platform Support

All modules are tested and work on every Pebble hardware platform:

| Platform | Model | Resolution | Shape | Color |
|----------|-------|------------|-------|-------|
| `aplite` | Pebble (Original) | 144x168 | Rectangular | B&W |
| `basalt` | Pebble Time | 144x168 | Rectangular | 64 colors |
| `chalk` | Pebble Time Round | 180x180 | Round | 64 colors |
| `diorite` | Pebble 2 | 144x168 | Rectangular | B&W |
| `emery` | Pebble Time 2 | 200x228 | Rectangular | 64 colors |
| `gabbro` | Pebble Time 2 Round | 228x228 | Round | 64 colors |

Platform-specific rendering is handled automatically via compile-time flags (`PBL_COLOR`, `PBL_ROUND`, `PBL_PLATFORM_*`).

## Examples

The `examples/` directory contains reference implementations:

- **`splash_window.c`** — A minimal splash screen template with timer-based auto-advance, bitmap icon, and platform-adaptive colors. Copy into your project and customize the marked `// CUSTOMIZE:` sections.

- **`demo-app/`** — A complete Pebble app that showcases all pebble-commons components. It displays a main menu listing each module; selecting one opens an interactive demo:
  - **Options Menu**: Shows a 4-item menu with accent color, animated selection, and a live toggle ("Dark mode: ON/OFF")
  - **Progress Layer**: Interactive progress bar — UP/DOWN to adjust manually, SELECT to toggle auto-fill animation
  - **Scroll Text**: Three scroll text layers demonstrating different speeds and text lengths
  - **Card Pager**: Three-page horizontal card view with slide animations and dot indicators
  - **Weather Icons**: Grid view of all 8 bundled weather icons with labels
  - **Match Result**: Sports score list with pulsing live indicator and match detail on select
  - **Weather View**: Two-column layout with temperature, wind, and large centered icon

### Running the demo app

**Prerequisites:** [Pebble SDK](https://developer.repebble.com/sdk/install/index.html) installed and available in your PATH.

```bash
cd examples/demo-app
pebble build
```

This builds for all 6 platforms (aplite, basalt, chalk, diorite, emery, gabbro). Then install on any emulator:

```bash
# Color rectangular (Pebble Time)
pebble install --emulator basalt

# Color round (Pebble Time Round)
pebble install --emulator chalk

# B&W rectangular (Pebble 2)
pebble install --emulator diorite

# Large color rectangular (Pebble Time 2)
pebble install --emulator emery

# Large color round (Pebble Time 2 Round)
pebble install --emulator gabbro
```

View logs while running:

```bash
pebble logs --emulator basalt
```

The demo app symlinks `src/c/commons/` to the library's `src/` directory, so the wscript glob compiles the library source automatically alongside the demo code.

## Project Structure

```
pebble-commons/
├── LICENSE
├── README.md
├── src/
│   ├── options_menu.h
│   ├── options_menu.c
│   ├── progress_layer.h
│   ├── progress_layer.c
│   ├── scroll_text_layer.h
│   ├── scroll_text_layer.c
│   ├── card_pager_layer.h
│   └── card_pager_layer.c
├── resources/
│   └── images/
│       ├── 30x30/          # Color weather icons
│       └── 30x30_bw/       # B&W weather icons
└── examples/
    ├── splash_window.c
    └── demo-app/           # Full runnable demo app
        ├── package.json
        ├── wscript
        └── src/c/
            ├── main.c
            └── windows/
                ├── splash_window.c
                ├── options_demo_window.c
                ├── progress_demo_window.c
                ├── scroll_demo_window.c
                ├── pager_demo_window.c
                ├── icons_demo_window.c
                ├── result_demo_window.c
                └── weather_demo_window.c
```

## Acknowledgments

Thanks to [Pebble](https://developer.repebble.com/) and the [Rebble](https://rebble.io/) community for keeping the platform alive. Most of the code in this library is inspired by patterns found in the official Pebble SDK examples and refined with the help of [Claude](https://claude.ai), Anthropic's AI assistant.

## License

[MIT](LICENSE)
