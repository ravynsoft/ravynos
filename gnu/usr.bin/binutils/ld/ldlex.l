%option nounput noyywrap

%{

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support.

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

#include "bfd.h"
#include "safe-ctype.h"
#include "bfdlink.h"
#include "ctf-api.h"
#include "ld.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include <ldgram.h>
#include "ldfile.h"
#include "ldlex.h"
#include "ldmain.h"
#include "libiberty.h"

/* The type of top-level parser input.
   yylex and yyparse (indirectly) both check this.  */
input_type parser_input;

/* Line number in the current input file.  */
unsigned int lineno;

/* The string we are currently lexing, or NULL if we are reading a
   file.  */
const char *lex_string = NULL;

/* Support for flex reading from more than one input file (stream).
   `include_stack' is flex's input state for each open file;
   `file_name_stack' is the file names.  `lineno_stack' is the current
   line numbers.

   If `include_stack_ptr' is 0, we haven't started reading anything yet.
   Otherwise, stack elements 0 through `include_stack_ptr - 1' are valid.  */

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yy_input (buf, max_size)

#ifndef YY_NO_UNPUT
#define YY_NO_UNPUT
#endif

#define MAX_INCLUDE_DEPTH 10
static YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
static const char *file_name_stack[MAX_INCLUDE_DEPTH];
static unsigned int lineno_stack[MAX_INCLUDE_DEPTH];
static unsigned int sysrooted_stack[MAX_INCLUDE_DEPTH];
static unsigned int include_stack_ptr = 0;
static int vers_node_nesting = 0;

static int yy_input (char *, int);
static void comment (void);
static void lex_warn_invalid (char *where, char *what);

/* STATES
	EXPRESSION	in an expression
	SCRIPT		in a script
	INPUTLIST	in a script, a filename-list
	MRI		in an MRI script
	WILD		inside the braces of an output section or overlay,
			for input section wildcards
	VERS_START	starting a Sun style mapfile
	VERS_SCRIPT	a Sun style mapfile
	VERS_NODE	a node within a Sun style mapfile
*/
#define RTOKEN(x)  {  yylval.token = x; return x; }

%}

%a 4000
%o 5000

WILDCHAR	[_a-zA-Z0-9\/\.\\\$\~\-\+\:\[\]\,\=\?\*\^\!]
FILENAMECHAR	[_a-zA-Z0-9\/\.\\\$\~\-\+\:\[\]\,\=]
NOCFILENAMECHAR	[_a-zA-Z0-9\/\.\\\$\~\-\+\:\[\]]
SYMBOLNAMECHAR  [_a-zA-Z0-9\/\.\\\$\~]
FILENAMECHAR1	[_a-zA-Z\/\.\\\$\~]
SYMBOLNAMECHAR1	[_a-zA-Z\.\\\$]
WHITE		[ \t\n\r]+

V_TAG [.$_a-zA-Z][._a-zA-Z0-9]*
V_IDENTIFIER [*?.$_a-zA-Z\[\]\-\!\^\\]([*?.$_a-zA-Z0-9\[\]\-\!\^\\]|::)*

%s SCRIPT
%s INPUTLIST
%s EXPRESSION
%s MRI
%s WILD
%s VERS_START
%s VERS_SCRIPT
%s VERS_NODE
%%

  if (parser_input != input_selected)
    {
      /* The first token of the input determines the initial parser state.  */
      input_type t = parser_input;
      parser_input = input_selected;
      switch (t)
	{
	case input_script: return INPUT_SCRIPT; break;
	case input_mri_script: return INPUT_MRI_SCRIPT; break;
	case input_version_script: return INPUT_VERSION_SCRIPT; break;
	case input_dynamic_list: return INPUT_DYNAMIC_LIST; break;
	case input_defsym: return INPUT_DEFSYM; break;
	default: abort ();
	}
    }

<SCRIPT,EXPRESSION,VERS_START,VERS_NODE,VERS_SCRIPT,INPUTLIST>"/*" {
				comment (); }

