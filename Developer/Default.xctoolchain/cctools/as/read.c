/* read.c - read a source file -
   Copyright (C) 1986,1987 Free Software Foundation, Inc.

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

#define MASK_CHAR (0xFF)	/* If your chars aren't 8 bits, you will
				   change this a bit.  But then, GNU isnt
				   spozed to run on your machine anyway.
				   (RMS is so shortsighted sometimes.)
				 */

#define MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT (16)
				/* This is the largest known floating point */
				/* format (for now). It will grow when we */
				/* do 4361 style flonums. */


/* Routines that read assembler source text to build spagetti in memory. */
/* Another group of these functions is in the expr.c module */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stuff/rnd.h"
#include "stuff/arch.h"
#include "stuff/best_arch.h"
#include "as.h"
#include "flonum.h"
#include "struc-symbol.h"
#include "expr.h"
#include "read.h"
#include "hash.h"
#include "obstack.h"
#include "md.h"
#include "symbols.h"
#include "sections.h"
#include "input-scrub.h"
#include "input-file.h"
#include "hex_value.h"
#include "messages.h"
#include "xmalloc.h"
#include "app.h"
#if defined(I386) && defined(ARCH64)
#include "i386.h"
#endif
#include "dwarf2dbg.h"

/*
 * Parsing of input is done off of this pointer which points to the next char
 * of source file to parse.
 */
char *input_line_pointer = NULL;

/*
 * buffer_limit is the value returned by the input_scrub_next_buffer() in
 * read_a_source_file() and is not static only so read_an_include_file can save
 * and restore it.
 */
char *buffer_limit = NULL;	/* -> 1 + last char in buffer. */

/* FROM line 164 */
#define TARGET_BYTES_BIG_ENDIAN 0 /* HACK */
/* TARGET_BYTES_BIG_ENDIAN is required to be defined to either 0 or 1
   in the tc-<CPU>.h file.  See the "Porting GAS" section of the
   internals manual.  */
int target_big_endian = TARGET_BYTES_BIG_ENDIAN;

/*
 * This table is used by the macros is_name_beginner() and is_part_of_name()
 * defined in read.h .
 */
#ifndef PPC
const
#endif /* PPC */
char lex_type[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       /* @ABCDEFGHIJKLMNO */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       /* PQRSTUVWXYZ[\]^_ */
  0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0,       /* _!"#$%&'()*+,-./ */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,       /* 0123456789:;<=>? */
  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,       /* @ABCDEFGHIJKLMNO */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 3,       /* PQRSTUVWXYZ[\]^_ */
  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,       /* `abcdefghijklmno */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0,       /* pqrstuvwxyz{|}~. */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* Allow all chars  */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* with the high bit */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* set in names */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 
};

/*
 * In: a character.
 * Out: TRUE if this character ends a line.
 */
static
#ifndef PPC
const
#endif /* PPC */
char is_end_of_line_tab[256] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, /* @abcdefghijklmno */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, /* 0123456789:;<=>? */
#if defined(M88K) || defined(PPC) || defined(HPPA)
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* @ABCDEFGHIJKLMNO */
#else
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
#endif
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*                  */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /*                  */
};
// binutils references the table directly, but that requires a cast
// whenever you try to index into the array with a char and that's
// annoying.  The function avoids this.  We'd make this a macro,
// but it needs to be referenced externally and we don't need to
// export the table.
char is_end_of_line(int c) { return is_end_of_line_tab[c & 0xFF]; }

/*
 * The conditional assembly feature (.if, .else, .elseif and .endif) is
 * implemented with cond_state that tells us what we are in the middle of 
 * processing.  ignore can be either TRUE or FALSE.  When TRUE we are ignoring
 * the block of code in the middle of a conditional.  MAX_IF_DEPTH is the
 * maximum depth that if's can be nested.
 */
#define MAX_IF_DEPTH 20
typedef enum {
    no_cond,	/* no conditional is being processed */
    if_cond,	/* inside if conditional */
    elseif_cond,/* inside elseif conditional */
    else_cond	/* inside else conditional */
}cond_type;

struct cond_state {
    cond_type	the_cond;
    int		cond_met;
    int		ignore;
};
typedef struct cond_state cond_stateS;
static cond_stateS the_cond_state = {no_cond, FALSE, FALSE};
static cond_stateS last_states[MAX_IF_DEPTH];
static int if_depth = 0;

/*
 * Assembler macros are implemented with these variables and functions.
 */
#define MAX_MACRO_DEPTH 20
static int macro_depth = 0;
static struct hash_control
	*ma_hash = NULL;	/* use before set up: NULL-> address error */
static struct obstack macros;	/* obstack for macro text */
static char *macro_name = NULL;	/* name of macro we are defining */
static int count_lines = TRUE;	/* turns line number counting on and off */
static int macros_on = TRUE;	/* .macros_on and .macros_off toggles this to
				   allow macros to be turned off, which allows
				   macros to override a machine instruction and
				   still use it. */
static void expand_macro(char *macro_contents);
static void macro_begin(void);


/*
 * The .dump and .load feature is implemented with these variables and
 * functions.
 */
static FILE *dump_fp = NULL;
static void write_macro(const char *string, PTR value);
static void write_symbol(const char *string, PTR value);


/* Functions private to this file */
static void parse_a_buffer(char *buffer);
#ifdef PPC
static void ppcasm_parse_a_buffer(char *buffer);
#endif
static void parse_line_comment(char **buffer);
static segT get_segmented_expression(expressionS *expP);
static void pseudo_op_begin(void);
#ifdef PPC
static void ppcasm_pseudo_op_begin(void);
#endif 
static void stab(uintptr_t what);
static char get_absolute_expression_and_terminator(int32_t *val_pointer);
static char *demand_copy_string(int *lenP);
static int is_it_end_of_statement(void);
static void equals(char *sym_name);
static int next_char_of_string(void);

#ifdef M68K /* we allow big cons only on the 68k machines */
/*
 * This is setup by read_begin() and used by big_cons() with using grow_bignum()
 * to make it bigger if needed.
 */
#define BIGNUM_BEGIN_SIZE (16)
static char *bignum_low;  /* Lowest char of bignum. */
static char *bignum_limit;/* 1st illegal address of bignum. */
static char *bignum_high; /* Highest char of bignum, may point to
			     (bignum_start-1), never >= bignum_limit. */
static void grow_bignum(void);
#endif /* M68K */
/*
 * This is set in read_a_source_file() to the section number of the text section
 * for used by the machine dependent md_assemble() to create line number stabs
 * for assembly instructions in the text section when -g is seen.
 */
uint32_t text_nsect = 0;

/*
 * These are the names of the section types used by the .section directive.
 */
struct type_name {
    char *name;
    unsigned type;
};
static struct type_name type_names[] = {
    { "regular",		  S_REGULAR },
    { "cstring_literals",	  S_CSTRING_LITERALS },
    { "4byte_literals",		  S_4BYTE_LITERALS },
    { "8byte_literals",		  S_8BYTE_LITERALS },
    { "16byte_literals",	  S_16BYTE_LITERALS },
    { "literal_pointers",	  S_LITERAL_POINTERS },
#if !(defined(I386) && defined(ARCH64))
    { "non_lazy_symbol_pointers", S_NON_LAZY_SYMBOL_POINTERS },
    { "lazy_symbol_pointers",	  S_LAZY_SYMBOL_POINTERS },
    { "symbol_stubs",		  S_SYMBOL_STUBS },
#endif
    { "mod_init_funcs",		  S_MOD_INIT_FUNC_POINTERS },
    { "mod_term_funcs",		  S_MOD_TERM_FUNC_POINTERS },
    { "coalesced",		  S_COALESCED },
    { "interposing",		  S_INTERPOSING },
    { "thread_local_regular",	  S_THREAD_LOCAL_REGULAR },
    { "thread_local_variables",	  S_THREAD_LOCAL_VARIABLES },
    { "thread_local_init_function_pointers",
				  S_THREAD_LOCAL_INIT_FUNCTION_POINTERS },
    { NULL, 0 }
};

/*
 * These are the names of the section attributes used by the .section directive.
 */
struct attribute_name {
    char *name;
    unsigned attribute;
};
static struct attribute_name attribute_names[] = {
    { "none",	  0 },
    { "pure_instructions", S_ATTR_PURE_INSTRUCTIONS },
    { "no_toc", S_ATTR_NO_TOC },
    { "strip_static_syms", S_ATTR_STRIP_STATIC_SYMS },
    { "no_dead_strip", S_ATTR_NO_DEAD_STRIP },
    { "live_support", S_ATTR_LIVE_SUPPORT },
    { "self_modifying_code", S_ATTR_SELF_MODIFYING_CODE },
    { "debug", S_ATTR_DEBUG },
    { NULL, 0 }
};

/*
 * These are the built in sections known to the assembler with a directive.
 * They are known as which segment and section name as well as the type &
 * attribute, and default alignment.
 */
struct builtin_section {
    char *directive;
    char *segname;
    char *sectname;
    uint32_t flags; /* type & attribute */
    uint32_t default_align;
    uint32_t sizeof_stub;
};
static const struct builtin_section builtin_sections[] = {
    /*
     * The text section must be first in this list as it is used by
     * read_a_source_file() to do the equivalent of a .text at the start
     * of the file.
     */
    { "text",                "__TEXT", "__text", S_ATTR_PURE_INSTRUCTIONS },
    { "const",               "__TEXT", "__const" },
    { "static_const",        "__TEXT", "__static_const" },
    { "cstring",             "__TEXT", "__cstring", S_CSTRING_LITERALS },
    { "literal4",            "__TEXT", "__literal4", S_4BYTE_LITERALS, 2 },
    { "literal8",            "__TEXT", "__literal8", S_8BYTE_LITERALS, 3 },
    { "literal16",           "__TEXT", "__literal16", S_16BYTE_LITERALS, 4 },
    { "constructor",         "__TEXT", "__constructor" },
    { "destructor",          "__TEXT", "__destructor" },
    { "fvmlib_init0",        "__TEXT", "__fvmlib_init0" },
    { "fvmlib_init1",        "__TEXT", "__fvmlib_init1" },
#if !(defined(I386) && defined(ARCH64))
    { "symbol_stub",	     "__TEXT", "__symbol_stub",
		S_SYMBOL_STUBS | S_ATTR_PURE_INSTRUCTIONS,
#if defined(M68K)
		1, 20
#endif
#if defined(I386)
		0, 16
#endif
#if defined(HPPA)
		2, 28
#endif
#if defined(SPARC)
		  2, 32
#endif
#if defined(PPC)
		  2, 20
#endif
		},
#endif
#if !(defined(I386) && defined(ARCH64))
    { "picsymbol_stub",	     "__TEXT", "__picsymbol_stub",
		S_SYMBOL_STUBS | S_ATTR_PURE_INSTRUCTIONS,
#if defined(M68K)
		1, 24
#endif
#if defined(I386)
		0, 26
#endif
#if defined(HPPA)
		2, 32
#endif
#if defined(SPARC)
		  2, 60
#endif
#if defined(PPC)
		  2, 36
#endif
		},
#endif
#if !(defined(I386) && defined(ARCH64))
    { "non_lazy_symbol_pointer","__DATA","__nl_symbol_ptr",
		S_NON_LAZY_SYMBOL_POINTERS, 2 },
    { "lazy_symbol_pointer", "__DATA", "__la_symbol_ptr",
		S_LAZY_SYMBOL_POINTERS, 2 },
#endif
    { "mod_init_func",	     "__DATA", "__mod_init_func",
		S_MOD_INIT_FUNC_POINTERS, 2 },
    { "mod_term_func",	     "__DATA", "__mod_term_func",
		S_MOD_TERM_FUNC_POINTERS, 2 },
    { "dyld",		     "__DATA", "__dyld" },
    { "data",                "__DATA", "__data" },
    { "static_data",         "__DATA", "__static_data" },
    { "const_data",          "__DATA", "__const" },
    { "tdata",		     "__DATA", "__thread_data",
		S_THREAD_LOCAL_REGULAR },
    { "tlv",		     "__DATA", "__thread_vars",
		S_THREAD_LOCAL_VARIABLES },
    { "thread_init_func",    "__DATA", "__thread_init",
		S_THREAD_LOCAL_INIT_FUNCTION_POINTERS },
    { "objc_class",          "__OBJC", "__class", S_ATTR_NO_DEAD_STRIP },
    { "objc_meta_class",     "__OBJC", "__meta_class", S_ATTR_NO_DEAD_STRIP },
    { "objc_string_object",  "__OBJC", "__string_object", S_ATTR_NO_DEAD_STRIP},
    { "objc_protocol",       "__OBJC", "__protocol", S_ATTR_NO_DEAD_STRIP },
    { "objc_cat_cls_meth",   "__OBJC", "__cat_cls_meth", S_ATTR_NO_DEAD_STRIP },
    { "objc_cat_inst_meth",  "__OBJC", "__cat_inst_meth", S_ATTR_NO_DEAD_STRIP},
    { "objc_cls_meth",       "__OBJC", "__cls_meth", S_ATTR_NO_DEAD_STRIP },
    { "objc_inst_meth",      "__OBJC", "__inst_meth", S_ATTR_NO_DEAD_STRIP },
    { "objc_message_refs",   "__OBJC", "__message_refs",
		S_LITERAL_POINTERS | S_ATTR_NO_DEAD_STRIP, 2},
    { "objc_cls_refs",       "__OBJC", "__cls_refs",
		S_LITERAL_POINTERS | S_ATTR_NO_DEAD_STRIP, 2},
    { "objc_class_names",    "__TEXT", "__cstring", S_CSTRING_LITERALS },
    { "objc_module_info",    "__OBJC", "__module_info", S_ATTR_NO_DEAD_STRIP },
    { "objc_symbols",        "__OBJC", "__symbols", S_ATTR_NO_DEAD_STRIP },
    { "objc_category",       "__OBJC", "__category", S_ATTR_NO_DEAD_STRIP },
    { "objc_meth_var_types", "__TEXT", "__cstring", S_CSTRING_LITERALS },
    { "objc_class_vars",     "__OBJC", "__class_vars", S_ATTR_NO_DEAD_STRIP },
    { "objc_instance_vars",  "__OBJC", "__instance_vars", S_ATTR_NO_DEAD_STRIP},
    { "objc_meth_var_names", "__TEXT", "__cstring", S_CSTRING_LITERALS },
    { "objc_selector_strs",  "__OBJC", "__selector_strs", S_CSTRING_LITERALS },
    { 0 }
};

/* set up pseudo-op tables */
static struct hash_control *po_hash = NULL;
#ifdef PPC
static struct hash_control *ppcasm_po_hash = NULL;
#endif

/*
 * The routines that implement the pseudo-ops.
 */
#if !defined(I860) /* i860 has it's own align and org */
static void s_align(int value, int bytes_p);
static void s_align_bytes(uintptr_t arg);
static void s_align_ptwo(uintptr_t arg);
static void s_org(uintptr_t value);
#endif
static void s_private_extern(uintptr_t value);
#if !(defined(I386) && defined(ARCH64))
static void s_indirect_symbol(uintptr_t value);
#endif
static void s_abort(uintptr_t value);
static void s_comm(uintptr_t value);
static void s_desc(uintptr_t value);
static void s_fill(uintptr_t value);
static void s_lcomm(uintptr_t value);
static void s_lsym(uintptr_t value);
static void s_set(uintptr_t value);
static void s_reference(uintptr_t value);
static void s_lazy_reference(uintptr_t value);
static void s_weak_reference(uintptr_t value);
static void s_weak_definition(uintptr_t value);
static void s_weak_def_can_be_hidden(uintptr_t value);
static void s_no_dead_strip(uintptr_t value);
static void s_symbol_resolver(uintptr_t value);
static void s_include(uintptr_t value);
static void s_dump(uintptr_t value);
static void s_load(uintptr_t value);
static void s_if(uintptr_t value);
static void s_elseif(uintptr_t value);
static void s_else(uintptr_t value);
static void s_endif(uintptr_t value);
static void s_macros_on(uintptr_t value);
static void s_macros_off(uintptr_t value);
static void s_section(uintptr_t value);
static void s_zerofill(uintptr_t value);
static uint32_t s_builtin_section(const struct builtin_section *s);
static void s_subsections_via_symbols(uintptr_t value);
static void s_machine(uintptr_t value);
static void s_secure_log_unique(uintptr_t value);
static void s_secure_log_reset(uintptr_t value);
static void s_inlineasm(uintptr_t value);
static void s_leb128(uintptr_t sign);
static void s_incbin(uintptr_t value);
static void s_data_region(uintptr_t value);
static void s_end_data_region(uintptr_t value);

#ifdef PPC
/*
 * The routines that implement the ppcasm pseudo-ops.
 */
static void s_ppcasm_end(uintptr_t value);
#endif /* PPC */

/*
 * The machine independent pseudo op table.
 */
static const pseudo_typeS pseudo_table[] = {
#if !defined(I860) /* i860 has it's own align and org */
  { "align",	s_align_ptwo,	1	},
  { "align32",	s_align_ptwo,	4	},
  { "p2align",	s_align_ptwo,	1	},
  { "p2alignw",	s_align_ptwo,	2	},
  { "p2alignl",	s_align_ptwo,	4	},
  { "balign",	s_align_bytes,	1	},
  { "balignw",	s_align_bytes,	2	},
  { "balignl",	s_align_bytes,  4	},
  { "org",	s_org,		0	},
#endif
#ifndef M88K /* m88k has it's own abs that uses the s_abs() in here */
  { "abs",	s_abs,		0	},
#endif
  { "private_extern",  s_private_extern, 0},
#if !(defined(I386) && defined(ARCH64)) /* x86-64 doesn't support .indirect_symbol */
  { "indirect_symbol", s_indirect_symbol, 0},
#endif
  { "abort",	s_abort,	0	},
  { "ascii",	stringer,	0	},
  { "asciz",	stringer,	1	},
  { "byte",	cons,		1	},
  { "comm",	s_comm,		0	},
  { "desc",	s_desc,		0	},
  { "double",	float_cons,	'd'	},
  { "appfile",	s_app_file,	0	},
  { "fill",	s_fill,		0	},
  { "globl",	s_globl,	0	},
  { "lcomm",	s_lcomm,	0	},
  { "line",	s_line,		0	},
  { "long",	cons,		4	},
  { "quad",	cons,		8	},
  { "lsym",	s_lsym,		0	},
  { "section",	s_section,	0	},
  { "zerofill",	s_zerofill,	S_ZEROFILL	},
  { "tbss",	s_zerofill,	S_THREAD_LOCAL_ZEROFILL	},
  { "secure_log_unique",s_secure_log_unique,	0	},
  { "secure_log_reset",s_secure_log_reset,	0	},
  { "set",	s_set,		0	},
  { "short",	cons,		2	},
  { "single",	float_cons,	'f'	},
  { "space",	s_space,	0	},
  { "sleb128",	s_leb128,	1},
  { "uleb128",	s_leb128,	0},
  { "stabd",	stab,		'd'	},
  { "stabn",	stab,		'n'	},
  { "stabs",	stab,		's'	},
  { "debug_note",	stab,		's'	},
  { "reference",s_reference,	0	},
  { "lazy_reference",s_lazy_reference,	0	},
  { "weak_reference",s_weak_reference,	0	},
  { "weak_definition",s_weak_definition,	0	},
  { "weak_def_can_be_hidden",s_weak_def_can_be_hidden,	0	},
  { "no_dead_strip",s_no_dead_strip,	0	},
  { "symbol_resolver",s_symbol_resolver,	0	},
  { "include",	s_include,	0	},
  { "macro",	s_macro,	0	},
  { "endmacro",	s_endmacro,	0	},
  { "endm",	s_endmacro,	0	},
  { "macros_on",s_macros_on,	0	},
  { "macros_off",s_macros_off,	0	},
  { "if",	s_if,		0	},
  { "elseif",	s_elseif,	0	},
  { "else",	s_else,		0	},
  { "endif",	s_endif,	0	},
  { "dump",	s_dump,		0	},
  { "load",	s_load,		0	},
  { "subsections_via_symbols",	s_subsections_via_symbols,	0	},
  { "machine",	s_machine,	0	},
  { "inlineasmstart",	s_inlineasm,	1	},
  { "inlineasmend",	s_inlineasm,	0	},
  { "incbin",	s_incbin,	0	},
  { "data_region",	s_data_region,	0	},
  { "end_data_region",	s_end_data_region,	0	},
  { NULL }	/* end sentinel */
};

