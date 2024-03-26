/* types.h ; MaWiM shared type definitions
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef TYPES_H
#define TYPES_H

#include "mawimctl_server.h"

#include <X11/Xlib.h>
#include <stdbool.h>

#define XNULL 0

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
  Window         x11_window;
  XWindowChanges changes;

  /* Metadata */
  bool                   managed;
  mawimctl_workspaceid_t workspace;

  /* Positioning */
  int x;
  int y;
  int width;
  int height;

  int row;
  int col;
  int cols_on_row;
} mawim_window_t;

typedef struct mawim_workspace {
  window_list_t   windows;
  mawim_window_t *focused_window;

  int active_row;
  int row_count;
} mawim_workspace_t;

typedef struct mawim {
  /* X11 */
  Display *display;
  int      default_screen;
  Window   root;
  Cursor   cursor;

  /* MaWiM */
  mawimctl_server_t *mawimctl;

  mawimctl_workspaceid_t workspace_count;
  mawimctl_workspaceid_t active_workspace;
  mawim_workspace_t *workspaces;

  /* Configuration */
  int max_cols;
  int max_rows;
} mawim_t;

/* clang-format on */

#endif /* #ifndef TYPES_H */
