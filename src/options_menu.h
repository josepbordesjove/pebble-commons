// pebble-commons: options_menu
// Reusable options menu with platform-adaptive UI for Pebble smartwatch apps.
// Rectangular displays use a MenuLayer with accent stripe; round displays use
// a custom canvas with animated scrolling and arc indicator.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <pebble.h>

// Maximum number of items in an options menu
#define OPTIONS_MENU_MAX_ITEMS 8

// Callback when an option is selected
typedef void (*OptionsMenuSelectCallback)(int index, void *context);

// A single menu option
typedef struct {
  const char *title;
  OptionsMenuSelectCallback callback;
} OptionsMenuItem;

// Configuration for the options menu
typedef struct {
  OptionsMenuItem items[OPTIONS_MENU_MAX_ITEMS];
  int num_items;
  GColor accent_color;
  void *callback_context;
} OptionsMenuConfig;

// Push an options menu window onto the stack
void options_menu_push(OptionsMenuConfig *config);

// Update the title of an item (e.g. for toggles). Call after push.
void options_menu_update_title(int index, const char *title);
