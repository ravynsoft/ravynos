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
#include "cr-statement.h"
#include "cr-parser.h"

/**
 *@file
 *Definition of the #CRStatement class.
 */

#define DECLARATION_INDENT_NB 2

static void cr_statement_clear (CRStatement * a_this);

static void  
parse_font_face_start_font_face_cb (CRDocHandler * a_this,
                                    CRParsingLocation *a_location)
{
        CRStatement *stmt = NULL;
        enum CRStatus status = CR_OK;

        stmt = cr_statement_new_at_font_face_rule (NULL, NULL);
        g_return_if_fail (stmt);

        status = cr_doc_handler_set_ctxt (a_this, stmt);
        g_return_if_fail (status == CR_OK);
}

static void
parse_font_face_unrecoverable_error_cb (CRDocHandler * a_this)
{
        CRStatement *stmt = NULL;
	CRStatement **stmtptr = NULL;
        enum CRStatus status = CR_OK;

        g_return_if_fail (a_this);

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        if (status != CR_OK) {
                cr_utils_trace_info ("Couldn't get parsing context. "
                                     "This may lead to some memory leaks.");
                return;
        }
        if (stmt) {
                cr_statement_destroy (stmt);
                cr_doc_handler_set_ctxt (a_this, NULL);
                return;
        }
}

static void
parse_font_face_property_cb (CRDocHandler * a_this,
                             CRString * a_name,
                             CRTerm * a_value, gboolean a_important)
{
        enum CRStatus status = CR_OK;
        CRString *name = NULL;
        CRDeclaration *decl = NULL;
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;

        g_return_if_fail (a_this && a_name);

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        g_return_if_fail (status == CR_OK && stmt);
        g_return_if_fail (stmt->type == AT_FONT_FACE_RULE_STMT);

        name = cr_string_dup (a_name) ;
        g_return_if_fail (name);
        decl = cr_declaration_new (stmt, name, a_value);
        if (!decl) {
                cr_utils_trace_info ("cr_declaration_new () failed.");
                goto error;
        }
        name = NULL;

        stmt->kind.font_face_rule->decl_list =
                cr_declaration_append (stmt->kind.font_face_rule->decl_list,
                                       decl);
        if (!stmt->kind.font_face_rule->decl_list)
                goto error;
        decl = NULL;

      error:
        if (decl) {
                cr_declaration_unref (decl);
                decl = NULL;
        }
        if (name) {
                cr_string_destroy (name);
                name = NULL;
        }
}

static void
parse_font_face_end_font_face_cb (CRDocHandler * a_this)
{
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;
        enum CRStatus status = CR_OK;

        g_return_if_fail (a_this);

	resultptr = &result;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) resultptr);
        g_return_if_fail (status == CR_OK && result);
        g_return_if_fail (result->type == AT_FONT_FACE_RULE_STMT);

        status = cr_doc_handler_set_result (a_this, result);
        g_return_if_fail (status == CR_OK);
}

static void
parse_page_start_page_cb (CRDocHandler * a_this,
                          CRString * a_name, 
                          CRString * a_pseudo_page,
                          CRParsingLocation *a_location)
{
        CRStatement *stmt = NULL;
        enum CRStatus status = CR_OK;
        CRString *page_name = NULL, *pseudo_name = NULL ;

        if (a_name)
                page_name = cr_string_dup (a_name) ;
        if (a_pseudo_page)
                pseudo_name = cr_string_dup (a_pseudo_page) ;

        stmt = cr_statement_new_at_page_rule (NULL, NULL, 
                                              page_name,
                                              pseudo_name);
        page_name = NULL ;
        pseudo_name = NULL ;
        g_return_if_fail (stmt);
        status = cr_doc_handler_set_ctxt (a_this, stmt);
        g_return_if_fail (status == CR_OK);
}

static void
parse_page_unrecoverable_error_cb (CRDocHandler * a_this)
{
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;
        enum CRStatus status = CR_OK;

        g_return_if_fail (a_this);

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        if (status != CR_OK) {
                cr_utils_trace_info ("Couldn't get parsing context. "
                                     "This may lead to some memory leaks.");
                return;
        }
        if (stmt) {
                cr_statement_destroy (stmt);
                stmt = NULL;
                cr_doc_handler_set_ctxt (a_this, NULL);
        }
}

static void
parse_page_property_cb (CRDocHandler * a_this,
                        CRString * a_name,
                        CRTerm * a_expression, gboolean a_important)
{
        CRString *name = NULL;
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;
        CRDeclaration *decl = NULL;
        enum CRStatus status = CR_OK;

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        g_return_if_fail (status == CR_OK && stmt->type == AT_PAGE_RULE_STMT);

        name = cr_string_dup (a_name);
        g_return_if_fail (name);

        decl = cr_declaration_new (stmt, name, a_expression);
        g_return_if_fail (decl);
        decl->important = a_important;
        stmt->kind.page_rule->decl_list =
                cr_declaration_append (stmt->kind.page_rule->decl_list, decl);
        g_return_if_fail (stmt->kind.page_rule->decl_list);
}

static void
parse_page_end_page_cb (CRDocHandler * a_this,
                        CRString * a_name, 
                        CRString * a_pseudo_page)
{
        enum CRStatus status = CR_OK;
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        g_return_if_fail (status == CR_OK && stmt);
        g_return_if_fail (stmt->type == AT_PAGE_RULE_STMT);

        status = cr_doc_handler_set_result (a_this, stmt);
        g_return_if_fail (status == CR_OK);
}

static void
parse_at_media_start_media_cb (CRDocHandler * a_this, 
                               GList * a_media_list,
                               CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        CRStatement *at_media = NULL;
        GList *media_list = NULL;

        g_return_if_fail (a_this && a_this->priv);

        if (a_media_list) {
                /*duplicate media list */
                media_list = cr_utils_dup_glist_of_cr_string 
                        (a_media_list);
        }

        g_return_if_fail (media_list);

        /*make sure cr_statement_new_at_media_rule works in this case. */
        at_media = cr_statement_new_at_media_rule (NULL, NULL, media_list);

        status = cr_doc_handler_set_ctxt (a_this, at_media);
        g_return_if_fail (status == CR_OK);
        status = cr_doc_handler_set_result (a_this, at_media);
        g_return_if_fail (status == CR_OK);
}

static void
parse_at_media_unrecoverable_error_cb (CRDocHandler * a_this)
{
        enum CRStatus status = CR_OK;
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;

        g_return_if_fail (a_this);

	stmtptr = &stmt;
        status = cr_doc_handler_get_result (a_this, (gpointer *) stmtptr);
        if (status != CR_OK) {
                cr_utils_trace_info ("Couldn't get parsing context. "
                                     "This may lead to some memory leaks.");
                return;
        }
        if (stmt) {
                cr_statement_destroy (stmt);
                stmt = NULL;
                cr_doc_handler_set_ctxt (a_this, NULL);
                cr_doc_handler_set_result (a_this, NULL);
        }
}

static void
parse_at_media_start_selector_cb (CRDocHandler * a_this,
                                  CRSelector * a_sellist)
{
        enum CRStatus status = CR_OK;
        CRStatement *at_media = NULL;
        CRStatement **at_media_ptr = NULL;
	CRStatement *ruleset = NULL;

        g_return_if_fail (a_this && a_this->priv && a_sellist);

	at_media_ptr = &at_media;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) at_media_ptr);
        g_return_if_fail (status == CR_OK && at_media);
        g_return_if_fail (at_media->type == AT_MEDIA_RULE_STMT);
        ruleset = cr_statement_new_ruleset (NULL, a_sellist, NULL, at_media);
        g_return_if_fail (ruleset);
        status = cr_doc_handler_set_ctxt (a_this, ruleset);
        g_return_if_fail (status == CR_OK);
}

static void
parse_at_media_property_cb (CRDocHandler * a_this,
                            CRString * a_name, CRTerm * a_value,
                            gboolean a_important)
{
        enum CRStatus status = CR_OK;

        /*
         *the current ruleset stmt, child of the 
         *current at-media being parsed.
         */
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;
        CRDeclaration *decl = NULL;
        CRString *name = NULL;

        g_return_if_fail (a_this && a_name);

        name = cr_string_dup (a_name) ;
        g_return_if_fail (name);

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, 
                                          (gpointer *) stmtptr);
        g_return_if_fail (status == CR_OK && stmt);
        g_return_if_fail (stmt->type == RULESET_STMT);

        decl = cr_declaration_new (stmt, name, a_value);
        g_return_if_fail (decl);
        decl->important = a_important;
        status = cr_statement_ruleset_append_decl (stmt, decl);
        g_return_if_fail (status == CR_OK);
}

static void
parse_at_media_end_selector_cb (CRDocHandler * a_this, 
                                CRSelector * a_sellist)
{
        enum CRStatus status = CR_OK;

        /*
         *the current ruleset stmt, child of the 
         *current at-media being parsed.
         */
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;

        g_return_if_fail (a_this && a_sellist);

	stmtptr = &stmt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) stmtptr);
        g_return_if_fail (status == CR_OK && stmt
                          && stmt->type == RULESET_STMT);
        g_return_if_fail (stmt->kind.ruleset->parent_media_rule);

        status = cr_doc_handler_set_ctxt
                (a_this, stmt->kind.ruleset->parent_media_rule);
        g_return_if_fail (status == CR_OK);
}

