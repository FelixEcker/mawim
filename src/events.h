/* events.h ; MaWiM Event Handling
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIM_EVENTS_H
#define MAWIM_EVENTS_H

#include "mawim.h"

/**
 * @brief handle the passed event
 * @return true - event went handled, false - event went unhandled
 */
bool mawim_handle_event(mawim_t *mawim, XEvent event);

#endif
