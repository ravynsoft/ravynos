%{ /* deffilep.y - parser for .def files */

/*   Copyright (C) 1995-2023 Free Software Foundation, Inc.

     This file is part of GNU Binutils.

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

#include "sysdep.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ld.h"
#include "ldmisc.h"
#include "deffile.h"

#define TRACE 0

#define ROUND_UP(a, b) (((a)+((b)-1))&~((b)-1))

#define SYMBOL_LIST_ARRAY_GROW 64

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in ld.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list.  */

#define	yymaxdepth def_maxdepth
#define	yyparse	def_parse
#define	yylex	def_lex
#define	yyerror	def_error
#define	yylval	def_lval
#define	yychar	def_char
#define	yydebug	def_debug
#define	yypact	def_pact
#define	yyr1	def_r1
#define	yyr2	def_r2
#define	yydef	def_def
#define	yychk	def_chk
#define	yypgo	def_pgo
#define	yyact	def_act
#define	yyexca	def_exca
#define yyerrflag def_errflag
#define yynerrs	def_nerrs
#define	yyps	def_ps
#define	yypv	def_pv
#define	yys	def_s
#define	yy_yys	def_yys
#define	yystate	def_state
#define	yytmp	def_tmp
#define	yyv	def_v
#define	yy_yyv	def_yyv
#define	yyval	def_val
#define	yylloc	def_lloc
#define yyreds	def_reds		/* With YYDEBUG defined.  */
#define yytoks	def_toks		/* With YYDEBUG defined.  */
#define yylhs	def_yylhs
#define yylen	def_yylen
#define yydefred def_yydefred
#define yydgoto	def_yydgoto
#define yysindex def_yysindex
#define yyrindex def_yyrindex
#define yygindex def_yygindex
#define yytable	 def_yytable
#define yycheck	 def_yycheck

typedef struct def_pool_str {
  struct def_pool_str *next;
  char data[1];
} def_pool_str;

static def_pool_str *pool_strs = NULL;

static char *def_pool_alloc (size_t sz);
static char *def_pool_strdup (const char *str);
static void def_pool_free (void);

static void def_description (const char *);
static void def_exports (const char *, const char *, int, int, const char *);
static void def_heapsize (int, int);
static void def_import (const char *, const char *, const char *, const char *,
			int, const char *);
static void def_image_name (const char *, bfd_vma, int);
static void def_section (const char *, int);
static void def_section_alt (const char *, const char *);
static void def_stacksize (int, int);
static void def_version (int, int);
static void def_directive (char *);
static void def_aligncomm (char *str, int align);
static void def_exclude_symbols (char *str);
static int def_parse (void);
static void def_error (const char *);
static int def_lex (void);

static int lex_forced_token = 0;
static const char *lex_parse_string = 0;
static const char *lex_parse_string_end = 0;

%}

%union {
  char *id;
  const char *id_const;
  int number;
  bfd_vma vma;
  char *digits;
};

%token NAME LIBRARY DESCRIPTION STACKSIZE_K HEAPSIZE CODE DATAU DATAL
%token SECTIONS EXPORTS IMPORTS VERSIONK BASE CONSTANTU CONSTANTL
%token PRIVATEU PRIVATEL ALIGNCOMM EXCLUDE_SYMBOLS
%token READ WRITE EXECUTE SHARED_K NONAMEU NONAMEL DIRECTIVE EQUAL
%token <id> ID
%token <digits> DIGITS
%type  <number> NUMBER
%type  <vma> VMA opt_base
%type  <digits> opt_digits
%type  <number> opt_ordinal
%type  <number> attr attr_list opt_number exp_opt_list exp_opt
%type  <id> opt_name opt_name2 opt_equal_name anylang_id opt_id
%type  <id> opt_equalequal_name symbol_list
%type  <id_const> keyword_as_name

%%

start: start command
	| command
	;

command:
		NAME opt_name opt_base { def_image_name ($2, $3, 0); }
	|	LIBRARY opt_name opt_base { def_image_name ($2, $3, 1); }
	|	DESCRIPTION ID { def_description ($2);}
	|	STACKSIZE_K NUMBER opt_number { def_stacksize ($2, $3);}
	|	HEAPSIZE NUMBER opt_number { def_heapsize ($2, $3);}
	|	CODE attr_list { def_section ("CODE", $2);}
	|	DATAU attr_list  { def_section ("DATA", $2);}
	|	SECTIONS seclist
	|	EXPORTS explist
	|	IMPORTS implist
	|	VERSIONK NUMBER { def_version ($2, 0);}
	|	VERSIONK NUMBER '.' NUMBER { def_version ($2, $4);}
	|	DIRECTIVE ID { def_directive ($2);}
	|	ALIGNCOMM anylang_id ',' NUMBER { def_aligncomm ($2, $4);}
	|	EXCLUDE_SYMBOLS symbol_list
	;


explist:
		/* EMPTY */
	|	expline
	|	explist expline
	;

expline:
		/* The opt_comma is necessary to support both the usual
		  DEF file syntax as well as .drectve syntax which
		  mandates <expsym>,<expoptlist>.  */
		opt_name2 opt_equal_name opt_ordinal opt_comma exp_opt_list opt_comma opt_equalequal_name
			{ def_exports ($1, $2, $3, $5, $7); }
	;
exp_opt_list:
		/* The opt_comma is necessary to support both the usual
		   DEF file syntax as well as .drectve syntax which
		   allows for comma separated opt list.  */
		exp_opt opt_comma exp_opt_list { $$ = $1 | $3; }
	|	{ $$ = 0; }
	;