static void
parse_at_media_end_media_cb (CRDocHandler * a_this, 
                             GList * a_media_list)
{
        enum CRStatus status = CR_OK;
        CRStatement *at_media = NULL;
        CRStatement **at_media_ptr = NULL;

        g_return_if_fail (a_this && a_this->priv);

	at_media_ptr = &at_media;
        status = cr_doc_handler_get_ctxt (a_this, 
                                          (gpointer *) at_media_ptr);
        g_return_if_fail (status == CR_OK && at_media);
        status = cr_doc_handler_set_result (a_this, at_media);
}

static void
parse_ruleset_start_selector_cb (CRDocHandler * a_this,
                                 CRSelector * a_sellist)
{
        CRStatement *ruleset = NULL;

        g_return_if_fail (a_this && a_this->priv && a_sellist);

        ruleset = cr_statement_new_ruleset (NULL, a_sellist, NULL, NULL);
        g_return_if_fail (ruleset);

        cr_doc_handler_set_result (a_this, ruleset);
}

static void
parse_ruleset_unrecoverable_error_cb (CRDocHandler * a_this)
{
        CRStatement *stmt = NULL;
        CRStatement **stmtptr = NULL;
        enum CRStatus status = CR_OK;

	stmtptr = &stmt;
        status = cr_doc_handler_get_result (a_this, (gpointer *) stmtptr);
        if (status != CR_OK) {
                cr_utils_trace_info ("Couldn't get parsing context. "
                                     "This may lead to some memory leaks.");
                return;
        }
        if (stmt) {
                cr_statement_destroy (stmt);
                stmt = NULL;
                cr_doc_handler_set_result (a_this, NULL);
        }
}

static void
parse_ruleset_property_cb (CRDocHandler * a_this,
                           CRString * a_name,
                           CRTerm * a_value, gboolean a_important)
{
        enum CRStatus status = CR_OK;
        CRStatement *ruleset = NULL;
        CRStatement **rulesetptr = NULL;
        CRDeclaration *decl = NULL;
        CRString *stringue = NULL;

        g_return_if_fail (a_this && a_this->priv && a_name);

        stringue = cr_string_dup (a_name);
        g_return_if_fail (stringue);

	rulesetptr = &ruleset;
        status = cr_doc_handler_get_result (a_this, (gpointer *) rulesetptr);
        g_return_if_fail (status == CR_OK
                          && ruleset 
                          && ruleset->type == RULESET_STMT);

        decl = cr_declaration_new (ruleset, stringue, a_value);
        g_return_if_fail (decl);
        decl->important = a_important;
        status = cr_statement_ruleset_append_decl (ruleset, decl);
        g_return_if_fail (status == CR_OK);
}

static void
parse_ruleset_end_selector_cb (CRDocHandler * a_this, 
                               CRSelector * a_sellist)
{
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;
        enum CRStatus status = CR_OK;

        g_return_if_fail (a_this && a_sellist);

	resultptr = &result;
        status = cr_doc_handler_get_result (a_this, (gpointer *) resultptr);

        g_return_if_fail (status == CR_OK
                          && result 
                          && result->type == RULESET_STMT);
}

static void
cr_statement_clear (CRStatement * a_this)
{
        g_return_if_fail (a_this);

        switch (a_this->type) {
        case AT_RULE_STMT:
                break;
        case RULESET_STMT:
                if (!a_this->kind.ruleset)
                        return;
                if (a_this->kind.ruleset->sel_list) {
                        cr_selector_unref (a_this->kind.ruleset->sel_list);
                        a_this->kind.ruleset->sel_list = NULL;
                }
                if (a_this->kind.ruleset->decl_list) {
                        cr_declaration_destroy
                                (a_this->kind.ruleset->decl_list);
                        a_this->kind.ruleset->decl_list = NULL;
                }
                g_free (a_this->kind.ruleset);
                a_this->kind.ruleset = NULL;
                break;

        case AT_IMPORT_RULE_STMT:
                if (!a_this->kind.import_rule)
                        return;
                if (a_this->kind.import_rule->url) {
                        cr_string_destroy 
                                (a_this->kind.import_rule->url) ;
                        a_this->kind.import_rule->url = NULL;
                }
                g_free (a_this->kind.import_rule);
                a_this->kind.import_rule = NULL;
                break;

        case AT_MEDIA_RULE_STMT:
                if (!a_this->kind.media_rule)
                        return;
                if (a_this->kind.media_rule->rulesets) {
                        cr_statement_destroy
                                (a_this->kind.media_rule->rulesets);
                        a_this->kind.media_rule->rulesets = NULL;
                }
                if (a_this->kind.media_rule->media_list) {
                        GList *cur = NULL;

                        for (cur = a_this->kind.media_rule->media_list;
                             cur; cur = cur->next) {
                                if (cur->data) {
                                        cr_string_destroy ((CRString *) cur->data);
                                        cur->data = NULL;
                                }

                        }
                        g_list_free (a_this->kind.media_rule->media_list);
                        a_this->kind.media_rule->media_list = NULL;
                }
                g_free (a_this->kind.media_rule);
                a_this->kind.media_rule = NULL;
                break;

        case AT_PAGE_RULE_STMT:
                if (!a_this->kind.page_rule)
                        return;

                if (a_this->kind.page_rule->decl_list) {
                        cr_declaration_destroy
                                (a_this->kind.page_rule->decl_list);
                        a_this->kind.page_rule->decl_list = NULL;
                }
                if (a_this->kind.page_rule->name) {
                        cr_string_destroy 
                                (a_this->kind.page_rule->name);
                        a_this->kind.page_rule->name = NULL;
                }
                if (a_this->kind.page_rule->pseudo) {
                        cr_string_destroy
                                (a_this->kind.page_rule->pseudo);
                        a_this->kind.page_rule->pseudo = NULL;
                }
                g_free (a_this->kind.page_rule);
                a_this->kind.page_rule = NULL;
                break;

        case AT_CHARSET_RULE_STMT:
                if (!a_this->kind.charset_rule)
                        return;

                if (a_this->kind.charset_rule->charset) {
                        cr_string_destroy
                                (a_this->kind.charset_rule->charset);
                        a_this->kind.charset_rule->charset = NULL;
                }
                g_free (a_this->kind.charset_rule);
                a_this->kind.charset_rule = NULL;
                break;

        case AT_FONT_FACE_RULE_STMT:
                if (!a_this->kind.font_face_rule)
                        return;

                if (a_this->kind.font_face_rule->decl_list) {
                        cr_declaration_unref
                                (a_this->kind.font_face_rule->decl_list);
                        a_this->kind.font_face_rule->decl_list = NULL;
                }
                g_free (a_this->kind.font_face_rule);
                a_this->kind.font_face_rule = NULL;
                break;

        default:
                break;
        }
}

/**
 * cr_statement_ruleset_to_string:
 *
 *@a_this: the current instance of #CRStatement
 *@a_indent: the number of whitespace to use for indentation
 *
 *Serializes the ruleset statement into a string
 *
 *Returns the newly allocated serialised string. Must be freed
 *by the caller, using g_free().
 */
static gchar *
cr_statement_ruleset_to_string (CRStatement const * a_this, glong a_indent)
{
        GString *stringue = NULL;
        gchar *tmp_str = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT, NULL);

        stringue = g_string_new (NULL);

        if (a_this->kind.ruleset->sel_list) {
                if (a_indent)
                        cr_utils_dump_n_chars2 (' ', stringue, a_indent);

                tmp_str =
                        (gchar *) cr_selector_to_string (a_this->kind.ruleset->
                                               sel_list);
                if (tmp_str) {
                        g_string_append (stringue, tmp_str);
                        g_free (tmp_str);
                        tmp_str = NULL;
                }
        }
        g_string_append (stringue, " {\n");
        if (a_this->kind.ruleset->decl_list) {
                tmp_str = (gchar *) cr_declaration_list_to_string2
                        (a_this->kind.ruleset->decl_list,
                         a_indent + DECLARATION_INDENT_NB, TRUE);
                if (tmp_str) {
                        g_string_append (stringue, tmp_str);
                        g_free (tmp_str);
                        tmp_str = NULL;
                }
                g_string_append (stringue, "\n");
                cr_utils_dump_n_chars2 (' ', stringue, a_indent);
        }
        g_string_append (stringue, "}");
        result = stringue->str;

        if (stringue) {
                g_string_free (stringue, FALSE);
                stringue = NULL;
        }
        if (tmp_str) {
                g_free (tmp_str);
                tmp_str = NULL;
        }
        return result;
}


/**
 * cr_statement_font_face_rule_to_string:
 *
 *@a_this: the current instance of #CRStatement to consider
 *It must be a font face rule statement.
 *@a_indent: the number of white spaces of indentation.
 *
 *Serializes a font face rule statement into a string.
 *
 *Returns the serialized string. Must be deallocated by the caller
 *using g_free().
 */
