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
#include "cr-utils.h"
#include "cr-om-parser.h"

/**
 *@CROMParser:
 *
 *The definition of the CSS Object Model Parser.
 *This parser uses (and sits) the SAC api of libcroco defined
 *in cr-parser.h and cr-doc-handler.h
 */

struct _CROMParserPriv {
        CRParser *parser;
};

#define PRIVATE(a_this) ((a_this)->priv)

/*
 *Forward declaration of a type defined later
 *in this file.
 */
struct _ParsingContext;
typedef struct _ParsingContext ParsingContext;

static ParsingContext *new_parsing_context (void);

static void destroy_context (ParsingContext * a_ctxt);

static void unrecoverable_error (CRDocHandler * a_this);

static void error (CRDocHandler * a_this);

static void property (CRDocHandler * a_this,
                      CRString * a_name, 
                      CRTerm * a_expression, 
                      gboolean a_important);

static void end_selector (CRDocHandler * a_this, 
                          CRSelector * a_selector_list);

static void start_selector (CRDocHandler * a_this, 
                            CRSelector * a_selector_list);

static void start_font_face (CRDocHandler * a_this,
                             CRParsingLocation *a_location);

static void end_font_face (CRDocHandler * a_this);

static void end_document (CRDocHandler * a_this);

static void start_document (CRDocHandler * a_this);

static void charset (CRDocHandler * a_this, 
                     CRString * a_charset,
                     CRParsingLocation *a_location);

static void start_page (CRDocHandler * a_this, CRString * a_page,
                        CRString * a_pseudo_page, 
                        CRParsingLocation *a_location);

static void end_page (CRDocHandler * a_this, CRString * a_page, 
                      CRString * a_pseudo_page);

static void start_media (CRDocHandler * a_this, 
                         GList * a_media_list,
                         CRParsingLocation *a_location);

static void end_media (CRDocHandler * a_this, 
                       GList * a_media_list);

static void import_style (CRDocHandler * a_this, 
                          GList * a_media_list,
                          CRString * a_uri, 
                          CRString * a_uri_default_ns,
                          CRParsingLocation *a_location);

struct _ParsingContext {
        CRStyleSheet *stylesheet;
        CRStatement *cur_stmt;
        CRStatement *cur_media_stmt;
};

/********************************************
 *Private methods
 ********************************************/

static ParsingContext *
new_parsing_context (void)
{
        ParsingContext *result = NULL;

        result = g_try_malloc (sizeof (ParsingContext));
        if (!result) {
                cr_utils_trace_info ("Out of Memory");
                return NULL;
        }
        memset (result, 0, sizeof (ParsingContext));
        return result;
}

static void
destroy_context (ParsingContext * a_ctxt)
{
        g_return_if_fail (a_ctxt);

        if (a_ctxt->stylesheet) {
                cr_stylesheet_destroy (a_ctxt->stylesheet);
                a_ctxt->stylesheet = NULL;
        }
        if (a_ctxt->cur_stmt) {
                cr_statement_destroy (a_ctxt->cur_stmt);
                a_ctxt->cur_stmt = NULL;
        }
        g_free (a_ctxt);
}

static enum CRStatus
cr_om_parser_init_default_sac_handler (CROMParser * a_this)
{
        CRDocHandler *sac_handler = NULL;
        gboolean created_handler = FALSE;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->parser,
                              CR_BAD_PARAM_ERROR);

        status = cr_parser_get_sac_handler (PRIVATE (a_this)->parser,
                                            &sac_handler);
        g_return_val_if_fail (status == CR_OK, status);

        if (!sac_handler) {
                sac_handler = cr_doc_handler_new ();
                created_handler = TRUE;
        }

        /*
         *initialyze here the sac handler.
         */
        sac_handler->start_document = start_document;
        sac_handler->end_document = end_document;
        sac_handler->start_selector = start_selector;
        sac_handler->end_selector = end_selector;
        sac_handler->property = property;
        sac_handler->start_font_face = start_font_face;
        sac_handler->end_font_face = end_font_face;
        sac_handler->error = error;
        sac_handler->unrecoverable_error = unrecoverable_error;
        sac_handler->charset = charset;
        sac_handler->start_page = start_page;
        sac_handler->end_page = end_page;
        sac_handler->start_media = start_media;
        sac_handler->end_media = end_media;
        sac_handler->import_style = import_style;

        if (created_handler) {
                status = cr_parser_set_sac_handler (PRIVATE (a_this)->parser,
                                                    sac_handler);
                cr_doc_handler_unref (sac_handler);
        }

        return status;

}