exp_opt:
		NONAMEU		{ $$ = 1; }
	|	NONAMEL		{ $$ = 1; }
	|	CONSTANTU	{ $$ = 2; }
	|	CONSTANTL	{ $$ = 2; }
	|	DATAU		{ $$ = 4; }
	|	DATAL		{ $$ = 4; }
	|	PRIVATEU	{ $$ = 8; }
	|	PRIVATEL	{ $$ = 8; }
	;
implist:
		implist impline
	|	impline
	;

impline:
	       ID '=' ID '.' ID '.' ID opt_equalequal_name
		 { def_import ($1, $3, $5, $7, -1, $8); }
       |       ID '=' ID '.' ID '.' NUMBER opt_equalequal_name
				 { def_import ($1, $3, $5,  0, $7, $8); }
       |       ID '=' ID '.' ID opt_equalequal_name
		 { def_import ($1, $3,	0, $5, -1, $6); }
       |       ID '=' ID '.' NUMBER opt_equalequal_name
		 { def_import ($1, $3,	0,  0, $5, $6); }
       |       ID '.' ID '.' ID opt_equalequal_name
		 { def_import( 0, $1, $3, $5, -1, $6); }
       |       ID '.' ID opt_equalequal_name
		 { def_import ( 0, $1,	0, $3, -1, $4); }
;

seclist:
		seclist secline
	|	secline
	;

secline:
	ID attr_list { def_section ($1, $2);}
	| ID ID { def_section_alt ($1, $2);}
	;

attr_list:
	attr_list opt_comma attr { $$ = $1 | $3; }
	| attr { $$ = $1; }
	;

opt_comma:
	','
	|
	;
opt_number: ',' NUMBER { $$=$2;}
	|	   { $$=-1;}
	;

attr:
		READ	{ $$ = 1;}
	|	WRITE	{ $$ = 2;}
	|	EXECUTE	{ $$=4;}
	|	SHARED_K { $$=8;}
	;


keyword_as_name: BASE { $$ = "BASE"; }
	 | CODE { $$ = "CODE"; }
	 | CONSTANTU { $$ = "CONSTANT"; }
	 | CONSTANTL { $$ = "constant"; }
	 | DATAU { $$ = "DATA"; }
	 | DATAL { $$ = "data"; }
	 | DESCRIPTION { $$ = "DESCRIPTION"; }
	 | DIRECTIVE { $$ = "DIRECTIVE"; }
	 | EXCLUDE_SYMBOLS { $$ = "EXCLUDE_SYMBOLS"; }
	 | EXECUTE { $$ = "EXECUTE"; }
	 | EXPORTS { $$ = "EXPORTS"; }
	 | HEAPSIZE { $$ = "HEAPSIZE"; }
	 | IMPORTS { $$ = "IMPORTS"; }
/* Disable LIBRARY keyword as valid symbol-name.  This is necessary
   for libtool, which places this command after EXPORTS command.
   This behavior is illegal by specification, but sadly required by
   by compatibility reasons.
   See PR binutils/13710
	 | LIBRARY { $$ = "LIBRARY"; } */
	 | NAME { $$ = "NAME"; }
	 | NONAMEU { $$ = "NONAME"; }
	 | NONAMEL { $$ = "noname"; }
	 | PRIVATEU { $$ = "PRIVATE"; }
	 | PRIVATEL { $$ = "private"; }
	 | READ { $$ = "READ"; }
	 | SHARED_K  { $$ = "SHARED"; }
	 | STACKSIZE_K { $$ = "STACKSIZE"; }
	 | VERSIONK { $$ = "VERSION"; }
	 | WRITE { $$ = "WRITE"; }
	 ;

opt_name2: ID { $$ = $1; }
	| '.' keyword_as_name
	  {
	    char *name = xmalloc (strlen ($2) + 2);
	    sprintf (name, ".%s", $2);
	    $$ = name;
	  }
	| '.' opt_name2
	  {
	    char *name = def_pool_alloc (strlen ($2) + 2);
	    sprintf (name, ".%s", $2);
	    $$ = name;
	  }
	| keyword_as_name '.' opt_name2
	  {
	    char *name = def_pool_alloc (strlen ($1) + 1 + strlen ($3) + 1);
	    sprintf (name, "%s.%s", $1, $3);
	    $$ = name;
	  }
	| ID '.' opt_name2
	  {
	    char *name = def_pool_alloc (strlen ($1) + 1 + strlen ($3) + 1);
	    sprintf (name, "%s.%s", $1, $3);
	    $$ = name;
	  }
	;

opt_name: opt_name2 { $$ = $1; }
	|		{ $$ = ""; }
	;

opt_equalequal_name: EQUAL ID	{ $$ = $2; }
	|							{ $$ = 0; }
	;

opt_ordinal:
	  '@' NUMBER     { $$ = $2;}
	|                { $$ = -1;}
	;

opt_equal_name:
	  '=' opt_name2	{ $$ = $2; }
	|		{ $$ =	0; }
	;

opt_base: BASE	'=' VMA	{ $$ = $3;}
	|	{ $$ = (bfd_vma) -1;}
	;

anylang_id: ID		{ $$ = $1; }
	| '.' ID
	  {
	    char *id = def_pool_alloc (strlen ($2) + 2);
	    sprintf (id, ".%s", $2);
	    $$ = id;
	  }
	| anylang_id '.' opt_digits opt_id
	  {
	    char *id = def_pool_alloc (strlen ($1) + 1 + strlen ($3) + strlen ($4) + 1);
	    sprintf (id, "%s.%s%s", $1, $3, $4);
	    $$ = id;
	  }
	;

symbol_list:
	anylang_id { def_exclude_symbols ($1); }
	|	symbol_list anylang_id { def_exclude_symbols ($2); }
	|	symbol_list ',' anylang_id { def_exclude_symbols ($3); }
	;

