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
 * You should have received a copy of the GNU Lesser 
 * General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <string.h>
#include "cr-sel-eng.h"

/**
 *@CRSelEng:
 *
 *The definition of the  #CRSelEng class.
 *The #CRSelEng is actually the "Selection Engine"
 *class. This is highly experimental for at the moment and
 *its api is very likely to change in a near future.
 */

#define PRIVATE(a_this) (a_this)->priv

struct CRPseudoClassSelHandlerEntry {
        guchar *name;
        enum CRPseudoType type;
        CRPseudoClassSelectorHandler handler;
};

struct _CRSelEngPriv {
        /*not used yet */
        gboolean case_sensitive;

        CRStyleSheet *sheet;
        /**
         *where to store the next statement
         *to be visited so that we can remember
         *it from one method call to another.
         */
        CRStatement *cur_stmt;
        GList *pcs_handlers;
        gint pcs_handlers_size;
} ;

static gboolean class_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                            xmlNode * a_node);

static gboolean id_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                         xmlNode * a_node);

static gboolean attr_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                           xmlNode * a_node);

static enum CRStatus sel_matches_node_real (CRSelEng * a_this,
                                            CRSimpleSel * a_sel,
                                            xmlNode * a_node,
                                            gboolean * a_result,
                                            gboolean a_eval_sel_list_from_end,
                                            gboolean a_recurse);

static enum CRStatus cr_sel_eng_get_matched_rulesets_real (CRSelEng * a_this,
                                                           CRStyleSheet *
                                                           a_stylesheet,
                                                           xmlNode * a_node,
                                                           CRStatement **
                                                           a_rulesets,
                                                           gulong * a_len);

static enum CRStatus put_css_properties_in_props_list (CRPropList ** a_props,
                                                       CRStatement *
                                                       a_ruleset);

static gboolean pseudo_class_add_sel_matches_node (CRSelEng * a_this,
                                                   CRAdditionalSel *
                                                   a_add_sel,
                                                   xmlNode * a_node);

static gboolean lang_pseudo_class_handler (CRSelEng * a_this,
                                           CRAdditionalSel * a_sel,
                                           xmlNode * a_node);

static gboolean first_child_pseudo_class_handler (CRSelEng * a_this,
                                                  CRAdditionalSel * a_sel,
                                                  xmlNode * a_node);

static xmlNode *get_next_element_node (xmlNode * a_node);

static xmlNode *get_next_child_element_node (xmlNode * a_node);

static xmlNode *get_prev_element_node (xmlNode * a_node);

static xmlNode *get_next_parent_element_node (xmlNode * a_node);

/* Quick strcmp.  Test only for == 0 or != 0, not < 0 or > 0.  */
#define strqcmp(str,lit,lit_len) \
  (strlen (str) != (lit_len) || memcmp (str, lit, lit_len))

static gboolean
lang_pseudo_class_handler (CRSelEng * a_this,
                           CRAdditionalSel * a_sel, xmlNode * a_node)
{
        xmlNode *node = a_node;
        xmlChar *val = NULL;
        gboolean result = FALSE;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, CR_BAD_PARAM_ERROR);

        if (strqcmp (a_sel->content.pseudo->name->stryng->str, 
                     "lang", 4)
            || a_sel->content.pseudo->type != FUNCTION_PSEUDO) {
                cr_utils_trace_info ("This handler is for :lang only");
                return CR_BAD_PSEUDO_CLASS_SEL_HANDLER_ERROR;
        }
        /*lang code should exist and be at least of length 2 */
        if (!a_sel->content.pseudo->extra
            || !a_sel->content.pseudo->extra->stryng
            || a_sel->content.pseudo->extra->stryng->len < 2)
                return FALSE;
        for (; node; node = get_next_parent_element_node (node)) {
                val = xmlGetProp (node, (const xmlChar *) "lang");
                if (val
                    && !strqcmp ((const char *) val,
                                 a_sel->content.pseudo->extra->stryng->str,
                                 a_sel->content.pseudo->extra->stryng->len)) {
                        result = TRUE;
                }
                if (val) {
                        xmlFree (val);
                        val = NULL;
                }
        }

        return result;
}

static gboolean
first_child_pseudo_class_handler (CRSelEng * a_this,
                                  CRAdditionalSel * a_sel, xmlNode * a_node)
{
        xmlNode *node = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, CR_BAD_PARAM_ERROR);

        if (strcmp (a_sel->content.pseudo->name->stryng->str,
                    "first-child")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :first-child only");
                return CR_BAD_PSEUDO_CLASS_SEL_HANDLER_ERROR;
        }
        if (!a_node->parent)
                return FALSE;
        node = get_next_child_element_node (a_node->parent);
        if (node == a_node)
                return TRUE;
        return FALSE;
}

static gboolean
pseudo_class_add_sel_matches_node (CRSelEng * a_this,
                                   CRAdditionalSel * a_add_sel,
                                   xmlNode * a_node)
{
        enum CRStatus status = CR_OK;
        CRPseudoClassSelectorHandler handler = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_add_sel
                              && a_add_sel->content.pseudo
                              && a_add_sel->content.pseudo->name
                              && a_add_sel->content.pseudo->name->stryng
                              && a_add_sel->content.pseudo->name->stryng->str
                              && a_node, CR_BAD_PARAM_ERROR);

        status = cr_sel_eng_get_pseudo_class_selector_handler
                (a_this, (guchar *) a_add_sel->content.pseudo->name->stryng->str,
                 a_add_sel->content.pseudo->type, &handler);
        if (status != CR_OK || !handler)
                return FALSE;

        return handler (a_this, a_add_sel, a_node);
}

/**
 *@param a_add_sel the class additional selector to consider.
 *@param a_node the xml node to consider.
 *@return TRUE if the class additional selector matches
 *the xml node given in argument, FALSE otherwise.
 */