<MRI,EXPRESSION>"$"([0-9A-Fa-f])+ {
				yylval.integer = bfd_scan_vma (yytext + 1, 0, 16);
				yylval.bigint.str = NULL;
				return INT;
			}

<MRI,EXPRESSION>([0-9A-Fa-f])+(H|h|X|x|B|b|O|o|D|d) {
				   int ibase ;
				   switch (yytext[yyleng - 1]) {
				    case 'X':
				    case 'x':
				    case 'H':
				    case 'h':
				     ibase = 16;
				     break;
				    case 'O':
				    case 'o':
				     ibase = 8;
				     break;
				    case 'B':
				    case 'b':
				     ibase = 2;
				     break;
				    default:
				     ibase = 10;
				   }
				   yylval.integer = bfd_scan_vma (yytext, 0,
								  ibase);
				   yylval.bigint.str = NULL;
				   return INT;
				 }
<SCRIPT,MRI,EXPRESSION>((("$"|0[xX])([0-9A-Fa-f])+)|(([0-9])+))(M|K|m|k)? {
				  char *s = yytext;
				  int ibase = 0;

				  if (*s == '$')
				    {
				      ++s;
				      ibase = 16;
				    }
				  yylval.integer = bfd_scan_vma (s, 0, ibase);
				  yylval.bigint.str = NULL;
				  if (yytext[yyleng - 1] == 'M'
				      || yytext[yyleng - 1] == 'm')
				    {
				      yylval.integer *= 1024 * 1024;
				    }
				  else if (yytext[yyleng - 1] == 'K'
				      || yytext[yyleng - 1]=='k')
				    {
				      yylval.integer *= 1024;
				    }
				  else if (yytext[0] == '0'
					   && (yytext[1] == 'x'
					       || yytext[1] == 'X'))
				    {
				      yylval.bigint.str = xstrdup (yytext + 2);
				    }
				  return INT;
				}

  /* Some tokens that only appear in expressions must be enabled for
     states other than EXPRESSION, since parser lookahead means they
     must be recognised before the parser switches the lexer out of
     SCRIPT or WILD state into EXPRESSION state.

     This sort of thing happens for example with NAME in ldgram.y
     "section" rule, which is immediately followed by ldlex_expression.
     However, if you follow the grammar from "sec_or_group_p1" you see
     "assignment" appearing in "statement_anywhere".  Now,
     "assignment" also has NAME as its first token, just like
     "section".  So the parser can't know whether it is in the
     "section" or the "assignment" rule until it has scanned the next
     token to find an assignment operator.  Thus the next token after
     NAME in the "section" rule may be lexed before the lexer is
     switched to EXPRESSION state, and there are quite a number of
     optional components.  The first token in all those components
     must be able to be lexed in SCRIPT state, as well as the
     assignment operators.  In fact, due to "opt_exp_with_type",
     anything that can appear on the left hand side of "exp" might
     need to be lexed in SCRIPT state.

     MRI mode tends to cover everything in MRI scripts.
  */
