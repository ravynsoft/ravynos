/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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
 * Author: Dodji Seketeli.
 */

#include <config.h>
#include <string.h>
#include "cr-declaration.h"
#include "cr-statement.h"
#include "cr-parser.h"

/**
 *@CRDeclaration:
 *
 *The definition of the #CRDeclaration class.
 */

/**
 * dump:
 *@a_this: the current instance of #CRDeclaration.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of indentation white char. 
 *
 *Dumps (serializes) one css declaration to a file.
 */
static void
dump (CRDeclaration const * a_this, FILE * a_fp, glong a_indent)
{
        guchar *str = NULL;

        g_return_if_fail (a_this);

        str = (guchar *) cr_declaration_to_string (a_this, a_indent);
        if (str) {
                fprintf (a_fp, "%s", str);
                g_free (str);
                str = NULL;
        }
}

/**
 * cr_declaration_new:
 * @a_statement: the statement this declaration belongs to. can be NULL.
 *@a_property: the property string of the declaration
 *@a_value: the value expression of the declaration.
 *Constructor of #CRDeclaration.
 *
 *Returns the newly built instance of #CRDeclaration, or NULL in
 *case of error.
 *
 *The returned CRDeclaration takes ownership of @a_property and @a_value.
 *(E.g. cr_declaration_destroy on this CRDeclaration will also free
 *@a_property and @a_value.)
 */
CRDeclaration *
cr_declaration_new (CRStatement * a_statement,
                    CRString * a_property, CRTerm * a_value)
{
        CRDeclaration *result = NULL;

        g_return_val_if_fail (a_property, NULL);

        if (a_statement)
                g_return_val_if_fail (a_statement
                                      && ((a_statement->type == RULESET_STMT)
                                          || (a_statement->type
                                              == AT_FONT_FACE_RULE_STMT)
                                          || (a_statement->type
                                              == AT_PAGE_RULE_STMT)), NULL);

        result = g_try_malloc (sizeof (CRDeclaration));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRDeclaration));
        result->property = a_property;
        result->value = a_value;

        if (a_value) {
                cr_term_ref (a_value);
        }
        result->parent_statement = a_statement;
        return result;
}

/**
 * cr_declaration_parse_from_buf:
 *@a_statement: the parent css2 statement of this
 *this declaration. Must be non NULL and of type
 *RULESET_STMT (must be a ruleset).
 *@a_str: the string that contains the statement.
 *@a_enc: the encoding of a_str.
 *
 *Parses a text buffer that contains
 *a css declaration.
 *Returns the parsed declaration, or NULL in case of error.
 */
CRDeclaration *
cr_declaration_parse_from_buf (CRStatement * a_statement,
                               const guchar * a_str, enum CREncoding a_enc)
{
        enum CRStatus status = CR_OK;
        CRTerm *value = NULL;
        CRString *property = NULL;
        CRDeclaration *result = NULL;
        CRParser *parser = NULL;
        gboolean important = FALSE;

        g_return_val_if_fail (a_str, NULL);
        if (a_statement)
                g_return_val_if_fail (a_statement->type == RULESET_STMT,
                                      NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_str, strlen ((const char *) a_str), a_enc, FALSE);
        g_return_val_if_fail (parser, NULL);

        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_parse_declaration (parser, &property,
                                              &value, &important);
        if (status != CR_OK || !property)
                goto cleanup;

        result = cr_declaration_new (a_statement, property, value);
        if (result) {
                property = NULL;
                value = NULL;
                result->important = important;
        }

      cleanup:

        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
        }

        if (property) {
                cr_string_destroy (property);
                property = NULL;
        }

        if (value) {
                cr_term_destroy (value);
                value = NULL;
        }

        return result;
}

/**
 * cr_declaration_parse_list_from_buf:
 *@a_str: the input buffer that contains the list of declaration to
 *parse.
 *@a_enc: the encoding of a_str
 *
 *Parses a ';' separated list of properties declaration.
 *Returns the parsed list of declaration, NULL if parsing failed.
 */
