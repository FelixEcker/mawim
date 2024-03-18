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
  window->next = NULL;
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
  if (window == NULL) {
    return;
  }

  /* Calculate */
  int count = mawim_get_wins_on_row(&mawim->windows, window->row, NULL);

  window->width =
      DisplayWidth(mawim->display, mawim->default_screen) / max(count, 1);
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

  window->changes.x = window->x;
  window->changes.y = window->y;
  window->changes.width = window->width;
  window->changes.height = window->height;

  int mask = CWX | CWY | CWWidth | CWHeight;
  XConfigureWindow(mawim->display, window->x11_window, mask, &window->changes);
  mawim_x11_flush(mawim);
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

  mawim_window_t **row_wins;
  int wins = mawim_get_wins_on_row(&mawim->windows, window->row, &row_wins);
  mawim_logf(LOG_DEBUG, "windows on row %d = %d out of %d\n", window->row, wins,
             mawim_window_count(&mawim->windows));
  if (window->col < 0) {
    window->col = wins - 1;
  }

  mawim_update_window(mawim, window);

  if (!new_row) {
    for (int i = 0; i < wins; i++) {
      mawim_logf(LOG_DEBUG, "Update! 0x%08x%s\n", window->x11_window,
                 row_wins[i]->x11_window == window->x11_window ? " (own)" : "");
      mawim_update_window(mawim, row_wins[i]);
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

  xfree(row_wins);

  return true;
}

void mawim_update_all_windows(mawim_t *mawim) {
  mawim_log(LOG_DEBUG, "Update ALL Windows!\n");

  mawim_window_t *current = mawim->windows.first;
  if (current == NULL) {
    return;
  }

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

int mawim_get_wins_on_row(window_list_t *list, int row,
                          mawim_window_t ***dest) {
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

  *dest = xmalloc(sizeof(mawim_window_t *) * count);
  current = list->first;

  int wix = 0;
  if (current->row == row) {
    (*dest)[wix] = current;
    wix++;
  }

  while (current->next != NULL && wix < count) {
    current = current->next;
    if (current->row == row) {
      (*dest)[wix] = current;
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
  mawim_logf(LOG_DEBUG, "x11_window = 0x%08x\n", mawim_window->x11_window);
  mawim_window->next = NULL;

  if (list->first == NULL) {
    mawim_logf(LOG_DEBUG, "SET FIRST WINDOW (%p)\n", mawim_window);
    list->first = mawim_window;
    list->last = mawim_window;

    return;
  }

  list->last->next = mawim_window;
  list->last = mawim_window;
}

void mawim_remove_window(mawim_t *mawim, Window window) {
  if (mawim->windows.first == NULL) {
    mawim_log(LOG_DEBUG, "remove_window: windows.first == NULL\n");
    return;
  }

  mawim_window_t *current = mawim->windows.first;
  mawim_window_t *previous = NULL;

  mawim_logf(LOG_DEBUG, "current = %p\n", current);
  while (current->next != NULL) {
    if (current->x11_window == window)
      break;

    previous = current;
    current = current->next;
  }

  if (previous != NULL) {
    previous->next = current->next;
  }

  if (current == mawim->windows.last) {
    mawim->windows.last = previous;
  }
  if (current == mawim->windows.first) {
    mawim->windows.first = current->next;
  }

  /* Update Column index for windows on the row */
  mawim_window_t **row_wins;

  /* This causes a memory leak; too bad! */
  int wins = mawim_get_wins_on_row(&mawim->windows, current->row, &row_wins);

  for (int i = 0; i < wins; i++) {
    if (row_wins[i]->col > current->col) {
      row_wins[i]->col--;
    }
  }

  if (wins - 1 <= 0) {
    mawim_log(LOG_DEBUG, "Moving up 1 row\n");
    for (int nr = current->row + 1; nr < mawim->max_rows; nr++) {
      wins = mawim_get_wins_on_row(&mawim->windows, nr, &row_wins);

      for (int i = 0; i < wins; i++) {
        row_wins[i]->row--;
      }
    }
  }

  /* xfree(row_wins); For some Reason row_wins is a stack-address (???) */
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
