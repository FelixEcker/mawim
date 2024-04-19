/* mawimctl_server.h ; header for mawimctl clients.
 * See github.com/FelixEcker/mawim.git -> doc/mawimctl.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIMCTL_CLIENT_H
#define MAWIMCTL_CLIENT_H

#include "mawimctl.h"

#include <stdbool.h>
#include <sys/un.h>

/* clang-format off */

typedef struct mawimctl_connection {
  int                 sock_fd;
  char               *sock_path;
  struct sockaddr_un  sock_name;
} mawimctl_connection_t;

/* clang-format on */

/**
 * @brief Establish a connection the mawimctl server / MaWiM
 * @param where The location in the filesystem where the client should connect
 * to
 * @return NULL if connect failed, pointer to newly allocated
 * mawimctl_connection_t structure otherwise
 */
mawimctl_connection_t *mawimctl_client_connect(char *where);

/**
 * @brief Send a command to the mawimctl server
 * @param connection The connection to send the command to
 * @param command The command
 * @return success
 */
bool mawimctl_client_send_command(mawimctl_connection_t *connection,
                                  mawimctl_command_t command);

/**
 * @brief Read a response from the mawimctl server into dest
 * @param connection The connection the response should be read from
 * @param dest Pointer to a response structure where the response should be
 * written to
 * @return success
 */
bool mawimctl_read_response(mawimctl_connection_t *connection,
                            mawimctl_response_t *dest);

#endif /* #ifndef MAWIMCTL_CLIENT_H */