static gchar *
cr_statement_font_face_rule_to_string (CRStatement const * a_this,
                                       glong a_indent)
{
        gchar *result = NULL, *tmp_str = NULL ;
        GString *stringue = NULL ;

        g_return_val_if_fail (a_this 
                              && a_this->type == AT_FONT_FACE_RULE_STMT,
                              NULL);

        if (a_this->kind.font_face_rule->decl_list) {
                stringue = g_string_new (NULL) ;
                g_return_val_if_fail (stringue, NULL) ;
                if (a_indent)
                        cr_utils_dump_n_chars2 (' ', stringue, 
                                        a_indent);
                g_string_append (stringue, "@font-face {\n");
                tmp_str = (gchar *) cr_declaration_list_to_string2 
                        (a_this->kind.font_face_rule->decl_list,
                         a_indent + DECLARATION_INDENT_NB, TRUE) ;
                if (tmp_str) {
                        g_string_append (stringue,
                                         tmp_str) ;
                        g_free (tmp_str) ;
                        tmp_str = NULL ;
                }
                g_string_append (stringue, "\n}");
        }
        if (stringue) {
                result = stringue->str ;
                g_string_free (stringue, FALSE) ;
                stringue = NULL ;
        }
        return result ;
}


/**
 * cr_statement_charset_to_string:
 *
 *Serialises an \@charset statement into a string.
 *@a_this: the statement to serialize.
 *@a_indent: the number of indentation spaces
 *
 *Returns the serialized charset statement. Must be
 *freed by the caller using g_free().
 */
static gchar *
cr_statement_charset_to_string (CRStatement const *a_this,
                                gulong a_indent)
{
        gchar *str = NULL ;
        GString *stringue = NULL ;

        g_return_val_if_fail (a_this
                              && a_this->type == AT_CHARSET_RULE_STMT,
                              NULL) ;

        if (a_this->kind.charset_rule
            && a_this->kind.charset_rule->charset
            && a_this->kind.charset_rule->charset->stryng
            && a_this->kind.charset_rule->charset->stryng->str) {
                str = g_strndup (a_this->kind.charset_rule->charset->stryng->str,
                                 a_this->kind.charset_rule->charset->stryng->len);
                g_return_val_if_fail (str, NULL);
                stringue = g_string_new (NULL) ;
                g_return_val_if_fail (stringue, NULL) ;
                cr_utils_dump_n_chars2 (' ', stringue, a_indent);
                g_string_append_printf (stringue, 
                                        "@charset \"%s\" ;", str);
                if (str) {
                        g_free (str);
                        str = NULL;
                }
        }
        if (stringue) {
                str = stringue->str ;
                g_string_free (stringue, FALSE) ;
        }
        return str ;
}


/**
 * cr_statement_at_page_rule_to_string:
 *
 *Serialises the at page rule statement into a string
 *@a_this: the current instance of #CRStatement. Must
 *be an "\@page" rule statement.
 *
 *Returns the serialized string. Must be freed by the caller
 */
static gchar *
cr_statement_at_page_rule_to_string (CRStatement const *a_this,
                                     gulong a_indent)
{
        GString *stringue = NULL;
        gchar *result = NULL ;

        stringue = g_string_new (NULL) ;

        cr_utils_dump_n_chars2 (' ', stringue, a_indent) ;
        g_string_append (stringue, "@page");
	if (a_this->kind.page_rule->name
	    && a_this->kind.page_rule->name->stryng) {
		g_string_append_printf 
		  (stringue, " %s",
		   a_this->kind.page_rule->name->stryng->str) ;
        } else {
                g_string_append (stringue, " ");
        }
	if (a_this->kind.page_rule->pseudo
	    && a_this->kind.page_rule->pseudo->stryng) {
		g_string_append_printf 
		  (stringue,  " :%s",
		   a_this->kind.page_rule->pseudo->stryng->str) ;
        }
        if (a_this->kind.page_rule->decl_list) {
                gchar *str = NULL ;
                g_string_append (stringue, " {\n");
                str = (gchar *) cr_declaration_list_to_string2
                        (a_this->kind.page_rule->decl_list,
                         a_indent + DECLARATION_INDENT_NB, TRUE) ;
                if (str) {
                        g_string_append (stringue, str) ;
                        g_free (str) ;
                        str = NULL ;
                }
                g_string_append (stringue, "\n}\n");
        }
        result = stringue->str ;
        g_string_free (stringue, FALSE) ;
        stringue = NULL ;
        return result ;
}


/**
 *Serializes an \@media statement.
 *@param a_this the current instance of #CRStatement
 *@param a_indent the number of spaces of indentation.
 *@return the serialized \@media statement. Must be freed
 *by the caller using g_free().
 */
static gchar *
cr_statement_media_rule_to_string (CRStatement const *a_this,
                                   gulong a_indent)
{
        gchar *str = NULL ;
        GString *stringue = NULL ;
        GList const *cur = NULL;

        g_return_val_if_fail (a_this->type == AT_MEDIA_RULE_STMT,
                              NULL);

        if (a_this->kind.media_rule) {
                stringue = g_string_new (NULL) ;                
                cr_utils_dump_n_chars2 (' ', stringue, a_indent);
                g_string_append (stringue, "@media");

                for (cur = a_this->kind.media_rule->media_list; cur;
                     cur = cur->next) {
                        if (cur->data) {
                                gchar *str2 = cr_string_dup2
                                        ((CRString const *) cur->data);

                                if (str2) {
                                        if (cur->prev) {
                                                g_string_append
                                                        (stringue, 
                                                         ",");
                                        }
                                        g_string_append_printf 
                                                (stringue, 
                                                 " %s", str2);
                                        g_free (str2);
                                        str2 = NULL;
                                }
                        }
                }
                g_string_append (stringue, " {\n");
                str = cr_statement_list_to_string
                        (a_this->kind.media_rule->rulesets,
                         a_indent + DECLARATION_INDENT_NB) ;
                if (str) {
                        g_string_append (stringue, str) ;
                        g_free (str) ;
                        str = NULL ;
                }
                g_string_append (stringue, "\n}");
        }
        if (stringue) {
                str = stringue->str ;
                g_string_free (stringue, FALSE) ;
        }
        return str ;
}


static gchar *
cr_statement_import_rule_to_string (CRStatement const *a_this,
                                    gulong a_indent)
{
        GString *stringue = NULL ;
        gchar *str = NULL;

        g_return_val_if_fail (a_this
                              && a_this->type == AT_IMPORT_RULE_STMT
                              && a_this->kind.import_rule,
                              NULL) ;

        if (a_this->kind.import_rule->url
            && a_this->kind.import_rule->url->stryng) { 
                stringue = g_string_new (NULL) ;
                g_return_val_if_fail (stringue, NULL) ;
                str = g_strndup (a_this->kind.import_rule->url->stryng->str,
                                 a_this->kind.import_rule->url->stryng->len);
                cr_utils_dump_n_chars2 (' ', stringue, a_indent);
                if (str) {
                        g_string_append_printf (stringue,
                                                "@import url(\"%s\")", 
                                                str);
                        g_free (str);
                        str = NULL ;
                } else          /*there is no url, so no import rule, get out! */
                        return NULL;

                if (a_this->kind.import_rule->media_list) {
                        GList const *cur = NULL;

                        for (cur = a_this->kind.import_rule->media_list;
                             cur; cur = cur->next) {
                                if (cur->data) {
                                        CRString const *crstr = cur->data;

                                        if (cur->prev) {
                                                g_string_append 
                                                        (stringue, ", ");
                                        }
                                        if (crstr 
                                            && crstr->stryng
                                            && crstr->stryng->str) {
                                                g_string_append_len 
                                                        (stringue,
                                                         crstr->stryng->str,
                                                         crstr->stryng->len) ;
                                        }
                                }
                        }
                }
                g_string_append (stringue, " ;");
        }
        if (stringue) {
                str = stringue->str ;
                g_string_free (stringue, FALSE) ;
                stringue = NULL ;
        }
        return str ;
}


/*******************
 *public functions
 ******************/

/**
 * cr_statement_does_buf_parses_against_core:
 *
 *@a_buf: the buffer to parse.
 *@a_encoding: the character encoding of a_buf.
 *
 *Tries to parse a buffer and says whether if the content of the buffer
 *is a css statement as defined by the "Core CSS Grammar" (chapter 4 of the
 *css spec) or not.
 *
 *Returns TRUE if the buffer parses against the core grammar, false otherwise.
 */
gboolean
cr_statement_does_buf_parses_against_core (const guchar * a_buf,
                                           enum CREncoding a_encoding)
{
        CRParser *parser = NULL;
        enum CRStatus status = CR_OK;
        gboolean result = FALSE;

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        g_return_val_if_fail (parser, FALSE);

        status = cr_parser_set_use_core_grammar (parser, TRUE);
        if (status != CR_OK) {
                goto cleanup;
        }

        status = cr_parser_parse_statement_core (parser);
        if (status == CR_OK) {
                result = TRUE;
        }

      cleanup:
        if (parser) {
                cr_parser_destroy (parser);
        }

        return result;
}

/**
 * cr_statement_parse_from_buf:
 *
 *@a_buf: the buffer to parse.
 *@a_encoding: the character encoding of a_buf.
 *
 *Parses a buffer that contains a css statement and returns 
 *an instance of #CRStatement in case of successful parsing.
 *TODO: at support of "\@import" rules.
 *
 *Returns the newly built instance of #CRStatement in case
 *of successful parsing, NULL otherwise.
 */
