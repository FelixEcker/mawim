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

#define BASE_VERSION "0.3.1 (develop-test)"

#ifndef DEBUG
#define MAWIM_VERSION BASE_VERSION
#else
#define MAWIM_VERSION BASE_VERSION " [" COMMIT_HASH ", debug build]"
#endif

/**
 * @brief flushes x11 events
 * @param mawim The mawim instance to flush with
 * @see mawim_x11_discarding_flush
 * @see XSync
 */
void mawim_x11_flush(mawim_t *mawim);

/**
 * @brief discards x11 events
 * @param mawim The mawim instance to flush with
 * @see mawim_x11_flush
 * @see XSync
 */
void mawim_x11_discarding_flush(mawim_t *mawim);

/**
 * @brief initialises x11 for mawim
 * @param mawim The mawim instance to initialise
 */
void mawim_x11_init(mawim_t *mawim);

/**
 * @brief does the x11 shutdown for mawim
 * @param mawim The mawim instance to shutdown
 */
void mawim_shutdown(mawim_t *mawim);

#endif /* #ifndef MAWIM_H */
