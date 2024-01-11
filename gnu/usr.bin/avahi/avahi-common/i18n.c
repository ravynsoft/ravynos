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

#include "i18n.h"

void avahi_init_i18n(void) {

    /* Not really thread safe, but this doesn't matter much since
     * bindtextdomain is supposed to be idempotent anyway. */

    static int done = 0;

    if (!done) {
        bindtextdomain(GETTEXT_PACKAGE, AVAHI_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        done = 1;
    }
}