CRDeclaration *
cr_declaration_parse_list_from_buf (const guchar * a_str,
                                    enum CREncoding a_enc)
{

        enum CRStatus status = CR_OK;
        CRTerm *value = NULL;
        CRString *property = NULL;
        CRDeclaration *result = NULL,
                *cur_decl = NULL;
        CRParser *parser = NULL;
        CRTknzr *tokenizer = NULL;
        gboolean important = FALSE;

        g_return_val_if_fail (a_str, NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_str, strlen ((const char *) a_str), a_enc, FALSE);
        g_return_val_if_fail (parser, NULL);
        status = cr_parser_get_tknzr (parser, &tokenizer);
        if (status != CR_OK || !tokenizer) {
                if (status == CR_OK)
                        status = CR_ERROR;
                goto cleanup;
        }
        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_parse_declaration (parser, &property,
                                              &value, &important);
        if (status != CR_OK || !property) {
                if (status != CR_OK)
                        status = CR_ERROR;
                goto cleanup;
        }
        result = cr_declaration_new (NULL, property, value);
        if (result) {
                property = NULL;
                value = NULL;
                result->important = important;
        }
        /*now, go parse the other declarations */
        for (;;) {
                guint32 c = 0;

                cr_parser_try_to_skip_spaces_and_comments (parser);
                status = cr_tknzr_peek_char (tokenizer, &c);
                if (status != CR_OK) {
                        if (status == CR_END_OF_INPUT_ERROR)
                                status = CR_OK;
                        goto cleanup;
                }
                if (c == ';') {
                        status = cr_tknzr_read_char (tokenizer, &c);
                } else {
                        break;
                }
                important = FALSE;
                cr_parser_try_to_skip_spaces_and_comments (parser);
                status = cr_parser_parse_declaration (parser, &property,
                                                      &value, &important);
                if (status != CR_OK || !property) {
                        if (status == CR_END_OF_INPUT_ERROR) {
                                status = CR_OK;
                        }
                        break;
                }
                cur_decl = cr_declaration_new (NULL, property, value);
                if (cur_decl) {
                        cur_decl->important = important;
                        result = cr_declaration_append (result, cur_decl);
                        property = NULL;
                        value = NULL;
                        cur_decl = NULL;
                } else {
                        break;
                }
        }

      cleanup:

        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
        }

        if (property) {
                cr_string_destroy (property);
                property = NULL;
        }

        if (value) {
                cr_term_destroy (value);
                value = NULL;
        }

        if (status != CR_OK && result) {
                cr_declaration_destroy (result);
                result = NULL;
        }
        return result;
}

/**
 * cr_declaration_append:
 *@a_this: the current declaration list.
 *@a_new: the declaration to append.
 *
 *Appends a new declaration to the current declarations list.
 *Returns the declaration list with a_new appended to it, or NULL
 *in case of error.
 */
CRDeclaration *
cr_declaration_append (CRDeclaration * a_this, CRDeclaration * a_new)
{
        CRDeclaration *cur = NULL;

        g_return_val_if_fail (a_new, NULL);

        if (!a_this)
                return a_new;

        for (cur = a_this; cur && cur->next; cur = cur->next) ;

        cur->next = a_new;
        a_new->prev = cur;

        return a_this;
}

/**
 * cr_declaration_unlink:
 *@a_decls: the declaration to unlink.
 *
 *Unlinks the declaration from the declaration list.
 *case of a successfull completion, NULL otherwise.
 *
 *Returns a pointer to the unlinked declaration in
 */
