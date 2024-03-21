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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "log.h"

static AvahiLogFunction log_function = NULL;

void avahi_set_log_function(AvahiLogFunction function) {
    log_function = function;
}

void avahi_log_ap(AvahiLogLevel level, const char*format, va_list ap) {
    char txt[256];

    vsnprintf(txt, sizeof(txt), format, ap);

    if (log_function)
        log_function(level, txt);
    else
        fprintf(stderr, "%s\n", txt);
}

void avahi_log(AvahiLogLevel level, const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(level, format, ap);
    va_end(ap);
}

void avahi_log_error(const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(AVAHI_LOG_ERROR, format, ap);
    va_end(ap);
}

void avahi_log_warn(const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(AVAHI_LOG_WARN, format, ap);
    va_end(ap);
}

void avahi_log_notice(const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(AVAHI_LOG_NOTICE, format, ap);
    va_end(ap);
}

void avahi_log_info(const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(AVAHI_LOG_INFO, format, ap);
    va_end(ap);
}

void avahi_log_debug(const char*format, ...) {
    va_list ap;
    va_start(ap, format);
    avahi_log_ap(AVAHI_LOG_DEBUG, format, ap);
    va_end(ap);
}
