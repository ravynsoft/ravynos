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
 * Author: Dodji Seketeli.
 */

#include <config.h>
#include <string.h>
#include "cr-string.h"

/**
 *Instanciates a #CRString
 *@return the newly instanciated #CRString
 *Must be freed with cr_string_destroy().
 */
CRString *
cr_string_new (void)
{
	CRString *result = NULL ;

	result = g_try_malloc (sizeof (CRString)) ;
	if (!result) {
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRString)) ;
        result->stryng = g_string_new (NULL) ;
	return result ;
}

/**
 *Instanciate a string and initialise it to
 *a_string.
 *@param a_string the initial string
 *@return the newly instanciated string.
 */
CRString  *
cr_string_new_from_string (const gchar * a_string)
{
	CRString *result = NULL ;

	result = cr_string_new () ;
	if (!result) {
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	if (a_string)
		g_string_append (result->stryng, a_string) ;
	return result ;
}

/**
 *Instanciates a #CRString from an instance of GString.
 *@param a_string the input string that will be copied into
 *the newly instanciated #CRString
 *@return the newly instanciated #CRString.
 */
CRString *
cr_string_new_from_gstring (GString const *a_string)
{
	CRString *result = NULL ;

	result = cr_string_new () ;
	if (!result) {
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	if (a_string) {
		g_string_append_len (result->stryng,
				     a_string->str,
				     a_string->len);

	}
	return result ;
}

CRString *
cr_string_dup (CRString const *a_this)
{
	CRString *result = NULL ;
	g_return_val_if_fail (a_this, NULL) ;

	result = cr_string_new_from_gstring (a_this->stryng) ;
	if (!result) {
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	cr_parsing_location_copy (&result->location,
                                  &a_this->location) ;
        return result ;
}

gchar *
cr_string_dup2 (CRString const *a_this)
{
        gchar *result = NULL ;

        g_return_val_if_fail (a_this, NULL) ;

        if (a_this 
            && a_this->stryng 
            && a_this->stryng->str) {
                result = g_strndup (a_this->stryng->str,
                                    a_this->stryng->len) ;
        }
        return result ;
}

/**
 *Returns a pointer to the internal raw NULL terminated string
 *of the current instance of #CRString.
 *@param a_this the current instance of #CRString
 */
const gchar *
cr_string_peek_raw_str (CRString const *a_this)
{
        g_return_val_if_fail (a_this, NULL) ;
        
        if (a_this->stryng && a_this->stryng->str)
                return a_this->stryng->str ;
        return NULL ;
}

/**
 *Returns the length of the internal raw NULL terminated
 *string of the current instance of #CRString.
 *@param a_this the current instance of #CRString.
 *@return the len of the internal raw NULL termninated string,
 *of -1 if no length can be returned.
 */
gint
cr_string_peek_raw_str_len (CRString const *a_this)
{
        g_return_val_if_fail (a_this && a_this->stryng,
                              -1) ;
        return a_this->stryng->len ;
}

/**
 *@param a_this the #CRString to destroy.
 */
void
cr_string_destroy (CRString *a_this)
{
	g_return_if_fail (a_this) ;

	if (a_this->stryng) {
		g_string_free (a_this->stryng, TRUE) ;
		a_this->stryng = NULL ;
	}
	g_free (a_this) ;
}
