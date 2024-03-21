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

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/prctl.h>

#include <avahi-core/log.h>

#include "caps.h"

int avahi_caps_reduce(void) {
    int ret = 0;
    cap_t caps;
    static cap_value_t cap_values[] = { CAP_SYS_CHROOT, CAP_SETUID, CAP_SETGID };

    /* Let's reduce our caps to the minimum set and tell Linux to keep
     * them across setuid(). This is called before we drop
     * privileges. */

    caps = cap_init();
    assert(caps);
    cap_clear(caps);

    cap_set_flag(caps, CAP_EFFECTIVE, 3, cap_values, CAP_SET);
    cap_set_flag(caps, CAP_PERMITTED, 3, cap_values, CAP_SET);

    if (cap_set_proc(caps) < 0) {
        avahi_log_error("cap_set_proc() failed: %s", strerror(errno));
        ret = -1;
    }
    cap_free(caps);

    /* Retain capabilities across setuid() */
    if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) < 0) {
        avahi_log_error("prctl(PR_SET_KEEPCAPS) failed: %s", strerror(errno));
        ret = -1;
    }

    return ret;
}

int avahi_caps_reduce2(void) {
    int ret = 0;
    cap_t caps;
    static cap_value_t cap_values[] = { CAP_SYS_CHROOT };

    /* Reduce our caps to the bare minimum and tell Linux not to keep
     * them across setuid(). This is called after we drop
     * privileges. */

    /* No longer retain caps across setuid() */
    if (prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0) < 0) {
        avahi_log_error("prctl(PR_SET_KEEPCAPS) failed: %s", strerror(errno));
        ret = -1;
    }

    caps = cap_init();
    assert(caps);
    cap_clear(caps);

    /* setuid() zeroed our effective caps, let's get them back */
    cap_set_flag(caps, CAP_EFFECTIVE, 1, cap_values, CAP_SET);
    cap_set_flag(caps, CAP_PERMITTED, 1, cap_values, CAP_SET);

    if (cap_set_proc(caps) < 0) {
        avahi_log_error("cap_set_proc() failed: %s", strerror(errno));
        ret = -1;
    }
    cap_free(caps);

    return ret;
}

int avahi_caps_drop_all(void) {
    cap_t caps;
    int ret = 0;

    /* Drop all capabilities and turn ourselves into a normal user process */

    caps = cap_init();
    assert(caps);
    cap_clear(caps);

    if (cap_set_proc(caps) < 0) {
        avahi_log_error("cap_set_proc() failed: %s", strerror(errno));
        ret = -1;
    }
    cap_free(caps);

    return ret;
}