static void
start_document (CRDocHandler * a_this)
{
        ParsingContext *ctxt = NULL;
        CRStyleSheet *stylesheet = NULL;

        g_return_if_fail (a_this);

        ctxt = new_parsing_context ();
        g_return_if_fail (ctxt);

        stylesheet = cr_stylesheet_new (NULL);
        ctxt->stylesheet = stylesheet;
        cr_doc_handler_set_ctxt (a_this, ctxt);
}

static void
start_font_face (CRDocHandler * a_this,
                 CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);
        g_return_if_fail (ctxt->cur_stmt == NULL);

        ctxt->cur_stmt =
                cr_statement_new_at_font_face_rule (ctxt->stylesheet, NULL);

        g_return_if_fail (ctxt->cur_stmt);
}

static void
end_font_face (CRDocHandler * a_this)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        CRStatement *stmts = NULL;

        g_return_if_fail (a_this);

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);
        g_return_if_fail
                (ctxt->cur_stmt
                 && ctxt->cur_stmt->type == AT_FONT_FACE_RULE_STMT
                 && ctxt->stylesheet);

        stmts = cr_statement_append (ctxt->stylesheet->statements,
                                     ctxt->cur_stmt);
        if (!stmts)
                goto error;

        ctxt->stylesheet->statements = stmts;
        stmts = NULL;
        ctxt->cur_stmt = NULL;

        return;

      error:

        if (ctxt->cur_stmt) {
                cr_statement_destroy (ctxt->cur_stmt);
                ctxt->cur_stmt = NULL;
        }

        if (!stmts) {
                cr_statement_destroy (stmts);
                stmts = NULL;
        }
}

static void
end_document (CRDocHandler * a_this)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);

        if (!ctxt->stylesheet || ctxt->cur_stmt)
                goto error;

        status = cr_doc_handler_set_result (a_this, ctxt->stylesheet);
        g_return_if_fail (status == CR_OK);

        ctxt->stylesheet = NULL;
        destroy_context (ctxt);
        cr_doc_handler_set_ctxt (a_this, NULL);

        return;

      error:
        if (ctxt) {
                destroy_context (ctxt);
        }
}

static void
charset (CRDocHandler * a_this, CRString * a_charset,
         CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        CRStatement *stmt = NULL,
                *stmt2 = NULL;
        CRString *charset = NULL;

        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);
        g_return_if_fail (ctxt->stylesheet);

        charset = cr_string_dup (a_charset) ;
        stmt = cr_statement_new_at_charset_rule (ctxt->stylesheet, charset);
        g_return_if_fail (stmt);
        stmt2 = cr_statement_append (ctxt->stylesheet->statements, stmt);
        if (!stmt2) {
                if (stmt) {
                        cr_statement_destroy (stmt);
                        stmt = NULL;
                }
                if (charset) {
                        cr_string_destroy (charset);
                }
                return;
        }
        ctxt->stylesheet->statements = stmt2;
        stmt2 = NULL;
}

static void
start_page (CRDocHandler * a_this, 
            CRString * a_page, 
            CRString * a_pseudo,
            CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);
        g_return_if_fail (ctxt->cur_stmt == NULL);

        ctxt->cur_stmt = cr_statement_new_at_page_rule
                (ctxt->stylesheet, NULL, NULL, NULL);
        if (a_page) {
                ctxt->cur_stmt->kind.page_rule->name =
                        cr_string_dup (a_page) ;

                if (!ctxt->cur_stmt->kind.page_rule->name) {
                        goto error;
                }
        }
        if (a_pseudo) {
                ctxt->cur_stmt->kind.page_rule->pseudo =
                        cr_string_dup (a_pseudo) ;
                if (!ctxt->cur_stmt->kind.page_rule->pseudo) {
                        goto error;
                }
        }
        return;

 error:
        if (ctxt->cur_stmt) {
                cr_statement_destroy (ctxt->cur_stmt);
                ctxt->cur_stmt = NULL;
        }
}

