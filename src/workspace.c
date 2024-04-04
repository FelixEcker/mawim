/* workspace.c ; MaWiM Workspace Management
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "workspace.h"

#include "mawimctl.h"
#include "window.h"

void mawim_activate_workspace(mawim_t *mawim,
                              mawimctl_workspaceid_t workspace) {}

void mawim_update_workspace(mawim_t *mawim, mawimctl_workspaceid_t workspace) {}

void mawim_update_workspaces(mawim_t *mawim) {}

mawim_window_t *
mawim_find_window_in_workspaces(mawim_t *mawim, Window x11_window,
                                window_list_t **out_window_list,
                                mawimctl_workspaceid_t *out_workspaceid) {
  for (mawimctl_workspaceid_t wid = 0; wid < mawim->workspace_count; wid++) {
    mawim_window_t *win =
        mawim_find_window(&mawim->workspaces[wid].windows, x11_window);
    if (win == NULL) {
      continue;
    }

    if (out_window_list != NULL) {
      *out_window_list = &mawim->workspaces[wid].windows;
    }

    if (out_workspaceid != NULL) {
      *out_workspaceid = wid;
    }

    return win;
  }
  return NULL;
}