static gboolean
class_add_sel_matches_node (CRAdditionalSel * a_add_sel, xmlNode * a_node)
{
        gboolean result = FALSE;
        xmlChar *klass = NULL,
                *cur = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == CLASS_ADD_SELECTOR
                              && a_add_sel->content.class_name
                              && a_add_sel->content.class_name->stryng
                              && a_add_sel->content.class_name->stryng->str
                              && a_node, FALSE);

        if (xmlHasProp (a_node, (const xmlChar *) "class")) {
                klass = xmlGetProp (a_node, (const xmlChar *) "class");
                for (cur = klass; cur && *cur; cur++) {
                        while (cur && *cur
                               && cr_utils_is_white_space (*cur) 
                               == TRUE)
                                cur++;

                        if (!strncmp ((const char *) cur, 
                                      a_add_sel->content.class_name->stryng->str,
                                      a_add_sel->content.class_name->stryng->len)) {
                                cur += a_add_sel->content.class_name->stryng->len;
                                if ((cur && !*cur)
                                    || cr_utils_is_white_space (*cur) == TRUE)
                                        result = TRUE;
                        } else {  /* if it doesn't match,  */
                                /*   then skip to next whitespace character to try again */
                                while (cur && *cur && !(cr_utils_is_white_space(*cur) == TRUE)) 
                                        cur++;
                        }
                        if (cur && !*cur)
                                break ;
                }
        }
        if (klass) {
                xmlFree (klass);
                klass = NULL;
        }
        return result;

}

/**
 *@return TRUE if the additional attribute selector matches
 *the current xml node given in argument, FALSE otherwise.
 *@param a_add_sel the additional attribute selector to consider.
 *@param a_node the xml node to consider.
 */
static gboolean
id_add_sel_matches_node (CRAdditionalSel * a_add_sel, xmlNode * a_node)
{
        gboolean result = FALSE;
        xmlChar *id = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_add_sel->content.id_name
                              && a_add_sel->content.id_name->stryng
                              && a_add_sel->content.id_name->stryng->str
                              && a_node, FALSE);
        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_node, FALSE);

        if (xmlHasProp (a_node, (const xmlChar *) "id")) {
                id = xmlGetProp (a_node, (const xmlChar *) "id");
                if (!strqcmp ((const char *) id, a_add_sel->content.id_name->stryng->str,
                              a_add_sel->content.id_name->stryng->len)) {
                        result = TRUE;
                }
        }
        if (id) {
                xmlFree (id);
                id = NULL;
        }
        return result;
}

/**
 *Returns TRUE if the instance of #CRAdditional selector matches
 *the node given in parameter, FALSE otherwise.
 *@param a_add_sel the additional selector to evaluate.
 *@param a_node the xml node against whitch the selector is to
 *be evaluated
 *return TRUE if the additional selector matches the current xml node
 *FALSE otherwise.
 */
static gboolean
attr_add_sel_matches_node (CRAdditionalSel * a_add_sel, xmlNode * a_node)
{
        CRAttrSel *cur_sel = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ATTRIBUTE_ADD_SELECTOR
                              && a_node, FALSE);

        for (cur_sel = a_add_sel->content.attr_sel;
             cur_sel; cur_sel = cur_sel->next) {
                switch (cur_sel->match_way) {
                case SET:
                        if (!cur_sel->name 
                            || !cur_sel->name->stryng
                            || !cur_sel->name->stryng->str)
                                return FALSE;

                        if (!xmlHasProp (a_node,
                                         (const xmlChar *) cur_sel->name->stryng->str))
                                return FALSE;
                        break;

                case EQUALS:
                        {
                                xmlChar *value = NULL;

                                if (!cur_sel->name 
                                    || !cur_sel->name->stryng
                                    || !cur_sel->name->stryng->str
                                    || !cur_sel->value
                                    || !cur_sel->value->stryng
                                    || !cur_sel->value->stryng->str)
                                        return FALSE;

                                if (!xmlHasProp 
                                    (a_node, 
                                     (const xmlChar *) cur_sel->name->stryng->str))
                                        return FALSE;

                                value = xmlGetProp 
                                        (a_node,
                                         (const xmlChar *) cur_sel->name->stryng->str);

                                if (value
                                    && strcmp 
                                    ((const char *) value, 
                                     cur_sel->value->stryng->str)) {
                                        xmlFree (value);
                                        return FALSE;
                                }
                                xmlFree (value);
                        }
                        break;

                case INCLUDES:
                        {
                                xmlChar *value = NULL,
                                        *ptr1 = NULL,
                                        *ptr2 = NULL,
                                        *cur = NULL;
                                gboolean found = FALSE;

                                if (!xmlHasProp 
                                    (a_node, 
                                     (const xmlChar *) cur_sel->name->stryng->str))
                                        return FALSE;
                                value = xmlGetProp 
                                        (a_node,
                                         (const xmlChar *) cur_sel->name->stryng->str);

                                if (!value)
                                        return FALSE;

                                /*
                                 *here, make sure value is a space
                                 *separated list of "words", where one
                                 *value is exactly cur_sel->value->str
                                 */
                                for (cur = value; *cur; cur++) {
                                        /*
                                         *set ptr1 to the first non white space
                                         *char addr.
                                         */
                                        while (cr_utils_is_white_space
                                               (*cur) == TRUE && *cur)
                                                cur++;
                                        if (!*cur)
                                                break;
                                        ptr1 = cur;

                                        /*
                                         *set ptr2 to the end the word.
                                         */
                                        while (cr_utils_is_white_space
                                               (*cur) == FALSE && *cur)
                                                cur++;
                                        cur--;
                                        ptr2 = cur;

                                        if (!strncmp
                                            ((const char *) ptr1, 
                                             cur_sel->value->stryng->str,
                                             ptr2 - ptr1 + 1)) {
                                                found = TRUE;
                                                break;
                                        }
                                        ptr1 = ptr2 = NULL;
                                }

                                if (found == FALSE) {
                                        xmlFree (value);
                                        return FALSE;
                                }
                                xmlFree (value);
                        }
                        break;

                case DASHMATCH:
                        {
                                xmlChar *value = NULL,
                                        *ptr1 = NULL,
                                        *ptr2 = NULL,
                                        *cur = NULL;
                                gboolean found = FALSE;

                                if (!xmlHasProp 
                                    (a_node, 
                                     (const xmlChar *) cur_sel->name->stryng->str))
                                        return FALSE;
                                value = xmlGetProp 
                                        (a_node,
                                         (const xmlChar *) cur_sel->name->stryng->str);

                                /*
                                 *here, make sure value is an hyphen
                                 *separated list of "words", each of which
                                 *starting with "cur_sel->value->str"
                                 */
                                for (cur = value; *cur; cur++) {
                                        if (*cur == '-')
                                                cur++;
                                        ptr1 = cur;

                                        while (*cur != '-' && *cur)
                                                cur++;
                                        cur--;
                                        ptr2 = cur;

                                        if (g_strstr_len
                                            ((const gchar *) ptr1, ptr2 - ptr1 + 1,
                                             cur_sel->value->stryng->str)
                                            == (gchar *) ptr1) {
                                                found = TRUE;
                                                break;
                                        }
                                }

                                if (found == FALSE) {
                                        xmlFree (value);
                                        return FALSE;
                                }
                                xmlFree (value);
                        }
                        break;
                default:
                        return FALSE;
                }
        }

        return TRUE;
}

