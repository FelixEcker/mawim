/* types.h ; MaWiM shared type definitions
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef TYPES_H
#define TYPES_H

#include <X11/Xlib.h>

#define true True
#define false False
#define XNULL 0

/* define or own bool type because we are cool like that */
typedef int bool;

/* clang-format off */

typedef struct mawim_window mawim_window_t;

typedef struct window_list {
  size_t          window_count;
  mawim_window_t *first;
  mawim_window_t *last;
} window_list_t;

typedef struct mawim_window {
  mawim_window_t *next;

  /* X11 */
  Window x11_window;
  XWindowChanges changes;
  
  /* Metadata */
  bool managed;
  
  /* Positioning */
  int x;
  int y;
  int width;
  int height;
} mawim_window_t;

typedef struct mawim {
  /* X11 */
  Display *display;
  int      default_screen;
  Window   root;
  Cursor   cursor;

  /* MaWiM */
  window_list_t windows;
} mawim_t;

/* clang-format on */

#endif /* #ifndef TYPES_H */