CRDeclaration *
cr_declaration_unlink (CRDeclaration * a_decl)
{
        CRDeclaration *result = a_decl;

        g_return_val_if_fail (result, NULL);

        /*
         *some sanity checks first
         */
        if (a_decl->prev) {
                g_return_val_if_fail (a_decl->prev->next == a_decl, NULL);

        }
        if (a_decl->next) {
                g_return_val_if_fail (a_decl->next->prev == a_decl, NULL);
        }

        /*
         *now, the real unlinking job.
         */
        if (a_decl->prev) {
                a_decl->prev->next = a_decl->next;
        }
        if (a_decl->next) {
                a_decl->next->prev = a_decl->prev;
        }
        if (a_decl->parent_statement) {
                CRDeclaration **children_decl_ptr = NULL;

                switch (a_decl->parent_statement->type) {
                case RULESET_STMT:
                        if (a_decl->parent_statement->kind.ruleset) {
                                children_decl_ptr =
                                        &a_decl->parent_statement->
                                        kind.ruleset->decl_list;
                        }

                        break;

                case AT_FONT_FACE_RULE_STMT:
                        if (a_decl->parent_statement->kind.font_face_rule) {
                                children_decl_ptr =
                                        &a_decl->parent_statement->
                                        kind.font_face_rule->decl_list;
                        }
                        break;
                case AT_PAGE_RULE_STMT:
                        if (a_decl->parent_statement->kind.page_rule) {
                                children_decl_ptr =
                                        &a_decl->parent_statement->
                                        kind.page_rule->decl_list;
                        }

                default:
                        break;
                }
                if (children_decl_ptr
                    && *children_decl_ptr && *children_decl_ptr == a_decl)
                        *children_decl_ptr = (*children_decl_ptr)->next;
        }

        a_decl->next = NULL;
        a_decl->prev = NULL;
        a_decl->parent_statement = NULL;

        return result;
}

/**
 * cr_declaration_prepend:
 * @a_this: the current declaration list.
 * @a_new: the declaration to prepend.
 *
 * prepends a declaration to the current declaration list.
 *
 * Returns the list with a_new prepended or NULL in case of error.
 */
CRDeclaration *
cr_declaration_prepend (CRDeclaration * a_this, CRDeclaration * a_new)
{
        CRDeclaration *cur = NULL;

        g_return_val_if_fail (a_new, NULL);

        if (!a_this)
                return a_new;

        a_this->prev = a_new;
        a_new->next = a_this;

        for (cur = a_new; cur && cur->prev; cur = cur->prev) ;

        return cur;
}

/**
 * cr_declaration_append2:
 *@a_this: the current declaration list.
 *@a_prop: the property string of the declaration to append.
 *@a_value: the value of the declaration to append.
 *
 *Appends a declaration to the current declaration list.
 *Returns the list with the new property appended to it, or NULL in
 *case of an error.
 */
CRDeclaration *
cr_declaration_append2 (CRDeclaration * a_this,
                        CRString * a_prop, CRTerm * a_value)
{
        CRDeclaration *new_elem = NULL;

        if (a_this) {
                new_elem = cr_declaration_new (a_this->parent_statement,
                                               a_prop, a_value);
        } else {
                new_elem = cr_declaration_new (NULL, a_prop, a_value);
        }

        g_return_val_if_fail (new_elem, NULL);

        return cr_declaration_append (a_this, new_elem);
}

/**
 * cr_declaration_dump:
 *@a_this: the current instance of #CRDeclaration.
 *@a_fp: the destination file.
 *@a_indent: the number of indentation white char.
 *@a_one_per_line: whether to put one declaration per line of not .
 *
 *
 *Dumps a declaration list to a file.
 */
void
cr_declaration_dump (CRDeclaration const * a_this, FILE * a_fp, glong a_indent,
                     gboolean a_one_per_line)
{
        CRDeclaration const *cur = NULL;

        g_return_if_fail (a_this);

        for (cur = a_this; cur; cur = cur->next) {
                if (cur->prev) {
                        if (a_one_per_line == TRUE)
                                fprintf (a_fp, ";\n");
                        else
                                fprintf (a_fp, "; ");
                }
                dump (cur, a_fp, a_indent);
        }
}

/**
 * cr_declaration_dump_one:
 *@a_this: the current instance of #CRDeclaration.
 *@a_fp: the destination file.
 *@a_indent: the number of indentation white char.
 *
 *Dumps the first declaration of the declaration list to a file.
 */
void
cr_declaration_dump_one (CRDeclaration const * a_this, FILE * a_fp, glong a_indent)
{
        g_return_if_fail (a_this);

        dump (a_this, a_fp, a_indent);
}