CRStatement *
cr_statement_parse_from_buf (const guchar * a_buf, enum CREncoding a_encoding)
{
        CRStatement *result = NULL;

        /*
         *The strategy of this function is "brute force".
         *It tries to parse all the types of CRStatement it knows about.
         *I could do this a smarter way but I don't have the time now.
         *I think I will revisit this when time of performances and
         *pull based incremental parsing comes.
         */

        result = cr_statement_ruleset_parse_from_buf (a_buf, a_encoding);
        if (!result) {
                result = cr_statement_at_charset_rule_parse_from_buf
                        (a_buf, a_encoding);
        } else {
                goto out;
        }

        if (!result) {
                result = cr_statement_at_media_rule_parse_from_buf
                        (a_buf, a_encoding);
        } else {
                goto out;
        }

        if (!result) {
                result = cr_statement_at_charset_rule_parse_from_buf
                        (a_buf, a_encoding);
        } else {
                goto out;
        }

        if (!result) {
                result = cr_statement_font_face_rule_parse_from_buf
                        (a_buf, a_encoding);

        } else {
                goto out;
        }

        if (!result) {
                result = cr_statement_at_page_rule_parse_from_buf
                        (a_buf, a_encoding);
        } else {
                goto out;
        }

        if (!result) {
                result = cr_statement_at_import_rule_parse_from_buf
                        (a_buf, a_encoding);
        } else {
                goto out;
        }

      out:
        return result;
}

/**
 * cr_statement_ruleset_parse_from_buf:
 *
 *@a_buf: the buffer to parse.
 *@a_enc: the character encoding of a_buf.
 *
 *Parses a buffer that contains a ruleset statement an instanciates
 *a #CRStatement of type RULESET_STMT.
 *
 *Returns the newly built instance of #CRStatement in case of successful parsing,
 *NULL otherwise.
 */
CRStatement *
cr_statement_ruleset_parse_from_buf (const guchar * a_buf,
                                     enum CREncoding a_enc)
{
        enum CRStatus status = CR_OK;
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;
        CRParser *parser = NULL;
        CRDocHandler *sac_handler = NULL;

        g_return_val_if_fail (a_buf, NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf), 
                                         a_enc, FALSE);

        g_return_val_if_fail (parser, NULL);

        sac_handler = cr_doc_handler_new ();
        g_return_val_if_fail (parser, NULL);

        sac_handler->start_selector = parse_ruleset_start_selector_cb;
        sac_handler->end_selector = parse_ruleset_end_selector_cb;
        sac_handler->property = parse_ruleset_property_cb;
        sac_handler->unrecoverable_error =
                parse_ruleset_unrecoverable_error_cb;

        cr_parser_set_sac_handler (parser, sac_handler);
        cr_parser_try_to_skip_spaces_and_comments (parser);
        status = cr_parser_parse_ruleset (parser);
        if (status != CR_OK) {
                goto cleanup;
        }

	resultptr = &result;
        status = cr_doc_handler_get_result (sac_handler,
                                            (gpointer *) resultptr);
        if (!((status == CR_OK) && result)) {
                if (result) {
                        cr_statement_destroy (result);
                        result = NULL;
                }
        }

      cleanup:
        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
                sac_handler = NULL ;
        }
        if (sac_handler) {
                cr_doc_handler_unref (sac_handler);
                sac_handler = NULL;
        }
        return result;
}

/**
 * cr_statement_new_ruleset:
 *
 *@a_sel_list: the list of #CRSimpleSel (selectors)
 *the rule applies to.
 *@a_decl_list: the list of instances of #CRDeclaration
 *that composes the ruleset.
 *@a_media_types: a list of instances of GString that
 *describe the media list this ruleset applies to.
 *
 *Creates a new instance of #CRStatement of type
 *#CRRulSet.
 *
 *Returns the new instance of #CRStatement or NULL if something
 *went wrong.
 */
CRStatement *
cr_statement_new_ruleset (CRStyleSheet * a_sheet,
                          CRSelector * a_sel_list,
                          CRDeclaration * a_decl_list,
                          CRStatement * a_parent_media_rule)
{
        CRStatement *result = NULL;

        g_return_val_if_fail (a_sel_list, NULL);

        if (a_parent_media_rule) {
                g_return_val_if_fail
                        (a_parent_media_rule->type == AT_MEDIA_RULE_STMT,
                         NULL);
                g_return_val_if_fail (a_parent_media_rule->kind.media_rule,
                                      NULL);
        }

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStatement));
        result->type = RULESET_STMT;
        result->kind.ruleset = g_try_malloc (sizeof (CRRuleSet));

        if (!result->kind.ruleset) {
                cr_utils_trace_info ("Out of memory");
                if (result)
                        g_free (result);
                return NULL;
        }

        memset (result->kind.ruleset, 0, sizeof (CRRuleSet));
        result->kind.ruleset->sel_list = a_sel_list;
        if (a_sel_list)
                cr_selector_ref (a_sel_list);
        result->kind.ruleset->decl_list = a_decl_list;

        if (a_parent_media_rule) {
                result->kind.ruleset->parent_media_rule = a_parent_media_rule;
                a_parent_media_rule->kind.media_rule->rulesets =
                        cr_statement_append
                        (a_parent_media_rule->kind.media_rule->rulesets,
                         result);
        }

        cr_statement_set_parent_sheet (result, a_sheet);

        return result;
}

/**
 * cr_statement_at_media_rule_parse_from_buf:
 *
 *@a_buf: the input to parse.
 *@a_enc: the encoding of the buffer.
 *
 *Parses a buffer that contains an "\@media" declaration
 *and builds an \@media css statement.
 *
 *Returns the \@media statement, or NULL if the buffer could not
 *be successfully parsed.
 */
CRStatement *
cr_statement_at_media_rule_parse_from_buf (const guchar * a_buf,
                                           enum CREncoding a_enc)
{
        CRParser *parser = NULL;
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;
        CRDocHandler *sac_handler = NULL;
        enum CRStatus status = CR_OK;

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf), 
                                         a_enc, FALSE);
        if (!parser) {
                cr_utils_trace_info ("Instantiation of the parser failed");
                goto cleanup;
        }

        sac_handler = cr_doc_handler_new ();
        if (!sac_handler) {
                cr_utils_trace_info
                        ("Instantiation of the sac handler failed");
                goto cleanup;
        }

        sac_handler->start_media = parse_at_media_start_media_cb;
        sac_handler->start_selector = parse_at_media_start_selector_cb;
        sac_handler->property = parse_at_media_property_cb;
        sac_handler->end_selector = parse_at_media_end_selector_cb;
        sac_handler->end_media = parse_at_media_end_media_cb;
        sac_handler->unrecoverable_error =
                parse_at_media_unrecoverable_error_cb;

        status = cr_parser_set_sac_handler (parser, sac_handler);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_parse_media (parser);
        if (status != CR_OK)
                goto cleanup;

	resultptr = &result;
        status = cr_doc_handler_get_result (sac_handler,
                                            (gpointer *) resultptr);
        if (status != CR_OK)
                goto cleanup;

      cleanup:

        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
                sac_handler = NULL ;
        }
        if (sac_handler) {
                cr_doc_handler_unref (sac_handler);
                sac_handler = NULL;
        }

        return result;
}

/**
 * cr_statement_new_at_media_rule:
 *
 *@a_ruleset: the ruleset statements contained
 *in the \@media rule.
 *@a_media: the media string list. A list of GString pointers.
 *
 *Instanciates an instance of #CRStatement of type
 *AT_MEDIA_RULE_STMT (\@media ruleset).
 *
 */
CRStatement *
cr_statement_new_at_media_rule (CRStyleSheet * a_sheet,
                                CRStatement * a_rulesets, GList * a_media)
{
        CRStatement *result = NULL,
                *cur = NULL;

        if (a_rulesets)
                g_return_val_if_fail (a_rulesets->type == RULESET_STMT, NULL);

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStatement));
        result->type = AT_MEDIA_RULE_STMT;

        result->kind.media_rule = g_try_malloc (sizeof (CRAtMediaRule));
        if (!result->kind.media_rule) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (result->kind.media_rule, 0, sizeof (CRAtMediaRule));
        result->kind.media_rule->rulesets = a_rulesets;
        for (cur = a_rulesets; cur; cur = cur->next) {
                if (cur->type != RULESET_STMT || !cur->kind.ruleset) {
                        cr_utils_trace_info ("Bad parameter a_rulesets. "
                                             "It should be a list of "
                                             "correct ruleset statement only !");
                        goto error;
                }
                cur->kind.ruleset->parent_media_rule = result;
        }

        result->kind.media_rule->media_list = a_media;
        if (a_sheet) {
                cr_statement_set_parent_sheet (result, a_sheet);
        }

        return result;

      error:
        return NULL;
}

/**
 * cr_statement_new_at_import_rule:
 *
 *@a_url: the url to connect to the get the file
 *to be imported.
 *@a_sheet: the imported parsed stylesheet.
 *
 *Creates a new instance of #CRStatment of type
 *#CRAtImportRule.
 *
 *Returns the newly built instance of #CRStatement.
 */
