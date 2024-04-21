/* events.h ; MaWiM Event Handling
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIM_EVENTS_H
#define MAWIM_EVENTS_H

#include "types.h"

extern const char *event_type_str[];

/**
 * @brief handle the passed event
 * @param mawim The mawim instance
 * @param event The XEvent to be handled
 * @return true - event went handled, false - event went unhandled
 */
bool mawim_handle_event(mawim_t *mawim, XEvent event);

#endif
