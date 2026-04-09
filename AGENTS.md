# Project Overview

This is an open-source **C source library** for [Pebble](https://developer.repebble.com/) smartwatch apps, built with **Pebble SDK v3**. It provides reusable UI components that work across all Pebble hardware platforms. The library is consumed as source files via git submodule — the Pebble SDK does not support linking precompiled C libraries.

- **Language:** C (C99, Pebble SDK conventions)
- **Build System:** WAF (wscript), integrated via the Pebble SDK toolchain
- **Integration:** Git submodule at `src/c/commons/`, auto-compiled by the default wscript glob `src/c/**/*.c`
- **Platforms:** 6 hardware targets (see below)

This project was built with the help of [Claude](https://claude.ai), Anthropic's AI assistant. The code is inspired by patterns found in the official [Pebble SDK examples](https://developer.repebble.com/) and refined with Claude's assistance.

# Target Platforms

All modules support every Pebble hardware platform:

| Platform | Model | Screen | Resolution | Shape | Color | Memory |
|----------|-------|--------|------------|-------|-------|--------|
| `aplite` | Pebble (Original) | Sharp Memory LCD | 144x168 | Rectangular | B&W (1-bit) | 24 KB |
| `basalt` | Pebble Time | Color e-Paper | 144x168 | Rectangular | 64 colors | 64 KB |
| `chalk` | Pebble Time Round | Color e-Paper | 180x180 | Round | 64 colors | 64 KB |
| `diorite` | Pebble 2 | Sharp Memory LCD | 144x168 | Rectangular | B&W (1-bit) | 64 KB |
| `emery` | Pebble Time 2 | Color e-Paper | 200x228 | Rectangular | 64 colors | 128 KB |
| `gabbro` | Pebble Time 2 Round | Color e-Paper | 228x228 | Round | 64 colors | 128 KB |

### Platform compile-time flags

```c
#ifdef PBL_COLOR          // Color display (basalt, chalk, emery, gabbro)
#ifdef PBL_BW             // B&W display (aplite, diorite)
#if PBL_ROUND             // Round display (chalk, gabbro)
#if !PBL_ROUND            // Rectangular display (aplite, basalt, diorite, emery)
#ifdef PBL_PLATFORM_APLITE
#ifdef PBL_PLATFORM_BASALT
#ifdef PBL_PLATFORM_CHALK
#ifdef PBL_PLATFORM_DIORITE
#ifdef PBL_PLATFORM_EMERY
#ifdef PBL_PLATFORM_GABBRO
```

# Architecture

## Module Structure

Each module follows a consistent pattern:

```
src/
  module_name.h    # Public API: types, function declarations
  module_name.c    # Implementation: static state, internal helpers, public functions
```

### API Design Conventions

- **Naming:** `module_name_action()` (e.g., `progress_layer_set_progress()`)
- **Create/Destroy:** Every module that allocates resources provides `_create()` and `_destroy()` functions
- **Layer access:** Modules wrapping a `Layer` provide `_get_layer()` to add to the view hierarchy
- **Header guards:** Use `#pragma once` (not `#ifndef` guards)
- **License headers:** Every source file includes `// SPDX-License-Identifier: MIT`

### Memory Management

- `_create()` allocates, `_destroy()` frees — always pair them
- Cancel all `AppTimer`s before destroying layers
- Set pointers to `NULL` after freeing
- Null-check in destroy functions for safety

### Platform Handling

- Use `#ifdef PBL_COLOR` / `#ifdef PBL_BW` for color-specific rendering
- Use `#if PBL_ROUND` / `#if !PBL_ROUND` for shape-specific layouts
- Use `PBL_IF_ROUND_ELSE(round_val, rect_val)` and `PBL_IF_COLOR_ELSE(color_val, bw_val)` for inline conditionals
- Always test on at least one rectangular B&W, one rectangular color, and one round platform

## Available Modules

| Module | Description | Files |
|--------|-------------|-------|
| `options_menu` | Platform-adaptive menu with accent stripe (rect) or animated arc (round) | `options_menu.h/c` |
| `progress_layer` | Configurable progress bar with colors and corner radius | `progress_layer.h/c` |
| `scroll_text_layer` | Auto-scrolling marquee text with bounce animation | `scroll_text_layer.h/c` |
| `card_pager_layer` | Horizontal paged cards with dot indicators and slide transitions | `card_pager_layer.h/c` |

## Resources

Weather icons are provided in `resources/images/`:
- `30x30/` — Color icons for basalt, chalk, emery, gabbro
- `30x30_bw/` — B&W icons for aplite, diorite

Icons must be manually added to each consuming app's `package.json` with `targetPlatforms`.

# Code Style

## C Conventions

- **Indentation:** 2 spaces, no tabs
- **Braces:** Opening brace on same line
- **Static globals:** Prefixed with `s_` (e.g., `static Window *s_window`)
- **Constants:** `#define` with UPPER_SNAKE_CASE (e.g., `#define ANIM_FRAMES 6`)
- **Types:** Use Pebble SDK types (`int16_t`, `uint16_t`, `GColor`, `GRect`, etc.)
- **Comments:** Use `//` for inline, `// ── Section ──` for visual separators

## File Organization

```c
// License header
// SPDX-License-Identifier: MIT

#include "module_name.h"        // Own header first
#include <pebble.h>             // SDK headers

// Private types (structs, enums)
// Static variables
// Static helper functions
// Public API implementations
```

## Pebble-Specific Patterns

- **Single-instance modules:** Static globals mean only one instance at a time (acceptable for single-threaded Pebble apps)
- **Timer-based animation:** Use `AppTimer` with `app_timer_register()` for frame-by-frame animation
- **Layer data storage:** Use `layer_create_with_data()` to attach custom data to layers
- **Compositing modes:** `GCompOpSet` for color displays, `GCompOpAssign` for B&W

# Build & Test

```bash
# Build for all platforms
pebble build

# Install and test on emulator
pebble install --emulator basalt
pebble install --emulator chalk
pebble install --emulator diorite
pebble install --emulator emery

# View logs
pebble logs --emulator basalt

# Run the demo app
cd examples/demo-app
pebble build
pebble install --emulator basalt
```

# Contributing

1. All new modules must support all 6 platforms
2. Follow the existing API naming conventions
3. Include `// SPDX-License-Identifier: MIT` in all source files
4. Add a demo screen in `examples/demo-app/` for new modules
5. Update `README.md` with API reference and examples
6. Test on at least basalt (color rect), chalk (color round), and diorite (B&W)