opt_digits: DIGITS	{ $$ = $1; }
	|		{ $$ = ""; }
	;

opt_id: ID		{ $$ = $1; }
	|		{ $$ = ""; }
	;

NUMBER: DIGITS		{ $$ = strtoul ($1, 0, 0); }
	;
VMA: DIGITS		{ $$ = (bfd_vma) strtoull ($1, 0, 0); }

%%

/*****************************************************************************
 API
 *****************************************************************************/

static FILE *the_file;
static const char *def_filename;
static int linenumber;
static def_file *def;
static int saw_newline;

struct directive
  {
    struct directive *next;
    char *name;
    int len;
  };

static struct directive *directives = 0;

def_file *
def_file_empty (void)
{
  def_file *rv = xmalloc (sizeof (def_file));
  memset (rv, 0, sizeof (def_file));
  rv->is_dll = -1;
  rv->base_address = (bfd_vma) -1;
  rv->stack_reserve = rv->stack_commit = -1;
  rv->heap_reserve = rv->heap_commit = -1;
  rv->version_major = rv->version_minor = -1;
  return rv;
}

def_file *
def_file_parse (const char *filename, def_file *add_to)
{
  struct directive *d;

  the_file = fopen (filename, "r");
  def_filename = filename;
  linenumber = 1;
  if (!the_file)
    {
      perror (filename);
      return 0;
    }
  if (add_to)
    {
      def = add_to;
    }
  else
    {
      def = def_file_empty ();
    }

  saw_newline = 1;
  if (def_parse ())
    {
      def_file_free (def);
      fclose (the_file);
      def_pool_free ();
      return 0;
    }

  fclose (the_file);

  while ((d = directives) != NULL)
    {
#if TRACE
      printf ("Adding directive %08x `%s'\n", d->name, d->name);
#endif
      def_file_add_directive (def, d->name, d->len);
      directives = d->next;
      free (d->name);
      free (d);
    }
  def_pool_free ();

  return def;
}

void
def_file_free (def_file *fdef)
{
  int i;
  unsigned int ui;

  if (!fdef)
    return;
  free (fdef->name);
  free (fdef->description);

  if (fdef->section_defs)
    {
      for (i = 0; i < fdef->num_section_defs; i++)
	{
	  free (fdef->section_defs[i].name);
	  free (fdef->section_defs[i].class);
	}
      free (fdef->section_defs);
    }

  for (i = 0; i < fdef->num_exports; i++)
    {
      if (fdef->exports[i].internal_name != fdef->exports[i].name)
        free (fdef->exports[i].internal_name);
      free (fdef->exports[i].name);
      free (fdef->exports[i].its_name);
    }
  free (fdef->exports);

  for (i = 0; i < fdef->num_imports; i++)
    {
      if (fdef->imports[i].internal_name != fdef->imports[i].name)
        free (fdef->imports[i].internal_name);
      free (fdef->imports[i].name);
      free (fdef->imports[i].its_name);
    }
  free (fdef->imports);

  while (fdef->modules)
    {
      def_file_module *m = fdef->modules;

      fdef->modules = fdef->modules->next;
      free (m);
    }

  while (fdef->aligncomms)
    {
      def_file_aligncomm *c = fdef->aligncomms;

      fdef->aligncomms = fdef->aligncomms->next;
      free (c->symbol_name);
      free (c);
    }

  for (ui = 0; ui < fdef->num_exclude_symbols; ui++)
    {
      free (fdef->exclude_symbols[ui].symbol_name);
    }
  free (fdef->exclude_symbols);

  free (fdef);
}

#ifdef DEF_FILE_PRINT
void
def_file_print (FILE *file, def_file *fdef)
{
  int i;

  fprintf (file, ">>>> def_file at 0x%08x\n", fdef);
  if (fdef->name)
    fprintf (file, "  name: %s\n", fdef->name ? fdef->name : "(unspecified)");
  if (fdef->is_dll != -1)
    fprintf (file, "  is dll: %s\n", fdef->is_dll ? "yes" : "no");
  if (fdef->base_address != (bfd_vma) -1)
    fprintf (file, "  base address: 0x%" PRIx64 "\n",
	     (uint64_t) fdef->base_address);
  if (fdef->description)
    fprintf (file, "  description: `%s'\n", fdef->description);
  if (fdef->stack_reserve != -1)
    fprintf (file, "  stack reserve: 0x%08x\n", fdef->stack_reserve);
  if (fdef->stack_commit != -1)
    fprintf (file, "  stack commit: 0x%08x\n", fdef->stack_commit);
  if (fdef->heap_reserve != -1)
    fprintf (file, "  heap reserve: 0x%08x\n", fdef->heap_reserve);
  if (fdef->heap_commit != -1)
    fprintf (file, "  heap commit: 0x%08x\n", fdef->heap_commit);

  if (fdef->num_section_defs > 0)
    {
      fprintf (file, "  section defs:\n");

      for (i = 0; i < fdef->num_section_defs; i++)
	{
	  fprintf (file, "    name: `%s', class: `%s', flags:",
		   fdef->section_defs[i].name, fdef->section_defs[i].class);
	  if (fdef->section_defs[i].flag_read)
	    fprintf (file, " R");
	  if (fdef->section_defs[i].flag_write)
	    fprintf (file, " W");
	  if (fdef->section_defs[i].flag_execute)
	    fprintf (file, " X");
	  if (fdef->section_defs[i].flag_shared)
	    fprintf (file, " S");
	  fprintf (file, "\n");
	}
    }

  if (fdef->num_exports > 0)
    {
      fprintf (file, "  exports:\n");

      for (i = 0; i < fdef->num_exports; i++)
	{
	  fprintf (file, "    name: `%s', int: `%s', ordinal: %d, flags:",
		   fdef->exports[i].name, fdef->exports[i].internal_name,
		   fdef->exports[i].ordinal);
	  if (fdef->exports[i].flag_private)
	    fprintf (file, " P");
	  if (fdef->exports[i].flag_constant)
	    fprintf (file, " C");
	  if (fdef->exports[i].flag_noname)
	    fprintf (file, " N");
	  if (fdef->exports[i].flag_data)
	    fprintf (file, " D");
	  fprintf (file, "\n");
	}
    }

  if (fdef->num_imports > 0)
    {
      fprintf (file, "  imports:\n");

      for (i = 0; i < fdef->num_imports; i++)
	{
	  fprintf (file, "    int: %s, from: `%s', name: `%s', ordinal: %d\n",
		   fdef->imports[i].internal_name,
		   fdef->imports[i].module,
		   fdef->imports[i].name,
		   fdef->imports[i].ordinal);
	}
    }

  if (fdef->version_major != -1)
    fprintf (file, "  version: %d.%d\n", fdef->version_major, fdef->version_minor);

  fprintf (file, "<<<< def_file at 0x%08x\n", fdef);
}
#endif

