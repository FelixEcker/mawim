/* error.h ; MaWiM Error Handling
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef ERROR_H
#define ERROR_H

#include <X11/Xlib.h>

#define ERRTEXT_BUFF_SIZE 1024

/**
 * @brief print the message and the exit with EXIT_FAILURE
 */
void mawim_panic(char *msg);

/**
 * @brief custom handler for X11 errors
 */
int mawim_x11_error_handler(Display *display, XErrorEvent *error);

#endif