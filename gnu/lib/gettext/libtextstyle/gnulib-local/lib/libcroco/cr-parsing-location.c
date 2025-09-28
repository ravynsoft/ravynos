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
#include "cr-parsing-location.h"

/**
 *@CRParsingLocation:
 *
 *Definition of the #CRparsingLocation class.
 */


/**
 * cr_parsing_location_new:
 *Instanciates a new parsing location.
 *
 *Returns the newly instanciated #CRParsingLocation.
 *Must be freed by cr_parsing_location_destroy()
 */
CRParsingLocation * 
cr_parsing_location_new (void)
{
	CRParsingLocation * result = NULL ;

	result = g_try_malloc (sizeof (CRParsingLocation)) ;
	if (!result) {
		cr_utils_trace_info ("Out of memory error") ;
		return NULL ;
	}
	cr_parsing_location_init (result) ;
	return result ;
}

/**
 * cr_parsing_location_init:
 *@a_this: the current instance of #CRParsingLocation.
 *
 *Initializes the an instance of #CRparsingLocation.
 *
 *Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus 
cr_parsing_location_init (CRParsingLocation *a_this)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	memset (a_this, 0, sizeof (CRParsingLocation)) ;
	return CR_OK ;
}

/**
 * cr_parsing_location_copy:
 *@a_to: the destination of the copy. 
 *Must be allocated by the caller.
 *@a_from: the source of the copy.
 *
 *Copies an instance of CRParsingLocation into another one.
 *
 *Returns CR_OK upon succesful completion, an error code
 *otherwise.
 */
enum CRStatus 
cr_parsing_location_copy (CRParsingLocation *a_to,
			  CRParsingLocation const *a_from)
{
	g_return_val_if_fail (a_to && a_from, CR_BAD_PARAM_ERROR) ;

	memcpy (a_to, a_from, sizeof (CRParsingLocation)) ;
	return CR_OK ;
}

/**
 * cr_parsing_location_to_string:
 *@a_this: the current instance of #CRParsingLocation.
 *@a_mask: a bitmap that defines which parts of the
 *parsing location are to be serialized (line, column or byte offset)
 *
 *Returns the serialized string or NULL in case of an error.
 */
gchar * 
cr_parsing_location_to_string (CRParsingLocation const *a_this,
			       enum CRParsingLocationSerialisationMask a_mask)
{
	GString *result = NULL ;
	gchar *str = NULL ;

	g_return_val_if_fail (a_this, NULL) ;

	if (!a_mask) {
		a_mask = DUMP_LINE | DUMP_COLUMN | DUMP_BYTE_OFFSET ;
	}
	result =g_string_new (NULL) ;
	if (!result)
		return NULL ;
	if (a_mask & DUMP_LINE) {
		g_string_append_printf (result, "line:%d ", 
					a_this->line) ;
	}
	if (a_mask & DUMP_COLUMN) {
		g_string_append_printf (result, "column:%d ", 
					a_this->column) ;
	}
	if (a_mask & DUMP_BYTE_OFFSET) {
		g_string_append_printf (result, "byte offset:%d ", 
					a_this->byte_offset) ;
	}
	if (result->len) {
		str = result->str ;
		g_string_free (result, FALSE) ;
	} else {
		g_string_free (result, TRUE) ;
	}
	return str ;
}

/**
 * cr_parsing_location_dump:
 * @a_this: current instance of #CRParsingLocation
 * @a_mask: the serialization mask.
 * @a_fp: the file pointer to dump the parsing location to.
 */
void
cr_parsing_location_dump (CRParsingLocation const *a_this,
			  enum CRParsingLocationSerialisationMask a_mask,
			  FILE *a_fp)
{
	gchar *str = NULL ;

	g_return_if_fail (a_this && a_fp) ;
	str = cr_parsing_location_to_string (a_this, a_mask) ;
	if (str) {
		fprintf (a_fp, "%s", str) ;
		g_free (str) ;
		str = NULL ;
	}
}

/**
 * cr_parsing_location_destroy:
 *@a_this: the current instance of #CRParsingLocation. Must
 *have been allocated with cr_parsing_location_new().
 *
 *Destroys the current instance of #CRParsingLocation
 */
void 
cr_parsing_location_destroy (CRParsingLocation *a_this)
{
	g_return_if_fail (a_this) ;
	g_free (a_this) ;
}

