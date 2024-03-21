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
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <avahi-common/malloc.h>
#include <avahi-core/log.h>

#include "ini-file-parser.h"

AvahiIniFile* avahi_ini_file_load(const char *fname) {
    AvahiIniFile *f;
    FILE *fo;
    AvahiIniFileGroup *group = NULL;
    unsigned line;

    assert(fname);

    if (!(fo = fopen(fname, "r"))) {
        avahi_log_error("Failed to open file '%s': %s", fname, strerror(errno));
        return NULL;
    }

    f = avahi_new(AvahiIniFile, 1);
    AVAHI_LLIST_HEAD_INIT(AvahiIniFileGroup, f->groups);
    f->n_groups = 0;

    line = 0;
    while (!feof(fo)) {
        char ln[256], *s, *e;
        AvahiIniFilePair *pair;

        if (!(fgets(ln, sizeof(ln), fo)))
            break;

        line++;

        s = ln + strspn(ln, " \t");
        s[strcspn(s, "\r\n")] = 0;

        /* Skip comments and empty lines */
        if (*s == '#' || *s == '%' || *s == 0)
            continue;

        if (*s == '[') {
            /* new group */

            if (!(e = strchr(s, ']'))) {
                avahi_log_error("Unclosed group header in %s:%u: <%s>", fname, line, s);
                goto fail;
            }

            *e = 0;

            group = avahi_new(AvahiIniFileGroup, 1);
            group->name = avahi_strdup(s+1);
            group->n_pairs = 0;
            AVAHI_LLIST_HEAD_INIT(AvahiIniFilePair, group->pairs);

            AVAHI_LLIST_PREPEND(AvahiIniFileGroup, groups, f->groups, group);
            f->n_groups++;
        } else {

            /* Normal assignment */
            if (!(e = strchr(s, '='))) {
                avahi_log_error("Missing assignment in %s:%u: <%s>", fname, line, s);
                goto fail;
            }

            if (!group) {
                avahi_log_error("Assignment outside group in %s:%u <%s>", fname, line, s);
                goto fail;
            }

            /* Split the key and the value */
            *(e++) = 0;

            pair = avahi_new(AvahiIniFilePair, 1);
            pair->key = avahi_strdup(s);
            pair->value = avahi_strdup(e);

            AVAHI_LLIST_PREPEND(AvahiIniFilePair, pairs, group->pairs, pair);
            group->n_pairs++;
        }
    }

    fclose(fo);

    return f;

fail:

    if (fo)
        fclose(fo);

    if (f)
        avahi_ini_file_free(f);

    return NULL;
}

void avahi_ini_file_free(AvahiIniFile *f) {
    AvahiIniFileGroup *g;
    assert(f);

    while ((g = f->groups)) {
        AvahiIniFilePair *p;

        while ((p = g->pairs)) {
            avahi_free(p->key);
            avahi_free(p->value);

            AVAHI_LLIST_REMOVE(AvahiIniFilePair, pairs, g->pairs, p);
            avahi_free(p);
        }

        avahi_free(g->name);

        AVAHI_LLIST_REMOVE(AvahiIniFileGroup, groups, f->groups, g);
        avahi_free(g);
    }

    avahi_free(f);
}

char** avahi_split_csv(const char *t) {
    unsigned n_comma = 0;
    const char *p;
    char **r, **i;

    for (p = t; *p; p++)
        if (*p == ',')
            n_comma++;

    i = r = avahi_new(char*, n_comma+2);

    for (;;) {
        size_t n, l = strcspn(t, ",");
        const char *c;

        /* Ignore leading blanks */
        for (c = t, n = l; isblank(*c); c++, n--);

        /* Ignore trailing blanks */
        for (; n > 0 && isblank(c[n-1]); n--);

        *(i++) = avahi_strndup(c, n);

        t += l;

        if (*t == 0)
            break;

        assert(*t == ',');
        t++;
    }

    *i = NULL;

    return r;
}

void avahi_strfreev(char **p) {
    char **i;

    if (!p)
        return;

    for (i = p; *i; i++)
        avahi_free(*i);

    avahi_free(p);
}
