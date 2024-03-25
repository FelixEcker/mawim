/* mawimctl_client.c ; implementation for mawimctl client-side communication.
 * See github.com/FelixEcker/mawim.git -> doc/mawimctl.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

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

mawimctl_response_t _parse_response(uint8_t *buff, int buffsize) {
  mawimctl_response_t ret = {
      .status = MAWIMCTL_STATUS_INVALID, .data_length = 0, .data = NULL};

  if (buffsize < MAWIMCTL_RESPONSE_BASESIZE) {
    return ret;
  }

  int cpyoffs = 0;
  memcpy(&ret.status, buff + cpyoffs, sizeof(ret.status));
  cpyoffs += sizeof(ret.status);

  memcpy(&ret.data_length, buff + cpyoffs, sizeof(ret.data_length));
  cpyoffs += sizeof(ret.data_length);

  int to_copy = ret.data_length > (buffsize - MAWIMCTL_RESPONSE_BASESIZE)
                    ? (buffsize - MAWIMCTL_RESPONSE_BASESIZE)
                    : ret.data_length;

  if (to_copy > 0) {
    ret.data = malloc(to_copy);
    memcpy(ret.data, buff + cpyoffs, to_copy);
  }

  return ret;
}

bool mawimctl_read_response(mawimctl_connection_t *connection,
                            mawimctl_response_t *dest) {
  if (connection == NULL || dest == NULL) {
    return false;
  }

  uint8_t recvbuf[MAWIMCTL_COMMAND_MAXSIZE];
  int bytes_read = read(connection->sock_fd, recvbuf, sizeof(recvbuf));

  if (bytes_read == -1) {
    fprintf(stderr, "failed to read response!\n");
    close(connection->sock_fd);
    return false;
  }

  *dest = _parse_response(recvbuf, bytes_read);

  return true;
}
