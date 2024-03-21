/* yyscript.y -- linker script grammar for gold.  */

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

/* This is a bison grammar to parse a subset of the original GNU ld
   linker script language.  */

%{

#include "config.h"
#include "diagnostics.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "script-c.h"

DIAGNOSTIC_IGNORE_UNUSED_BUT_SET_VARIABLE

%}

/* We need to use a pure parser because we might be multi-threaded.
   We pass some arguments through the parser to the lexer.  */

%pure-parser

%parse-param {void* closure}
%lex-param {void* closure}

/* Since we require bison anyhow, we take advantage of it.  */

%error-verbose

/* The values associated with tokens.  */

%union {
  /* A string.  */
  struct Parser_string string;
  /* A number.  */
  uint64_t integer;
  /* An expression.  */
  Expression_ptr expr;
  /* An output section header.  */
  struct Parser_output_section_header output_section_header;
  /* An output section trailer.  */
  struct Parser_output_section_trailer output_section_trailer;
  /* A section constraint.  */
  enum Section_constraint constraint;
  /* A complete input section specification.  */
  struct Input_section_spec input_section_spec;
  /* A list of wildcard specifications, with exclusions.  */
  struct Wildcard_sections wildcard_sections;
  /* A single wildcard specification.  */
  struct Wildcard_section wildcard_section;
  /* A list of strings.  */
  String_list_ptr string_list;
  /* Information for a program header.  */
  struct Phdr_info phdr_info;
  /* Used for version scripts and within VERSION {}.  */
  struct Version_dependency_list* deplist;
  struct Version_expression_list* versyms;
  struct Version_tree* versnode;
  enum Script_section_type section_type;
}

/* Operators, including a precedence table for expressions.  */

%right PLUSEQ MINUSEQ MULTEQ DIVEQ '=' LSHIFTEQ RSHIFTEQ ANDEQ OREQ
%right '?' ':'
%left OROR
%left ANDAND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left '<' '>' LE GE
%left LSHIFT RSHIFT
%left '+' '-'
%left '*' '/' '%'

/* A fake operator used to indicate unary operator precedence.  */
%right UNARY

/* Constants.  */

%token <string> STRING
%token <string> QUOTED_STRING
%token <integer> INTEGER

/* Keywords.  This list is taken from ldgram.y and ldlex.l in the old
   GNU linker, with the keywords which only appear in MRI mode
   removed.  Not all these keywords are actually used in this grammar.
   In most cases the keyword is recognized as the token name in upper
   case.  The comments indicate where this is not the case.  */

%token ABSOLUTE
%token ADDR
%token ALIGN_K		/* ALIGN */
%token ALIGNOF
%token ASSERT_K		/* ASSERT */
%token AS_NEEDED
%token AT
%token BIND
%token BLOCK
%token BYTE
%token CONSTANT
%token CONSTRUCTORS
%token COPY
%token CREATE_OBJECT_SYMBOLS
%token DATA_SEGMENT_ALIGN
%token DATA_SEGMENT_END
%token DATA_SEGMENT_RELRO_END
%token DEFINED
%token DSECT
%token ENTRY
%token EXCLUDE_FILE
%token EXTERN
%token FILL
%token FLOAT
%token FORCE_COMMON_ALLOCATION
%token GLOBAL		/* global */
%token GROUP
%token HIDDEN
%token HLL
%token INCLUDE
%token INHIBIT_COMMON_ALLOCATION
%token INFO
%token INPUT
%token KEEP
%token LEN
%token LENGTH		/* LENGTH, l, len */
%token LOADADDR
%token LOCAL		/* local */
%token LONG
%token MAP
%token MAX_K		/* MAX */
%token MEMORY
%token MIN_K		/* MIN */
%token NEXT
%token NOCROSSREFS
%token NOFLOAT
%token NOLOAD
%token ONLY_IF_RO
%token ONLY_IF_RW
%token ORG
%token ORIGIN		/* ORIGIN, o, org */
%token OUTPUT
%token OUTPUT_ARCH
%token OUTPUT_FORMAT
%token OVERLAY
%token PHDRS
%token PROVIDE
%token PROVIDE_HIDDEN
%token QUAD
%token SEARCH_DIR
%token SECTIONS
%token SEGMENT_START
%token SHORT
%token SIZEOF
%token SIZEOF_HEADERS	/* SIZEOF_HEADERS, sizeof_headers */
%token SORT_BY_ALIGNMENT
%token SORT_BY_INIT_PRIORITY
%token SORT_BY_NAME
%token SPECIAL
%token SQUAD
%token STARTUP
%token SUBALIGN
%token SYSLIB
%token TARGET_K		/* TARGET */
%token TRUNCATE
%token VERSIONK		/* VERSION */

