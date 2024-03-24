/* mawimctl_server.h ; mawimctl server implementation.
 * See github.com/FelixEcker/mawim.git -> mawimctl/protocol.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "mawimctl_server.h"

#include "logging.h"
#include "mawimctl.h"
#include "xmem.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

mawimctl_response_t mawimctl_invalid_command_response = {
    .status = MAWIMCTL_INVALID_COMMAND, .data_length = 0, .data = NULL};

mawimctl_response_t mawimctl_invalid_data_format_response = {
    .status = MAWIMCTL_INVALID_DATA_FORMAT, .data_length = 0, .data = NULL};

mawimctl_response_t mawimctl_no_such_workspace_response = {
    .status = MAWIMCTL_NO_SUCH_WORKSPACE, .data_length = 0, .data = NULL};

mawimctl_response_t mawimctl_internal_error_response = {
    .status = MAWIMCTL_INTENRAL_ERROR, .data_length = 0, .data = NULL};

mawimctl_response_t mawimctl_generic_ok_response = {
    .status = MAWIMCTL_OK, .data_length = 0, .data = NULL};

void _try_remove_old_sock(char *where) {
  if (access(where, F_OK) != 0) {
    return;
  }

  /* attempt to connect to the socket */
  int sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (sock_fd == -1) {
    return;
  }

  /* Binding */
  struct sockaddr_un sock_name;

  memset(&sock_name, 0, sizeof(sock_name));
  sock_name.sun_family = AF_UNIX;
  strncpy(sock_name.sun_path, where, sizeof(sock_name.sun_path) - 1);

  int ret =
      connect(sock_fd, (const struct sockaddr *)&sock_name, sizeof(sock_name));

  if (ret == -1) {
    remove(where);
  } else {
    mawim_logf(LOG_WARNING, "could'nt remove existing socket at \"%s\"\n",
               where);
  }
}

mawimctl_server_t *mawimctl_server_start(char *where) {
  char *errstr;

  if (where == NULL) {
    where = MAWIMCTL_DEFAULT_SOCK_LOCATION;
  }

  _try_remove_old_sock(where);

  mawimctl_server_t *server = xmalloc(sizeof(mawimctl_server_t));
  server->sock_path = where;

  /* Initialise Socket */
  server->sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (server->sock_fd == -1) {
    errstr = strerror(errno);
    mawim_logf(LOG_ERROR,
               "mawimctl_server: error creating server socket: %s "
               "(OS Error %d)\n",
               errstr, errno);
    xfree(server);
    return NULL;
  }

  /* Make the server socket non-blocking */
  fcntl(server->sock_fd, F_SETFL, O_NONBLOCK);

  /* Binding */
  memset(&server->sock_name, 0, sizeof(server->sock_name));
  server->sock_name.sun_family = AF_UNIX;
  strncpy(server->sock_name.sun_path, server->sock_path,
          sizeof(server->sock_name.sun_path) - 1);

  int ret = bind(server->sock_fd, (const struct sockaddr *)&server->sock_name,
                 sizeof(server->sock_name));

  if (ret == -1) {
    errstr = strerror(errno);
    mawim_logf(LOG_ERROR,
               "mawimctl_server: error binding server socket: %s "
               "(OS Error %d)\n",
               errstr, errno);
    xfree(server);
    return NULL;
  }

  ret = listen(server->sock_fd, MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE);
  if (ret == -1) {
    errstr = strerror(errno);
    mawim_logf(LOG_ERROR,
               "mawimctl_server: error listening on server socket: %s "
               "(OS Error %d)\n",
               errstr, errno);
    xfree(server);
    return NULL;
  }

  return server;
}

void mawimctl_server_stop(mawimctl_server_t *server) {
  for (int i = 0; i < server->pending_cmd_count; i++) {
    close(server->pending_cmds[i].sender_fd);
  }

  xfree(server->pending_cmds);

  close(server->sock_fd);

  xfree(server);
  mawim_log(LOG_INFO, "mawimctl_server: stopped.\n");
}

mawimctl_command_t _parse_command(uint8_t *raw_buffer, int buffer_size) {
  mawimctl_command_t command;

  int cpy_offs = 0;

  memcpy(&command.command_identifier, raw_buffer,
         sizeof(command.command_identifier));
  cpy_offs += sizeof(command.command_identifier);

  memcpy(&command.data_length, raw_buffer + cpy_offs,
         sizeof(command.data_length));
  cpy_offs += sizeof(command.data_length);

  int to_copy = (buffer_size - MAWIMCTL_COMMAND_BASESIZE) != command.data_length
                    ? (buffer_size - MAWIMCTL_COMMAND_BASESIZE)
                    : command.data_length;

  if (to_copy > 0) {
    command.data = xmalloc(to_copy);
    memcpy(command.data, raw_buffer + cpy_offs, to_copy);
  }

  return command;
}