#ifdef PPC
/*
 * The pseudo op table for the ppcasm flavor of the PowerPC assembler.
 */
static const pseudo_typeS ppcasm_pseudo_table[] = {
  { "include",	s_include,	0	},
  { "end",	s_ppcasm_end,	0	},
  { NULL }	/* end sentinel */
};
#endif /* PPC */

/*
 * True if .secure_log_unique has been used without; reset by .secure_log_reset
 */
static enum bool s_secure_log_used = FALSE;

/*
 * read_begin() initializes the assember to read assembler source input.
 */
void
read_begin(
void)
{
      pseudo_op_begin();
      macro_begin();
      obstack_begin(&notes, 5000);

#ifdef M68K /* we allow big cons only on the 68k machines */
      bignum_low = xmalloc((int32_t)BIGNUM_BEGIN_SIZE);
      bignum_limit = bignum_low + BIGNUM_BEGIN_SIZE;
#endif
}

#ifdef PPC
/*
 * ppcasm_read_begin() does all the things needed to set up for reading ppcasm
 * PowerPC assembly syntax.
 */
void
ppcasm_read_begin(
void)
{
	/*
	 * For ppcasm allow '\r' as an end of line character and don't treat
	 * '@' and ':' as an end of line characters.
	 */
	is_end_of_line_tab['\r'] = 1;
	is_end_of_line_tab['@'] = 0;
	is_end_of_line_tab[':'] = 0;

	ppcasm_pseudo_op_begin();
}
#endif /* PPC */

/*
 * pseudo_op_begin() creates a hash table of pseudo ops from the machine
 * independent and machine dependent pseudo op tables.
 */
static
void
pseudo_op_begin(
void)
{
    const char *errtxt;
    const pseudo_typeS *pop;
    uint32_t i;
    pseudo_typeS *sections_pseudo_table;

	po_hash = hash_new();
	errtxt = NULL;

	for(pop = pseudo_table;
	    pop->poc_name && (!errtxt || *errtxt == '\0');
	    pop++)
	  errtxt = hash_insert(po_hash, pop->poc_name, (char *)pop);

	for(pop = md_pseudo_table;
	    pop->poc_name && (!errtxt || *errtxt == '\0');
	    pop++)
	  errtxt = hash_insert(po_hash, pop->poc_name, (char *)pop);

	for(i = 0; builtin_sections[i].directive != NULL; i++)
	    ;
	sections_pseudo_table = xmalloc((i + 1) * sizeof(pseudo_typeS));
	for(i = 0; builtin_sections[i].directive != NULL; i++){
	    sections_pseudo_table[i].poc_name = builtin_sections[i].directive;
	    sections_pseudo_table[i].poc_handler =
				      (void (*)(uintptr_t))s_builtin_section;
	    sections_pseudo_table[i].poc_val = (uintptr_t)(builtin_sections +i);
	}
	sections_pseudo_table[i].poc_name = NULL;
	for(pop = (const pseudo_typeS *)sections_pseudo_table;
	    pop->poc_name && (!errtxt || *errtxt == '\0');
	    pop++)
	    errtxt = hash_insert(po_hash, pop->poc_name, (char *)pop);

	if(errtxt != NULL && *errtxt != '\0'){
	    as_fatal("error constructing pseudo-op table (%s)", errtxt);
	}
}

#ifdef PPC
/*
 * ppcasm_pseudo_op_begin() creates a hash table of pseudo ops for use with the
 * ppcasm flavor of the PowerPC assembler.
 */
static
void
ppcasm_pseudo_op_begin(
void)
{
    const char *errtxt;
    char *uppercase;
    const pseudo_typeS *pop;
    uint32_t len, i;

	ppcasm_po_hash = hash_new();
	errtxt = NULL;
	for(pop = ppcasm_pseudo_table; pop->poc_name != NULL; pop++){
	    errtxt = hash_insert(ppcasm_po_hash, pop->poc_name, (char *)pop);
	    if(errtxt != NULL && *errtxt != '\0')
		as_fatal("error constructing pseudo-op table (%s)", errtxt);

	    len = strlen(pop->poc_name);
	    uppercase = xmalloc(len);
	    strcpy(uppercase, pop->poc_name);
	    for(i = 0; i < len; i++)
		uppercase[i] = toupper(uppercase[i]);
	    errtxt = hash_insert(ppcasm_po_hash, uppercase, (char *)pop);
	    if(errtxt != NULL && *errtxt != '\0')
		as_fatal("error constructing pseudo-op table (%s)", errtxt);
	}
}
#endif /* PPC */

/*
 * The NeXT version of: read_a_source_file()
 *
 * This differs from the GNU version by taking the guts of the GNU
 * read_a_source_file() (with the outer most loop removed) and renaming it
 * parse_a_buffer().  With the NeXT version of read_a_source file simply
 * containing that outer loop and a call to parse_a_buffer().  This is done
 * So that expand_macro() and parse_line_comment() can call parse_a_buffer()
 * with the buffers they create.
 */
void
read_a_source_file(
char *buffer)	/* 1st character of each buffer of lines is here. */
{
    cond_stateS	starting_cond_state;
    short starting_if_depth;

    symbolS *symbolP;

	starting_cond_state = the_cond_state;
	starting_if_depth = if_depth;

	/* Do not change segments or subsegments if this is a .include */
	if(doing_include == FALSE){
	    /*
	     * This is a new file so switch start as if a .text was seen.  This
	     * call to s_builtin_section() relys on the fact that the text
	     * section is first in the built in sections list.
	     */
	    if(flagseen['n'] == FALSE)
		text_nsect = s_builtin_section(builtin_sections);

	    /*
	     * If the -g flag is present generate the lead stabs for this
	     * physical file that is not an include file.  Each physical file's
	     * stabs are enclosed by a pair of source name stabs, N_SO, (one at
	     * the begining of the file with the name of the file and one at the
	     * end with the name "").  This is seen by nm(1) as:
	     * 	00000000 - 01 0000    SO {standard input}
	     *  ...
	     *	00000020 - 01 0000    SO
	     * To make the debugger work line numbers stabs, N_SLINE, must be
	     * contained "in a function" (after a function stab, N_FUN).  To
	     * make a function stab work it must have a type number.  Since type
	     * numbers 1 and 2 (the 1 in "int:t1=..." and the 2 in "char:t2=..."
	     * are "magic" to the debugger we use type 3 for the types of the
	     * function stabs we generate for each text label (see the routine
	     * make_stab_for_symbol() in symbols.c).  So at lead stabs at the
	     * begining of each physical file include three type stabs, L_LSYM
	     * with the correct symbol name.  The since we must have the types
	     * 1 and 2 they are just what the 'C' would produce but we don't
	     * use them.  Type 3 is the void type like the 'C' compiler would
	     * produce which we use for the function stabs' type.  These three
	     * look like this to nm(1):
	     *	00000000 - 00 0000  LSYM int:t1=r1;-2147483648;2147483647;
	     *	00000000 - 00 0000  LSYM char:t2=r2;0;127;
	     *	00000000 - 00 0000  LSYM void:t3=3
	     *
	     * Then for each text label we see, make_stab_for_symbol() will
	     * generate a stab like this (for the example lable _main):
	     *	00000000 - 01 0007   FUN _main:F3
	     * where the 'F' in F3 is an upper case 'F' for global labels and
	     * a lower case 'f' for non globals.
	     *
	     * Then for each instruction we assemble in the text we generate
	     * a line number, S_LINE, stab (see md_assembler in m68k.c, m88k.c
	     * etc).  These look like:
	     *	00000000 - 01 0008 SLINE
	     * where the 0008 is the line number.
	     */
	    if(flagseen['g']){
		symbolP = symbol_new(
			physical_input_file,
			100 /* N_SO */,
			text_nsect,
			0,
			(int)(obstack_next_free(&frags) - frag_now->fr_literal),
			frag_now);
		symbolP = symbol_new(
			"int:t1=r1;-2147483648;2147483647;",
			128 /* N_LSYM */,
			0,0,0,
			&zero_address_frag);
		symbolP = symbol_new(
			"char:t2=r2;0;127;",
			128 /* N_LSYM */,
			0,0,0,
			&zero_address_frag);
		symbolP = symbol_new(
			"void:t3=3",
			128 /* N_LSYM */,
			0,0,0,
			&zero_address_frag);
	    }
	    /*
	     * If the --gdwarf2 flag is present generate a .file for this.
	     */
	    if(debug_type == DEBUG_DWARF2){
		dwarf2_file(physical_input_file, ++dwarf2_file_number);
	    }
	}
	else{
	    /*
	     * If we are now reading an include file we will bracket it's
	     * stabs with a pair of:
	     *	00000010 - 01 0000   SOL include_file
	     *	...
	     *	0000001c - 01 0000   SOL previous_file
	     * We generate the first N_SOL here and the one for the
	     * previous_file in s_include() in read.c.
	     *
	     * CAVAT: This will only work if the include file starts off in the
	     * (__TEXT,__text) sections and ends in the (__TEXT,__text) section.
	     */
	    if(flagseen['g'] && frchain_now->frch_nsect == text_nsect){
		symbolP = symbol_new(
			physical_input_file,
			132 /* N_SOL */,
			text_nsect,
			0,
			(int)(obstack_next_free(&frags) - frag_now->fr_literal),
			frag_now);
	    }
	}

	while((buffer_limit = input_scrub_next_buffer(&buffer)) != NULL){
#ifdef PPC
	    if(flagseen[(int)'p'] == TRUE)
		ppcasm_parse_a_buffer(buffer);
	    else
#endif /* PPC */
		parse_a_buffer(buffer);
	}

	if(the_cond_state.the_cond != starting_cond_state.the_cond ||
	   the_cond_state.ignore != starting_cond_state.ignore||
	   if_depth != starting_if_depth)
	    as_bad("file contains unmatched .ifs or .elses");

	if(macro_name != NULL)
	    as_bad("file contains unmatched .macro and .endmacro for: %s",
		    macro_name);

	if(doing_include == FALSE){
	    /* See the comment at the top of this routine for a description of
	       what is going on here */
	    if(flagseen['n'] == FALSE)
		text_nsect = s_builtin_section(builtin_sections);
	    if(flagseen['g']){
		(void)symbol_new(
			"",
			100 /* N_SO */,
			text_nsect,
			0,
			(int)(obstack_next_free(&frags) - frag_now->fr_literal),
			frag_now);
	    }
	}
}

/*
 * parse_a_buffer() operates on a buffer of lines.  It drives the
 * parsing of lines of assembly code.  The lines are assumed to be "well formed"
 * assembly so the syntax recognized in here is that produced by the output of
 * the assembly preprocessor (app) or by the compiler when it produces a file
 * that starts with "#NO_APP\n".  A "well formed" assembly is lines with exactly
 * zero or one leading "well formed space character" (' ', '\t' or '\f')
 * followed by lines of:
 *	zero or more lables (a name or a digit followed by a colon)
 *		each followed by zero or one "well formed space character"
 *	exactly one of the following followed by a logicial end of line:
 *    		a pseudo opcode
 *			followed by zero or one space (' ') characters and it's
 *			arguments (the space is required when the first
 *			character of the first argument could be part of a name)
 *    		a macro to be expanded
 *    		a machine opcode
 *    		a null statement
 *		an assignment to a symbol
 *    		a full line comment (in the case of "well formed" assembly it
 *				     must be "#APP\n" of a collection of lines
 *				     wrapped in "#APP\n ... #NO_APP\n")
 * 
 * input:
 *	buffer		pointer to the start of the buffer of lines
 *			(passed as an argument)
 *	buffer_limit	pointer to the end of the buffer of lines, that is the
 *			the character it points to is NOT part of the buffer
 *			(buffer_limit is declared in this file)
 *
 * Assumptions about the buffer of lines:
 *	buffer[-1] == '\n'	as done in input-scrub.c with the cpp macro
 *				BEFORE_STRING ("\n")
 *	buffer_limit[-1] == '\n' also as done in input-scrub.c which handles
 *				partial lines internally to itself and always
 *				passes back a buffer of complete lines.
 *
 * input/output: (for other parsing routines)
 *	input_line_pointer	pointer to the next thing in the buffer after
 * 				what has been recognized (a global)
 */
static
void
parse_a_buffer(
char *buffer)
{
    char c;		/* contains the first non-space character the current
			   word used to figure out what it is */
    char *s;		/* points to a name with character after the name
			   replaced with a '\0' so it is a 'C' string */
    char after_name;	/* contains that first character after a name that
			   got replaced with a '\0' */
    char *after_name_pointer;/* points to the end of the name where the '\0' is
			   for error use only */
    char end_of_line;	/* contains an end of line character that got replaced
			   with a '\0' */
    char *start_of_line;/* points to the locical start of line we're parsing,
			   used only for macro expansion */
    pseudo_typeS *pop;	/* pointer to a pseudo op stucture returned by
			   hash_find(po_hash, s+1) to determine if it is one */
    char *the_macro;	/* pointer to a macro name returned by
			   hash_find(ma_hash, s) to determine if it is one */
    int temp;		/* the value of a number label as an integer, 1: == 1 */
    char *backup;

	/* since this is a buffer of full lines it must end in a new line */
	know(buffer_limit[-1] == '\n');

	input_line_pointer = buffer;

	/* while we have more of this buffer to parse keep parsing */
	while(input_line_pointer < buffer_limit){
	    /*
	     * At the top of this loop we know that we just parsed a label or we
	     * are at the beginning of a logical line (since their can be more
	     * than one label on a line).  start_of_line is only used by
	     * expand_macro()
	     */
	    start_of_line = input_line_pointer;

	    /*
	     * If we are not counting lines (as in the case when called by
	     * expand_macro() ) and we just previously scaned over a newline
	     * (a physical end of line) bump the line counters (see the comments
	     * at the head of this routine about "assumptions about the buffer"
	     * and why it is safe to index input_line_pointer by -1.
	     */
	    if(count_lines == TRUE && input_line_pointer[-1] == '\n')
		bump_line_counters ();

	    /*
	     * We expect a "well-formed" assembler statement.  This means it was
	     * processed by app or produced by a compiler where the file started
	     * with a leading "#APP\n".  A "well-formed" statement allows zero
	     * or one leading white space characters.
	     */
	    c = *input_line_pointer;
	    input_line_pointer++;
	    if(c == '\t' || c == ' ' || c=='\f'){
		c = *input_line_pointer;
		input_line_pointer++;
	    }
	    know(c != ' ');	/* No further leading whitespace. */
	    /*
	     * c contains the 1st significant character, *input_line_pointer
	     * points after that character.
	     */

	    /*
	     * look for the begining of a name which could be one of the
	     * following assembly statements:
	     *    A pseudo opcode and locical end of line
	     *    A macro to be expanded and locical end of line
	     *    A machine opcode and locical end of line
	     *    A user-defined label (name not digit)(no end of line needed)
	     * At NeXT labels can be enclosed in ""'s so that Objective-C like
	     * names (with spaces and colons) can be part of a name, the
	     * routine get_symbol_end() knows about this.
	     */
	    if(is_name_beginner(c) || c == '"'){
		if( c == '"')
		    s = input_line_pointer--;
		else
		    s = --input_line_pointer;
		after_name = get_symbol_end(); /* name's delimiter */
		after_name_pointer = input_line_pointer;
		/*
		 * after_name is the character after symbol.  That character's
		 * place in the input line is now '\0',done by get_symbol_end().
		 * s points to the beginning of the symbol (in the case of a
		 * pseudo-op, *s == '.').  *input_line_pointer == '\0' where
		 * after_name was.  after_name_pointer is recorded so it their
		 * is an error after the line has been restored the '\0' can
		 * be reset and the name printed.
		 */

		/*
		 * Look for a name that should be a pseudo op.  That is it is
		 * not a user defined label or an assignment to a symbol name.
		 * This must be done so such things as ".foo:" and ".bar=1" are
		 * not mistaken for illegal pseudo ops and that something like
		 * ".long: .long 1" creates a symbol named ".long".
		 */
		if(*s == '.' &&
		   (after_name != ':' &&
		    after_name != '=' &&
		   !((after_name == ' ' || after_name == '\t') &&
		     input_line_pointer[1] == '=') ) ){
		    /*
		     * Lookup what should be a pseudo op and then restore the
		     * line.
		     */
		    pop = (pseudo_typeS *)hash_find(po_hash, s+1);
		    *input_line_pointer = after_name;

		    /*
		     * A pseudo op must be followed by character that is not
		     * part of a name so it can be parsed.  If their is a first
		     * argument that could start with a character in a name then
		     * one "well formed space" (space or a tab) must follow the
		     * pseudo op (otherwise the space is optional).
		     */
		    if(after_name == ' ' || after_name == '\t')
			input_line_pointer++;

		    /*
		     * Now the current state of the line is the after_name has
		     * been placed back in the line (the line is restored) and
		     * input_line_pointer is at the start of the first argument
		     * of the pseudo op (if any).
		     */
		    if(the_cond_state.ignore){
			/*
			 * When ignoring a block of code during conditional
			 * assembly we can't ignore .if, .else, and .endif
			 * pseudo ops.
			 */
			if(pop != NULL &&
		           ( (pop->poc_handler == s_if) ||
			     (pop->poc_handler == s_elseif) ||
			     (pop->poc_handler == s_else) ||
			     (pop->poc_handler == s_endif) ) )
			    (*pop->poc_handler)(pop->poc_val);
			else
			    totally_ignore_line();
		    }
		    else if(macro_name){
			/*
			 * When defining a macro we can't ignore .endmacro
			 * pseudo ops.
			 */
			if(pop != NULL &&
			   pop->poc_handler == s_endmacro)
				(*pop->poc_handler)(pop->poc_val);
			else
			    add_to_macro_definition(start_of_line);
		    }
		    else{
			if(pop != NULL)
			    (*pop->poc_handler)(pop->poc_val);
			else{
			    after_name = *after_name_pointer;
			    *after_name_pointer = '\0';
			    /*
			     * If macros are on see if this is a use of a macro
			     * otherwise it is an unknown pseudo op.
			     */
			    if(macros_on == TRUE &&
			       (the_macro = hash_find(ma_hash, s)) != NULL){
				*after_name_pointer = after_name;
				expand_macro(the_macro);
			    }
			    else{
				as_bad ("Unknown pseudo-op: %s", s);
				*after_name_pointer = after_name;
				ignore_rest_of_line();
			    }
			}
		    }
		    continue;

		} /* if(*s == '.' && ... ) */

		/*
		 * If we are in a conditional and the state is that we are now
		 * not including lines to be assembled then ignore the line.
		 */
		if(the_cond_state.ignore){
		    *input_line_pointer = after_name;
		    totally_ignore_line();
		}
		/*
		 * If we are in the state of defining a macro then take the line
		 * for the macro definition.
		 */
		else if(macro_name != NULL){
		    *input_line_pointer = after_name;
		    add_to_macro_definition(start_of_line);
	        }
		/*
		 * Look for a user defined label.
		 */
		else if(after_name == ':'){
		    colon(s, 0);
#ifdef I860
		    /*
		     * Intel :: feature, which makes the label global if
		     * followed by two "::"'s  . This is ifdef'ed in so their
		     * is no else cause thus the slightly odd logic.
		     */
		    if(input_line_pointer[1] == ':'){
			struct symbol *symbolP;

			symbolP = symbol_find_or_make(s);
			symbolP->sy_type |= N_EXT; /* make symbol name global */
			*input_line_pointer = ':'; /* Restore first ':' */
			input_line_pointer++;	 /* step over first ':' */
		    }
#endif
		    /* put ':' back for error messages and step over it */
		    *input_line_pointer = ':';
		    input_line_pointer++;
		}
		/*
		 * Parse the assignment to a symbol.  The syntax for this is
		 * <symbol><equal><expression>.
		 */
		else if(after_name == '=' ||
		       ((after_name == ' ' || after_name == '\t') &&
		       input_line_pointer[1] == '=')){
		    equals(s);
		    demand_empty_rest_of_line();
		}
		/*
		 * If macros are on see if this is a use of a macro.
		 */
		else if(macros_on == TRUE &&
			(the_macro = hash_find(ma_hash, s)) != NULL){
		    *input_line_pointer = after_name;
		    expand_macro(the_macro);
		}
		/*
		 * Now assume it is a machine instruction and if not it
		 * will be handled as an error.  Machine instructions must be
		 * one to a line.
		 */
		else{
		    *input_line_pointer = after_name;
		    while(is_end_of_line(*input_line_pointer) == 0)
			input_line_pointer++;
		    end_of_line = *input_line_pointer;
		    *input_line_pointer = '\0';
		    md_assemble(s);
		    *input_line_pointer = end_of_line;
		    input_line_pointer++;
		}
		/*
		 * At this point we have parsed all things that could have
		 * started with a name.  Since one of these things (user defined
		 * lables could appear more than once on a line we do a continue
		 * here and start parsing as if at the begining of another
		 * logicial line.
		 */
		continue;

	    } /* if(is_name_beginner(c) || c == '"') */

	    /* empty statement */
	    if(is_end_of_line(c))
		continue;

	    /*
	     * If we are in a conditional and the state is that we are now
	     * not including lines to be assembled then ignore the line.
	     */
	    if(the_cond_state.ignore){
		totally_ignore_line();
		continue;
	    }

	    /*
	     * If we are in the state of defining a macro then take the line
	     * for the macro definition.
	     */
	    if(macro_name != NULL){
		add_to_macro_definition(start_of_line);
		continue;
	    }

	    /* local label  ("4:") */
	    if(isdigit(c)){
		backup = input_line_pointer;
		temp = c - '0';
		/* Read the whole number.  */
		while(isdigit(*input_line_pointer))
		{
		    temp = (temp * 10) + *input_line_pointer - '0';
		    ++input_line_pointer;
		}
		if(*input_line_pointer++ == ':'){
		    local_colon(temp);
		    continue;
		}
		input_line_pointer = backup;
	    }

	    /*
	     * The only full line comment that should make it here is the first
	     * of the pair of "#APP\n ... #NO_APP\n" that the compiler uses to
	     * wrap around asm() statements.  If that is the case then
	     * parse_line_comment() creates a buffer with those lines in it and
	     * calls parse_a_buffer() with that buffer.  Then returns here
	     * skiping over that part of the current buffer.
	     */
	    if(c != '\0' && strchr(md_line_comment_chars, c) != NULL){
		parse_line_comment(&buffer);
		continue;
	    }

	    as_bad("Junk character %d (%c).", c, c);
	    ignore_rest_of_line();

	} /* while(input_line_pointer < buffer_limit) */
}

