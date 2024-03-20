/* xmem.h ; MaWiM memory management helpers
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef XMEM_H
#define XMEM_H

#if !defined(XMEM_LOGGING)
#define XMEM_LOGGING 1
#endif

#if !defined(XMEM_LOG_FREES)
#define XMEM_LOG_FREES 0
#endif

#include <stddef.h>

void *xmalloc(size_t size);

void *xrealloc(void *org, size_t size);

void xfree(void *ptr);

#endif /* #ifndef XMEM_H */
