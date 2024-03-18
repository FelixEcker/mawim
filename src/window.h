/* window.h ; Window and Window-List header
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "xbool.h"

#include <X11/Xlib.h>

typedef struct mawim_window mawim_window_t;

/* clang-format off */

typedef struct mawim_window {
  mawim_window_t *next;

  /* X11 */
  Window x11_window;
  
  /* Metadata */
  bool managed;
  
  /* Positioning */
  int x;
  int y;
  int width;
  int height;
} mawim_window_t;

typedef struct window_list {
  size_t          window_count;
  mawim_window_t *first;
  mawim_window_t *last;
} window_list_t;

/* clang-format on */

/**
 * @brief Finds the provided x11 window in the provided window list
 */
mawim_window_t *mawim_find_window(window_list_t *list, Window window);

/**
 * @brief Checks if the given x11 window is currently being managed
 */
bool mawim_is_window_managed(window_list_t *list, Window window);

/**
 * @brief Appends the given mawim window to the list
 */
void mawim_append_window(window_list_t *list, mawim_window_t *mawim_window);

/**
 * @brief Removes the given window from the window list
 */
void mawim_remove_window(window_list_t *list, Window window);

/**
 * @brief Destroys the given window list
 */
void mawim_destroy_window_list(window_list_t *list);

#endif /* #ifndef WINDOW_H */