/**
 * cr_declaration_to_string:
 *@a_this: the current instance of #CRDeclaration.
 *@a_indent: the number of indentation white char
 *to put before the actual serialisation.
 *
 *Serializes the declaration into a string
 *Returns the serialized form the declaration. The caller must
 *free the string using g_free().
 */
gchar *
cr_declaration_to_string (CRDeclaration const * a_this, gulong a_indent)
{
        GString *stringue = NULL;

        gchar *str = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this, NULL);

	stringue = g_string_new (NULL);

	if (a_this->property 
	    && a_this->property->stryng
	    && a_this->property->stryng->str) {
		str = g_strndup (a_this->property->stryng->str,
				 a_this->property->stryng->len);
		if (str) {
			cr_utils_dump_n_chars2 (' ', stringue, 
						a_indent);
			g_string_append (stringue, str);
			g_free (str);
			str = NULL;
		} else
                        goto error;

                if (a_this->value) {
                        guchar *value_str = NULL;

                        value_str = cr_term_to_string (a_this->value);
                        if (value_str) {
                                g_string_append_printf (stringue, " : %s",
                                                        value_str);
                                g_free (value_str);
                        } else
                                goto error;
                }
                if (a_this->important == TRUE) {
                        g_string_append_printf (stringue, " %s",
                                                "!important");
                }
        }
        if (stringue && stringue->str) {
                result = stringue->str;
                g_string_free (stringue, FALSE);
        }
        return result;

      error:
        if (stringue) {
                g_string_free (stringue, TRUE);
                stringue = NULL;
        }
        if (str) {
                g_free (str);
                str = NULL;
        }

        return result;
}

/**
 * cr_declaration_list_to_string:
 *@a_this: the current instance of #CRDeclaration.
 *@a_indent: the number of indentation white char
 *to put before the actual serialisation.
 *
 *Serializes the declaration list into a string
 */
guchar *
cr_declaration_list_to_string (CRDeclaration const * a_this, gulong a_indent)
{
        CRDeclaration const *cur = NULL;
        GString *stringue = NULL;
        guchar *str = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this, NULL);

        stringue = g_string_new (NULL);

        for (cur = a_this; cur; cur = cur->next) {
                str = (guchar *) cr_declaration_to_string (cur, a_indent);
                if (str) {
                        g_string_append_printf (stringue, "%s;", str);
                        g_free (str);
                } else
                        break;
        }
        if (stringue && stringue->str) {
                result = (guchar *) stringue->str;
                g_string_free (stringue, FALSE);
        }

        return result;
}

/**
 * cr_declaration_list_to_string2:
 *@a_this: the current instance of #CRDeclaration.
 *@a_indent: the number of indentation white char
 *@a_one_decl_per_line: whether to output one doc per line or not.
 *to put before the actual serialisation.
 *
 *Serializes the declaration list into a string
 *Returns the serialized form the declararation.
 */
guchar *
cr_declaration_list_to_string2 (CRDeclaration const * a_this,
                                gulong a_indent, gboolean a_one_decl_per_line)
{
        CRDeclaration const *cur = NULL;
        GString *stringue = NULL;
        guchar *str = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this, NULL);

        stringue = g_string_new (NULL);

        for (cur = a_this; cur; cur = cur->next) {
                str = (guchar *) cr_declaration_to_string (cur, a_indent);
                if (str) {
                        if (a_one_decl_per_line == TRUE) {
                                if (cur->next)
                                        g_string_append_printf (stringue,
                                                                "%s;\n", str);
                                else
                                        g_string_append (stringue,
                                                         (const gchar *) str);
                        } else {
                                if (cur->next)
                                        g_string_append_printf (stringue,
                                                                "%s;", str);
                                else
                                        g_string_append (stringue,
                                                         (const gchar *) str);
                        }
                        g_free (str);
                } else
                        break;
        }
        if (stringue && stringue->str) {
                result = (guchar *) stringue->str;
                g_string_free (stringue, FALSE);
        }

        return result;
}