static void
end_page (CRDocHandler * a_this, 
          CRString * a_page, 
          CRString * a_pseudo_page)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        CRStatement *stmt = NULL;

        (void) a_page;
        (void) a_pseudo_page;

        g_return_if_fail (a_this);

	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);

        g_return_if_fail (status == CR_OK && ctxt);

        g_return_if_fail (ctxt->cur_stmt
                          && ctxt->cur_stmt->type == AT_PAGE_RULE_STMT
                          && ctxt->stylesheet);

        stmt = cr_statement_append (ctxt->stylesheet->statements,
                                    ctxt->cur_stmt);

        if (stmt) {
                ctxt->stylesheet->statements = stmt;
                stmt = NULL;
                ctxt->cur_stmt = NULL;
        }

        if (ctxt->cur_stmt) {
                cr_statement_destroy (ctxt->cur_stmt);
                ctxt->cur_stmt = NULL;
        }
        a_page = NULL;          /*keep compiler happy */
        a_pseudo_page = NULL;   /*keep compiler happy */
}

static void
start_media (CRDocHandler * a_this, 
             GList * a_media_list,
             CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        GList *media_list = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);

        g_return_if_fail (ctxt
                          && ctxt->cur_stmt == NULL
                          && ctxt->cur_media_stmt == NULL
                          && ctxt->stylesheet);
        if (a_media_list) {
                /*duplicate the media_list */
                media_list = cr_utils_dup_glist_of_cr_string 
                        (a_media_list);
        }
        ctxt->cur_media_stmt =
                cr_statement_new_at_media_rule
                (ctxt->stylesheet, NULL, media_list);

}

static void
end_media (CRDocHandler * a_this, GList * a_media_list)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        CRStatement *stmts = NULL;

        (void) a_media_list;

        g_return_if_fail (a_this);

	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);

        g_return_if_fail (status == CR_OK && ctxt);

        g_return_if_fail (ctxt
                          && ctxt->cur_media_stmt
                          && ctxt->cur_media_stmt->type == AT_MEDIA_RULE_STMT
                          && ctxt->stylesheet);

        stmts = cr_statement_append (ctxt->stylesheet->statements,
                                     ctxt->cur_media_stmt);

        if (!stmts) {
                cr_statement_destroy (ctxt->cur_media_stmt);
                ctxt->cur_media_stmt = NULL;
        }

        ctxt->stylesheet->statements = stmts;
        stmts = NULL;

        ctxt->cur_stmt = NULL ;
        ctxt->cur_media_stmt = NULL ;
        a_media_list = NULL;
}

static void
import_style (CRDocHandler * a_this, 
              GList * a_media_list,
              CRString * a_uri, 
              CRString * a_uri_default_ns,
              CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        CRString *uri = NULL;
        CRStatement *stmt = NULL,
                *stmt2 = NULL;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        GList *media_list = NULL ;

        (void) a_uri_default_ns;

        g_return_if_fail (a_this);

	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);

        g_return_if_fail (status == CR_OK && ctxt);

        g_return_if_fail (ctxt->stylesheet);

        uri = cr_string_dup (a_uri) ;

        if (a_media_list)
                media_list = cr_utils_dup_glist_of_cr_string (a_media_list) ;

        stmt = cr_statement_new_at_import_rule
                (ctxt->stylesheet, uri, media_list, NULL);

        if (!stmt)
                goto error;

        if (ctxt->cur_stmt) {
                stmt2 = cr_statement_append (ctxt->cur_stmt, stmt);
                if (!stmt2)
                        goto error;
                ctxt->cur_stmt = stmt2;
                stmt2 = NULL;
                stmt = NULL;
        } else {
                stmt2 = cr_statement_append (ctxt->stylesheet->statements,
                                             stmt);
                if (!stmt2)
                        goto error;
                ctxt->stylesheet->statements = stmt2;
                stmt2 = NULL;
                stmt = NULL;
        }

        return;

      error:
        if (uri) {
                cr_string_destroy (uri);
        }

        if (stmt) {
                cr_statement_destroy (stmt);
                stmt = NULL;
        }
        a_uri_default_ns = NULL; /*keep compiler happy */
}

