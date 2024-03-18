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

/* management operations */

/**
 * @brief Begin managing a window
 */
bool mawim_manage_window(mawim_t *mawim, mawim_window_t *window,
                         XConfigureRequestEvent event);

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