CRStatement *
cr_statement_new_at_import_rule (CRStyleSheet * a_container_sheet,
                                 CRString * a_url,
                                 GList * a_media_list,
                                 CRStyleSheet * a_imported_sheet)
{
        CRStatement *result = NULL;

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStatement));
        result->type = AT_IMPORT_RULE_STMT;

        result->kind.import_rule = g_try_malloc (sizeof (CRAtImportRule));

        if (!result->kind.import_rule) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }

        memset (result->kind.import_rule, 0, sizeof (CRAtImportRule));
        result->kind.import_rule->url = a_url;
        result->kind.import_rule->media_list = a_media_list;
        result->kind.import_rule->sheet = a_imported_sheet;
        if (a_container_sheet)
                cr_statement_set_parent_sheet (result, a_container_sheet);

        return result;
}

/**
 * cr_statement_at_import_rule_parse_from_buf:
 *
 *@a_buf: the buffer to parse.
 *@a_encoding: the encoding of a_buf.
 *
 *Parses a buffer that contains an "\@import" rule and
 *instanciate a #CRStatement of type AT_IMPORT_RULE_STMT
 *
 *Returns the newly built instance of #CRStatement in case of 
 *a successful parsing, NULL otherwise.
 */
CRStatement *
cr_statement_at_import_rule_parse_from_buf (const guchar * a_buf,
                                            enum CREncoding a_encoding)
{
        enum CRStatus status = CR_OK;
        CRParser *parser = NULL;
        CRStatement *result = NULL;
        GList *media_list = NULL;
        CRString *import_string = NULL;
        CRParsingLocation location = {0} ;

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        if (!parser) {
                cr_utils_trace_info ("Instantiation of parser failed.");
                goto cleanup;
        }

        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_parse_import (parser,
                                         &media_list, 
                                         &import_string,
                                         &location);
        if (status != CR_OK || !import_string)
                goto cleanup;

        result = cr_statement_new_at_import_rule (NULL, import_string,
                                                  media_list, NULL);
        if (result) {
                cr_parsing_location_copy (&result->location,
                                          &location) ;
                import_string = NULL;
                media_list = NULL;
        }

 cleanup:
        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
        }
        if (media_list) {
                for (; media_list;
                     media_list = g_list_next (media_list)) {
                        if (media_list->data) {
                                cr_string_destroy ((CRString*)media_list->data);
                                media_list->data = NULL;
                        }
                }
                g_list_free (media_list);
                media_list = NULL;
        }
        if (import_string) {
                cr_string_destroy (import_string);
                import_string = NULL;
        }

        return result;
}

/**
 * cr_statement_new_at_page_rule:
 *
 *@a_decl_list: a list of instances of #CRDeclarations
 *which is actually the list of declarations that applies to
 *this page rule.
 *@a_selector: the page rule selector.
 *
 *Creates a new instance of #CRStatement of type
 *#CRAtPageRule.
 *
 *Returns the newly built instance of #CRStatement or NULL
 *in case of error.
 */
CRStatement *
cr_statement_new_at_page_rule (CRStyleSheet * a_sheet,
                               CRDeclaration * a_decl_list,
                               CRString * a_name, CRString * a_pseudo)
{
        CRStatement *result = NULL;

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStatement));
        result->type = AT_PAGE_RULE_STMT;

        result->kind.page_rule = g_try_malloc (sizeof (CRAtPageRule));

        if (!result->kind.page_rule) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }

        memset (result->kind.page_rule, 0, sizeof (CRAtPageRule));
        if (a_decl_list) {
                result->kind.page_rule->decl_list = a_decl_list;
                cr_declaration_ref (a_decl_list);
        }
        result->kind.page_rule->name = a_name;
        result->kind.page_rule->pseudo = a_pseudo;
        if (a_sheet)
                cr_statement_set_parent_sheet (result, a_sheet);

        return result;
}

/**
 * cr_statement_at_page_rule_parse_from_buf:
 *
 *@a_buf: the character buffer to parse.
 *@a_encoding: the character encoding of a_buf.
 *
 *Parses a buffer that contains an "\@page" production and,
 *if the parsing succeeds, builds the page statement.
 *
 *Returns the newly built at page statement in case of successful parsing,
 *NULL otherwise.
 */
CRStatement *
cr_statement_at_page_rule_parse_from_buf (const guchar * a_buf,
                                          enum CREncoding a_encoding)
{
        enum CRStatus status = CR_OK;
        CRParser *parser = NULL;
        CRDocHandler *sac_handler = NULL;
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;

        g_return_val_if_fail (a_buf, NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        if (!parser) {
                cr_utils_trace_info ("Instantiation of the parser failed.");
                goto cleanup;
        }

        sac_handler = cr_doc_handler_new ();
        if (!sac_handler) {
                cr_utils_trace_info
                        ("Instantiation of the sac handler failed.");
                goto cleanup;
        }

        sac_handler->start_page = parse_page_start_page_cb;
        sac_handler->property = parse_page_property_cb;
        sac_handler->end_page = parse_page_end_page_cb;
        sac_handler->unrecoverable_error = parse_page_unrecoverable_error_cb;

        status = cr_parser_set_sac_handler (parser, sac_handler);
        if (status != CR_OK)
                goto cleanup;

        /*Now, invoke the parser to parse the "@page production" */
        cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;
        status = cr_parser_parse_page (parser);
        if (status != CR_OK)
                goto cleanup;

	resultptr = &result;
        status = cr_doc_handler_get_result (sac_handler,
                                            (gpointer *) resultptr);

      cleanup:

        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
                sac_handler = NULL ;
        }
        if (sac_handler) {
                cr_doc_handler_unref (sac_handler);
                sac_handler = NULL;
        }
        return result;
}

/**
 * cr_statement_new_at_charset_rule:
 *
 *@a_charset: the string representing the charset.
 *Note that the newly built instance of #CRStatement becomes
 *the owner of a_charset. The caller must not free a_charset !!!.
 *
 *Creates a new instance of #CRStatement of type
 *#CRAtCharsetRule.
 *
 *Returns the newly built instance of #CRStatement or NULL
 *if an error arises.
 */
CRStatement *
cr_statement_new_at_charset_rule (CRStyleSheet * a_sheet, 
                                  CRString * a_charset)
{
        CRStatement *result = NULL;

        g_return_val_if_fail (a_charset, NULL);

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStatement));
        result->type = AT_CHARSET_RULE_STMT;

        result->kind.charset_rule = g_try_malloc (sizeof (CRAtCharsetRule));

        if (!result->kind.charset_rule) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (result->kind.charset_rule, 0, sizeof (CRAtCharsetRule));
        result->kind.charset_rule->charset = a_charset;
        cr_statement_set_parent_sheet (result, a_sheet);

        return result;
}

/**
 * cr_statement_at_charset_rule_parse_from_buf:
 *
 *@a_buf: the buffer to parse.
 *@a_encoding: the character encoding of the buffer.
 *
 *Parses a buffer that contains an '\@charset' rule and
 *creates an instance of #CRStatement of type AT_CHARSET_RULE_STMT.
 *
 *Returns the newly built instance of #CRStatement.
 */
CRStatement *
cr_statement_at_charset_rule_parse_from_buf (const guchar * a_buf,
                                             enum CREncoding a_encoding)
{
        enum CRStatus status = CR_OK;
        CRParser *parser = NULL;
        CRStatement *result = NULL;
        CRString *charset = NULL;

        g_return_val_if_fail (a_buf, NULL);

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        if (!parser) {
                cr_utils_trace_info ("Instantiation of the parser failed.");
                goto cleanup;
        }

        /*Now, invoke the parser to parse the "@charset production" */
        cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;
        status = cr_parser_parse_charset (parser, &charset, NULL);
        if (status != CR_OK || !charset)
                goto cleanup;

        result = cr_statement_new_at_charset_rule (NULL, charset);
        if (result)
                charset = NULL;

      cleanup:

        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
        }
        if (charset) {
                cr_string_destroy (charset);
        }

        return result;
}

/**
 * cr_statement_new_at_font_face_rule:
 *
 *@a_font_decls: a list of instances of #CRDeclaration. Each declaration
 *is actually a font declaration.
 *
 *Creates an instance of #CRStatement of type #CRAtFontFaceRule.
 *
 *Returns the newly built instance of #CRStatement.
 */
CRStatement *
cr_statement_new_at_font_face_rule (CRStyleSheet * a_sheet,
                                    CRDeclaration * a_font_decls)
{
        CRStatement *result = NULL;

        result = g_try_malloc (sizeof (CRStatement));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRStatement));
        result->type = AT_FONT_FACE_RULE_STMT;

        result->kind.font_face_rule = g_try_malloc
                (sizeof (CRAtFontFaceRule));

        if (!result->kind.font_face_rule) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (result->kind.font_face_rule, 0, sizeof (CRAtFontFaceRule));

        result->kind.font_face_rule->decl_list = a_font_decls;
        if (a_sheet)
                cr_statement_set_parent_sheet (result, a_sheet);

        return result;
}

