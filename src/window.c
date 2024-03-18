/* window.c ; MaWiM Window Management and Window-List
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "window.h"

#include "xmem.h"

#include <stdlib.h>

/* management operations */

void mawim_manage_window(mawim_t *mawim, mawim_window_t *window) {
  window->managed = true;
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

  return current;
}

bool mawim_is_window_managed(window_list_t *list, Window window) {
  mawim_window_t *mawim_window = mawim_find_window(list, window);

  if (mawim_window == NULL) {
    return false;
  }

  return mawim_window->managed;
}

void mawim_append_window(window_list_t *list, mawim_window_t *mawim_window) {
  mawim_window->next = NULL;

  if (list->first == NULL) {
    list->first = mawim_window;

    if (list->last != NULL) {
      mawim_window->next = list->last;
    } else {
      list->last = mawim_window;
    }

    return;
  }

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
