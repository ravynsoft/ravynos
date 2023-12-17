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
#include <stdio.h>
#include <string.h>
#include "cr-term.h"
#include "cr-num.h"
#include "cr-parser.h"

/**
 *@file
 *Definition of the #CRTem class.
 */

static void
cr_term_clear (CRTerm * a_this)
{
        g_return_if_fail (a_this);

        switch (a_this->type) {
        case TERM_NUMBER:
                if (a_this->content.num) {
                        cr_num_destroy (a_this->content.num);
                        a_this->content.num = NULL;
                }
                break;

        case TERM_FUNCTION:
                if (a_this->ext_content.func_param) {
                        cr_term_destroy (a_this->ext_content.func_param);
                        a_this->ext_content.func_param = NULL;
                }
        case TERM_STRING:
        case TERM_IDENT:
        case TERM_URI:
        case TERM_HASH:
                if (a_this->content.str) {
                        cr_string_destroy (a_this->content.str);
                        a_this->content.str = NULL;
                }
                break;

        case TERM_RGB:
                if (a_this->content.rgb) {
                        cr_rgb_destroy (a_this->content.rgb);
                        a_this->content.rgb = NULL;
                }
                break;

        case TERM_UNICODERANGE:
        case TERM_NO_TYPE:
        default:
                break;
        }

        a_this->type = TERM_NO_TYPE;
}

/**
 *Instanciate a #CRTerm.
 *@return the newly build instance
 *of #CRTerm.
 */
CRTerm *
cr_term_new (void)
{
        CRTerm *result = NULL;

        result = g_try_malloc (sizeof (CRTerm));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRTerm));
        return result;
}

/**
 *Parses an expresion as defined by the css2 spec
 *and builds the expression as a list of terms.
 *@param a_buf the buffer to parse.
 *@return a pointer to the first term of the expression or
 *NULL if parsing failed.
 */
CRTerm *
cr_term_parse_expression_from_buf (const guchar * a_buf,
                                   enum CREncoding a_encoding)
{
        CRParser *parser = NULL;
        CRTerm *result = NULL;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_buf, NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        g_return_val_if_fail (parser, NULL);

        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK) {
                goto cleanup;
        }
        status = cr_parser_parse_expr (parser, &result);
        if (status != CR_OK) {
                if (result) {
                        cr_term_destroy (result);
                        result = NULL;
                }
        }

      cleanup:
        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
        }

        return result;
}

enum CRStatus
cr_term_set_number (CRTerm * a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_NUMBER;
        a_this->content.num = a_num;
        return CR_OK;
}

enum CRStatus
cr_term_set_function (CRTerm * a_this, CRString * a_func_name,
                      CRTerm * a_func_param)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_FUNCTION;
        a_this->content.str = a_func_name;
        a_this->ext_content.func_param = a_func_param;
        return CR_OK;
}

enum CRStatus
cr_term_set_string (CRTerm * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_STRING;
        a_this->content.str = a_str;
        return CR_OK;
}

enum CRStatus
cr_term_set_ident (CRTerm * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_IDENT;
        a_this->content.str = a_str;
        return CR_OK;
}

enum CRStatus
cr_term_set_uri (CRTerm * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_URI;
        a_this->content.str = a_str;
        return CR_OK;
}

enum CRStatus
cr_term_set_rgb (CRTerm * a_this, CRRgb * a_rgb)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_RGB;
        a_this->content.rgb = a_rgb;
        return CR_OK;
}

enum CRStatus
cr_term_set_hash (CRTerm * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_term_clear (a_this);

        a_this->type = TERM_HASH;
        a_this->content.str = a_str;
        return CR_OK;
}

/**
 *Appends a new term to the current list of #CRTerm.
 *
 *@param a_this the "this pointer" of the current instance
 *of #CRTerm .
 *@param a_new_term the term to append.
 *@return the list of terms with the a_new_term appended to it.
 */
CRTerm *
cr_term_append_term (CRTerm * a_this, CRTerm * a_new_term)
{
        CRTerm *cur = NULL;

        g_return_val_if_fail (a_new_term, NULL);

        if (a_this == NULL)
                return a_new_term;

        for (cur = a_this; cur->next; cur = cur->next) ;

        cur->next = a_new_term;
        a_new_term->prev = cur;

        return a_this;
}