/* Helper routine to check for identity of string pointers,
   which might be NULL.  */

static int
are_names_equal (const char *s1, const char *s2)
{
  if (!s1 && !s2)
    return 0;
  if (!s1 || !s2)
    return (!s1 ? -1 : 1);
  return strcmp (s1, s2);
}

static int
cmp_export_elem (const def_file_export *e, const char *ex_name,
		 const char *in_name, const char *its_name,
		 int ord)
{
  int r;

  if ((r = are_names_equal (ex_name, e->name)) != 0)
    return r;
  if ((r = are_names_equal (in_name, e->internal_name)) != 0)
    return r;
  if ((r = are_names_equal (its_name, e->its_name)) != 0)
    return r;
  return (ord - e->ordinal);
}

/* Search the position of the identical element, or returns the position
   of the next higher element. If last valid element is smaller, then MAX
   is returned. The max parameter indicates the number of elements in the
   array. On return, *is_ident indicates whether the returned array index
   points at an element which is identical to the one searched for.  */

static unsigned int
find_export_in_list (def_file_export *b, unsigned int max,
		     const char *ex_name, const char *in_name,
		     const char *its_name, int ord, bool *is_ident)
{
  int e;
  unsigned int l, r, p;

  *is_ident = false;
  if (!max)
    return 0;
  if ((e = cmp_export_elem (b, ex_name, in_name, its_name, ord)) <= 0)
    {
      if (!e)
	*is_ident = true;
      return 0;
    }
  if (max == 1)
    return 1;
  if ((e = cmp_export_elem (b + (max - 1), ex_name, in_name, its_name, ord)) > 0)
    return max;
  else if (!e || max == 2)
    {
      if (!e)
	*is_ident = true;
      return max - 1;
    }
  l = 0; r = max - 1;
  while (l < r)
    {
      p = (l + r) / 2;
      e = cmp_export_elem (b + p, ex_name, in_name, its_name, ord);
      if (!e)
	{
	  *is_ident = true;
	  return p;
	}
      else if (e < 0)
	r = p - 1;
      else if (e > 0)
	l = p + 1;
    }
  if ((e = cmp_export_elem (b + l, ex_name, in_name, its_name, ord)) > 0)
    ++l;
  else if (!e)
    *is_ident = true;
  return l;
}

def_file_export *
def_file_add_export (def_file *fdef,
		     const char *external_name,
		     const char *internal_name,
		     int ordinal,
		     const char *its_name,
		     bool *is_dup)
{
  def_file_export *e;
  unsigned int pos;

  if (internal_name && !external_name)
    external_name = internal_name;
  if (external_name && !internal_name)
    internal_name = external_name;

  /* We need to avoid duplicates.  */
  *is_dup = false;
  pos = find_export_in_list (fdef->exports, fdef->num_exports,
		     external_name, internal_name,
		     its_name, ordinal, is_dup);

  if (*is_dup)
    return (fdef->exports + pos);

  if ((unsigned)fdef->num_exports >= fdef->max_exports)
    {
      fdef->max_exports += SYMBOL_LIST_ARRAY_GROW;
      fdef->exports = xrealloc (fdef->exports,
				fdef->max_exports * sizeof (def_file_export));
    }

  e = fdef->exports + pos;
  /* If we're inserting in the middle of the array, we need to move the
     following elements forward.  */
  if (pos != (unsigned)fdef->num_exports)
    memmove (&e[1], e, (sizeof (def_file_export) * (fdef->num_exports - pos)));
  /* Wipe the element for use as a new entry.  */
  memset (e, 0, sizeof (def_file_export));
  e->name = xstrdup (external_name);
  e->internal_name = xstrdup (internal_name);
  e->its_name = (its_name ? xstrdup (its_name) : NULL);
  e->ordinal = ordinal;
  fdef->num_exports++;
  return e;
}

def_file_module *
def_get_module (def_file *fdef, const char *name)
{
  def_file_module *s;

  for (s = fdef->modules; s; s = s->next)
    if (strcmp (s->name, name) == 0)
      return s;

  return NULL;
}

static def_file_module *
def_stash_module (def_file *fdef, const char *name)
{
  def_file_module *s;

  if ((s = def_get_module (fdef, name)) != NULL)
      return s;
  s = xmalloc (sizeof (def_file_module) + strlen (name));
  s->next = fdef->modules;
  fdef->modules = s;
  s->user_data = 0;
  strcpy (s->name, name);
  return s;
}

