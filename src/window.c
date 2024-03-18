/* window.c ; MaWiM Window Management and Window-List
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "window.h"

#include "logging.h"
#include "xmem.h"

#include <math.h>
#include <stdlib.h>

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

/* management operations */

mawim_window_t *mawim_create_window(Window win, int x, int y, int width,
                                    int height, bool managed) {
  mawim_window_t *window = xmalloc(sizeof(mawim_window_t));
  window->x11_window = win;
  window->managed = managed;
  window->x = x;
  window->y = y;
  window->width = width;
  window->height = height;
  window->row = -1;
  window->col = -1;

  return window;
}

void mawim_update_window(mawim_t *mawim, mawim_window_t *window) {
  /* Calculate */
  int count = mawim_get_wins_on_row(&mawim->windows, window->row, NULL);

  window->width = DisplayWidth(mawim->display, mawim->default_screen) / max(count, 1);
  window->height = DisplayHeight(mawim->display, mawim->default_screen) /
                   (mawim->active_row + 1);
  window->x = window->width * window->col;
  window->y = window->height * window->row;

  mawim_logf(
      LOG_DEBUG,
      "New Dimensions: active row %d col %d row %d, size %dx%d, pos %dx%d\n",
      mawim->active_row + 1, window->col, window->row, window->width,
      window->height, window->x, window->y);

  /* Apply */

  XMoveResizeWindow(mawim->display, window->x11_window, window->x, window->y,
                    window->width, window->height);
}

bool mawim_manage_window(mawim_t *mawim, mawim_window_t *window,
                         XConfigureRequestEvent event) {
  if (!window->managed) {
    return false;
  }

  if (mawim_find_window(&mawim->windows, window->x11_window) == NULL) {
    return false;
  }

  bool new_row = false;

  if (window->row < 0) {
    window->row = mawim->active_row;
    if (mawim_get_wins_on_row(&mawim->windows, window->row, NULL) ==
        mawim->max_rows) {
      int old = window->row;
      window->row = min(window->row + 1, mawim->max_rows - 1);

      /* New Row Created */
      new_row = old != window->row;
    }
  }

  mawim_window_t *row_wins;
  int wins = mawim_get_wins_on_row(&mawim->windows, window->row, &row_wins);
  mawim_logf(LOG_DEBUG, "windows on row %d = %d out of %d\n", window->row, wins,
             mawim_window_count(&mawim->windows));
  if (window->col < 0) {
    window->col = wins - 1;
  }

  mawim_update_window(mawim, window);

  if (!new_row) {
    for (int i = 0; i < wins; i++) {
      mawim_update_window(mawim, &row_wins[i]);
    }
  } else {
    mawim->active_row = window->row;

    mawim_window_t *current = mawim->windows.first;
    mawim_update_window(mawim, current);

    while (current->next != NULL) {
      current = current->next;
      mawim_update_window(mawim, current);
    }
  }

  return true;
}

void mawim_update_all_windows(mawim_t *mawim) {
  mawim_window_t *current = mawim->windows.first;
  mawim_update_window(mawim, current);

  while (current->next != NULL) {
    current = current->next;
    mawim_update_window(mawim, current);
  }
}

void mawim_unmanage_window(mawim_t *mawim, mawim_window_t *window) {
  window->managed = false;
}

/* window list operations */

mawim_window_t *mawim_find_window(window_list_t *list, Window window) {
  if (list->first == NULL) {
    return NULL;
  }

  mawim_window_t *current = list->first;
  while (current->next != NULL && current->x11_window != window) {
    current = current->next;
  }

  return current->x11_window == window ? current : NULL;
}

int mawim_get_wins_on_row(window_list_t *list, int row, mawim_window_t **dest) {
  if (list->first == NULL) {
    return 0;
  }

  mawim_window_t *current = list->first;
  int count = current->row == row ? 1 : 0;

  while (current->next != NULL) {
    current = current->next;
    if (current->row == row) {
      count++;
    }
  }

  if (dest == NULL) {
    return count;
  }

  /* Loop a second time to get all the windows */

  *dest = xmalloc(sizeof(mawim_window_t) * count);
  current = list->first;

  int wix = 0;
  if (current->row == row) {
    dest[wix] = current;
    wix++;
  }

  while (current->next != NULL) {
    current = current->next;
    if (current->row == row) {
      dest[wix] = current;
      wix++;
    }
  }

  return count;
}

bool mawim_is_window_managed(window_list_t *list, Window window) {
  mawim_window_t *mawim_window = mawim_find_window(list, window);

  if (mawim_window == NULL) {
    return false;
  }

  return mawim_window->managed;
}

int mawim_window_count(window_list_t *list) {
  if (list->first == NULL) {
    return 0;
  }

  int count = 1;
  mawim_window_t *current = list->first;

  while (current->next != NULL) {
    count++;
    current = current->next;
  }

  return count;
}

void mawim_append_window(window_list_t *list, mawim_window_t *mawim_window) {
  mawim_log(LOG_DEBUG, "APPEND!\n");
  mawim_window->next = NULL;

  if (list->first == NULL) {
    list->first = mawim_window;
    list->last = mawim_window;

    return;
  }

  list->last->next = mawim_window;
  list->last = mawim_window;
}

void mawim_remove_window(window_list_t *list, Window window) {
  if (list->first == NULL) {
    return;
  }

  mawim_window_t *current = list->first;
  mawim_window_t *previous = NULL;

  while (current->next != NULL) {
    if (current->x11_window == window)
      break;

    previous = current;
    current = current->next;
  }

  if (current->x11_window != window) {
    return;
  }

  if (previous != NULL) {
    previous->next = current->next;
  }

  if (current == list->last) {
    list->last = previous;
  }
  if (current == list->first) {
    list->first = NULL;
  }

  xfree(current);
}

void mawim_destroy_window_list(window_list_t *list) {
  if (list->first == NULL) {
    return;
  }

  mawim_window_t *current = list->first;

  while (current->next != NULL) {
    mawim_window_t *next = current->next;
    xfree(current);
    current = next;
  }

  xfree(current);
}