/**
 * cr_declaration_nr_props:
 *@a_this: the current instance of #CRDeclaration.
 *Return the number of properties in the declaration
 */
gint
cr_declaration_nr_props (CRDeclaration const * a_this)
{
        CRDeclaration const *cur = NULL;
        int nr = 0;

        g_return_val_if_fail (a_this, -1);

        for (cur = a_this; cur; cur = cur->next)
                nr++;
        return nr;
}

/**
 * cr_declaration_get_from_list:
 *@a_this: the current instance of #CRDeclaration.
 *@itemnr: the index into the declaration list.
 *
 *Use an index to get a CRDeclaration from the declaration list.
 *
 *Returns #CRDeclaration at position itemnr, 
 *if itemnr > number of declarations - 1,
 *it will return NULL.
 */
CRDeclaration *
cr_declaration_get_from_list (CRDeclaration * a_this, int itemnr)
{
        CRDeclaration *cur = NULL;
        int nr = 0;

        g_return_val_if_fail (a_this, NULL);

        for (cur = a_this; cur; cur = cur->next)
                if (nr++ == itemnr)
                        return cur;
        return NULL;
}

/**
 * cr_declaration_get_by_prop_name:
 *@a_this: the current instance of #CRDeclaration.
 *@a_prop: the property name to search for.
 *
 *Use property name to get a CRDeclaration from the declaration list.
 *Returns #CRDeclaration with property name a_prop, or NULL if not found.
 */
CRDeclaration *
cr_declaration_get_by_prop_name (CRDeclaration * a_this,
                                 const guchar * a_prop)
{
        CRDeclaration *cur = NULL;

        g_return_val_if_fail (a_this, NULL);
        g_return_val_if_fail (a_prop, NULL);

        for (cur = a_this; cur; cur = cur->next) {
		if (cur->property 
		    && cur->property->stryng
		    && cur->property->stryng->str) {
			if (!strcmp (cur->property->stryng->str, 
				     (const char *) a_prop)) {
				return cur;
			}
		}
	}
        return NULL;
}

/**
 * cr_declaration_ref:
 *@a_this: the current instance of #CRDeclaration.
 *
 *Increases the ref count of the current instance of #CRDeclaration.
 */
void
cr_declaration_ref (CRDeclaration * a_this)
{
        g_return_if_fail (a_this);

        a_this->ref_count++;
}

/**
 * cr_declaration_unref:
 *@a_this: the current instance of #CRDeclaration.
 *
 *Decrements the ref count of the current instance of #CRDeclaration.
 *If the ref count reaches zero, the current instance of #CRDeclaration
 *if destroyed.
 *Returns TRUE if @a_this was destroyed (ref count reached zero),
 *FALSE otherwise.
 */
gboolean
cr_declaration_unref (CRDeclaration * a_this)
{
        g_return_val_if_fail (a_this, FALSE);

        if (a_this->ref_count) {
                a_this->ref_count--;
        }

        if (a_this->ref_count == 0) {
                cr_declaration_destroy (a_this);
                return TRUE;
        }
        return FALSE;
}

/**
 * cr_declaration_destroy:
 *@a_this: the current instance of #CRDeclaration.
 *
 *Destructor of the declaration list.
 */
void
cr_declaration_destroy (CRDeclaration * a_this)
{
        CRDeclaration *cur = NULL;

        g_return_if_fail (a_this);

        /*
         * Go to the last element of the list.
         */
        for (cur = a_this; cur->next; cur = cur->next)
                g_assert (cur->next->prev == cur);

        /*
         * Walk backward the list and free each "next" element.
         * Meanwhile, free each property/value pair contained in the list.
         */
        for (; cur; cur = cur->prev) {
                g_free (cur->next);
                cur->next = NULL;

                if (cur->property) {
                        cr_string_destroy (cur->property);
                        cur->property = NULL;
                }

                if (cur->value) {
                        cr_term_destroy (cur->value);
                        cur->value = NULL;
                }
        }

        g_free (a_this);
}
