#include "mawimctl_client.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

mawimctl_connection_t *mawimctl_client_connect(char *where) {
  char *errstr;

  if (where == NULL) {
    where = MAWIMCTL_DEFAULT_SOCK_LOCATION;
  }

  mawimctl_connection_t *connection = malloc(sizeof(mawimctl_connection_t));
  if (connection == NULL) {
    fprintf(stderr, "Couldn't allocate memory for connection!\n");
    return NULL;
  }

  connection->sock_path = where;

  /* Initialise Socket */
  connection->sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (connection->sock_fd == -1) {
    errstr = strerror(errno);
    fprintf(stderr,
            "mawimctl_connection: error creating connection: %s "
            "(OS Error %d)\n",
            errstr, errno);
    free(connection);
    return NULL;
  }

  /* Binding */
  memset(&connection->sock_name, 0, sizeof(connection->sock_name));
  connection->sock_name.sun_family = AF_UNIX;
  strncpy(connection->sock_name.sun_path, connection->sock_path,
          sizeof(connection->sock_name.sun_path) - 1);

  int ret = connect(connection->sock_fd,
                    (const struct sockaddr *)&connection->sock_name,
                    sizeof(connection->sock_name));
  if (ret == -1) {
    errstr = strerror(errno);
    fprintf(stderr,
            "mawimctl_connection: error connecting: %s "
            "(OS Error %d)\n",
            errstr, errno);
    free(connection);
    return NULL;
  }

  return connection;
}

bool mawimctl_client_send_command(mawimctl_connection_t *connection,
                                  mawimctl_command_t command) {
  size_t sendbuf_size = MAWIMCTL_COMMAND_BASESIZE + command.data_length;
  uint8_t *sendbuf = malloc(sendbuf_size);
  memset(sendbuf, 0, sendbuf_size);

  /* Copy data to send buffer */
  int cpyoffs = 0;

  memcpy(sendbuf + cpyoffs, &command.command_identifier,
         sizeof(command.command_identifier));
  cpyoffs += sizeof(command.command_identifier);

  memcpy(sendbuf + cpyoffs, &command.flags, sizeof(command.flags));
  cpyoffs += sizeof(command.flags);

  if (command.data != NULL) {
    memcpy(sendbuf + cpyoffs, &command.data_length,
           sizeof(command.data_length));
    cpyoffs += sizeof(command.data_length);

    memcpy(sendbuf + cpyoffs, command.data, command.data_length);
  }

  bool success = write(connection->sock_fd, sendbuf, sendbuf_size) != -1;

  free(sendbuf);
  return success;
}

int mawimctl_read_response(mawimctl_connection_t *connection,
                           mawimctl_response_t *dest) {}

int main(void) {
  mawimctl_connection_t *connection = mawimctl_client_connect(NULL);
  if (connection == NULL) {
    return 1;
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

exit:
  free(connection);
  return ret;
}
