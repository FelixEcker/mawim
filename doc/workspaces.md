# MaWiM Workspaces
## Introduction
MaWiM supports workspaces through mawimctl. Workspaces are identified using an
ID starting at 1, going all the way to `UINT8_MAX`.

### Internal Representation
MaWiM stores workspaces using the `mawim_workspace_t` structure. This structure
holds a list of windows the workspace is managing, the window which is focused
on the workspace and information about which row is active and how many rows the
workspace currently has.

### Workspace Switching
When switching a workspace the function `mawim_activate_workspace()`, as defined
in `workspace.h`, is used. This function does the following:
    1. Check if the workspace id is valid and does not match the previously
       active workspace id.
    2. Get the previously active workspace and hide all of its windows.
    3. Update the active workspace id field in the `mawim_t` structure.

## Managing X11 Events
The management of X11 events becomes a bit more complicated with workspaces.
This section aims to document the basic steps taken for each event to which
workspaces are relevant or seem relevan.

### XDestroyWindowEvent
When a DestroyEvent/DestroyNotify occurs, MaWiM needs to get the workspace which
is managing the window being destroyed and the `mawim_window_t` strucure
representing the window. This can be done using
`mawim_find_window_in_workspace()`. The window can then be unmanaged and removed
from the workspaces window list using `mawim_unmanage_window()` and
`mawim_remove_window()` as defined in `window.h`+

### XConfigureRequest
When a configuration request occurs, MaWiM needs to check if the window
generating the request is already being managed in a workspace and if not assign
it to a workspace. This can be done using `mawim_find_window_in_workspace()` and
`mawim_append_window()`. After this initial step all that needs to be done it to
manage the window, which can be done through `mawim_manage_window()`.

### XLeaveWindowEvent
When a LeaveWindowEvent/LeaveNotify occurs, MaWiM needs to find on which window
the mouse was hovering when the event was generated. For this logic is already
implemented in the function `get_hovered_window` in `events.c`, which internally
just uses the window list of the currently active workspace for searching.

## Reference
### types.h
#### mawim_workspace_t
```c
typedef struct mawim_workspace {...} mawim_workspace_t;
```

This structure type represents a MaWiM workspace

* `window_list_t   windows;` List of windows being managed by the workspace
* `mawim_window_t *focused_window;` The window which has the focus on the workspace
* `int             active_row;` The active row of the workspace where windows should spawn
* `int             row_count;` The count of rows currently in use by the workspace.

### workspace.h
#### mawim_activate_workspace()
```c
void mawim_activate_workspace(mawim_t *mawim, mawimctl_workspaceid_t workspace);
```

Activates the specified workspace.

Parameters:
* mawim - Pointer to the mawim structure which contains the workspace.
* workspace - ID of the workspace to be activated.

#### mawim_update_workspace()
```c
void mawim_update_workspace(mawim_t *mawim, mawimctl_workspaceid_t workspace);
```

Update the specified workspace.

Parameters:
* mawim - Pointer to the mawim structure containing the workspace.
* workspace - ID of the workspace to be updated

#### mawim_update_workspaces()
```c
void mawim_update_workspaces(mawim_t *mawim);
```

Updates all workspaces.

Parameters:
* mawim - Poiner to the mawim structure containing the workspaces to be updated.

#### mawim_find_window_in_workspaces()
```c
mawim_window_t *mawim_find_window_in_workspaces(mawim_t, Window x11_window, window_list_t **out_window_list, mawim_workspaceid_t *out_workspaceid);
```

 Searches all workspaces for the given X11 window.

 Parameters:
 * out_window_list - If not NULL, the window list containing the window will be written to it.
 * out_workspaceid - If not NULL, the ID of the workspace containing the window will be written to it.

 Returns:
* The pointer to the mawim_window structure representing the given X11 window.
* NULL if no matching mawim_window structure was found.