#ifdef PPC
/*
 * asmppc_parse_a_buffer() operates on a buffer of lines.  It drives the
 * parsing of lines of assembly code for the PowerPC -ppcasm option.
 *
.* The lines are processed by the assembly preprocessor (app) but spaces have
 * been preserved by it when -ppcasm is specified.
 *
 * ppcasm assembly lines have the following fields:
 *
 *	Label Operation Operand Comment
 *
 * The label identifier in the Label field must end with a space or a colon (and
 * must start at the beginning of the line)
 *
 * The operation field contains the assembler directive, the instruction
 * mnemonic or macro call.
 *
 * The operand follows the operation field for machine instructions, assembler
 * directives, and macros.  However not all operations require operands.  The
 * operand field is separated from the operation field by at least one space.
 * The operand field can contain subfields separated by commas.  The number of
 * subfields is determined by the type of operation.  Spaces can appear
 * anywhere in the operand field except within a single symbol.
 * 
 * The comment field starts with a ; or a # .
 * 
 * input:
 *	buffer		pointer to the start of the buffer of lines
 *			(passed as an argument)
 *	buffer_limit	pointer to the end of the buffer of lines, that is the
 *			the character it points to is NOT part of the buffer
 *			(buffer_limit is declared in this file)
 *
 * Assumptions about the buffer of lines:
 *	buffer[-1] == '\n'	as done in input-scrub.c with the cpp macro
 *				BEFORE_STRING ("\n")
 *	buffer_limit[-1] == '\n' also as done in input-scrub.c which handles
 *				partial lines internally to itself and always
 *				passes back a buffer of complete lines.
 *
 * input/output: (for other parsing routines)
 *	input_line_pointer	pointer to the next thing in the buffer after
 * 				what has been recognized (a global)
 */
static
void
ppcasm_parse_a_buffer(
char *buffer)
{
    char c;		/* contains the first non-space character the current
			   word used to figure out what it is */
    char *s;		/* points to a name with character after the name
			   replaced with a '\0' so it is a 'C' string */
    char after_name;	/* contains that first character after a name that
			   got replaced with a '\0' */
    char *after_name_pointer;/* points to the end of the name where the '\0' is
			   for error use only */
    char end_of_line;	/* contains an end of line character that got replaced
			   with a '\0' */
    char *start_of_line;/* points to the locical start of line we're parsing,
			   used only for macro expansion */
    pseudo_typeS *pop;	/* pointer to a pseudo op stucture returned by
			   hash_find(ppcasm_po_hash, s) to determine if it is */

	/* since this is a buffer of full lines it must end in a new line */
	know(buffer_limit[-1] == '\n');

	input_line_pointer = buffer;

	/* while we have more of this buffer to parse keep parsing */
	while(input_line_pointer < buffer_limit){
	    /*
	     * Save a pointer to the start of the line so we can tell a label
	     * without a trailing colon from the operation field.
	     */
	    start_of_line = input_line_pointer;

	    /*
	     * If we are not counting lines (as in the case when called by
	     * expand_macro() ) and we just previously scaned over a newline
	     * (a physical end of line) bump the line counters (see the comments
	     * at the head of this routine about "assumptions about the buffer"
	     * and why it is safe to index input_line_pointer by -1.
	     */
	    if(count_lines == TRUE && input_line_pointer[-1] == '\n')
		bump_line_counters ();

	    /*
	     * We are at the start of a line and if there is a name then this
	     * must be a label.
	     */
	    c = *input_line_pointer;
	    if(is_name_beginner(c)){
		s = input_line_pointer;
		while(is_part_of_name(c)){
		    input_line_pointer++;
		    c = *input_line_pointer;
		}
		after_name = c;
		after_name_pointer = input_line_pointer;
		*after_name_pointer = '\0';
		colon(s, 0);
		*after_name_pointer = after_name;
		/*
		 * A colon after the name is optional and may have a spaces
		 * before it.
		 */
		if(*input_line_pointer == ' ')
		    input_line_pointer++;
		if(c == ':'){
		    input_line_pointer++;
		    c = *input_line_pointer;
		}
	    }

	    /*
	     * Now that we have passed the label at the start of the line if
	     * any skip any spaces that follow it.
	     */
	    if(*input_line_pointer == ' ')
		input_line_pointer++;
	    c = *input_line_pointer;

	    /*
	     * The next thing on the line is the operation field which is a name
	     * of a directive, instruction mnemonic or macro name.
	     */
	    if(is_name_beginner(c)){
		s = input_line_pointer;
		while(is_part_of_name(c)){
		    input_line_pointer++;
		    c = *input_line_pointer;
		}
		after_name = c;
		after_name_pointer = input_line_pointer;
		*after_name_pointer = '\0';

		/*
		 * Lookup what might be a ppcasm pseudo op and then restore the
		 * line.
		 */
		pop = (pseudo_typeS *)hash_find(ppcasm_po_hash, s);
		*input_line_pointer = after_name;

		if(*input_line_pointer == ' ')
		    input_line_pointer++;

		if(pop != NULL){
		    (*pop->poc_handler)(pop->poc_val);
		    continue;
		}
		/*
		 * Now assume the name in the operation field is a machine
		 * instruction and if not it will be handled as an error.
		 */
		else{
		    while(is_end_of_line(*input_line_pointer) == 0)
			input_line_pointer++;
		    end_of_line = *input_line_pointer;
		    *input_line_pointer = '\0';
		    md_assemble(s);
		    *input_line_pointer = end_of_line;
		    input_line_pointer++;
		    continue;
		}
	    }

	    /*
	     * If we ran into the end of the line we are done with this line.
	     */
	    /* empty statement */
	    if(is_end_of_line(c)){
		input_line_pointer++;
		continue;
	    }

	    as_bad("Junk character %d (%c).", c, c);
	    ignore_rest_of_line();

	} /* while(input_line_pointer < buffer_limit) */
}
#endif /* PPC */

/*
 * parse_line_comment() parses a line comment for parse_a_buffer().  Since
 * parse_a_buffer() only operates on "well formed" assembly the only legal
 * line comment that should appear is a "#APP\n ... #NO_APP\n" pair which
 * tells us to scrub the characters between them and then parse them.
 */
static
void
parse_line_comment(
char **buffer)
{
    char *s;
    char *ends;

    char *new_buf;
    char *new_tmp;
    int	 new_length;

    char *tmp_buf;
    char *old_input_line_pointer;
    char *old_buffer_limit;


	/* parse_a_buffer should never see any line comment if app is on */
	know(preprocess == FALSE);

	s = input_line_pointer;
	/* This must be a #APP\n line comment if not ignore it */
	if(strncmp(s,"APP\n",4) != 0)
	    return;

	if(count_lines == TRUE)
	    bump_line_counters();
	s += sizeof("APP\n") - 1;

	/*
	 * Search for the matching #NO_APP\n in this buffer, if it is found
	 * in this buffer the un-scrubed characters between the "#APP\n" and
	 * "#NO_APP\n" start where s is pointing to and end where ends is
	 * pointing to.
	 */
	ends = strstr(s, "#NO_APP\n");

	tmp_buf = NULL;
	if(ends == NULL){
	    /* The matching #NO_APP\n for the #APP\n wasn't in this buffer. */
	    long	tmp_len;
	    long	num;

	    /*
	     * First create a temporary place (tmp_buf of size tmp_len) to
	     * collect the un-scrubbed characters between the "#APP\n" and the
	     * "#NO_APP\n" (or end of file) when we find it in some buffer.
	     */
	    tmp_len = buffer_limit - s;
	    tmp_buf = xmalloc(tmp_len);

	    /*
	     * Copy the end of the buffer that contains the first part of
	     * the un-scrubbed contents starting just after the "#APP\n".
	     * This is so the the current buffer (buffer) can be used to
	     * collect the the rest of the un-scrubbed contents and to find
	     * the matching "#NO_APP\n".
	     */
	    memcpy(tmp_buf, s, tmp_len);

	    /*
	     * This loop collects the remaining un-scrubed contents between
	     * "#APP\n" and the "#NO_APP\n" into tmp_buf (adjusting tmp_len)
	     * and looks for the matching "#NO_APP\n".
	     */
	    do{
		buffer_limit = input_scrub_next_buffer(buffer);
		/*
		 * We treat runing into the end of the file as if it was the
		 * "#NO_APP" we were looking for.
		 */
		if(buffer_limit == NULL)
		    break;

		ends = strstr(*buffer, "#NO_APP\n");
		if(ends != NULL)
		    num = ends - *buffer;
		else
		    num = buffer_limit - *buffer;

		tmp_buf = xrealloc(tmp_buf, tmp_len + num);
		memcpy(tmp_buf + tmp_len, *buffer, num);
		tmp_len += num;
	    }while(ends == NULL);

	    /*
	     * Now set up buffer, buffer_limit and input_line_pointer be past
	     * all the characters of the "#APP\n ... #NO_APP\n" set so that
	     * when we return parsing will be picked up from their.
	     */
	    if(ends != NULL)
		input_line_pointer = ends + sizeof("#NO_APP\n") - 1;
	    else{
		input_line_pointer = *buffer;
		buffer_limit = *buffer;
	    }

	    /*
	     * Now set s to the start, and ends to the end of the un-scrubed
	     * contents of the collected characters between the "#APP\n" and
	     * "#NO_APP\n" pair.
	     */
	    s = tmp_buf;
	    ends = s + tmp_len;
	}
	else{
	    /*
	     * The matching "#NO_APP\n" was in the buffer as we were called so
	     * s is the start, and ends is the end of the un-scrubed contents
	     * of the characters between the "#APP\n" and "#NO_APP\n" pair.
	     * Now to set up buffer, buffer_limit and input_line_pointer be past
	     * all the characters of the "#APP\n ... #NO_APP\n" set so that
	     * when we return parsing will be picked up from their all that has
	     * to be done is move the input_line_pointer past the "#NO_APP\n".
	     */
	    input_line_pointer = ends + sizeof("#NO_APP\n") - 1;
	}

	/*
	 * Now that we have the un-scrubed characters beween s and ends setup
	 * to scrub them into a new buffer (new_buf of size new_length to
	 * new_tmp).
	 */
	new_length = 100;
	new_buf = xmalloc(new_length);
	new_tmp = new_buf;
	*new_tmp++ = '\n'; /* place leading \n in buffer for parse_a_buffer */

	scrub_string = s;
	scrub_last_string = ends;
	for(;;){
	    int c;

	    c = do_scrub_next_char_from_string();
	    if(c == EOF)
		break;
	    *new_tmp++ = c;
	    if(new_tmp == new_buf + new_length){
		new_buf = xrealloc(new_buf, new_length + 100);
		new_tmp = new_buf + new_length;
		new_length += 100;
	    }
	}
	*new_tmp = '\n'; /* place trailing \n in buffer for parse_a_buffer */

	/*
	 * If we used a temporary buffer to collect the un-scrubbed characters
	 * it is no longer needed and can be free()'ed.
	 */
	if(tmp_buf != NULL)
	    free(tmp_buf);

	/*
	 * Now we are ready to recursively call parse_a_buffer() with our buffer
	 * of scrubed characters.  So save the state of parse_a_buffer() and set
	 * it up with our buffer of scrubed characters.
	 */
	old_input_line_pointer = input_line_pointer;
	old_buffer_limit = buffer_limit;

	input_line_pointer = new_buf;
	buffer_limit = new_tmp;
#ifdef PPC
	if(flagseen[(int)'p'] == TRUE)
	    ppcasm_parse_a_buffer(new_buf);
	else
#endif /* PPC */
	    parse_a_buffer(new_buf);

	/*
	 * Free the buffer that held the scrubbed characters
	 */
	free(new_buf);

	/*
	 * After coming back from our recursive call parse_a_buffer() we want 
	 * resume parsing after the "#NO_APP\n".  So bump the line counters
	 * for the "#NO_APP\n" and restore the state so we can return to
	 * parse_a_buffer().
	 */
	if(count_lines == TRUE)
	    bump_line_counters();
	input_line_pointer = old_input_line_pointer;
	buffer_limit = old_buffer_limit;

	return;
}

/*
 * s_abort() implements the pseudo op:
 *	.abort [ "abort_string" ]
 */
static
void
s_abort(
uintptr_t value)
{
    char *p;

	p = input_line_pointer;
	while(is_end_of_line(*p) == FALSE)
	    p++;
	*p = '\0';
	
	as_fatal(".abort %s detected.  Assembly stopping.", input_line_pointer);
}

#if !defined(I860) /* i860 has it's own align and org */
/*
 * s_align_bytes() handles the .align pseudo-op where ".align 4" means align to
 * a 4 byte boundary.
 */
static
void
s_align_bytes(
uintptr_t arg)
{
	s_align((int)arg, 1);
}

/*
 * s_align_ptwo() handles the .align pseudo-op on where ".align 4" means align
 * to a 2**4 boundary.
 */
static
void
s_align_ptwo(
uintptr_t arg)
{
	s_align((int)arg, 0);
}

/*
 * s_align() implements the pseudo ops
 *  .align    align_expression [ , 1byte_fill_expression [,max_bytes_to_fill]]
 *  .p2align  align_expression [ , 1byte_fill_expression [,max_bytes_to_fill]]
 *  .p2alignw align_expression [ , 2byte_fill_expression [,max_bytes_to_fill]]
 *  .p2alignl align_expression [ , 4byte_fill_expression [,max_bytes_to_fill]]
 *  .align32  align_expression [ , 4byte_fill_expression [,max_bytes_to_fill]]
 * Where align_expression is a power of 2 alignment.
 * 
 * The parameter fill_size can only be 1, 2 or 4 which is the size of the
 * fill_expression.  If the parameter bytes_p is non-zero the alignment value
 * is interpreted as the byte boundary, rather than the power of 2.
 */
