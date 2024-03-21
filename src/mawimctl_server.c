/* mawimctl_server.h ; mawimctl server implementation.
 * See github.com/FelixEcker/mawim.git -> mawimctl/protocol.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "mawimctl_server.h"

#include "logging.h"
#include "xmem.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>

mawimctl_server_t *mawimctl_server_start(char *where) {
  char *errstr;

  if (where == NULL) {
    where = MAWIMCTL_DEFAULT_SOCK_LOCATION;
  }

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

void mawimctl_server_stop(mawimctl_server_t *server) { /* TODO: Implement */ }

void _handle_incoming_command(mawimctl_server_t *server, int fd) {
  if (server == NULL) {
    return;
  }
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
