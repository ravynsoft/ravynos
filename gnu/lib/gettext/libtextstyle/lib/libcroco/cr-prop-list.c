/* libcroco - Library for parsing and applying CSS
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2003-2004 Dodji Seketeli.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * Author: Dodji Seketeli
 */

#include <config.h>
#include <string.h>
#include "cr-prop-list.h"

#define PRIVATE(a_obj) (a_obj)->priv

struct _CRPropListPriv {
        CRString *prop;
        CRDeclaration *decl;
        CRPropList *next;
        CRPropList *prev;
};

static CRPropList *cr_prop_list_allocate (void);

/**
 *Default allocator of CRPropList
 *@return the newly allocated CRPropList or NULL
 *if an error arises.
 */
static CRPropList *
cr_prop_list_allocate (void)
{
        CRPropList *result = NULL;

        result = g_try_malloc (sizeof (CRPropList));
        if (!result) {
                cr_utils_trace_info ("could not allocate CRPropList");
                return NULL;
        }
        memset (result, 0, sizeof (CRPropList));
        PRIVATE (result) = g_try_malloc (sizeof (CRPropListPriv));
        if (!result) {
                cr_utils_trace_info ("could not allocate CRPropListPriv");
                g_free (result);
                return NULL;
        }
        memset (PRIVATE (result), 0, sizeof (CRPropListPriv));
        return result;
}

/****************
 *public methods
 ***************/

/**
 * cr_prop_list_append:
 *@a_this: the current instance of #CRPropList
 *@a_to_append: the property list to append
 *
 *Appends a property list to the current one.
 *
 *Returns the resulting prop list, or NULL if an error
 *occurred
 */
CRPropList *
cr_prop_list_append (CRPropList * a_this, CRPropList * a_to_append)
{
        CRPropList *cur = NULL;

        g_return_val_if_fail (a_to_append, NULL);

        if (!a_this)
                return a_to_append;

        /*go fetch the last element of the list */
        for (cur = a_this;
             cur && PRIVATE (cur) && PRIVATE (cur)->next;
             cur = PRIVATE (cur)->next) ;
        g_return_val_if_fail (cur, NULL);
        PRIVATE (cur)->next = a_to_append;
        PRIVATE (a_to_append)->prev = cur;
        return a_this;
}

/**
 * cr_prop_list_append2:
 *Appends a pair of prop/declaration to
 *the current prop list.
 *@a_this: the current instance of #CRPropList
 *@a_prop: the property to consider
 *@a_decl: the declaration to consider
 *
 *Returns the resulting property list, or NULL in case
 *of an error.
 */
CRPropList *
cr_prop_list_append2 (CRPropList * a_this,
                      CRString * a_prop, 
		      CRDeclaration * a_decl)
{
        CRPropList *list = NULL,
                *result = NULL;

        g_return_val_if_fail (a_prop && a_decl, NULL);

        list = cr_prop_list_allocate ();
        g_return_val_if_fail (list && PRIVATE (list), NULL);

        PRIVATE (list)->prop = a_prop;
        PRIVATE (list)->decl = a_decl;

        result = cr_prop_list_append (a_this, list);
        return result;
}

/**
 * cr_prop_list_prepend:
 *@a_this: the current instance of #CRPropList
 *@a_to_prepend: the new list to prepend.
 *
 *Prepends a list to the current list
 *Returns the new properties list.
 */
CRPropList *
cr_prop_list_prepend (CRPropList * a_this, CRPropList * a_to_prepend)
{
        CRPropList *cur = NULL;

        g_return_val_if_fail (a_to_prepend, NULL);

        if (!a_this)
                return a_to_prepend;

        for (cur = a_to_prepend; cur && PRIVATE (cur)->next;
             cur = PRIVATE (cur)->next) ;
        g_return_val_if_fail (cur, NULL);
        PRIVATE (cur)->next = a_this;
        PRIVATE (a_this)->prev = cur;
        return a_to_prepend;
}

/**
 * cr_prop_list_prepend2:
 *@a_this: the current instance of #CRPropList
 *@a_prop_name: property name to append
 *@a_decl: the property value to append.
 *
 *Prepends a propertie to a list of properties 
 *
 *Returns the new property list.
 */
CRPropList *
cr_prop_list_prepend2 (CRPropList * a_this,
                       CRString * a_prop_name, CRDeclaration * a_decl)
{
        CRPropList *list = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_prop_name && a_decl, NULL);

        list = cr_prop_list_allocate ();
        g_return_val_if_fail (list, NULL);
        PRIVATE (list)->prop = a_prop_name;
        PRIVATE (list)->decl = a_decl;
        result = cr_prop_list_prepend (a_this, list);
        return result;
}

/**
 * cr_prop_list_set_prop:
 *@a_this: the current instance of #CRPropList
 *@a_prop: the property to set
 *
 *Sets the property of a CRPropList
 */
enum CRStatus
cr_prop_list_set_prop (CRPropList * a_this, CRString * a_prop)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_prop, CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->prop = a_prop;
        return CR_OK;
}

