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

#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <syslog.h>

#include "warn.h"

#ifndef COMPAT_LAYER
#define COMPAT_LAYER "Apple Bonjour"
#endif

#ifndef CGI_SUBSYSTEM
#define CGI_SUBSYSTEM "libdns_sd"
#endif

static pthread_mutex_t linkage_mutex = PTHREAD_MUTEX_INITIALIZER;
static int linkage_warning = 0;

const char *avahi_exe_name(void) {
#ifdef HAVE_GETPROGNAME
    return getprogname();
#elif defined(__linux__)
    static char exe_name[1024] = "";
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Yes, I know, this is not portable. But who cares? It's for
     * cosmetics only, anyway. */

    pthread_mutex_lock(&mutex);

    if (exe_name[0] == 0) {
        int k;

        if ((k = readlink("/proc/self/exe", exe_name, sizeof(exe_name)-1)) < 0)
            snprintf(exe_name, sizeof(exe_name), "(unknown)");
        else {
            char *slash;

            assert((size_t) k <= sizeof(exe_name)-1);
            exe_name[k] = 0;

            if ((slash = strrchr(exe_name, '/')))
                memmove(exe_name, slash+1, strlen(slash)+1);
        }
    }

    pthread_mutex_unlock(&mutex);

    return exe_name;
#else
#ifdef __GNUC__
#warning "avahi_exe_name() needs to be implemented for your operating system"
#endif
    return "(unknown)";
#endif
}

void avahi_warn(const char *fmt, ...) {
    char msg[512]  = "*** WARNING *** ";
    va_list ap;
    size_t n;

    assert(fmt);

    va_start(ap, fmt);
    n = strlen(msg);
    vsnprintf(msg + n, sizeof(msg) - n, fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s\n", msg);

    openlog(avahi_exe_name(), LOG_PID, LOG_USER);
    syslog(LOG_WARNING, "%s", msg);
    closelog();
}

void avahi_warn_linkage(void) {
    int w;

    pthread_mutex_lock(&linkage_mutex);
    w = linkage_warning;
    linkage_warning = 1;
    pthread_mutex_unlock(&linkage_mutex);

    if (!w && !getenv("AVAHI_COMPAT_NOWARN")) {
        avahi_warn("The program '%s' uses the "COMPAT_LAYER" compatibility layer of Avahi.", avahi_exe_name());
        avahi_warn("Please fix your application to use the native API of Avahi!");
        avahi_warn("For more information see <http://0pointer.de/blog/projects/avahi-compat.html>");
    }
}

void avahi_warn_unsupported(const char *function) {
    avahi_warn("The program '%s' called '%s()' which is not supported (or only supported partially) in the "COMPAT_LAYER" compatibility layer of Avahi.", avahi_exe_name(), function);
    avahi_warn("Please fix your application to use the native API of Avahi!");
    avahi_warn("For more information see <http://0pointer.de/blog/projects/avahi-compat.html>");
}