/**
 * cr_statement_font_face_rule_parse_from_buf:
 *
 *
 *@a_buf: the buffer to parse.
 *@a_encoding: the character encoding of a_buf.
 *
 *Parses a buffer that contains an "\@font-face" rule and builds
 *an instance of #CRStatement of type AT_FONT_FACE_RULE_STMT out of it.
 *
 *Returns the newly built instance of #CRStatement in case of successufull
 *parsing, NULL otherwise.
 */
CRStatement *
cr_statement_font_face_rule_parse_from_buf (const guchar * a_buf,
                                            enum CREncoding a_encoding)
{
        CRStatement *result = NULL;
        CRStatement **resultptr = NULL;
        CRParser *parser = NULL;
        CRDocHandler *sac_handler = NULL;
        enum CRStatus status = CR_OK;

        parser = cr_parser_new_from_buf ((guchar*)a_buf, strlen ((const char *) a_buf),
                                         a_encoding, FALSE);
        if (!parser)
                goto cleanup;

        sac_handler = cr_doc_handler_new ();
        if (!sac_handler)
                goto cleanup;

        /*
         *set sac callbacks here
         */
        sac_handler->start_font_face = parse_font_face_start_font_face_cb;
        sac_handler->property = parse_font_face_property_cb;
        sac_handler->end_font_face = parse_font_face_end_font_face_cb;
        sac_handler->unrecoverable_error =
                parse_font_face_unrecoverable_error_cb;

        status = cr_parser_set_sac_handler (parser, sac_handler);
        if (status != CR_OK)
                goto cleanup;

        /*
         *cleanup spaces of comment that may be there before the real
         *"@font-face" thing.
         */
        status = cr_parser_try_to_skip_spaces_and_comments (parser);
        if (status != CR_OK)
                goto cleanup;

        status = cr_parser_parse_font_face (parser);
        if (status != CR_OK)
                goto cleanup;

	resultptr = &result;
        status = cr_doc_handler_get_result (sac_handler,
                                            (gpointer *) resultptr);
        if (status != CR_OK || !result)
                goto cleanup;

      cleanup:
        if (parser) {
                cr_parser_destroy (parser);
                parser = NULL;
                sac_handler = NULL ;
        }
        if (sac_handler) {
                cr_doc_handler_unref (sac_handler);
                sac_handler = NULL;
        }
        return result;
}

/**
 * cr_statement_set_parent_sheet:
 *
 *@a_this: the current instance of #CRStatement.
 *@a_sheet: the sheet that contains the current statement.
 *
 *Sets the container stylesheet.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_set_parent_sheet (CRStatement * a_this, CRStyleSheet * a_sheet)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        a_this->parent_sheet = a_sheet;
        return CR_OK;
}

/**
 * cr_statement_get_parent_sheet:
 *
 *@a_this: the current #CRStatement.
 *@a_sheet: out parameter. A pointer to the sheets that
 *
 *Gets the sheets that contains the current statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_get_parent_sheet (CRStatement * a_this, CRStyleSheet ** a_sheet)
{
        g_return_val_if_fail (a_this && a_sheet, CR_BAD_PARAM_ERROR);
        *a_sheet = a_this->parent_sheet;
        return CR_OK;
}

/**
 * cr_statement_append:
 *
 *@a_this: the current instance of the statement list.
 *@a_new: a_new the new instance of #CRStatement to append.
 *
 *Appends a new statement to the statement list.
 *
 *Returns the new list statement list, or NULL in cas of failure.
 */
CRStatement *
cr_statement_append (CRStatement * a_this, CRStatement * a_new)
{
        CRStatement *cur = NULL;

        g_return_val_if_fail (a_new, NULL);

        if (!a_this) {
                return a_new;
        }

        /*walk forward in the current list to find the tail list element */
        for (cur = a_this; cur && cur->next; cur = cur->next) ;

        cur->next = a_new;
        a_new->prev = cur;

        return a_this;
}

/**
 * cr_statement_prepend:
 *
 *@a_this: the current instance of #CRStatement.
 *@a_new: the new statement to prepend.
 *
 *Prepends the an instance of #CRStatement to
 *the current statement list.
 *
 *Returns the new list with the new statement prepended,
 *or NULL in case of an error.
 */
CRStatement *
cr_statement_prepend (CRStatement * a_this, CRStatement * a_new)
{
        CRStatement *cur = NULL;

        g_return_val_if_fail (a_new, NULL);

        if (!a_this)
                return a_new;

        a_new->next = a_this;
        a_this->prev = a_new;

        /*walk backward in the prepended list to find the head list element */
        for (cur = a_new; cur && cur->prev; cur = cur->prev) ;

        return cur;
}

/**
 * cr_statement_unlink:
 *
 *@a_this: the current statements list.
 *@a_to_unlink: the statement to unlink from the list.
 *
 *Unlinks a statement from the statements list.
 *
 *Returns the new list where a_to_unlink has been unlinked
 *from, or NULL in case of error.
 */
CRStatement *
cr_statement_unlink (CRStatement * a_stmt)
{
        CRStatement *result = a_stmt;

        g_return_val_if_fail (result, NULL);

        /**
         *Some sanity checks first
         */
        if (a_stmt->next) {
                g_return_val_if_fail (a_stmt->next->prev == a_stmt, NULL);
        }
        if (a_stmt->prev) {
                g_return_val_if_fail (a_stmt->prev->next == a_stmt, NULL);
        }

        /**
         *Now, the real unlinking job.
         */
        if (a_stmt->next) {
                a_stmt->next->prev = a_stmt->prev;
        }
        if (a_stmt->prev) {
                a_stmt->prev->next = a_stmt->next;
        }

        if (a_stmt->parent_sheet
            && a_stmt->parent_sheet->statements == a_stmt) {
                a_stmt->parent_sheet->statements =
                        a_stmt->parent_sheet->statements->next;
        }

        a_stmt->next = NULL;
        a_stmt->prev = NULL;
        a_stmt->parent_sheet = NULL;

        return result;
}

/**
 * cr_statement_nr_rules:
 *
 *@a_this: the current instance of #CRStatement.
 *
 *Gets the number of rules in the statement list;
 *
 *Returns number of rules in the statement list.
 */
gint
cr_statement_nr_rules (CRStatement const * a_this)
{
        CRStatement const *cur = NULL;
        int nr = 0;

        g_return_val_if_fail (a_this, -1);

        for (cur = a_this; cur; cur = cur->next)
                nr++;
        return nr;
}

/**
 * cr_statement_get_from_list:
 *
 *@a_this: the current instance of #CRStatement.
 *@itemnr: the index into the statement list.
 *
 *Use an index to get a CRStatement from the statement list.
 *
 *Returns CRStatement at position itemnr, if itemnr > number of statements - 1,
 *it will return NULL.
 */
CRStatement *
cr_statement_get_from_list (CRStatement * a_this, int itemnr)
{
        CRStatement *cur = NULL;
        int nr = 0;

        g_return_val_if_fail (a_this, NULL);

        for (cur = a_this; cur; cur = cur->next)
                if (nr++ == itemnr)
                        return cur;
        return NULL;
}

/**
 * cr_statement_ruleset_set_sel_list:
 *
 *@a_this: the current ruleset statement.
 *@a_sel_list: the selector list to set. Note
 *that this function increments the ref count of a_sel_list.
 *The sel list will be destroyed at the destruction of the
 *current instance of #CRStatement.
 *
 *Sets a selector list to a ruleset statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_set_sel_list (CRStatement * a_this,
                                   CRSelector * a_sel_list)
{
        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT,
                              CR_BAD_PARAM_ERROR);

        if (a_this->kind.ruleset->sel_list)
                cr_selector_unref (a_this->kind.ruleset->sel_list);

        a_this->kind.ruleset->sel_list = a_sel_list;

        if (a_sel_list)
                cr_selector_ref (a_sel_list);

        return CR_OK;
}

/**
 * cr_statement_ruleset_get_declarations:
 *
 *@a_this: the current instance of #CRStatement.
 *@a_decl_list: out parameter. A pointer to the the returned
 *list of declaration. Must not be NULL.
 *
 *Gets a pointer to the list of declaration contained
 *in the ruleset statement.
 *
 *Returns CR_OK upon successful completion, an error code if something
 *bad happened.
 */
enum CRStatus
cr_statement_ruleset_get_declarations (CRStatement * a_this,
                                       CRDeclaration ** a_decl_list)
{
        g_return_val_if_fail (a_this
                              && a_this->type == RULESET_STMT
                              && a_this->kind.ruleset
                              && a_decl_list, CR_BAD_PARAM_ERROR);

        *a_decl_list = a_this->kind.ruleset->decl_list;

        return CR_OK;
}

