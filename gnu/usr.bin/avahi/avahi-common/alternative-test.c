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

#include "alternative.h"
#include "malloc.h"
#include "domain.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    const char* const test_strings[] = {
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXüüüüüüü",
        "gurke",
        "-",
        " #",
        "1",
        "#0",
        " #0",
        " #1",
        "#-1",
        " #-1",
        "-0",
        "--0",
        "-1",
        "--1",
        "-2",
        "gurke1",
        "gurke0",
        "gurke-2",
        "gurke #0",
        "gurke #1",
        "gurke #",
        "gurke#1",
        "gurke-",
        "gurke---",
        "gurke #",
        "gurke ###",
        NULL
    };

    char *r = NULL;
    int i, j, k;

    for (k = 0; test_strings[k]; k++) {

        printf(">>>>>%s<<<<\n", test_strings[k]);

        for (j = 0; j < 2; j++) {

            for (i = 0; i <= 100; i++) {
                char *n;

                n = i == 0 ? avahi_strdup(test_strings[k]) : (j ? avahi_alternative_service_name(r) : avahi_alternative_host_name(r));
                avahi_free(r);
                r = n;

                if (j)
                    assert(avahi_is_valid_service_name(n));
                else
                    assert(avahi_is_valid_host_name(n));

                printf("%s\n", r);
            }
        }
    }

    avahi_free(r);
    return 0;
}
