/* as.h - global header file
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef AS_H_
#define AS_H_

extern char *apple_flags;
#define APPLE_INC_VERSION "Apple Inc version"
/* apple_version is in apple_version.c which is created by the Makefile */
extern char apple_version[];
/* the GNU version is set in as.c */
extern char version_string[];

#define _(String) (String)
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

/*
 * CAPITALISED names are #defined.
 * "lowercaseT" is a typedef of "lowercase" objects.
 * "lowercaseP" is type "pointer to object of type 'lowercase'".
 * "lowercaseS" is typedef struct ... lowercaseS.
 *
 * #define SUSPECT when debugging.
 * If TEST is #defined, then we are testing a module.
 */

/* These #defines are for parameters of entire assembler. */

/*
 * asserts() from <assert.h> are DISabled when NDEBUG is defined and
 * asserts() from <assert.h> are ENabled  when NDEBUG is undefined.
 * For speed NDEBUG is defined so assert()'s are left out.
#undef NDEBUG
 */
#define NDEBUG

/*
 * For speed SUSPECT is undefined.
#define SUSPECT
 */
#undef SUSPECT

/* These #imports are for type definitions etc. */
#include <stdint.h>
#import <stdio.h>
#import <assert.h>
#import <mach/machine.h>

/* These defines are potentially useful */
#undef FALSE
#define FALSE	(0)
#undef TRUE
#define TRUE	(!FALSE)
#define ASSERT	assert

#define BAD_CASE(value)							\
{									\
  as_fatal ("Case value %d unexpected at line %d of file \"%s\"\n",	\
	   value, __LINE__, __FILE__);					\
}

/* These are assembler-wide concepts */

/* FROM line 206 */
#ifdef BFD_ASSEMBLER
extern bfd *stdoutput;
typedef bfd_vma addressT;
typedef bfd_signed_vma offsetT;
#else
/* These are 64-bit values so we can use 64-bit values with 32-bit targets. */
typedef uint64_t addressT;
typedef int64_t offsetT;
#endif

#ifdef SUSPECT
#define register		/* no registers: helps debugging */
#define know(p) ASSERT(p)	/* know() is less ugly than #ifdef SUSPECT/ */
				/* assert()/#endif. */
#else
#define know(p)			/* know() checks are no-op.ed */
#endif


/* FROM line 262 */
/*
 * This table describes the use of segments as EXPRESSION types.
 *
 *	X_seg	X_add_symbol  X_subtract_symbol	X_add_number
 * SEG_NONE						no (legal) expression
 * SEG_BIG					*	> 32 bits const.
 * SEG_ABSOLUTE				     	0
 * SEG_SECT		*		     	0
 * SEG_UNKNOWN		*			0
 * SEG_DIFFSECT		0		*	0
 *
 * The blank fields MUST be 0, and are nugatory.
 * The '0' fields MAY be 0. The '*' fields MAY NOT be 0.
 *
 * SEG_BIG: A floating point number or an integer larger than 32 bits.
 *   For a floating point number:
 *	X_add_number is < 0
 * 	    The result is in the global variable generic_floating_point_number.
 *	    The value in X_add_number is -'c' where c is the character that
 *	    introduced the constant.  e.g. "0f6.9" will have  -'f' as a
 *	    X_add_number value.
 *   For an integer larger than 32 bits:
 *	X_add_number > 0
 *	    The result is in the global variable generic_bignum.
 *	    The value in X_add_number is a count of how many littlenums it
 *	    took to represent the bignum.
 */
typedef enum {
    SEG_ABSOLUTE,	/* absolute */
    SEG_SECT,		/* normal defined section */
    SEG_DIFFSECT,	/* difference between symbols in sections */
    SEG_UNKNOWN,	/* expression involving an undefined symbol */
    SEG_NONE,		/* no expression */
    SEG_BIG		/* bigger than 32 bits constant */
} segT;

#define absolute_section	SEG_ABSOLUTE

/* FROM line 285 */
typedef int subsegT;

/* Type of debugging information we should generate.  We currently support
   stabs, ECOFF, and DWARF2.

   NOTE!  This means debug information about the assembly source code itself
   and _not_ about possible debug information from a high-level language.
   This is especially relevant to DWARF2, since the compiler may emit line
   number directives that the assembler resolves.  */

enum debug_info_type
{
  DEBUG_UNSPECIFIED,
  DEBUG_NONE,
  DEBUG_STABS,
  DEBUG_ECOFF,
  DEBUG_DWARF,
  DEBUG_DWARF2
};

extern enum debug_info_type debug_type;

/*
 * main program "as.c" (command arguments etc)
 */

/* ['x'] TRUE if "-x" seen. */
extern char flagseen[128];

/* name of emitted object file, argument to -o if specified */
extern char *out_file_name;

typedef struct frag fragS;

/* TRUE if -force_cpusubtype_ALL is specified */
extern int force_cpusubtype_ALL;

/* set to the corresponding cpusubtype if -arch flag is specified */
extern cpu_subtype_t archflag_cpusubtype;
extern char *specific_archflag;

/* TRUE if the .subsections_via_symbols directive was seen */
extern int subsections_via_symbols;

/* -I path options for .includes */
struct directory_stack {
    struct directory_stack *next;
    char *fname;
};
extern struct directory_stack include_defaults[];
extern struct directory_stack *include;

/* FROM 317 */
#define undefined_section	SEG_UNKNOWN

#include "expr.h"
#include "write_object.h"

/* STUFF from write.h */
/* This is the name of a fake symbol which will never appear in the
   assembler output.  S_IS_LOCAL detects it because of the \001.  */
#ifndef FAKE_LABEL_NAME
#define FAKE_LABEL_NAME "L0\001"
#endif

#ifdef __GNUC__
#define as_bad_where(MY_FILE, MY_LINE, ...)	\
  do {						\
    layout_file = MY_FILE;			\
    layout_line = MY_LINE;			\
    as_bad (__VA_ARGS__);			\
} while (0)
#endif

/* FROM 317 */
#define undefined_section	SEG_UNKNOWN

/* non-NULL if AS_SECURE_LOG_FILE is set */
extern const char *secure_log_file;

#ifndef OCTETS_PER_BYTE_POWER
#define OCTETS_PER_BYTE_POWER 0
#endif
#ifndef OCTETS_PER_BYTE
#define OCTETS_PER_BYTE (1<<OCTETS_PER_BYTE_POWER)
#endif
#if OCTETS_PER_BYTE != (1<<OCTETS_PER_BYTE_POWER)
 #error "Octets per byte conflicts with its power-of-two definition!"
#endif

#endif /* AS_H_ */
