/* workspace.h ; MaWiM Workspace Management
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "types.h"

/**
 * @brief Activates a workspace (hide all other workspaces, etc.)
 */
void mawim_activate_workspace(mawim_t *mawim, mawimctl_workspaceid_t workspace);

/**
 * @brief Update the specified workspace
 */
void mawim_update_workspace(mawim_t *mawim, mawimctl_workspaceid_t workspace);

/**
 * @brief Update all workspaces
 */
void mawim_update_workspaces(mawim_t *mawim);

/**
 * @brief searches all workspaces for the given X11 window.
 *
 * @param out_window_list If not NULL, the window list containing the window
 *                        will be written to it.
 * @param out_workspaceid If not NULL, the ID of the workspace containing the
 *                        window will be written to it.
 */
mawim_window_t *
mawim_find_window_in_workspaces(mawim_t, Window x11_window,
                                window_list_t **out_window_list,
                                mawim_workspaceid_t *out_workspaceid);

#endif /* #infdef WORKSPACE_H */
