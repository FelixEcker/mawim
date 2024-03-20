/* mawim.h ; MaWiM core
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef MAWIM_H
#define MAWIM_H

#include "types.h"

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#define MAWIM_VERSION "0.0.0"

/**
 * @brief flushes x11 events
 * @see mawim_x11_discarding_flush
 * @see XSync
 */
void mawim_x11_flush(mawim_t *mawim);

/**
 * @brief discards x11 events
 * @see mawim_x11_flush
 * @see XSync
 */
void mawim_x11_discarding_flush(mawim_t *mawim);

/**
 * @brief initialises x11 for mawim
 */
void mawim_x11_init(mawim_t *mawim);

/**
 * @brief does the x11 shutdown for mawim
 */
void mawim_x11_shutdown(mawim_t *mawim);

#endif /* #ifndef MAWIM_H */