/**
 *Evaluates if a given additional selector matches an xml node.
 *@param a_add_sel the additional selector to consider.
 *@param a_node the xml node to consider.
 *@return TRUE is a_add_sel matches a_node, FALSE otherwise.
 */
static gboolean
additional_selector_matches_node (CRSelEng * a_this,
                                  CRAdditionalSel * a_add_sel,
                                  xmlNode * a_node)
{
        CRAdditionalSel *cur_add_sel = NULL, *tail = NULL ;
        gboolean evaluated = FALSE ;

        for (tail = a_add_sel ; 
             tail && tail->next; 
             tail = tail->next) ;

        g_return_val_if_fail (tail, FALSE) ;

        for (cur_add_sel = tail ;
             cur_add_sel ;
             cur_add_sel = cur_add_sel->prev) {

                evaluated = TRUE ;
                if (cur_add_sel->type == NO_ADD_SELECTOR) {
                        return FALSE;
                }

                if (cur_add_sel->type == CLASS_ADD_SELECTOR
                    && cur_add_sel->content.class_name
                    && cur_add_sel->content.class_name->stryng
                    && cur_add_sel->content.class_name->stryng->str) {
                        if (class_add_sel_matches_node (cur_add_sel,
                                                        a_node) == FALSE) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == ID_ADD_SELECTOR
                           && cur_add_sel->content.id_name
                           && cur_add_sel->content.id_name->stryng
                           && cur_add_sel->content.id_name->stryng->str) {
                        if (id_add_sel_matches_node (cur_add_sel, a_node) == FALSE) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == ATTRIBUTE_ADD_SELECTOR
                           && cur_add_sel->content.attr_sel) {
                        /*
                         *here, call a function that does the match
                         *against an attribute additionnal selector
                         *and an xml node.
                         */
                        if (attr_add_sel_matches_node (cur_add_sel, a_node)
                            == FALSE) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == PSEUDO_CLASS_ADD_SELECTOR
                           && cur_add_sel->content.pseudo) {
                        if (pseudo_class_add_sel_matches_node
                            (a_this, cur_add_sel, a_node) == TRUE) {
                                return TRUE;
                        }
                        return FALSE;
                }
        }
        if (evaluated == TRUE)
                return TRUE;
        return FALSE ;
}

static xmlNode *
get_next_element_node (xmlNode * a_node)
{
        xmlNode *cur_node = NULL;

        g_return_val_if_fail (a_node, NULL);

        cur_node = a_node->next;
        while (cur_node && cur_node->type != XML_ELEMENT_NODE) {
                cur_node = cur_node->next;
        }
        return cur_node;
}

static xmlNode *
get_next_child_element_node (xmlNode * a_node)
{
        xmlNode *cur_node = NULL;

        g_return_val_if_fail (a_node, NULL);

        cur_node = a_node->children;
        if (!cur_node)
                return cur_node;
        if (a_node->children->type == XML_ELEMENT_NODE)
                return a_node->children;
        return get_next_element_node (a_node->children);
}

static xmlNode *
get_prev_element_node (xmlNode * a_node)
{
        xmlNode *cur_node = NULL;

        g_return_val_if_fail (a_node, NULL);

        cur_node = a_node->prev;
        while (cur_node && cur_node->type != XML_ELEMENT_NODE) {
                cur_node = cur_node->prev;
        }
        return cur_node;
}

static xmlNode *
get_next_parent_element_node (xmlNode * a_node)
{
        xmlNode *cur_node = NULL;

        g_return_val_if_fail (a_node, NULL);

        cur_node = a_node->parent;
        while (cur_node && cur_node->type != XML_ELEMENT_NODE) {
                cur_node = cur_node->parent;
        }
        return cur_node;
}

