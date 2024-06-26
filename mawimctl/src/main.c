/* main.c ; main file for the mawimctl client.
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "mawimctl.h"
#include "mawimctl_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(char *msg) {
  fprintf(stderr, "mawimctl panic'd: %s\n", msg);
  exit(EXIT_FAILURE);
}

const char *ERRNAMES[] = {"Ok",
                          "Invalid Command",
                          "Invalid Data Format",
                          "No Such Workspace",
                          "Configuration file is missing",
                          "Configuration file is malformed",
                          "No window currently focused",
                          "MaWiM encountered an internal error"};

#define MAWIMCTL_CLIENT_BASEVERSION "1.0.1"

#ifndef DEBUG
#define MAWIMCTL_CLIENT_VERSION MAWIMCTL_CLIENT_BASEVERSION
#else
#define MAWIMCTL_CLIENT_VERSION                                                \
  MAWIMCTL_CLIENT_BASEVERSION " [" COMMIT_HASH ", debug build]"
#endif

#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define macro_connection_null_panic()                                          \
  panic(__FILE__ ":" STRINGIFY(__LINE__) ": connection is NULL!")

#define connection_non_null(a)                                                 \
  if (a == NULL) {                                                             \
    macro_connection_null_panic();                                             \
  }

#define send_cmd(a, b)                                                         \
  if (!mawimctl_client_send_command(a, b)) {                                   \
    fprintf(stderr, "failed to send command!\n");                              \
    return 1;                                                                  \
  }

#define read_resp(a, b)                                                        \
  if (!mawimctl_read_response(a, b)) {                                         \
    fprintf(stderr, "failed to read response!\n");                             \
    return 2;                                                                  \
  }

#define handle_resp(c)                                                         \
  if (c.status != MAWIMCTL_OK) {                                               \
    fprintf(stderr,                                                            \
            "MaWiM responded with an error status: "                           \
            "%s\n",                                                            \
            c.status < MAWIMCTL_STATUS_INVALID ? ERRNAMES[c.status]            \
                                               : "unknown error");             \
    return 3;                                                                  \
  }

#define do_cmd(a, b, c)                                                        \
  connection_non_null(a);                                                      \
  send_cmd(a, b);                                                              \
  read_resp(a, &c);                                                            \
  handle_resp(c);

/* command handlers */

int do_generic_cmd(mawimctl_connection_t *connection, uint8_t cmd_id) {
  mawimctl_command_t cmd = {
      .command_identifier = cmd_id, .flags = 0, .data_length = 0, .data = NULL};
  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);
  return 0;
}

int do_close_focused(mawimctl_connection_t *connection, int argc, char **argv) {
  return do_generic_cmd(connection, MAWIMCTL_CLOSE_FOCUSED);
}

int get_version(mawimctl_connection_t *connection, int argc, char **argv) {
  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_GET_VERSION,
                            .flags = 0,
                            .data_length = 0,
                            .data = NULL};
  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);

  fprintf(stdout, "%s\n", (char *)resp.data);

  return 0;
}

int get_workspace(mawimctl_connection_t *connection, int argc, char **argv) {
  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_GET_WORKSPACE,
                            .flags = 0,
                            .data_length = 0,
                            .data = NULL};
  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);

  if (resp.data == NULL) {
    panic("response data is NULL!");
  }

  fprintf(stdout, "%d\n", (mawimctl_workspaceid_t)*resp.data);

  return 0;
}

int do_move_focused_to_workspace(mawimctl_connection_t *connection, int argc,
                                 char **argv) {
  if (argc < 1) {
    fprintf(
        stderr,
        "move_focused_to_workspace expects 1 argument: workspace number!\n");
    return 1;
  }

  mawimctl_workspaceid_t wanted_workspace = atoi(argv[0]);
  if (wanted_workspace < 1 || wanted_workspace > 9) {
    panic("atoi for wanted_workspace failed!");
  }

  mawimctl_command_t cmd = {.command_identifier =
                                MAWIMCTL_MOVE_FOCUSED_TO_WORKSPACE,
                            .flags = 0,
                            .data_length = sizeof(wanted_workspace),
                            .data = malloc(sizeof(wanted_workspace))};

  if (cmd.data == NULL) {
    panic("unable to allocate data for command!\n");
  }

  memcpy(cmd.data, &wanted_workspace, sizeof(wanted_workspace));

  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);

  return 0;
}

int do_reload(mawimctl_connection_t *connection, int argc, char **argv) {
  return do_generic_cmd(connection, MAWIMCTL_RELOAD);
}

int set_workspace(mawimctl_connection_t *connection, int argc, char **argv) {
  if (argc < 1) {
    fprintf(stderr, "set_workspace expects 1 argument: workspace number!\n");
    return 1;
  }

  mawimctl_workspaceid_t wanted_workspace = atoi(argv[0]);
  if (wanted_workspace < 1 || wanted_workspace > 9) {
    panic("atoi for wanted_workspace failed!");
  }

  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_SET_WORKSPACE,
                            .flags = 0,
                            .data_length = sizeof(wanted_workspace),
                            .data = malloc(sizeof(wanted_workspace))};

  if (cmd.data == NULL) {
    panic("unable to allocate data for command!\n");
  }

  memcpy(cmd.data, &wanted_workspace, sizeof(wanted_workspace));

  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);

  return 0;
}

struct handler {
  char *cmd_name;
  char *params_str; /* used for help output */
  int (*handler)(mawimctl_connection_t *, int argc, char **argv);
};

const struct handler cmd_handlers[] = {
    {.cmd_name = "close_focused",
     .params_str = "",
     .handler = &do_close_focused},
    {.cmd_name = "get_version", .params_str = "", .handler = &get_version},
    {.cmd_name = "get_workspace", .params_str = "", .handler = &get_workspace},
    {.cmd_name = "move_focused_to_workspace",
     .params_str = "<workspace number>",
     .handler = &do_move_focused_to_workspace},
    {.cmd_name = "reload", .params_str = "", .handler = &do_reload},
    {.cmd_name = "set_workspace",
     .params_str = "<workspace number>",
     .handler = &set_workspace},
};

const int cmd_handlers_count = sizeof(cmd_handlers) / sizeof(struct handler);

void list_commands() {
  fprintf(stderr, "=> client version " MAWIMCTL_CLIENT_VERSION
                  "\n=> protocol version " MAWIMCTL_VERSION "\n\n");
  fprintf(stderr, "Usage: mawimctl <command [parameters]>\n");
  fprintf(stderr, "Commands:\n");
  for (int i = 0; i < cmd_handlers_count; i++) {
    fprintf(stderr, "\t%s %s\n", cmd_handlers[i].cmd_name,
            cmd_handlers[i].params_str);
  }
}

int main(int argc, char **argv) {
  if (argc == 1) {
    list_commands();
    return 0;
  }

  int ret = -1;

  for (int i = 0; i < cmd_handlers_count; i++) {
    if (strcmp(argv[1], cmd_handlers[i].cmd_name) == 0) {
      mawimctl_connection_t *connection =
          mawimctl_client_connect(getenv("MAWIMCTL_SOCK"));
      if (connection == NULL) {
        macro_connection_null_panic();
      }

      ret = cmd_handlers[i].handler(connection, argc - 2, &argv[2]);
      free(connection);
      break;
    }
  }

  if (ret == -1) {
    fprintf(stderr, "no such command!\n");
  }

  return ret;
}
