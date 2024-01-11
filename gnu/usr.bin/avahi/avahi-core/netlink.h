#ifndef foonetlinkhfoo
#define foonetlinkhfoo

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

#include <sys/socket.h>
#include <asm/types.h>
#include <inttypes.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <avahi-common/watch.h>

typedef struct AvahiNetlink AvahiNetlink;

typedef void (*AvahiNetlinkCallback)(AvahiNetlink *n, struct nlmsghdr *m, void* userdata);

AvahiNetlink *avahi_netlink_new(const AvahiPoll *poll_api, uint32_t groups, AvahiNetlinkCallback callback, void* userdata);
void avahi_netlink_free(AvahiNetlink *n);
int avahi_netlink_send(AvahiNetlink *n, struct nlmsghdr *m, unsigned *ret_seq);
int avahi_netlink_work(AvahiNetlink *n, int block);

#endif