/**
 *Evaluate a selector (a simple selectors list) and says
 *if it matches the xml node given in parameter.
 *The algorithm used here is the following:
 *Walk the combinator separated list of simple selectors backward, starting
 *from the end of the list. For each simple selector, looks if
 *if matches the current node.
 *
 *@param a_this the selection engine.
 *@param a_sel the simple selection list.
 *@param a_node the xml node.
 *@param a_result out parameter. Set to true if the
 *selector matches the xml node, FALSE otherwise.
 *@param a_recurse if set to TRUE, the function will walk to
 *the next simple selector (after the evaluation of the current one) 
 *and recursively evaluate it. Must be usually set to TRUE unless you
 *know what you are doing.
 */
static enum CRStatus
sel_matches_node_real (CRSelEng * a_this, CRSimpleSel * a_sel,
                       xmlNode * a_node, gboolean * a_result,
                       gboolean a_eval_sel_list_from_end,
                       gboolean a_recurse)
{
        CRSimpleSel *cur_sel = NULL;
        xmlNode *cur_node = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_this && a_node
                              && a_result, CR_BAD_PARAM_ERROR);

        *a_result = FALSE;

        if (a_node->type != XML_ELEMENT_NODE)
                return CR_OK;

        if (a_eval_sel_list_from_end == TRUE) {
                /*go and get the last simple selector of the list */
                for (cur_sel = a_sel;
                     cur_sel && cur_sel->next; cur_sel = cur_sel->next) ;
        } else {
                cur_sel = a_sel;
        }

        for (cur_node = a_node; cur_sel; cur_sel = cur_sel->prev) {
                if (((cur_sel->type_mask & TYPE_SELECTOR)
                     && (cur_sel->name 
                         && cur_sel->name->stryng
                         && cur_sel->name->stryng->str)
                     && (!strcmp (cur_sel->name->stryng->str,
                                  (const char *) cur_node->name)))
                    || (cur_sel->type_mask & UNIVERSAL_SELECTOR)) {
                        /*
                         *this simple selector
                         *matches the current xml node
                         *Let's see if the preceding
                         *simple selectors also match
                         *their xml node counterpart.
                         */
                        if (cur_sel->add_sel) {
                                if (additional_selector_matches_node (a_this, cur_sel->add_sel, 
                                                                      cur_node) == TRUE) {
                                        goto walk_a_step_in_expr;
                                } else {
                                        goto done;
                                }
                        } else {
                                goto walk_a_step_in_expr;
                        }                                
                } 
                if (!(cur_sel->type_mask & TYPE_SELECTOR)
                    && !(cur_sel->type_mask & UNIVERSAL_SELECTOR)) {
                        if (!cur_sel->add_sel) {
                                goto done;
                        }
                        if (additional_selector_matches_node
                            (a_this, cur_sel->add_sel, cur_node)
                            == TRUE) {
                                goto walk_a_step_in_expr;
                        } else {
                                goto done;
                        }
                } else {
                        goto done ;
                }

        walk_a_step_in_expr:
                if (a_recurse == FALSE) {
                        *a_result = TRUE;
                        goto done;
                }

                /*
                 *here, depending on the combinator of cur_sel
                 *choose the axis of the xml tree traversal
                 *and walk one step in the xml tree.
                 */
                if (!cur_sel->prev)
                        break;

                switch (cur_sel->combinator) {
                case NO_COMBINATOR:
                        break;

                case COMB_WS:  /*descendant selector */
                {
                        xmlNode *n = NULL;
                        enum CRStatus status = CR_OK;
                        gboolean matches = FALSE;

                        /*
                         *walk the xml tree upward looking for a parent
                         *node that matches the preceding selector.
                         */
                        for (n = cur_node->parent; n; n = n->parent) {
                                status = sel_matches_node_real
                                        (a_this, cur_sel->prev,
                                         n, &matches, FALSE, TRUE);

                                if (status != CR_OK)
                                        goto done;

                                if (matches == TRUE) {
                                        cur_node = n ;
                                        break;
                                }
                        }

                        if (!n) {
                                /*
                                 *didn't find any ancestor that matches
                                 *the previous simple selector.
                                 */
                                goto done;
                        }
                        /*
                         *in this case, the preceding simple sel
                         *will have been interpreted twice, which
                         *is a cpu and mem waste ... I need to find
                         *another way to do this. Anyway, this is
                         *my first attempt to write this function and
                         *I am a bit clueless.
                         */
                        break;
                }

                case COMB_PLUS:
                        cur_node = get_prev_element_node (cur_node);
                        if (!cur_node)
                                goto done;
                        break;

                case COMB_GT:
                        cur_node = get_next_parent_element_node (cur_node);
                        if (!cur_node)
                                goto done;
                        break;

                default:
                        goto done;
                }
                continue;
        }

        /*
         *if we reached this point, it means the selector matches
         *the xml node.
         */
        *a_result = TRUE;

 done:
        return CR_OK;
}


/**
 *Returns  array of the ruleset statements that matches the
 *given xml node.
 *The engine keeps in memory the last statement he
 *visited during the match. So, the next call
 *to this function will eventually return a rulesets list starting
 *from the last ruleset statement visited during the previous call.
 *The enable users to get matching rulesets in an incremental way.
 *Note that for each statement returned, 
 *the engine calculates the specificity of the selector
 *that matched the xml node and stores it in the "specifity" field
 *of the statement structure.
 *
 *@param a_sel_eng the current selection engine
 *@param a_node the xml node for which the request
 *is being made.
 *@param a_sel_list the list of selectors to perform the search in.
 *@param a_rulesets in/out parameter. A pointer to the
 *returned array of rulesets statements that match the xml node
 *given in parameter. The caller allocates the array before calling this
 *function.
 *@param a_len in/out parameter the length (in sizeof (#CRStatement*)) 
 *of the returned array.
 *(the length of a_rulesets, more precisely).
 *The caller must set it to the length of a_ruleset prior to calling this
 *function. In return, the function sets it to the length 
 *(in sizeof (#CRStatement)) of the actually returned CRStatement array.
 *@return CR_OUTPUT_TOO_SHORT_ERROR if found more rulesets than the size
 *of the a_rulesets array. In this case, the first *a_len rulesets found
 *are put in a_rulesets, and a further call will return the following
 *ruleset(s) following the same principle.
 *@return CR_OK if all the rulesets found have been returned. In this
 *case, *a_len is set to the actual number of ruleset found.
 *@return CR_BAD_PARAM_ERROR in case any of the given parameter are
 *bad (e.g null pointer).
 *@return CR_ERROR if any other error occurred.
 */