static void
start_selector (CRDocHandler * a_this, CRSelector * a_selector_list)
{
        enum CRStatus status = CR_OK ;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);
        if (ctxt->cur_stmt) {
                /*hmm, this should be NULL so free it */
                cr_statement_destroy (ctxt->cur_stmt);
                ctxt->cur_stmt = NULL;
        }

        ctxt->cur_stmt = cr_statement_new_ruleset
                (ctxt->stylesheet, a_selector_list, NULL, NULL);
}

static void
end_selector (CRDocHandler * a_this, CRSelector * a_selector_list)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        (void) a_selector_list;

        g_return_if_fail (a_this);

	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);

        g_return_if_fail (status == CR_OK && ctxt);

        g_return_if_fail (ctxt->cur_stmt && ctxt->stylesheet);

        if (ctxt->cur_stmt) {
                CRStatement *stmts = NULL;

                if (ctxt->cur_media_stmt) {
                        CRAtMediaRule *media_rule = NULL;

                        media_rule = ctxt->cur_media_stmt->kind.media_rule;

                        stmts = cr_statement_append
                                (media_rule->rulesets, ctxt->cur_stmt);

                        if (!stmts) {
                                cr_utils_trace_info
                                        ("Could not append a new statement");
                                cr_statement_destroy (media_rule->rulesets);
                                ctxt->cur_media_stmt->
                                        kind.media_rule->rulesets = NULL;
                                return;
                        }
                        media_rule->rulesets = stmts;
                        ctxt->cur_stmt = NULL;
                } else {
                        stmts = cr_statement_append
                                (ctxt->stylesheet->statements,
                                 ctxt->cur_stmt);
                        if (!stmts) {
                                cr_utils_trace_info
                                        ("Could not append a new statement");
                                cr_statement_destroy (ctxt->cur_stmt);
                                ctxt->cur_stmt = NULL;
                                return;
                        }
                        ctxt->stylesheet->statements = stmts;
                        ctxt->cur_stmt = NULL;
                }

        }

        a_selector_list = NULL; /*keep compiler happy */
}

static void
property (CRDocHandler * a_this,
          CRString * a_name, 
          CRTerm * a_expression, 
          gboolean a_important)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;
        CRDeclaration *decl = NULL,
                *decl2 = NULL;
        CRString *str = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);

        /*
         *make sure a current ruleset statement has been allocated
         *already.
         */
        g_return_if_fail
                (ctxt->cur_stmt
                 &&
                 (ctxt->cur_stmt->type == RULESET_STMT
                  || ctxt->cur_stmt->type == AT_FONT_FACE_RULE_STMT
                  || ctxt->cur_stmt->type == AT_PAGE_RULE_STMT));

        if (a_name) {
                str = cr_string_dup (a_name);
                g_return_if_fail (str);
        }

        /*instanciates a new declaration */
        decl = cr_declaration_new (ctxt->cur_stmt, str, a_expression);
        g_return_if_fail (decl);
        str = NULL;
        decl->important = a_important;
        /*
         *add the new declaration to the current statement
         *being build.
         */
        switch (ctxt->cur_stmt->type) {
        case RULESET_STMT:
                decl2 = cr_declaration_append
                        (ctxt->cur_stmt->kind.ruleset->decl_list, decl);
                if (!decl2) {
                        cr_declaration_destroy (decl);
                        cr_utils_trace_info
                                ("Could not append decl to ruleset");
                        goto error;
                }
                ctxt->cur_stmt->kind.ruleset->decl_list = decl2;
                decl = NULL;
                decl2 = NULL;
                break;

        case AT_FONT_FACE_RULE_STMT:
                decl2 = cr_declaration_append
                        (ctxt->cur_stmt->kind.font_face_rule->decl_list,
                         decl);
                if (!decl2) {
                        cr_declaration_destroy (decl);
                        cr_utils_trace_info
                                ("Could not append decl to ruleset");
                        goto error;
                }
                ctxt->cur_stmt->kind.font_face_rule->decl_list = decl2;
                decl = NULL;
                decl2 = NULL;
                break;
        case AT_PAGE_RULE_STMT:
                decl2 = cr_declaration_append
                        (ctxt->cur_stmt->kind.page_rule->decl_list, decl);
                if (!decl2) {
                        cr_declaration_destroy (decl);
                        cr_utils_trace_info
                                ("Could not append decl to ruleset");
                        goto error;
                }
                ctxt->cur_stmt->kind.page_rule->decl_list = decl2;
                decl = NULL;
                decl2 = NULL;
                break;

        default:
                goto error;
                break;
        }

        return;

      error:
        if (str) {
                g_free (str);
                str = NULL;
        }

        if (decl) {
                cr_declaration_destroy (decl);
                decl = NULL;
        }
}