/**
 * cr_prop_list_get_prop:
 *@a_this: the current instance of #CRPropList
 *@a_prop: out parameter. The returned property
 *
 *Getter of the property associated to the current instance
 *of #CRPropList
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_prop_list_get_prop (CRPropList const * a_this, CRString ** a_prop)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_prop, CR_BAD_PARAM_ERROR);

        *a_prop = PRIVATE (a_this)->prop;
        return CR_OK;
}

/**
 * cr_prop_list_set_decl:
 * @a_this: the current instance of #CRPropList
 * @a_decl: the new property value.
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_prop_list_set_decl (CRPropList * a_this, CRDeclaration * a_decl)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_decl, CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->decl = a_decl;
        return CR_OK;
}

/**
 * cr_prop_list_get_decl:
 * @a_this: the current instance of #CRPropList
 * @a_decl: out parameter. The property value
 *
 * Returns CR_OK upon successful completion.
 */
enum CRStatus
cr_prop_list_get_decl (CRPropList const * a_this, CRDeclaration ** a_decl)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_decl, CR_BAD_PARAM_ERROR);

        *a_decl = PRIVATE (a_this)->decl;
        return CR_OK;
}

/**
 * cr_prop_list_lookup_prop:
 *@a_this: the current instance of #CRPropList
 *@a_prop: the property to lookup
 *@a_prop_list: out parameter. The property/declaration
 *pair found (if and only if the function returned code if CR_OK)
 *
 *Lookup a given property/declaration pair
 *
 *Returns CR_OK if a prop/decl pair has been found,
 *CR_VALUE_NOT_FOUND_ERROR if not, or an error code if something
 *bad happens.
 */
enum CRStatus
cr_prop_list_lookup_prop (CRPropList * a_this,
                          CRString * a_prop, CRPropList ** a_pair)
{
        CRPropList *cur = NULL;

        g_return_val_if_fail (a_prop && a_pair, CR_BAD_PARAM_ERROR);

        if (!a_this)
                return CR_VALUE_NOT_FOUND_ERROR;

        g_return_val_if_fail (PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        for (cur = a_this; cur; cur = PRIVATE (cur)->next) {
                if (PRIVATE (cur)->prop
		    && PRIVATE (cur)->prop->stryng
                    && PRIVATE (cur)->prop->stryng->str
		    && a_prop->stryng
                    && a_prop->stryng->str
                    && !strcmp (PRIVATE (cur)->prop->stryng->str, 
				a_prop->stryng->str))
                        break;
        }

        if (cur) {
                *a_pair = cur;
                return CR_OK;
        }

        return CR_VALUE_NOT_FOUND_ERROR;
}

/**
 * cr_prop_list_get_next:
 *@a_this: the current instance of CRPropList
 *
 *Gets the next prop/decl pair in the list
 *
 *Returns the next prop/declaration pair of the list, 
 *or NULL if we reached end of list (or if an error occurs)
 */
CRPropList *
cr_prop_list_get_next (CRPropList * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), NULL);

        return PRIVATE (a_this)->next;
}

/**
 * cr_prop_list_get_prev:
 *@a_this: the current instance of CRPropList
 *
 *Gets the previous prop/decl pair in the list
 *
 *Returns the previous prop/declaration pair of the list, 
 *or NULL if we reached end of list (or if an error occurs)
 */
CRPropList *
cr_prop_list_get_prev (CRPropList * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), NULL);

        return PRIVATE (a_this)->prev;
}

/**
 * cr_prop_list_unlink:
 *@a_this: the current list of prop/decl pairs
 *@a_pair: the prop/decl pair to unlink.
 *
 *Unlinks a prop/decl pair from the list
 *
 *Returns the new list or NULL in case of an error.
 */
CRPropList *
cr_prop_list_unlink (CRPropList * a_this, CRPropList * a_pair)
{
        CRPropList *prev = NULL,
                *next = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pair, NULL);

        /*some sanity checks */
        if (PRIVATE (a_pair)->next) {
                next = PRIVATE (a_pair)->next;
                g_return_val_if_fail (PRIVATE (next), NULL);
                g_return_val_if_fail (PRIVATE (next)->prev == a_pair, NULL);
        }
        if (PRIVATE (a_pair)->prev) {
                prev = PRIVATE (a_pair)->prev;
                g_return_val_if_fail (PRIVATE (prev), NULL);
                g_return_val_if_fail (PRIVATE (prev)->next == a_pair, NULL);
        }
        if (prev) {
                PRIVATE (prev)->next = next;
        }
        if (next) {
                PRIVATE (next)->prev = prev;
        }
        PRIVATE (a_pair)->prev = PRIVATE (a_pair)->next = NULL;
        if (a_this == a_pair) {
                if (next)
                        return next;
                return NULL;
        }
        return a_this;
}

/**
 * cr_prop_list_destroy:
 * @a_this: the current instance of #CRPropList
 */
void
cr_prop_list_destroy (CRPropList * a_this)
{
        CRPropList *tail = NULL,
                *cur = NULL;

        g_return_if_fail (a_this && PRIVATE (a_this));

        for (tail = a_this;
             tail && PRIVATE (tail) && PRIVATE (tail)->next;
             tail = cr_prop_list_get_next (tail)) ;
        g_return_if_fail (tail);

        cur = tail;

        while (cur) {
                tail = PRIVATE (cur)->prev;
                if (tail && PRIVATE (tail))
                        PRIVATE (tail)->next = NULL;
                PRIVATE (cur)->prev = NULL;
                g_free (PRIVATE (cur));
                PRIVATE (cur) = NULL;
                g_free (cur);
                cur = tail;
        }
}
