/* logging.h ; MaWiM logger header
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause License; See the LICENSE file for further
 * information.
 */

#ifndef LOGGING_H
#define LOGGING_H

typedef enum log_level {
  LOG_INVALID = -1,
  LOG_DEBUG = 0,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
} log_level_t;

#if !defined(DEFAULT_LOG_LEVEL)
#define DEFAULT_LOG_LEVEL LOG_INFO
#endif

extern log_level_t mawim_log_level;

log_level_t str_to_loglvl(char *str);

int mawim_logf(log_level_t level, const char *format, ...);
int mawim_logf_noprefix(log_level_t level, const char *format, ...);
void mawim_log(int level, char *msg);

#endif /* #ifndef LOGGING_H */
