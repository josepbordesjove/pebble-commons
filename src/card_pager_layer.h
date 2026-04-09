// pebble-commons: card_pager_layer
// A horizontal paged card view with dot indicators and slide animations.
// Content slides left/right on UP/DOWN buttons while dots stay fixed at
// the bottom. Each page is drawn by a user-provided callback.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <pebble.h>

typedef struct CardPagerLayer CardPagerLayer;

// Callback to draw a page. Called with the page index and the drawing area.
typedef void (*CardPagerDrawCallback)(GContext *ctx, GRect bounds, int page, void *context);

// Create a card pager with the given frame and number of pages
CardPagerLayer *card_pager_layer_create(GRect frame, int num_pages);

// Destroy the card pager layer
void card_pager_layer_destroy(CardPagerLayer *pager);

// Get the underlying Layer for adding to a parent
Layer *card_pager_layer_get_layer(CardPagerLayer *pager);

// Set the draw callback for page content
void card_pager_layer_set_draw_callback(CardPagerLayer *pager,
                                        CardPagerDrawCallback callback,
                                        void *context);

// Set number of pages (resets to page 0)
void card_pager_layer_set_num_pages(CardPagerLayer *pager, int num_pages);

// Get current page index
int card_pager_layer_get_current_page(CardPagerLayer *pager);

// Navigate to a specific page (with animation)
void card_pager_layer_set_current_page(CardPagerLayer *pager, int page, bool animated);

// Navigate to the next/previous page (with animation). Returns true if navigated.
bool card_pager_layer_next(CardPagerLayer *pager);
bool card_pager_layer_previous(CardPagerLayer *pager);

// Set the active dot color (default: GColorBlack)
void card_pager_layer_set_active_dot_color(CardPagerLayer *pager, GColor color);

// Set the inactive dot color (default: GColorDarkGray on color, GColorBlack on B&W)
void card_pager_layer_set_inactive_dot_color(CardPagerLayer *pager, GColor color);

// Mark the content as dirty (triggers redraw)
void card_pager_layer_mark_dirty(CardPagerLayer *pager);

// Returns true if a page transition animation is in progress
bool card_pager_layer_is_animating(CardPagerLayer *pager);