static int
cmp_import_elem (const def_file_import *e, const char *ex_name,
		 const char *in_name, const char *module,
		 int ord)
{
  int r;

  if ((r = are_names_equal (module, (e->module ? e->module->name : NULL))))
    return r;
  if ((r = are_names_equal (ex_name, e->name)) != 0)
    return r;
  if ((r = are_names_equal (in_name, e->internal_name)) != 0)
    return r;
  if (ord != e->ordinal)
    return (ord < e->ordinal ? -1 : 1);
  return 0;
}

/* Search the position of the identical element, or returns the position
   of the next higher element. If last valid element is smaller, then MAX
   is returned. The max parameter indicates the number of elements in the
   array. On return, *is_ident indicates whether the returned array index
   points at an element which is identical to the one searched for.  */

static unsigned int
find_import_in_list (def_file_import *b, unsigned int max,
		     const char *ex_name, const char *in_name,
		     const char *module, int ord, bool *is_ident)
{
  int e;
  unsigned int l, r, p;

  *is_ident = false;
  if (!max)
    return 0;
  if ((e = cmp_import_elem (b, ex_name, in_name, module, ord)) <= 0)
    {
      if (!e)
	*is_ident = true;
      return 0;
    }
  if (max == 1)
    return 1;
  if ((e = cmp_import_elem (b + (max - 1), ex_name, in_name, module, ord)) > 0)
    return max;
  else if (!e || max == 2)
    {
      if (!e)
	*is_ident = true;
      return max - 1;
    }
  l = 0; r = max - 1;
  while (l < r)
    {
      p = (l + r) / 2;
      e = cmp_import_elem (b + p, ex_name, in_name, module, ord);
      if (!e)
	{
	  *is_ident = true;
	  return p;
	}
      else if (e < 0)
	r = p - 1;
      else if (e > 0)
	l = p + 1;
    }
  if ((e = cmp_import_elem (b + l, ex_name, in_name, module, ord)) > 0)
    ++l;
  else if (!e)
    *is_ident = true;
  return l;
}

static void
fill_in_import (def_file_import *i,
		const char *name,
		def_file_module *module,
		int ordinal,
		const char *internal_name,
		const char *its_name)
{
  memset (i, 0, sizeof (def_file_import));
  if (name)
    i->name = xstrdup (name);
  i->module = module;
  i->ordinal = ordinal;
  if (internal_name)
    i->internal_name = xstrdup (internal_name);
  else
    i->internal_name = i->name;
  i->its_name = (its_name ? xstrdup (its_name) : NULL);
}

def_file_import *
def_file_add_import (def_file *fdef,
		     const char *name,
		     const char *module,
		     int ordinal,
		     const char *internal_name,
		     const char *its_name,
		     bool *is_dup)
{
  def_file_import *i;
  unsigned int pos;

  /* We need to avoid here duplicates.  */
  *is_dup = false;
  pos = find_import_in_list (fdef->imports, fdef->num_imports,
			     name,
			     (!internal_name ? name : internal_name),
			     module, ordinal, is_dup);
  if (*is_dup)
    return fdef->imports + pos;

  if ((unsigned)fdef->num_imports >= fdef->max_imports)
    {
      fdef->max_imports += SYMBOL_LIST_ARRAY_GROW;
      fdef->imports = xrealloc (fdef->imports,
				fdef->max_imports * sizeof (def_file_import));
    }
  i = fdef->imports + pos;
  /* If we're inserting in the middle of the array, we need to move the
     following elements forward.  */
  if (pos != (unsigned)fdef->num_imports)
    memmove (i + 1, i, sizeof (def_file_import) * (fdef->num_imports - pos));

  fill_in_import (i, name, def_stash_module (fdef, module), ordinal,
		  internal_name, its_name);
  fdef->num_imports++;

  return i;
}

int
def_file_add_import_from (def_file *fdef,
			  int num_imports,
			  const char *name,
			  const char *module,
			  int ordinal,
			  const char *internal_name,
			  const char *its_name ATTRIBUTE_UNUSED)
{
  def_file_import *i;
  bool is_dup;
  unsigned int pos;

  /* We need to avoid here duplicates.  */
  is_dup = false;
  pos = find_import_in_list (fdef->imports, fdef->num_imports,
			     name, internal_name ? internal_name : name,
			     module, ordinal, &is_dup);
  if (is_dup)
    return -1;
  if (fdef->imports && pos != (unsigned)fdef->num_imports)
    {
      i = fdef->imports + pos;
      if (i->module && strcmp (i->module->name, module) == 0)
	return -1;
    }

  if ((unsigned)fdef->num_imports + num_imports - 1 >= fdef->max_imports)
    {
      fdef->max_imports = fdef->num_imports + num_imports +
			  SYMBOL_LIST_ARRAY_GROW;

      fdef->imports = xrealloc (fdef->imports,
				fdef->max_imports * sizeof (def_file_import));
    }
  i = fdef->imports + pos;
  /* If we're inserting in the middle of the array, we need to move the
     following elements forward.  */
  if (pos != (unsigned)fdef->num_imports)
    memmove (i + num_imports, i,
	     sizeof (def_file_import) * (fdef->num_imports - pos));

  return pos;
}

def_file_import *
def_file_add_import_at (def_file *fdef,
			int pos,
			const char *name,
			const char *module,
			int ordinal,
			const char *internal_name,
			const char *its_name)
{
  def_file_import *i = fdef->imports + pos;

  fill_in_import (i, name, def_stash_module (fdef, module), ordinal,
		  internal_name, its_name);
  fdef->num_imports++;

  return i;
}

