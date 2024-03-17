#ifndef MAWIM_H
#define MAWIM_H

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#define true True
#define false False
#define XNULL 0

typedef int bool;

/* clang-format off */

typedef struct mawim {
  /* X11 */
  Display *display;
  Window   root_window;
  Cursor   cursor;
} mawim_t;

/* clang-format on */

/**
 * @brief print the message and the exit with EXIT_FAILURE
 */
void mawim_panic(char *msg);

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
