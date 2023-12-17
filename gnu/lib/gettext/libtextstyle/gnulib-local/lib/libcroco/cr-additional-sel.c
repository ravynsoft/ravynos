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
 *
 */

#include <config.h>
#include "cr-additional-sel.h"
#include "string.h"

/**
 * CRAdditionalSel:
 *
 * #CRAdditionalSel abstracts an additionnal selector.
 * An additional selector is the selector part
 * that comes after the combination of type selectors.
 * It can be either "a class selector (the .class part),
 * a pseudo class selector, an attribute selector 
 * or an id selector.
 */

/**
 * cr_additional_sel_new:
 *
 * Default constructor of #CRAdditionalSel.
 * Returns the newly build instance of #CRAdditionalSel.
 */
CRAdditionalSel *
cr_additional_sel_new (void)
{
        CRAdditionalSel *result = NULL;

        result = g_try_malloc (sizeof (CRAdditionalSel));

        if (result == NULL) {
                cr_utils_trace_debug ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRAdditionalSel));

        return result;
}

/**
 * cr_additional_sel_new_with_type:
 * @a_sel_type: the type of the newly built instance 
 * of #CRAdditionalSel.
 *
 * Constructor of #CRAdditionalSel.
 * Returns the newly built instance of #CRAdditionalSel.
 */
CRAdditionalSel *
cr_additional_sel_new_with_type (enum AddSelectorType a_sel_type)
{
        CRAdditionalSel *result = NULL;

        result = cr_additional_sel_new ();

        g_return_val_if_fail (result, NULL);

        result->type = a_sel_type;

        return result;
}

/**
 * cr_additional_sel_set_class_name:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_class_name: the new class name to set.
 *
 * Sets a new class name to a
 * CLASS additional selector.
 */
void
cr_additional_sel_set_class_name (CRAdditionalSel * a_this,
                                  CRString * a_class_name)
{
        g_return_if_fail (a_this && a_this->type == CLASS_ADD_SELECTOR);

        if (a_this->content.class_name) {
                cr_string_destroy (a_this->content.class_name);
        }

        a_this->content.class_name = a_class_name;
}

/**
 * cr_additional_sel_set_id_name:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_id: the new id to set.
 *
 * Sets a new id name to an
 * ID additional selector.
 */
void
cr_additional_sel_set_id_name (CRAdditionalSel * a_this, CRString * a_id)
{
        g_return_if_fail (a_this && a_this->type == ID_ADD_SELECTOR);

        if (a_this->content.id_name) {
                cr_string_destroy (a_this->content.id_name);
        }

        a_this->content.id_name = a_id;
}

/**
 * cr_additional_sel_set_pseudo:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_pseudo: the new pseudo to set.
 *
 * Sets a new pseudo to a
 * PSEUDO additional selector.
 */
void
cr_additional_sel_set_pseudo (CRAdditionalSel * a_this, CRPseudo * a_pseudo)
{
        g_return_if_fail (a_this
                          && a_this->type == PSEUDO_CLASS_ADD_SELECTOR);

        if (a_this->content.pseudo) {
                cr_pseudo_destroy (a_this->content.pseudo);
        }

        a_this->content.pseudo = a_pseudo;
}

/**
 * cr_additional_sel_set_attr_sel:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_sel: the new instance of #CRAttrSel to set.
 *
 * Sets a new instance of #CRAttrSel to 
 * a ATTRIBUTE additional selector.
 */
void
cr_additional_sel_set_attr_sel (CRAdditionalSel * a_this, CRAttrSel * a_sel)
{
        g_return_if_fail (a_this && a_this->type == ATTRIBUTE_ADD_SELECTOR);

        if (a_this->content.attr_sel) {
                cr_attr_sel_destroy (a_this->content.attr_sel);
        }

        a_this->content.attr_sel = a_sel;
}