/**
 *Prepends a term to the list of terms represented by a_this.
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRTerm .
 *@param a_new_term the term to prepend.
 *@return the head of the new list.
 */
CRTerm *
cr_term_prepend_term (CRTerm * a_this, CRTerm * a_new_term)
{
        g_return_val_if_fail (a_this && a_new_term, NULL);

        a_new_term->next = a_this;
        a_this->prev = a_new_term;

        return a_new_term;
}

/**
 *Serializes the expression represented by
 *the chained instances of #CRterm.
 *@param a_this the current instance of #CRTerm
 *@return the zero terminated string containing the serialized
 *form of #CRTerm. MUST BE FREED BY THE CALLER using g_free().
 */
guchar *
cr_term_to_string (CRTerm const * a_this)
{
        GString *str_buf = NULL;
        CRTerm const *cur = NULL;
        guchar *result = NULL,
                *content = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);
        g_return_val_if_fail (str_buf, NULL);

        for (cur = a_this; cur; cur = cur->next) {
                if ((cur->content.str == NULL)
                    && (cur->content.num == NULL)
                    && (cur->content.str == NULL)
                    && (cur->content.rgb == NULL))
                        continue;

                switch (cur->the_operator) {
                case DIVIDE:
                        g_string_append (str_buf, " / ");
                        break;

                case COMMA:
                        g_string_append (str_buf, ", ");
                        break;

                case NO_OP:
                        if (cur->prev) {
                                g_string_append (str_buf, " ");
                        }
                        break;
                default:

                        break;
                }

                switch (cur->unary_op) {
                case PLUS_UOP:
                        g_string_append (str_buf, "+");
                        break;

                case MINUS_UOP:
                        g_string_append (str_buf, "-");
                        break;

                default:
                        break;
                }

                switch (cur->type) {
                case TERM_NUMBER:
                        if (cur->content.num) {
                                content = cr_num_to_string (cur->content.num);
                        }

                        if (content) {
                                g_string_append (str_buf, (const gchar *) content);
                                g_free (content);
                                content = NULL;
                        }

                        break;

                case TERM_FUNCTION:
                        if (cur->content.str) {
                                content = (guchar *) g_strndup
                                        (cur->content.str->stryng->str,
                                         cur->content.str->stryng->len);
                        }

                        if (content) {
                                g_string_append_printf (str_buf, "%s(",
                                                        content);

                                if (cur->ext_content.func_param) {
                                        guchar *tmp_str = NULL;

                                        tmp_str = cr_term_to_string
                                                (cur->
                                                 ext_content.func_param);

                                        if (tmp_str) {
                                                g_string_append (str_buf, 
								 (const gchar *) tmp_str);
                                                g_free (tmp_str);
                                                tmp_str = NULL;
                                        }
                                }
                                g_string_append (str_buf, ")");
                                g_free (content);
                                content = NULL;
                        }

                        break;

                case TERM_STRING:
                        if (cur->content.str) {
                                content = (guchar *) g_strndup
                                        (cur->content.str->stryng->str,
                                         cur->content.str->stryng->len);
                        }

                        if (content) {
                                g_string_append_printf (str_buf,
                                                        "\"%s\"", content);
                                g_free (content);
                                content = NULL;
                        }
                        break;

                case TERM_IDENT:
                        if (cur->content.str) {
                                content = (guchar *) g_strndup
                                        (cur->content.str->stryng->str,
                                         cur->content.str->stryng->len);
                        }

                        if (content) {
                                g_string_append (str_buf, (const gchar *) content);
                                g_free (content);
                                content = NULL;
                        }
                        break;

                case TERM_URI:
                        if (cur->content.str) {
                                content = (guchar *) g_strndup
                                        (cur->content.str->stryng->str,
                                         cur->content.str->stryng->len);
                        }

                        if (content) {
                                g_string_append_printf
                                        (str_buf, "url(%s)", content);
                                g_free (content);
                                content = NULL;
                        }
                        break;

                case TERM_RGB:
                        if (cur->content.rgb) {
                                guchar *tmp_str = NULL;

                                g_string_append (str_buf, "rgb(");
                                tmp_str = cr_rgb_to_string (cur->content.rgb);

                                if (tmp_str) {
                                        g_string_append (str_buf, (const gchar *) tmp_str);
                                        g_free (tmp_str);
                                        tmp_str = NULL;
                                }
                                g_string_append (str_buf, ")");
                        }

                        break;

                case TERM_UNICODERANGE:
                        g_string_append
                                (str_buf,
                                 "?found unicoderange: dump not supported yet?");
                        break;

                case TERM_HASH:
                        if (cur->content.str) {
                                content = (guchar *) g_strndup
                                        (cur->content.str->stryng->str,
                                         cur->content.str->stryng->len);
                        }

                        if (content) {
                                g_string_append_printf (str_buf,
                                                        "#%s", content);
                                g_free (content);
                                content = NULL;
                        }
                        break;

                default:
                        g_string_append (str_buf,
                                         "Unrecognized Term type");
                        break;
                }
        }

        if (str_buf) {
                result =(guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
                str_buf = NULL;
        }

        return result;
}

