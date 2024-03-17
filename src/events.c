#include "events.h"

#include "logging.h"

void handle_button_press(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got ButtonPress!\n");
  XAllowEvents(mawim->display, ReplayPointer, CurrentTime);
  mawim_x11_flush(mawim);
}

void handle_create_notify(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got CreateNotify!\n");
}

void handle_destroy_notify(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got DestroyNotify!\n");
}

void handle_reparent_notify(mawim_t *mawim, XEvent event) {
  mawim_log(LOG_DEBUG, "Got ReparentNotify!\n");
}

void handle_configure_request(mawim_t *mawim, XConfigureRequestEvent event) {
  mawim_log(LOG_DEBUG, "Got ConfigureRequest!\n");

  XWindowChanges changes;
  changes.x = event.x;
  changes.y = event.y;
  changes.width = event.width;
  changes.height = event.height;
  changes.border_width = event.border_width;
  changes.sibling = event.above;
  changes.stack_mode = event.detail;

  XConfigureWindow(mawim->display, event.window, event.value_mask, &changes);
  mawim_logf(
      LOG_DEBUG,
      "Configured Window 0x%08x to dimensions %dx%d at coordinates %dx%d\n",
      event.window, changes.width, changes.height, changes.x, changes.y);
}

void handle_map_request(mawim_t *mawim, XMapRequestEvent event) {
  mawim_log(LOG_DEBUG, "Got MapRequest!\n");

  XMapWindow(mawim->display, event.window);

  mawim_logf(LOG_DEBUG, "Mapped Window 0x%08x\n", event.window);
}

void handle_leave_notify(mawim_t *mawim, XLeaveWindowEvent event) {
  mawim_log(LOG_DEBUG, "Got LeaveNotify!\n");

  XSetInputFocus(mawim->display, event.window, RevertToPointerRoot, CurrentTime);
  mawim_x11_flush(mawim);

  mawim_logf(LOG_DEBUG, "Set input focus to window 0x%08x\n", event.window);
}

void handle_enter_notify(mawim_t *mawim, XEnterWindowEvent event) {
  mawim_log(LOG_DEBUG, "Got EnterNotify! (Not Handling)\n");
}

bool mawim_handle_event(mawim_t *mawim, XEvent event) {
  switch (event.type) {
  case ButtonPress:
    handle_button_press(mawim, event);
    return true;
  case CreateNotify:
    handle_create_notify(mawim, event);
    return true;
  case DestroyNotify:
    handle_destroy_notify(mawim, event);
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
