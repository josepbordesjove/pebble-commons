# pebble-commons

Shared C source library for Pebble smartwatch apps (SDK v3). Not a compiled library — consumers include the source files via git submodule at `src/c/commons/`.

## Structure

- `src/` — Reusable C modules (options_menu, progress_layer, scroll_text_layer, card_pager_layer)
- `resources/images/` — Weather icons organized by size (30x30 color, 30x30_bw for B&W)
- `examples/splash_window.c` — Reference splash screen template (not compiled)
- `examples/demo-app/` — Complete Pebble app showcasing all 4 modules and weather icons

## Key Constraints

- Pebble SDK v3 WAF build system only compiles source — no precompiled library linking
- The default wscript glob `src/c/**/*.c` auto-picks up submodule source
- All code must handle 6 platforms via `#ifdef PBL_ROUND`, `#ifdef PBL_COLOR`, `#ifdef PBL_PLATFORM_EMERY`
- Only one options_menu instance can be active at a time (static globals, single-threaded)