static enum CRStatus
cr_sel_eng_get_matched_rulesets_real (CRSelEng * a_this,
                                      CRStyleSheet * a_stylesheet,
                                      xmlNode * a_node,
                                      CRStatement ** a_rulesets,
                                      gulong * a_len)
{
        CRStatement *cur_stmt = NULL;
        CRSelector *sel_list = NULL,
                *cur_sel = NULL;
        gboolean matches = FALSE;
        enum CRStatus status = CR_OK;
        gulong i = 0;

        g_return_val_if_fail (a_this
                              && a_stylesheet
                              && a_node && a_rulesets, CR_BAD_PARAM_ERROR);

        if (!a_stylesheet->statements) {
                *a_rulesets = NULL;
                *a_len = 0;
                return CR_OK;
        }

        /*
         *if this stylesheet is "new one"
         *let's remember it for subsequent calls.
         */
        if (PRIVATE (a_this)->sheet != a_stylesheet) {
                PRIVATE (a_this)->sheet = a_stylesheet;
                PRIVATE (a_this)->cur_stmt = a_stylesheet->statements;
        }

        /*
         *walk through the list of statements and,
         *get the selectors list inside the statements that
         *contain some, and try to match our xml node in these
         *selectors lists.
         */
        for (cur_stmt = PRIVATE (a_this)->cur_stmt, i = 0;
             (PRIVATE (a_this)->cur_stmt = cur_stmt);
             cur_stmt = cur_stmt->next) {
                /*
                 *initialyze the selector list in which we will
                 *really perform the search.
                 */
                sel_list = NULL;

                /*
                 *get the the damn selector list in 
                 *which we have to look
                 */
                switch (cur_stmt->type) {
                case RULESET_STMT:
                        if (cur_stmt->kind.ruleset
                            && cur_stmt->kind.ruleset->sel_list) {
                                sel_list = cur_stmt->kind.ruleset->sel_list;
                        }
                        break;

                case AT_MEDIA_RULE_STMT:
                        if (cur_stmt->kind.media_rule
                            && cur_stmt->kind.media_rule->rulesets
                            && cur_stmt->kind.media_rule->rulesets->
                            kind.ruleset
                            && cur_stmt->kind.media_rule->rulesets->
                            kind.ruleset->sel_list) {
                                sel_list =
                                        cur_stmt->kind.media_rule->
                                        rulesets->kind.ruleset->sel_list;
                        }
                        break;

                case AT_IMPORT_RULE_STMT:
                        /*
                         *some recursivity may be needed here.
                         *I don't like this :(
                         */
                        break;
                default:
                        break;
                }

                if (!sel_list)
                        continue;

                /*
                 *now, we have a comma separated selector list to look in.
                 *let's walk it and try to match the xml_node
                 *on each item of the list.
                 */
                for (cur_sel = sel_list; cur_sel; cur_sel = cur_sel->next) {
                        if (!cur_sel->simple_sel)
                                continue;

                        status = cr_sel_eng_matches_node
                                (a_this, cur_sel->simple_sel,
                                 a_node, &matches);

                        if (status == CR_OK && matches == TRUE) {
                                /*
                                 *bingo!!! we found one ruleset that
                                 *matches that fucking node.
                                 *lets put it in the out array.
                                 */

                                if (i < *a_len) {
                                        a_rulesets[i] = cur_stmt;
                                        i++;

                                        /*
                                         *For the cascade computing algorithm
                                         *(which is gonna take place later)
                                         *we must compute the specificity
                                         *(css2 spec chap 6.4.1) of the selector
                                         *that matched the current xml node
                                         *and store it in the css2 statement
                                         *(statement == ruleset here).
                                         */
                                        status = cr_simple_sel_compute_specificity (cur_sel->simple_sel);

                                        g_return_val_if_fail (status == CR_OK,
                                                              CR_ERROR);
                                        cur_stmt->specificity =
                                                cur_sel->simple_sel->
                                                specificity;
                                } else
                                {
                                        *a_len = i;
                                        return CR_OUTPUT_TOO_SHORT_ERROR;
                                }
                        }
                }
        }

        /*
         *if we reached this point, it means
         *we reached the end of stylesheet.
         *no need to store any info about the stylesheet
         *anymore.
         */
        g_return_val_if_fail (!PRIVATE (a_this)->cur_stmt, CR_ERROR);
        PRIVATE (a_this)->sheet = NULL;
        *a_len = i;
        return CR_OK;
}

