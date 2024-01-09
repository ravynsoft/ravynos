/*
 * linklist.c - linked lists
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "linklist.pro"

/*
 * Anatomy of a LinkList
 *
 * LinkList with 4 nodes:
 *
 * LinkList is a        first   last   flags   (LinkList)
 * union; see zsh.h     next    prev   dat     (LinkNode)
 *                    +-------+------+------+
 *                    |       |      |      | See comment in subst.c
 *     +------------> |   |   |   |  |      | about LF_ARRAY.
 *     |              +---|---+---|--+------+
 *     |                  |       |
 *     |     +------------+       +--------------+
 *     |     |                                   |
 *     |    \|/                                 \|/
 *     |   +----+      +----+      +----+      +----+
 *     |   |    |      |    |      |    |      | \/ |  X here is NULL.
 *     |   |  -------> |  -------> |  -------> | /\ |
 *     |   |next|      |next|      |next|      |next|
 *     |   +----+      +----+      +----+      +----+
 *     |   |    |      |    |      |    |      |    |
 *     +------  | <-------  | <-------  | <-------  |
 *         |prev|      |prev|      |prev|      |prev|
 *         +----+      +----+      +----+      +----+
 *         |    |      |    |      |    |      |    | Pointers to data,
 *         |dat |      |dat |      |dat |      |dat | usually char **.
 *         +----+      +----+      +----+      +----+
 *        LinkNode    LinkNode    LinkNode    LinkNode
 *
 *
 * Empty LinkList:
 *                    first   last   flags
 *                   +------+------+-------+
 *             +---> | NULL |      |   0   |
 *             |     |      |   |  |       |
 *             |     +------+---|--+-------+
 *             |                |
 *             +----------------+
 *
 * Traversing a LinkList:
 * Traversing forward through a list uses an iterator-style paradigm.
 * for (LinkNode node = firstnode(list); node; incnode(node)) {
 *     // Access/manipulate the node using macros (see zsh.h)
 * }
 *
 * Traversing backwards is the same, with a small caveat.
 * for (LinkNode node = lastnode(list); node != &list->node; decnode(node)) {
 *     // The loop condition should be obvious given the above diagrams.
 * }
 *
 * If you're going to be moving back and forth, best to AND both
 * conditions.
 *
 * while (node && node != &list->node) {
 *     // If both incnode(list) and decnode(list) are used, and it's
 *     // unknown at which end of the list traversal will stop.
 * }
 *
 * Macros and functions prefixed with 'z' (ie znewlinklist,
 * zinsertlinknode) use permanent allocation, which you have to free
 * manually (with freelinklist(), maybe?). Non-z-prefixed
 * macros/functions allocate from heap, which will be automatically
 * freed.
 *
 */

/* Get an empty linked list header */

/**/
mod_export LinkList
newlinklist(void)
{
    LinkList list;

    list = (LinkList) zhalloc(sizeof *list);
    list->list.first = NULL;
    list->list.last = &list->node;
    list->list.flags = 0;
    return list;
}

/**/
mod_export LinkList
znewlinklist(void)
{
    LinkList list;

    list = (LinkList) zalloc(sizeof *list);
    if (!list)
	return NULL;
    list->list.first = NULL;
    list->list.last = &list->node;
    list->list.flags = 0;
    return list;
}

/* Insert a node in a linked list after a given node */

/**/
mod_export LinkNode
insertlinknode(LinkList list, LinkNode node, void *dat)
{
    LinkNode tmp, new;

    tmp = node->next;
    node->next = new = (LinkNode) zhalloc(sizeof *tmp);
    new->prev = node;
    new->dat = dat;
    new->next = tmp;
    if (tmp)
	tmp->prev = new;
    else
	list->list.last = new;
    return new;
}

/**/
mod_export LinkNode
zinsertlinknode(LinkList list, LinkNode node, void *dat)
{
    LinkNode tmp, new;

    tmp = node->next;
    node->next = new = (LinkNode) zalloc(sizeof *tmp);
    if (!new)
	return NULL;
    new->prev = node;
    new->dat = dat;
    new->next = tmp;
    if (tmp)
	tmp->prev = new;
    else
	list->list.last = new;
    return new;
}

/* Insert an already-existing node into a linked list after a given node */

/**/
mod_export LinkNode
uinsertlinknode(LinkList list, LinkNode node, LinkNode new)
{
    LinkNode tmp = node->next;
    node->next = new;
    new->prev = node;
    new->next = tmp;
    if (tmp)
	tmp->prev = new;
    else
	list->list.last = new;
    return new;
}

/* Insert a list in another list */

/**/
mod_export void
insertlinklist(LinkList l, LinkNode where, LinkList x)
{
    LinkNode nx;

    nx = where->next;
    if (!firstnode(l))
	return;
    where->next = firstnode(l);
    l->list.last->next = nx;
    l->list.first->prev = where;
    if (nx)
	nx->prev = lastnode(l);
    else
	x->list.last = lastnode(l);
}

/* Pop the top node off a linked list and free it. */

/**/
mod_export void *
getlinknode(LinkList list)
{
    void *dat;
    LinkNode node;

    if (!(node = firstnode(list)))
	return NULL;
    dat = node->dat;
    list->list.first = node->next;
    if (node->next)
	node->next->prev = &list->node;
    else
	list->list.last = &list->node;
    zfree(node, sizeof *node);
    return dat;
}