/* Keywords, part 2.  These are keywords that are unique to gold,
   and not present in the old GNU linker.  As before, unless the
   comments say otherwise, the keyword is recognized as the token
   name in upper case. */

%token OPTION

/* Special tokens used to tell the grammar what type of tokens we are
   parsing.  The token stream always begins with one of these tokens.
   We do this because version scripts can appear embedded within
   linker scripts, and because --defsym uses the expression
   parser.  */
%token PARSING_LINKER_SCRIPT
%token PARSING_VERSION_SCRIPT
%token PARSING_DEFSYM
%token PARSING_DYNAMIC_LIST
%token PARSING_SECTIONS_BLOCK
%token PARSING_SECTION_COMMANDS
%token PARSING_MEMORY_DEF

/* Non-terminal types, where needed.  */

%type <expr> parse_exp exp
%type <expr> opt_at opt_align opt_subalign opt_fill
%type <output_section_header> section_header opt_address_and_section_type
%type <section_type> section_type
%type <output_section_trailer> section_trailer
%type <constraint> opt_constraint
%type <string_list> opt_phdr
%type <integer> data_length
%type <input_section_spec> input_section_no_keep
%type <wildcard_sections> wildcard_sections
%type <wildcard_section> wildcard_file wildcard_section
%type <string_list> exclude_names
%type <string> wildcard_name
%type <integer> phdr_type memory_attr
%type <phdr_info> phdr_info
%type <versyms> vers_defns
%type <versnode> vers_tag
%type <deplist> verdep
%type <string> string

%%

/* Read the special token to see what to read next.  */
top:
	  PARSING_LINKER_SCRIPT linker_script
	| PARSING_VERSION_SCRIPT version_script
	| PARSING_DEFSYM defsym_expr
        | PARSING_DYNAMIC_LIST dynamic_list_expr
        | PARSING_SECTIONS_BLOCK sections_block
        | PARSING_SECTION_COMMANDS section_cmds
        | PARSING_MEMORY_DEF memory_defs
	;

/* A file contains a list of commands.  */
linker_script:
	  linker_script file_cmd
	| /* empty */
	;

/* A command which may appear at top level of a linker script.  */
file_cmd:
	  EXTERN '(' extern_name_list ')'
	| FORCE_COMMON_ALLOCATION
	    { script_set_common_allocation(closure, 1); }
	| GROUP
	    { script_start_group(closure); }
	  '(' input_list ')'
	    { script_end_group(closure); }
	| INHIBIT_COMMON_ALLOCATION
	    { script_set_common_allocation(closure, 0); }
	| INPUT '(' input_list ')'
	| MEMORY '{' memory_defs '}'
        | OPTION '(' string ')'
	    { script_parse_option(closure, $3.value, $3.length); }
	| OUTPUT_FORMAT '(' string ')'
	    {
	      if (!script_check_output_format(closure, $3.value, $3.length,
					      NULL, 0, NULL, 0))
		YYABORT;
	    }
	| OUTPUT_FORMAT '(' string ',' string ',' string ')'
	    {
	      if (!script_check_output_format(closure, $3.value, $3.length,
					      $5.value, $5.length,
					      $7.value, $7.length))
		YYABORT;
	    }
	| PHDRS '{' phdrs_defs '}'
	| SEARCH_DIR '(' string ')'
	    { script_add_search_dir(closure, $3.value, $3.length); }
	| SECTIONS '{'
	    { script_start_sections(closure); }
	  sections_block '}'
	    { script_finish_sections(closure); }
	| TARGET_K '(' string ')'
	    { script_set_target(closure, $3.value, $3.length); }
        | VERSIONK '{'
            { script_push_lex_into_version_mode(closure); }
          version_script '}'
            { script_pop_lex_mode(closure); }
	| ENTRY '(' string ')'
	    { script_set_entry(closure, $3.value, $3.length); }
	| assignment end
	| ASSERT_K '(' parse_exp ',' string ')'
	    { script_add_assertion(closure, $3, $5.value, $5.length); }
	| INCLUDE string
	    { script_include_directive(PARSING_LINKER_SCRIPT, closure,
				       $2.value, $2.length); }
	| ignore_cmd
	| ';'
	;

