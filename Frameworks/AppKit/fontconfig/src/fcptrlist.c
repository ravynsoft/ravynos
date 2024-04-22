/*
 * fontconfig/src/fcptrlist.c
 *
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"

typedef struct _FcPtrListEntry {
    struct _FcPtrListEntry	*next;
    void			*data;
} FcPtrListEntry;
struct _FcPtrList {
    FcDestroyFunc	destroy_func;
    FcPtrListEntry	*list;
};
typedef struct _FcPtrListIterPrivate {
    const FcPtrList	*list;
    FcPtrListEntry	*entry;
    FcPtrListEntry	*prev;
} FcPtrListIterPrivate;

FcPtrList *
FcPtrListCreate (FcDestroyFunc func)
{
    FcPtrList *ret = (FcPtrList *) malloc (sizeof (FcPtrList));

    if (ret)
    {
	ret->destroy_func = func;
	ret->list = NULL;
    }

    return ret;
}

void
FcPtrListDestroy (FcPtrList *list)
{
    FcPtrListIter iter;

    if (list)
    {
	FcPtrListIterInit (list, &iter);
	do
	{
	    if (FcPtrListIterGetValue (list, &iter))
		list->destroy_func (FcPtrListIterGetValue (list, &iter));
	    FcPtrListIterRemove (list, &iter);
	} while (FcPtrListIterIsValid (list, &iter));

	free (list);
    }
}

void
FcPtrListIterInit (const FcPtrList	*list,
		 FcPtrListIter		*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;

    priv->list = list;
    priv->entry = list->list;
    priv->prev = NULL;
}

void
FcPtrListIterInitAtLast (FcPtrList	*list,
		       FcPtrListIter	*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;
    FcPtrListEntry **e, **p;

    e = &list->list;
    p = e;
    for (; *e; p = e, e = &(*e)->next);

    priv->list = list;
    priv->entry = *e;
    priv->prev = *p;
}

FcBool
FcPtrListIterNext (const FcPtrList	*list,
		 FcPtrListIter		*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;

    if (list != priv->list)
	return FcFalse;
    priv->prev = priv->entry;
    priv->entry = priv->entry->next;

    return priv->entry != NULL;
}

FcBool
FcPtrListIterIsValid (const FcPtrList	*list,
		    const FcPtrListIter	*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;

    return list == priv->list && priv->entry;
}

void *
FcPtrListIterGetValue (const FcPtrList		*list,
		     const FcPtrListIter	*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;

    if (list != priv->list ||
	!priv->entry)
	return NULL;

    return priv->entry->data;
}

FcBool
FcPtrListIterAdd (FcPtrList	*list,
		FcPtrListIter	*iter,
		void		*data)
{
    FcPtrListEntry *e;
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;

    if (list != priv->list)
	return FcFalse;

    e = (FcPtrListEntry *) malloc (sizeof (FcPtrListEntry));
    if (!e)
	return FcFalse;
    e->data = data;

    if (priv->entry)
    {
	e->next = priv->entry->next;
	priv->entry->next = e;
    }
    else
    {
	e->next = NULL;
	if (priv->prev)
	{
	    priv->prev->next = e;
	    priv->entry = priv->prev;
	}
	else
	{
	    list->list = e;
	    priv->entry = e;

	    return FcTrue;
	}
    }

    return FcPtrListIterNext (list, iter);
}

FcBool
FcPtrListIterRemove (FcPtrList		*list,
		   FcPtrListIter	*iter)
{
    FcPtrListIterPrivate *priv = (FcPtrListIterPrivate *) iter;
    FcPtrListEntry *e;

    if (list != priv->list)
	return FcFalse;
    if (!priv->entry)
	return FcTrue;

    if (list->list == priv->entry)
	list->list = list->list->next;
    e = priv->entry;
    if (priv->prev)
	priv->prev->next = priv->entry->next;
    priv->entry = priv->entry->next;
    free (e);

    return FcTrue;
}

#define __fcplist__
#include "fcaliastail.h"
#undef __fcplist__
