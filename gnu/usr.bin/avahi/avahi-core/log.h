#ifndef foologhfoo
#define foologhfoo

/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <stdarg.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/gccmacro.h>

/** \file log.h Extensible logging subsystem */

AVAHI_C_DECL_BEGIN

/** Log level for avahi_log_xxx() */
typedef enum {
    AVAHI_LOG_ERROR  = 0,    /**< Error messages */
    AVAHI_LOG_WARN   = 1,    /**< Warning messages */
    AVAHI_LOG_NOTICE = 2,    /**< Notice messages */
    AVAHI_LOG_INFO   = 3,    /**< Info messages */
    AVAHI_LOG_DEBUG  = 4,    /**< Debug messages */
    AVAHI_LOG_LEVEL_MAX
} AvahiLogLevel;

/** Prototype for a user supplied log function */
typedef void (*AvahiLogFunction)(AvahiLogLevel level, const char *txt);

/** Set a user supplied log function, replacing the default which
 * prints to log messages unconditionally to STDERR. Pass NULL for
 * resetting to the default log function */
void avahi_set_log_function(AvahiLogFunction function);

/** Issue a log message using a va_list object */
void avahi_log_ap(AvahiLogLevel level, const char *format, va_list ap);

/** Issue a log message by passing a log level and a format string */
void avahi_log(AvahiLogLevel level, const char*format, ...) AVAHI_GCC_PRINTF_ATTR23;

/** Shortcut for avahi_log(AVAHI_LOG_ERROR, ...) */
void avahi_log_error(const char*format, ...) AVAHI_GCC_PRINTF_ATTR12;

/** Shortcut for avahi_log(AVAHI_LOG_WARN, ...) */
void avahi_log_warn(const char*format, ...) AVAHI_GCC_PRINTF_ATTR12;

/** Shortcut for avahi_log(AVAHI_LOG_NOTICE, ...) */
void avahi_log_notice(const char*format, ...) AVAHI_GCC_PRINTF_ATTR12;

/** Shortcut for avahi_log(AVAHI_LOG_INFO, ...) */
void avahi_log_info(const char*format, ...) AVAHI_GCC_PRINTF_ATTR12;

/** Shortcut for avahi_log(AVAHI_LOG_DEBUG, ...) */
void avahi_log_debug(const char*format, ...) AVAHI_GCC_PRINTF_ATTR12;

AVAHI_C_DECL_END

#endif