/* Pop the top node off a linked list without freeing it. */

/**/
mod_export void *
ugetnode(LinkList list)
{
    void *dat;
    LinkNode node;

    if (!(node = firstnode(list)))
	return NULL;
    dat = node->dat;
    list->list.first = node->next;
    if (node->next)
	node->next->prev = &list->node;
    else
	list->list.last = &list->node;
    return dat;
}

/* Remove a node from a linked list */

/**/
mod_export void *
remnode(LinkList list, LinkNode nd)
{
    void *dat;

    nd->prev->next = nd->next;
    if (nd->next)
	nd->next->prev = nd->prev;
    else
	list->list.last = nd->prev;
    dat = nd->dat;
    zfree(nd, sizeof *nd);

    return dat;
}

/* Remove a node from a linked list without freeing */

/**/
mod_export void *
uremnode(LinkList list, LinkNode nd)
{
    void *dat;

    nd->prev->next = nd->next;
    if (nd->next)
	nd->next->prev = nd->prev;
    else
	list->list.last = nd->prev;
    dat = nd->dat;
    return dat;
}

/* Free a linked list */

/**/
mod_export void
freelinklist(LinkList list, FreeFunc freefunc)
{
    LinkNode node, next;

    for (node = firstnode(list); node; node = next) {
	next = node->next;
	if (freefunc)
	    freefunc(node->dat);
	zfree(node, sizeof *node);
    }
    zfree(list, sizeof *list);
}

/* Count the number of nodes in a linked list */

/**/
mod_export int
countlinknodes(LinkList list)
{
    LinkNode nd;
    int ct = 0;

    for (nd = firstnode(list); nd; incnode(nd), ct++);
    return ct;
}

/* Make specified node first, moving preceding nodes to end */

/**/
mod_export void
rolllist(LinkList l, LinkNode nd)
{
    l->list.last->next = firstnode(l);
    l->list.first->prev = lastnode(l);
    l->list.first = nd;
    l->list.last = nd->prev;
    nd->prev = &l->node;
    l->list.last->next = 0;
}

/* Create linklist of specified size. node->dats are not initialized. */

/**/
mod_export LinkList
newsizedlist(int size)
{
    LinkList list;
    LinkNode node;

    list = (LinkList) zhalloc(sizeof *list + (size * sizeof *node));

    list->list.first = &list[1].node;
    for (node = firstnode(list); size; size--, node++) {
	node->prev = node - 1;
	node->next = node + 1;
    }
    list->list.last = node - 1;
    list->list.first->prev = &list->node;
    node[-1].next = NULL;

    return list;
}

/*
 * Join two linked lists.  Neither may be null, though either
 * may be empty.
 *
 * It is assumed the pieces come from the heap, but if not it is
 * safe to free LinkList second.
 */

/**/
mod_export LinkList
joinlists(LinkList first, LinkList second)
{
    LinkNode moveme = firstnode(second);
    if (moveme) {
	if (firstnode(first)) {
	    LinkNode anchor = lastnode(first);
	    anchor->next = moveme;
	    moveme->prev = anchor;
	} else {
	    first->list.first = moveme;
	    moveme->prev = &first->node;
	}
	first->list.last = second->list.last;

	second->list.first = second->list.last = NULL;
    }
    return first;
}

/*
 * Return the node whose data is the pointer "dat", else NULL.
 * Can be used as a boolean test.
 */

/**/
mod_export LinkNode
linknodebydatum(LinkList list, void *dat)
{
    LinkNode node;

    for (node = firstnode(list); node; incnode(node))
	if (getdata(node) == dat)
	    return node;

    return NULL;
}

/*
 * Return the node whose data matches the string "dat", else NULL.
 */

/**/
mod_export LinkNode
linknodebystring(LinkList list, char *dat)
{
    LinkNode node;

    for (node = firstnode(list); node; incnode(node))
	if (!strcmp((char *)getdata(node), dat))
	    return node;

    return NULL;
}

/*
 * Convert a linked list whose data elements are strings to
 * an array.  Memory is off the heap and the elements of the
 * array are the same elements as the linked list data if copy is
 * 0, else copied onto the heap.
 */

/**/
mod_export char **
hlinklist2array(LinkList list, int copy)
{
    int l = countlinknodes(list);
    char **ret = (char **) zhalloc((l + 1) * sizeof(char *)), **p;
    LinkNode n;

    for (n = firstnode(list), p = ret; n; incnode(n), p++) {
	*p = (char *) getdata(n);
	if (copy)
	    *p = dupstring(*p);
    }
    *p = NULL;

    return ret;
}

/*
 * Convert a linked list whose data elements are strings to
 * a permanently-allocated array.  The elements of the array are the same
 * elements as the linked list data if copy is 0, else they are duplicated
 * into permanent memory so the result is a permanently allocated,
 * freearrayable array that's a deep copy of the linked list.
 */

/**/
mod_export char **
zlinklist2array(LinkList list, int copy)
{
    int l = countlinknodes(list);
    char **ret = (char **) zalloc((l + 1) * sizeof(char *)), **p;
    LinkNode n;

    for (n = firstnode(list), p = ret; n; incnode(n), p++) {
	*p = (char *) getdata(n);
	if (copy)
	    *p = ztrdup(*p);
    }
    *p = NULL;

    return ret;
}