/* Search the position of the identical element, or returns the position
   of the next higher element. If last valid element is smaller, then MAX
   is returned. The max parameter indicates the number of elements in the
   array. On return, *is_ident indicates whether the returned array index
   points at an element which is identical to the one searched for.  */

static unsigned int
find_exclude_in_list (def_file_exclude_symbol *b, unsigned int max,
		      const char *name, bool *is_ident)
{
  int e;
  unsigned int l, r, p;

  *is_ident = false;
  if (!max)
    return 0;
  if ((e = strcmp (b[0].symbol_name, name)) <= 0)
    {
      if (!e)
	*is_ident = true;
      return 0;
    }
  if (max == 1)
    return 1;
  if ((e = strcmp (b[max - 1].symbol_name, name)) > 0)
    return max;
  else if (!e || max == 2)
    {
      if (!e)
	*is_ident = true;
      return max - 1;
    }
  l = 0; r = max - 1;
  while (l < r)
    {
      p = (l + r) / 2;
      e = strcmp (b[p].symbol_name, name);
      if (!e)
	{
	  *is_ident = true;
	  return p;
	}
      else if (e < 0)
	r = p - 1;
      else if (e > 0)
	l = p + 1;
    }
  if ((e = strcmp (b[l].symbol_name, name)) > 0)
    ++l;
  else if (!e)
    *is_ident = true;
  return l;
}

static def_file_exclude_symbol *
def_file_add_exclude_symbol (def_file *fdef, const char *name)
{
  def_file_exclude_symbol *e;
  unsigned pos;
  bool is_dup = false;

  pos = find_exclude_in_list (fdef->exclude_symbols, fdef->num_exclude_symbols,
			      name, &is_dup);

  /* We need to avoid duplicates.  */
  if (is_dup)
    return (fdef->exclude_symbols + pos);

  if (fdef->num_exclude_symbols >= fdef->max_exclude_symbols)
    {
      fdef->max_exclude_symbols += SYMBOL_LIST_ARRAY_GROW;
      fdef->exclude_symbols = xrealloc (fdef->exclude_symbols,
					fdef->max_exclude_symbols * sizeof (def_file_exclude_symbol));
    }

  e = fdef->exclude_symbols + pos;
  /* If we're inserting in the middle of the array, we need to move the
     following elements forward.  */
  if (pos != fdef->num_exclude_symbols)
    memmove (&e[1], e, (sizeof (def_file_exclude_symbol) * (fdef->num_exclude_symbols - pos)));
  /* Wipe the element for use as a new entry.  */
  memset (e, 0, sizeof (def_file_exclude_symbol));
  e->symbol_name = xstrdup (name);
  fdef->num_exclude_symbols++;
  return e;
}

struct
{
  char *param;
  int token;
}
diropts[] =
{
  { "-heap", HEAPSIZE },
  { "-stack", STACKSIZE_K },
  { "-attr", SECTIONS },
  { "-export", EXPORTS },
  { "-aligncomm", ALIGNCOMM },
  { "-exclude-symbols", EXCLUDE_SYMBOLS },
  { 0, 0 }
};

void
def_file_add_directive (def_file *my_def, const char *param, int len)
{
  def_file *save_def = def;
  const char *pend = param + len;
  char * tend = (char *) param;
  int i;

  def = my_def;

  while (param < pend)
    {
      while (param < pend
	     && (ISSPACE (*param) || *param == '\n' || *param == 0))
	param++;

      if (param == pend)
	break;

      /* Scan forward until we encounter any of:
	  - the end of the buffer
	  - the start of a new option
	  - a newline separating options
	  - a NUL separating options.  */
      for (tend = (char *) (param + 1);
	   (tend < pend
	    && !(ISSPACE (tend[-1]) && *tend == '-')
	    && *tend != '\n' && *tend != 0);
	   tend++)
	;

      for (i = 0; diropts[i].param; i++)
	{
	  len = strlen (diropts[i].param);

	  if (tend - param >= len
	      && strncmp (param, diropts[i].param, len) == 0
	      && (param[len] == ':' || param[len] == ' '))
	    {
	      lex_parse_string_end = tend;
	      lex_parse_string = param + len + 1;
	      lex_forced_token = diropts[i].token;
	      saw_newline = 0;
	      if (def_parse ())
		continue;
	      break;
	    }
	}

      if (!diropts[i].param)
	{
	  if (tend < pend)
	    {
	      char saved;

	      saved = * tend;
	      * tend = 0;
	      /* xgettext:c-format */
	      einfo (_("Warning: .drectve `%s' unrecognized\n"), param);
	      * tend = saved;
	    }
	  else
	    {
	      einfo (_("Warning: corrupt .drectve at end of def file\n"));
	    }
	}

      lex_parse_string = 0;
      param = tend;
    }

  def = save_def;
  def_pool_free ();
}

/* Parser Callbacks.  */

static void
def_image_name (const char *name, bfd_vma base, int is_dll)
{
  /* If a LIBRARY or NAME statement is specified without a name, there is nothing
     to do here.  We retain the output filename specified on command line.  */
  if (*name)
    {
      const char* image_name = lbasename (name);

      if (image_name != name)
	einfo (_("%s:%d: Warning: path components stripped from %s, '%s'\n"),
	       def_filename, linenumber, is_dll ? "LIBRARY" : "NAME",
	       name);
      free (def->name);
      /* Append the default suffix, if none specified.  */
      if (strchr (image_name, '.') == 0)
	{
	  const char * suffix = is_dll ? ".dll" : ".exe";

	  def->name = xmalloc (strlen (image_name) + strlen (suffix) + 1);
	  sprintf (def->name, "%s%s", image_name, suffix);
	}
      else
	def->name = xstrdup (image_name);
    }

  /* Honor a BASE address statement, even if LIBRARY string is empty.  */
  def->base_address = base;
  def->is_dll = is_dll;
}