<MRI,WILD>"]"				{ RTOKEN(']'); }
<MRI,WILD>"["				{ RTOKEN('['); }
<SCRIPT,EXPRESSION,MRI,WILD>"<<="	{ RTOKEN(LSHIFTEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>">>="	{ RTOKEN(RSHIFTEQ); }
<EXPRESSION,MRI>"||"			{ RTOKEN(OROR); }
<EXPRESSION,MRI>"=="			{ RTOKEN(EQ); }
<EXPRESSION,MRI>"!="			{ RTOKEN(NE); }
<EXPRESSION,MRI>">="			{ RTOKEN(GE); }
<EXPRESSION,MRI>"<="			{ RTOKEN(LE); }
<EXPRESSION,MRI>"<<"			{ RTOKEN(LSHIFT); }
<EXPRESSION,MRI>">>"			{ RTOKEN(RSHIFT); }
<SCRIPT,EXPRESSION,MRI,WILD>"+="	{ RTOKEN(PLUSEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>"-="	{ RTOKEN(MINUSEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>"*="	{ RTOKEN(MULTEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>"/="	{ RTOKEN(DIVEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>"&="	{ RTOKEN(ANDEQ); }
<SCRIPT,EXPRESSION,MRI,WILD>"|="	{ RTOKEN(OREQ); }
<EXPRESSION,MRI>"&&"			{ RTOKEN(ANDAND); }
<SCRIPT,EXPRESSION,MRI>">"		{ RTOKEN('>'); }
<SCRIPT,EXPRESSION,MRI,INPUTLIST>","	{ RTOKEN(','); }
<EXPRESSION,MRI,WILD>"&"		{ RTOKEN('&'); }
<EXPRESSION,MRI>"|"			{ RTOKEN('|'); }
<SCRIPT,EXPRESSION,MRI>"~"		{ RTOKEN('~'); }
<SCRIPT,EXPRESSION,MRI>"!"		{ RTOKEN('!'); }
<EXPRESSION,MRI>"?"			{ RTOKEN('?'); }
<EXPRESSION,MRI>"*"			{ RTOKEN('*'); }
<SCRIPT,EXPRESSION,MRI>"+"		{ RTOKEN('+'); }
<SCRIPT,EXPRESSION,MRI>"-"		{ RTOKEN('-'); }
<EXPRESSION,MRI>"/"			{ RTOKEN('/'); }
<EXPRESSION,MRI>"%"			{ RTOKEN('%'); }
<EXPRESSION,MRI>"<"			{ RTOKEN('<'); }
<SCRIPT,EXPRESSION,MRI,WILD>"="		{ RTOKEN('='); }
<SCRIPT,EXPRESSION,MRI,WILD>"}"		{ RTOKEN('}'); }
<SCRIPT,EXPRESSION,MRI,WILD>"{"		{ RTOKEN('{'); }
<SCRIPT,EXPRESSION,MRI,WILD,INPUTLIST>")" { RTOKEN(')'); }
<SCRIPT,EXPRESSION,MRI,WILD,INPUTLIST>"(" { RTOKEN('('); }
<SCRIPT,EXPRESSION,MRI>":"		{ RTOKEN(':'); }
<SCRIPT,EXPRESSION,MRI,WILD>";"		{ RTOKEN(';'); }
<SCRIPT>"MEMORY"			{ RTOKEN(MEMORY); }
<SCRIPT>"REGION_ALIAS"			{ RTOKEN(REGION_ALIAS); }
<SCRIPT>"LD_FEATURE"			{ RTOKEN(LD_FEATURE); }
<SCRIPT,EXPRESSION>"ORIGIN"		{ RTOKEN(ORIGIN); }
<SCRIPT>"VERSION"			{ RTOKEN(VERSIONK); }
<SCRIPT,EXPRESSION>"BLOCK"		{ RTOKEN(BLOCK); }
<SCRIPT,EXPRESSION>"BIND"		{ RTOKEN(BIND); }
<SCRIPT,EXPRESSION>"LENGTH"		{ RTOKEN(LENGTH); }
<SCRIPT,EXPRESSION>"ALIGN"		{ RTOKEN(ALIGN_K); }
<SCRIPT,EXPRESSION>"DATA_SEGMENT_ALIGN"	{ RTOKEN(DATA_SEGMENT_ALIGN); }
<SCRIPT,EXPRESSION>"DATA_SEGMENT_RELRO_END" { RTOKEN(DATA_SEGMENT_RELRO_END); }
<SCRIPT,EXPRESSION>"DATA_SEGMENT_END"	{ RTOKEN(DATA_SEGMENT_END); }
<SCRIPT,EXPRESSION>"ADDR"		{ RTOKEN(ADDR); }
<SCRIPT,EXPRESSION>"LOADADDR"		{ RTOKEN(LOADADDR); }
<SCRIPT,EXPRESSION>"ALIGNOF"		{ RTOKEN(ALIGNOF); }
<SCRIPT,EXPRESSION>"ABSOLUTE"		{ RTOKEN(ABSOLUTE); }
<SCRIPT,EXPRESSION>"MAX"		{ RTOKEN(MAX_K); }
<SCRIPT,EXPRESSION>"MIN"		{ RTOKEN(MIN_K); }
<SCRIPT,EXPRESSION>"LOG2CEIL"		{ RTOKEN(LOG2CEIL); }
<SCRIPT,EXPRESSION,WILD>"ASSERT"	{ RTOKEN(ASSERT_K); }
<SCRIPT>"ENTRY"				{ RTOKEN(ENTRY); }
<SCRIPT,MRI>"EXTERN"			{ RTOKEN(EXTERN); }
<SCRIPT,EXPRESSION>"NEXT"		{ RTOKEN(NEXT); }
<SCRIPT,EXPRESSION>"SIZEOF_HEADERS"	{ RTOKEN(SIZEOF_HEADERS); }
<SCRIPT,EXPRESSION>"SEGMENT_START"	{ RTOKEN(SEGMENT_START); }
<SCRIPT>"MAP"				{ RTOKEN(MAP); }
<SCRIPT,EXPRESSION>"SIZEOF"		{ RTOKEN(SIZEOF); }
<SCRIPT>"TARGET"			{ RTOKEN(TARGET_K); }
<SCRIPT>"SEARCH_DIR"			{ RTOKEN(SEARCH_DIR); }
<SCRIPT>"OUTPUT"			{ RTOKEN(OUTPUT); }
<SCRIPT>"INPUT"				{ RTOKEN(INPUT); }
<SCRIPT>"GROUP"				{ RTOKEN(GROUP); }
<INPUTLIST>"AS_NEEDED"			{ RTOKEN(AS_NEEDED); }
<SCRIPT,EXPRESSION>"DEFINED"		{ RTOKEN(DEFINED); }
<WILD>"CREATE_OBJECT_SYMBOLS"		{ RTOKEN(CREATE_OBJECT_SYMBOLS); }
<WILD>"CONSTRUCTORS"			{ RTOKEN(CONSTRUCTORS); }
<SCRIPT>"FORCE_COMMON_ALLOCATION"	{ RTOKEN(FORCE_COMMON_ALLOCATION); }
<SCRIPT>"FORCE_GROUP_ALLOCATION"	{ RTOKEN(FORCE_GROUP_ALLOCATION); }
<SCRIPT>"INHIBIT_COMMON_ALLOCATION"	{ RTOKEN(INHIBIT_COMMON_ALLOCATION); }
<SCRIPT>"SECTIONS"			{ RTOKEN(SECTIONS); }
<SCRIPT>"INSERT"			{ RTOKEN(INSERT_K); }
<SCRIPT>"AFTER"				{ RTOKEN(AFTER); }
<SCRIPT>"BEFORE"			{ RTOKEN(BEFORE); }
<WILD>"FILL"				{ RTOKEN(FILL); }
<SCRIPT>"STARTUP"			{ RTOKEN(STARTUP); }
<SCRIPT>"OUTPUT_FORMAT"			{ RTOKEN(OUTPUT_FORMAT); }
<SCRIPT>"OUTPUT_ARCH"			{ RTOKEN(OUTPUT_ARCH); }
<SCRIPT>"HLL"				{ RTOKEN(HLL); }
<SCRIPT>"SYSLIB"			{ RTOKEN(SYSLIB); }
<SCRIPT>"FLOAT"				{ RTOKEN(FLOAT); }
<WILD>"QUAD"				{ RTOKEN(QUAD); }
<WILD>"SQUAD"				{ RTOKEN(SQUAD); }
<WILD>"LONG"				{ RTOKEN(LONG); }
<WILD>"SHORT"				{ RTOKEN(SHORT); }
<WILD>"BYTE"				{ RTOKEN(BYTE); }
<WILD>"ASCIZ"				{ RTOKEN(ASCIZ); }
<WILD>"LINKER_VERSION"			{ RTOKEN(LINKER_VERSION); }
<SCRIPT>"NOFLOAT"			{ RTOKEN(NOFLOAT); }
<SCRIPT,EXPRESSION>"NOCROSSREFS"	{ RTOKEN(NOCROSSREFS); }
<SCRIPT,EXPRESSION>"NOCROSSREFS_TO"	{ RTOKEN(NOCROSSREFS_TO); }
<SCRIPT,EXPRESSION>"OVERLAY"		{ RTOKEN(OVERLAY); }
<WILD>"SORT_BY_NAME"			{ RTOKEN(SORT_BY_NAME); }
<WILD>"SORT_BY_ALIGNMENT"		{ RTOKEN(SORT_BY_ALIGNMENT); }
<WILD>"SORT"				{ RTOKEN(SORT_BY_NAME); }
<WILD>"SORT_BY_INIT_PRIORITY"		{ RTOKEN(SORT_BY_INIT_PRIORITY); }
<WILD>"SORT_NONE"			{ RTOKEN(SORT_NONE); }
<EXPRESSION>"NOLOAD"			{ RTOKEN(NOLOAD); }
<EXPRESSION>"READONLY"			{ RTOKEN(READONLY); }
<EXPRESSION>"DSECT"			{ RTOKEN(DSECT); }
<EXPRESSION>"COPY"			{ RTOKEN(COPY); }
<EXPRESSION>"INFO"			{ RTOKEN(INFO); }
<EXPRESSION>"TYPE"			{ RTOKEN(TYPE); }
<SCRIPT,EXPRESSION>"ONLY_IF_RO"		{ RTOKEN(ONLY_IF_RO); }
<SCRIPT,EXPRESSION>"ONLY_IF_RW"		{ RTOKEN(ONLY_IF_RW); }
<SCRIPT,EXPRESSION>"SPECIAL"		{ RTOKEN(SPECIAL); }
<SCRIPT>"o"				{ RTOKEN(ORIGIN); }
<SCRIPT>"org"				{ RTOKEN(ORIGIN); }
<SCRIPT>"l"				{ RTOKEN(LENGTH); }
<SCRIPT>"len"				{ RTOKEN(LENGTH); }
<WILD>"INPUT_SECTION_FLAGS"		{ RTOKEN(INPUT_SECTION_FLAGS); }
<SCRIPT,EXPRESSION,WILD,MRI>"INCLUDE"	{ RTOKEN(INCLUDE);}
<SCRIPT>"PHDRS"				{ RTOKEN(PHDRS); }
<SCRIPT,EXPRESSION,WILD>"AT"		{ RTOKEN(AT);}
<SCRIPT,EXPRESSION>"ALIGN_WITH_INPUT"	{ RTOKEN(ALIGN_WITH_INPUT);}
<SCRIPT,EXPRESSION>"SUBALIGN"		{ RTOKEN(SUBALIGN);}
<SCRIPT,EXPRESSION,WILD>"HIDDEN"	{ RTOKEN(HIDDEN); }
<SCRIPT,EXPRESSION,WILD>"PROVIDE"	{ RTOKEN(PROVIDE); }
<SCRIPT,EXPRESSION,WILD>"PROVIDE_HIDDEN" { RTOKEN(PROVIDE_HIDDEN); }
<WILD>"KEEP"				{ RTOKEN(KEEP); }
<WILD>"EXCLUDE_FILE"			{ RTOKEN(EXCLUDE_FILE); }
<SCRIPT,EXPRESSION>"CONSTANT"		{ RTOKEN(CONSTANT);}

<MRI>"#".*\n?			{ ++ lineno; }
<MRI>"\n"			{ ++ lineno;  RTOKEN(NEWLINE); }
<MRI>"*".*			{ /* Mri comment line */ }
<MRI>";".*			{ /* Mri comment line */ }
<MRI>"END"			{ RTOKEN(ENDWORD); }
<MRI>"ABSOLUTE"			{ RTOKEN(ABSOLUTE); }
<MRI>"ALIGNMOD"			{ RTOKEN(ALIGNMOD);}
<MRI>"ALIGN"			{ RTOKEN(ALIGN_K);}
<MRI>"CHIP"			{ RTOKEN(CHIP); }
<MRI>"BASE"			{ RTOKEN(BASE); }
<MRI>"ALIAS"			{ RTOKEN(ALIAS); }
<MRI>"TRUNCATE"			{ RTOKEN(TRUNCATE); }
<MRI>"LOAD"			{ RTOKEN(LOAD); }
<MRI>"PUBLIC"			{ RTOKEN(PUBLIC); }
<MRI>"ORDER"			{ RTOKEN(ORDER); }
<MRI>"NAME"			{ RTOKEN(NAMEWORD); }
<MRI>"FORMAT"			{ RTOKEN(FORMAT); }
<MRI>"CASE"			{ RTOKEN(CASE); }
<MRI>"START"			{ RTOKEN(START); }
<MRI>"LIST".*			{ RTOKEN(LIST); /* LIST and ignore to end of line */ }
<MRI>"SECT"			{ RTOKEN(SECT); }
<MRI>"end"			{ RTOKEN(ENDWORD); }
<MRI>"absolute"			{ RTOKEN(ABSOLUTE); }
<MRI>"alignmod"			{ RTOKEN(ALIGNMOD);}
<MRI>"align"			{ RTOKEN(ALIGN_K);}
<MRI>"chip"			{ RTOKEN(CHIP); }
<MRI>"base"			{ RTOKEN(BASE); }
<MRI>"alias"			{ RTOKEN(ALIAS); }
<MRI>"truncate"			{ RTOKEN(TRUNCATE); }
<MRI>"load"			{ RTOKEN(LOAD); }
<MRI>"public"			{ RTOKEN(PUBLIC); }
<MRI>"order"			{ RTOKEN(ORDER); }
<MRI>"name"			{ RTOKEN(NAMEWORD); }
<MRI>"format"			{ RTOKEN(FORMAT); }
<MRI>"case"			{ RTOKEN(CASE); }
<MRI>"extern"			{ RTOKEN(EXTERN); }
<MRI>"start"			{ RTOKEN(START); }
<MRI>"list".*			{ RTOKEN(LIST); /* LIST and ignore to end of line */ }
<MRI>"sect"			{ RTOKEN(SECT); }

<MRI>{FILENAMECHAR1}{NOCFILENAMECHAR}*	{
/* Filename without commas, needed to parse mri stuff */
				  yylval.name = xstrdup (yytext);
				  return NAME;
				}


<SCRIPT,INPUTLIST>{FILENAMECHAR1}{FILENAMECHAR}*	{
				  yylval.name = xstrdup (yytext);
				  return NAME;
				}
<INPUTLIST>"="{FILENAMECHAR1}{FILENAMECHAR}*	{
/* Filename to be prefixed by --sysroot or when non-sysrooted, nothing.  */
				  yylval.name = xstrdup (yytext);
				  return NAME;
				}
<INPUTLIST>"-l"{FILENAMECHAR}+ {
				  yylval.name = xstrdup (yytext + 2);
				  return LNAME;
				}
<EXPRESSION>{SYMBOLNAMECHAR1}{SYMBOLNAMECHAR}* {
				  yylval.name = xstrdup (yytext);
				  return NAME;
				}
  /* The following rule is to prevent a fill expression on the output
     section before /DISCARD/ interpreting the '/' as a divide.  */
<EXPRESSION>"/DISCARD/"		{
				  yylval.name = xstrdup (yytext);
				  return NAME;
				}
<WILD>{WILDCHAR}* {
		/* Annoyingly, this pattern can match comments, and we have
		   longest match issues to consider.  So if the first two
		   characters are a comment opening, put the input back and
		   try again.  */
		if (yytext[0] == '/' && yytext[1] == '*')
		  {
		    yyless (2);
		    comment ();
		  }
		else
		  {
		    yylval.name = xstrdup (yytext);
		    return NAME;
		  }
	}

<SCRIPT,EXPRESSION,WILD,VERS_NODE,INPUTLIST>"\""[^\"]*"\"" {
		/* No matter the state, quotes give what's inside.  */
		yylval.name = xmemdup (yytext + 1, yyleng - 2, yyleng - 1);
		return NAME;
	}

<SCRIPT,EXPRESSION,WILD,VERS_START,VERS_NODE,VERS_SCRIPT,INPUTLIST>"\n" {
				lineno++; }
<MRI,SCRIPT,EXPRESSION,WILD,VERS_START,VERS_NODE,VERS_SCRIPT,INPUTLIST>[ \t\r]+ {
				/* Eat up whitespace */ }
<SCRIPT,EXPRESSION,WILD,VERS_START,VERS_NODE,VERS_SCRIPT>#.* {
				/* Eat up comments */ }

<VERS_NODE,VERS_SCRIPT>[:,;]	{ return *yytext; }

<VERS_NODE>global		{ RTOKEN(GLOBAL); }

<VERS_NODE>local		{ RTOKEN(LOCAL); }

<VERS_NODE>extern		{ RTOKEN(EXTERN); }

<VERS_NODE>{V_IDENTIFIER}	{ yylval.name = xstrdup (yytext);
				  return VERS_IDENTIFIER; }

<VERS_SCRIPT>{V_TAG}		{ yylval.name = xstrdup (yytext);
				  return VERS_TAG; }

<VERS_START>"{"			{ BEGIN(VERS_SCRIPT); return *yytext; }

<VERS_SCRIPT>"{"		{ BEGIN(VERS_NODE);
				  vers_node_nesting = 0;
				  return *yytext;
				}
<VERS_SCRIPT>"}"		{ return *yytext; }
<VERS_NODE>"{"			{ vers_node_nesting++; return *yytext; }
<VERS_NODE>"}"			{ if (--vers_node_nesting < 0)
				    BEGIN(VERS_SCRIPT);
				  return *yytext;
				}

<<EOF>> {
  include_stack_ptr--;
  if (include_stack_ptr == 0)
    {
      lineno = 0;
      yyterminate ();
    }
  else
    yy_switch_to_buffer (include_stack[include_stack_ptr]);

  lineno = lineno_stack[include_stack_ptr];
  input_flags.sysrooted = sysrooted_stack[include_stack_ptr];

  return END;
}

<SCRIPT,WILD,MRI,VERS_START,VERS_SCRIPT,VERS_NODE>.	lex_warn_invalid (_(" in script"), yytext);
<EXPRESSION>.	lex_warn_invalid (_(" in expression"), yytext);

%%


/* Switch flex to reading script file NAME, open on FILE,
   saving the current input info on the include stack.  */

void
lex_push_file (FILE *file, const char *name, unsigned int sysrooted)
{
  if (include_stack_ptr >= MAX_INCLUDE_DEPTH)
    {
      einfo (_("%F:includes nested too deeply\n"));
    }
  file_name_stack[include_stack_ptr] = name;
  lineno_stack[include_stack_ptr] = lineno;
  sysrooted_stack[include_stack_ptr] = input_flags.sysrooted;
  include_stack[include_stack_ptr] = YY_CURRENT_BUFFER;

  include_stack_ptr++;
  lineno = 1;
  input_flags.sysrooted = sysrooted;
  yyin = file;
  yy_switch_to_buffer (yy_create_buffer (yyin, YY_BUF_SIZE));
}

/* Return a newly created flex input buffer containing STRING,
   which is SIZE bytes long.  */

static YY_BUFFER_STATE
yy_create_string_buffer (const char *string, size_t size)
{
  YY_BUFFER_STATE b;

  b = xmalloc (sizeof (struct yy_buffer_state));
  b->yy_input_file = 0;
  b->yy_buf_size = size;

  /* yy_ch_buf has to be 2 characters longer than the size given because
     we need to put in 2 end-of-buffer characters.  */
  b->yy_ch_buf = xmalloc ((size_t) b->yy_buf_size + 3);

  b->yy_ch_buf[0] = '\n';
  strcpy (b->yy_ch_buf+1, string);
  b->yy_ch_buf[size+1] = YY_END_OF_BUFFER_CHAR;
  b->yy_ch_buf[size+2] = YY_END_OF_BUFFER_CHAR;
  b->yy_n_chars = size+1;
  b->yy_buf_pos = &b->yy_ch_buf[1];

  b->yy_is_our_buffer = 1;
  b->yy_is_interactive = 0;
  b->yy_at_bol = 1;
  b->yy_fill_buffer = 0;

  /* flex 2.4.7 changed the interface.  FIXME: We should not be using
     a flex internal interface in the first place!  */
#ifdef YY_BUFFER_NEW
  b->yy_buffer_status = YY_BUFFER_NEW;
#else
  b->yy_eof_status = EOF_NOT_SEEN;
#endif

  return b;
}

/* Switch flex to reading from STRING, saving the current input info
   on the include stack.  */

void
lex_redirect (const char *string, const char *fake_filename, unsigned int count)
{
  YY_BUFFER_STATE tmp;

  yy_init = 0;
  if (include_stack_ptr >= MAX_INCLUDE_DEPTH)
    {
      einfo (_("%F: macros nested too deeply\n"));
    }
  file_name_stack[include_stack_ptr] = fake_filename;
  lineno_stack[include_stack_ptr] = lineno;
  include_stack[include_stack_ptr] = YY_CURRENT_BUFFER;
  include_stack_ptr++;
  lineno = count;
  tmp = yy_create_string_buffer (string, strlen (string));
  yy_switch_to_buffer (tmp);
}

/* Functions to switch to a different flex start condition,
   saving the current start condition on `state_stack'.  */

static int state_stack[MAX_INCLUDE_DEPTH * 2];
static int *state_stack_p = state_stack;

void
ldlex_script (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (SCRIPT);
}

void
ldlex_inputlist (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (INPUTLIST);
}

void
ldlex_mri_script (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (MRI);
}

void
ldlex_version_script (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (VERS_START);
}

void
ldlex_version_file (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (VERS_SCRIPT);
}

void
ldlex_expression (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (EXPRESSION);
}

void
ldlex_wild (void)
{
  *(state_stack_p)++ = yy_start;
  BEGIN (WILD);
}

void
ldlex_popstate (void)
{
  yy_start = *(--state_stack_p);
}

/* In cases where the parser needs to look ahead and the context
   changes from expression to script or vice-versa, throw away a
   NAME.  What constitutes a NAME depends on context.  */

void
ldlex_backup (void)
{
  yyless (0);
}

/* Return the current file name, or the previous file if no file is
   current.  */

const char*
ldlex_filename (void)
{
  return file_name_stack[include_stack_ptr - (include_stack_ptr != 0)];
}


/* Place up to MAX_SIZE characters in BUF and return
   either the number of characters read, or 0 to indicate EOF.  */

static int
yy_input (char *buf, int max_size)
{
  int result = 0;
  if (YY_CURRENT_BUFFER->yy_input_file)
    {
      if (yyin)
	{
	  result = fread (buf, 1, max_size, yyin);
	  if (result < max_size && ferror (yyin))
	    einfo (_("%F%P: read in flex scanner failed\n"));
	}
    }
  return result;
}

/* Eat the rest of a C-style comment.  */

static void
comment (void)
{
  int c;

  while (1)
    {
      c = input();
      while (c != '*' && c != 0)
	{
	  if (c == '\n')
	    lineno++;
	  c = input();
	}

      if (c == '*')
	{
	  c = input();
	  while (c == '*')
	    c = input();
	  if (c == '/')
	    break;			/* found the end */
	}

      if (c == '\n')
	lineno++;

      if (c == 0)
	{
	  einfo (_("%F%P: EOF in comment\n"));
	  break;
	}
    }
}

/* Warn the user about a garbage character WHAT in the input
   in context WHERE.  */

static void
lex_warn_invalid (char *where, char *what)
{
  char buf[5];

  /* If we have found an input file whose format we do not recognize,
     and we are therefore treating it as a linker script, and we find
     an invalid character, then most likely this is a real object file
     of some different format.  Treat it as such.  */
  if (ldfile_assumed_script)
    {
      bfd_set_error (bfd_error_file_not_recognized);
      einfo (_("%F%s: file not recognized: %E\n"), ldlex_filename ());
    }

  if (! ISPRINT (*what))
    {
      sprintf (buf, "\\%03o", *(unsigned char *) what);
      what = buf;
    }

  einfo (_("%P:%pS: ignoring invalid character `%s'%s\n"), NULL, what, where);
}