static void
error (CRDocHandler * a_this)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

        g_return_if_fail (a_this);
	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK && ctxt);

        if (ctxt->cur_stmt) {
                cr_statement_destroy (ctxt->cur_stmt);
                ctxt->cur_stmt = NULL;
        }
}

static void
unrecoverable_error (CRDocHandler * a_this)
{
        enum CRStatus status = CR_OK;
        ParsingContext *ctxt = NULL;
        ParsingContext **ctxtptr = NULL;

	ctxtptr = &ctxt;
        status = cr_doc_handler_get_ctxt (a_this, (gpointer *) ctxtptr);
        g_return_if_fail (status == CR_OK);

        if (ctxt) {
                if (ctxt->stylesheet) {
                        status = cr_doc_handler_set_result
                                (a_this, ctxt->stylesheet);
                        g_return_if_fail (status == CR_OK);
                }
                g_free (ctxt);
                cr_doc_handler_set_ctxt (a_this, NULL);
        }
}

/********************************************
 *Public methods
 ********************************************/

/**
 * cr_om_parser_new:
 *@a_input: the input stream.
 *
 *Constructor of the CROMParser.
 *Returns the newly built instance of #CROMParser.
 */
CROMParser *
cr_om_parser_new (CRInput * a_input)
{
        CROMParser *result = NULL;
        enum CRStatus status = CR_OK;

        result = g_try_malloc (sizeof (CROMParser));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CROMParser));
        PRIVATE (result) = g_try_malloc (sizeof (CROMParserPriv));

        if (!PRIVATE (result)) {
                cr_utils_trace_info ("Out of memory");
                goto error;
        }

        memset (PRIVATE (result), 0, sizeof (CROMParserPriv));

        PRIVATE (result)->parser = cr_parser_new_from_input (a_input);

        if (!PRIVATE (result)->parser) {
                cr_utils_trace_info ("parsing instantiation failed");
                goto error;
        }

        status = cr_om_parser_init_default_sac_handler (result);

        if (status != CR_OK) {
                goto error;
        }

        return result;

      error:

        if (result) {
                cr_om_parser_destroy (result);
        }

        return NULL;
}

