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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#include <avahi-common/malloc.h>

#include "setproctitle.h"

#if !defined(HAVE_SETPROCTITLE) && defined(__linux__)
static char** argv_buffer = NULL;
static size_t argv_size = 0;

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif

#endif

void avahi_init_proc_title(int argc, char **argv) {

#if !defined(HAVE_SETPROCTITLE) && defined(__linux__)

    unsigned i;
    char **new_environ, *endptr;

    /* This code is really really ugly. We make some memory layout
     * assumptions and reuse the environment array as memory to store
     * our process title in */

    for (i = 0; environ[i]; i++);

    endptr = i ? environ[i-1] + strlen(environ[i-1]) : argv[argc-1] + strlen(argv[argc-1]);

    argv_buffer = argv;
    argv_size = endptr - argv_buffer[0];

    /* Make a copy of environ */

    new_environ = avahi_malloc(sizeof(char*) * (i + 1));
    for (i = 0; environ[i]; i++)
        new_environ[i] = avahi_strdup(environ[i]);
    new_environ[i] = NULL;

    environ = new_environ;

#endif
}

void avahi_set_proc_title(const char *name, const char *fmt,...) {
#ifdef HAVE_SETPROCTITLE
    char t[256];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(t, sizeof(t), fmt, ap);
    va_end(ap);

    setproctitle("-%s", t);
#elif defined(__linux__)
    size_t l;
    va_list ap;

    if (!argv_buffer)
        return;

    va_start(ap, fmt);
    vsnprintf(argv_buffer[0], argv_size, fmt, ap);
    va_end(ap);

    l = strlen(argv_buffer[0]);

    memset(argv_buffer[0] + l, 0, argv_size - l);
    argv_buffer[1] = NULL;
#endif

#if defined(HAVE_SYS_PRCTL_H) && defined(PR_SET_NAME)

    if (name)
        prctl(PR_SET_NAME, (unsigned long) name, 0, 0, 0);

#endif
}
