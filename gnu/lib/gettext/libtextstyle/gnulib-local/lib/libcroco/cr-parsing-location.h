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

#ifndef __CR_PARSING_LOCATION_H__
#define __CR_PARSING_LOCATION_H__

#include "cr-utils.h"

G_BEGIN_DECLS

/**
 *@file
 *The declaration of the CRParsingLocation
 *object. This object keeps track of line/column/byte offset/
 *at which the parsing of a given CSS construction appears.
 */

typedef struct _CRParsingLocation CRParsingLocation;
struct _CRParsingLocation {
	guint line ;
	guint column ;
	guint byte_offset ;
} ;


enum CRParsingLocationSerialisationMask {
	DUMP_LINE = 1,
	DUMP_COLUMN = 1 << 1,
	DUMP_BYTE_OFFSET = 1 << 2
} ;

CRParsingLocation * cr_parsing_location_new (void) ;

enum CRStatus cr_parsing_location_init (CRParsingLocation *a_this) ;

enum CRStatus cr_parsing_location_copy (CRParsingLocation *a_to,
					CRParsingLocation const *a_from) ;

gchar * cr_parsing_location_to_string (CRParsingLocation const *a_this,
				       enum CRParsingLocationSerialisationMask a_mask) ;
void cr_parsing_location_dump (CRParsingLocation const *a_this,
			       enum CRParsingLocationSerialisationMask a_mask,
			       FILE *a_fp) ;

void cr_parsing_location_destroy (CRParsingLocation *a_this) ;



G_END_DECLS
#endif
