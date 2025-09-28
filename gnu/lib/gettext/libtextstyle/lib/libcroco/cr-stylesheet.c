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
 * Copyright (C) 2002-2004 Dodji Seketeli
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
#include "string.h"
#include "cr-stylesheet.h"

/**
 *@file
 *The definition of the #CRStyleSheet class
 */

/**
 *Constructor of the #CRStyleSheet class.
 *@param the initial list of css statements.
 *@return the newly built css2 stylesheet, or NULL in case of error.
 */
CRStyleSheet *
cr_stylesheet_new (CRStatement * a_stmts)
{
        CRStyleSheet *result;

        result = g_try_malloc (sizeof (CRStyleSheet));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRStyleSheet));

        if (a_stmts)
                result->statements = a_stmts;

        return result;
}

/**
 *@param a_this the current instance of #CRStyleSheet
 *@return the serialized stylesheet.
 */
gchar *
cr_stylesheet_to_string (CRStyleSheet const *a_this)
{
	gchar *str = NULL;
	GString *stringue = NULL;
	CRStatement const *cur_stmt = NULL;

        g_return_val_if_fail (a_this, NULL);

	if (a_this->statements) {
		stringue = g_string_new (NULL) ;
		g_return_val_if_fail (stringue, NULL) ;
	}
        for (cur_stmt = a_this->statements;
             cur_stmt; cur_stmt = cur_stmt->next) {
		if (cur_stmt->prev) {
			g_string_append (stringue, "\n\n") ;
		}
		str = cr_statement_to_string (cur_stmt, 0) ;
		if (str) {
			g_string_append (stringue, str) ;
			g_free (str) ;
			str = NULL ;
		}
        }
	if (stringue) {
		str = stringue->str ;
		g_string_free (stringue, FALSE) ;
		stringue = NULL ;
	}
	return str ;
}

/**
 *Dumps the current css2 stylesheet to a file.
 *@param a_this the current instance of #CRStyleSheet.
 *@param a_fp the destination file
 */
void
cr_stylesheet_dump (CRStyleSheet const * a_this, FILE * a_fp)
{
	gchar *str = NULL ;

        g_return_if_fail (a_this);

	str = cr_stylesheet_to_string (a_this) ;
	if (str) {
		fprintf (a_fp, "%s", str) ;
		g_free (str) ;
		str = NULL ;
	}
}

/**
 *Return the number of rules in the stylesheet.
 *@param a_this the current instance of #CRStyleSheet.
 *@return number of rules in the stylesheet.
 */
gint
cr_stylesheet_nr_rules (CRStyleSheet const * a_this)
{
        g_return_val_if_fail (a_this, -1);

        return cr_statement_nr_rules (a_this->statements);
}

/**
 *Use an index to get a CRStatement from the rules in a given stylesheet.
 *@param a_this the current instance of #CRStatement.
 *@param itemnr the index into the rules.
 *@return CRStatement at position itemnr, if itemnr > number of rules - 1,
 *it will return NULL.
 */
CRStatement *
cr_stylesheet_statement_get_from_list (CRStyleSheet * a_this, int itemnr)
{
        g_return_val_if_fail (a_this, NULL);

        return cr_statement_get_from_list (a_this->statements, itemnr);
}

void
cr_stylesheet_ref (CRStyleSheet * a_this)
{
        g_return_if_fail (a_this);

        a_this->ref_count++;
}

gboolean
cr_stylesheet_unref (CRStyleSheet * a_this)
{
        g_return_val_if_fail (a_this, FALSE);

        if (a_this->ref_count)
                a_this->ref_count--;

        if (!a_this->ref_count) {
                cr_stylesheet_destroy (a_this);
                return TRUE;
        }

        return FALSE;
}

/**
 *Destructor of the #CRStyleSheet class.
 *@param a_this the current instance of the #CRStyleSheet class.
 */
void
cr_stylesheet_destroy (CRStyleSheet * a_this)
{
        g_return_if_fail (a_this);

        if (a_this->statements) {
                cr_statement_destroy (a_this->statements);
                a_this->statements = NULL;
        }
        g_free (a_this);
}