/**
 * cr_om_parser_parse_buf:
 *@a_this: the current instance of #CROMParser.
 *@a_buf: the in memory buffer to parse.
 *@a_len: the length of the in memory buffer in number of bytes.
 *@a_enc: the encoding of the in memory buffer.
 *@a_result: out parameter the resulting style sheet
 *
 *Parses the content of an in memory  buffer.
 *
 *Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_om_parser_parse_buf (CROMParser * a_this,
                        const guchar * a_buf,
                        gulong a_len,
                        enum CREncoding a_enc, CRStyleSheet ** a_result)
{

        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && a_result, CR_BAD_PARAM_ERROR);

        if (!PRIVATE (a_this)->parser) {
                PRIVATE (a_this)->parser = cr_parser_new (NULL);
        }

        status = cr_parser_parse_buf (PRIVATE (a_this)->parser,
                                      a_buf, a_len, a_enc);

        if (status == CR_OK) {
                CRStyleSheet *result = NULL;
                CRStyleSheet **resultptr = NULL;
                CRDocHandler *sac_handler = NULL;

                cr_parser_get_sac_handler (PRIVATE (a_this)->parser,
                                           &sac_handler);
                g_return_val_if_fail (sac_handler, CR_ERROR);
		resultptr = &result;
                status = cr_doc_handler_get_result (sac_handler,
                                                    (gpointer *) resultptr);
                g_return_val_if_fail (status == CR_OK, status);

                if (result)
                        *a_result = result;
        }

        return status;
}

/**
 * cr_om_parser_simply_parse_buf:
 *@a_buf: the css2 in memory buffer.
 *@a_len: the length of the in memory buffer.
 *@a_enc: the encoding of the in memory buffer.
 *@a_result: out parameter. The resulting css2 style sheet.
 *
 *The simpler way to parse an in memory css2 buffer.
 *
 *Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_om_parser_simply_parse_buf (const guchar * a_buf,
                               gulong a_len,
                               enum CREncoding a_enc,
                               CRStyleSheet ** a_result)
{
        CROMParser *parser = NULL;
        enum CRStatus status = CR_OK;

        parser = cr_om_parser_new (NULL);
        if (!parser) {
                cr_utils_trace_info ("Could not create om parser");
                cr_utils_trace_info ("System possibly out of memory");
                return CR_ERROR;
        }

        status = cr_om_parser_parse_buf (parser, a_buf, a_len,
                                         a_enc, a_result);

        if (parser) {
                cr_om_parser_destroy (parser);
                parser = NULL;
        }

        return status;
}

/**
 * cr_om_parser_parse_file:
 *@a_this: the current instance of the cssom parser.
 *@a_file_uri: the uri of the file. 
 *(only local file paths are suppported so far)
 *@a_enc: the encoding of the file.
 *@a_result: out parameter. A pointer 
 *the build css object model.
 *
 *Parses a css2 stylesheet contained
 *in a file.
 *
 * Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus
cr_om_parser_parse_file (CROMParser * a_this,
                         const guchar * a_file_uri,
                         enum CREncoding a_enc, CRStyleSheet ** a_result)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && a_file_uri && a_result,
                              CR_BAD_PARAM_ERROR);

        if (!PRIVATE (a_this)->parser) {
                PRIVATE (a_this)->parser = cr_parser_new_from_file
                        (a_file_uri, a_enc);
        }

        status = cr_parser_parse_file (PRIVATE (a_this)->parser,
                                       a_file_uri, a_enc);

        if (status == CR_OK) {
                CRStyleSheet *result = NULL;
                CRStyleSheet **resultptr = NULL;
                CRDocHandler *sac_handler = NULL;

                cr_parser_get_sac_handler (PRIVATE (a_this)->parser,
                                           &sac_handler);
                g_return_val_if_fail (sac_handler, CR_ERROR);
		resultptr = &result;
                status = cr_doc_handler_get_result
                        (sac_handler, (gpointer *) resultptr);
                g_return_val_if_fail (status == CR_OK, status);
                if (result)
                        *a_result = result;
        }

        return status;
}

/**
 * cr_om_parser_simply_parse_file:
 *@a_file_path: the css2 local file path.
 *@a_enc: the file encoding.
 *@a_result: out parameter. The returned css stylesheet.
 *Must be freed by the caller using cr_stylesheet_destroy.
 *
 *The simpler method to parse a css2 file.
 *
 *Returns CR_OK upon successfull completion, an error code otherwise.
 *Note that this method uses cr_om_parser_parse_file() so both methods
 *have the same return values.
 */
enum CRStatus
cr_om_parser_simply_parse_file (const guchar * a_file_path,
                                enum CREncoding a_enc,
                                CRStyleSheet ** a_result)
{
        CROMParser *parser = NULL;
        enum CRStatus status = CR_OK;

        parser = cr_om_parser_new (NULL);
        if (!parser) {
                cr_utils_trace_info ("Could not allocate om parser");
                cr_utils_trace_info ("System may be out of memory");
                return CR_ERROR;
        }

        status = cr_om_parser_parse_file (parser, a_file_path,
                                          a_enc, a_result);
        if (parser) {
                cr_om_parser_destroy (parser);
                parser = NULL;
        }

        return status;
}

