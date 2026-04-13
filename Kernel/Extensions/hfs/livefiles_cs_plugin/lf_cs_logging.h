//
// Copyright (c) 2009-2019 Apple Inc. All rights reserved.
//
// lf_cs_logging.h - Defines for helper methods for logging info, error,
//                   warning and debug messages for livefiles Apple_CoreStorage
//                   plugin.
//
#ifndef _LF_CS_LOGGING_H
#define _LF_CS_LOGGING_H

#if LF_CS_USE_OSLOG
#include <os/log.h>

#define debugmsg(fmt, ...) \
	os_log_debug(OS_LOG_DEFAULT, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define infomsg(fmt, ...)  \
	os_log_info(OS_LOG_DEFAULT, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define warnmsg(fmt, ...) \
	os_log(OS_LOG_DEFAULT, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define errmsg(fmt, ...) \
	os_log_error(OS_LOG_DEFAULT, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

#else /* !LF_CS_USE_OSLOG */

void log_debug(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void log_info(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void log_warn(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void log_err(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

#define debugmsg(fmt, ...) \
	log_debug("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define infomsg(fmt, ...) \
	log_info("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define warnmsg(fmt, ...) \
	log_warn("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define errmsg(fmt, ...) \
	log_err("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

#endif /* LF_CS_USE_OSLOG */
#endif /* _LF_CS_LOGGING_H */
