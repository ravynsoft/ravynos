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

#include <avahi-common/malloc.h>

#include "ini-file-parser.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {

    AvahiIniFile *f;
    AvahiIniFileGroup *g;

    if (!(f = avahi_ini_file_load("avahi-daemon.conf"))) {
        return 1;
    }

    printf("%u groups\n", f->n_groups);

    for (g = f->groups; g; g = g->groups_next) {
        AvahiIniFilePair *p;
        printf("<%s> (%u pairs)\n", g->name, g->n_pairs);

        for (p = g->pairs; p; p = p->pairs_next) {
            char **split, **i;

            printf("\t<%s> = ", p->key);
            split = avahi_split_csv(p->value);

            for (i = split; *i; i++)
                printf("<%s> ", *i);

            avahi_strfreev(split);

            printf("\n");
        }
    }

    avahi_ini_file_free(f);
    return 0;
}