static enum CRStatus
put_css_properties_in_props_list (CRPropList ** a_props, CRStatement * a_stmt)
{
        CRPropList *props = NULL,
                *pair = NULL,
                *tmp_props = NULL;
        CRDeclaration *cur_decl = NULL;

        g_return_val_if_fail (a_props && a_stmt
                              && a_stmt->type == RULESET_STMT
                              && a_stmt->kind.ruleset, CR_BAD_PARAM_ERROR);

        props = *a_props;

        for (cur_decl = a_stmt->kind.ruleset->decl_list;
             cur_decl; cur_decl = cur_decl->next) {
                CRDeclaration *decl;

                decl = NULL;
                pair = NULL;

                if (!cur_decl->property 
                    || !cur_decl->property->stryng
                    || !cur_decl->property->stryng->str)
                        continue;
                /*
                 *First, test if the property is not
                 *already present in our properties list
                 *If yes, apply the cascading rules to
                 *compute the precedence. If not, insert
                 *the property into the list
                 */
                cr_prop_list_lookup_prop (props,
                                          cur_decl->property, 
                                          &pair);

                if (!pair) {
                        tmp_props = cr_prop_list_append2
                                (props, cur_decl->property, cur_decl);
                        if (tmp_props) {
                                props = tmp_props;
                                tmp_props = NULL;
                        }
                        continue;
                }

                /*
                 *A property with the same name already exists.
                 *We must apply here 
                 *some cascading rules
                 *to compute the precedence.
                 */
                cr_prop_list_get_decl (pair, &decl);
                g_return_val_if_fail (decl, CR_ERROR);

                /*
                 *first, look at the origin.
                 *6.4.1 says: 
                 *"for normal declarations, 
                 *author style sheets override user 
                 *style sheets which override 
                 *the default style sheet."
                 */
                if (decl->parent_statement
                    && decl->parent_statement->parent_sheet
                    && (decl->parent_statement->parent_sheet->origin
                        < a_stmt->parent_sheet->origin)) {
                        /*
                         *if the already selected declaration
                         *is marked as being !important the current
                         *declaration must not overide it 
                         *(unless the already selected declaration 
                         *has an UA origin)
                         */
                        if (decl->important == TRUE
                            && decl->parent_statement->parent_sheet->origin
                            != ORIGIN_UA) {
                                continue;
                        }
                        tmp_props = cr_prop_list_unlink (props, pair);
                        if (props) {
                                cr_prop_list_destroy (pair);
                        }
                        props = tmp_props;
                        tmp_props = NULL;
                        props = cr_prop_list_append2
                                (props, cur_decl->property, cur_decl);

                        continue;
                } else if (decl->parent_statement
                           && decl->parent_statement->parent_sheet
                           && (decl->parent_statement->
                               parent_sheet->origin
                               > a_stmt->parent_sheet->origin)) {
                        cr_utils_trace_info
                                ("We should not reach this line\n");
                        continue;
                }

                /*
                 *A property with the same
                 *name and the same origin already exists.
                 *shit. This is lasting longer than expected ...
                 *Luckily, the spec says in 6.4.1:
                 *"more specific selectors will override 
                 *more general ones"
                 *and
                 *"if two rules have the same weight, 
                 *origin and specificity, 
                 *the later specified wins"
                 */
                if (a_stmt->specificity
                    >= decl->parent_statement->specificity) {
                        if (decl->important == TRUE)
                                continue;
                        props = cr_prop_list_unlink (props, pair);
                        if (pair) {
                                cr_prop_list_destroy (pair);
                                pair = NULL;
                        }
                        props = cr_prop_list_append2 (props,
                                                      cur_decl->property,
                                                      cur_decl);
                }
        }
        /*TODO: this may leak. Check this out */
        *a_props = props;

        return CR_OK;
}

static void
set_style_from_props (CRStyle * a_style, CRPropList * a_props)
{
        CRPropList *cur = NULL;
        CRDeclaration *decl = NULL;

        for (cur = a_props; cur; cur = cr_prop_list_get_next (cur)) {
                cr_prop_list_get_decl (cur, &decl);
                cr_style_set_style_from_decl (a_style, decl);
                decl = NULL;
        }
}

/****************************************
 *PUBLIC METHODS
 ****************************************/

/**
 * cr_sel_eng_new:
 *Creates a new instance of #CRSelEng.
 *
 *Returns the newly built instance of #CRSelEng of
 *NULL if an error occurs.
 */
CRSelEng *
cr_sel_eng_new (void)
{
        CRSelEng *result = NULL;

        result = g_try_malloc (sizeof (CRSelEng));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRSelEng));

        PRIVATE (result) = g_try_malloc (sizeof (CRSelEngPriv));
        if (!PRIVATE (result)) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (PRIVATE (result), 0, sizeof (CRSelEngPriv));
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "first-child",
                 IDENT_PSEUDO, (CRPseudoClassSelectorHandler)
                 first_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "lang",
                 FUNCTION_PSEUDO, (CRPseudoClassSelectorHandler)
                 lang_pseudo_class_handler);

        return result;
}

