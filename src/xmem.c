/* xmem.c ; MaWiM memory management helpers
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#include "xmem.h"

#include "mawim.h"

#if XMEM_LOGGING
#include "logging.h"
#endif

#include <stdlib.h>

void *xmalloc(size_t size) {
  void *ret = malloc(size);
  if (ret == NULL) {
#if XMEM_LOGGING
    mawim_logf(LOG_ERROR, "xmalloc failed, size = %zu", size);
#endif
    mawim_panic("xmalloc error!");
  }

  return ret;
}

void *xrealloc(void *org, size_t size) {
  void *ret = realloc(org, size);
  if (ret == NULL) {
#if XMEM_LOGGING
    mawim_logf(LOG_ERROR, "xrealloc failed, org = %p ; size = %zu", org, size);
#endif
    mawim_panic("xrealloc error!");
  }

  return ret;
}

void xfree(void *ptr) {
  if (ptr != NULL) {
    mawim_logf(LOG_DEBUG, "xfree on %p\n", ptr);
    free(ptr);
    return;
  }

#if XMEM_LOG_FREES
  mawim_logf(LOG_ERROR, "xfree received NULL\n");
#endif
}