/* Top level commands which we ignore.  The GNU linker uses these to
   select the output format, but we don't offer a choice.  Ignoring
   these is more-or-less OK since most scripts simply explicitly
   choose the default.  */
ignore_cmd:
	  OUTPUT_ARCH '(' string ')'
	;

/* A list of external undefined symbols.  We put the lexer into
   expression mode so that commas separate names; this is what the GNU
   linker does.  */

extern_name_list:
	    { script_push_lex_into_expression_mode(closure); }
	  extern_name_list_body
	    { script_pop_lex_mode(closure); }
	;

extern_name_list_body:
	  string
	    { script_add_extern(closure, $1.value, $1.length); }
	| extern_name_list_body string
	    { script_add_extern(closure, $2.value, $2.length); }
	| extern_name_list_body ',' string
	    { script_add_extern(closure, $3.value, $3.length); }
	;

/* A list of input file names.  */
input_list:
	  input_list_element
	| input_list opt_comma input_list_element
	;

/* An input file name.  */
input_list_element:
	  string
	    { script_add_file(closure, $1.value, $1.length); }
	| '-' STRING
	    { script_add_library(closure, $2.value, $2.length); }
	| AS_NEEDED
	    { script_start_as_needed(closure); }
	  '(' input_list ')'
	    { script_end_as_needed(closure); }
	;

/* Commands in a SECTIONS block.  */
sections_block:
	  sections_block section_block_cmd
	| /* empty */
	;

/* A command which may appear within a SECTIONS block.  */
section_block_cmd:
	  ENTRY '(' string ')'
	    { script_set_entry(closure, $3.value, $3.length); }
	| assignment end
	| ASSERT_K '(' parse_exp ',' string ')'
	    { script_add_assertion(closure, $3, $5.value, $5.length); }
	| INCLUDE string
	    { script_include_directive(PARSING_SECTIONS_BLOCK, closure,
				       $2.value, $2.length); }
	| string section_header
	    { script_start_output_section(closure, $1.value, $1.length, &$2); }
	  '{' section_cmds '}' section_trailer
	    { script_finish_output_section(closure, &$7); }
	;

/* The header of an output section in a SECTIONS block--everything
   after the name.  */
section_header:
	    { script_push_lex_into_expression_mode(closure); }
	  opt_address_and_section_type opt_at opt_align opt_subalign
	    { script_pop_lex_mode(closure); }
	  opt_constraint
	    {
	      $$.address = $2.address;
	      $$.section_type = $2.section_type;
	      $$.load_address = $3;
	      $$.align = $4;
	      $$.subalign = $5;
	      $$.constraint = $7;
	    }
	;

/* The optional address followed by the optional section type.  This
   is a separate nonterminal to avoid a shift/reduce conflict on
   '(' in section_header.  */

