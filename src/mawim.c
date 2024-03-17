#include "mawim.h"

#include "x_redefs.h"

#include <stdio.h>
#include <stdlib.h>

void mawim_panic(char *msg) {
  fprintf(stderr, "%s", msg);
  exit(EXIT_FAILURE);
}

void mawim_x11_flush(mawim_t *mawim) {
  XSync(mawim->display, false);
}

void mawim_x11_discarding_flush(mawim_t *mawim) {
  XSync(mawim->display, true);
}

void mawim_x11_init(mawim_t *mawim) {
  mawim->display = XOpenDisplay(XNULL);
  if (mawim->display == NULL) {
    mawim_panic("Could not open a X display!\n");
  }

  mawim->root_window = DefaultRootWindow(mawim->display);

  /* Cursor Setup */
  mawim->cursor = XCreateFontCursor(mawim->display, XC_left_ptr);
  XDefineCursor(mawim->display, mawim->root_window, mawim->cursor);
  XSync(mawim->display, false);

  /* Input Setup */
  XGrabButton(mawim->display, Button1, 0, mawim->root_window, 0,
              ButtonPressMask, GrabModeSync, GrabModeAsync, XNULL, XNULL);
}

void mawim_x11_shutdown(mawim_t *mawim) {
  XCloseDisplay(mawim->display);
}

int main(void) {
  fprintf(stderr, "Running MaWiM!\n");

  mawim_t mawim;

  mawim_x11_init(&mawim);

  XEvent event;
  while (true) {
    fprintf(stdout, "AJOSDJP\n");
    XNextEvent(mawim.display, &event);

    switch (event.type) {
    case ButtonPress:
      fprintf(stdout, "Button Press!\n");
      XAllowEvents(mawim.display, ReplayPointer, CurrentTime);
      XSync(mawim.display, 0);
      break;
    }
  }

  mawim_x11_shutdown(&mawim);

  fprintf(stderr, "MaWiM: Goodbye!\n");
  return 0;
}
