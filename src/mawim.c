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
#include "types.h"
#include "xmem.h"

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We do this define because otherwise nanosleep(); is not available for us */
#define __USE_POSIX199309
#include <time.h>

void mawim_workspace_init(mawim_t *mawim) {
  if (mawim->workspaces == NULL) {
    mawim->workspaces =
        xmalloc(mawim->workspace_count * sizeof(*mawim->workspaces));
  } else {
    /* TODO: Reinit workspaces */
  }

  for (mawimctl_workspaceid_t wid = 0; wid < mawim->workspace_count; wid++) {
    mawim_workspace_t *workspace = &mawim->workspaces[wid];

    workspace->windows.first = NULL;
    workspace->windows.last = (mawim_window_t *)0xfeedface;
    workspace->focused_window = NULL;
    workspace->active_row = 0;
    workspace->row_count = 1;
  }
}

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

void help() {
  printf("   __  ___    _      ___ __  ___\n");
  printf("  /  |/  /__ | | /| / (_)  |/  /\n");
  printf(" / /|_/ / _ `/ |/ |/ / / /|_/ / \n");
  printf("/_/  /_/\\_,_/|__/|__/_/_/  /_/  \n");
  printf("\n");
  printf("v" MAWIM_VERSION "\n");
  printf("\t--help              Show this help text\n");
  printf("\t--verbosity=<0..3>  Specifies the log verbosity\n");
  printf("\n");
}

void parse_args(int argc, char **argv) {
  const char *ARG_VERBOSITY = "--verbosity=";
  const char *ARG_HELP = "--help";

  for (int i = 0; i < argc; i++) {
    if (strncmp(argv[i], ARG_VERBOSITY, strlen(ARG_VERBOSITY)) == 0) {
      log_level_t wanted = atoi(strchr(argv[i], '=') + 1);
      if (wanted > LOG_ERROR || wanted < LOG_DEBUG) {
        wanted = DEFAULT_LOG_LEVEL;
      }

      mawim_log_level = wanted;
      continue;
    }

    if (strncmp(argv[i], ARG_HELP, strlen(ARG_HELP)) == 0) {
      help();
      exit(0);
    }
  }
}

int main(int argc, char **argv) {
  mawim_log_level = DEFAULT_LOG_LEVEL;

  parse_args(argc, argv);

  mawim_log(LOG_INFO, "Running MaWiM v" MAWIM_VERSION "\n");

  mawim_t mawim = {
      .max_cols = 2,
      .max_rows = 3,
      .workspace_count = 2,
      .active_workspace = 1,
      .workspaces = NULL,
  };

  mawim_workspace_init(&mawim);

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

      if (cmd.data != NULL) {
        xfree(cmd.data);
      }
    }

    nanosleep(&sleep_time, NULL);
  }

  mawim_shutdown(&mawim);

  fprintf(stderr, "MaWiM: Goodbye!\n");
  return 0;
}