static void
def_description (const char *text)
{
  int len = def->description ? strlen (def->description) : 0;

  len += strlen (text) + 1;
  if (def->description)
    {
      def->description = xrealloc (def->description, len);
      strcat (def->description, text);
    }
  else
    {
      def->description = xmalloc (len);
      strcpy (def->description, text);
    }
}

static void
def_stacksize (int reserve, int commit)
{
  def->stack_reserve = reserve;
  def->stack_commit = commit;
}

static void
def_heapsize (int reserve, int commit)
{
  def->heap_reserve = reserve;
  def->heap_commit = commit;
}

static void
def_section (const char *name, int attr)
{
  def_file_section *s;
  int max_sections = ROUND_UP (def->num_section_defs, 4);

  if (def->num_section_defs >= max_sections)
    {
      max_sections = ROUND_UP (def->num_section_defs+1, 4);

      if (def->section_defs)
	def->section_defs = xrealloc (def->section_defs,
				      max_sections * sizeof (def_file_import));
      else
	def->section_defs = xmalloc (max_sections * sizeof (def_file_import));
    }
  s = def->section_defs + def->num_section_defs;
  memset (s, 0, sizeof (def_file_section));
  s->name = xstrdup (name);
  if (attr & 1)
    s->flag_read = 1;
  if (attr & 2)
    s->flag_write = 1;
  if (attr & 4)
    s->flag_execute = 1;
  if (attr & 8)
    s->flag_shared = 1;

  def->num_section_defs++;
}

static void
def_section_alt (const char *name, const char *attr)
{
  int aval = 0;

  for (; *attr; attr++)
    {
      switch (*attr)
	{
	case 'R':
	case 'r':
	  aval |= 1;
	  break;
	case 'W':
	case 'w':
	  aval |= 2;
	  break;
	case 'X':
	case 'x':
	  aval |= 4;
	  break;
	case 'S':
	case 's':
	  aval |= 8;
	  break;
	}
    }
  def_section (name, aval);
}

static void
def_exports (const char *external_name,
	     const char *internal_name,
	     int ordinal,
	     int flags,
	     const char *its_name)
{
  def_file_export *dfe;
  bool is_dup = false;

  if (!internal_name && external_name)
    internal_name = external_name;
#if TRACE
  printf ("def_exports, ext=%s int=%s\n", external_name, internal_name);
#endif

  dfe = def_file_add_export (def, external_name, internal_name, ordinal,
			     its_name, &is_dup);

  /* We might check here for flag redefinition and warn.  For now we
     ignore duplicates silently.  */
  if (is_dup)
    return;

  if (flags & 1)
    dfe->flag_noname = 1;
  if (flags & 2)
    dfe->flag_constant = 1;
  if (flags & 4)
    dfe->flag_data = 1;
  if (flags & 8)
    dfe->flag_private = 1;
}

static void
def_import (const char *internal_name,
	    const char *module,
	    const char *dllext,
	    const char *name,
	    int ordinal,
	    const char *its_name)
{
  char *buf = 0;
  const char *ext = dllext ? dllext : "dll";
  bool is_dup = false;

  buf = xmalloc (strlen (module) + strlen (ext) + 2);
  sprintf (buf, "%s.%s", module, ext);
  module = buf;

  def_file_add_import (def, name, module, ordinal, internal_name, its_name,
		       &is_dup);
  free (buf);
}

static void
def_version (int major, int minor)
{
  def->version_major = major;
  def->version_minor = minor;
}

static void
def_directive (char *str)
{
  struct directive *d = xmalloc (sizeof (struct directive));

  d->next = directives;
  directives = d;
  d->name = xstrdup (str);
  d->len = strlen (str);
}

static void
def_aligncomm (char *str, int align)
{
  def_file_aligncomm *c, *p;

  p = NULL;
  c = def->aligncomms;
  while (c != NULL)
    {
      int e = strcmp (c->symbol_name, str);
      if (!e)
	{
	  /* Not sure if we want to allow here duplicates with
	     different alignments, but for now we keep them.  */
	  e = (int) c->alignment - align;
	  if (!e)
	    return;
	}
      if (e > 0)
	break;
      c = (p = c)->next;
    }

  c = xmalloc (sizeof (def_file_aligncomm));
  c->symbol_name = xstrdup (str);
  c->alignment = (unsigned int) align;
  if (!p)
    {
      c->next = def->aligncomms;
      def->aligncomms = c;
    }
  else
    {
      c->next = p->next;
      p->next = c;
    }
}

static void
def_exclude_symbols (char *str)
{
  def_file_add_exclude_symbol (def, str);
}

static void
def_error (const char *err)
{
  einfo ("%P: %s:%d: %s\n",
	 def_filename ? def_filename : "<unknown-file>", linenumber, err);
}


/* Lexical Scanner.  */

#undef TRACE
#define TRACE 0

/* Never freed, but always reused as needed, so no real leak.  */
static char *buffer = 0;
static int buflen = 0;
static int bufptr = 0;

static void
put_buf (char c)
{
  if (bufptr == buflen)
    {
      buflen += 50;		/* overly reasonable, eh?  */
      if (buffer)
	buffer = xrealloc (buffer, buflen + 1);
      else
	buffer = xmalloc (buflen + 1);
    }
  buffer[bufptr++] = c;
  buffer[bufptr] = 0;		/* not optimal, but very convenient.  */
}

