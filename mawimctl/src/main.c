#include "mawimctl_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(char *msg) {
  fprintf(stderr, "mawimctl panic'd: %s\n", msg);
  exit(EXIT_FAILURE);
}

struct handler {
  char  *cmd_name;
  char  *params_str; /* used or help output */
  int  (*handler)(mawimctl_connection_t*, int argc, char **argv);
};

/* clang-format off */

#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define macro_connection_null_panic() panic( __FILE__  ":" \
                                             STRINGIFY(__LINE__) \
                                             ": connection is NULL!" )

#define connection_non_null(a) if (a == NULL) { macro_connection_null_panic(); }

#define send_cmd(a, b) if (!mawimctl_client_send_command(a, b)) { \
                         fprintf(stderr, "failed to send command!\n"); \
                         return 1; \
                       }

#define read_resp(a, b) if (!mawimctl_read_response(a, b)) { \
                          fprintf(stderr, "failed to read response!\n"); \
                          return 2; \
                        }

#define handle_resp(c) if (c.status != MAWIMCTL_OK) { \
                         fprintf(stderr, \
                                 "MaWiM responded with an error status: " \
                                 "%d\n", c.status); \
                         return 3; \
                       }

#define do_cmd(a, b, c) connection_non_null(a); \
                        send_cmd(a, b); \
                        read_resp(a, &c); \
                        handle_resp(c);

/* clang-format on */

int get_version(mawimctl_connection_t *connection, int argc, char **argv) {
  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_GET_VERSION,
                            .flags = 0,
                            .data_length = 0,
                            .data = NULL};
  mawimctl_response_t resp;
  do_cmd(connection, cmd, resp);

  fprintf(stdout, "%s\n", (char *) resp.data);

  return 0;
}

const struct handler cmd_handlers[] = {
  {.cmd_name = "close_focused", .params_str = "", .handler = NULL},
  {.cmd_name = "get_version", .params_str = "", .handler = &get_version},
  {.cmd_name = "get_workspace", .params_str = "", .handler = NULL},
  {.cmd_name = "move_focused_to_workspace", .params_str = "<workspace number>", .handler = NULL},
  {.cmd_name = "reload", .params_str = "", .handler = NULL},
  {.cmd_name = "set_workspace", .params_str = "<workspace number>", .handler = NULL},
};

const int cmd_handlers_count = sizeof(cmd_handlers) / sizeof(struct handler);

void list_commands() {
  fprintf(stderr, "Usage: mawimctl <command [parameters]>\n");
  fprintf(stderr, "Commands:\n");
  for (int i = 0; i < cmd_handlers_count; i++) {
    fprintf(stderr, "\t%s %s\n", cmd_handlers[i].cmd_name, cmd_handlers[i].params_str);
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
      mawimctl_connection_t *connection = mawimctl_client_connect(NULL);
      if (connection == NULL) {
        macro_connection_null_panic();
      }

      ret = cmd_handlers[i].handler(connection, argc, argv);
      free(connection);
      break;
    }
  }

  if (ret == -1) {
    fprintf(stderr, "no such command!\n");
  }

  return ret;
}
