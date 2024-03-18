#include "window.h"

#include <stdlib.h>

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

  free(current);
}

void mawim_destroy_window_list(window_list_t *list) {
  if (list->first == NULL) {
    return;
  }

  mawim_window_t *current = list->first;
  
  while (current->next != NULL) {
    mawim_window_t *next = current->next;
    free(current);
    current = next;
  }

  free(current);
}
