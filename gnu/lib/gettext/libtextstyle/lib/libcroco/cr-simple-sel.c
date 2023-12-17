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
 *
 * Author: Dodji Seketeli
 */

#include <config.h>
#include <string.h>
#include <glib.h>
#include "cr-simple-sel.h"

/**
 * cr_simple_sel_new:
 *
 *The constructor of #CRSimpleSel.
 *
 *Returns the new instance of #CRSimpleSel.
 */
CRSimpleSel *
cr_simple_sel_new (void)
{
        CRSimpleSel *result = NULL;

        result = g_try_malloc (sizeof (CRSimpleSel));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRSimpleSel));

        return result;
}

/**
 * cr_simple_sel_append_simple_sel:
 *
 *Appends a simpe selector to the current list of simple selector.
 *
 *@a_this: the this pointer of the current instance of #CRSimpleSel.
 *@a_sel: the simple selector to append.
 *
 *Returns: the new list upon successfull completion, an error code otherwise.
 */
CRSimpleSel *
cr_simple_sel_append_simple_sel (CRSimpleSel * a_this, CRSimpleSel * a_sel)
{
        CRSimpleSel *cur = NULL;

        g_return_val_if_fail (a_sel, NULL);

        if (a_this == NULL)
                return a_sel;

        for (cur = a_this; cur->next; cur = cur->next) ;

        cur->next = a_sel;
        a_sel->prev = cur;

        return a_this;
}

/**
 * cr_simple_sel_prepend_simple_sel:
 *
 *@a_this: the this pointer of the current instance of #CRSimpleSel.
 *@a_sel: the simple selector to prepend.
 *
 *Prepends a simple selector to the current list of simple selectors.
 *
 *Returns the new list upon successfull completion, an error code otherwise.
 */
CRSimpleSel *
cr_simple_sel_prepend_simple_sel (CRSimpleSel * a_this, CRSimpleSel * a_sel)
{
        g_return_val_if_fail (a_sel, NULL);

        if (a_this == NULL)
                return a_sel;

        a_sel->next = a_this;
        a_this->prev = a_sel;

        return a_sel;
}

guchar *
cr_simple_sel_to_string (CRSimpleSel const * a_this)
{
        GString *str_buf = NULL;
        guchar *result = NULL;

        CRSimpleSel const *cur = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);
        for (cur = a_this; cur; cur = cur->next) {
                if (cur->name) {
                        guchar *str = (guchar *) g_strndup (cur->name->stryng->str,
                                                 cur->name->stryng->len);

                        if (str) {
                                switch (cur->combinator) {
                                case COMB_WS:
                                        g_string_append (str_buf, " ");
                                        break;

                                case COMB_PLUS:
                                        g_string_append (str_buf, "+");
                                        break;

                                case COMB_GT:
                                        g_string_append (str_buf, ">");
                                        break;

                                default:
                                        break;
                                }

                                g_string_append (str_buf, (const gchar *) str);
                                g_free (str);
                                str = NULL;
                        }
                }

                if (cur->add_sel) {
                        guchar *tmp_str = NULL;

                        tmp_str = cr_additional_sel_to_string (cur->add_sel);
                        if (tmp_str) {
                                g_string_append (str_buf, (const gchar *) tmp_str);
                                g_free (tmp_str);
                                tmp_str = NULL;
                        }
                }
        }

        if (str_buf) {
                result = (guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
                str_buf = NULL;
        }

        return result;
}


guchar *
cr_simple_sel_one_to_string (CRSimpleSel const * a_this)
{
        GString *str_buf = NULL;
        guchar *result = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);
        if (a_this->name) {
                guchar *str = (guchar *) g_strndup (a_this->name->stryng->str,
                                         a_this->name->stryng->len);

                if (str) {
                        g_string_append_printf (str_buf, "%s", str);
                        g_free (str);
                        str = NULL;
                }
        }

        if (a_this->add_sel) {
                guchar *tmp_str = NULL;

                tmp_str = cr_additional_sel_to_string (a_this->add_sel);
                if (tmp_str) {
                        g_string_append_printf
                                (str_buf, "%s", tmp_str);
                        g_free (tmp_str);
                        tmp_str = NULL;
                }
        }

        if (str_buf) {
                result = (guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
                str_buf = NULL;
        }

        return result;
}

/**
 * cr_simple_sel_dump:
 *@a_this: the current instance of #CRSimpleSel.
 *@a_fp: the destination file pointer.
 *
 *Dumps the selector to a file.
 *TODO: add the support of unicode in the dump.
 *
 *Returns CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_simple_sel_dump (CRSimpleSel const * a_this, FILE * a_fp)
{
        guchar *tmp_str = NULL;

        g_return_val_if_fail (a_fp, CR_BAD_PARAM_ERROR);

        if (a_this) {
                tmp_str = cr_simple_sel_to_string (a_this);
                if (tmp_str) {
                        fprintf (a_fp, "%s", tmp_str);
                        g_free (tmp_str);
                        tmp_str = NULL;
                }
        }

        return CR_OK;
}

/**
 * cr_simple_sel_compute_specificity:
 *
 *@a_this: the current instance of #CRSimpleSel
 *
 *Computes the selector (combinator separated list of simple selectors)
 *as defined in the css2 spec in chapter 6.4.3
 *
 *Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_simple_sel_compute_specificity (CRSimpleSel * a_this)
{
        CRAdditionalSel const *cur_add_sel = NULL;
        CRSimpleSel const *cur_sel = NULL;
        gulong a = 0,
                b = 0,
                c = 0;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        for (cur_sel = a_this; cur_sel; cur_sel = cur_sel->next) {
                if (cur_sel->type_mask & TYPE_SELECTOR) {
                        c++;    /*hmmh, is this a new language ? */
                } else if (!cur_sel->name 
                           || !cur_sel->name->stryng
                           || !cur_sel->name->stryng->str) {
                        if (cur_sel->add_sel->type ==
                            PSEUDO_CLASS_ADD_SELECTOR) {
                                /*
                                 *this is a pseudo element, and
                                 *the spec says, "ignore pseudo elements".
                                 */
                                continue;
                        }
                }

                for (cur_add_sel = cur_sel->add_sel;
                     cur_add_sel; cur_add_sel = cur_add_sel->next) {
                        switch (cur_add_sel->type) {
                        case ID_ADD_SELECTOR:
                                a++;
                                break;

                        case NO_ADD_SELECTOR:
                                continue;

                        default:
                                b++;
                                break;
                        }
                }
        }

        /*we suppose a, b and c have 1 to 3 digits */
        a_this->specificity = a * 1000000 + b * 1000 + c;

        return CR_OK;
}

/**
 * cr_simple_sel_destroy:
 *
 *@a_this: the this pointer of the current instance of #CRSimpleSel.
 *
 *The destructor of the current instance of
 *#CRSimpleSel.
 */
void
cr_simple_sel_destroy (CRSimpleSel * a_this)
{
        g_return_if_fail (a_this);

        if (a_this->name) {
                cr_string_destroy (a_this->name);
                a_this->name = NULL;
        }

        if (a_this->add_sel) {
                cr_additional_sel_destroy (a_this->add_sel);
                a_this->add_sel = NULL;
        }

        if (a_this->next) {
                cr_simple_sel_destroy (a_this->next);
        }

        if (a_this) {
                g_free (a_this);
        }
}
