/* mawimctl_server.h ; header for mawimctl servers.
 * See github.com/FelixEcker/mawim.git -> doc/mawimctl.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIMCTL_SERVER_H
#define MAWIMCTL_SERVER_H

#include "mawimctl.h"

#include <stdbool.h>
#include <sys/un.h>

#ifndef MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE
#define MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE 20
#endif

extern mawimctl_response_t mawimctl_invalid_command_response;
extern mawimctl_response_t mawimctl_invalid_data_format_response;
extern mawimctl_response_t mawimctl_no_such_workspace_response;
extern mawimctl_response_t mawimctl_internal_error_response;

extern mawimctl_response_t mawimctl_generic_ok_response;

/* clang-format off */

typedef struct mawimctl_server {
  char               *sock_path;
  struct sockaddr_un  sock_name;
  int                 sock_fd;

  int                 pending_cmd_count;
  mawimctl_command_t *pending_cmds;
} mawimctl_server_t;

/* clang-format on */

/**
 * @brief Starts a mawimctl server
 * @param where The location in the filesystem where the socket should be
 * created. If NULL it will default to MAWIMCTL_DEFAULT_SOCK_LOCATION
 * @return Heap-Allocated server structure, NULL on failure
 */
mawimctl_server_t *mawimctl_server_start(char *where);

/**
 * @brief Stops a mawimctl server and frees the structure
 * @param server The server instance to be stopped
 */
void mawimctl_server_stop(mawimctl_server_t *server);

/**
 * @brief Updates the server. This includes accepting all new connection
 *        requests, reading their commands an queueing them.
 * @param server The server instance to be updated
 */
void mawimctl_server_update(mawimctl_server_t *server);

/**
 * @brief Gets the first pending command in the server's queue.
 * @param server The server instance from which the command should be grabbed
 * @param dest_container Pointer to a mawimctl_command_t structure where the
 * next command should be written to
 * @return true if a command was in queue, false if the queue was empty.
 */
bool mawimctl_server_next_command(mawimctl_server_t *server,
                                  mawimctl_command_t *dest_container);

/**
 * @brief Sends a response to the specified client.
 * @param server The server to send the response from
 * @param sockfd The socket file descriptor of the client to send it to
 * @param response The response data
 * @return true on successful response
 */
bool mawimctl_server_respond(mawimctl_server_t *server, int sockfd,
                             mawimctl_response_t response);

#endif /* #ifndef MAWIMCTL_SERVER_H */
