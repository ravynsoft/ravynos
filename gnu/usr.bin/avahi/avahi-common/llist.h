#ifndef foollistfoo
#define foollistfoo

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

/** \file llist.h A simple macro based linked list implementation */

#include <assert.h>

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

/** The head of the linked list. Use this in the structure that shall
 * contain the head of the linked list */
#define AVAHI_LLIST_HEAD(t,name) t *name

/** The pointers in the linked list's items. Use this in the item structure */
#define AVAHI_LLIST_FIELDS(t,name) t *name##_next, *name##_prev

/** Initialize the list's head */
#define AVAHI_LLIST_HEAD_INIT(t,head) do { (head) = NULL; } while(0)

/** Initialize a list item */
#define AVAHI_LLIST_INIT(t,name,item) do { \
                               t *_item = (item); \
                               assert(_item); \
                               _item->name##_prev = _item->name##_next = NULL; \
                               } while(0)

/** Prepend an item to the list */
#define AVAHI_LLIST_PREPEND(t,name,head,item) do { \
                                        t **_head = &(head), *_item = (item); \
                                        assert(_item); \
                                        if ((_item->name##_next = *_head)) \
                                           _item->name##_next->name##_prev = _item; \
                                        _item->name##_prev = NULL; \
                                        *_head = _item; \
                                        } while (0)

/** Remove an item from the list */
#define AVAHI_LLIST_REMOVE(t,name,head,item) do { \
                                    t **_head = &(head), *_item = (item); \
                                    assert(_item); \
                                    if (_item->name##_next) \
                                       _item->name##_next->name##_prev = _item->name##_prev; \
                                    if (_item->name##_prev) \
                                       _item->name##_prev->name##_next = _item->name##_next; \
                                    else {\
                                       assert(*_head == _item); \
                                       *_head = _item->name##_next; \
                                    } \
                                    _item->name##_next = _item->name##_prev = NULL; \
                                    } while(0)

AVAHI_C_DECL_END

#endif