/**
 * cr_additional_sel_append:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_sel: the new instance to #CRAdditional to append.
 *
 * Appends a new instance of #CRAdditional to the
 * current list of #CRAdditional.
 *
 * Returns the new list of CRAdditionalSel or NULL if an error arises.
 */
CRAdditionalSel *
cr_additional_sel_append (CRAdditionalSel * a_this, CRAdditionalSel * a_sel)
{
        CRAdditionalSel *cur_sel = NULL;

        g_return_val_if_fail (a_sel, NULL);

        if (a_this == NULL) {
                return a_sel;
        }

        if (a_sel == NULL)
                return NULL;

        for (cur_sel = a_this;
             cur_sel && cur_sel->next; cur_sel = cur_sel->next) ;

        g_return_val_if_fail (cur_sel != NULL, NULL);

        cur_sel->next = a_sel;
        a_sel->prev = cur_sel;

        return a_this;
}

/**
 * cr_additional_sel_prepend:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 * @a_sel: the new instance to #CRAdditional to preappend.
 *
 * Preppends a new instance of #CRAdditional to the
 * current list of #CRAdditional.
 *
 * Returns the new list of CRAdditionalSel or NULL if an error arises.
 */
CRAdditionalSel *
cr_additional_sel_prepend (CRAdditionalSel * a_this, CRAdditionalSel * a_sel)
{
        g_return_val_if_fail (a_sel, NULL);

        if (a_this == NULL) {
                return a_sel;
        }

        a_sel->next = a_this;
        a_this->prev = a_sel;

        return a_sel;
}