guchar *
cr_term_one_to_string (CRTerm const * a_this)
{
        GString *str_buf = NULL;
        guchar *result = NULL,
                *content = NULL;

        g_return_val_if_fail (a_this, NULL);

        str_buf = g_string_new (NULL);
        g_return_val_if_fail (str_buf, NULL);

        if ((a_this->content.str == NULL)
            && (a_this->content.num == NULL)
            && (a_this->content.str == NULL)
            && (a_this->content.rgb == NULL))
                return NULL ;

        switch (a_this->the_operator) {
        case DIVIDE:
                g_string_append_printf (str_buf, " / ");
                break;

        case COMMA:
                g_string_append_printf (str_buf, ", ");
                break;

        case NO_OP:
                if (a_this->prev) {
                        g_string_append_printf (str_buf, " ");
                }
                break;
        default:

                break;
        }

        switch (a_this->unary_op) {
        case PLUS_UOP:
                g_string_append_printf (str_buf, "+");
                break;

        case MINUS_UOP:
                g_string_append_printf (str_buf, "-");
                break;

        default:
                break;
        }

        switch (a_this->type) {
        case TERM_NUMBER:
                if (a_this->content.num) {
                        content = cr_num_to_string (a_this->content.num);
                }

                if (content) {
                        g_string_append (str_buf, (const gchar *) content);
                        g_free (content);
                        content = NULL;
                }

                break;

        case TERM_FUNCTION:
                if (a_this->content.str) {
                        content = (guchar *) g_strndup
                                (a_this->content.str->stryng->str,
                                 a_this->content.str->stryng->len);
                }

                if (content) {
                        g_string_append_printf (str_buf, "%s(",
                                                content);

                        if (a_this->ext_content.func_param) {
                                guchar *tmp_str = NULL;

                                tmp_str = cr_term_to_string
                                        (a_this->
                                         ext_content.func_param);

                                if (tmp_str) {
                                        g_string_append_printf
                                                (str_buf,
                                                 "%s", tmp_str);
                                        g_free (tmp_str);
                                        tmp_str = NULL;
                                }

                                g_string_append_printf (str_buf, ")");
                                g_free (content);
                                content = NULL;
                        }
                }

                break;

        case TERM_STRING:
                if (a_this->content.str) {
                        content = (guchar *) g_strndup
                                (a_this->content.str->stryng->str,
                                 a_this->content.str->stryng->len);
                }

                if (content) {
                        g_string_append_printf (str_buf,
                                                "\"%s\"", content);
                        g_free (content);
                        content = NULL;
                }
                break;

        case TERM_IDENT:
                if (a_this->content.str) {
                        content = (guchar *) g_strndup
                                (a_this->content.str->stryng->str,
                                 a_this->content.str->stryng->len);
                }

                if (content) {
                        g_string_append (str_buf, (const gchar *) content);
                        g_free (content);
                        content = NULL;
                }
                break;

        case TERM_URI:
                if (a_this->content.str) {
                        content = (guchar *) g_strndup
                                (a_this->content.str->stryng->str,
                                 a_this->content.str->stryng->len);
                }

                if (content) {
                        g_string_append_printf
                                (str_buf, "url(%s)", content);
                        g_free (content);
                        content = NULL;
                }
                break;

        case TERM_RGB:
                if (a_this->content.rgb) {
                        guchar *tmp_str = NULL;

                        g_string_append_printf (str_buf, "rgb(");
                        tmp_str = cr_rgb_to_string (a_this->content.rgb);

                        if (tmp_str) {
                                g_string_append (str_buf, (const gchar *) tmp_str);
                                g_free (tmp_str);
                                tmp_str = NULL;
                        }
                        g_string_append_printf (str_buf, ")");
                }

                break;

        case TERM_UNICODERANGE:
                g_string_append_printf
                        (str_buf,
                         "?found unicoderange: dump not supported yet?");
                break;

        case TERM_HASH:
                if (a_this->content.str) {
                        content = (guchar *) g_strndup
                                (a_this->content.str->stryng->str,
                                 a_this->content.str->stryng->len);
                }

                if (content) {
                        g_string_append_printf (str_buf,
                                                "#%s", content);
                        g_free (content);
                        content = NULL;
                }
                break;

        default:
                g_string_append_printf (str_buf,
                                        "%s",
                                        "Unrecognized Term type");
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
 *Dumps the expression (a list of terms connected by operators)
 *to a file.
 *TODO: finish the dump. The dump of some type of terms have not yet been
 *implemented.
 *@param a_this the current instance of #CRTerm.
 *@param a_fp the destination file pointer.
 */
void
cr_term_dump (CRTerm const * a_this, FILE * a_fp)
{
        guchar *content = NULL;

        g_return_if_fail (a_this);

        content = cr_term_to_string (a_this);

        if (content) {
                fprintf (a_fp, "%s", content);
                g_free (content);
        }
}

/**
 *Return the number of terms in the expression.
 *@param a_this the current instance of #CRTerm.
 *@return number of terms in the expression.
 */
int
cr_term_nr_values (CRTerm const *a_this)
{
	CRTerm const *cur = NULL ;
	int nr = 0;

	g_return_val_if_fail (a_this, -1) ;

	for (cur = a_this ; cur ; cur = cur->next)
		nr ++;
	return nr;
}

/**
 *Use an index to get a CRTerm from the expression.
 *@param a_this the current instance of #CRTerm.
 *@param itemnr the index into the expression.
 *@return CRTerm at position itemnr, if itemnr > number of terms - 1,
 *it will return NULL.
 */
CRTerm *
cr_term_get_from_list (CRTerm *a_this, int itemnr)
{
	CRTerm *cur = NULL ;
	int nr = 0;

	g_return_val_if_fail (a_this, NULL) ;

	for (cur = a_this ; cur ; cur = cur->next)
		if (nr++ == itemnr)
			return cur;
	return NULL;
}

/**
 *Increments the reference counter of the current instance
 *of #CRTerm.*
 *@param a_this the current instance of #CRTerm.
 */
void
cr_term_ref (CRTerm * a_this)
{
        g_return_if_fail (a_this);

        a_this->ref_count++;
}

/**
 *Decrements the ref count of the current instance of
 *#CRTerm. If the ref count reaches zero, the instance is
 *destroyed.
 *@param a_this the current instance of #CRTerm.
 *@return TRUE if the current instance has been destroyed, FALSE otherwise.
 */
gboolean
cr_term_unref (CRTerm * a_this)
{
        g_return_val_if_fail (a_this, FALSE);

        if (a_this->ref_count) {
                a_this->ref_count--;
        }

        if (a_this->ref_count == 0) {
                cr_term_destroy (a_this);
                return TRUE;
        }

        return FALSE;
}

/**
 *The destructor of the the #CRTerm class.
 *@param a_this the "this pointer" of the current instance
 *of #CRTerm.
 */
void
cr_term_destroy (CRTerm * a_this)
{
        g_return_if_fail (a_this);

        cr_term_clear (a_this);

        if (a_this->next) {
                cr_term_destroy (a_this->next);
                a_this->next = NULL;
        }

        if (a_this) {
                g_free (a_this);
        }

}
