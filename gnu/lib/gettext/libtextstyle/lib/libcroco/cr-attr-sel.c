/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

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
 */

#include <config.h>
#include <stdio.h>
#include "cr-attr-sel.h"

/**
 * CRAttrSel:
 *
 * #CRAdditionalSel abstracts an attribute selector.
 * Attributes selectors are described in the css2 spec [5.8].
 * There are more generally used in the css2 selectors described in
 * css2 spec [5] .
 */

/**
 * cr_attr_sel_new:
 * The constructor of #CRAttrSel.
 * Returns the newly allocated instance
 * of #CRAttrSel.
 */
CRAttrSel *
cr_attr_sel_new (void)
{
        CRAttrSel *result = NULL;

        result = g_malloc0 (sizeof (CRAttrSel));

        return result;
}

/**
 * cr_attr_sel_append_attr_sel:
 * @a_this: the this pointer of the current instance of  #CRAttrSel.
 * @a_attr_sel: selector to append.
 *
 * Appends an attribute selector to the current list of
 * attribute selectors represented by a_this.
 * Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_attr_sel_append_attr_sel (CRAttrSel * a_this, CRAttrSel * a_attr_sel)
{
        CRAttrSel *cur_sel = NULL;

        g_return_val_if_fail (a_this && a_attr_sel, 
                              CR_BAD_PARAM_ERROR);

        for (cur_sel = a_this; 
             cur_sel->next; 
             cur_sel = cur_sel->next) ;

        cur_sel->next = a_attr_sel;
        a_attr_sel->prev = cur_sel;

        return CR_OK;
}

/**
 * cr_attr_sel_prepend_attr_sel:
 *@a_this: the "this pointer" of the current instance *of #CRAttrSel.
 *@a_attr_sel: the attribute selector to append.
 *
 *Prepends an attribute selector to the list of
 *attributes selector represented by a_this.
 *Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_attr_sel_prepend_attr_sel (CRAttrSel * a_this, 
                              CRAttrSel * a_attr_sel)
{
        g_return_val_if_fail (a_this && a_attr_sel, 
                              CR_BAD_PARAM_ERROR);

        a_attr_sel->next = a_this;
        a_this->prev = a_attr_sel;

        return CR_OK;
}

/**
 * cr_attr_sel_to_string:
 * @a_this: the current instance of #CRAttrSel.
 *
 * Serializes an attribute selector into a string
 * Returns the serialized attribute selector.
 */
guchar *
cr_attr_sel_to_string (CRAttrSel const * a_this)
{
        CRAttrSel const *cur = NULL;
        guchar *result = NULL;
        GString *str_buf = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);

        for (cur = a_this; cur; cur = cur->next) {
                if (cur->prev) {
                        g_string_append_c (str_buf, ' ');
                }

                if (cur->name) {
                        guchar *name = NULL;

                        name = (guchar *) g_strndup (cur->name->stryng->str, 
                                          cur->name->stryng->len);
                        if (name) {
                                g_string_append (str_buf, (const gchar *) name);
                                g_free (name);
                                name = NULL;
                        }
                }

                if (cur->value) {
                        guchar *value = NULL;

                        value = (guchar *) g_strndup (cur->value->stryng->str, 
                                           cur->value->stryng->len);
                        if (value) {
                                switch (cur->match_way) {
                                case SET:
                                        break;

                                case EQUALS:
                                        g_string_append_c (str_buf, '=');
                                        break;

                                case INCLUDES:
                                        g_string_append (str_buf, "~=");
                                        break;

                                case DASHMATCH:
                                        g_string_append (str_buf, "|=");
                                        break;

                                default:
                                        break;
                                }

                                g_string_append_printf
                                        (str_buf, "\"%s\"", value);

                                g_free (value);
                                value = NULL;
                        }
                }
        }

        if (str_buf) {
                result = (guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
        }

        return result;
}

/**
 * cr_attr_sel_dump:
 * @a_this: the "this pointer" of the current instance of
 * #CRAttrSel.
 * @a_fp: the destination file.
 *
 * Dumps the current instance of #CRAttrSel to a file.
 */
void
cr_attr_sel_dump (CRAttrSel const * a_this, FILE * a_fp)
{
        guchar *tmp_str = NULL;

        g_return_if_fail (a_this);

        tmp_str = cr_attr_sel_to_string (a_this);

        if (tmp_str) {
                fprintf (a_fp, "%s", tmp_str);
                g_free (tmp_str);
                tmp_str = NULL;
        }
}

/**
 *cr_attr_sel_destroy:
 *@a_this: the "this pointer" of the current
 *instance of #CRAttrSel.
 *
 *Destroys the current instance of #CRAttrSel.
 *Frees all the fields if they are non null.
 */
void
cr_attr_sel_destroy (CRAttrSel * a_this)
{
        g_return_if_fail (a_this);

        if (a_this->name) {
                cr_string_destroy (a_this->name);
                a_this->name = NULL;
        }

        if (a_this->value) {
                cr_string_destroy (a_this->value);
                a_this->value = NULL;
        }

        if (a_this->next) {
                cr_attr_sel_destroy (a_this->next);
                a_this->next = NULL;
        }

        if (a_this) {
                g_free (a_this);
                a_this = NULL;
        }
}