static struct
{
  char *name;
  int token;
}
tokens[] =
{
  { "BASE", BASE },
  { "CODE", CODE },
  { "CONSTANT", CONSTANTU },
  { "constant", CONSTANTL },
  { "DATA", DATAU },
  { "data", DATAL },
  { "DESCRIPTION", DESCRIPTION },
  { "DIRECTIVE", DIRECTIVE },
  { "EXCLUDE_SYMBOLS", EXCLUDE_SYMBOLS },
  { "EXECUTE", EXECUTE },
  { "EXPORTS", EXPORTS },
  { "HEAPSIZE", HEAPSIZE },
  { "IMPORTS", IMPORTS },
  { "LIBRARY", LIBRARY },
  { "NAME", NAME },
  { "NONAME", NONAMEU },
  { "noname", NONAMEL },
  { "PRIVATE", PRIVATEU },
  { "private", PRIVATEL },
  { "READ", READ },
  { "SECTIONS", SECTIONS },
  { "SEGMENTS", SECTIONS },
  { "SHARED", SHARED_K },
  { "STACKSIZE", STACKSIZE_K },
  { "VERSION", VERSIONK },
  { "WRITE", WRITE },
  { 0, 0 }
};

static int
def_getc (void)
{
  int rv;

  if (lex_parse_string)
    {
      if (lex_parse_string >= lex_parse_string_end)
	rv = EOF;
      else
	rv = *lex_parse_string++;
    }
  else
    {
      rv = fgetc (the_file);
    }
  if (rv == '\n')
    saw_newline = 1;
  return rv;
}

static int
def_ungetc (int c)
{
  if (lex_parse_string)
    {
      lex_parse_string--;
      return c;
    }
  else
    return ungetc (c, the_file);
}

static int
def_lex (void)
{
  int c, i, q;

  if (lex_forced_token)
    {
      i = lex_forced_token;
      lex_forced_token = 0;
#if TRACE
      printf ("lex: forcing token %d\n", i);
#endif
      return i;
    }

  c = def_getc ();

  /* Trim leading whitespace.  */
  while (c != EOF && (c == ' ' || c == '\t') && saw_newline)
    c = def_getc ();

  if (c == EOF)
    {
#if TRACE
      printf ("lex: EOF\n");
#endif
      return 0;
    }

  if (saw_newline && c == ';')
    {
      do
	{
	  c = def_getc ();
	}
      while (c != EOF && c != '\n');
      if (c == '\n')
	return def_lex ();
      return 0;
    }

  /* Must be something else.  */
  saw_newline = 0;

  if (ISDIGIT (c))
    {
      bufptr = 0;
      while (c != EOF && (ISXDIGIT (c) || (c == 'x')))
	{
	  put_buf (c);
	  c = def_getc ();
	}
      if (c != EOF)
	def_ungetc (c);
      yylval.digits = def_pool_strdup (buffer);
#if TRACE
      printf ("lex: `%s' returns DIGITS\n", buffer);
#endif
      return DIGITS;
    }

  if (ISALPHA (c) || strchr ("$:-_?@", c))
    {
      bufptr = 0;
      q = c;
      put_buf (c);
      c = def_getc ();

      if (q == '@')
	{
	  if (ISBLANK (c) ) /* '@' followed by whitespace.  */
	    return (q);
	  else if (ISDIGIT (c)) /* '@' followed by digit.  */
	    {
	      def_ungetc (c);
	      return (q);
	    }
#if TRACE
	  printf ("lex: @ returns itself\n");
#endif
	}

      while (c != EOF && (ISALNUM (c) || strchr ("$:-_?/@<>", c)))
	{
	  put_buf (c);
	  c = def_getc ();
	}
      if (c != EOF)
	def_ungetc (c);
      if (ISALPHA (q)) /* Check for tokens.  */
	{
	  for (i = 0; tokens[i].name; i++)
	    if (strcmp (tokens[i].name, buffer) == 0)
	      {
#if TRACE
	        printf ("lex: `%s' is a string token\n", buffer);
#endif
	        return tokens[i].token;
	      }
	}
#if TRACE
      printf ("lex: `%s' returns ID\n", buffer);
#endif
      yylval.id = def_pool_strdup (buffer);
      return ID;
    }

  if (c == '\'' || c == '"')
    {
      q = c;
      c = def_getc ();
      bufptr = 0;

      while (c != EOF && c != q)
	{
	  put_buf (c);
	  c = def_getc ();
	}
      yylval.id = def_pool_strdup (buffer);
#if TRACE
      printf ("lex: `%s' returns ID\n", buffer);
#endif
      return ID;
    }

  if ( c == '=')
    {
      c = def_getc ();
      if (c == '=')
	{
#if TRACE
	  printf ("lex: `==' returns EQUAL\n");
#endif
	  return EQUAL;
	}
      def_ungetc (c);
#if TRACE
      printf ("lex: `=' returns itself\n");
#endif
      return '=';
    }
  if (c == '.' || c == ',')
    {
#if TRACE
      printf ("lex: `%c' returns itself\n", c);
#endif
      return c;
    }

  if (c == '\n')
    {
      linenumber++;
      saw_newline = 1;
    }

  /*printf ("lex: 0x%02x ignored\n", c); */
  return def_lex ();
}

static char *
def_pool_alloc (size_t sz)
{
  def_pool_str *e;

  e = (def_pool_str *) xmalloc (sizeof (def_pool_str) + sz);
  e->next = pool_strs;
  pool_strs = e;
  return e->data;
}

static char *
def_pool_strdup (const char *str)
{
  char *s;
  size_t len;
  if (!str)
    return NULL;
  len = strlen (str) + 1;
  s = def_pool_alloc (len);
  memcpy (s, str, len);
  return s;
}

static void
def_pool_free (void)
{
  def_pool_str *p;
  while ((p = pool_strs) != NULL)
    {
      pool_strs = p->next;
      free (p);
    }
}
