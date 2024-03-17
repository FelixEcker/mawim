#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

void mawim_panic(char *msg) {
  fprintf(stderr, "%s", msg);
  exit(EXIT_FAILURE);
}

int main(void) {
  Display *display;
  Window root_window;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    mawim_panic("Could not open a X display!\n");
  }

  root_window = DefaultRootWindow(display);

  XCloseDisplay(display);

  return 0;
}
