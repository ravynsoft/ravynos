/*
 * GSUnion.h
 * File to set up a typedef for a union capable of containing various types.
 * Copyright (C) 1999  Free Software Foundation, Inc.
 * 
 * Author:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
 * Created:	Apr 1999
 * 
 * This file is part of the GNUstep Base Library.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111 USA. */

/* Need Foundation/NSObjCRuntime.h for type declarations.
 */
#import <Foundation/NSObjCRuntime.h>

/* These are not defined in older Mac OS X systems */
#ifndef NSINTEGER_DEFINED
typedef intptr_t NSInteger;
typedef uintptr_t NSUInteger;
#define NSINTEGER_DEFINED 1
#endif

/*
 *	Definitions for bitmap mask of types of element in union.
 */
#ifndef	GSUNION_OBJ

#define	GSUNION_OBJ	0x0001
#define	GSUNION_CLS	0x0002
#define	GSUNION_SEL	0x0004
#define	GSUNION_PTR	0x0080
#define	GSUNION_NSINT	0x1000

#define	GSUNION_ALL	0x1fff

#endif	/* GSUNION_OBJ */


/*
 * Produce a typedef for a union with name 'GSUNION' containing elements
 * specified in the GSUNION_TYPES mask, and optionally with an extra
 * element 'ext' of the type specified in GSUNION_EXTRA
 *
 * You can include this file more than once in order to produce different
 * typedefs as long as you redefine 'GSUNION' before each inclusion.
 */

#if	defined(GSUNION) && defined(GSUNION_TYPES)

typedef	union {
  NSUInteger    addr;	/* Always present */
#if	((GSUNION_TYPES) & GSUNION_OBJ)
  id		obj;
  NSObject	*nso;
#endif
#if	((GSUNION_TYPES) & GSUNION_CLS)
  Class		cls;
#endif
#if	((GSUNION_TYPES) & GSUNION_SEL)
  SEL		sel;
#endif
#if	((GSUNION_TYPES) & GSUNION_NSINT)
  NSInteger 	nsi;
  NSUInteger	nsu;
#endif
#if	((GSUNION_TYPES) & GSUNION_PTR)
  void		*ptr;
  const void	*cptr;
  char		*str;
  const char	*cstr;
#endif

/* Warning ... if this value is declared in the union, and its type is not
 * the same size as a pointer, then care must be taken in case of confusion
 * caused when an assignment to a variable using one of the union's types
 * causes part of the variable to ebe left with undefined content from the
 * point of view of another of the union's types.
 */
#if	defined(GSUNION_EXTRA)
  GSUNION_EXTRA	ext;
#endif
} GSUNION;

#endif

