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

#endif /* #infdef WORKSPACE_H */