/**
 * cr_statement_ruleset_get_sel_list:
 *
 *@a_this: the current ruleset statement.
 *@a_list: out parameter. The returned selector list,
 *if and only if the function returned CR_OK.
 *
 *Gets a pointer to the selector list contained in
 *the current ruleset statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_get_sel_list (CRStatement const * a_this, CRSelector ** a_list)
{
        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
                              && a_this->kind.ruleset, CR_BAD_PARAM_ERROR);

        *a_list = a_this->kind.ruleset->sel_list;

        return CR_OK;
}

/**
 * cr_statement_ruleset_set_decl_list:
 *
 *@a_this: the current ruleset statement.
 *@a_list: the declaration list to be added to the current
 *ruleset statement.
 *
 *Sets a declaration list to the current ruleset statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_set_decl_list (CRStatement * a_this,
                                    CRDeclaration * a_list)
{
        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
                              && a_this->kind.ruleset, CR_BAD_PARAM_ERROR);

        if (a_this->kind.ruleset->decl_list == a_list)
                return CR_OK;

        if (a_this->kind.ruleset->sel_list) {
                cr_declaration_destroy (a_this->kind.ruleset->decl_list);
        }

        a_this->kind.ruleset->sel_list = NULL;

        return CR_OK;
}

/**
 * cr_statement_ruleset_append_decl2:
 *
 *@a_this: the current statement.
 *@a_prop: the property of the declaration.
 *@a_value: the value of the declaration.
 *
 *Appends a declaration to the current ruleset statement.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_statement_ruleset_append_decl2 (CRStatement * a_this,
                                   CRString * a_prop, 
                                   CRTerm * a_value)
{
        CRDeclaration *new_decls = NULL;

        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
                              && a_this->kind.ruleset, CR_BAD_PARAM_ERROR);

        new_decls = cr_declaration_append2
                (a_this->kind.ruleset->decl_list, 
                 a_prop, a_value);
        g_return_val_if_fail (new_decls, CR_ERROR);
        a_this->kind.ruleset->decl_list = new_decls;

        return CR_OK;
}

/**
 * cr_statement_ruleset_append_decl:
 *
 *Appends a declaration to the current statement.
 *
 *@a_this: the current statement.
 *@a_declaration: the declaration to append.
 *
 *Returns CR_OK upon sucessful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_statement_ruleset_append_decl (CRStatement * a_this,
                                  CRDeclaration * a_decl)
{
        CRDeclaration *new_decls = NULL;

        g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
                              && a_this->kind.ruleset, CR_BAD_PARAM_ERROR);

        new_decls = cr_declaration_append
                (a_this->kind.ruleset->decl_list, a_decl);
        g_return_val_if_fail (new_decls, CR_ERROR);
        a_this->kind.ruleset->decl_list = new_decls;

        return CR_OK;
}

/**
 * cr_statement_at_import_rule_set_imported_sheet:
 *
 *Sets a stylesheet to the current \@import rule.
 *@a_this: the current \@import rule.
 *@a_sheet: the stylesheet. The stylesheet is owned
 *by the current instance of #CRStatement, that is, the 
 *stylesheet will be destroyed when the current instance
 *of #CRStatement is destroyed.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_set_imported_sheet (CRStatement * a_this,
                                                CRStyleSheet * a_sheet)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_IMPORT_RULE_STMT
                              && a_this->kind.import_rule,
                              CR_BAD_PARAM_ERROR);

        a_this->kind.import_rule->sheet = a_sheet;

        return CR_OK;
}

/**
 * cr_statement_at_import_rule_get_imported_sheet:
 *
 *@a_this: the current \@import rule statement.
 *@a_sheet: out parameter. The returned stylesheet if and
 *only if the function returns CR_OK.
 *
 *Gets the stylesheet contained by the \@import rule statement.
 *Returns CR_OK upon sucessful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_get_imported_sheet (CRStatement * a_this,
                                                CRStyleSheet ** a_sheet)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_IMPORT_RULE_STMT
                              && a_this->kind.import_rule,
                              CR_BAD_PARAM_ERROR);
        *a_sheet = a_this->kind.import_rule->sheet;
        return CR_OK;

}

/**
 * cr_statement_at_import_rule_set_url:
 *
 *@a_this: the current \@import rule statement.
 *@a_url: the url to set.
 *
 *Sets an url to the current \@import rule statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_set_url (CRStatement * a_this, 
                                     CRString * a_url)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_IMPORT_RULE_STMT
                              && a_this->kind.import_rule,
                              CR_BAD_PARAM_ERROR);

        if (a_this->kind.import_rule->url) {
                cr_string_destroy (a_this->kind.import_rule->url);
        }

        a_this->kind.import_rule->url = a_url;

        return CR_OK;
}

/**
 * cr_statement_at_import_rule_get_url:
 *
 *@a_this: the current \@import rule statement.
 *@a_url: out parameter. The returned url if
 *and only if the function returned CR_OK.
 *
 *Gets the url of the \@import rule statement.
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_get_url (CRStatement const * a_this,
                                     CRString ** a_url)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_IMPORT_RULE_STMT
                              && a_this->kind.import_rule,
                              CR_BAD_PARAM_ERROR);

        *a_url = a_this->kind.import_rule->url;

        return CR_OK;
}

/**
 * cr_statement_at_media_nr_rules:
 *
 *@a_this: the current instance of #CRStatement.
 *
 *Returns the number of rules in the media rule;
 */
int
cr_statement_at_media_nr_rules (CRStatement const * a_this)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_MEDIA_RULE_STMT
                              && a_this->kind.media_rule, CR_BAD_PARAM_ERROR);

        return cr_statement_nr_rules (a_this->kind.media_rule->rulesets);
}

/**
 * cr_statement_at_media_get_from_list:
 *
 *@a_this: the current instance of #CRStatement.
 *@itemnr: the index into the media rule list of rules.
 *
 *Use an index to get a CRStatement from the media rule list of rules.
 *
 *Returns CRStatement at position itemnr, if itemnr > number of rules - 1,
 *it will return NULL.
 */
CRStatement *
cr_statement_at_media_get_from_list (CRStatement * a_this, int itemnr)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_MEDIA_RULE_STMT
                              && a_this->kind.media_rule, NULL);

        return cr_statement_get_from_list (a_this->kind.media_rule->rulesets,
                                           itemnr);
}

/**
 * cr_statement_at_page_rule_set_declarations:
 *
 *@a_this: the current \@page rule statement.
 *@a_decl_list: the declaration list to add. Will be freed
 *by the current instance of #CRStatement when it is destroyed.
 *
 *Sets a declaration list to the current \@page rule statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_page_rule_set_declarations (CRStatement * a_this,
                                            CRDeclaration * a_decl_list)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_PAGE_RULE_STMT
                              && a_this->kind.page_rule, CR_BAD_PARAM_ERROR);

        if (a_this->kind.page_rule->decl_list) {
                cr_declaration_unref (a_this->kind.page_rule->decl_list);
        }

        a_this->kind.page_rule->decl_list = a_decl_list;

        if (a_decl_list) {
                cr_declaration_ref (a_decl_list);
        }

        return CR_OK;
}

/**
 * cr_statement_at_page_rule_get_declarations:
 *
 *@a_this: the current \@page rule statement.
 *@a_decl_list: out parameter. The returned declaration list.
 *
 *Gets the declaration list associated to the current \@page rule
 *statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_page_rule_get_declarations (CRStatement * a_this,
                                            CRDeclaration ** a_decl_list)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_PAGE_RULE_STMT
                              && a_this->kind.page_rule, CR_BAD_PARAM_ERROR);

        *a_decl_list = a_this->kind.page_rule->decl_list;

        return CR_OK;
}

/**
 * cr_statement_at_charset_rule_set_charset:
 *
 *
 *@a_this: the current \@charset rule statement.
 *@a_charset: the charset to set.
 *
 *Sets the charset of the current \@charset rule statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_charset_rule_set_charset (CRStatement * a_this,
                                          CRString * a_charset)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_CHARSET_RULE_STMT
                              && a_this->kind.charset_rule,
                              CR_BAD_PARAM_ERROR);

        if (a_this->kind.charset_rule->charset) {
                cr_string_destroy (a_this->kind.charset_rule->charset);
        }
        a_this->kind.charset_rule->charset = a_charset;
        return CR_OK;
}

/**
 * cr_statement_at_charset_rule_get_charset:
 *@a_this: the current \@charset rule statement.
 *@a_charset: out parameter. The returned charset string if
 *and only if the function returned CR_OK.
 *
 *Gets the charset string associated to the current
 *\@charset rule statement.
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_charset_rule_get_charset (CRStatement const * a_this,
                                          CRString ** a_charset)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_CHARSET_RULE_STMT
                              && a_this->kind.charset_rule,
                              CR_BAD_PARAM_ERROR);

        *a_charset = a_this->kind.charset_rule->charset;

        return CR_OK;
}

/**
 * cr_statement_at_font_face_rule_set_decls:
 *
 *@a_this: the current \@font-face rule statement.
 *@a_decls: the declarations list to set.
 *
 *Sets a declaration list to the current \@font-face rule statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_set_decls (CRStatement * a_this,
                                          CRDeclaration * a_decls)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_FONT_FACE_RULE_STMT
                              && a_this->kind.font_face_rule,
                              CR_BAD_PARAM_ERROR);

        if (a_this->kind.font_face_rule->decl_list) {
                cr_declaration_unref (a_this->kind.font_face_rule->decl_list);
        }

        a_this->kind.font_face_rule->decl_list = a_decls;
        cr_declaration_ref (a_decls);

        return CR_OK;
}

/**
 * cr_statement_at_font_face_rule_get_decls:
 *
 *@a_this: the current \@font-face rule statement.
 *@a_decls: out parameter. The returned declaration list if
 *and only if this function returns CR_OK.
 *
 *Gets the declaration list associated to the current instance
 *of \@font-face rule statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_get_decls (CRStatement * a_this,
                                          CRDeclaration ** a_decls)
{
        g_return_val_if_fail (a_this
                              && a_this->type == AT_FONT_FACE_RULE_STMT
                              && a_this->kind.font_face_rule,
                              CR_BAD_PARAM_ERROR);

        *a_decls = a_this->kind.font_face_rule->decl_list;

        return CR_OK;
}

/**
 * cr_statement_at_font_face_rule_add_decl:
 *
 *@a_this: the current \@font-face rule statement.
 *@a_prop: the property of the declaration.
 *@a_value: the value of the declaration.
 *
 *Adds a declaration to the current \@font-face rule
 *statement.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_add_decl (CRStatement * a_this,
                                         CRString * a_prop, CRTerm * a_value)
{
        CRDeclaration *decls = NULL;

        g_return_val_if_fail (a_this
                              && a_this->type == AT_FONT_FACE_RULE_STMT
                              && a_this->kind.font_face_rule,
                              CR_BAD_PARAM_ERROR);

        decls = cr_declaration_append2
                (a_this->kind.font_face_rule->decl_list, 
                 a_prop, a_value);

        g_return_val_if_fail (decls, CR_ERROR);

        if (a_this->kind.font_face_rule->decl_list == NULL)
                cr_declaration_ref (decls);

        a_this->kind.font_face_rule->decl_list = decls;

        return CR_OK;
}


/**
 * cr_statement_to_string:
 *
 *@a_this: the current statement to serialize
 *@a_indent: the number of white space of indentation.
 *
 *Serializes a css statement into a string
 *
 *Returns the serialized statement. Must be freed by the caller
 *using g_free().
 */
