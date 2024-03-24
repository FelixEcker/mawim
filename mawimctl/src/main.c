#include "mawimctl_client.h"

#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char **argv) {
  if (argc == 1) {
    list_commands();
    return 0;
  }

  mawimctl_connection_t *connection = mawimctl_client_connect(NULL);
  if (connection == NULL) {
    return false;
  }

  int ret = 0;

  mawimctl_command_t cmd = {.command_identifier = MAWIMCTL_GET_VERSION,
                            .flags = 0,
                            .data_length = 0,
                            .data = NULL};

  if (!mawimctl_client_send_command(connection, cmd)) {
    fprintf(stderr, "failed to send command!\n");
    ret = 1;
    goto exit;
  }

  mawimctl_response_t resp;
  if (!mawimctl_read_response(connection, &resp)) {
    ret = 1;
    goto exit;
  }

  printf("mawim version (%d bytes): %s\n", resp.data_length, (char *)resp.data);

exit:
  free(connection);
  return ret;
}
