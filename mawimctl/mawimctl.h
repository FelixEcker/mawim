/* mawimctl.h ; type definitions for the mawimctl protocol.
 * See github.com/FelixEcker/mawim.git -> mawimctl/protocol.md
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIMCTL_H
#define MAWIMCTL_H

#include <stdint.h>

/* clang-format off */

#define MAWIMCTL_DEFAULT_SOCK_LOCATION "/tmp/mawim.control.socket"
#define MAWIMCTL_FLAG_NO_RESPONSE 0b10000000

enum mawimctl_cmd_id {
  MAWIMCTL_GET_VERSION = 0,
  MAWIMCTL_GET_WORKSPACE,
  MAWIMCTL_SET_WORKSPACE,
  MAWIMCTL_RELOAD,
  MAWIMCTL_CLOSE_FOCUSED,
  MAWIMCTL_MOVE_FOCUSED_TO_WORKSPACE,

  /* Has to be last value */
  MAWIMCTL_CMD_INVALID,
};

enum mawimctl_status {
  MAWIMCTL_OK,
  MAWIMCTL_INVALID_COMMAND,
  MAWIMCTL_INVALID_DATA_FORMAT,
  MAWIMCTL_NO_SUCH_WORKSPACE,
  MAWIMCTL_CONFIG_MISSING,
  MAWIMCTL_CONFIG_MALFORMED,
  MAWIMCTL_NO_WINDOW_FOCUSED,
  MAWIMCTL_INTENRAL_ERROR,

  /* Has to be last value */
  MAWIMCTL_STATUS_INVALID,
};

typedef struct mawimctl_command {
  int sender_fd;

  /* Data */
  uint8_t   command_identifier;
  uint8_t   flags;
  uint16_t  data_length;
  uint8_t  *data;
} mawimctl_command_t;

#define MAWIMCTL_COMMAND_BASESIZE 5
#define MAWIMCTL_COMMAND_MAXSIZE MAWIMCTL_COMMAND_BASESIZE + UINT16_MAX

typedef struct mawimctl_response {
  uint8_t   status;
  uint16_t  data_length;
  uint8_t  *data;
} mawimctl_response_t;

#define MAWIMCTL_RESPONSE_BASESIZE 3
#define MAWIMCTL_RESPONSE_MAXSIZE MAWIMCTL_RESPONSE_BASESIZE + UINT16_MAX

/* clang-format on */

#endif /* #ifndef MAWIMCTL_H */
