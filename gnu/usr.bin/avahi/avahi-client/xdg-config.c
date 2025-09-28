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

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "xdg-config.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

FILE *avahi_xdg_config_open(const char *filename) {
    FILE *f;
    const char *e, *d;
    char fn[PATH_MAX], *p = NULL, buf[2048], *s = NULL;

    assert(filename);

    if ((e = getenv("XDG_CONFIG_HOME")) && *e)
        snprintf(p = fn, sizeof(fn), "%s/%s", e, filename);
    else if ((e = getenv("HOME")) && *e)
        snprintf(p = fn, sizeof(fn), "%s/.config/%s", e, filename);

    if (p) {
        if ((f = fopen(p, "r")))
            return f;
        else if (errno != ENOENT)
            return NULL;
    }

    if (!(d = getenv("XDG_CONFIG_DIRS")) || !*d)
        d = "/etc/xdg";

    snprintf(buf, sizeof(buf), "%s", d);

    for (e = strtok_r(buf, ":", &s); e; e = strtok_r(NULL, ":", &s)) {
        snprintf(fn, sizeof(fn), "%s/%s", e, filename);

        if ((f = fopen(fn, "r")))
            return f;
    }

    return NULL;
}
