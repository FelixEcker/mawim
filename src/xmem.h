/* xmem.h ; MaWiM memory management helpers
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef XMEM_H
#define XMEM_H

#include "error.h"

#include <stddef.h>

#define _STR(x) #x
#define STR(x) _STR(x)

#define xmalloc(s)                                                             \
  ({                                                                           \
    void *ret;                                                                 \
    ret = malloc(s);                                                           \
    if (ret == NULL) {                                                         \
      mawim_panic("xmalloc returned NULL!");                                   \
    }                                                                          \
    ret;                                                                       \
  })

#define xrealloc(o, s)                                                         \
  ({                                                                           \
    void *ret;                                                                 \
    ret = realloc(o, s);                                                       \
    if (ret == NULL) {                                                         \
      mawim_panic("xrealloc returned NULL!");                                  \
    }                                                                          \
    ret;                                                                       \
  })

#define xfree(p)                                                               \
  if (p != NULL) {                                                             \
    free(p);                                                                   \
  } else {                                                                     \
    mawim_log(LOG_ERROR, __FILE__ ":" STR(__LINE__) ": xfree received NULL!"); \
  }

#endif /* #ifndef XMEM_H */