static
void
s_align(
int fill_size,
int bytes_p)
{
    int power_of_2_alignment, byte_alignment, i;
    int32_t temp_fill, fill_specified, max_bytes_to_fill;
    char fill[4];

	if(fill_size != 1 && fill_size != 2 && fill_size != 4)
	    as_bad("Internal error, s_align() called with bad fill_size %d",
		    fill_size);

	power_of_2_alignment = 0;
	if(bytes_p == 0){
	    power_of_2_alignment = (int)get_absolute_expression();
	}
	else{
	    byte_alignment = (int)get_absolute_expression();
	    if(byte_alignment != 0){
		for(i = 0; (byte_alignment & 1) == 0; i++)
		    byte_alignment >>= 1;
		if(byte_alignment != 1)
		    as_bad("alignment not a power of 2");
		power_of_2_alignment = i;
	    }
	}
#define MAX_ALIGNMENT (15)
	if(power_of_2_alignment > MAX_ALIGNMENT)
	    as_warn("Alignment too large: %d. assumed.",
		    power_of_2_alignment = MAX_ALIGNMENT);
	else if(power_of_2_alignment < 0){
	    as_warn("Alignment negative. 0 assumed.");
	    power_of_2_alignment = 0;
	}
	temp_fill = 0;
	fill_specified = 0;
	max_bytes_to_fill = 0;
	if(*input_line_pointer == ','){
	    input_line_pointer ++;
	    if(*input_line_pointer != ','){
		temp_fill = (int32_t)get_absolute_expression ();
		fill_specified = 1;
	    }
	    if(*input_line_pointer == ','){
		input_line_pointer ++;
		max_bytes_to_fill = (int32_t)get_absolute_expression ();
	    }
	}

	/*
	 * If the fill has not been specified and this section has
	 * machine instructions then pad the section with nops.
	 */
	if(fill_specified == 0 &&
	   ((frchain_now->frch_section.flags & S_ATTR_SOME_INSTRUCTIONS) ==
	     S_ATTR_SOME_INSTRUCTIONS ||
	    (frchain_now->frch_section.flags & S_ATTR_PURE_INSTRUCTIONS) ==
	     S_ATTR_PURE_INSTRUCTIONS) ){
#ifdef M68K
	    if(power_of_2_alignment >= 1){
		temp_fill = 0x4e71; /* m68k nop */
		fill_size = 2; /* 2 byte fill size */
	    }
#endif /* M68K */
#ifdef I386
	    temp_fill = 0x90; /* i386 nop */
	    fill_size = 1; /* 1 byte fill size */
#endif /* I386 */
#ifdef HPPA
	    if(power_of_2_alignment >= 2){
		temp_fill = 0x08000240; /* hppa nop */
		fill_size = 4; /* 4 byte fill size */
	    }
#endif /* HPPA */
#ifdef SPARC
	    if(power_of_2_alignment >= 2){
		temp_fill = 0x01000000; /* sparc nop */
		fill_size = 4; /* 4 byte fill size */
	    }
#endif /* SPARC */
#ifdef M88K
	    if(power_of_2_alignment >= 2){
		temp_fill = 0xf4005800; /* m88k 'or r0,r0,r0' instruction */
		fill_size = 4; /* 4 byte fill size */
	    }
#endif /* M88K */
#ifdef PPC
	    if(power_of_2_alignment >= 2){
		temp_fill = 0x60000000; /* ppc nop */
		fill_size = 4; /* 4 byte fill size */
	    }
#endif /* PPC */
#ifdef ARM
	    if(power_of_2_alignment >= 1){
		extern int thumb_mode; /* from arm.c */
		if(thumb_mode){
	    	    if(archflag_cpusubtype == CPU_SUBTYPE_ARM_V7 ||
	    	       archflag_cpusubtype == CPU_SUBTYPE_ARM_V7F ||
	    	       archflag_cpusubtype == CPU_SUBTYPE_ARM_V7K){
			temp_fill = 0xbf00; /* thumb2 nop */
			fill_size = 2; /* 2 byte fill size */
		    }
		    else{
			temp_fill = 0x46c0; /* thumb1 nop */
			fill_size = 2; /* 2 byte fill size */
		    }
		}
		else if(power_of_2_alignment >= 2){
		    temp_fill = 0xe1a00000; /* arm nop */
		    fill_size = 4; /* 4 byte fill size */
		}
	    }
#endif /* ARM */
	    ; /* empty statement for other architectures */
	}

	md_number_to_chars(fill, temp_fill, fill_size);

	/* Only make a frag if we HAVE to. . . */
	if(power_of_2_alignment != 0)
	    frag_align(power_of_2_alignment, fill, fill_size,max_bytes_to_fill);

	/*
	 * If there is not a max_bytes_to_fill specified and this alignment is
	 * larger than any previous alignment then this becomes the section's
	 * alignment.  If there is a max_bytes_to_fill then this is handled in
	 * relax_section() if the alignment can be done without exceeding
	 * max_bytes_to_fill.
	 */
	if(max_bytes_to_fill == 0 &&
           frchain_now->frch_section.align <
	   (uint32_t)power_of_2_alignment)
	    frchain_now->frch_section.align = power_of_2_alignment;

	demand_empty_rest_of_line();
}
#endif /* !defined(I860) i860 has it's own align and org */

/*
 * s_comm() implements the pseudo op:
 *	.comm name , expression
 */
static
void
s_comm(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    signed_target_addr_t temp;
    symbolS *symbolP;
    int power_of_2_alignment;

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	/* just after name is now '\0' */
	p = input_line_pointer;
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    as_bad("Expected comma after symbol-name");
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer++; /* skip ',' */
	if((temp = get_absolute_expression ()) < 0){
	    as_bad(".COMMon length (" TA_DFMT ".) <0! Ignored.", temp);
	    ignore_rest_of_line();
	    return;
	}
	power_of_2_alignment = 0;
#define MAX_ALIGNMENT (15)
	if(*input_line_pointer == ','){
	    input_line_pointer++;
	    power_of_2_alignment = (int)get_absolute_expression();
	    if(power_of_2_alignment > MAX_ALIGNMENT)
		as_warn("Alignment too large: %d. assumed.",
			power_of_2_alignment = MAX_ALIGNMENT);
	    else if(power_of_2_alignment < 0){
		as_warn("Alignment negative. 0 assumed.");
		power_of_2_alignment = 0;
	    }
	}
	*p = 0;
	symbolP = symbol_find_or_make(name);
	*p = c;
	if((symbolP->sy_type & N_TYPE) != N_UNDF ||
	   symbolP->sy_other != 0 ||
	   (symbolP->sy_desc & ~N_NO_DEAD_STRIP) != 0) {
	    as_bad("Ignoring attempt to re-define symbol");
	    ignore_rest_of_line();
	    return;
	}
	if(symbolP->sy_value != 0){
	    if(symbolP->sy_value != (uint32_t)temp)
		as_bad("Length of .comm \"%s\" is already " TA_DFMT ". Not "
			"changed to " TA_DFMT ".", symbolP->sy_name,
			symbolP->sy_value, temp);
	}
	else{
	    symbolP -> sy_value = temp;
	    symbolP -> sy_type |= N_EXT;
	    SET_COMM_ALIGN(symbolP->sy_desc, power_of_2_alignment);
	}
	know(symbolP->sy_frag == &zero_address_frag);
	demand_empty_rest_of_line();
}

/*
 * s_desc() implements the pseudo op:
 *	.desc name , expression
 * sets the n_desc field of a symbol.
 */
static
void
s_desc(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;
    int temp;

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;
	symbolP = symbol_table_lookup(name);
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    *p = 0;
	    as_bad("Expected comma after name \"%s\"", name);
	    *p = c;
	    ignore_rest_of_line();
	}
	else{
	    input_line_pointer++;
	    temp = (int)get_absolute_expression();
	    *p = 0;
	    symbolP = symbol_find_or_make(name);
	    *p = c;
	    symbolP->sy_desc = temp;
	}
	demand_empty_rest_of_line();
}

/*
 * s_app_file() implements the pseudo op:
 *	.file name [ level_number ]
 * the level number is generated by /lib/cpp and is just ignored.
 */
void
s_app_file(
uintptr_t value)
{
    char *s;
    int length;
    struct symbol *symbolP;

	/* Some assemblers tolerate immediately following '"' */
	if((s = demand_copy_string(&length))){
	    SKIP_WHITESPACE();
	    if(*input_line_pointer >= '0' && *input_line_pointer <= '9'){
		while(*input_line_pointer >= '0' &&
		      *input_line_pointer <= '9')
		      input_line_pointer++;
	    }
	    new_logical_line(s, -1);
	    demand_empty_rest_of_line();

	    /*
	     * This is to generate stabs for debugging assembly code.
	     * See the comments about stabs in read_a_source_file()
	     * for a description of what is going on here.
	     */
	    if(flagseen['g'] && frchain_now->frch_nsect == text_nsect){
		symbolP = symbol_new(
			      logical_input_file,
			      132 /* N_SOL */,
			      text_nsect,
			      0,
			      (int)(obstack_next_free(&frags) -
				    frag_now->fr_literal),
			      frag_now);
	    }
	}
}

/*
 * s_fill() implements the pseudo op:
 *	.fill repeat_expression , fill_size , fill_expression
 */
static
void
s_fill(
uintptr_t value)
{
    int32_t temp_repeat;
    int32_t temp_size;
    int32_t temp_fill;
    char *p;

	if(get_absolute_expression_and_terminator(&temp_repeat) != ','){
	    input_line_pointer--; /* Backup over what was not a ','. */
	    as_bad("Expect comma after rep-size in .fill");
	    ignore_rest_of_line();
	    return;
	}
	if(get_absolute_expression_and_terminator(&temp_size) != ','){
	    input_line_pointer--; /* Backup over what was not a ','. */
	    as_bad("Expected comma after size in .fill");
	    ignore_rest_of_line();
	    return;
	}
	/*
	 * This is to be compatible with BSD 4.2 AS, not for any rational
	 * reason.
	 */
#define BSD_FILL_SIZE_CROCK_8 (8)
	if(temp_size > BSD_FILL_SIZE_CROCK_8){
	    as_bad(".fill size clamped to %d.", BSD_FILL_SIZE_CROCK_8);
	    temp_size = BSD_FILL_SIZE_CROCK_8 ;
	}
	if(temp_size < 0){
	    as_bad("Size negative: .fill ignored.");
	    temp_size = 0;
	}
	/*
	 * bug fix, if md_number_to_chars() is called with something other than
	 * 1,2 or 4 it calls abort().  So we don't let the size be something
	 * like 3. Bug #13017.
	 */
	else if(temp_size != 0 &&
		temp_size != 1 &&
		temp_size != 2 &&
		temp_size != 4 &&
		temp_size != 8){
	    as_bad(".fill size must be 0,1,2,4 or 8, .fill ignored");
	    temp_size = 0;
	}
	else if(temp_repeat <= 0){
	    as_bad(".fill repeat <= 0, .fill ignored");
	    temp_size = 0;
	}
	temp_fill = (int32_t)get_absolute_expression();
	/*
	 * Note: .fill (),0 emits no frag (since we are asked to .fill 0 bytes)
	 * but emits no error message because it seems a legal thing to do.
	 * It is a degenerate case of .fill but could be emitted by a compiler.
	 */
	if(temp_size != 0){
	      p = frag_var(rs_fill,
			   (int)temp_size,
			   (int)temp_size,
			   (relax_substateT)0,
			   (symbolS *)0,
			   temp_repeat,
			   (char *)0);
	      memset(p, '\0', (int)temp_size);
	      /*
	       * The magic number BSD_FILL_SIZE_CROCK_4 is from BSD 4.2 VAX
	       * flavoured AS. The following bizzare behaviour is to be
	       * compatible with above.  I guess they tried to take up to 8
	       * bytes from a 4-byte expression and they forgot to sign extend.
	       */
#define BSD_FILL_SIZE_CROCK_4 (4)
	      md_number_to_chars(p,
				 temp_fill,
				 temp_size > BSD_FILL_SIZE_CROCK_4 ?
					BSD_FILL_SIZE_CROCK_4 : (int)temp_size);
	}
	demand_empty_rest_of_line();
}

/*
 * s_globl() implements the pseudo op:
 *	.globl name [ , name ]
 */
void
s_globl(
uintptr_t value)
{
    char *name;
    int c;
    symbolS *symbolP;

	do{
	    if(*input_line_pointer == '"')
		name = input_line_pointer + 1;
	    else
		name = input_line_pointer;
	    c = get_symbol_end();
	    symbolP = symbol_find_or_make(name);
	    *input_line_pointer = c;
	    SKIP_WHITESPACE();
	    symbolP->sy_type |= N_EXT;
	    if(c == ','){
		input_line_pointer++;
		SKIP_WHITESPACE();
		if(*input_line_pointer == '\n')
		    c = '\n';
	    }
	}while(c == ',');
	demand_empty_rest_of_line();
}

/*
 * s_private_extern() implements the pseudo op:
 *	.private_extern name [ , name ]
 */
static
void
s_private_extern(
uintptr_t value)
{
    char *name;
    int c;
    symbolS *symbolP;

	do{
	    if(*input_line_pointer == '"')
		name = input_line_pointer + 1;
	    else
		name = input_line_pointer;
	    c = get_symbol_end();
	    symbolP = symbol_find_or_make(name);
	    *input_line_pointer = c;
	    SKIP_WHITESPACE();
	    symbolP->sy_type |= N_EXT;
	    symbolP->sy_type |= N_PEXT;
	    if(c == ','){
		input_line_pointer++;
		SKIP_WHITESPACE();
		if(*input_line_pointer == '\n')
		    c = '\n';
	    }
	}while(c == ',');
	demand_empty_rest_of_line();
}

#if !(defined(I386) && defined(ARCH64))
/*
 * s_indirect_symbol() implements the pseudo op:
 *	.indirect_symbol name
 */
static
void
s_indirect_symbol(
uintptr_t value)
{
    char *name;
    int c;
    uint32_t section_type;

	if(!flagseen['k'])
	    as_fatal("incompatible feature used: .indirect_symbol (must "
		     "specify \"-dynamic\" to be used)");
	if(frchain_now == NULL){
	    know(flagseen['n']);
	    as_fatal("with -n a section directive must be seen before assembly "
		     "can begin");
	}
	section_type = frchain_now->frch_section.flags & SECTION_TYPE;
	if(section_type != S_NON_LAZY_SYMBOL_POINTERS &&
	   section_type != S_LAZY_SYMBOL_POINTERS &&
	   section_type != S_SYMBOL_STUBS){
	    as_bad("indirect symbol not in a symbol pointer or stub section, "
		    ".indirect_symbol ignored");
	    ignore_rest_of_line();
	    return;
	}

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	indirect_symbol_new(name,
			    frag_now,
			    (int)(obstack_next_free(&frags) -
				  frag_now->fr_literal));
	*input_line_pointer = c;

	demand_empty_rest_of_line();
}
#endif

/*
 * s_lcomm() implements the pseudo op:
 *	.lcomm name , size_expression [ , align_expression ]
 */
static
void
s_lcomm(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    signed_target_addr_t size;
    symbolS *symbolP;
    int align;
    static frchainS *bss = NULL;

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    as_bad("Expected comma after name");
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer ++;
	if((size = get_absolute_expression()) < 0){
	    as_bad("BSS length (" TA_DFMT ".) <0! Ignored.", size);
	    ignore_rest_of_line();
	    return;
	}
#define MAX_ALIGNMENT (15)
	align = 0;
	if(*input_line_pointer == ','){
	    input_line_pointer++;
	    align = (int)get_absolute_expression();
	    if(align > MAX_ALIGNMENT){
		as_warn("Alignment too large: %d. assumed.", MAX_ALIGNMENT);
		align = MAX_ALIGNMENT;
	    }
	    else if(align < 0){
		as_warn("Alignment negative. 0 assumed.");
		align = 0;
	    }
	}
	*p = 0;
	symbolP = symbol_find_or_make(name);
	*p = c;

	if((symbolP->sy_type & N_TYPE) == N_UNDF && symbolP->sy_value == 0){
	    if(bss == NULL){
		bss = section_new(SEG_DATA, SECT_BSS, S_ZEROFILL, 0, 0);
		bss->frch_root = xmalloc(SIZEOF_STRUCT_FRAG);
		memset(bss->frch_root, '\0', SIZEOF_STRUCT_FRAG);
		bss->frch_last = bss->frch_root;
	    }
	    bss->frch_root->fr_address = rnd(bss->frch_root->fr_address,
					     1 << align);
	    symbolP->sy_value = (uint32_t)bss->frch_root->fr_address;
	    symbolP->sy_type  = N_SECT;
	    symbolP->sy_other = bss->frch_nsect;
	    symbolP->sy_frag  = bss->frch_root;
	    bss->frch_root->fr_address += size;
	    /*
	     * If this alignment is larger than any previous alignment then this
	     * becomes the section's alignment.
	     */
	    if(bss->frch_section.align < (uint32_t)align)
		bss->frch_section.align = align;
	}
	else
	    as_bad("Ignoring attempt to re-define symbol.");
	demand_empty_rest_of_line();
}

/*
 * s_line() implements the pseudo op:
 *	.line line_number
 */
void
s_line(
uintptr_t value)
{
	/*
	 * Assume delimiter is part of expression. BSD4.2 as fails with
	 * delightful bug, so we are not being incompatible here.
	 */
	/*
	 * Since the assembler bumps it's line counters at the end of a line
	 * and it is the case that the .line is on it's own line what the
	 * intent is that the line number is for the next line.  Thus
	 * the -1 .  This is the way cpp'ed assembler files work which is the
	 * common case.
	 */
	new_logical_line((char *)NULL, (int)(get_absolute_expression()) - 1);
	demand_empty_rest_of_line();
}

/*
 * s_lsym() implements the pseudo op:
 *	.lsym name , expression
 */
static
void
s_lsym(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    segT segment;
    expressionS exp;
    symbolS *symbolP;

	/* we permit ANY expression: BSD4.2 demands constants */
	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    *p = 0;
	    as_bad("Expected comma after name \"%s\"", name);
	    *p = c;
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer++;
	segment = expression(&exp);
	if(segment != SEG_ABSOLUTE && segment != SEG_SECT){
/* this warning still need fixing */
	    as_bad("Bad expression: %s", seg_name[(int)segment]);
	    ignore_rest_of_line();
	    return;
	}
	know(segment == SEG_ABSOLUTE || segment == SEG_SECT);
	*p = 0;
	if(segment == SEG_SECT)
	    symbolP = symbol_new(name,
				 N_SECT,
	    			 frchain_now->frch_nsect,
				 0,
				 (valueT)(exp.X_add_number),
				 &zero_address_frag);
	else
	    symbolP = symbol_new(name,
				 N_ABS,
	    			 0,
				 0,
				 (valueT)(exp.X_add_number),
				 &zero_address_frag);
	*p = c;
	demand_empty_rest_of_line();
}

#if !defined(I860) /* i860 has it's own align and org */
/*
 * s_org() implements the pseudo op:
 *	.org  expression
 */
static
void
s_org(
uintptr_t value)
{
    segT segment;
    expressionS exp;
    int32_t temp_fill;
    char *p;

	/*
	 * Don't believe the documentation of BSD 4.2 AS.
	 * There is no such thing as a sub-segment-relative origin.
	 * Any absolute origin is given a warning, then assumed to be
	 * segment-relative.
	 * Any segmented origin expression ("foo+42") had better be in the right
	 * segment or the .org is ignored.
	 *
	 * BSD 4.2 AS warns if you try to .org backwards. We cannot because we
	 * never know sub-segment sizes when we are reading code.
	 * BSD will crash trying to emit -ve numbers of filler bytes in certain
	 * .orgs. We don't crash, but see as-write for that code.
	 */
	segment = get_known_segmented_expression(&exp);
	if(*input_line_pointer == ','){
	    input_line_pointer ++;
	    temp_fill = (int32_t)get_absolute_expression ();
	}
	else
	    temp_fill = 0;
	if((segment != SEG_SECT ||
	    exp.X_add_symbol->sy_other != frchain_now->frch_nsect) &&
	    segment != SEG_ABSOLUTE)
	    as_bad("Illegal expression. current section assumed.");
	p = frag_var(rs_org,
		     1,
		     1,
		     (relax_substateT)0,
		     exp.X_add_symbol,
		     (int32_t)exp.X_add_number,
		     (char *)0);
	*p = temp_fill;
	demand_empty_rest_of_line();
}
#endif /* !defined(I860) i860 has it's own align and org */

