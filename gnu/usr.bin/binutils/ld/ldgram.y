/* A YACC grammar to parse a superset of the AT&T linker scripting language.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support (steve@cygnus.com).

   This file is part of the GNU Binutils.

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

%{
/*

 */

#define DONTDECLARE_MALLOC

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ctf-api.h"
#include "ld.h"
#include "ldexp.h"
#include "ldver.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"
#include "ldmisc.h"
#include "ldmain.h"
#include "mri.h"
#include "ldctor.h"
#include "ldlex.h"

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

static enum section_type sectype;
static etree_type *sectype_value;
static lang_memory_region_type *region;

static bool ldgram_had_keep = false;
static char *ldgram_vers_current_lang = NULL;

#define ERROR_NAME_MAX 20
static char *error_names[ERROR_NAME_MAX];
static int error_index;
#define PUSH_ERROR(x) if (error_index < ERROR_NAME_MAX) error_names[error_index] = x; error_index++;
#define POP_ERROR()   error_index--;
%}
%union {
  bfd_vma integer;
  struct big_int
    {
      bfd_vma integer;
      char *str;
    } bigint;
  fill_type *fill;
  char *name;
  const char *cname;
  struct wildcard_spec wildcard;
  struct wildcard_list *wildcard_list;
  struct name_list *name_list;
  struct flag_info_list *flag_info_list;
  struct flag_info *flag_info;
  int token;
  union etree_union *etree;
  struct phdr_info
    {
      bool filehdr;
      bool phdrs;
      union etree_union *at;
      union etree_union *flags;
    } phdr;
  struct lang_nocrossref *nocrossref;
  struct lang_output_section_phdr_list *section_phdr;
  struct bfd_elf_version_deps *deflist;
  struct bfd_elf_version_expr *versyms;
  struct bfd_elf_version_tree *versnode;
}

%type <etree> exp opt_exp_with_type mustbe_exp opt_at phdr_type phdr_val
%type <etree> opt_exp_without_type opt_subalign opt_align
%type <fill> fill_opt fill_exp
%type <name_list> exclude_name_list
%type <wildcard_list> section_name_list
%type <flag_info_list> sect_flag_list
%type <flag_info> sect_flags
%type <name> memspec_opt memspec_at_opt paren_script_name casesymlist
%type <cname> wildcard_name
%type <wildcard> section_name_spec filename_spec wildcard_maybe_exclude
%token <bigint> INT
%token <name> NAME LNAME
%type <integer> length
%type <phdr> phdr_qualifiers
%type <nocrossref> nocrossref_list
%type <section_phdr> phdr_opt
%type <integer> opt_nocrossrefs

%right <token> PLUSEQ MINUSEQ MULTEQ DIVEQ  '=' LSHIFTEQ RSHIFTEQ   ANDEQ OREQ
%right <token> '?' ':'
%left <token> OROR
%left <token>  ANDAND
%left <token> '|'
%left <token>  '^'
%left  <token> '&'
%left <token>  EQ NE
%left  <token> '<' '>' LE GE
%left  <token> LSHIFT RSHIFT

%left  <token> '+' '-'
%left  <token> '*' '/' '%'

%right UNARY
%token END
%left <token> '('
%token <token> ALIGN_K BLOCK BIND QUAD SQUAD LONG SHORT BYTE ASCIZ
%token SECTIONS PHDRS INSERT_K AFTER BEFORE LINKER_VERSION
%token DATA_SEGMENT_ALIGN DATA_SEGMENT_RELRO_END DATA_SEGMENT_END
%token SORT_BY_NAME SORT_BY_ALIGNMENT SORT_NONE
%token SORT_BY_INIT_PRIORITY
%token '{' '}'
%token SIZEOF_HEADERS OUTPUT_FORMAT FORCE_COMMON_ALLOCATION OUTPUT_ARCH
%token INHIBIT_COMMON_ALLOCATION FORCE_GROUP_ALLOCATION
%token SEGMENT_START
%token INCLUDE
%token MEMORY
%token REGION_ALIAS
%token LD_FEATURE
%token NOLOAD DSECT COPY INFO OVERLAY
%token READONLY
%token TYPE
%token DEFINED TARGET_K SEARCH_DIR MAP ENTRY
%token <integer> NEXT
%token SIZEOF ALIGNOF ADDR LOADADDR MAX_K MIN_K
%token STARTUP HLL SYSLIB FLOAT NOFLOAT NOCROSSREFS NOCROSSREFS_TO
%token ORIGIN FILL
%token LENGTH CREATE_OBJECT_SYMBOLS INPUT GROUP OUTPUT CONSTRUCTORS
%token ALIGNMOD AT SUBALIGN HIDDEN PROVIDE PROVIDE_HIDDEN AS_NEEDED
%type <token> assign_op atype attributes_opt sect_constraint opt_align_with_input
%type <name>  filename
%token CHIP LIST SECT ABSOLUTE  LOAD NEWLINE ENDWORD ORDER NAMEWORD ASSERT_K
%token LOG2CEIL FORMAT PUBLIC DEFSYMEND BASE ALIAS TRUNCATE REL
%token INPUT_SCRIPT INPUT_MRI_SCRIPT INPUT_DEFSYM CASE EXTERN START
%token <name> VERS_TAG VERS_IDENTIFIER
%token GLOBAL LOCAL VERSIONK INPUT_VERSION_SCRIPT
%token KEEP ONLY_IF_RO ONLY_IF_RW SPECIAL INPUT_SECTION_FLAGS ALIGN_WITH_INPUT
%token EXCLUDE_FILE
%token CONSTANT
%type <versyms> vers_defns
%type <versnode> vers_tag
%type <deflist> verdep
%token INPUT_DYNAMIC_LIST