gchar *
cr_statement_to_string (CRStatement const * a_this, gulong a_indent)
{
        gchar *str = NULL ;

        if (!a_this)
                return NULL;

        switch (a_this->type) {
        case RULESET_STMT:
                str = cr_statement_ruleset_to_string 
                        (a_this, a_indent);
                break;

        case AT_FONT_FACE_RULE_STMT:
                str = cr_statement_font_face_rule_to_string 
                        (a_this, a_indent) ;
                break;

        case AT_CHARSET_RULE_STMT:
                str = cr_statement_charset_to_string
                        (a_this, a_indent);                
                break;

        case AT_PAGE_RULE_STMT:
                str = cr_statement_at_page_rule_to_string
                        (a_this, a_indent);
                break;

        case AT_MEDIA_RULE_STMT:
                str = cr_statement_media_rule_to_string
                        (a_this, a_indent);
                break;

        case AT_IMPORT_RULE_STMT:
                str = cr_statement_import_rule_to_string
                        (a_this, a_indent);
                break;

        default:
                cr_utils_trace_info ("Statement unrecognized");
                break;
        }
        return str ;
}

gchar*
cr_statement_list_to_string (CRStatement const *a_this, gulong a_indent)
{
        CRStatement const *cur_stmt = NULL ;
        GString *stringue = NULL ;
        gchar *str = NULL ;

        g_return_val_if_fail (a_this, NULL) ;

        stringue = g_string_new (NULL) ;
        if (!stringue) {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        for (cur_stmt = a_this ; cur_stmt;
             cur_stmt = cur_stmt->next) {
                str = cr_statement_to_string (cur_stmt, a_indent) ;
                if (str) {
                        if (!cur_stmt->prev) {
                                g_string_append (stringue, str) ;
                        } else {
                                g_string_append_printf 
                                        (stringue, "\n%s", str) ;
                        }
                        g_free (str) ;
                        str = NULL ;
                }                
        }
        str = stringue->str ;
        g_string_free (stringue, FALSE) ;
        return str ;
}

/**
 * cr_statement_dump:
 *
 *@a_this: the current css2 statement.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of white space indentation characters.
 *
 *Dumps the css2 statement to a file.
 */
void
cr_statement_dump (CRStatement const * a_this, FILE * a_fp, gulong a_indent)
{
        gchar *str = NULL ;

        if (!a_this)
                return;

        str = cr_statement_to_string (a_this, a_indent) ;
        if (str) {
                fprintf (a_fp, "%s",str) ;
                g_free (str) ;
                str = NULL ;
        }
}

/**
 * cr_statement_dump_ruleset:
 *
 *@a_this: the current instance of #CRStatement.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of indentation white spaces to add.
 *
 *Dumps a ruleset statement to a file.
 */
void
cr_statement_dump_ruleset (CRStatement const * a_this, FILE * a_fp, glong a_indent)
{
        gchar *str = NULL;

        g_return_if_fail (a_fp && a_this);
        str = cr_statement_ruleset_to_string (a_this, a_indent);
        if (str) {
                fprintf (a_fp, "%s", str);
                g_free (str);
                str = NULL;
        }
}

/**
 * cr_statement_dump_font_face_rule:
 *
 *@a_this: the current instance of font face rule statement.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of white space indentation.
 *
 *Dumps a font face rule statement to a file.
 */
void
cr_statement_dump_font_face_rule (CRStatement const * a_this, FILE * a_fp,
                                  glong a_indent)
{
        gchar *str = NULL ;
        g_return_if_fail (a_this 
                          && a_this->type == AT_FONT_FACE_RULE_STMT);

        str = cr_statement_font_face_rule_to_string (a_this,
                                                     a_indent) ;
        if (str) {
                fprintf (a_fp, "%s", str) ;
                g_free (str) ;
                str = NULL ;
        }
}

/**
 * cr_statement_dump_charset:
 *
 *@a_this: the current instance of the \@charset rule statement.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of indentation white spaces.
 *
 *Dumps an \@charset rule statement to a file.
 */
void
cr_statement_dump_charset (CRStatement const * a_this, FILE * a_fp, gulong a_indent)
{
        gchar *str = NULL;

        g_return_if_fail (a_this && a_this->type == AT_CHARSET_RULE_STMT);

        str = cr_statement_charset_to_string (a_this,
                                              a_indent) ;
        if (str) {
                fprintf (a_fp, "%s", str) ;
                g_free (str) ;
                str = NULL ;
        }
}


/**
 * cr_statement_dump_page:
 *
 *@a_this: the statement to dump on stdout.
 *@a_fp: the destination file pointer.
 *@a_indent: the number of indentation white spaces.
 *
 *Dumps an \@page rule statement on stdout.
 */
void
cr_statement_dump_page (CRStatement const * a_this, FILE * a_fp, gulong a_indent)
{
        gchar *str = NULL;

        g_return_if_fail (a_this
                          && a_this->type == AT_PAGE_RULE_STMT
                          && a_this->kind.page_rule);

        str = cr_statement_at_page_rule_to_string (a_this, a_indent) ;
        if (str) {
                fprintf (a_fp, "%s", str);
                g_free (str) ;
                str = NULL ; 
        }
}


/**
 * cr_statement_dump_media_rule:
 *
 *@a_this: the statement to dump.
 *@a_fp: the destination file pointer
 *@a_indent: the number of white spaces indentation.
 *
 *Dumps an \@media rule statement to a file.
 */
void
cr_statement_dump_media_rule (CRStatement const * a_this,
                              FILE * a_fp,
                              gulong a_indent)
{
        gchar *str = NULL ;
        g_return_if_fail (a_this->type == AT_MEDIA_RULE_STMT);

        str = cr_statement_media_rule_to_string (a_this, a_indent) ;
        if (str) {
                fprintf (a_fp, "%s", str) ;
                g_free (str) ;
                str = NULL ;
        }
}

/**
 * cr_statement_dump_import_rule:
 *
 *@a_fp: the destination file pointer.
 *@a_indent: the number of white space indentations.
 *
 *Dumps an \@import rule statement to a file.
 */
void
cr_statement_dump_import_rule (CRStatement const * a_this, FILE * a_fp,
                               gulong a_indent)
{
        gchar *str = NULL ;
        g_return_if_fail (a_this
                          && a_this->type == AT_IMPORT_RULE_STMT
                          && a_fp
                          && a_this->kind.import_rule);

        str = cr_statement_import_rule_to_string (a_this, a_indent) ;
        if (str) {
                fprintf (a_fp, "%s", str) ;
                g_free (str) ;
                str = NULL ;
        }
}

/**
 * cr_statement_destroy:
 *
 * @a_this: the current instance of #CRStatement.
 *
 *Destructor of #CRStatement.
 */
void
cr_statement_destroy (CRStatement * a_this)
{
        CRStatement *cur = NULL;

        g_return_if_fail (a_this);

        /*go get the tail of the list */
        for (cur = a_this; cur && cur->next; cur = cur->next) {
                cr_statement_clear (cur);
        }

        if (cur)
                cr_statement_clear (cur);

        if (cur->prev == NULL) {
                g_free (a_this);
                return;
        }

        /*walk backward and free next element */
        for (cur = cur->prev; cur && cur->prev; cur = cur->prev) {
                if (cur->next) {
                        g_free (cur->next);
                        cur->next = NULL;
                }
        }

        if (!cur)
                return;

        /*free the one remaining list */
        if (cur->next) {
                g_free (cur->next);
                cur->next = NULL;
        }

        g_free (cur);
        cur = NULL;
}