/**
 * cr_sel_eng_register_pseudo_class_sel_handler:
 *@a_this: the current instance of #CRSelEng
 *@a_pseudo_class_sel_name: the name of the pseudo class selector.
 *@a_pseudo_class_type: the type of the pseudo class selector.
 *@a_handler: the actual handler or callback to be called during
 *the selector evaluation process.
 *
 *Adds a new handler entry in the handlers entry table.
 *
 *Returns CR_OK, upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_register_pseudo_class_sel_handler (CRSelEng * a_this,
                                              guchar * a_name,
                                              enum CRPseudoType a_type,
                                              CRPseudoClassSelectorHandler
                                              a_handler)
{
        struct CRPseudoClassSelHandlerEntry *handler_entry = NULL;
        GList *list = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_handler && a_name, CR_BAD_PARAM_ERROR);

        handler_entry = g_try_malloc
                (sizeof (struct CRPseudoClassSelHandlerEntry));
        if (!handler_entry) {
                return CR_OUT_OF_MEMORY_ERROR;
        }
        memset (handler_entry, 0,
                sizeof (struct CRPseudoClassSelHandlerEntry));
        handler_entry->name = (guchar *) g_strdup ((const gchar *) a_name);
        handler_entry->type = a_type;
        handler_entry->handler = a_handler;
        list = g_list_append (PRIVATE (a_this)->pcs_handlers, handler_entry);
        if (!list) {
                return CR_OUT_OF_MEMORY_ERROR;
        }
        PRIVATE (a_this)->pcs_handlers = list;
        return CR_OK;
}

enum CRStatus
cr_sel_eng_unregister_pseudo_class_sel_handler (CRSelEng * a_this,
                                                guchar * a_name,
                                                enum CRPseudoType a_type)
{
        GList *elem = NULL,
                *deleted_elem = NULL;
        gboolean found = FALSE;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = elem->data;
                if (!strcmp ((const char *) entry->name, (const char *) a_name)
                    && entry->type == a_type) {
                        found = TRUE;
                        break;
                }
        }
        if (found == FALSE)
                return CR_PSEUDO_CLASS_SEL_HANDLER_NOT_FOUND_ERROR;
        PRIVATE (a_this)->pcs_handlers = g_list_delete_link
                (PRIVATE (a_this)->pcs_handlers, elem);
        entry = elem->data;
        if (entry->name)
                g_free (entry->name);
        g_free (elem);
        g_list_free (deleted_elem);

        return CR_OK;
}

/**
 * cr_sel_eng_unregister_all_pseudo_class_sel_handlers:
 *@a_this: the current instance of #CRSelEng .
 *
 *Unregisters all the pseudo class sel handlers
 *and frees all the associated allocated datastructures.
 *
 *Returns CR_OK upon succesful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_sel_eng_unregister_all_pseudo_class_sel_handlers (CRSelEng * a_this)
{
        GList *elem = NULL;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        if (!PRIVATE (a_this)->pcs_handlers)
                return CR_OK;
        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = elem->data;
                if (!entry)
                        continue;
                if (entry->name) {
                        g_free (entry->name);
                        entry->name = NULL;
                }
                g_free (entry);
                elem->data = NULL;
        }
        g_list_free (PRIVATE (a_this)->pcs_handlers);
        PRIVATE (a_this)->pcs_handlers = NULL;
        return CR_OK;
}

enum CRStatus
cr_sel_eng_get_pseudo_class_selector_handler (CRSelEng * a_this,
                                              guchar * a_name,
                                              enum CRPseudoType a_type,
                                              CRPseudoClassSelectorHandler *
                                              a_handler)
{
        GList *elem = NULL;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;
        gboolean found = FALSE;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_name, CR_BAD_PARAM_ERROR);

        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = elem->data;
                if (!strcmp ((const char *) a_name, (const char *) entry->name)
                    && entry->type == a_type) {
                        found = TRUE;
                        break;
                }
        }

        if (found == FALSE)
                return CR_PSEUDO_CLASS_SEL_HANDLER_NOT_FOUND_ERROR;
        *a_handler = entry->handler;
        return CR_OK;
}

/**
 * cr_sel_eng_matches_node:
 *@a_this: the selection engine.
 *@a_sel: the simple selector against which the xml node 
 *is going to be matched.
 *@a_node: the node against which the selector is going to be matched.
 *@a_result: out parameter. The result of the match. Is set to
 *TRUE if the selector matches the node, FALSE otherwise. This value
 *is considered if and only if this functions returns CR_OK.
 *
 *Evaluates a chained list of simple selectors (known as a css2 selector).
 *Says wheter if this selector matches the xml node given in parameter or
 *not.
 *
 *Returns the CR_OK if the selection ran correctly, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_matches_node (CRSelEng * a_this, CRSimpleSel * a_sel,
                         xmlNode * a_node, gboolean * a_result)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_this && a_node
                              && a_result, CR_BAD_PARAM_ERROR);

        if (a_node->type != XML_ELEMENT_NODE) {
                *a_result = FALSE;
                return CR_OK;
        }

        return sel_matches_node_real (a_this, a_sel, 
                                      a_node, a_result, 
                                      TRUE, TRUE);
}

/**
 * cr_sel_eng_get_matched_rulesets:
 *@a_this: the current instance of the selection engine.
 *@a_sheet: the stylesheet that holds the selectors.
 *@a_node: the xml node to consider during the walk thru
 *the stylesheet.
 *@a_rulesets: out parameter. A pointer to an array of
 *rulesets statement pointers. *a_rulesets is allocated by
 *this function and must be freed by the caller. However, the caller
 *must not alter the rulesets statements pointer because they
 *point to statements that are still in the css stylesheet.
 *@a_len: the length of *a_ruleset.
 *
 *Returns an array of pointers to selectors that matches
 *the xml node given in parameter.
 *
 *Returns CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_get_matched_rulesets (CRSelEng * a_this,
                                 CRStyleSheet * a_sheet,
                                 xmlNode * a_node,
                                 CRStatement *** a_rulesets, gulong * a_len)
{
        CRStatement **stmts_tab = NULL;
        enum CRStatus status = CR_OK;
        gulong tab_size = 0,
                tab_len = 0,
                index = 0;
        gushort stmts_chunck_size = 8;

        g_return_val_if_fail (a_this
                              && a_sheet
                              && a_node
                              && a_rulesets && *a_rulesets == NULL
                              && a_len, CR_BAD_PARAM_ERROR);

        stmts_tab = g_try_malloc (stmts_chunck_size * sizeof (CRStatement *));

        if (!stmts_tab) {
                cr_utils_trace_info ("Out of memory");
                status = CR_ERROR;
                goto error;
        }
        memset (stmts_tab, 0, stmts_chunck_size * sizeof (CRStatement *));

        tab_size = stmts_chunck_size;
        tab_len = tab_size;

        while ((status = cr_sel_eng_get_matched_rulesets_real
                (a_this, a_sheet, a_node, stmts_tab + index, &tab_len))
               == CR_OUTPUT_TOO_SHORT_ERROR) {
                stmts_tab = g_try_realloc (stmts_tab,
                                           (tab_size + stmts_chunck_size)
                                           * sizeof (CRStatement *));
                if (!stmts_tab) {
                        cr_utils_trace_info ("Out of memory");
                        status = CR_ERROR;
                        goto error;
                }
                tab_size += stmts_chunck_size;
                index += tab_len;
                tab_len = tab_size - index;
        }

        tab_len = tab_size - stmts_chunck_size + tab_len;
        *a_rulesets = stmts_tab;
        *a_len = tab_len;

        return CR_OK;

      error:

        if (stmts_tab) {
                g_free (stmts_tab);
                stmts_tab = NULL;

        }

        *a_len = 0;
        return status;
}


enum CRStatus
cr_sel_eng_get_matched_properties_from_cascade (CRSelEng * a_this,
                                                CRCascade * a_cascade,
                                                xmlNode * a_node,
                                                CRPropList ** a_props)
{
        CRStatement **stmts_tab = NULL;
        enum CRStatus status = CR_OK;
        gulong tab_size = 0,
                tab_len = 0,
                i = 0,
                index = 0;
        enum CRStyleOrigin origin = 0;
        gushort stmts_chunck_size = 8;
        CRStyleSheet *sheet = NULL;

        g_return_val_if_fail (a_this
                              && a_cascade
                              && a_node && a_props, CR_BAD_PARAM_ERROR);

        for (origin = ORIGIN_UA; origin < NB_ORIGINS; origin++) {
                sheet = cr_cascade_get_sheet (a_cascade, origin);
                if (!sheet)
                        continue;
                if (tab_size - index < 1) {
                        stmts_tab = g_try_realloc
                                (stmts_tab, (tab_size + stmts_chunck_size)
                                 * sizeof (CRStatement *));
                        if (!stmts_tab) {
                                cr_utils_trace_info ("Out of memory");
                                status = CR_ERROR;
                                goto cleanup;
                        }
                        tab_size += stmts_chunck_size;
                        /*
                         *compute the max size left for
                         *cr_sel_eng_get_matched_rulesets_real()'s output tab 
                         */
                        tab_len = tab_size - index;
                }
                while ((status = cr_sel_eng_get_matched_rulesets_real
                        (a_this, sheet, a_node, stmts_tab + index, &tab_len))
                       == CR_OUTPUT_TOO_SHORT_ERROR) {
                        stmts_tab = g_try_realloc
                                (stmts_tab, (tab_size + stmts_chunck_size)
                                 * sizeof (CRStatement *));
                        if (!stmts_tab) {
                                cr_utils_trace_info ("Out of memory");
                                status = CR_ERROR;
                                goto cleanup;
                        }
                        tab_size += stmts_chunck_size;
                        index += tab_len;
                        /*
                         *compute the max size left for
                         *cr_sel_eng_get_matched_rulesets_real()'s output tab 
                         */
                        tab_len = tab_size - index;
                }
                if (status != CR_OK) {
                        cr_utils_trace_info ("Error while running "
                                             "selector engine");
                        goto cleanup;
                }
                index += tab_len;
                tab_len = tab_size - index;
        }

        /*
         *TODO, walk down the stmts_tab and build the
         *property_name/declaration hashtable.
         *Make sure one can walk from the declaration to
         *the stylesheet.
         */
        for (i = 0; i < index; i++) {
                CRStatement *stmt = stmts_tab[i];

                if (!stmt)
                        continue;
                switch (stmt->type) {
                case RULESET_STMT:
                        if (!stmt->parent_sheet)
                                continue;
                        status = put_css_properties_in_props_list
                                (a_props, stmt);
                        break;
                default:
                        break;
                }

        }
        status = CR_OK ;
 cleanup:
        if (stmts_tab) {
                g_free (stmts_tab);
                stmts_tab = NULL;
        }

        return status;
}

