/* window.c ; MaWiM Window Management and Window-List
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "window.h"

#include "logging.h"
#include "mawim.h"
#include "mawimctl.h"
#include "types.h"
#include "xmem.h"

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
  if (window == NULL || window->row < 0 || window->col < 0) {
    return;
  }

  if (window->workspace != mawim->active_workspace) {
    XWithdrawWindow(mawim->display, window->x11_window, mawim->default_screen);
  } else {
    XMapWindow(mawim->display, window->x11_window);
  }

  mawim_workspace_t *workspace = &mawim->workspaces[window->workspace - 1];

  /* Calculate */
  int count = mawim_get_wins_on_row(&workspace->windows, window->workspace,
                                    window->row, NULL);

  mawim_logf(LOG_DEBUG, "active workspace: %d, window workspace: %d\n",
             mawim->active_workspace, window->workspace);

  window->width =
      DisplayWidth(mawim->display, mawim->default_screen) / max(count, 1);
  window->height = DisplayHeight(mawim->display, mawim->default_screen) /
                   workspace->row_count;
  window->x = window->width * window->col;
  window->y = window->height * window->row;

  mawim_logf(
      LOG_DEBUG,
      "New Dimensions: active row %d col %d row %d, size %dx%d, pos %dx%d\n",
      workspace->row_count, window->col, window->row, window->width,
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
/*  if (!window->managed) {
    return false;
  } */

  mawim_workspace_t *workspace = &mawim->workspaces[window->workspace - 1];

  if (mawim_find_window(&workspace->windows, window->x11_window) == NULL) {
    return false;
  }

  bool new_row = false;

  if (window->row < 0) {
    window->row = workspace->active_row;
    mawim_logf(LOG_DEBUG, "Attempting to manage on current active row %d\n",
               workspace->active_row);
    if (mawim_get_wins_on_row(&workspace->windows, window->workspace,
                              window->row, NULL) == mawim->max_rows) {
      int old = window->row;
      window->row = min(window->row + 1, mawim->max_rows - 1);

      /* New Row Created */
      new_row = old != window->row;

      if (new_row) {
        workspace->row_count++;
      }
    }
  }

  mawim_window_t **row_wins;
  int wins = mawim_get_wins_on_row(&workspace->windows, window->workspace,
                                   window->row, &row_wins);

  if (window->col < 0) {
    window->col = wins - 1;
  }

  mawim_update_window(mawim, window);

  if (!new_row) {
    for (int i = 0; i < wins; i++) {
      mawim_update_window(mawim, row_wins[i]);
    }
  } else {
    workspace->active_row = window->row;

    mawim_window_t *current = workspace->windows.first;
    mawim_update_window(mawim, current);

    while (current->next != NULL) {
      current = current->next;
      mawim_update_window(mawim, current);
    }
  }

  xfree(row_wins);

  return true;
}

void mawim_unmanage_window(mawim_t *mawim, mawim_window_t *window) {
  int oldrow = window->row;
  int oldcol = window->col;
  int oldworkspace = window->workspace;

  window->managed = false;

  /* Set row and col to -1 to avoid counting this window with
   * mawim_get_wins_on_row().
   */
  window->row = -1;
  window->col = -1;

  mawim_workspace_t *workspace = &mawim->workspaces[oldworkspace - 1];

  mawim_window_t **row_windows;
  int window_count = mawim_get_wins_on_row(&workspace->windows, oldworkspace,
                                           oldrow, &row_windows);

  if (window_count > 0) {
    /* Update Windows on Same Row */
    for (int ix = 0; ix < window_count; ix++) {
      if (row_windows[ix]->col > oldcol) {
        row_windows[ix]->col--;
      }
    }

    for (int ix = 0; ix < window_count; ix++) {
      if (row_windows[ix]->x11_window == window->x11_window) {
        continue;
      }
      mawim_update_window(mawim, row_windows[ix]);
    }

    xfree(row_windows);
  } else if (workspace->row_count > 1) {
    /* Move Windows which are a row below up one */

    xfree(row_windows);

    if ((workspace->row_count - 1) > oldrow) {
      for (int crow = oldrow + 1; crow < workspace->row_count; crow++) {
        window_count = mawim_get_wins_on_row(&workspace->windows, oldworkspace,
                                             crow, &row_windows);

        for (int cwin = 0; cwin < window_count; cwin++) {
          row_windows[cwin]->row--;
        }

        xfree(row_windows);
      }
    }

    workspace->row_count--;

    if ((workspace->active_row + 1) > workspace->row_count) {
      workspace->active_row = workspace->row_count - 1;
    }

    mawim_update_all_windows(mawim);
  }
}

void mawim_update_all_windows(mawim_t *mawim) {
  mawim_log(LOG_DEBUG, "Update ALL Windows!\n");

  for (mawimctl_workspaceid_t wid = 0; wid < mawim->workspace_count; wid++) {
    mawim_window_t *current = mawim->workspaces[wid].windows.first;
    if (current == NULL) {
      return;
    }

    mawim_update_window(mawim, current);

    while (current->next != NULL) {
      current = current->next;
      mawim_update_window(mawim, current);
    }
  }
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

int mawim_get_wins_on_row(window_list_t *list, mawimctl_workspaceid_t workspace,
                          int row, mawim_window_t ***dest) {
  if (list->first == NULL) {
    return 0;
  }

  mawim_window_t *current = list->first;
  int count = current->row == row ? 1 : 0;

  while (current->next != NULL) {
    current = current->next;
    if (current->row == row && current->workspace == workspace) {
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
    if (current->row == row && current->workspace == workspace) {
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

void mawim_remove_window(window_list_t *windows, Window window) {
  if (windows->first == NULL) {
    mawim_log(LOG_DEBUG, "remove_window: windows.first == NULL\n");
    return;
  }

  mawim_window_t *current = windows->first;
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

  if (current == windows->last) {
    windows->last = previous;
  }
  if (current == windows->first) {
    windows->first = current->next;
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
