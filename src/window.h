/* window.h ; MaWiM Window Management and Window-List
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "types.h"

/* window operations */

/**
 * @brief Create a heap allocated window
 * @param win The x11 window to be associated with the new mawim_window
 * @param x The X coordinate of the window
 * @param y The Y coordinate of the window
 * @param width The window's width
 * @param height The window's height
 * @return The newly allocated window
 */
mawim_window_t *mawim_create_window(Window win, int x, int y, int width,
                                    int height);

/**
 * @brief Updates a window's geometry and configures it
 * @param mawim The mawim instance
 * @param window The window to be updated
 */
void mawim_update_window(mawim_t *mawim, mawim_window_t *window);

/**
 * @brief Begin managing a window
 * @param mawim The mawim instance
 * @param window The window to be managed
 * @param event The ConfigureRequestEvent causing this management call
 * @return true on success
 */
bool mawim_manage_window(mawim_t *mawim, mawim_window_t *window);

/**
 * @brief Stop managing a window
 * @param mawim The mawim instance
 * @param window The window to be unmanaged
 */
void mawim_unmanage_window(mawim_t *mawim, mawim_window_t *window);

/**
 * @brief Update all the windows
 * @param mawim The mawim instance
 */
void mawim_update_all_windows(mawim_t *mawim);

/* window list operations */

/**
 * @brief Finds the provided x11 window in the provided window list
 * @param list The list to operate on
 * @param window The x11 to search with
 * @return NULL if no window was found, otherwise pointer to the heap-allocated
 * window
 */
mawim_window_t *mawim_find_window(window_list_t *list, Window window);

/**
 * @brief Finds all windows on the given row
 * @param list The list to operate on
 * @param workspace The workspace to search in
 * @param row The row where the windows are to be searched
 * @param dest (nullable) The destination mawim_window_t* array
 * @return Count of windows on the row in the specified workspace
 */
int mawim_get_wins_on_row(window_list_t *list, mawimctl_workspaceid_t workspace,
                          int row, mawim_window_t ***dest);

/**
 * @brief Appends the given mawim window to the list. The next field of the
 * passed window will be NULLed
 * @param list The list to operate on
 * @param mawim_window The window to be appended
 */
void mawim_append_window(window_list_t *list, mawim_window_t *mawim_window);

/**
 * @brief Removes the given window from the window list
 * @param windows The list to operate on
 * @param window The X11 window associated with the mawim_window_t structure to
 * be removed
 */
void mawim_remove_window(window_list_t *windows, Window window,
                         bool should_free);

/**
 * @brief Destroys the given window list
 * @param list The list to destroy
 */
void mawim_destroy_window_list(window_list_t *list);

#endif /* #ifndef WINDOW_H */
