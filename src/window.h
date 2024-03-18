/* window.h ; MaWiM Window Management and Window-List
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "mawim.h"
#include "types.h"

/* window operations */

mawim_window_t *mawim_create_window(Window win, int x, int y, int width,
                                    int height, bool managed);

/**
 * @brief Updates a window's geometry and configures it
 */
void mawim_update_window(mawim_t *mawim, mawim_window_t *window);

/**
 * @brief Begin managing a window
 */
bool mawim_manage_window(mawim_t *mawim, mawim_window_t *window,
                         XConfigureRequestEvent event);

void mawim_update_all_windows(mawim_t *mawim);

/**
 * @brief Stop managing a window
 */
void mawim_unmanage_window(mawim_t *mawim, mawim_window_t *window);

/* window list operations */

/**
 * @brief Finds the provided x11 window in the provided window list
 */
mawim_window_t *mawim_find_window(window_list_t *list, Window window);

/**
 * @brief Finds all windows on the given row
 */
int mawim_get_wins_on_row(window_list_t *list, int row, mawim_window_t ***dest);

/**
 * @brief Checks if the given x11 window is currently being managed
 */
bool mawim_is_window_managed(window_list_t *list, Window window);

/**
 * @brief Get the amount of windows currently registered with MaWiM
 */
int mawim_window_count(window_list_t *list);

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