%%

file:
		INPUT_SCRIPT script_file
	|	INPUT_MRI_SCRIPT mri_script_file
	|	INPUT_VERSION_SCRIPT version_script_file
	|	INPUT_DYNAMIC_LIST dynamic_list_file
	|	INPUT_DEFSYM defsym_expr
	;


filename:  NAME;


defsym_expr:
		{ ldlex_expression(); }
		assignment
		{ ldlex_popstate(); }
	;

/* SYNTAX WITHIN AN MRI SCRIPT FILE */
mri_script_file:
		{
		  ldlex_mri_script ();
		  PUSH_ERROR (_("MRI style script"));
		}
	     mri_script_lines
		{
		  ldlex_popstate ();
		  mri_draw_tree ();
		  POP_ERROR ();
		}
	;

mri_script_lines:
		mri_script_lines mri_script_command NEWLINE
	|
	;

mri_script_command:
		CHIP  exp
	|	CHIP  exp ',' exp
	|	NAME	{
			einfo(_("%F%P: unrecognised keyword in MRI style script '%s'\n"),$1);
			}
	|	LIST	{
			config.map_filename = "-";
			}
	|	ORDER ordernamelist
	|	ENDWORD
	|	PUBLIC NAME '=' exp
			{ mri_public($2, $4); }
	|	PUBLIC NAME ',' exp
			{ mri_public($2, $4); }
	|	PUBLIC NAME  exp
			{ mri_public($2, $3); }
	|	FORMAT NAME
			{ mri_format($2); }
	|	SECT NAME ',' exp
			{ mri_output_section($2, $4);}
	|	SECT NAME  exp
			{ mri_output_section($2, $3);}
	|	SECT NAME '=' exp
			{ mri_output_section($2, $4);}
	|	ALIGN_K NAME '=' exp
			{ mri_align($2,$4); }
	|	ALIGN_K NAME ',' exp
			{ mri_align($2,$4); }
	|	ALIGNMOD NAME '=' exp
			{ mri_alignmod($2,$4); }
	|	ALIGNMOD NAME ',' exp
			{ mri_alignmod($2,$4); }
	|	ABSOLUTE mri_abs_name_list
	|	LOAD	 mri_load_name_list
	|	NAMEWORD NAME
			{ mri_name($2); }
	|	ALIAS NAME ',' NAME
			{ mri_alias($2,$4,0);}
	|	ALIAS NAME ',' INT
			{ mri_alias ($2, 0, (int) $4.integer); }
	|	BASE	 exp
			{ mri_base($2); }
	|	TRUNCATE INT
		{ mri_truncate ((unsigned int) $2.integer); }
	|	CASE casesymlist
	|	EXTERN extern_name_list
	|	INCLUDE filename
		{ ldfile_open_command_file ($2); }
		mri_script_lines END
	|	START NAME
		{ lang_add_entry ($2, false); }
	|
	;

ordernamelist:
	      ordernamelist ',' NAME	     { mri_order($3); }
	|     ordernamelist  NAME	  { mri_order($2); }
	|
	;

mri_load_name_list:
		NAME
			{ mri_load($1); }
	|	mri_load_name_list ',' NAME { mri_load($3); }
	;

mri_abs_name_list:
		NAME
			{ mri_only_load($1); }
	|	mri_abs_name_list ','  NAME
			{ mri_only_load($3); }
	;

casesymlist:
	  /* empty */ { $$ = NULL; }
	| NAME
	| casesymlist ',' NAME
	;

extern_name_list:
	NAME
			{ ldlang_add_undef ($1, false); }
	| extern_name_list NAME
			{ ldlang_add_undef ($2, false); }
	| extern_name_list ',' NAME
			{ ldlang_add_undef ($3, false); }
	;

script_file:
	{ ldlex_script (); }
	ifile_list
	{ ldlex_popstate (); }
	;

ifile_list:
	ifile_list ifile_p1
	|
	;


ifile_p1:
		memory
	|	sections
	|	phdrs
	|	startup
	|	high_level_library
	|	low_level_library
	|	floating_point_support
	|	statement_anywhere
	|	version
	|	 ';'
	|	TARGET_K '(' NAME ')'
		{ lang_add_target($3); }
	|	SEARCH_DIR '(' filename ')'
		{ ldfile_add_library_path ($3, false); }
	|	OUTPUT '(' filename ')'
		{ lang_add_output($3, 1); }
	|	OUTPUT_FORMAT '(' NAME ')'
		  { lang_add_output_format ($3, (char *) NULL,
					    (char *) NULL, 1); }
	|	OUTPUT_FORMAT '(' NAME ',' NAME ',' NAME ')'
		  { lang_add_output_format ($3, $5, $7, 1); }
	|	OUTPUT_ARCH '(' NAME ')'
		  { ldfile_set_output_arch ($3, bfd_arch_unknown); }
	|	FORCE_COMMON_ALLOCATION
		{ command_line.force_common_definition = true ; }
	|	FORCE_GROUP_ALLOCATION
		{ command_line.force_group_allocation = true ; }
	|	INHIBIT_COMMON_ALLOCATION
		{ link_info.inhibit_common_definition = true ; }
	|	INPUT '(' input_list ')'
	|	GROUP
		  { lang_enter_group (); }
		    '(' input_list ')'
		  { lang_leave_group (); }
	|	MAP '(' filename ')'
		{ lang_add_map($3); }
	|	INCLUDE filename
		{ ldfile_open_command_file ($2); }
		ifile_list END
	|	NOCROSSREFS '(' nocrossref_list ')'
		{
		  lang_add_nocrossref ($3);
		}
	|	NOCROSSREFS_TO '(' nocrossref_list ')'
		{
		  lang_add_nocrossref_to ($3);
		}
	|	EXTERN '(' { ldlex_expression (); } extern_name_list ')'
			{ ldlex_popstate (); }
	|	INSERT_K AFTER NAME
		{ lang_add_insert ($3, 0); }
	|	INSERT_K BEFORE NAME
		{ lang_add_insert ($3, 1); }
	|	REGION_ALIAS '(' NAME ',' NAME ')'
		{ lang_memory_region_alias ($3, $5); }
	|	LD_FEATURE '(' NAME ')'
		{ lang_ld_feature ($3); }
	;