guchar *
cr_additional_sel_to_string (CRAdditionalSel const * a_this)
{
        guchar *result = NULL;
        GString *str_buf = NULL;
        CRAdditionalSel const *cur = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);

        for (cur = a_this; cur; cur = cur->next) {
                switch (cur->type) {
                case CLASS_ADD_SELECTOR:
                        {
                                guchar *name = NULL;

                                if (cur->content.class_name) {
                                        name = (guchar *) g_strndup
                                                (cur->content.class_name->stryng->str,
                                                 cur->content.class_name->stryng->len);

                                        if (name) {
                                                g_string_append_printf
                                                        (str_buf, ".%s",
                                                         name);
                                                g_free (name);
                                                name = NULL;
                                        }
                                }
                        }
                        break;

                case ID_ADD_SELECTOR:
                        {
                                guchar *name = NULL;

                                if (cur->content.id_name) {
                                        name = (guchar *) g_strndup
                                                (cur->content.id_name->stryng->str,
                                                 cur->content.id_name->stryng->len);

                                        if (name) {
                                                g_string_append_printf
                                                        (str_buf, "#%s",
                                                         name);
                                                g_free (name);
                                                name = NULL;
                                        }
                                }
                        }

                        break;

                case PSEUDO_CLASS_ADD_SELECTOR:
                        {
                                if (cur->content.pseudo) {
                                        guchar *tmp_str = NULL;

                                        tmp_str = cr_pseudo_to_string
                                                (cur->content.pseudo);
                                        if (tmp_str) {
                                                g_string_append_printf
                                                        (str_buf, ":%s",
                                                         tmp_str);
                                                g_free (tmp_str);
                                                tmp_str = NULL;
                                        }
                                }
                        }
                        break;

                case ATTRIBUTE_ADD_SELECTOR:
                        if (cur->content.attr_sel) {
                                guchar *tmp_str = NULL;

                                g_string_append_c (str_buf, '[');
                                tmp_str = cr_attr_sel_to_string
                                        (cur->content.attr_sel);
                                if (tmp_str) {
                                        g_string_append_printf
                                                (str_buf, "%s]", tmp_str);
                                        g_free (tmp_str);
                                        tmp_str = NULL;
                                }
                        }
                        break;

                default:
                        break;
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
cr_additional_sel_one_to_string (CRAdditionalSel const *a_this)
{
        guchar *result = NULL;
        GString *str_buf = NULL;

        g_return_val_if_fail (a_this, NULL) ;

        str_buf = g_string_new (NULL) ;

        switch (a_this->type) {
        case CLASS_ADD_SELECTOR:
        {
                guchar *name = NULL;

                if (a_this->content.class_name) {
                        name = (guchar *) g_strndup
                                (a_this->content.class_name->stryng->str,
                                 a_this->content.class_name->stryng->len);

                        if (name) {
                                g_string_append_printf
                                        (str_buf, ".%s",
                                         name);
                                g_free (name);
                                name = NULL;
                        }
                }
        }
        break;

        case ID_ADD_SELECTOR:
        {
                guchar *name = NULL;

                if (a_this->content.id_name) {
                        name = (guchar *) g_strndup
                                (a_this->content.id_name->stryng->str,
                                 a_this->content.id_name->stryng->len);

                        if (name) {
                                g_string_append_printf
                                        (str_buf, "#%s",
                                         name);
                                g_free (name);
                                name = NULL;
                        }
                }
        }

        break;

        case PSEUDO_CLASS_ADD_SELECTOR:
        {
                if (a_this->content.pseudo) {
                        guchar *tmp_str = NULL;

                        tmp_str = cr_pseudo_to_string
                                (a_this->content.pseudo);
                        if (tmp_str) {
                                g_string_append_printf
                                        (str_buf, ":%s",
                                         tmp_str);
                                g_free (tmp_str);
                                tmp_str = NULL;
                        }
                }
        }
        break;

        case ATTRIBUTE_ADD_SELECTOR:
                if (a_this->content.attr_sel) {
                        guchar *tmp_str = NULL;

                        g_string_append_printf (str_buf, "[");
                        tmp_str = cr_attr_sel_to_string
                                (a_this->content.attr_sel);
                        if (tmp_str) {
                                g_string_append_printf
                                        (str_buf, "%s]", tmp_str);
                                g_free (tmp_str);
                                tmp_str = NULL;
                        }
                }
                break;

        default:
                break;
        }

        if (str_buf) {
                result = (guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
                str_buf = NULL;
        }

        return result;
}

/**
 * cr_additional_sel_dump:
 * @a_this: the "this pointer" of the current instance of
 * #CRAdditionalSel.
 * @a_fp: the destination file.
 *
 * Dumps the current instance of #CRAdditionalSel to a file
 */
void
cr_additional_sel_dump (CRAdditionalSel const * a_this, FILE * a_fp)
{
        guchar *tmp_str = NULL;

        g_return_if_fail (a_fp);

        if (a_this) {
                tmp_str = cr_additional_sel_to_string (a_this);
                if (tmp_str) {
                        fprintf (a_fp, "%s", tmp_str);
                        g_free (tmp_str);
                        tmp_str = NULL;
                }
        }
}

/**
 * cr_additional_sel_destroy:
 * @a_this: the "this pointer" of the current instance
 * of #CRAdditionalSel .
 *
 * Destroys an instance of #CRAdditional.
 */
void
cr_additional_sel_destroy (CRAdditionalSel * a_this)
{
        g_return_if_fail (a_this);

        switch (a_this->type) {
        case CLASS_ADD_SELECTOR:
                cr_string_destroy (a_this->content.class_name);
                a_this->content.class_name = NULL;
                break;

        case PSEUDO_CLASS_ADD_SELECTOR:
                cr_pseudo_destroy (a_this->content.pseudo);
                a_this->content.pseudo = NULL;
                break;

        case ID_ADD_SELECTOR:
                cr_string_destroy (a_this->content.id_name);
                a_this->content.id_name = NULL;
                break;

        case ATTRIBUTE_ADD_SELECTOR:
                cr_attr_sel_destroy (a_this->content.attr_sel);
                a_this->content.attr_sel = NULL;
                break;

        default:
                break;
        }

        if (a_this->next) {
                cr_additional_sel_destroy (a_this->next);
        }

        g_free (a_this);
}
