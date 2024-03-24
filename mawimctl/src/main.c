#include "mawimctl_client.h"

#include <stdio.h>
#include <stdlib.h>

void panic(char *msg) {
  fprintf(stderr, "mawimctl panic'd: %s\n", msg);
  exit(EXIT_FAILURE);
}

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
                          fprintf(stderr, " failed to read response!\n"); \
                          return 2; \
                        }

#define do_cmd(a, b, c) connection_non_null(a); send_cmd(a, b); read_resp(a, c);

/* clang-format on */

void list_commands() {
  fprintf(stderr, "Usage: mawimctl <command [parameters]>\n");
  fprintf(stderr, "Commands:\n");
  fprintf(stderr, "\tget_version\n");
  fprintf(stderr, "\tget_workspace\n");
  fprintf(stderr, "\tset_workspace <workspace number>\n");
  fprintf(stderr, "\treload\n");
  fprintf(stderr, "\tclose_focused\n");
  fprintf(stderr, "\tmove_focused_to_workspace <workspace number>\n");
}

int get_version(mawimctl_connection_t *connection) {
  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_GET_VERSION,
                            .flags = 0,
                            .data_length = 0,
                            .data = NULL};
  mawimctl_response_t resp;

  do_cmd(connection, cmd, &resp);

  return 0;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    list_commands();
    return 0;
  }

  mawimctl_connection_t *connection = mawimctl_client_connect(NULL);
  if (connection == NULL) {
    return false;
  }

  int ret = get_version(connection);

  free(connection);
  return ret;
}