void _queue_command(mawimctl_server_t *server, mawimctl_command_t command) {
  if (server->pending_cmd_count == 0) {
    if (server->pending_cmds != NULL) {
      xfree(server->pending_cmds);
    }

    server->pending_cmds = xmalloc(sizeof(command));
  } else {
    server->pending_cmds =
        xrealloc(server->pending_cmds,
                 sizeof(command) * (server->pending_cmd_count + 1));
  }

  server->pending_cmds[server->pending_cmd_count] = command;
  server->pending_cmd_count++;
}

void _handle_incoming_command(mawimctl_server_t *server, int fd) {
  if (server == NULL) {
    return;
  }

  /* Receive */
  uint8_t recvbuf[MAWIMCTL_COMMAND_MAXSIZE];
  int bytes_read = read(fd, recvbuf, sizeof(recvbuf));

  if (bytes_read == -1) {
    mawim_log(LOG_ERROR, "mawimctl_server: failed to read command!\n");
    close(fd);
    return;
  }

  mawim_logf(LOG_DEBUG, "mawimctl_server: read %d bytes\n", bytes_read);

  if (bytes_read < MAWIMCTL_COMMAND_BASESIZE) {
    mawim_log(LOG_ERROR, "mawimctl_server: received invalid data format!\n");
    if (!mawimctl_server_respond(server, fd,
                                 mawimctl_invalid_data_format_response)) {
      mawim_log(LOG_ERROR, "mawimctl_server: failed to send response!\n");
    }
    close(fd);
    return;
  }

  mawimctl_command_t command = _parse_command(recvbuf, bytes_read);
  command.sender_fd = fd;

  /* Handle invalid command */
  if (command.command_identifier >= MAWIMCTL_CMD_INVALID) {
    mawim_log(LOG_ERROR, "mawimctl_server: received invalid command!\n");
    if (!mawimctl_server_respond(server, fd,
                                 mawimctl_invalid_command_response)) {
      mawim_log(LOG_ERROR, "mawimctl_server: failed to send response!\n");
    }
    close(fd);
    return;
  }

  _queue_command(server, command);
}

void mawimctl_server_update(mawimctl_server_t *server) {
  if (server == NULL) {
    return;
  }

  while (true) {
    int newfd = accept(server->sock_fd, NULL, NULL);
    if (newfd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      char *errstr = strerror(errno);
      mawim_logf(LOG_ERROR,
                 "mawimctl_server: error accepting on server socket: %s "
                 "(OS Error %d)\n",
                 errstr, errno);
      return;
    }

    mawim_log(LOG_DEBUG, "mawimctl_server: accepted 1 connection\n");
    _handle_incoming_command(server, newfd);
  }
}

bool mawimctl_server_next_command(mawimctl_server_t *server,
                                  mawimctl_command_t *dest_container) {
  if (server == NULL || server->pending_cmd_count == 0) {
    return false;
  }

  *dest_container = server->pending_cmds[0];
  server->pending_cmd_count--;

  if (server->pending_cmd_count == 0) {
    xfree(server->pending_cmds);
    server->pending_cmds = NULL;
    return true;
  }

  /* Move every queued command up 1 slot */
  for (int wix = 0; wix < server->pending_cmd_count; wix++) {
    server->pending_cmds[wix] = server->pending_cmds[wix + 1];
  }

  server->pending_cmds =
      xrealloc(server->pending_cmds,
               sizeof(mawimctl_command_t) * server->pending_cmd_count);

  return true;
}

bool mawimctl_server_respond(mawimctl_server_t *server, int sockfd,
                             mawimctl_response_t response) {
  size_t sendbuf_size = MAWIMCTL_RESPONSE_BASESIZE + response.data_length;
  uint8_t *sendbuf = xmalloc(sendbuf_size);

  /* Copy data to send buffer */
  int cpyoffs = 0;

  memcpy(sendbuf + cpyoffs, &response.status, sizeof(response.status));
  cpyoffs += sizeof(response.status);

  if (response.data != NULL) {
    memcpy(sendbuf + cpyoffs, &response.data_length,
           sizeof(response.data_length));
    cpyoffs += sizeof(response.data_length);

    memcpy(sendbuf + cpyoffs, response.data, response.data_length);
  }

  int ret = write(sockfd, sendbuf, sendbuf_size);

  if (ret == -1) {
    char *errstr = strerror(errno);
    mawim_logf(LOG_ERROR, "mawimctl_server: %s (OS Error %d)\n", errstr, errno);
  } else {
    mawim_logf(LOG_DEBUG, "mawimctl_server: sent %d out of %d bytes!\n", ret,
               sendbuf_size);
  }

  xfree(sendbuf);
  return true;
}
