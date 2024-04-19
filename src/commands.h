/* commands.h ; header file for mawimctl commands handler
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "types.h"

/**
 * @brief handles the passed mawimctl command
 * @param mawim The mawim instance
 * @param cmd The command to be handled
 * @return true - command went handled
 */
bool mawim_handle_ctl_command(mawim_t *mawim, mawimctl_command_t cmd);

#endif /* #ifndef COMMANDS_H */
