#include "error.h"

#include "logging.h"
#include "xmem.h"

#include <X11/Xlib.h>

int mawim_x11_error_handler(Display *display, XErrorEvent *error) {
  char *errtext = xmalloc(ERRTEXT_BUFF_SIZE);
  XGetErrorText(display, error->error_code, errtext, ERRTEXT_BUFF_SIZE);

  mawim_logf(LOG_ERROR, "Received X11 error: %s\n", errtext);

  xfree(errtext);

  return 1;
}