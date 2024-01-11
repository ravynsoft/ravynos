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

#include <config.h>
#ifdef HAVE_GDBM
#include <gdbm.h>
#endif
#ifdef HAVE_DBM
#include <ndbm.h>
#include <fcntl.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>

#include <avahi-common/malloc.h>

#include "stdb.h"

#ifdef HAVE_GDBM
static GDBM_FILE gdbm_file = NULL;
#endif
#ifdef HAVE_DBM
static DBM *dbm_file = NULL;
#endif
static char *buffer = NULL;
static char *enum_key = NULL;

static int init(void) {

#ifdef HAVE_GDBM
    if (gdbm_file)
        return 0;

    if (!(gdbm_file = gdbm_open((char*) DATABASE_FILE, 0, GDBM_READER, 0, NULL)))
        return -1;
#endif
#ifdef HAVE_DBM
    if (dbm_file)
        return 0;

    if (!(dbm_file = dbm_open((char*) DATABASE_FILE, O_RDONLY, 0)))
        return -1;
#endif

    return 0;
}

const char* stdb_lookup(const char *name) {
    datum key, data;
    const char *loc;

    if (init() < 0)
        goto fail;

    data.dptr = NULL;
    data.dsize = 0;

    if ((loc = setlocale(LC_MESSAGES, NULL))) {
        char k[256];

        snprintf(k, sizeof(k), "%s[%s]", name, loc);
        key.dptr = k;
        key.dsize = strlen(k);
#ifdef HAVE_GDBM
        data = gdbm_fetch(gdbm_file, key);
#endif
#ifdef HAVE_DBM
        data = dbm_fetch(dbm_file, key);
#endif

        if (!data.dptr) {
            char l[32], *e;
            snprintf(l, sizeof(l), "%s", loc);

            if ((e = strchr(l, '@'))) {
                *e = 0;
                snprintf(k, sizeof(k), "%s[%s]", name, l);
                key.dptr = k;
                key.dsize = strlen(k);
#ifdef HAVE_GDBM
                data = gdbm_fetch(gdbm_file, key);
#endif
#ifdef HAVE_DBM
                data = dbm_fetch(dbm_file, key);
#endif
            }

            if (!data.dptr) {
                if ((e = strchr(l, '_'))) {
                    *e = 0;
                    snprintf(k, sizeof(k), "%s[%s]", name, l);
                    key.dptr = k;
                    key.dsize = strlen(k);
#ifdef HAVE_GDBM
                    data = gdbm_fetch(gdbm_file, key);
#endif
#ifdef HAVE_DBM
                    data = dbm_fetch(dbm_file, key);
#endif
                }
            }
        }
    }

    if (!data.dptr) {
        key.dptr = (char*) name;
        key.dsize = strlen(name);
#ifdef HAVE_GDBM
        data = gdbm_fetch(gdbm_file, key);
#endif
#ifdef HAVE_DBM
        data = dbm_fetch(dbm_file, key);
#endif
    }

    if (!data.dptr)
        goto fail;

    avahi_free(buffer);
    buffer = avahi_strndup(data.dptr, data.dsize);
    free(data.dptr);

    return buffer;

fail:

    return name;
}

void stdb_shutdown(void) {
#ifdef HAVE_GDBM
    if (gdbm_file)
        gdbm_close(gdbm_file);

    gdbm_file = NULL;
#endif
#ifdef HAVE_DBM
    if (dbm_file)
        dbm_close(dbm_file);

    dbm_file = NULL;
#endif

    avahi_free(buffer);
    avahi_free(enum_key);

    buffer = enum_key = NULL;
}

char *stdb_getent(void) {
    datum key;

    if (init() < 0)
        return NULL;

    for (;;) {

        if (!enum_key) {
#ifdef HAVE_GDBM
            key = gdbm_firstkey(gdbm_file);
#endif
#ifdef HAVE_DBM
            key = dbm_firstkey(dbm_file);
#endif
        } else {
            key.dptr = enum_key;
            key.dsize = strlen(enum_key);

#ifdef HAVE_GDBM
            key = gdbm_nextkey(gdbm_file, key);
#endif
#ifdef HAVE_DBM
            key = dbm_nextkey(dbm_file);
#endif
        }

        avahi_free(enum_key);
        enum_key = NULL;

        if (!key.dptr)
            return NULL;

        enum_key = avahi_strndup(key.dptr, key.dsize);
        free(key.dptr);

        if (!strchr(enum_key, '['))
            return enum_key;
    }
}

void stdb_setent(void) {
    avahi_free(enum_key);
    enum_key = NULL;
}