enum CRStatus
cr_sel_eng_get_matched_style (CRSelEng * a_this,
                              CRCascade * a_cascade,
                              xmlNode * a_node,
                              CRStyle * a_parent_style, 
                              CRStyle ** a_style,
                              gboolean a_set_props_to_initial_values)
{
        enum CRStatus status = CR_OK;

        CRPropList *props = NULL;

        g_return_val_if_fail (a_this && a_cascade
                              && a_node && a_style, CR_BAD_PARAM_ERROR);

        status = cr_sel_eng_get_matched_properties_from_cascade
                (a_this, a_cascade, a_node, &props);

        g_return_val_if_fail (status == CR_OK, status);
        if (props) {
                if (!*a_style) {
                        *a_style = cr_style_new (a_set_props_to_initial_values) ;
                        g_return_val_if_fail (*a_style, CR_ERROR);
                } else {
                        if (a_set_props_to_initial_values == TRUE) {
                                cr_style_set_props_to_initial_values (*a_style) ;
                        } else {
                                cr_style_set_props_to_default_values (*a_style);
                        }
                }
                (*a_style)->parent_style = a_parent_style;

                set_style_from_props (*a_style, props);
                if (props) {
                        cr_prop_list_destroy (props);
                        props = NULL;
                }
        }
        return CR_OK;
}

/**
 * cr_sel_eng_destroy:
 *@a_this: the current instance of the selection engine.
 *
 *The destructor of #CRSelEng
 */
void
cr_sel_eng_destroy (CRSelEng * a_this)
{
        g_return_if_fail (a_this);

        if (!PRIVATE (a_this))
                goto end ;
        if (PRIVATE (a_this)->pcs_handlers) {
                cr_sel_eng_unregister_all_pseudo_class_sel_handlers
                        (a_this) ;
                PRIVATE (a_this)->pcs_handlers = NULL ;
        }
        g_free (PRIVATE (a_this));
        PRIVATE (a_this) = NULL;
 end:
        if (a_this) {
                g_free (a_this);
        }
}