/**
 * cr_om_parser_parse_paths_to_cascade:
 *@a_this: the current instance of #CROMParser
 *@a_author_path: the path to the author stylesheet
 *@a_user_path: the path to the user stylesheet
 *@a_ua_path: the path to the User Agent stylesheet
 *@a_encoding: the encoding of the sheets.
 *@a_result: out parameter. The resulting cascade if the parsing
 *was okay
 *
 *Parses three sheets located by their paths and build a cascade
 *
 *Returns CR_OK upon successful completion, an error code otherwise
 */
enum CRStatus
cr_om_parser_parse_paths_to_cascade (CROMParser * a_this,
                                     const guchar * a_author_path,
                                     const guchar * a_user_path,
                                     const guchar * a_ua_path,
                                     enum CREncoding a_encoding,
                                     CRCascade ** a_result)
{
        enum CRStatus status = CR_OK;

        /*0->author sheet, 1->user sheet, 2->UA sheet */
        CRStyleSheet *sheets[3];
        guchar *paths[3];
        CRCascade *result = NULL;
        gint i = 0;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        memset (sheets, 0, sizeof (CRStyleSheet*) * 3);
        paths[0] = (guchar *) a_author_path;
        paths[1] = (guchar *) a_user_path;
        paths[2] = (guchar *) a_ua_path;

        for (i = 0; i < 3; i++) {
                status = cr_om_parser_parse_file (a_this, paths[i],
                                                  a_encoding, &sheets[i]);
                if (status != CR_OK) {
                        if (sheets[i]) {
                                cr_stylesheet_unref (sheets[i]);
                                sheets[i] = NULL;
                        }
                        continue;
                }
        }
        result = cr_cascade_new (sheets[0], sheets[1], sheets[2]);
        if (!result) {
                for (i = 0; i < 3; i++) {
                        cr_stylesheet_unref (sheets[i]);
                        sheets[i] = 0;
                }
                return CR_ERROR;
        }
        *a_result = result;
        return CR_OK;
}

/**
 * cr_om_parser_simply_parse_paths_to_cascade:
 *@a_author_path: the path to the author stylesheet
 *@a_user_path: the path to the user stylesheet
 *@a_ua_path: the path to the User Agent stylesheet
 *@a_encoding: the encoding of the sheets.
 *@a_result: out parameter. The resulting cascade if the parsing
 *was okay
 *
 *Parses three sheets located by their paths and build a cascade
 *
 *Returns CR_OK upon successful completion, an error code otherwise
 */
enum CRStatus
cr_om_parser_simply_parse_paths_to_cascade (const guchar * a_author_path,
                                            const guchar * a_user_path,
                                            const guchar * a_ua_path,
                                            enum CREncoding a_encoding,
                                            CRCascade ** a_result)
{
        enum CRStatus status = CR_OK;
        CROMParser *parser = NULL;

        parser = cr_om_parser_new (NULL);
        if (!parser) {
                cr_utils_trace_info ("could not allocated om parser");
                cr_utils_trace_info ("System may be out of memory");
                return CR_ERROR;
        }
        status = cr_om_parser_parse_paths_to_cascade (parser,
                                                      a_author_path,
                                                      a_user_path,
                                                      a_ua_path,
                                                      a_encoding, a_result);
        if (parser) {
                cr_om_parser_destroy (parser);
                parser = NULL;
        }
        return status;
}

/**
 * cr_om_parser_destroy:
 *@a_this: the current instance of #CROMParser.
 *
 *Destructor of the #CROMParser.
 */
void
cr_om_parser_destroy (CROMParser * a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this));

        if (PRIVATE (a_this)->parser) {
                cr_parser_destroy (PRIVATE (a_this)->parser);
                PRIVATE (a_this)->parser = NULL;
        }

        if (PRIVATE (a_this)) {
                g_free (PRIVATE (a_this));
                PRIVATE (a_this) = NULL;
        }

        if (a_this) {
                g_free (a_this);
                a_this = NULL;
        }
}