input_list:
		{ ldlex_inputlist(); }
		input_list1
		{ ldlex_popstate(); }

input_list1:
		NAME
		{ lang_add_input_file($1,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	input_list1 ',' NAME
		{ lang_add_input_file($3,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	input_list1 NAME
		{ lang_add_input_file($2,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	LNAME
		{ lang_add_input_file($1,lang_input_file_is_l_enum,
				 (char *)NULL); }
	|	input_list1 ',' LNAME
		{ lang_add_input_file($3,lang_input_file_is_l_enum,
				 (char *)NULL); }
	|	input_list1 LNAME
		{ lang_add_input_file($2,lang_input_file_is_l_enum,
				 (char *)NULL); }
	|	AS_NEEDED '('
		  { $<integer>$ = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = true; }
		     input_list1 ')'
		  { input_flags.add_DT_NEEDED_for_regular = $<integer>3; }
	|	input_list1 ',' AS_NEEDED '('
		  { $<integer>$ = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = true; }
		     input_list1 ')'
		  { input_flags.add_DT_NEEDED_for_regular = $<integer>5; }
	|	input_list1 AS_NEEDED '('
		  { $<integer>$ = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = true; }
		     input_list1 ')'
		  { input_flags.add_DT_NEEDED_for_regular = $<integer>4; }
	;

sections:
		SECTIONS '{' sec_or_group_p1 '}'
	;

sec_or_group_p1:
		sec_or_group_p1 section
	|	sec_or_group_p1 statement_anywhere
	|
	;

statement_anywhere:
		ENTRY '(' NAME ')'
		{ lang_add_entry ($3, false); }
	|	assignment separator
	|	ASSERT_K  {ldlex_expression ();} '(' exp ',' NAME ')'
		{ ldlex_popstate ();
		  lang_add_assignment (exp_assert ($4, $6)); }
	;

wildcard_name:
		NAME
			{
			  $$ = $1;
			}
	;

wildcard_maybe_exclude:
		wildcard_name
			{
			  $$.name = $1;
			  $$.sorted = none;
			  $$.exclude_name_list = NULL;
			  $$.section_flag_list = NULL;
			}
	|	EXCLUDE_FILE '(' exclude_name_list ')' wildcard_name
			{
			  $$.name = $5;
			  $$.sorted = none;
			  $$.exclude_name_list = $3;
			  $$.section_flag_list = NULL;
			}
	;

filename_spec:
		wildcard_maybe_exclude
	|	SORT_BY_NAME '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_name;
			}
	|	SORT_NONE '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_none;
			}
	;

section_name_spec:
		wildcard_maybe_exclude
	|	SORT_BY_NAME '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_name;
			}
	|	SORT_BY_ALIGNMENT '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_alignment;
			}
	|	SORT_NONE '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_none;
			}
	|	SORT_BY_NAME '(' SORT_BY_ALIGNMENT '(' wildcard_maybe_exclude ')' ')'
			{
			  $$ = $5;
			  $$.sorted = by_name_alignment;
			}
	|	SORT_BY_NAME '(' SORT_BY_NAME '(' wildcard_maybe_exclude ')' ')'
			{
			  $$ = $5;
			  $$.sorted = by_name;
			}
	|	SORT_BY_ALIGNMENT '(' SORT_BY_NAME '(' wildcard_maybe_exclude ')' ')'
			{
			  $$ = $5;
			  $$.sorted = by_alignment_name;
			}
	|	SORT_BY_ALIGNMENT '(' SORT_BY_ALIGNMENT '(' wildcard_maybe_exclude ')' ')'
			{
			  $$ = $5;
			  $$.sorted = by_alignment;
			}
	|	SORT_BY_INIT_PRIORITY '(' wildcard_maybe_exclude ')'
			{
			  $$ = $3;
			  $$.sorted = by_init_priority;
			}
	;

sect_flag_list:	NAME
			{
			  struct flag_info_list *n;
			  n = ((struct flag_info_list *) xmalloc (sizeof *n));
			  if ($1[0] == '!')
			    {
			      n->with = without_flags;
			      n->name = &$1[1];
			    }
			  else
			    {
			      n->with = with_flags;
			      n->name = $1;
			    }
			  n->valid = false;
			  n->next = NULL;
			  $$ = n;
			}
	|	sect_flag_list '&' NAME
			{
			  struct flag_info_list *n;
			  n = ((struct flag_info_list *) xmalloc (sizeof *n));
			  if ($3[0] == '!')
			    {
			      n->with = without_flags;
			      n->name = &$3[1];
			    }
			  else
			    {
			      n->with = with_flags;
			      n->name = $3;
			    }
			  n->valid = false;
			  n->next = $1;
			  $$ = n;
			}
	;

sect_flags:
		INPUT_SECTION_FLAGS '(' sect_flag_list ')'
			{
			  struct flag_info *n;
			  n = ((struct flag_info *) xmalloc (sizeof *n));
			  n->flag_list = $3;
			  n->flags_initialized = false;
			  n->not_with_flags = 0;
			  n->only_with_flags = 0;
			  $$ = n;
			}
	;

exclude_name_list:
		exclude_name_list wildcard_name
			{
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = $2;
			  tmp->next = $1;
			  $$ = tmp;
			}
	|
		wildcard_name
			{
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = $1;
			  tmp->next = NULL;
			  $$ = tmp;
			}
	;

section_name_list:
		section_name_list opt_comma section_name_spec
			{
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = $1;
			  tmp->spec = $3;
			  $$ = tmp;
			}
	|
		section_name_spec
			{
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = NULL;
			  tmp->spec = $1;
			  $$ = tmp;
			}
	;

input_section_spec_no_keep:
		NAME
			{
			  struct wildcard_spec tmp;
			  tmp.name = $1;
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = NULL;
			  lang_add_wild (&tmp, NULL, ldgram_had_keep);
			}
	|	sect_flags NAME
			{
			  struct wildcard_spec tmp;
			  tmp.name = $2;
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = $1;
			  lang_add_wild (&tmp, NULL, ldgram_had_keep);
			}
	|	'[' section_name_list ']'
			{
			  lang_add_wild (NULL, $2, ldgram_had_keep);
			}
	|	sect_flags '[' section_name_list ']'
			{
			  struct wildcard_spec tmp;
			  tmp.name = NULL;
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = $1;
			  lang_add_wild (&tmp, $3, ldgram_had_keep);
			}
	|	filename_spec '(' section_name_list ')'
			{
			  lang_add_wild (&$1, $3, ldgram_had_keep);
			}
	|	sect_flags filename_spec '(' section_name_list ')'
			{
			  $2.section_flag_list = $1;
			  lang_add_wild (&$2, $4, ldgram_had_keep);
			}
	;

input_section_spec:
		input_section_spec_no_keep
	|	KEEP '('
			{ ldgram_had_keep = true; }
		input_section_spec_no_keep ')'
			{ ldgram_had_keep = false; }
	;

statement:
	';'
	| assignment separator
	| CREATE_OBJECT_SYMBOLS
		{
		  lang_add_attribute (lang_object_symbols_statement_enum);
		}
	| CONSTRUCTORS
		{
		  lang_add_attribute (lang_constructors_statement_enum);
		}
	| SORT_BY_NAME '(' CONSTRUCTORS ')'
		{
		  constructors_sorted = true;
		  lang_add_attribute (lang_constructors_statement_enum);
		}
	| input_section_spec
	| length '(' mustbe_exp ')'
		{
		  lang_add_data ((int) $1, $3);
		}
	| ASCIZ NAME
		{
		  lang_add_string ($2);
		}
	| FILL '(' fill_exp ')'
		{
		  lang_add_fill ($3);
		}
	| LINKER_VERSION
		{
		  lang_add_version_string ();
		}
	| ASSERT_K
		{ ldlex_expression (); }
	  '(' exp ',' NAME ')' separator
		{
		  ldlex_popstate ();
		  lang_add_assignment (exp_assert ($4, $6));
		}
	| INCLUDE filename
		{
		  ldfile_open_command_file ($2);
		}
	  statement_list_opt END
	;

statement_list:
		statement_list statement
	|	statement
	;

statement_list_opt:
		/* empty */
	|	statement_list
	;

length:
		QUAD
			{ $$ = $1; }
	|	SQUAD
			{ $$ = $1; }
	|	LONG
			{ $$ = $1; }
	|	SHORT
			{ $$ = $1; }
	|	BYTE
			{ $$ = $1; }
	;

fill_exp:
	mustbe_exp
		{
		  $$ = exp_get_fill ($1, 0, _("fill value"));
		}
	;

fill_opt:
	  '=' fill_exp
		{ $$ = $2; }
	|	{ $$ = (fill_type *) 0; }
	;

assign_op:
		PLUSEQ
			{ $$ = '+'; }
	|	MINUSEQ
			{ $$ = '-'; }
	|	MULTEQ
			{ $$ = '*'; }
	|	DIVEQ
			{ $$ = '/'; }
	|	LSHIFTEQ
			{ $$ = LSHIFT; }
	|	RSHIFTEQ
			{ $$ = RSHIFT; }
	|	ANDEQ
			{ $$ = '&'; }
	|	OREQ
			{ $$ = '|'; }

	;

separator:	';' | ','
	;


assignment:
		NAME '=' mustbe_exp
		{
		  lang_add_assignment (exp_assign ($1, $3, false));
		}
	|	NAME assign_op mustbe_exp
		{
		  lang_add_assignment (exp_assign ($1,
						   exp_binop ($2,
							      exp_nameop (NAME,
									  $1),
							      $3), false));
		}
	|	HIDDEN '(' NAME '=' mustbe_exp ')'
		{
		  lang_add_assignment (exp_assign ($3, $5, true));
		}
	|	PROVIDE '(' NAME '=' mustbe_exp ')'
		{
		  lang_add_assignment (exp_provide ($3, $5, false));
		}
	|	PROVIDE_HIDDEN '(' NAME '=' mustbe_exp ')'
		{
		  lang_add_assignment (exp_provide ($3, $5, true));
		}
	;


opt_comma:
		','	|	;


memory:
		MEMORY '{' memory_spec_list_opt '}'
	;

memory_spec_list_opt: memory_spec_list | ;

memory_spec_list:
		memory_spec_list opt_comma memory_spec
	|	memory_spec
	;


memory_spec:	NAME
		{ region = lang_memory_region_lookup ($1, true); }
		attributes_opt ':'
		origin_spec opt_comma length_spec
		{}
	|	INCLUDE filename
		{ ldfile_open_command_file ($2); }
		memory_spec_list_opt END
	;

origin_spec:
	ORIGIN '=' mustbe_exp
		{
		  region->origin_exp = $3;
		}
	;

length_spec:
	     LENGTH '=' mustbe_exp
		{
		  if (yychar == NAME)
		    {
		      yyclearin;
		      ldlex_backup ();
		    }
		  region->length_exp = $3;
		}
	;

attributes_opt:
		/* empty */
		  { /* dummy action to avoid bison 1.25 error message */ }
	|	'(' attributes_list ')'
	;

attributes_list:
		attributes_string
	|	attributes_list attributes_string
	;

attributes_string:
		NAME
		  { lang_set_flags (region, $1, 0); }
	|	'!' NAME
		  { lang_set_flags (region, $2, 1); }
	;

startup:
	STARTUP '(' filename ')'
		{ lang_startup($3); }
	;

high_level_library:
		HLL '(' high_level_library_NAME_list ')'
	|	HLL '(' ')'
			{ ldemul_hll((char *)NULL); }
	;

high_level_library_NAME_list:
		high_level_library_NAME_list opt_comma filename
			{ ldemul_hll($3); }
	|	filename
			{ ldemul_hll($1); }
	;

low_level_library:
	SYSLIB '(' low_level_library_NAME_list ')'
	;

low_level_library_NAME_list:
		low_level_library_NAME_list opt_comma filename
			{ ldemul_syslib($3); }
	|
	;

floating_point_support:
		FLOAT
			{ lang_float(true); }
	|	NOFLOAT
			{ lang_float(false); }
	;

nocrossref_list:
		/* empty */
		{
		  $$ = NULL;
		}
	|	NAME nocrossref_list
		{
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = $1;
		  n->next = $2;
		  $$ = n;
		}
	|	NAME ',' nocrossref_list
		{
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = $1;
		  n->next = $3;
		  $$ = n;
		}
	;

paren_script_name:	{ ldlex_script (); }
		'(' NAME ')'
			{ ldlex_popstate (); $$ = $3; }

mustbe_exp:		{ ldlex_expression (); }
		exp
			{ ldlex_popstate (); $$ = $2; }
	;

exp	:
		'-' exp %prec UNARY
			{ $$ = exp_unop ('-', $2); }
	|	'(' exp ')'
			{ $$ = $2; }
	|	NEXT '(' exp ')' %prec UNARY
			{ $$ = exp_unop ((int) $1,$3); }
	|	'!' exp %prec UNARY
			{ $$ = exp_unop ('!', $2); }
	|	'+' exp %prec UNARY
			{ $$ = $2; }
	|	'~' exp %prec UNARY
			{ $$ = exp_unop ('~', $2);}

	|	exp '*' exp
			{ $$ = exp_binop ('*', $1, $3); }
	|	exp '/' exp
			{ $$ = exp_binop ('/', $1, $3); }
	|	exp '%' exp
			{ $$ = exp_binop ('%', $1, $3); }
	|	exp '+' exp
			{ $$ = exp_binop ('+', $1, $3); }
	|	exp '-' exp
			{ $$ = exp_binop ('-' , $1, $3); }
	|	exp LSHIFT exp
			{ $$ = exp_binop (LSHIFT , $1, $3); }
	|	exp RSHIFT exp
			{ $$ = exp_binop (RSHIFT , $1, $3); }
	|	exp EQ exp
			{ $$ = exp_binop (EQ , $1, $3); }
	|	exp NE exp
			{ $$ = exp_binop (NE , $1, $3); }
	|	exp LE exp
			{ $$ = exp_binop (LE , $1, $3); }
	|	exp GE exp
			{ $$ = exp_binop (GE , $1, $3); }
	|	exp '<' exp
			{ $$ = exp_binop ('<' , $1, $3); }
	|	exp '>' exp
			{ $$ = exp_binop ('>' , $1, $3); }
	|	exp '&' exp
			{ $$ = exp_binop ('&' , $1, $3); }
	|	exp '^' exp
			{ $$ = exp_binop ('^' , $1, $3); }
	|	exp '|' exp
			{ $$ = exp_binop ('|' , $1, $3); }
	|	exp '?' exp ':' exp
			{ $$ = exp_trinop ('?' , $1, $3, $5); }
	|	exp ANDAND exp
			{ $$ = exp_binop (ANDAND , $1, $3); }
	|	exp OROR exp
			{ $$ = exp_binop (OROR , $1, $3); }
	|	DEFINED '(' NAME ')'
			{ $$ = exp_nameop (DEFINED, $3); }
	|	INT
			{ $$ = exp_bigintop ($1.integer, $1.str); }
	|	SIZEOF_HEADERS
			{ $$ = exp_nameop (SIZEOF_HEADERS,0); }

	|	ALIGNOF paren_script_name
			{ $$ = exp_nameop (ALIGNOF, $2); }
	|	SIZEOF	paren_script_name
			{ $$ = exp_nameop (SIZEOF, $2); }
	|	ADDR	paren_script_name
			{ $$ = exp_nameop (ADDR, $2); }
	|	LOADADDR paren_script_name
			{ $$ = exp_nameop (LOADADDR, $2); }
	|	CONSTANT '(' NAME ')'
			{ $$ = exp_nameop (CONSTANT,$3); }
	|	ABSOLUTE '(' exp ')'
			{ $$ = exp_unop (ABSOLUTE, $3); }
	|	ALIGN_K '(' exp ')'
			{ $$ = exp_unop (ALIGN_K,$3); }
	|	ALIGN_K '(' exp ',' exp ')'
			{ $$ = exp_binop (ALIGN_K,$3,$5); }
	|	DATA_SEGMENT_ALIGN '(' exp ',' exp ')'
			{ $$ = exp_binop (DATA_SEGMENT_ALIGN, $3, $5); }
	|	DATA_SEGMENT_RELRO_END '(' exp ',' exp ')'
			{ $$ = exp_binop (DATA_SEGMENT_RELRO_END, $5, $3); }
	|	DATA_SEGMENT_END '(' exp ')'
			{ $$ = exp_unop (DATA_SEGMENT_END, $3); }
	|	SEGMENT_START { ldlex_script (); } '(' NAME
			{ ldlex_popstate (); } ',' exp ')'
			{ /* The operands to the expression node are
			     placed in the opposite order from the way
			     in which they appear in the script as
			     that allows us to reuse more code in
			     fold_binary.  */
			  $$ = exp_binop (SEGMENT_START,
					  $7,
					  exp_nameop (NAME, $4)); }
	|	BLOCK '(' exp ')'
			{ $$ = exp_unop (ALIGN_K,$3); }
	|	NAME
			{ $$ = exp_nameop (NAME,$1); }
	|	MAX_K '(' exp ',' exp ')'
			{ $$ = exp_binop (MAX_K, $3, $5 ); }
	|	MIN_K '(' exp ',' exp ')'
			{ $$ = exp_binop (MIN_K, $3, $5 ); }
	|	ASSERT_K '(' exp ',' NAME ')'
			{ $$ = exp_assert ($3, $5); }
	|	ORIGIN paren_script_name
			{ $$ = exp_nameop (ORIGIN, $2); }
	|	LENGTH paren_script_name
			{ $$ = exp_nameop (LENGTH, $2); }
	|	LOG2CEIL '(' exp ')'
			{ $$ = exp_unop (LOG2CEIL, $3); }
	;


memspec_at_opt:
		AT '>' NAME { $$ = $3; }
	|	{ $$ = 0; }
	;

opt_at:
		AT '(' exp ')' { $$ = $3; }
	|	{ $$ = 0; }
	;

opt_align:
		ALIGN_K '(' exp ')' { $$ = $3; }
	|	{ $$ = 0; }
	;

opt_align_with_input:
		ALIGN_WITH_INPUT { $$ = ALIGN_WITH_INPUT; }
	|	{ $$ = 0; }
	;

opt_subalign:
		SUBALIGN '(' exp ')' { $$ = $3; }
	|	{ $$ = 0; }
	;

sect_constraint:
		ONLY_IF_RO { $$ = ONLY_IF_RO; }
	|	ONLY_IF_RW { $$ = ONLY_IF_RW; }
	|	SPECIAL { $$ = SPECIAL; }
	|	{ $$ = 0; }
	;

section:	NAME
			{ ldlex_expression(); }
		opt_exp_with_type
		opt_at
		opt_align
		opt_align_with_input
		opt_subalign
		sect_constraint
			{
			  ldlex_popstate ();
			  ldlex_wild ();
			  lang_enter_output_section_statement ($1, $3, sectype,
					sectype_value, $5, $7, $4, $8, $6);
			}
		'{'
		statement_list_opt
		'}'
			{ ldlex_popstate (); }
		memspec_opt memspec_at_opt phdr_opt fill_opt
			{
			  /* fill_opt may have switched the lexer into
			     expression state, and back again, but in
			     order to find the end of the fill
			     expression the parser must look ahead one
			     token.  If it is a NAME, throw it away as
			     it will have been lexed in the wrong
			     state.  */
			  if (yychar == NAME)
			    {
			      yyclearin;
			      ldlex_backup ();
			    }
			  lang_leave_output_section_statement ($17, $14,
							       $16, $15);
			}
		opt_comma
	|	OVERLAY
			{ ldlex_expression (); }
		opt_exp_without_type opt_nocrossrefs opt_at opt_subalign
			{ ldlex_popstate (); }
		'{'
			{
			  lang_enter_overlay ($3, $6);
			}
		overlay_section
		'}'
		memspec_opt memspec_at_opt phdr_opt fill_opt
			{
			  if (yychar == NAME)
			    {
			      yyclearin;
			      ldlex_backup ();
			    }
			  lang_leave_overlay ($5, (int) $4,
					      $15, $12, $14, $13);
			}
		opt_comma
	|	/* The GROUP case is just enough to support the gcc
		   svr3.ifile script.  It is not intended to be full
		   support.  I'm not even sure what GROUP is supposed
		   to mean.  */
		GROUP
			{ ldlex_expression (); }
		opt_exp_with_type
			{
			  ldlex_popstate ();
			  lang_add_assignment (exp_assign (".", $3, false));
			}
		'{' sec_or_group_p1 '}'
	|	INCLUDE filename
			{
			  ldfile_open_command_file ($2);
			}
		sec_or_group_p1 END
	;

type:
	   NOLOAD  { sectype = noload_section; }
	|  DSECT   { sectype = noalloc_section; }
	|  COPY    { sectype = noalloc_section; }
	|  INFO    { sectype = noalloc_section; }
	|  OVERLAY { sectype = noalloc_section; }
        |  READONLY '(' TYPE '=' exp ')' { sectype = typed_readonly_section; sectype_value = $5; }
	|  READONLY { sectype = readonly_section; }
	|  TYPE '=' exp { sectype = type_section; sectype_value = $3; }
        ;

atype:
		'(' type ')'
	|	/* EMPTY */ { sectype = normal_section; }
	|	'(' ')' { sectype = normal_section; }
	;

opt_exp_with_type:
		exp atype ':'		{ $$ = $1; }
	|	atype ':'		{ $$ = (etree_type *)NULL;  }
	|	/* The BIND cases are to support the gcc svr3.ifile
		   script.  They aren't intended to implement full
		   support for the BIND keyword.  I'm not even sure
		   what BIND is supposed to mean.  */
		BIND '(' exp ')' atype ':' { $$ = $3; }
	|	BIND '(' exp ')' BLOCK '(' exp ')' atype ':'
		{ $$ = $3; }
	;

opt_exp_without_type:
		exp ':'		{ $$ = $1; }
	|	':'		{ $$ = (etree_type *) NULL;  }
	;

opt_nocrossrefs:
		/* empty */
			{ $$ = 0; }
	|	NOCROSSREFS
			{ $$ = 1; }
	;

memspec_opt:
		'>' NAME
		{ $$ = $2; }
	|	{ $$ = DEFAULT_MEMORY_REGION; }
	;

phdr_opt:
		/* empty */
		{
		  $$ = NULL;
		}
	|	phdr_opt ':' NAME
		{
		  struct lang_output_section_phdr_list *n;

		  n = ((struct lang_output_section_phdr_list *)
		       xmalloc (sizeof *n));
		  n->name = $3;
		  n->used = false;
		  n->next = $1;
		  $$ = n;
		}
	;

overlay_section:
		/* empty */
	|	overlay_section
		NAME
			{
			  ldlex_wild ();
			  lang_enter_overlay_section ($2);
			}
		'{'
		statement_list_opt
		'}'
			{ ldlex_popstate (); }
		phdr_opt fill_opt
			{
			  if (yychar == NAME)
			    {
			      yyclearin;
			      ldlex_backup ();
			    }
			  lang_leave_overlay_section ($9, $8);
			}
		opt_comma
	;

phdrs:
		PHDRS '{' phdr_list '}'
	;

phdr_list:
		/* empty */
	|	phdr_list phdr
	;

phdr:
		NAME { ldlex_expression (); }
		  phdr_type phdr_qualifiers { ldlex_popstate (); }
		  ';'
		{
		  lang_new_phdr ($1, $3, $4.filehdr, $4.phdrs, $4.at,
				 $4.flags);
		}
	;

phdr_type:
		exp
		{
		  $$ = $1;

		  if ($1->type.node_class == etree_name
		      && $1->type.node_code == NAME)
		    {
		      const char *s;
		      unsigned int i;
		      static const char * const phdr_types[] =
			{
			  "PT_NULL", "PT_LOAD", "PT_DYNAMIC",
			  "PT_INTERP", "PT_NOTE", "PT_SHLIB",
			  "PT_PHDR", "PT_TLS"
			};

		      s = $1->name.name;
		      for (i = 0;
			   i < sizeof phdr_types / sizeof phdr_types[0];
			   i++)
			if (strcmp (s, phdr_types[i]) == 0)
			  {
			    $$ = exp_intop (i);
			    break;
			  }
		      if (i == sizeof phdr_types / sizeof phdr_types[0])
			{
			  if (strcmp (s, "PT_GNU_EH_FRAME") == 0)
			    $$ = exp_intop (0x6474e550);
			  else if (strcmp (s, "PT_GNU_STACK") == 0)
			    $$ = exp_intop (0x6474e551);
			  else if (strcmp (s, "PT_GNU_RELRO") == 0)
			    $$ = exp_intop (0x6474e552);
			  else if (strcmp (s, "PT_GNU_PROPERTY") == 0)
			    $$ = exp_intop (0x6474e553);
			  else
			    {
			      einfo (_("\
%X%P:%pS: unknown phdr type `%s' (try integer literal)\n"),
				     NULL, s);
			      $$ = exp_intop (0);
			    }
			}
		    }
		}
	;

phdr_qualifiers:
		/* empty */
		{
		  memset (&$$, 0, sizeof (struct phdr_info));
		}
	|	NAME phdr_val phdr_qualifiers
		{
		  $$ = $3;
		  if (strcmp ($1, "FILEHDR") == 0 && $2 == NULL)
		    $$.filehdr = true;
		  else if (strcmp ($1, "PHDRS") == 0 && $2 == NULL)
		    $$.phdrs = true;
		  else if (strcmp ($1, "FLAGS") == 0 && $2 != NULL)
		    $$.flags = $2;
		  else
		    einfo (_("%X%P:%pS: PHDRS syntax error at `%s'\n"),
			   NULL, $1);
		}
	|	AT '(' exp ')' phdr_qualifiers
		{
		  $$ = $5;
		  $$.at = $3;
		}
	;

phdr_val:
		/* empty */
		{
		  $$ = NULL;
		}
	| '(' exp ')'
		{
		  $$ = $2;
		}
	;

dynamic_list_file:
		{
		  ldlex_version_file ();
		  PUSH_ERROR (_("dynamic list"));
		}
		dynamic_list_nodes
		{
		  ldlex_popstate ();
		  POP_ERROR ();
		}
	;

dynamic_list_nodes:
		dynamic_list_node
	|	dynamic_list_nodes dynamic_list_node
	;

dynamic_list_node:
		'{' dynamic_list_tag '}' ';'
	;

dynamic_list_tag:
		vers_defns ';'
		{
		  lang_append_dynamic_list (current_dynamic_list_p, $1);
		}
	;

/* This syntax is used within an external version script file.  */

version_script_file:
		{
		  ldlex_version_file ();
		  PUSH_ERROR (_("VERSION script"));
		}
		vers_nodes
		{
		  ldlex_popstate ();
		  POP_ERROR ();
		}
	;

/* This is used within a normal linker script file.  */

version:
		{
		  ldlex_version_script ();
		}
		VERSIONK '{' vers_nodes '}'
		{
		  ldlex_popstate ();
		}
	;

vers_nodes:
		vers_node
	|	vers_nodes vers_node
	;

vers_node:
		'{' vers_tag '}' ';'
		{
		  lang_register_vers_node (NULL, $2, NULL);
		}
	|	VERS_TAG '{' vers_tag '}' ';'
		{
		  lang_register_vers_node ($1, $3, NULL);
		}
	|	VERS_TAG '{' vers_tag '}' verdep ';'
		{
		  lang_register_vers_node ($1, $3, $5);
		}
	;

verdep:
		VERS_TAG
		{
		  $$ = lang_add_vers_depend (NULL, $1);
		}
	|	verdep VERS_TAG
		{
		  $$ = lang_add_vers_depend ($1, $2);
		}
	;

vers_tag:
		/* empty */
		{
		  $$ = lang_new_vers_node (NULL, NULL);
		}
	|	vers_defns ';'
		{
		  $$ = lang_new_vers_node ($1, NULL);
		}
	|	GLOBAL ':' vers_defns ';'
		{
		  $$ = lang_new_vers_node ($3, NULL);
		}
	|	LOCAL ':' vers_defns ';'
		{
		  $$ = lang_new_vers_node (NULL, $3);
		}
	|	GLOBAL ':' vers_defns ';' LOCAL ':' vers_defns ';'
		{
		  $$ = lang_new_vers_node ($3, $7);
		}
	;

vers_defns:
		VERS_IDENTIFIER
		{
		  $$ = lang_new_vers_pattern (NULL, $1, ldgram_vers_current_lang, false);
		}
	|	NAME
		{
		  $$ = lang_new_vers_pattern (NULL, $1, ldgram_vers_current_lang, true);
		}
	|	vers_defns ';' VERS_IDENTIFIER
		{
		  $$ = lang_new_vers_pattern ($1, $3, ldgram_vers_current_lang, false);
		}
	|	vers_defns ';' NAME
		{
		  $$ = lang_new_vers_pattern ($1, $3, ldgram_vers_current_lang, true);
		}
	|	vers_defns ';' EXTERN NAME '{'
			{
			  $<name>$ = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = $4;
			}
		vers_defns opt_semicolon '}'
			{
			  struct bfd_elf_version_expr *pat;
			  for (pat = $7; pat->next != NULL; pat = pat->next);
			  pat->next = $1;
			  $$ = $7;
			  ldgram_vers_current_lang = $<name>6;
			}
	|	EXTERN NAME '{'
			{
			  $<name>$ = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = $2;
			}
		vers_defns opt_semicolon '}'
			{
			  $$ = $5;
			  ldgram_vers_current_lang = $<name>4;
			}
	|	GLOBAL
		{
		  $$ = lang_new_vers_pattern (NULL, "global", ldgram_vers_current_lang, false);
		}
	|	vers_defns ';' GLOBAL
		{
		  $$ = lang_new_vers_pattern ($1, "global", ldgram_vers_current_lang, false);
		}
	|	LOCAL
		{
		  $$ = lang_new_vers_pattern (NULL, "local", ldgram_vers_current_lang, false);
		}
	|	vers_defns ';' LOCAL
		{
		  $$ = lang_new_vers_pattern ($1, "local", ldgram_vers_current_lang, false);
		}
	|	EXTERN
		{
		  $$ = lang_new_vers_pattern (NULL, "extern", ldgram_vers_current_lang, false);
		}
	|	vers_defns ';' EXTERN
		{
		  $$ = lang_new_vers_pattern ($1, "extern", ldgram_vers_current_lang, false);
		}
	;

opt_semicolon:
		/* empty */
	|	';'
	;

%%
void
yyerror(arg)
     const char *arg;
{
  if (ldfile_assumed_script)
    einfo (_("%P:%s: file format not recognized; treating as linker script\n"),
	   ldlex_filename ());
  if (error_index > 0 && error_index < ERROR_NAME_MAX)
    einfo (_("%F%P:%pS: %s in %s\n"), NULL, arg, error_names[error_index - 1]);
  else
    einfo ("%F%P:%pS: %s\n", NULL, arg);
}
