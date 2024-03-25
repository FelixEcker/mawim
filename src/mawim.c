/* mawim.c ; MaWiM core
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "mawim.h"

#include "commands.h"
#include "error.h"
#include "events.h"
#include "logging.h"

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We do this define because otherwise nanosleep(); is not available for us */
#define __USE_POSIX199309
#include <time.h>

void mawim_x11_flush(mawim_t *mawim) { XSync(mawim->display, false); }

void mawim_x11_discarding_flush(mawim_t *mawim) { XSync(mawim->display, true); }

void mawim_x11_init(mawim_t *mawim) {
  mawim->display = XOpenDisplay(XNULL);
  if (mawim->display == NULL) {
    mawim_panic("Could not open a X display!\n");
  }

  XSetErrorHandler(mawim_x11_error_handler);

  mawim->root = DefaultRootWindow(mawim->display);
  mawim->default_screen = DefaultScreen(mawim->display);

  /* Cursor Setup */
  mawim->cursor = XCreateFontCursor(mawim->display, XC_left_ptr);
  XDefineCursor(mawim->display, mawim->root, mawim->cursor);
  XSync(mawim->display, false);

  /* Input Setup */
  int mask = SubstructureRedirectMask | SubstructureNotifyMask |
             EnterWindowMask | LeaveWindowMask;

  XSelectInput(mawim->display, mawim->root, mask);
  mawim_x11_flush(mawim);

  XGrabButton(mawim->display, Button1, AnyModifier, mawim->root, 0,
              ButtonPressMask, GrabModeSync, GrabModeAsync, XNULL, XNULL);
  mawim_x11_flush(mawim);
}

void mawim_shutdown(mawim_t *mawim) {
  XCloseDisplay(mawim->display);
  mawimctl_server_stop(mawim->mawimctl);
}

int main(void) {
  mawim_log_level = DEFAULT_LOG_LEVEL;

  mawim_log(LOG_INFO, "Running MaWiM v" MAWIM_VERSION "\n");

  mawim_t mawim = {
      .windows.first = NULL,
      .windows.last = NULL,
      .max_cols = 2,
      .max_rows = 3,
      .active_row = 0,
      .row_count = 1,
      .workspace_count = 1,
      .active_workspace = 1,
  };

  mawim_x11_init(&mawim);

  mawim.mawimctl = mawimctl_server_start(getenv("MAWIMCTL_SOCK"));
  if (mawim.mawimctl == NULL) {
    mawim_panic("Failed to create mawimctl server!\n");
  }

  struct timespec sleep_time;
  sleep_time.tv_nsec = 1000 * 1000 * 100;
  sleep_time.tv_sec = 0;

  XEvent event;
  while (true) {
    /* Process X11 Events */
    while (XPending(mawim.display) > 0) {
      XNextEvent(mawim.display, &event);

      bool handled = mawim_handle_event(&mawim, event);
      if (!handled) {
        mawim_logf(LOG_WARNING, "got unexpected event: %s\n",
                   event_type_str[event.type]);
      }
    }

    /* Process mawimctl Events */
    mawimctl_server_update(mawim.mawimctl);

    mawimctl_command_t cmd;
    while (mawimctl_server_next_command(mawim.mawimctl, &cmd)) {
      mawim_logf(LOG_DEBUG, "Handling mawimctl command %d\n",
                 cmd.command_identifier);

      bool handled = mawim_handle_ctl_command(&mawim, cmd);
      if (!handled) {
        mawim_logf(LOG_WARNING, "got unexpected mawimctl command: %d\n",
                   cmd.command_identifier);
      }
    }

    nanosleep(&sleep_time, NULL);
  }

  mawim_shutdown(&mawim);

  fprintf(stderr, "MaWiM: Goodbye!\n");
  return 0;
}