opt_address_and_section_type:
	':'
	    {
	      $$.address = NULL;
	      $$.section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
	| '(' ')' ':'
	    {
	      $$.address = NULL;
	      $$.section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
	| exp ':'
	    {
	      $$.address = $1;
	      $$.section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
	| exp '(' ')' ':'
	    {
	      $$.address = $1;
	      $$.section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
	| '(' section_type ')' ':'
	    {
	      $$.address = NULL;
	      $$.section_type = $2;
	    }
	| exp '(' section_type ')' ':'
	    {
	      $$.address = $1;
	      $$.section_type = $3;
	    }
	;

/* We only support NOLOAD.  */
section_type:
	NOLOAD
	    { $$ = SCRIPT_SECTION_TYPE_NOLOAD; }
	| DSECT
	    {
	      yyerror(closure, "DSECT section type is unsupported");
	      $$ = SCRIPT_SECTION_TYPE_DSECT;
	    }
	| COPY
	    {
	      yyerror(closure, "COPY section type is unsupported");
	      $$ = SCRIPT_SECTION_TYPE_COPY;
	    }
	| INFO
	    {
	      yyerror(closure, "INFO section type is unsupported");
	      $$ = SCRIPT_SECTION_TYPE_INFO;
	    }
	| OVERLAY
	    {
	      yyerror(closure, "OVERLAY section type is unsupported");
	      $$ = SCRIPT_SECTION_TYPE_OVERLAY;
	    }
	;

/* The address at which an output section should be loaded.  */
opt_at:
	  /* empty */
	    { $$ = NULL; }
	| AT '(' exp ')'
	    { $$ = $3; }
	;

/* The alignment of an output section.  */
opt_align:
	  /* empty */
	    { $$ = NULL; }
	| ALIGN_K '(' exp ')'
	    { $$ = $3; }
	;

/* The input section alignment within an output section.  */
opt_subalign:
	  /* empty */
	    { $$ = NULL; }
	| SUBALIGN '(' exp ')'
	    { $$ = $3; }
	;

/* A section constraint.  */
opt_constraint:
	  /* empty */
	    { $$ = CONSTRAINT_NONE; }
	| ONLY_IF_RO
	    { $$ = CONSTRAINT_ONLY_IF_RO; }
	| ONLY_IF_RW
	    { $$ = CONSTRAINT_ONLY_IF_RW; }
	| SPECIAL
	    { $$ = CONSTRAINT_SPECIAL; }
	;

/* The trailer of an output section in a SECTIONS block.  */
section_trailer:
	  opt_memspec opt_at_memspec opt_phdr opt_fill opt_comma
	    {
	      $$.fill = $4;
	      $$.phdrs = $3;
	    }
	;

/* A memory specification for an output section.  */
opt_memspec:
	  '>' string
	    { script_set_section_region(closure, $2.value, $2.length, 1); }
	| /* empty */
	;

/* A memory specification for where to load an output section.  */
opt_at_memspec:
	  AT '>' string
	    { script_set_section_region(closure, $3.value, $3.length, 0); }
	| /* empty */
	;

/* The program segment an output section should go into.  */
opt_phdr:
	  opt_phdr ':' string
	    { $$ = script_string_list_push_back($1, $3.value, $3.length); }
	| /* empty */
	    { $$ = NULL; }
	;

/* The value to use to fill an output section.  FIXME: This does not
   handle a string of arbitrary length.  */
opt_fill:
	  '=' parse_exp
	    { $$ = $2; }
	| /* empty */
	    { $$ = NULL; }
	;

/* Commands which may appear within the description of an output
   section in a SECTIONS block.  */
section_cmds:
	  /* empty */
	| section_cmds section_cmd
	;

/* A command which may appear within the description of an output
   section in a SECTIONS block.  */
section_cmd:
	  assignment end
	| input_section_spec
	| data_length '(' parse_exp ')'
	    { script_add_data(closure, $1, $3); }
	| ASSERT_K '(' parse_exp ',' string ')'
	    { script_add_assertion(closure, $3, $5.value, $5.length); }
	| FILL '(' parse_exp ')'
	    { script_add_fill(closure, $3); }
	| CONSTRUCTORS
	    {
	      /* The GNU linker uses CONSTRUCTORS for the a.out object
		 file format.  It does nothing when using ELF.  Since
		 some ELF linker scripts use it although it does
		 nothing, we accept it and ignore it.  */
	    }
	| SORT_BY_NAME '(' CONSTRUCTORS ')'
	| INCLUDE string
	    { script_include_directive(PARSING_SECTION_COMMANDS, closure,
				       $2.value, $2.length); }
	| ';'
	;

/* The length of data which may appear within the description of an
   output section in a SECTIONS block.  */
data_length:
	  QUAD
	    { $$ = QUAD; }
	| SQUAD
	    { $$ = SQUAD; }
	| LONG
	    { $$ = LONG; }
	| SHORT
	    { $$ = SHORT; }
	| BYTE
	    { $$ = BYTE; }
	;

/* An input section specification.  This may appear within the
   description of an output section in a SECTIONS block.  */
input_section_spec:
	  input_section_no_keep
	    { script_add_input_section(closure, &$1, 0); }
	| KEEP '(' input_section_no_keep ')'
	    { script_add_input_section(closure, &$3, 1); }
	;

/* An input section specification within a KEEP clause.  */
input_section_no_keep:
	  string
	    {
	      $$.file.name = $1;
	      $$.file.sort = SORT_WILDCARD_NONE;
	      $$.input_sections.sections = NULL;
	      $$.input_sections.exclude = NULL;
	    }
	| wildcard_file '(' wildcard_sections ')'
	    {
	      $$.file = $1;
	      $$.input_sections = $3;
	    }
	;

/* A wildcard file specification.  */
wildcard_file:
	  wildcard_name
	    {
	      $$.name = $1;
	      $$.sort = SORT_WILDCARD_NONE;
	    }
	| SORT_BY_NAME '(' wildcard_name ')'
	    {
	      $$.name = $3;
	      $$.sort = SORT_WILDCARD_BY_NAME;
	    }
	;

/* A list of wild card section specifications.  */
wildcard_sections:
	  wildcard_sections opt_comma wildcard_section
	    {
	      $$.sections = script_string_sort_list_add($1.sections, &$3);
	      $$.exclude = $1.exclude;
	    }
	| wildcard_section
	    {
	      $$.sections = script_new_string_sort_list(&$1);
	      $$.exclude = NULL;
	    }
	| wildcard_sections opt_comma EXCLUDE_FILE '(' exclude_names ')'
	    {
	      $$.sections = $1.sections;
	      $$.exclude = script_string_list_append($1.exclude, $5);
	    }
	| EXCLUDE_FILE '(' exclude_names ')'
	    {
	      $$.sections = NULL;
	      $$.exclude = $3;
	    }
	;

/* A single wild card specification.  */
wildcard_section:
	  wildcard_name
	    {
	      $$.name = $1;
	      $$.sort = SORT_WILDCARD_NONE;
	    }
	| SORT_BY_NAME '(' wildcard_section ')'
	    {
	      $$.name = $3.name;
	      switch ($3.sort)
		{
		case SORT_WILDCARD_NONE:
		  $$.sort = SORT_WILDCARD_BY_NAME;
		  break;
		case SORT_WILDCARD_BY_NAME:
		case SORT_WILDCARD_BY_NAME_BY_ALIGNMENT:
		  break;
		case SORT_WILDCARD_BY_ALIGNMENT:
		case SORT_WILDCARD_BY_ALIGNMENT_BY_NAME:
		  $$.sort = SORT_WILDCARD_BY_NAME_BY_ALIGNMENT;
		  break;
		default:
		  abort();
		}
	    }
	| SORT_BY_ALIGNMENT '(' wildcard_section ')'
	    {
	      $$.name = $3.name;
	      switch ($3.sort)
		{
		case SORT_WILDCARD_NONE:
		  $$.sort = SORT_WILDCARD_BY_ALIGNMENT;
		  break;
		case SORT_WILDCARD_BY_ALIGNMENT:
		case SORT_WILDCARD_BY_ALIGNMENT_BY_NAME:
		  break;
		case SORT_WILDCARD_BY_NAME:
		case SORT_WILDCARD_BY_NAME_BY_ALIGNMENT:
		  $$.sort = SORT_WILDCARD_BY_ALIGNMENT_BY_NAME;
		  break;
		default:
		  abort();
		}
	    }
	| SORT_BY_INIT_PRIORITY '(' wildcard_name ')'
	    {
	      $$.name = $3;
	      $$.sort = SORT_WILDCARD_BY_INIT_PRIORITY;
	    }
	;

/* A list of file names to exclude.  */
exclude_names:
	  exclude_names opt_comma wildcard_name
	    { $$ = script_string_list_push_back($1, $3.value, $3.length); }
	| wildcard_name
	    { $$ = script_new_string_list($1.value, $1.length); }
	;

/* A single wildcard name.  We recognize '*' and '?' specially since
   they are expression tokens.  */
wildcard_name:
	  string
	    { $$ = $1; }
	| '*'
	    {
	      $$.value = "*";
	      $$.length = 1;
	    }
	| '?'
	    {
	      $$.value = "?";
	      $$.length = 1;
	    }
	;

/* A list of MEMORY definitions.  */
memory_defs:
	  memory_defs opt_comma memory_def
	| /* empty */
	;

/* A single MEMORY definition.  */
memory_def:
	  string memory_attr ':' memory_origin '=' parse_exp opt_comma memory_length '=' parse_exp
	  { script_add_memory(closure, $1.value, $1.length, $2, $6, $10); }
	|
	  INCLUDE string
	  { script_include_directive(PARSING_MEMORY_DEF, closure,
				     $2.value, $2.length); }
	|
	;

/* The (optional) attributes of a MEMORY region.  */
memory_attr:
	  '(' string ')'
	  { $$ = script_parse_memory_attr(closure, $2.value, $2.length, 0); }
        | /* Inverted attributes. */
	  '(' '!' string ')'
	  { $$ = script_parse_memory_attr(closure, $3.value, $3.length, 1); }
	| /* empty */
	    { $$ = 0; }
	;

memory_origin:
          ORIGIN
	|
	  ORG
	|
	  'o'
	;

memory_length:
          LENGTH
	|
	  LEN
	|
	  'l'
	;

/* A list of program header definitions.  */
phdrs_defs:
	  phdrs_defs phdr_def
	| /* empty */
	;

/* A program header definition.  */
phdr_def:
	  string phdr_type phdr_info ';'
	    { script_add_phdr(closure, $1.value, $1.length, $2, &$3); }
	;

/* A program header type.  The GNU linker accepts a general expression
   here, but that would be a pain because we would have to dig into
   the expression structure.  It's unlikely that anybody uses anything
   other than a string or a number here, so that is all we expect.  */
phdr_type:
	  string
	    { $$ = script_phdr_string_to_type(closure, $1.value, $1.length); }
	| INTEGER
	    { $$ = $1; }
	;

/* Additional information for a program header.  */
phdr_info:
	  /* empty */
	    { memset(&$$, 0, sizeof(struct Phdr_info)); }
	| string phdr_info
	    {
	      $$ = $2;
	      if ($1.length == 7 && strncmp($1.value, "FILEHDR", 7) == 0)
		$$.includes_filehdr = 1;
	      else
		yyerror(closure, "PHDRS syntax error");
	    }
	| PHDRS phdr_info
	    {
	      $$ = $2;
	      $$.includes_phdrs = 1;
	    }
	| string '(' INTEGER ')' phdr_info
	    {
	      $$ = $5;
	      if ($1.length == 5 && strncmp($1.value, "FLAGS", 5) == 0)
		{
		  $$.is_flags_valid = 1;
		  $$.flags = $3;
		}
	      else
		yyerror(closure, "PHDRS syntax error");
	    }
	| AT '(' parse_exp ')' phdr_info
	    {
	      $$ = $5;
	      $$.load_address = $3;
	    }
	;

/* Set a symbol to a value.  */
assignment:
	  string '=' parse_exp
	    { script_set_symbol(closure, $1.value, $1.length, $3, 0, 0); }
	| string PLUSEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_add(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string MINUSEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_sub(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string MULTEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_mult(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string DIVEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_div(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string LSHIFTEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_lshift(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string RSHIFTEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_rshift(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string ANDEQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_bitwise_and(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| string OREQ parse_exp
	    {
	      Expression_ptr s = script_exp_string($1.value, $1.length);
	      Expression_ptr e = script_exp_binary_bitwise_or(s, $3);
	      script_set_symbol(closure, $1.value, $1.length, e, 0, 0);
	    }
	| HIDDEN '(' string '=' parse_exp ')'
	    { script_set_symbol(closure, $3.value, $3.length, $5, 0, 1); }
	| PROVIDE '(' string '=' parse_exp ')'
	    { script_set_symbol(closure, $3.value, $3.length, $5, 1, 0); }
	| PROVIDE_HIDDEN '(' string '=' parse_exp ')'
	    { script_set_symbol(closure, $3.value, $3.length, $5, 1, 1); }
	;

/* Parse an expression, putting the lexer into the right mode.  */
parse_exp:
	    { script_push_lex_into_expression_mode(closure); }
	  exp
	    {
	      script_pop_lex_mode(closure);
	      $$ = $2;
	    }
	;

/* An expression.  */
exp:
	  '(' exp ')'
	    { $$ = $2; }
	| '-' exp %prec UNARY
	    { $$ = script_exp_unary_minus($2); }
	| '!' exp %prec UNARY
	    { $$ = script_exp_unary_logical_not($2); }
	| '~' exp %prec UNARY
	    { $$ = script_exp_unary_bitwise_not($2); }
	| '+' exp %prec UNARY
	    { $$ = $2; }
	| exp '*' exp
	    { $$ = script_exp_binary_mult($1, $3); }
	| exp '/' exp
	    { $$ = script_exp_binary_div($1, $3); }
	| exp '%' exp
	    { $$ = script_exp_binary_mod($1, $3); }
	| exp '+' exp
	    { $$ = script_exp_binary_add($1, $3); }
	| exp '-' exp
	    { $$ = script_exp_binary_sub($1, $3); }
	| exp LSHIFT exp
	    { $$ = script_exp_binary_lshift($1, $3); }
	| exp RSHIFT exp
	    { $$ = script_exp_binary_rshift($1, $3); }
	| exp EQ exp
	    { $$ = script_exp_binary_eq($1, $3); }
	| exp NE exp
	    { $$ = script_exp_binary_ne($1, $3); }
	| exp LE exp
	    { $$ = script_exp_binary_le($1, $3); }
	| exp GE exp
	    { $$ = script_exp_binary_ge($1, $3); }
	| exp '<' exp
	    { $$ = script_exp_binary_lt($1, $3); }
	| exp '>' exp
	    { $$ = script_exp_binary_gt($1, $3); }
	| exp '&' exp
	    { $$ = script_exp_binary_bitwise_and($1, $3); }
	| exp '^' exp
	    { $$ = script_exp_binary_bitwise_xor($1, $3); }
	| exp '|' exp
	    { $$ = script_exp_binary_bitwise_or($1, $3); }
	| exp ANDAND exp
	    { $$ = script_exp_binary_logical_and($1, $3); }
	| exp OROR exp
	    { $$ = script_exp_binary_logical_or($1, $3); }
	| exp '?' exp ':' exp
	    { $$ = script_exp_trinary_cond($1, $3, $5); }
	| INTEGER
	    { $$ = script_exp_integer($1); }
	| string
	    { $$ = script_symbol(closure, $1.value, $1.length); }
	| MAX_K '(' exp ',' exp ')'
	    { $$ = script_exp_function_max($3, $5); }
	| MIN_K '(' exp ',' exp ')'
	    { $$ = script_exp_function_min($3, $5); }
	| DEFINED '(' string ')'
	    { $$ = script_exp_function_defined($3.value, $3.length); }
	| SIZEOF_HEADERS
	    { $$ = script_exp_function_sizeof_headers(); }
	| ALIGNOF '(' string ')'
	    { $$ = script_exp_function_alignof($3.value, $3.length); }
	| SIZEOF '(' string ')'
	    { $$ = script_exp_function_sizeof($3.value, $3.length); }
	| ADDR '(' string ')'
	    { $$ = script_exp_function_addr($3.value, $3.length); }
	| LOADADDR '(' string ')'
	    { $$ = script_exp_function_loadaddr($3.value, $3.length); }
	| ORIGIN '(' string ')'
	    { $$ = script_exp_function_origin(closure, $3.value, $3.length); }
	| LENGTH '(' string ')'
	    { $$ = script_exp_function_length(closure, $3.value, $3.length); }
	| CONSTANT '(' string ')'
	    { $$ = script_exp_function_constant($3.value, $3.length); }
	| ABSOLUTE '(' exp ')'
	    { $$ = script_exp_function_absolute($3); }
	| ALIGN_K '(' exp ')'
	    { $$ = script_exp_function_align(script_exp_string(".", 1), $3); }
	| ALIGN_K '(' exp ',' exp ')'
	    { $$ = script_exp_function_align($3, $5); }
	| BLOCK '(' exp ')'
	    { $$ = script_exp_function_align(script_exp_string(".", 1), $3); }
	| DATA_SEGMENT_ALIGN '(' exp ',' exp ')'
	    {
	      script_data_segment_align(closure);
	      $$ = script_exp_function_data_segment_align($3, $5);
	    }
	| DATA_SEGMENT_RELRO_END '(' exp ',' exp ')'
	    {
	      script_data_segment_relro_end(closure);
	      $$ = script_exp_function_data_segment_relro_end($3, $5);
	    }
	| DATA_SEGMENT_END '(' exp ')'
	    { $$ = script_exp_function_data_segment_end($3); }
	| SEGMENT_START '(' string ',' exp ')'
	    {
	      $$ = script_exp_function_segment_start($3.value, $3.length, $5);
	      /* We need to take note of any SEGMENT_START expressions
		 because they change the behaviour of -Ttext, -Tdata and
		 -Tbss options.  */
	      script_saw_segment_start_expression(closure);
	    }
	| ASSERT_K '(' exp ',' string ')'
	    { $$ = script_exp_function_assert($3, $5.value, $5.length); }
	;

/* Handle the --defsym option.  */
defsym_expr:
	  string '=' parse_exp
	    { script_set_symbol(closure, $1.value, $1.length, $3, 0, 0); }
	;

/* Handle the --dynamic-list option.  A dynamic list has the format
   { sym1; sym2; extern "C++" { namespace::sym3 }; };
   We store the symbol we see in the "local" list; that is where
   Command_line::in_dynamic_list() will look to do its check.
   TODO(csilvers): More than one of these brace-lists can appear, and
   should just be merged and treated as a single list.  */
dynamic_list_expr: dynamic_list_nodes ;

dynamic_list_nodes:
	  dynamic_list_node
	| dynamic_list_nodes dynamic_list_node
        ;

dynamic_list_node:
          '{' vers_defns ';' '}' ';'
            { script_new_vers_node (closure, NULL, $2); }
        ;

/* A version script.  */
version_script:
	  vers_nodes
	;

vers_nodes:
	  vers_node
	| vers_nodes vers_node
	;

vers_node:
	  '{' vers_tag '}' ';'
	    {
	      script_register_vers_node (closure, NULL, 0, $2, NULL);
	    }
	| string '{' vers_tag '}' ';'
	    {
	      script_register_vers_node (closure, $1.value, $1.length, $3,
					 NULL);
	    }
	| string '{' vers_tag '}' verdep ';'
	    {
	      script_register_vers_node (closure, $1.value, $1.length, $3, $5);
	    }
	;

verdep:
	  string
	    {
	      $$ = script_add_vers_depend (closure, NULL, $1.value, $1.length);
	    }
	| verdep string
	    {
	      $$ = script_add_vers_depend (closure, $1, $2.value, $2.length);
	    }
	;

vers_tag:
	  /* empty */
	    { $$ = script_new_vers_node (closure, NULL, NULL); }
	| vers_defns ';'
	    { $$ = script_new_vers_node (closure, $1, NULL); }
	| GLOBAL ':' vers_defns ';'
	    { $$ = script_new_vers_node (closure, $3, NULL); }
	| LOCAL ':' vers_defns ';'
	    { $$ = script_new_vers_node (closure, NULL, $3); }
	| GLOBAL ':' vers_defns ';' LOCAL ':' vers_defns ';'
	    { $$ = script_new_vers_node (closure, $3, $7); }
	;

/* Here is one of the rare places we care about the distinction
   between STRING and QUOTED_STRING.  For QUOTED_STRING, we do exact
   matching on the pattern, so we pass in true for the exact_match
   parameter.  For STRING, we do glob matching and pass in false.  */
vers_defns:
	  STRING
	    {
	      $$ = script_new_vers_pattern (closure, NULL, $1.value,
					    $1.length, 0);
	    }
	| QUOTED_STRING
	    {
	      $$ = script_new_vers_pattern (closure, NULL, $1.value,
					    $1.length, 1);
	    }
	| vers_defns ';' STRING
	    {
	      $$ = script_new_vers_pattern (closure, $1, $3.value,
                                            $3.length, 0);
	    }
	| vers_defns ';' QUOTED_STRING
	    {
	      $$ = script_new_vers_pattern (closure, $1, $3.value,
                                            $3.length, 1);
	    }
        | /* Push string on the language stack. */
          EXTERN string '{'
	    { version_script_push_lang (closure, $2.value, $2.length); }
	  vers_defns opt_semicolon '}'
	    {
	      $$ = $5;
	      version_script_pop_lang(closure);
	    }
        | /* Push string on the language stack.  This is more complicated
             than the other cases because we need to merge the linked-list
             state from the pre-EXTERN defns and the post-EXTERN defns.  */
          vers_defns ';' EXTERN string '{'
	    { version_script_push_lang (closure, $4.value, $4.length); }
	  vers_defns opt_semicolon '}'
	    {
	      $$ = script_merge_expressions ($1, $7);
	      version_script_pop_lang(closure);
	    }
        | EXTERN  // "extern" as a symbol name
	    {
	      $$ = script_new_vers_pattern (closure, NULL, "extern",
					    sizeof("extern") - 1, 1);
	    }
	| vers_defns ';' EXTERN
	    {
	      $$ = script_new_vers_pattern (closure, $1, "extern",
					    sizeof("extern") - 1, 1);
	    }
	;

/* A string can be either a STRING or a QUOTED_STRING.  Almost all the
   time we don't care, and we use this rule.  */
string:
          STRING
	    { $$ = $1; }
	| QUOTED_STRING
	    { $$ = $1; }
	;

/* Some statements require a terminator, which may be a semicolon or a
   comma.  */
end:
	  ';'
	| ','
	;

/* An optional semicolon.  */
opt_semicolon:
	  ';'
	|  /* empty */
	;

/* An optional comma.  */
opt_comma:
	  ','
	| /* empty */
	;

%%