/*
 * s_set() implements the pseudo op:
 *	.set name , expression
 */
static
void
s_set(
uintptr_t value)
{
    char *name;
    char delim;
    char *end_name;
    symbolS *symbolP;

	if( * input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = delim;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    *end_name = 0;
	    as_bad("Expected comma after name \"%s\"", name);
	    *end_name = delim;
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer++;
	*end_name = 0;
	if(name[0] == '.' && name[1] == '\0'){
	    /* Turn 'set . , mumble' into a .org mumble */
	    segT segment;
	    expressionS exp;
	    char *ptr;

	    segment = get_known_segmented_expression(&exp);
	    if((segment != SEG_SECT ||
		exp.X_add_symbol->sy_other != frchain_now->frch_nsect) &&
		segment != SEG_ABSOLUTE)
		as_bad("Illegal expression. current section assumed.");
	    ptr = frag_var(rs_org,
			   1,
			   1,
			   (relax_substateT)0,
			   exp.X_add_symbol,
			   (int32_t)exp.X_add_number,
			   (char *)0);
	    *ptr = 0;
	    *end_name = delim;
	    return;
	}
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= N_NO_DEAD_STRIP;
	*end_name = delim;
	pseudo_set(symbolP);
	demand_empty_rest_of_line();
}

/*
 * s_abs() implements the pseudo op:
 *	.abs name , expression
 * which sets symbol to 1 or 0 depending on if the expression is an absolute
 * expression.  This is intended for use in macros.
 */
void
s_abs(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    segT segment;
    expressionS exp;
    symbolS *symbolP;

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    *p = 0;
	    as_bad("Expected comma after name \"%s\"", name);
	    *p = c;
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer++;
	*p = 0;
	segment = expression(&exp);
	symbolP = symbol_find_or_make(name);
	symbolP->sy_type = N_ABS;
	symbolP->sy_other = 0; /* NO_SECT */
	symbolP->sy_frag = &zero_address_frag;
	if(segment == SEG_ABSOLUTE)
	    symbolP->sy_value = 1;
	else
	    symbolP->sy_value = 0;
	*p = c;
	totally_ignore_line();
}

/*
 * s_space() implements the pseudo op:
 *	.space repeat_expression [ , fill_expression ]
 */
void
s_space(
uintptr_t value)
{
    int32_t temp_repeat;
    int32_t temp_fill;
    char *p;

	/* Just like .fill, but temp_size = 1 */
	if(get_absolute_expression_and_terminator(&temp_repeat) == ','){
	    temp_fill = (int32_t)get_absolute_expression();
	}
	else{
	    input_line_pointer--; /* Backup over what was not a ','. */
	    temp_fill = 0;
	}
	if(temp_repeat <= 0){
	    as_bad("Repeat < 0, .space ignored");
	    ignore_rest_of_line();
	    return;
	}
	p = frag_var(rs_fill,
		     1,
		     1,
		     (relax_substateT)0,
		     (symbolS *)0,
		     temp_repeat,
		     (char *)0);
	*p = temp_fill;
	demand_empty_rest_of_line();
}

static
uint32_t
s_builtin_section(
const struct builtin_section *s)
{
    frchainS *frcP;

	if(!flagseen['k']){
	    if((s->flags & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS ||
	       (s->flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS ||
	       (s->flags & SECTION_TYPE) == S_SYMBOL_STUBS ||
#if !(defined(I386) && defined(ARCH64))
	       (s->flags & SECTION_TYPE) == S_MOD_INIT_FUNC_POINTERS ||
	       (s->flags & SECTION_TYPE) == S_MOD_TERM_FUNC_POINTERS ||
#endif
	       (s->flags & SECTION_ATTRIBUTES & ~S_ATTR_PURE_INSTRUCTIONS) != 0)
		as_fatal("incompatible feature used: directive .%s (must "
			 "specify \"-dynamic\" to be used)", s->directive);
	}
	frcP = section_new(s->segname, s->sectname,
			   s->flags & SECTION_TYPE,
			   s->flags & SECTION_ATTRIBUTES, 
			   s->sizeof_stub);
	if(frcP->frch_section.align < s->default_align)
	    frcP->frch_section.align = s->default_align;
	return(frcP->frch_nsect);
}

/*
 * s_section() implements the pseudo op:
 *	.section segname , sectname [[[ , type ] , attribute] , sizeof_stub]
 */
static
void
s_section(
uintptr_t value)
{
    char *segname, *sectname, *typename;
    char c, d, e, *p, *q, *r;
    struct type_name *type_name;
    uint32_t type, attribute;
    section_t s;
    frchainS *frcP;
    uint32_t sizeof_stub;

    struct attribute_name *attribute_name;
    char *attributename, *sizeof_stub_name, f, g, *t, *u, *endp;

	segname = input_line_pointer;
	do{
	    c = *input_line_pointer++ ;
	}while(c != ',' && c != '\0' && c != '\n');
	if(c != ','){
	    as_bad("Expected comma after segment-name");
	    ignore_rest_of_line();
	    return;
	}
	p = input_line_pointer - 1;

	SKIP_WHITESPACE();
	sectname = input_line_pointer;
	do{
	    d = *input_line_pointer++ ;
	}while(d != ',' && d != '\0' && d != '\n');
	if(p + 1 == input_line_pointer){
	    as_bad("Expected section-name after comma");
	    ignore_rest_of_line();
	    return;
	}
	q = input_line_pointer - 1;

	*p = 0;
	if(strlen(segname) > sizeof(s.segname)){
	    as_bad("segment-name: %s too long (maximum %ld characters)",
		    segname, sizeof(s.segname));
	    ignore_rest_of_line();
	    *p = c;
	    return;
	}

	*q = 0;
	if(strlen(sectname) > sizeof(s.sectname)){
	    as_bad("section-name: %s too long (maximum %ld characters)",
		    sectname, sizeof(s.sectname));
	    ignore_rest_of_line();
	    return;
	}
	/*
	 * Now see if the optional section type is present.
	 */
	type = 0;
	type_name = type_names;
	attribute = 0;
	attribute_name = attribute_names;
	sizeof_stub = 0;
	if(d == ','){
	    typename = input_line_pointer;
	    do{
		e = *input_line_pointer++ ;
	    }
	    while(e != ',' && !(is_end_of_line(e)));
	    r = input_line_pointer - 1;
	    *r = 0;
	    for(type_name = type_names; type_name->name != NULL; type_name++)
		if(strcmp(type_name->name, typename) == 0)
		    break;
	    if(type_name->name == NULL){
		as_bad("unknown section type: %s", typename);
		ignore_rest_of_line();
		return;
	    }
	    *r = e;
	    type = type_name->type;
	    /*
	     * Now see if the optional section attribute is present.
	     */
	    if(e == ','){
		do{
		    attributename = input_line_pointer;
		    do{
			f = *input_line_pointer++ ;
		    }while(f != ',' && f != '+' && !(is_end_of_line(f)));
		    t = input_line_pointer - 1;
		    *t = 0;
		    for(attribute_name = attribute_names;
			attribute_name->name != NULL;
			attribute_name++)
			if(strcmp(attribute_name->name, attributename) == 0)
			    break;
		    if(attribute_name->name == NULL){
			as_bad("unknown section attribute: %s", attributename);
			ignore_rest_of_line();
			return;
		    }
		    *t = f;
		    attribute |= attribute_name->attribute;
		}while(f == '+');

		/*
		 * Now get the section stub size if this is a stub section.
		 */
		if(type == S_SYMBOL_STUBS){
		    if(f == ','){
			sizeof_stub_name = input_line_pointer;
			do{
			    g = *input_line_pointer++ ;
			}while(!(is_end_of_line(g)));
			u = input_line_pointer - 1;
			*u = 0;
			sizeof_stub = (uint32_t)strtoul(sizeof_stub_name,
							&endp, 0);
			if(*endp != '\0'){
			    as_bad("size of stub section: %s not a proper "
				    "number", sizeof_stub_name);
			    ignore_rest_of_line();
			    return;
			}
			*u = g;
		    }
		    else{
			as_bad("missing size of stub section (%s,%s)", segname,
				sectname);
			ignore_rest_of_line();
			return;
		    }
		}
	    }
	    else if(type == S_SYMBOL_STUBS){
		as_bad("missing size of stub section (%s,%s)", segname,
			sectname);
		ignore_rest_of_line();
		return;
	    }
	}
	input_line_pointer--;

	if(!flagseen['k']){
	    if(type == S_NON_LAZY_SYMBOL_POINTERS ||
	       type == S_LAZY_SYMBOL_POINTERS ||
	       type == S_SYMBOL_STUBS ||
	       type == S_MOD_INIT_FUNC_POINTERS ||
	       type == S_MOD_TERM_FUNC_POINTERS)
		as_fatal("incompatible feature used: section type %s (must "
			 "specify \"-dynamic\" to be "
			 "used)", type_name->name);
	}

	frcP = section_new(segname, sectname, type, attribute, sizeof_stub);
	*p = c;
	*q = d;
	demand_empty_rest_of_line();
}

/*
 * s_zerofill() implements the pseudo op:
 *	.zerofill segname , sectname [, symbolname , size_expression [ , align]]
 */
static
void
s_zerofill(
uintptr_t value)
{
    char *directive, *segname, *sectname, c, d, *p, *q, *name;
    section_t s;
    frchainS *frcP;
    symbolS *symbolP;
    uint64_t size;
    int align;

	if(value == S_THREAD_LOCAL_ZEROFILL){
	    directive = "tbss";
	    frcP = section_new("__DATA", "__thread_bss", (uint32_t)value, 0, 0);
	    if(frcP->frch_root == NULL){
		frcP->frch_root = xmalloc(SIZEOF_STRUCT_FRAG);
		frcP->frch_last = frcP->frch_root;
		memset(frcP->frch_root, '\0', SIZEOF_STRUCT_FRAG);
	    }
	}
	else{
	    directive = "zerofill";
	    segname = input_line_pointer;
	    do{
		c = *input_line_pointer++ ;
	    }while(c != ' ' && c != ',' && c != '\0' && c != '\n');
	    p = input_line_pointer - 1;
	    while(c == ' '){
		c = *input_line_pointer++ ;
	    }
	    if(c != ','){
		as_bad("Expected comma after segment-name");
		ignore_rest_of_line();
		return;
	    }

	    SKIP_WHITESPACE();
	    sectname = input_line_pointer;
	    do{
		d = *input_line_pointer++ ;
	    }while(d != ',' && d != '\0' && d != '\n');
	    if(p + 1 == input_line_pointer){
		as_bad("Expected section-name after comma");
		ignore_rest_of_line();
		return;
	    }
	    q = input_line_pointer - 1;

	    *p = 0;
	    if(strlen(segname) > sizeof(s.segname)){
		as_bad("segment-name: %s too long (maximum %ld characters)",
			segname, sizeof(s.segname));
		ignore_rest_of_line();
		*p = c;
		return;
	    }

	    *q = 0;
	    if(strlen(sectname) > sizeof(s.sectname)){
		as_bad("section-name: %s too long (maximum %ld characters)",
			sectname, sizeof(s.sectname));
		ignore_rest_of_line();
		*p = c;
		*q = d;
		return;
	    }

	    frcP = section_new(segname, sectname, (uint32_t)value, 0, 0);
	    if(frcP->frch_root == NULL){
		frcP->frch_root = xmalloc(SIZEOF_STRUCT_FRAG);
		frcP->frch_last = frcP->frch_root;
		memset(frcP->frch_root, '\0', SIZEOF_STRUCT_FRAG);
	    }
	    *p = c;
	    *q = d;
	    /*
	     * If this is the end of the line all that was wanted was to create
	     * the the section which is now done, so return.
	     */
	    if(d != ',')
		return;
	}

	if(*input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;
	*p = c;
	SKIP_WHITESPACE();
	if(*input_line_pointer != ','){
	    as_bad("Expected comma after symbol-name");
	    ignore_rest_of_line();
	    return;
	}
	input_line_pointer ++;
	if((int)(size = get_absolute_expression()) < 0){
	    as_bad("%s size (%lld.) <0! Ignored.", directive, size);
	    ignore_rest_of_line();
	    return;
	}
	align = 0;
	if(*input_line_pointer == ','){
	    input_line_pointer++;
	    align = (int)get_absolute_expression();
	    if(align > MAX_ALIGNMENT){
		as_warn("Alignment too large: %d. assumed.", MAX_ALIGNMENT);
		align = MAX_ALIGNMENT;
	    }
	    else if(align < 0){
		as_warn("Alignment negative. 0 assumed.");
		align = 0;
	    }
	    /*
	     * If this alignment is larger than any previous alignment then this
	     * becomes the section's alignment.
	     */
	    if(frcP->frch_section.align < (uint32_t)align)
		frcP->frch_section.align = align;
	}
	*p = 0;
	symbolP = symbol_find_or_make(name);
	*p = c;

	if((symbolP->sy_type & N_TYPE) == N_UNDF && symbolP->sy_value == 0){
	    frcP->frch_root->fr_address = rnd(frcP->frch_root->fr_address,
					      1 << align);
	    symbolP->sy_value = (uint32_t)frcP->frch_root->fr_address;
	    symbolP->sy_type  = N_SECT | (symbolP->sy_type & (N_EXT | N_PEXT));
	    symbolP->sy_other = frcP->frch_nsect;
	    symbolP->sy_frag  = frcP->frch_root;
	    frcP->frch_root->fr_address += size;
	}
	else
	    as_bad("Ignoring attempt to re-define symbol.");

	demand_empty_rest_of_line();
}

/*
 * s_reference() implements the pseudo op:
 *	.reference name
 */
static
void
s_reference(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= N_NO_DEAD_STRIP;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_lazy_reference() implements the pseudo op:
 *	.lazy_reference name
 */
static
void
s_lazy_reference(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(!flagseen['k'])
	    as_fatal("incompatible feature used: .lazy_reference (must specify "
		     "\"-dynamic\" to be used)");

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	if((symbolP->sy_type & N_TYPE) == N_UNDF && symbolP->sy_value == 0)
	    symbolP->sy_desc |= REFERENCE_FLAG_UNDEFINED_LAZY;
	symbolP->sy_desc |= N_NO_DEAD_STRIP;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_weak_reference() implements the pseudo op:
 *	.weak_reference name
 */
static
void
s_weak_reference(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(!flagseen['k'])
	    as_fatal("incompatible feature used: .weak_reference (must specify "
		     "\"-dynamic\" to be used)");

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	if((symbolP->sy_type & N_TYPE) == N_UNDF && symbolP->sy_value == 0)
	    symbolP->sy_desc |= N_WEAK_REF;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_weak_definition() implements the pseudo op:
 *	.weak_definition name
 */
static
void
s_weak_definition(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= N_WEAK_DEF;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_weak_def_can_be_hidden() implements the pseudo op:
 *	.weak_def_can_be_hidden name
 */
static
void
s_weak_def_can_be_hidden(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= (N_WEAK_DEF | N_WEAK_REF);
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_no_dead_strip() implements the pseudo op:
 *	.no_dead_strip name
 */
static
void
s_no_dead_strip(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= N_NO_DEAD_STRIP;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_symbol_resolver() implements the pseudo op:
 *	.symbol_resolver name
 */
static
void
s_symbol_resolver(
uintptr_t value)
{
    char *name;
    char c;
    char *p;
    symbolS *symbolP;

	if(* input_line_pointer == '"')
	    name = input_line_pointer + 1;
	else
	    name = input_line_pointer;
	c = get_symbol_end();
	p = input_line_pointer;

	*p = 0;
	symbolP = symbol_find_or_make(name);
	symbolP->sy_desc |= N_SYMBOL_RESOLVER;
	*p = c;
	demand_empty_rest_of_line();
}

/*
 * s_include() implements the pseudo op:
 *	.include "filename"
 */
static
void
s_include(
uintptr_t value)
{
	char *filename;
	int length;
	symbolS *symbolP;

	/* Some assemblers tolerate immediately following '"' */
	if((filename = demand_copy_string( & length ) )) {
	    demand_empty_rest_of_line();
	    read_an_include_file(filename);
	}

	/*
	 * This is to generate stabs for debugging assembly code.
	 * See the second comment about stabs in read_a_source_file()
	 * for a description of what is going on here
	 */
	if(flagseen['g'] && frchain_now->frch_nsect == text_nsect){
	    symbolP = symbol_new(
			    physical_input_file,
			    132 /* N_SOL */,
			    text_nsect,
			    0,
			    (int)(obstack_next_free(&frags) -
				  frag_now->fr_literal),
			    frag_now);
	}
}

/*
 * demand_empty_rest_of_line() checks to make sure we are at the end of a line
 * and if not ignores the rest of the line.
 * This is global so machine dependent pseudo-ops can use this.
 */
void
demand_empty_rest_of_line(
void)
{
	SKIP_WHITESPACE();
	if(is_end_of_line(*input_line_pointer))
	    input_line_pointer++;
	else
	    ignore_rest_of_line();
}

/* we simply ignore the rest of this statement */
void
s_ignore(
uintptr_t arg ATTRIBUTE_UNUSED)
{
  totally_ignore_line ();
}

/*
 * ignore_rest_of_line() advances input_line_pointer to the next line and if
 * there is anything left on the current line print a warning.
 * This is global so machine dependent pseudo-ops can use this.
 */
void
ignore_rest_of_line(
void)
{
	if(!is_end_of_line(*input_line_pointer)){
	    as_bad("Rest of line ignored. 1st junk character valued %d (%c).",
		    *input_line_pointer, *input_line_pointer);
	    while(input_line_pointer < buffer_limit &&
		  !is_end_of_line(*input_line_pointer))
		input_line_pointer++;
	}
	input_line_pointer++;	/* Return pointing just after end-of-line. */
	know(is_end_of_line(input_line_pointer[-1]));
}

/*
 *			stab()
 *
 * Handle .stabX directives, which used to be open-coded.
 * So much creeping featurism overloaded the semantics that we decided
 * to put all .stabX thinking in one place. Here.
 *
 * We try to make any .stabX directive legal. Other people's AS will often
 * do assembly-time consistency checks: eg assigning meaning to n_type bits
 * and "protecting" you from setting them to certain values. (They also zero
 * certain bits before emitting symbols. Tut tut.)
 *
 * If an expression is not absolute we either gripe or use the relocation
 * information. Other people's assemblers silently forget information they
 * don't need and invent information they need that you didn't supply.
 *
 * .stabX directives always make a symbol table entry. It may be junk if
 * the rest of your .stabX directive is malformed.
 */
static
void
stab(
uintptr_t what) /* d == .stabd, n == .stabn, and s == .stabs */
{
    symbolS *symbolP;
    char *string;
    int saved_type;
    int length;
    int goof;	/* TRUE if we have aborted. */
    int32_t longint;

	saved_type = 0;
	symbolP = NULL;
	/*
	 * Enter with input_line_pointer pointing past .stabX and any following
	 * whitespace.
	 */
	goof = FALSE;
	if(what == 's'){
	    string = demand_copy_C_string(&length);
	    SKIP_WHITESPACE();
	    if(*input_line_pointer == ',')
		input_line_pointer ++;
	    else{
		as_bad("I need a comma after symbol's name");
		goof = TRUE;
	    }
	}
	else
	    string = "";

	/*
	 * Input_line_pointer->after ','.  String -> symbol name.
	 */
	if(!goof){
	    symbolP = symbol_new(string, 0,0,0,0,(struct frag *)0);
	    switch(what){
	    case 'd':
		symbolP->sy_name = NULL; /* .stabd feature. */
		symbolP->sy_value = (int)(obstack_next_free(&frags) -
				    frag_now->fr_literal);
		symbolP->sy_frag = frag_now;
		break;

	    case 'n':
	    case 's':
		symbolP->sy_frag = &zero_address_frag;
		break;

	    default:
		BAD_CASE( (int)what );
		break;
	    }
	    if(get_absolute_expression_and_terminator(&longint) == ','){
		saved_type = longint;
		symbolP->sy_type = longint;
	    }
	    else{
		as_bad("I want a comma after the n_type expression");
		goof = TRUE;
		input_line_pointer--; /* Backup over a non-',' char. */
	    }
	}

	if(!goof){
	    if(get_absolute_expression_and_terminator(&longint) == ',')
		symbolP->sy_other = longint;
	    else {
		as_bad("I want a comma after the n_other expression");
		goof = TRUE;
		input_line_pointer--; /* Backup over a non-',' char. */
	    }
	}

	if(!goof){
	    symbolP->sy_desc = get_absolute_expression();
	    if(what == 's' || what == 'n'){
		if(*input_line_pointer != ','){
		    as_bad( "I want a comma after the n_desc expression" );
		    goof = TRUE;
		}
		else
		    input_line_pointer ++;
	    }
	}

	if((!goof) && (what=='s' || what=='n')){
	    pseudo_set(symbolP);
	    symbolP->sy_type = saved_type;
	}
	else if(!goof){
	    /* for stabd the sy_other (n_sect) gets set to the current section*/
	    symbolP->sy_other = frchain_now->frch_nsect;
	}
	if(goof)
	    ignore_rest_of_line();
	else
	    demand_empty_rest_of_line();
}

/*
 *			pseudo_set()
 *
 * In:	Pointer to a symbol.
 *	Input_line_pointer -> expression.
 *
 * Out:	Input_line_pointer -> just after any whitespace after expression.
 *	Tried to set symbol to value of expression.
 *	Will change sy_type, sy_value, sy_frag;
 *(old ->> May set need_pass_2 == TRUE. <<-- commented out by GNU below it
 * uses symbolP->sy_forward = exp.X_add_symbol;)
 */
void
pseudo_set(
symbolS *symbolP)
{
    expressionS exp;
    segT segment;
    int ext;

	know(symbolP);		/* NULL pointer is logic error. */
	ext = (symbolP->sy_type & (N_EXT | N_PEXT));
	segment = expression(&exp);

	switch(segment){
	case SEG_BIG:
	    as_bad("%s number illegal. Absolute 0 assumed.",
		    exp.X_add_number > 0 ? "Bignum" : "Floating-Point");
	    symbolP->sy_type = N_ABS | ext;
	    symbolP->sy_other = 0; /* NO_SECT */
	    symbolP->sy_value = 0;
	    symbolP->sy_frag = &zero_address_frag;
	    break;

	case SEG_NONE:
	    as_bad("No expression:  Using absolute 0");
	    symbolP->sy_type = N_ABS | ext;
	    symbolP->sy_other = 0; /* NO_SECT */
	    symbolP->sy_value = 0;
	    symbolP->sy_frag = &zero_address_frag;
	    break;

	case SEG_DIFFSECT:
	    if(exp.X_add_symbol && exp.X_subtract_symbol){
		if(exp.X_add_symbol->sy_frag !=
		   exp.X_subtract_symbol->sy_frag ||
		   exp.X_add_symbol->sy_type == N_UNDF ||
		   exp.X_subtract_symbol->sy_type == N_UNDF ){
		    expressionS *expression;

		    expression = xmalloc(sizeof(expressionS));
		    *expression = exp;
		    symbolP->expression = expression;
		}
		else{
		    exp.X_add_number += exp.X_add_symbol->sy_value -
					exp.X_subtract_symbol->sy_value;
		}
	    }
	    else if(exp.X_add_symbol &&
	            exp.X_subtract_symbol == NULL &&
	            exp.X_add_symbol->expression != NULL){
		    expressionS *expression;

		    expression = xmalloc(sizeof(expressionS));
		    memcpy(expression, exp.X_add_symbol->expression,
			   sizeof(expressionS));
		    symbolP->expression = expression;
	    }
	    else
		as_bad("Complex expression. Absolute segment assumed." );
	    symbolP->sy_type = N_ABS | ext;
	    symbolP->sy_other = 0; /* NO_SECT */
	    symbolP->sy_value = (uint32_t)exp.X_add_number;
	    symbolP->sy_frag = &zero_address_frag;
	    break;

	case SEG_ABSOLUTE:
	    symbolP->sy_type = N_ABS | ext;
	    symbolP->sy_other = 0; /* NO_SECT */
	    symbolP->sy_value = (uint32_t)exp.X_add_number;
	    symbolP->sy_frag = &zero_address_frag;
	    symbolP->expression = NULL;
	    break;

	case SEG_SECT:
	    symbolP->sy_type  = N_SECT | ext;
	    symbolP->sy_other = exp.X_add_symbol->sy_other;
	    symbolP->sy_value = (uint32_t)exp.X_add_number + exp.X_add_symbol->sy_value;
	    symbolP->sy_frag  = exp.X_add_symbol->sy_frag;
	    break;
	  
	case SEG_UNKNOWN:
	    symbolP->sy_forward = exp.X_add_symbol;
/* commented out by GNU */
/* as_bad("unknown symbol"); */
/* need_pass_2 = TRUE; */
	    break;
	  
	default:
	    BAD_CASE(segment);
	    break;
	}
}

/*
 *			cons()
 *
 * CONStruct more frag of .bytes, or .words etc.
 * This understands EXPRESSIONS, as opposed to big_cons().
 *
 * Bug (?)
 *
 * This has a split personality. We use expression() to read the
 * value. We can detect if the value won't fit in a byte or word.
 * But we can't detect if expression() discarded significant digits
 * in the case of a long. Not worth the crocks required to fix it.
 *
 * Worker function to do .byte, .short, .long, .quad statements.
 * This clobbers input_line_pointer, checks end-of-line.
 */
void
cons(	
uintptr_t nbytes) /* nbytes == 1 for .byte, 2 for .word, 4 for .long, 8 for .quad */
{
    char c;
    signed_expr_t
    mask,		/* high-order bits to truncate */
    unmask,		/* what bits we will store */
    get,		/* the bits of the expression we get */
    use;		/* the bits of the expression after truncation */
    char *p;		/* points into the frag */
    segT segment;
    expressionS exp;
#ifndef TC_CONS_FIX_NEW
    fixS *fixP;
#endif

	memset(&exp, '\0', sizeof(exp));
	/*
	 * Input_line_pointer -> 1st char after pseudo-op-code and could legally
	 * be a end-of-line. (Or, less legally an eof - which we cope with.)
	 */
	if(nbytes >= (int)sizeof(signed_expr_t))
	    mask = 0;
	else 
	    /* Don't store these bits. */
	    mask = ~0ULL << (BITS_PER_CHAR * nbytes);
	unmask = ~mask;		/* Do store these bits. */

	/*
	 * The following awkward logic is to parse ZERO or more expressions,
	 * comma seperated. Recall an expression includes its leading &
	 * trailing blanks. We fake a leading ',' if there is (supposed to
	 * be) a 1st expression, and keep demanding 1 expression for each ','.
	 */
	if(is_it_end_of_statement()){
	    c = 0;			/* Skip loop. */
	    input_line_pointer++;	/* Matches end-of-loop 'correction'. */
	}
	else
	    c = ',';			/* Do loop. */

	while(c == ','){
#ifdef TC_PARSE_CONS_EXPRESSION
	    segment = TC_PARSE_CONS_EXPRESSION(&exp, (int)nbytes);
#else
	    segment = expression(&exp); /* At least scan over the expression */
#endif

	    if(segment == SEG_DIFFSECT && exp.X_add_symbol == NULL){
		as_bad("Subtracting symbol \"%s\"(segment\"%s\") is too "
			"hard. Absolute segment assumed.",
			exp.X_subtract_symbol->sy_name,
			seg_name[(int)N_TYPE_seg[
			    exp.X_subtract_symbol->sy_type & N_TYPE]]);
		segment = SEG_ABSOLUTE;
		/* Leave exp .X_add_number alone. */
	    }
	    p = frag_more((int)nbytes);
	    switch(segment){
	    case SEG_BIG:
		/*
		 * Handle bignums small enough to fit in a long long and
		 * thus be passed directly to md_number_to_chars.
		 */
		if(exp.X_add_number > 0 &&
		   (((LITTLENUM_NUMBER_OF_BITS * exp.X_add_number) / 8) <=
		   sizeof(int64_t))){
		    int i;
		    int64_t sum;

		    sum = 0;
		    for(i = 0; i < exp.X_add_number; ++i)
			sum = (sum << LITTLENUM_NUMBER_OF_BITS) +
			      generic_bignum[(exp.X_add_number - 1) - i];
		    md_number_to_chars(p, sum, (int)nbytes);
		}
		else
		{
		    as_bad("%s number illegal. Absolute 0 assumed.",
			    exp.X_add_number > 0 ? "Bignum" : "Floating-Point");
		    md_number_to_chars(p, (int32_t)0, (int)nbytes);
	        }
		break;

	    case SEG_NONE:
		as_bad("0 assumed for missing expression");
		exp.X_add_number = 0;
		know(exp.X_add_symbol == NULL);
		/* fall into SEG_ABSOLUTE */

	    case SEG_ABSOLUTE:
		get = exp.X_add_number;
		use = get & unmask;
		if((get & mask) && (get & mask) != mask){
		    /* Leading bits contain both 0s & 1s. */
		    as_bad("Value 0x%llx truncated to 0x%llx.", get, use);
		}
  		dwarf2_emit_insn((int)nbytes);
		/* put bytes in right order. */
		md_number_to_chars(p, use, (int)nbytes);
		break;

	    case SEG_DIFFSECT:
	    case SEG_UNKNOWN:
	    case SEG_SECT:
#ifdef TC_CONS_FIX_NEW
		TC_CONS_FIX_NEW(frag_now,
		    (unsigned int)(p - frag_now->fr_literal),
		    (unsigned int)nbytes,
		    &exp);
#else
		fixP = fix_new(frag_now,
			(int)(p - frag_now->fr_literal),
			(int)nbytes,
			exp.X_add_symbol,
			exp.X_subtract_symbol,
			(int)exp.X_add_number,
			0,
			0,
			0);
		/*
		 * If we have the special assembly time constant expression
		 * of the difference of two symbols defined in the same section 
		 * then divided by exactly 2 mark the fix to indicate this.
		 */
		fixP->fx_sectdiff_divide_by_two = exp.X_sectdiff_divide_by_two;
#endif
		break;

	    default:
		BAD_CASE(segment);
		break;
	    }			/* switch(segment) */
	    c = *input_line_pointer++;
	}				/* while(c==',') */
	input_line_pointer--;	/* Put terminator back into stream. */
	demand_empty_rest_of_line();
}

#ifdef M68K /* we allow big cons only on the 68k machines */
/*
 *			big_cons()
 *
 * CONStruct more frag(s) of .quads, or .octa etc.
 * Makes 0 or more new frags.
 * This understands only bignums, not expressions. Cons() understands
 * expressions.
 *
 * Constants recognised are '0...'(octal) '0x...'(hex) '...'(decimal).
 *
 * This creates objects with struct obstack_control objs, destroying
 * any context objs held about a partially completed object. Beware!
 *
 *
 * I think it sucks to have 2 different types of integers, with 2
 * routines to read them, store them etc.
 * It would be nicer to permit bignums in expressions and only
 * complain if the result overflowed. However, due to "efficiency"...
 *
 * Worker function to do .quad and .octa statements.
 * This clobbers input_line_pointer, checks end-of-line.
 */
void
big_cons(
uintptr_t nbytes) /* 8 == .quad, 16 == .octa ... */
{
    char c;	/* input_line_pointer -> c. */
    int radix;
    int32_t length;/* Number of chars in an object. */
    int digit;	/* Value of 1 digit. */
    int carry;	/* For multi-precision arithmetic. */
    int work;	/* For multi-precision arithmetic. */
    char *p,*q;	/* For multi-precision arithmetic. */
    int i;

	/*
	 * The following awkward logic is to parse ZERO or more strings,
	 * comma seperated. Recall an expression includes its leading &
	 * trailing blanks. We fake a leading ',' if there is (supposed to
	 * be) a 1st expression, and keep demanding 1 expression for each ','.
	 */
	if(is_it_end_of_statement()){
	    c = 0;			/* Skip loop. */
	}
	else{
	    c = ',';			/* Do loop. */
	    --input_line_pointer;
	}
	while(c == ','){
	    ++input_line_pointer;
	    SKIP_WHITESPACE();
	    c = *input_line_pointer;
	    /* c contains 1st non-blank char of what we hope is a number */
	    if(c == '0'){
		c = *++input_line_pointer;
		if(c == 'x' || c=='X'){
		    c = *++input_line_pointer;
		    radix = 16;
		}
		else{
		    radix = 8;
		}
	    }
	    else{
		radix = 10;
	    }
	    /*
	     * This feature (?) is here to stop people worrying about
	     * mysterious zero constants: which is what they get when
	     * they completely omit digits.
	     */
	    if(hex_value[(int)c] >= radix){
		as_bad("Missing digits. 0 assumed.");
	    }
	    bignum_high = bignum_low - 1; /* Start constant with 0 chars. */
	    for( ;
		(digit = hex_value[(int)c]) < radix;
		c = *++input_line_pointer){
		/* Multiply existing number by radix, then add digit. */
		carry = digit;
		for(p = bignum_low; p <= bignum_high; p++){
		    work = (*p & MASK_CHAR) * radix + carry;
		    *p = work & MASK_CHAR;
		    carry = work >> BITS_PER_CHAR;
		}
		if(carry){
		    grow_bignum();
		    *bignum_high = carry & MASK_CHAR;
		    know((carry & ~ MASK_CHAR) == 0);
		}
	    }
	    length = bignum_high - bignum_low + 1;
	    if(length > nbytes){
		as_bad("Most significant bits truncated in integer constant.");
	    }
	    else{
		int32_t leading_zeroes;

		for(leading_zeroes = nbytes - length;
		    leading_zeroes;
		    leading_zeroes--){
		    grow_bignum();
		    *bignum_high = 0;
		}
	    }
	    p = frag_more(nbytes);
	    if(md_target_byte_sex == BIG_ENDIAN_BYTE_SEX){
		q = (char *)bignum_low;
		for(i = nbytes - 1; i >= 0; i--)
		    *p++ = q[i];
	    }
	    else{
		memcpy(p, bignum_low, (int)nbytes);
	    }
	    /* C contains character after number. */
	    SKIP_WHITESPACE();
	    c = *input_line_pointer;
	    /* C contains 1st non-blank character after number. */
	}
	demand_empty_rest_of_line();
}

/*
 * grow_bignum() extends bignum (that is adjust bignum_low, bignum_high and
 * bignum_limit).
 */
static
void
grow_bignum(
void)
{
    int32_t length;

	bignum_high++;
	if(bignum_high >= bignum_limit)
	{
	    length = bignum_limit - bignum_low;
	    bignum_low = xrealloc(bignum_low, length + length);
	    bignum_high = bignum_low + length;
	    bignum_limit = bignum_low + length + length;
	}
}
#endif /* M68K we allow big cons only on the 68k machines */

/*
 *			float_cons()
 *
 * CONStruct some more frag chars of .floats .ffloats etc.
 * Makes 0 or more new frags.
 * This understands only floating literals, not expressions. Sorry.
 *
 * A floating constant is defined by atof_generic(), except it is preceded
 * by 0d 0f 0g or 0h. After observing the STRANGE way my BSD AS does its
 * reading, I decided to be incompatible. This always tries to give you
 * rounded bits to the precision of the pseudo-op. Former AS did premature
 * truncatation, restored noisy bits instead of trailing 0s AND gave you
 * a choice of 2 flavours of noise according to which of 2 floating-point
 * scanners you directed AS to use.
 *
 * In:	input_line_pointer -> whitespace before, or '0' of flonum.
 *
 * Worker function to do .double, .float, .single statements.
 * This clobbers input_line-pointer, checks end-of-line.
 */
void
float_cons(
uintptr_t float_type) /* 'f':.ffloat ... 'F':.float ... */
{
    char *p;
    char c;
    int length;	/* Number of chars in an object. */
    char *err;	/* Error from scanning floating literal. */
    char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];

	/*
	 * The following awkward logic is to parse ZERO or more strings,
	 * comma seperated. Recall an expression includes its leading &
	 * trailing blanks. We fake a leading ',' if there is (supposed to
	 * be) a 1st expression, and keep demanding 1 expression for each ','.
	 */
	if(is_it_end_of_statement()){
	    c = 0;			/* Skip loop. */
	    ++input_line_pointer;	/* -> past termintor. */
	}
	else{
	    c = ',';			/* Do loop. */
	}
	while(c == ','){
	    /* input_line_pointer -> 1st char of a flonum (we hope!). */
	    SKIP_WHITESPACE();
	    /*
	     * Skip any 0{letter} that may be present. Don't even check if the
	     * letter is legal. Someone may invent a "z" format and this routine
	     * has no use for such information. Lusers beware: you get
	     * diagnostics if your input is ill-conditioned.
	     */
	    if(input_line_pointer[0] == '0' && isalpha(input_line_pointer[1]))
		input_line_pointer+=2;

	    err = md_atof((int)float_type, temp, &length);
	    know(length <=  MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT);
	    know(length > 0);
	    if(err != NULL && *err != '\0'){
		as_bad( "Bad floating literal: %s", err);
		ignore_rest_of_line();
		/* Input_line_pointer -> just after end-of-line. */
		c = 0;		/* Break out of loop. */
	    }
	    else{
		p = frag_more(length);
		memcpy(p, temp, length);
		SKIP_WHITESPACE();
		c = *input_line_pointer ++;
		/* C contains 1st non-white character after number. */
		/* input_line_pointer -> just after terminator (c). */
	    }
	}
	--input_line_pointer;		/* -> terminator (is not ','). */
	demand_empty_rest_of_line();
}

static void
emit_leb128_expr (expressionS *exp, int sign)
{
  segT op = exp->X_op;
#ifdef notyet
  unsigned int nbytes;
#endif

  if (op == O_absent)
    {
      as_warn (_("zero assumed for missing expression"));
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_big && exp->X_add_number <= 0)
    {
      as_bad (_("floating point number invalid"));
      exp->X_add_number = 0;
      op = O_constant;
    }
#ifdef notyet
  else if (op == O_register)
    {
      as_warn (_("register value used as expression"));
      op = O_constant;
    }
  else if (op == O_constant
	   && sign
	   && (exp->X_add_number < 0) != !exp->X_unsigned)
    {
      /* We're outputting a signed leb128 and the sign of X_add_number
	 doesn't reflect the sign of the original value.  Convert EXP
	 to a correctly-extended bignum instead.  */
      convert_to_bignum (exp);
      op = O_big;
    }

  /* Let check_eh_frame know that data is being emitted.  nbytes == -1 is
     a signal that this is leb128 data.  It shouldn't optimize this away.  */
  nbytes = (unsigned int) -1;
  if (check_eh_frame (exp, &nbytes))
    abort ();

  /* Let the backend know that subsequent data may be byte aligned.  */
#ifdef md_cons_align
  md_cons_align (1);
#endif
#endif /* notyet */

  if (op == O_constant)
    {
      /* If we've got a constant, emit the thing directly right now.  */

      valueT value = (valueT)exp->X_add_number;
      int size;
      char *p;

      size = sizeof_leb128 (value, sign);
      p = frag_more (size);
      output_leb128 (p, value, sign);
    }
#ifdef notyet
  else if (op == O_big)
    {
      /* O_big is a different sort of constant.  */

      int size;
      char *p;

      size = output_big_leb128 (NULL, generic_bignum, exp->X_add_number, sign);
      p = frag_more (size);
      output_big_leb128 (p, generic_bignum, exp->X_add_number, sign);
    }
#endif /* notyet */
  else
    {
      /* Otherwise, we have to create a variable sized fragment and
	 resolve things later.  */

#ifdef OLD
      frag_var (rs_leb128, sizeof_uleb128 (~(valueT) 0), 0, sign,
		make_expr_symbol (exp), 0, (char *) NULL);
#else
      symbolS *sym;
      expressionS *expression;
  
      sym = symbol_temp_new(exp->X_add_symbol->sy_other /* GUESS */, 0, NULL);
      expression = xmalloc(sizeof(expressionS));
      *expression = *exp;
      sym->expression = expression;
      sym->sy_frag = &zero_address_frag;
      frag_var (rs_leb128, sizeof_leb128 ( ((valueT) (~(valueT) 0) >> 1), 0),
		0, sign, sym, 0, (char *) NULL);
      frchain_now->has_rs_leb128s = TRUE;
#endif

    }
}

/* Parse the .sleb128 and .uleb128 pseudos.  */

static
void
s_leb128(
uintptr_t sign)
{
  expressionS exp;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  do
    {
      expression (&exp);
      emit_leb128_expr (&exp, (int)sign);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}

/*
 *			stringer()
 *
 * We read 0 or more ',' seperated, double-quoted strings.
 *
 * Worker function to do .ascii etc statements.
 * Checks end-of-line.
 */
void
stringer(
uintptr_t append_zero) /* 0: don't append '\0', else 1 */
{
    int c;

	/*
	 * The following awkward logic is to parse ZERO or more strings,
	 * comma seperated. Recall a string expression includes spaces
	 * before the opening '\"' and spaces after the closing '\"'.
	 * We fake a leading ',' if there is (supposed to be)
	 * a 1st, expression. We keep demanding expressions for each
	 * ','.
	 */
	if(is_it_end_of_statement()){
	    c = 0;			/* Skip loop. */
	    ++ input_line_pointer;	/* Compensate for end of loop. */
	}
	else{
	    c = ',';			/* Do loop. */
	}
	for( ; c == ',';  c = *input_line_pointer++){
	    SKIP_WHITESPACE();
	    if(*input_line_pointer == '\"'){
		++input_line_pointer; /* -> 1st char of string. */
		while((c = next_char_of_string()) >= 0){
		    FRAG_APPEND_1_CHAR(c);
		}
		if(append_zero){
		    FRAG_APPEND_1_CHAR(0);
		}
		know(input_line_pointer[-1] == '\"');
	    }
	    else{
		as_bad("Expected \"-ed string");
	    }
	    SKIP_WHITESPACE();
	}
	--input_line_pointer;
	demand_empty_rest_of_line();
}

/*
 * next_char_of_string() is used by stringer() and demand_copy_string() and
 * returns the next character from input_line_pointer that is in the string or 
 * -1 for the trailing " character.  This routine handles escaped characters
 * like \b, \f, etc.
 */
static
int
next_char_of_string(
void)
{
    int c;
    int32_t number, i;

	c = *input_line_pointer++;
	/* make sure the 0xff char is not returned as -1 */
	c = (c & MASK_CHAR);
	switch(c){
	case '\"':
#ifdef PPC
	    if(flagseen[(int)'p'] == TRUE)
		break;
#endif /* PPC */
	    c = -1;
	    break;

#ifdef PPC
	case '\'':
	    if(flagseen[(int)'p'] == TRUE)
		c = -1;
	    break;
#endif /* PPC */

	case '\\':
	    c = *input_line_pointer++;
	    switch(c){
	    case 'b':
		c = '\b';
		break;
	    case 'f':
		c = '\f';
		break;
	    case 'n':
		c = '\n';
		break;
	    case 'r':
		c = '\r';
		break;
	    case 't':
		c = '\t';
		break;
	    case '\\':
	    case '"':
	    case '\'':
		break;		/* As itself. */
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		for(i = 0, number = 0;
		    i < 3 && isdigit(c) && c < '8';
		    i++, c = *input_line_pointer++)
		    number = number * 8 + c - '0';
		c = number;
		--input_line_pointer;
		break;
	    case '\n':
		/* To be compatible with BSD 4.2 as: give the user a linefeed */
		c = '\n';
		break;

	    default:
		as_bad( "Bad escaped character in string, '?' assumed" );
		c = '?';
		break;
	    }
	    break;
	default:
	    break;
	}
	return(c);
}

/*
 * get_segmented_expression() is passed an expression to fill in and return that
 * is anything except a bignum or a missing expression.
 */
static
segT
get_segmented_expression(
expressionS *expP)
{
    segT retval;

	retval = expression(expP);
	if(retval == SEG_NONE || retval == SEG_BIG){
	    as_bad("Expected address expression: absolute 0 assumed");
	    retval = expP->X_seg = SEG_ABSOLUTE;
	    expP->X_add_number   = 0;
	    expP->X_add_symbol   = NULL;
	    expP->X_subtract_symbol = NULL;
	}
	return(retval);		/* SEG_ ABSOLUTE,UNKNOWN,SECT */
}

/*
 * get_known_segmented_expression() is passed an expression to fill in and
 * return that is anything except an unknown, bignum or a missing expression.
 */
segT
get_known_segmented_expression(
expressionS *expP)
{
    segT retval;
    char *name1;
    char *name2;

	retval = get_segmented_expression(expP);
	if(retval == SEG_UNKNOWN){
	    name1 = expP->X_add_symbol ?
		    expP->X_add_symbol->sy_name : "";
	    name2 = expP->X_subtract_symbol ?
		    expP->X_subtract_symbol->sy_name : "";
	    if(name1 && name2){
		as_bad("Symbols \"%s\" \"%s\" are undefined: absolute 0 "
			"assumed.", name1, name2);
	    }
	    else{
		as_bad("Symbol \"%s\" undefined: absolute 0 assumed.",
			name1 ? name1 : name2);
	    }
	    retval      = SEG_ABSOLUTE;
	    expP->X_seg = SEG_ABSOLUTE;
	    expP->X_add_number = 0;
	    expP->X_add_symbol      = NULL;
	    expP->X_subtract_symbol = NULL;
	}
	know(retval == SEG_ABSOLUTE ||
	     retval == SEG_SECT ||
	     retval == SEG_DIFFSECT);
	return(retval);
}

/*
 * get_absolute_expression() gets an absolute expression and returns the value
 * of that expression.
 */
signed_target_addr_t
get_absolute_expression(
void)
{
    expressionS exp;
    segT s;

	s = expression(&exp);
	if(s != SEG_ABSOLUTE){
/* is this right? if not absolute: no message and return 0 */
	    if(s != SEG_NONE){
		as_bad("Bad Absolute Expression, absolute 0 assumed.");
	    }
	    exp.X_add_number = 0;
	}
	return (signed_target_addr_t)exp.X_add_number;
}

/*
 * get_absolute_expression_and_terminator() gets an absolute expression and
 * returning the value of that expression indirectly through val_pointer and
 * returns the terminator.
 */
static
char			/* return terminator */
get_absolute_expression_and_terminator(
int32_t *val_pointer)	/* return value of expression */
{
    *val_pointer = (int32_t)get_absolute_expression();
    return(*input_line_pointer++);
}

/*
 *			demand_copy_C_string()
 *
 * Like demand_copy_string, but return NULL if the string contains any '\0's.
 * Give a warning if that happens.
 */
char *
demand_copy_C_string(
int *len_pointer)
{
    char *s;
    int len;

	if((s = demand_copy_string(len_pointer))){
	    for(len = *len_pointer; len > 0; len--){
		if(*s == '\0'){
		    s = 0;
		    len = 1;
		    *len_pointer = 0;
		    as_bad("This string may not contain \'\\0\'");
		}
	    }
	}
	return(s);
}

/*
 *			demand_copy_string()
 *
 * Demand string, but return a safe (=private) copy of the string.
 * Return NULL if we can't read a string here.
 */
static
char *
demand_copy_string(
int *lenP)
{
    int c;
    int len;
    char *retval;

	len = 0;
	SKIP_WHITESPACE();
#ifdef PPC
	if((flagseen[(int)'p'] == TRUE  && *input_line_pointer == '\'') ||
	   (flagseen[(int)'p'] == FALSE && *input_line_pointer == '\"'))
#else
	if(*input_line_pointer == '\"')
#endif
	{
	    input_line_pointer++;	/* Skip opening quote. */
	    while((c = next_char_of_string()) >= 0){
		(void)(obstack_1grow(&notes, c));
		len++;
	    }
	    /*
	     * This next line is so demand_copy_C_string will return a null
	     * termanated string.
	     */
	    (void)(obstack_1grow(&notes, '\0'));
	    retval = obstack_finish(&notes);
	}
	else{
	    as_bad("Missing string");
	    retval = NULL;
	    ignore_rest_of_line();
	}
	*lenP = len;
	return(retval);
}

/*
 *		is_it_end_of_statement()
 *
 * In:	Input_line_pointer -> next character.
 *
 * Do:	Skip input_line_pointer over all whitespace.
 *
 * Out:	TRUE if input_line_pointer -> end-of-line.
 */
static
int
is_it_end_of_statement(
void)
{
	SKIP_WHITESPACE();
	return(is_end_of_line(*input_line_pointer));
}

/*
 * equals() implements the assembly statement:
 *	 x = expression
 */
static
void
equals(
char *sym_name)
{
    struct symbol *symbolP;
    segT segment;
    expressionS exp;
    char *p;

	/* Turn '. = mumble' into a .org mumble */
	if(sym_name[0]=='.' && sym_name[1]=='\0'){
	    if(input_line_pointer[1] == '=')
		input_line_pointer += 2;
	    else
		*input_line_pointer++ = '=';		/* Put it back */
	    if(*input_line_pointer==' ' || *input_line_pointer=='\t')
		input_line_pointer++;
	    segment = get_known_segmented_expression(&exp);
	    if((segment != SEG_SECT ||
		exp.X_add_symbol->sy_other != frchain_now->frch_nsect) &&
		segment != SEG_ABSOLUTE)
	    as_bad("Illegal expression. current section assumed.");
	    p = frag_var(rs_org,
			 1,
			 1,
			 (relax_substateT)0,
			 exp.X_add_symbol,
			 (int32_t)exp.X_add_number,
			 (char *)0);
	    *p = 0;
	    return;
	}

	symbolP = symbol_find_or_make(sym_name);
	if(symbolP->sy_type & N_ABS)
	    symbolP->sy_desc |= N_NO_DEAD_STRIP;
	if(input_line_pointer[1] == '=')
	    input_line_pointer += 2;
	else
	    *input_line_pointer++ = '=';		/* Put it back */
	if(*input_line_pointer==' ' || *input_line_pointer=='\t')
	    input_line_pointer++;
	pseudo_set(symbolP);
}

/*
 * s_if() implements the pseudo op:
 *	.if expression
 * that does conditional assembly using assembler defined expressions.
 */
static
void
s_if(
uintptr_t value)
{
	if(if_depth >= MAX_IF_DEPTH)
	    as_fatal("You can't nest if's more than %d levels deep",
		     MAX_IF_DEPTH);
	last_states[if_depth++] = the_cond_state;
	the_cond_state.the_cond = if_cond;
	if(the_cond_state.ignore)
	    totally_ignore_line();
	else{
	    the_cond_state.cond_met = (int)get_absolute_expression();
	    the_cond_state.ignore = !the_cond_state.cond_met;
	    demand_empty_rest_of_line();
	}
}

/*
 * s_elseif() implements the pseudo op:
 *	.elseif expression
 * that does conditional assembly using assembler defined expressions.
 */
static
void
s_elseif(
uintptr_t value)
{
    int last_ignore_state;

	if(the_cond_state.the_cond != if_cond &&
	   the_cond_state.the_cond != elseif_cond)
	    as_fatal("Encountered a .elseif that doesn't follow a .if or an "
		     ".elseif");
	the_cond_state.the_cond = elseif_cond;

	last_ignore_state = FALSE;
	if(if_depth)
	    last_ignore_state = last_states[if_depth-1].ignore;
        if(last_ignore_state || the_cond_state.cond_met){
	    the_cond_state.ignore = TRUE;
	    totally_ignore_line();
	}
	else{
	    the_cond_state.cond_met = (int)get_absolute_expression();
	    the_cond_state.ignore = !the_cond_state.cond_met;
	    demand_empty_rest_of_line();
	}
}

/*
 * s_else() implements the pseudo op:
 *	.else
 * that does conditional assembly using assembler defined expressions.
 */
static
void
s_else(
uintptr_t value)
{
    int last_ignore_state;

	if(the_cond_state.the_cond != if_cond &&
	   the_cond_state.the_cond != elseif_cond)
	    as_fatal("Encountered a .else that doesn't follow a .if or an "
		     ".elseif");
	the_cond_state.the_cond = else_cond;
	last_ignore_state = FALSE;
	if(if_depth)
	    last_ignore_state = last_states[if_depth-1].ignore;
        if(last_ignore_state || the_cond_state.cond_met)
	    the_cond_state.ignore = TRUE;
	else
	    the_cond_state.ignore = FALSE;
	demand_empty_rest_of_line();
}

/*
 * s_endif() implements the pseudo op:
 *	.endif
 * that does conditional assembly using assembler defined expressions.
 */
static
void
s_endif(
uintptr_t value)
{
	if((the_cond_state.the_cond == no_cond) || (if_depth == 0))
	    as_fatal("Encountered a .endif that doesn't follow a .if or .else");
	the_cond_state = last_states[--if_depth];
	demand_empty_rest_of_line();
}

/* 
 * totally_ignore_line() ignores lines during conditional assembly.
 */
void
totally_ignore_line(
void)
{
	if(!is_end_of_line(*input_line_pointer)){
	    while(input_line_pointer < buffer_limit &&
		  !is_end_of_line(*input_line_pointer)){
		input_line_pointer ++;
	    }
	}
	input_line_pointer++;	/* Return pointing just after end-of-line. */
	know(is_end_of_line(input_line_pointer[-1]));
}

/*
 * s_macros_on() implements the pseudo op:
 *	.macros_on
 */
static
void
s_macros_on(
uintptr_t value)
{
	macros_on = TRUE;
	demand_empty_rest_of_line();
}

/*
 * s_macros_off() implements the pseudo op:
 *	.macros_off
 */
void
s_macros_off(
uintptr_t value)
{
	macros_on = FALSE;
	demand_empty_rest_of_line();
}

/*
 * s_macro() implements the pseudo op:
 *	.macro macro_name
 * that defines a macro.
 */
void
s_macro(
uintptr_t value)
{
    int c;
    pseudo_typeS *pop;

	if(macro_name)
	    as_bad("Can't define a macro inside another macro definition");
	else{
	    SKIP_WHITESPACE();
	    while(is_part_of_name(c = *input_line_pointer ++))
		(void)(obstack_1grow (&macros, c));
	    (void)(obstack_1grow(&macros, '\0'));
	    --input_line_pointer;
	    macro_name = obstack_finish(&macros);
	    if(macro_name == NULL)
		as_bad("Missing name of macro");
	    if(*macro_name == '.'){
		pop = (pseudo_typeS *)hash_find(po_hash, macro_name + 1);
		if(pop != NULL)
		    as_bad("Pseudo-op name: %s can't be a macro name",
			    macro_name);
	    }
	}
	totally_ignore_line();
}

/*
 * s_endmacro() implements the pseudo op:
 *	.endmacro
 * which is the end of a macro definition.
 */
void
s_endmacro(
uintptr_t value)
{
    const char *errorString;

	if(!macro_name){
	    as_bad ("This .endmacro does not match with a preceding .macro");
	    ignore_rest_of_line();
	}
	else{
	    (void)(obstack_1grow(&macros, '\0'));
	    errorString = hash_insert(ma_hash, macro_name,
				      obstack_finish(&macros));
	    if(errorString != NULL && *errorString)
		as_warn("The macro named \"%s\" is already defined",
			macro_name);
	    macro_name = NULL;
	}
}

/*
 * macro_begin() initializes macros.
 */
static
void
macro_begin(
void)
{
	ma_hash = hash_new();
	obstack_begin(&macros, 5000);
}

/*
 * add_to_macro_definition() is called after a .macro to store the contents of
 * a macro into the obstack.
 */
void
add_to_macro_definition(
char *char_pointer)
{
    char c;

	do{
	    c = *char_pointer ++;
	    know(c != '\0');
	    (void)(obstack_1grow(&macros, c));
	}while((c != ':') && !(is_end_of_line(c)));
	if(char_pointer > input_line_pointer)
	    input_line_pointer = char_pointer;
}

/*
 * expand_macro() is called to expand macros.
 */
static
void
expand_macro(
char *macro_contents)
{
    char *buffer;
    char c;
    int index, nargs;
    char *last_buffer_limit;
    int last_count_lines;
    char *last_input_line_pointer;
    char *arguments [10]; /* at most 10 arguments, each is substituted */

	if(macro_depth >= MAX_MACRO_DEPTH)
	   as_fatal("You can't nest macros more than %d levels deep",
		    MAX_MACRO_DEPTH);
	macro_depth++;

	/* copy each argument to a object in the macro obstack */
	nargs = 0;
	c = '\0';
	for(index = 0; index < 10; index ++){
	    if(*input_line_pointer == ' ')
		++input_line_pointer;
	    know(*input_line_pointer != ' ');
	    c = *input_line_pointer;
	    if(is_end_of_line(c))
		arguments[index] = NULL;
	    else{
		int parenthesis_depth = 0;
		do{
		    SKIP_WHITESPACE();
		    c = *input_line_pointer++;
		    if(parenthesis_depth){
			if(c == ')')
			    parenthesis_depth --;
		    }
		    else{
			if(c == '(')
			    parenthesis_depth ++;
			else
			    if(is_end_of_line(c) ||
			       (c == ' ') || (c == ','))
			    break;
		    }
		    know(c != '\0');
		    if(is_end_of_line(c))
			as_bad("mismatched parenthesis");
		    (void)(obstack_1grow(&macros, c));
		}while(1);
		(void)(obstack_1grow(&macros, '\0'));
		arguments[index] = obstack_finish(&macros);
		nargs++;
		if(is_end_of_line(c))
		    --input_line_pointer;
		else if(c == ' ')
		    if(*input_line_pointer == ',')
			input_line_pointer++;
	    }
	}
	if(!is_end_of_line(c)){
	    as_bad("More than 10 arguments not allowed for macros");
	    ignore_rest_of_line();
	}
	/*
	 * Build a buffer containing the macro contents with arguments
	 * substituted
	 */
	(void)(obstack_1grow(&macros, '\n'));
	while((c = *macro_contents++)){
	    if(c == '$'){
		if(*macro_contents == '$'){
		    macro_contents++;
		}
		else if((*macro_contents >= '0') && (*macro_contents <= '9')){
		    index = *macro_contents++ - '0';
		    last_input_line_pointer = macro_contents;
		    macro_contents = arguments[index];
		    if(macro_contents){
			while ((c = * macro_contents ++))
			(void)(obstack_1grow (&macros, c));
		    }
		    macro_contents = last_input_line_pointer;
		    continue;
		}
		else if (*macro_contents == 'n'){
		    macro_contents++ ;
		    (void)(obstack_1grow(&macros, nargs + '0'));
		    continue;
		}
	    }
	    (void)(obstack_1grow (&macros, c));
	}
	(void)(obstack_1grow (&macros, '\n'));
	(void)(obstack_1grow (&macros, '\0'));
	last_buffer_limit = buffer_limit;
	last_count_lines = count_lines;
	last_input_line_pointer = input_line_pointer;
	buffer_limit = obstack_next_free (&macros) - 1;
	buffer = obstack_finish (&macros);
	count_lines = FALSE;
	/*
	printf("expanded macro: %s", buffer + 1);
	*/
#ifdef PPC
	if(flagseen[(int)'p'] == TRUE)
	    ppcasm_parse_a_buffer(buffer + 1);
	else
#endif /* PPC */
	    parse_a_buffer(buffer + 1);
	obstack_free (&macros, buffer);
	for(index = 9; index >= 0; index --)
	    if(arguments[index])
		obstack_free(&macros, arguments[index]);
	buffer_limit = last_buffer_limit;
	count_lines = last_count_lines;
	input_line_pointer = last_input_line_pointer;
	macro_depth--;
}

/*
 * s_dump() implements the pseudo op:
 *	.dump filename
 * that does a quick binary dump of symbol tables.
 */
static
void
s_dump(
uintptr_t value)
{
    char *filename;
    int length;
    static char null_string[] = "";

	if((filename = demand_copy_string(&length))){
	    demand_empty_rest_of_line();
	    if((dump_fp = fopen(filename, "w+"))){
		hash_traverse(ma_hash, write_macro);
		fwrite(null_string, 1, 1, dump_fp);
		hash_traverse(sy_hash, write_symbol);
		fwrite(null_string, 1, 1, dump_fp);
		fclose(dump_fp);
	    }
	    else
		as_bad("couldn't write to dump file: \"%s\"", filename);
	}
}

/*
 * write_macro() used by hash_traverse indirectly through s_dump() to write one
 * macro.
 */
static
void
write_macro(
const char *string,
PTR value1)
{
        char *value = value1;
	know(string);
	know(value);
	know(strlen(string));
	fwrite(string, (strlen(string) + 1), 1, dump_fp);
	fwrite(value, (strlen(value) + 1), 1, dump_fp);
}

/*
 * write_symbol() used by hash_traverse indirectly through s_dump() to write one
 * N_ABS symbol and its value.
 */
static
void
write_symbol(
const char *string,
PTR value)
{
    symbolS *symbolP;

    	symbolP = (symbolS *)value;
	know(symbolP);
	if(((symbolP->sy_type) & N_TYPE) == N_ABS){
	    know(string);
	    know(strlen(string));
	    fwrite(string, (strlen(string) + 1), 1, dump_fp);
	    fwrite(&(symbolP -> sy_value), 4, 1, dump_fp);
	}
}

/*
 * s_load() implements the pseudo op:
 *	.load filename
 * that does a quick binary load of symbol tables.
 */
static
void
s_load(
uintptr_t value)
{
    char *char_pointer;
    char *filename;
    int length;
    char the_char;
    symbolS	*the_symbol;
    symbolS	*temp_symbol_lastP;
    static symbolS *dump_symbol_lastP;

	if((filename = demand_copy_string(&length))){
	    demand_empty_rest_of_line();
	    if((dump_fp = fopen(filename, "r+"))){
		do{
		    do{
			the_char = getc_unlocked(dump_fp);
			(void)(obstack_1grow(&macros, the_char));
		    }while(the_char);
		    char_pointer = obstack_finish (&macros);
		    if(!(*char_pointer))
			break;
		    do{
			the_char = getc_unlocked(dump_fp);
			(void)(obstack_1grow(&macros, the_char));
		    }while(the_char);
		    if(hash_insert(ma_hash, char_pointer,
				   obstack_finish(&macros)))
			as_bad("a macro named \"%s\" encountered in a .load "
			        "is already defined", char_pointer);
		}while(1);
	        /*
		 * We don't want to link in symbols that were loaded so they
		 * don't go out in the object file.  Instead these symbols
		 * should go out in the object file that did the .dump .
		 */
		temp_symbol_lastP = symbol_lastP;
		symbol_lastP = dump_symbol_lastP;
		do{
		    do{
			the_char = getc_unlocked(dump_fp);
			(void)(obstack_1grow(&macros, the_char));
		    }while(the_char);
		    char_pointer = obstack_base(&macros);
		    obstack_next_free(&macros) = char_pointer;
		    if(!(*char_pointer))
			break;
		    the_symbol = symbol_find_or_make(char_pointer);
		    the_symbol->sy_type = N_ABS;
		    char_pointer = (char *)&the_symbol->sy_value;
		    *char_pointer++ = getc_unlocked(dump_fp);
		    *char_pointer++ = getc_unlocked(dump_fp);
		    *char_pointer++ = getc_unlocked(dump_fp);
		    *char_pointer++ = getc_unlocked(dump_fp);
		    the_symbol->sy_frag = &zero_address_frag;
		}while(1);
		dump_symbol_lastP = symbol_lastP;
		symbol_lastP = temp_symbol_lastP;
		fclose(dump_fp);
	    }
	    else
		as_fatal("Couldn't find the dump file: \"%s\"", filename);
	}
}

/*
 * s_subsections_via_symbols() implements the pseudo op:
 *	.subsections_via_symbols
 * which will cause the MH_SUBSECTIONS_VIA_SYMBOLS flag to be set in the output
 * file.  This indicates to the static linker it is safe to divide up the
 * sections into sub-sections via symbols for dead code stripping.
 */
static
void
s_subsections_via_symbols(
uintptr_t value)
{
	demand_empty_rest_of_line();
	subsections_via_symbols = TRUE;
}

/*
 * s_machine() implements the pseudo op:
 *	.machine <arch_name>
 * where <arch_name> is allowed to be the same strings as the argument to the
 * command line argument -arch <arch_name> .
 */
static
void
s_machine(
uintptr_t value)
{
    char *arch_name, c;
    struct arch_flag arch_flag;
    cpu_subtype_t new_cpusubtype;
    const struct arch_flag *family_arch_flag;

	arch_name = input_line_pointer;
	/*
	 * Can't call get_symbol_end() here as some arch names have '-' in them.
	 */
	do{
	    c = *input_line_pointer++ ;
	}while(c != '\0' && c != '\n' && c != '\t' && c != ' ');
	*--input_line_pointer = 0;

	if(force_cpusubtype_ALL == FALSE){
	    family_arch_flag = NULL;
	    if(strcmp(arch_name, "all") == 0){
		family_arch_flag = get_arch_family_from_cputype(md_cputype);
		if(family_arch_flag != NULL)
		    arch_flag = *family_arch_flag;
	    }
	    if(family_arch_flag == NULL &&
	       get_arch_from_flag(arch_name, &arch_flag) == 0){
		as_bad("unknown .machine argument: %s", arch_name);
		return;
	    }
	    if(arch_flag.cputype != md_cputype){
		as_bad("invalid .machine argument: %s", arch_name);
	    }
	    else{
		new_cpusubtype = cpusubtype_combine(md_cputype,
						    md_cpusubtype,
						    arch_flag.cpusubtype);
		if(new_cpusubtype == -1){
		    as_bad(".machine argument: %s can not be combined "
			    "with previous .machine directives, -arch "
			    "arguments or machine specific instructions",
			    arch_name);
		}
		else{
		    archflag_cpusubtype = new_cpusubtype;
		}
	    }
	}

	*input_line_pointer = c;
	demand_empty_rest_of_line();
}

/*
 * s_secure_log_reset() implements the pseudo op:
 *	.secure_log_reset
 * .secure_log_reset takes no parameters, and resets the "unique" counter. As
 * it is an error if a .s_secure_log_unique directive is seen twice without
 * and .secure_log_reset appearing between them.
 */
static
void
s_secure_log_reset(
uintptr_t value)
{
	s_secure_log_used = FALSE;
	demand_empty_rest_of_line();
}

/*
 * s_secure_log_unique() implements the pseudo op:
 *	.s_secure_log_unique log_msg
 * This opens the file given by the environment varable AS_SECURE_LOG_FILE, and 
 * appends the current filename, line number, and the text given as the log_msg
 * in the directive.  If this is present, but AS_SECURE_LOG_FILE is not set,
 * an error message is generated.   If this appears twice without
 * .secure_log_reset appearing between them, an error message is generated.
 */
static
void
s_secure_log_unique(
uintptr_t value)
{
    FILE *secure_log_fp;
    char *log_msg, c;

	if(s_secure_log_used != FALSE)
	    as_fatal(".secure_log_unique specified multiple times");

	if(secure_log_file == FALSE)
	    as_fatal(".secure_log_unique used but AS_SECURE_LOG_FILE "
		     "environment variable unset.");

	log_msg = input_line_pointer;
	do{
	    c = *input_line_pointer++;
	} while(is_end_of_line(c) == FALSE);
	*--input_line_pointer = 0;

	if((secure_log_fp = fopen(secure_log_file, "a+"))){
	    char *file;
	    unsigned int line;

	    as_file_and_line(&file, &line);
		fprintf(secure_log_fp, "%s:%d:%s\n",
			(file != NULL) ? file : "unknown",
			line, log_msg);

	    fclose(secure_log_fp);
	}
	else
	    as_fatal("couldn't write to secure log file: \"%s\"",
		     secure_log_file);

	s_secure_log_used = TRUE;

	*input_line_pointer = c;
	demand_empty_rest_of_line();
}

/*
 * When inlineasm_checks is non-zero, then these variable are set and used
 * when reporting errors for the properties of GCC function-scope inline asms.
 */
int inlineasm_checks = 0;
char *inlineasm_file_name = NULL;
int inlineasm_line_number = 0;
int inlineasm_column_number = 0;

/*
 * s_inlineasm() handles the pseudo ops:
 *	.inlineasmstart [[["file_name"] [,<line_number>]] [,<column_number>]]
 *	.inlineasmend
 * The parameter value is 1 for start and 0 for end.  The arguments to the
 * start directive are optional.
 *
 * This causes the assembler enforces properties required of GCC function-scope
 * inline asms.
 *
 * The requirement that does not allow non-numeric labels to be defined in an
 * inline asm is checked for in colon().
 */
static
void
s_inlineasm(
uintptr_t value)
{
    int length;

	inlineasm_checks = (int)value;
	inlineasm_file_name = NULL;
	inlineasm_line_number = 0;
	inlineasm_column_number = 0;

	SKIP_WHITESPACE();
	if(value == 1 && *input_line_pointer == '"'){
	    if((inlineasm_file_name = demand_copy_string(&length))){
		SKIP_WHITESPACE();
		if(*input_line_pointer == ','){
		    input_line_pointer++;
		    inlineasm_line_number = (int)get_absolute_expression();
		    SKIP_WHITESPACE();
		    if(*input_line_pointer == ','){
			input_line_pointer++;
			inlineasm_column_number =(int)get_absolute_expression();
		    }
		}
	    }
	}
	demand_empty_rest_of_line();
}

/*
 * s_incbin() implements the pseudo op:
 *	.incbin "filename"
 */
static
void
s_incbin(
uintptr_t value)
{
    char *filename, *whole_file_name, *p;
    int length;
    FILE *fp;
    int the_char;

	/* Some assemblers tolerate immediately following '"' */
	if((filename = demand_copy_string( & length ) )) {
	    demand_empty_rest_of_line();
	    whole_file_name = find_an_include_file(filename);
	    if(whole_file_name != NULL &&
	       (fp = fopen(whole_file_name, "r"))){
		do{
		    the_char = getc_unlocked(fp);
		    if (the_char != -1){
	    		p = frag_more(1);
			*p = the_char;
		    }
		}while(the_char != -1);
		fclose(fp);
		return;
	    }
	    as_fatal("Couldn't find the .incbin file: \"%s\"", filename);
	}
}

/*
 * s_data_region() parses and ignores the pseudo op:
 *	.data_region  { region_type }
 *	region_type := "jt8" | "jt16" | "jt32" | "jta32"
 */
static
void
s_data_region(
uintptr_t value)
{
    char *region_type, c;

	c = *input_line_pointer;
	if(c != '\n'){
	    region_type = input_line_pointer;
	    do{
		c = *input_line_pointer++;
	    }while(c != '\n');
	    input_line_pointer--;
        }
        demand_empty_rest_of_line();
}

/*
 * s_end_data_region() parses and ignores the pseudo op:
 *	.end_data_region
 */
static
void
s_end_data_region(
uintptr_t value)
{
        demand_empty_rest_of_line();
}

#ifdef SPARC

/* Special stuff to allow assembly of Sun assembler sources
   This unfortunatley needs to be here instead of sparc.c because it
   uses the hash tables defined here.
   see also sparc.c for pseudo_table entries 
*/

/* Handle the SUN sparc assembler .seg directive. .seg should only occur with
   either a ".text" or ".data" argument. Call .text or .data accordingly
*/
void
s_seg (ignore)
     int ignore;
{
  pseudo_typeS *ps_t;
  char s[32];

  printf("S_SEG\n");

  if (strncmp (input_line_pointer, "\"text\"", 6) == 0)
    {
      input_line_pointer += 6;
      /* relies on .text being first section */
      (void)s_builtin_section(builtin_sections);
      demand_empty_rest_of_line();
      return;
    }
  if (strncmp (input_line_pointer, "\"data\"", 6) == 0)
    {
      /* copy the argument */
      input_line_pointer++;
      strncpy(s, input_line_pointer, 4);
      input_line_pointer += 5;
      /* find the section table index for .data */
      ps_t = (pseudo_typeS *) hash_find(po_hash, s);

      if (ps_t == 0)
	as_bad ("invalid .seg argument");

      printf("INDEX %s, %p\n", s, (void *)ps_t->poc_val);

      s_builtin_section ((const struct builtin_section *)ps_t->poc_val);
      demand_empty_rest_of_line();
      return;
    }
  as_bad ("Unknown segment type");
  demand_empty_rest_of_line ();
}

#endif /* SPARC */

#ifdef PPC
/*
 * 
 */
/*
 * s_ppcasm_end() implements the ppcasm pseudo op:
 *	end
 * it is basicly ignored.
 */
static
void
s_ppcasm_end(
uintptr_t value)
{
      demand_empty_rest_of_line();
}
#endif /* PPC */

/* Return the size of a LEB128 value.  */

#ifndef ARCH64
static inline int
sizeof_sleb128_32 (int32_t value)
{
  register int size = 0;
  register unsigned byte;

  do
    {
      byte = (value & 0x7f);
      /* Sadly, we cannot rely on typical arithmetic right shift behaviour.
	 Fortunately, we can structure things so that the extra work reduces
	 to a noop on systems that do things "properly".  */
      value = (value >> 7) | ~(-(offsetT)1 >> 7);
      size += 1;
    }
  while (!(((value == 0) && ((byte & 0x40) == 0))
	   || ((value == -1) && ((byte & 0x40) != 0))));

  return size;
}
#endif /* !defined(ARCH64) */

#ifdef ARCH64
static inline int
sizeof_sleb128_64 (int64_t value)
{
  register int size = 0;
  register unsigned byte;

  do
    {
      byte = (value & 0x7f);
      /* Sadly, we cannot rely on typical arithmetic right shift behaviour.
	 Fortunately, we can structure things so that the extra work reduces
	 to a noop on systems that do things "properly".  */
      value = (value >> 7) | ~(-(offsetT)1 >> 7);
      size += 1;
    }
  while (!(((value == 0) && ((byte & 0x40) == 0))
	   || ((value == -1) && ((byte & 0x40) != 0))));

  return size;
}
#endif /* ARCH64 */

#ifndef ARCH64
static inline int
sizeof_uleb128_32 (uint32_t value)
{
  register int size = 0;
  register unsigned byte;

  do
    {
      byte = (value & 0x7f);
      value >>= 7;
      size += 1;
    }
  while (value != 0);

  return size;
}
#endif /* !defined(ARCH64) */

#ifdef ARCH64
static inline int
sizeof_uleb128_64 (uint64_t value)
{
  register int size = 0;
  register unsigned byte;

  do
    {
      byte = (value & 0x7f);
      value >>= 7;
      size += 1;
    }
  while (value != 0);

  return size;
}
#endif /* ARCH64 */

#ifdef ARCH64
int
sizeof_leb128 (valueT value, int sign)
{
  if (sign)
    return sizeof_sleb128_64 ((offsetT) value);
  else
    return sizeof_uleb128_64 (value);
}
#else
int
sizeof_leb128 (valueT value, int sign)
{
  if (sign)
    return sizeof_sleb128_32 (value);
  else
    return sizeof_uleb128_32 (value);
}
#endif

/* Output a LEB128 value.  */

static inline int
output_sleb128 (char *p, offsetT value)
{
  register char *orig = p;
  register int more;

  do
    {
      unsigned byte = (value & 0x7f);

      /* Sadly, we cannot rely on typical arithmetic right shift behaviour.
	 Fortunately, we can structure things so that the extra work reduces
	 to a noop on systems that do things "properly".  */
      value = (value >> 7) | ~(-(offsetT)1 >> 7);

      more = !((((value == 0) && ((byte & 0x40) == 0))
		|| ((value == -1) && ((byte & 0x40) != 0))));
      if (more)
	byte |= 0x80;

      *p++ = byte;
    }
  while (more);

  return (int)(p - orig);
}

static inline int
output_uleb128 (char *p, valueT value)
{
  char *orig = p;

  do
    {
      unsigned byte = (value & 0x7f);
      value >>= 7;
      if (value != 0)
	/* More bytes to follow.  */
	byte |= 0x80;

      *p++ = byte;
    }
  while (value != 0);

  return (int)(p - orig);
}

int
output_leb128 (char *p, valueT value, int sign)
{
  if (sign)
    return output_sleb128 (p, (offsetT) value);
  else
    return output_uleb128 (p, value);
}
