/* mawim.c ; MaWiM core
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "mawim.h"

#include "events.h"
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>

void mawim_panic(char *msg) {
  fprintf(stderr, "%s", msg);
  exit(EXIT_FAILURE);
}

void mawim_x11_flush(mawim_t *mawim) { XSync(mawim->display, false); }

void mawim_x11_discarding_flush(mawim_t *mawim) { XSync(mawim->display, true); }

void mawim_x11_init(mawim_t *mawim) {
  mawim->display = XOpenDisplay(XNULL);
  if (mawim->display == NULL) {
    mawim_panic("Could not open a X display!\n");
  }

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

void mawim_x11_shutdown(mawim_t *mawim) { XCloseDisplay(mawim->display); }

int main(void) {
  mawim_log_level = DEFAULT_LOG_LEVEL;

  mawim_log(LOG_INFO, "Running MaWiM v" MAWIM_VERSION "\n");

  mawim_t mawim;
  mawim.windows.first = NULL;
  mawim.windows.last = NULL;
  mawim.max_cols = 2;
  mawim.max_rows = 3;
  mawim.active_row = 0;

  mawim_x11_init(&mawim);

  XEvent event;
  while (true) {
    XNextEvent(mawim.display, &event);

    bool handled = mawim_handle_event(&mawim, event);
    if (!handled) {
      mawim_logf(LOG_WARNING, "got unexpected event: %s\n",
                 event_type_str[event.type]);
    }
  }

  mawim_x11_shutdown(&mawim);

  fprintf(stderr, "MaWiM: Goodbye!\n");
  return 0;
}
