/* events.c ; MaWiM Event Handling
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "events.h"

#include "logging.h"
#include "mawim.h"
#include "types.h"
#include "window.h"

const char *event_type_str[37] = {
    (char *)0,        (char *)1,        "KeyPress",         "KeyRelease",
    "ButtonPress",    "ButtonRelease",  "MotionNotify",     "EnterNotify",
    "LeaveNotify",    "FocusIn",        "FocusOut",         "KeymapNotify",
    "Expose",         "GraphicsExpose", "NoExpose",         "VisibilityNotify",
    "CreateNotify",   "DestroyNotify",  "UnmapNotify",      "MapNotify",
    "MapRequest",     "ReparentNotify", "ConfigureNotify",  "ConfigureRequest",
    "GravityNotify",  "ResizeRequest",  "CirculateNotify",  "CirculateRequest",
    "PropertyNotify", "SelectionClear", "SelectionRequest", "SelectionNotify",
    "ColormapNotify", "ClientMessage",  "MappingNotify",    "GenericEvent",
    "LASTEvent"};

void handle_button_press(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got ButtonPress!\n");
  XAllowEvents(mawim->display, ReplayPointer, CurrentTime);
  mawim_x11_flush(mawim);
}

void handle_create_notify(mawim_t *mawim, XCreateWindowEvent event) {
  mawim_log(LOG_DEBUG, "Got CreateNotify!\n");
}

void handle_destroy_notify(mawim_t *mawim, XDestroyWindowEvent event) {
  mawim_logf(LOG_DEBUG, "Got DestroyNotify (window 0x%08x)!\n", event.window);

  mawim_window_t *mawim_window =
      mawim_find_window(&mawim->windows, event.window);

  if (mawim_window != NULL) {
    mawim_unmanage_window(mawim, mawim_window);
    mawim_remove_window(mawim, event.window);
  } else {
    mawim_log(LOG_DEBUG, "Nothing to destroy!\n");
  }

  mawim_log(LOG_DEBUG, "DestroyNotify finished!\n");
}

void handle_reparent_notify(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got ReparentNotify!\n");
}

void handle_configure_request(mawim_t *mawim, XConfigureRequestEvent event) {
  mawim_log(LOG_DEBUG, "Got ConfigureRequest!\n");

  mawim_window_t *mawim_win = mawim_find_window(&mawim->windows, event.window);
  if (mawim_win == NULL) {
    mawim_logf(LOG_WARNING,
               "ConfigureRequest is for window 0x%08x which was not previously "
               "registered!\n",
               event.window);

    mawim_window_t *window = mawim_create_window(
        event.window, event.x, event.y, event.width, event.height, true);
    mawim_append_window(&mawim->windows, window);
    mawim_win = window;
  }

  bool manage_result = mawim_manage_window(mawim, mawim_win, event);
  if (manage_result) {
    mawim_log(LOG_DEBUG, "Window is being managed now!\n");
  } else {
    mawim_log(LOG_DEBUG, "Window is not being managed!\n");
  }

  mawim_logf(
      LOG_DEBUG,
      "Configured Window 0x%08x to dimensions %dx%d at coordinates %dx%d\n",
      event.window, mawim_win->changes.width, mawim_win->changes.height,
      mawim_win->changes.x, mawim_win->changes.y);
}

void handle_map_request(mawim_t *mawim, XMapRequestEvent event) {
  mawim_log(LOG_DEBUG, "Got MapRequest!\n");

  XMapWindow(mawim->display, event.window);

  mawim_logf(LOG_DEBUG, "Mapped Window 0x%08x\n", event.window);
}

Window get_hovered_window(mawim_t *mawim, int x, int y) {
  Window root;
  Window parent;
  Window *children;
  uint32_t nchildren;

  XQueryTree(mawim->display, mawim->root, &root, &parent, &children,
             &nchildren);

  Window match = root;
  for (uint32_t i = 0; i < nchildren; i++) {
    Window win = children[i];

    XWindowAttributes attribs;
    XGetWindowAttributes(mawim->display, win, &attribs);

    int wx = attribs.x;
    int wy = attribs.y;
    int right = attribs.x + wx;
    int bottom = attribs.y + wy;

    if (wx <= x && wy <= y && right >= x && bottom >= y) {
      match = win;
      break;
    }
  }

  return match;
}

void handle_leave_notify(mawim_t *mawim, XLeaveWindowEvent event) {
  mawim_log(LOG_DEBUG, "Got LeaveNotify!\n");

  Window window = get_hovered_window(mawim, event.x_root, event.y_root);

  XSetInputFocus(mawim->display, window, RevertToPointerRoot, CurrentTime);
  mawim_x11_flush(mawim);

  mawim->focused_window = mawim_find_window(&mawim->windows, window);
  if (mawim->focused_window == NULL) {
    mawim_log(LOG_WARNING, "newly focused window is not in window list!\n");
  }

  mawim_logf(LOG_DEBUG, "root window is 0x%08x\n", mawim->root);
  mawim_logf(LOG_DEBUG, "event.root is 0x%08x\n", event.root);
  mawim_logf(LOG_DEBUG, "Set input focus to window 0x%08x\n", window);
}

void handle_enter_notify(mawim_t *mawim, XEnterWindowEvent event) {
  mawim_log(LOG_DEBUG, "Got EnterNotify! (Not Handling)\n");
}

bool mawim_handle_event(mawim_t *mawim, XEvent event) {
  mawim_logf(LOG_DEBUG, "Processing Event: %d\n", event.type);
  switch (event.type) {
  case ButtonPress:
    handle_button_press(mawim, event);
    return true;
  case CreateNotify:
    handle_create_notify(mawim, event.xcreatewindow);
    return true;
  case DestroyNotify:
    handle_destroy_notify(mawim, event.xdestroywindow);
    return true;
  case ReparentNotify:
    handle_reparent_notify(mawim, event);
    return true;
  case ConfigureRequest:
    handle_configure_request(mawim, event.xconfigurerequest);
    return true;
  case MapRequest:
    handle_map_request(mawim, event.xmaprequest);
    return true;
  case LeaveNotify:
    handle_leave_notify(mawim, event.xcrossing);
    return true;
  case EnterNotify:
    handle_enter_notify(mawim, event.xcrossing);
    return true;
  default:
    return false;
  }

  return true;
}
