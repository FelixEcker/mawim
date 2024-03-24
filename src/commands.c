/* commands.c ; mawimctl commands handler
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "commands.h"

#include "logging.h"
#include "mawim.h"
#include "mawimctl_server.h"
#include "xmem.h"

#include <string.h>

mawimctl_response_t handle_set_workspace(mawim_t *mawim,
                                         mawimctl_command_t cmd) {
  mawimctl_response_t resp = mawimctl_generic_ok_response;
  if (cmd.data_length != 1) {
    return mawimctl_invalid_data_format_response;
  }

  if (cmd.data == NULL) {
    mawim_log(
        LOG_ERROR,
        "handle_set_workspace: cmd.data_length is 1 but cmd.data is NULL!\n");
    return mawimctl_internal_error_response;
  }

  uint8_t wanted_workspace = cmd.data[0];
  if (wanted_workspace > mawim->workspace_count) {
    return mawimctl_no_such_workspace_response;
  }

  mawim->active_workspace = wanted_workspace;

  /* mawim_update_workspace(maiwm); */

  return resp;
}

bool mawim_handle_ctl_command(mawim_t *mawim, mawimctl_command_t cmd) {
  mawimctl_response_t resp = mawimctl_generic_ok_response;

  switch (cmd.command_identifier) {
  case MAWIMCTL_GET_VERSION:
    resp.data_length = sizeof(MAWIM_VERSION) + 1;
    resp.data = xmalloc(resp.data_length);
    memcpy(resp.data, MAWIM_VERSION, resp.data_length);
    break;
  case MAWIMCTL_GET_WORKSPACE:
    resp.data_length = sizeof(mawim->active_workspace);
    resp.data = xmalloc(resp.data_length);
    memcpy(resp.data, &mawim->active_workspace, resp.data_length);
    break;
  case MAWIMCTL_SET_WORKSPACE:
    resp = handle_set_workspace(mawim, cmd);
    break;
  default:
    return false;
  }

  bool resp_succ =
      mawimctl_server_respond(mawim->mawimctl, cmd.sender_fd, resp);
  if (!resp_succ) {
    mawim_log(LOG_ERROR, "failed to send mawimctl response!\n");
  }

  if (resp.data != NULL) {
    xfree(resp.data);
  }

  return true;
}
