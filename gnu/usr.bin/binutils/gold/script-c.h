/* script-c.h -- C interface for linker scripts in gold.  */

/* Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>.

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* This file exists so that both the bison parser and script.cc can
   include it, so that they can communicate back and forth.  */

#ifndef GOLD_SCRIPT_C_H
#define GOLD_SCRIPT_C_H

#ifdef __cplusplus
#include <vector>
#include <string>
#endif

#ifdef __cplusplus

// For the C++ code we declare the various supporting structures in
// the gold namespace.  For the C code we declare it at the top level.
// The namespace level should not affect the layout of the structure.

namespace gold
{
#endif

/* A string value for the bison parser.  */

struct Parser_string
{
  const char* value;
  size_t length;
};

/* The expression functions deal with pointers to Expression objects.
   Since the bison parser generates C code, this is a hack to keep the
   C++ code type safe.  This hacks assumes that all pointers look
   alike.  */

#ifdef __cplusplus
class Expression;
typedef Expression* Expression_ptr;
#else
typedef void* Expression_ptr;
#endif

/* Script_section type.  */
enum Script_section_type
{
  /* No section type.  */
  SCRIPT_SECTION_TYPE_NONE,
  SCRIPT_SECTION_TYPE_NOLOAD,
  SCRIPT_SECTION_TYPE_DSECT,
  SCRIPT_SECTION_TYPE_COPY,
  SCRIPT_SECTION_TYPE_INFO,
  SCRIPT_SECTION_TYPE_OVERLAY
};

/* A constraint for whether to use a particular output section
   definition.  */

enum Section_constraint
{
  /* No constraint.  */
  CONSTRAINT_NONE,
  /* Only if all input sections are read-only.  */
  CONSTRAINT_ONLY_IF_RO,
  /* Only if at least input section is writable.  */
  CONSTRAINT_ONLY_IF_RW,
  /* Special constraint.  */
  CONSTRAINT_SPECIAL
};

/* The information we store for an output section header in the bison
   parser.  */

struct Parser_output_section_header
{
  /* The address.  This may be NULL.  */
  Expression_ptr address;
  /* Section type.  May be NULL string.  */ 
  enum Script_section_type section_type;
  /* The load address, from the AT specifier.  This may be NULL.  */
  Expression_ptr load_address;
  /* The alignment, from the ALIGN specifier.  This may be NULL.  */
  Expression_ptr align;
  /* The input section alignment, from the SUBALIGN specifier.  This
     may be NULL.  */
  Expression_ptr subalign;
  /* A constraint on this output section.  */
  enum Section_constraint constraint;
};

/* We keep vectors of strings.  In order to manage this in both C and
   C++, we use a pointer to a vector.  This assumes that all pointers
   look the same.  */

#ifdef __cplusplus
typedef std::vector<std::string> String_list;
typedef String_list* String_list_ptr;
#else
typedef void* String_list_ptr;
#endif

/* The information we store for an output section trailer in the bison
   parser.  */

struct Parser_output_section_trailer
{
  /* The fill value.  This may be NULL.  */
  Expression_ptr fill;
  /* The program segments this section should go into.  This may be
     NULL.  */
  String_list_ptr phdrs;
};

/* The different sorts we can find in a linker script.  */

enum Sort_wildcard
{
  SORT_WILDCARD_NONE,
  SORT_WILDCARD_BY_NAME,
  SORT_WILDCARD_BY_ALIGNMENT,
  SORT_WILDCARD_BY_NAME_BY_ALIGNMENT,
  SORT_WILDCARD_BY_ALIGNMENT_BY_NAME,
  SORT_WILDCARD_BY_INIT_PRIORITY
};

/* The information we build for a single wildcard specification.  */

struct Wildcard_section
{
  /* The wildcard spec itself.  */
  struct Parser_string name;
  /* How the entries should be sorted.  */
  enum Sort_wildcard sort;
};

/* A vector of Wildcard_section entries.  */

#ifdef __cplusplus
typedef std::vector<Wildcard_section> String_sort_list;
typedef String_sort_list* String_sort_list_ptr;
#else
typedef void* String_sort_list_ptr;
#endif

/* A list of wildcard specifications, which may include EXCLUDE_FILE
   clauses.  */

struct Wildcard_sections
{
  /* Wildcard specs.  */
  String_sort_list_ptr sections;
  /* Exclusions.  */
  String_list_ptr exclude;
};

/* A complete input section specification.  */

struct Input_section_spec
{
  /* The file name.  */
  struct Wildcard_section file;
  /* The list of sections.  */
  struct Wildcard_sections input_sections;
};

/* Information for a program header.  */

struct Phdr_info
{
  /* A boolean value: whether to include the file header.  */
  int includes_filehdr;
  /* A boolean value: whether to include the program headers.  */
  int includes_phdrs;
  /* A boolean value: whether the flags field is valid.  */
  int is_flags_valid;
  /* The value to use for the flags.  */
  unsigned int flags;
  /* The load address.  */
  Expression_ptr load_address;
};

struct Version_dependency_list;
struct Version_expression_list;
struct Version_tree;

#ifdef __cplusplus
extern "C" {
#endif

/* The bison parser definitions.  */

#include "yyscript.h"

/* The bison parser function.  */

extern int
yyparse(void* closure);

/* Called by the bison parser skeleton to return the next token.  */

extern int
yylex(YYSTYPE*, void* closure);

/* Called by the bison parser skeleton to report an error.  */

extern void
yyerror(void* closure, const char*);

/* Called by the bison parser to add an external symbol (a symbol in
   an EXTERN declaration) to the link.  */

extern void
script_add_extern(void* closure, const char*, size_t);

/* Called by the bison parser to add a file to the link.  */

extern void
script_add_file(void* closure, const char*, size_t);

/* Called by the bison parser to add a library to the link.  */

extern void
script_add_library(void* closure, const char*, size_t);

/* Called by the bison parser to start and stop a group.  */

extern void
script_start_group(void* closure);
extern void
script_end_group(void* closure);

/* Called by the bison parser to start and end an AS_NEEDED list.  */

extern void
script_start_as_needed(void* closure);
extern void
script_end_as_needed(void* closure);

/* Called by the bison parser to set the entry symbol.  */

extern void
script_set_entry(void* closure, const char*, size_t);

/* Called by the bison parser to set whether to define common symbols.  */

extern void
script_set_common_allocation(void* closure, int);

/* Called by the bison parser to parse an OPTION.  */

extern void
script_parse_option(void* closure, const char*, size_t);

/* Called by the bison parser to handle OUTPUT_FORMAT.  This return 0
   if the parse should be aborted.  */

extern int
script_check_output_format(void* closure, const char*, size_t,
			   const char*, size_t, const char*, size_t);

/* Called by the bison parser to handle TARGET.  */
extern void
script_set_target(void* closure, const char*, size_t);

/* Called by the bison parser to handle SEARCH_DIR.  */

extern void
script_add_search_dir(void* closure, const char*, size_t);

/* Called by the bison parser to push the lexer into expression
   mode.  */

extern void
script_push_lex_into_expression_mode(void* closure);

/* Called by the bison parser to push the lexer into version
   mode.  */

extern void
script_push_lex_into_version_mode(void* closure);

/* Called by the bison parser to pop the lexer mode.  */

extern void
script_pop_lex_mode(void* closure);

/* Called by the bison parser to get the value of a symbol.  This is
   called for a reference to a symbol, but is not called for something
   like "sym += 10".  Uses of the special symbol "." can just call
   script_exp_string.  */

extern Expression_ptr
script_symbol(void* closure, const char*, size_t);

/* Called by the bison parser to set a symbol to a value.  PROVIDE is
   non-zero if the symbol should be provided--only defined if there is
   an undefined reference.  HIDDEN is non-zero if the symbol should be
   hidden.  */

extern void
script_set_symbol(void* closure, const char*, size_t, Expression_ptr,
		  int provide, int hidden);

/* Called by the bison parser to add an assertion.  */

extern void
script_add_assertion(void* closure, Expression_ptr, const char* message,
		     size_t messagelen);

/* Called by the bison parser to start a SECTIONS clause.  */

extern void
script_start_sections(void* closure);

/* Called by the bison parser to finish a SECTIONS clause.  */

extern void
script_finish_sections(void* closure);

/* Called by the bison parser to start handling input section
   specifications for an output section.  */

extern void
script_start_output_section(void* closure, const char* name, size_t namelen,
			    const struct Parser_output_section_header*);

/* Called by the bison parser when done handling input section
   specifications for an output section.  */

extern void
script_finish_output_section(void* closure,
			     const struct Parser_output_section_trailer*);

/* Called by the bison parser to handle a data statement (LONG, BYTE,
   etc.) in an output section.  */

extern void
script_add_data(void* closure, int data_token, Expression_ptr val);

/* Called by the bison parser to set the fill value in an output
   section.  */

extern void
script_add_fill(void* closure, Expression_ptr val);

/* Called by the bison parser to add an input section specification to
   an output section.  The KEEP parameter is non-zero if this is
   within a KEEP clause, meaning that the garbage collector should not
   discard it.  */

extern void
script_add_input_section(void* closure, const struct Input_section_spec*,
			 int keep);

/* Create a new list of string and sort entries.  */

extern String_sort_list_ptr
script_new_string_sort_list(const struct Wildcard_section*);

/* Add an entry to a list of string and sort entries.  */

extern String_sort_list_ptr
script_string_sort_list_add(String_sort_list_ptr,
			    const struct Wildcard_section*);

/* Create a new list of strings.  */

extern String_list_ptr
script_new_string_list(const char*, size_t);

/* Add an element to a list of strings.  */

extern String_list_ptr
script_string_list_push_back(String_list_ptr, const char*, size_t);

/* Concatenate two string lists.  */

extern String_list_ptr
script_string_list_append(String_list_ptr, String_list_ptr);

/* Define a new program header.  */

extern void
script_add_phdr(void* closure, const char* name, size_t namelen,
		unsigned int type, const struct Phdr_info*);

/* Convert a program header string to a type.  */

extern unsigned int
script_phdr_string_to_type(void* closure, const char*, size_t);

/* Handle DATA_SEGMENT_ALIGN and DATA_SEGMENT_RELRO_END.  */

extern void
script_data_segment_align(void* closure);

extern void
script_data_segment_relro_end(void* closure);

/* Record the fact that a SEGMENT_START expression is seen.  */

extern void
script_saw_segment_start_expression(void* closure);

/* Called by the bison parser for MEMORY regions.  */

extern void
script_add_memory(void*, const char*, size_t, unsigned int,
		  Expression_ptr, Expression_ptr);

extern unsigned int
script_parse_memory_attr(void*, const char*, size_t, int);

extern void
script_set_section_region(void*, const char*, size_t, int);

extern void
script_include_directive(int, void *, const char*, size_t);
  
/* Called by the bison parser for expressions.  */

extern Expression_ptr
script_exp_unary_minus(Expression_ptr);
extern Expression_ptr
script_exp_unary_logical_not(Expression_ptr);
extern Expression_ptr
script_exp_unary_bitwise_not(Expression_ptr);
extern Expression_ptr
script_exp_binary_mult(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_div(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_mod(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_add(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_sub(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_lshift(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_rshift(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_eq(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_ne(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_le(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_ge(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_lt(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_gt(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_bitwise_and(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_bitwise_xor(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_bitwise_or(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_logical_and(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_binary_logical_or(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_trinary_cond(Expression_ptr, Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_integer(uint64_t);
extern Expression_ptr
script_exp_string(const char*, size_t);
extern Expression_ptr
script_exp_function_max(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_function_min(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_function_defined(const char*, size_t);
extern Expression_ptr
script_exp_function_sizeof_headers(void);
extern Expression_ptr
script_exp_function_alignof(const char*, size_t);
extern Expression_ptr
script_exp_function_sizeof(const char*, size_t);
extern Expression_ptr
script_exp_function_addr(const char*, size_t);
extern Expression_ptr
script_exp_function_loadaddr(const char*, size_t);
extern Expression_ptr
script_exp_function_origin(void*, const char*, size_t);
extern Expression_ptr
script_exp_function_length(void*, const char*, size_t);
extern Expression_ptr
script_exp_function_constant(const char*, size_t);
extern Expression_ptr
script_exp_function_absolute(Expression_ptr);
extern Expression_ptr
script_exp_function_align(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_function_data_segment_align(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_function_data_segment_relro_end(Expression_ptr, Expression_ptr);
extern Expression_ptr
script_exp_function_data_segment_end(Expression_ptr);
extern Expression_ptr
script_exp_function_segment_start(const char*, size_t, Expression_ptr);
extern Expression_ptr
script_exp_function_assert(Expression_ptr, const char*, size_t);

extern void
script_register_vers_node(void* closure,
			  const char* tag,
			  int taglen,
			  struct Version_tree*,
			  struct Version_dependency_list*);

extern struct Version_dependency_list*
script_add_vers_depend(void* closure,
		       struct Version_dependency_list* existing_dependencies,
		       const char* depend_to_add, int deplen);

extern struct Version_expression_list*
script_new_vers_pattern(void* closure,
			struct Version_expression_list*,
			const char*, int, int);

extern struct Version_expression_list*
script_merge_expressions(struct Version_expression_list* a,
                         struct Version_expression_list* b);

extern struct Version_tree*
script_new_vers_node(void* closure,
		     struct Version_expression_list* global,
		     struct Version_expression_list* local);

extern void
version_script_push_lang(void* closure, const char* lang, int langlen);

extern void
version_script_pop_lang(void* closure);

#ifdef __cplusplus
} // End extern "C"
#endif

#ifdef __cplusplus
} // End namespace gold.
#endif

#endif /* !defined(GOLD_SCRIPT_C_H) */
