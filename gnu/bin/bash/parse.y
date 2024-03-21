/* parse.y - Yacc grammar for bash. */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

%{
#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#include "filecntl.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#include <stdio.h>
#include "chartypes.h"
#include <signal.h>

#include "memalloc.h"

#include "bashintl.h"

#define NEED_STRFTIME_DECL	/* used in externs.h */

#include "shell.h"
#include "execute_cmd.h"
#include "typemax.h"		/* SIZE_MAX if needed */
#include "trap.h"
#include "flags.h"
#include "parser.h"
#include "mailcheck.h"
#include "test.h"
#include "builtins.h"
#include "builtins/common.h"
#include "builtins/builtext.h"

#include "shmbutil.h"

#if defined (READLINE)
#  include "bashline.h"
#  include <readline/readline.h>
#endif /* READLINE */

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif /* HISTORY */

#if defined (JOB_CONTROL)
#  include "jobs.h"
#else
extern int cleanup_dead_jobs PARAMS((void));
#endif /* JOB_CONTROL */

#if defined (ALIAS)
#  include "alias.h"
#else
typedef void *alias_t;
#endif /* ALIAS */

#if defined (PROMPT_STRING_DECODE)
#  ifndef _MINIX
#    include <sys/param.h>
#  endif
#  include <time.h>
#  if defined (TM_IN_SYS_TIME)
#    include <sys/types.h>
#    include <sys/time.h>
#  endif /* TM_IN_SYS_TIME */
#  include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#define END_ALIAS	-2

#ifdef DEBUG
#  define YYDEBUG 1
#else
#  define YYDEBUG 0
#endif

#if defined (HANDLE_MULTIBYTE)
#  define last_shell_getc_is_singlebyte \
	((shell_input_line_index > 1) \
		? shell_input_line_property[shell_input_line_index - 1] \
		: 1)
#  define MBTEST(x)	((x) && last_shell_getc_is_singlebyte)
#else
#  define last_shell_getc_is_singlebyte	1
#  define MBTEST(x)	((x))
#endif

#define EXTEND_SHELL_INPUT_LINE_PROPERTY() \
do { \
    if (shell_input_line_len + 2 > shell_input_line_propsize) \
      { \
	shell_input_line_propsize = shell_input_line_len + 2; \
	shell_input_line_property = (char *)xrealloc (shell_input_line_property, \
				    shell_input_line_propsize); \
      } \
} while (0)

#if defined (EXTENDED_GLOB)
extern int extended_glob, extglob_flag;
#endif

#if defined (TRANSLATABLE_STRINGS)
extern int dump_translatable_strings, dump_po_strings;
extern int singlequote_translations;
#endif /* TRANSLATABLE_STRINGS */

#if !defined (errno)
extern int errno;
#endif

/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

#ifdef DEBUG
static void debug_parser PARAMS((int));
#endif

static int yy_getc PARAMS((void));
static int yy_ungetc PARAMS((int));

#if defined (READLINE)
static int yy_readline_get PARAMS((void));
static int yy_readline_unget PARAMS((int));
#endif

static int yy_string_get PARAMS((void));
static int yy_string_unget PARAMS((int));
static int yy_stream_get PARAMS((void));
static int yy_stream_unget PARAMS((int));

static int shell_getc PARAMS((int));
static void shell_ungetc PARAMS((int));
static void discard_until PARAMS((int));

static void push_string PARAMS((char *, int, alias_t *));
static void pop_string PARAMS((void));
static void free_string_list PARAMS((void));

static char *read_a_line PARAMS((int));

static int reserved_word_acceptable PARAMS((int));
static int yylex PARAMS((void));

static void push_heredoc PARAMS((REDIRECT *));
static char *mk_alexpansion PARAMS((char *));
static int alias_expand_token PARAMS((char *));
static int time_command_acceptable PARAMS((void));
static int special_case_tokens PARAMS((char *));
static int read_token PARAMS((int));
static char *parse_matched_pair PARAMS((int, int, int, int *, int));
static char *parse_comsub PARAMS((int, int, int, int *, int));
#if defined (ARRAY_VARS)
static char *parse_compound_assignment PARAMS((int *));
#endif
#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
static int parse_dparen PARAMS((int));
static int parse_arith_cmd PARAMS((char **, int));
#endif
#if defined (COND_COMMAND)
static void cond_error PARAMS((void));
static COND_COM *cond_expr PARAMS((void));
static COND_COM *cond_or PARAMS((void));
static COND_COM *cond_and PARAMS((void));
static COND_COM *cond_term PARAMS((void));
static int cond_skip_newlines PARAMS((void));
static COMMAND *parse_cond_command PARAMS((void));
#endif
#if defined (ARRAY_VARS)
static int token_is_assignment PARAMS((char *, int));
static int token_is_ident PARAMS((char *, int));
#endif
static int read_token_word PARAMS((int));
static void discard_parser_constructs PARAMS((int));

static char *error_token_from_token PARAMS((int));
static char *error_token_from_text PARAMS((void));
static void print_offending_line PARAMS((void));
static void report_syntax_error PARAMS((char *));

static void handle_eof_input_unit PARAMS((void));
static void prompt_again PARAMS((int));
#if 0
static void reset_readline_prompt PARAMS((void));
#endif
static void print_prompt PARAMS((void));

#if defined (HANDLE_MULTIBYTE)
static void set_line_mbstate PARAMS((void));
static char *shell_input_line_property = NULL;
static size_t shell_input_line_propsize = 0;
#else
#  define set_line_mbstate()
#endif

extern int yyerror PARAMS((const char *));

#ifdef DEBUG
extern int yydebug;
#endif

/* Default prompt strings */
char *primary_prompt = PPROMPT;
char *secondary_prompt = SPROMPT;

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Displayed after reading a command but before executing it in an interactive shell */
char *ps0_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* Non-zero means we expand aliases in commands. */
int expand_aliases = 0;

/* If non-zero, the decoded prompt string undergoes parameter and
   variable substitution, command substitution, arithmetic substitution,
   string expansion, process substitution, and quote removal in
   decode_prompt_string. */
int promptvars = 1;

/* If non-zero, $'...' and $"..." are expanded when they appear within
   a ${...} expansion, even when the expansion appears within double
   quotes. */
int extended_quote = 1;

/* The number of lines read from input while creating the current command. */
int current_command_line_count;

/* The number of lines in a command saved while we run parse_and_execute */
int saved_command_line_count;

/* The token that currently denotes the end of parse. */
int shell_eof_token;

/* The token currently being read. */
int current_token;

/* The current parser state. */
int parser_state;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
static REDIRECT *redir_stack[HEREDOC_MAX];
int need_here_doc;

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
static char *shell_input_line = (char *)NULL;
static size_t shell_input_line_index;
static size_t shell_input_line_size;	/* Amount allocated for shell_input_line. */
static size_t shell_input_line_len;	/* strlen (shell_input_line) */

/* Either zero or EOF. */
static int shell_input_line_terminator;

/* The line number in a script on which a function definition starts. */
static int function_dstart;

/* The line number in a script on which a function body starts. */
static int function_bstart;

/* The line number in a script at which an arithmetic for command starts. */
static int arith_for_lineno;

/* The decoded prompt string.  Used if READLINE is not defined or if
   editing is turned off.  Analogous to current_readline_prompt. */
static char *current_decoded_prompt;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
static int last_read_token;

/* The token read prior to last_read_token. */
static int token_before_that;

/* The token read prior to token_before_that. */
static int two_tokens_ago;

static int global_extglob;

/* The line number in a script where the word in a `case WORD', `select WORD'
   or `for WORD' begins.  This is a nested command maximum, since the array
   index is decremented after a case, select, or for command is parsed. */
#define MAX_CASE_NEST	128
static int word_lineno[MAX_CASE_NEST+1];
static int word_top = -1;

/* If non-zero, it is the token that we want read_token to return
   regardless of what text is (or isn't) present to be read.  This
   is reset by read_token.  If token_to_read == WORD or
   ASSIGNMENT_WORD, yylval.word should be set to word_desc_to_read. */
static int token_to_read;
static WORD_DESC *word_desc_to_read;

static REDIRECTEE source;
static REDIRECTEE redir;

static FILE *yyoutstream;
static FILE *yyerrstream;
%}

%union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
}

/* Reserved words.  Members of the first group are only recognized
   in the case that they are preceded by a list_terminator.  Members
   of the second group are for [[...]] commands.  Members of the
   third group are recognized only under special circumstances. */
%token IF THEN ELSE ELIF FI CASE ESAC FOR SELECT WHILE UNTIL DO DONE FUNCTION COPROC
%token COND_START COND_END COND_ERROR
%token IN BANG TIME TIMEOPT TIMEIGN

/* More general tokens. yylex () knows how to make these. */
%token <word> WORD ASSIGNMENT_WORD REDIR_WORD
%token <number> NUMBER
%token <word_list> ARITH_CMD ARITH_FOR_EXPRS
%token <command> COND_CMD
%token AND_AND OR_OR GREATER_GREATER LESS_LESS LESS_AND LESS_LESS_LESS
%token GREATER_AND SEMI_SEMI SEMI_AND SEMI_SEMI_AND
%token LESS_LESS_MINUS AND_GREATER AND_GREATER_GREATER LESS_GREATER
%token GREATER_BAR BAR_AND

/* Special; never created by yylex; only set by parse_comsub and xparse_dolparen */
%token DOLPAREN

/* The types that the various syntactical units return. */

%type <command> inputunit command pipeline pipeline_command
%type <command> list0 list1 compound_list simple_list simple_list1
%type <command> simple_command shell_command
%type <command> for_command select_command case_command group_command
%type <command> arith_command
%type <command> cond_command
%type <command> arith_for_command
%type <command> coproc
%type <command> comsub
%type <command> function_def function_body if_command elif_clause subshell
%type <redirect> redirection redirection_list
%type <element> simple_command_element
%type <word_list> word_list pattern
%type <pattern> pattern_list case_clause_sequence case_clause
%type <number> timespec
%type <number> list_terminator

%start inputunit

%left '&' ';' '\n' yacc_EOF
%left AND_AND OR_OR
%right '|' BAR_AND
%%

inputunit:	simple_list simple_list_terminator
			{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = $1;
			  eof_encountered = 0;
			  /* discard_parser_constructs (0); */
			  if (parser_state & PST_CMDSUBST)
			    parser_state |= PST_EOFTOKEN;
			  YYACCEPT;
			}
	|	comsub
			{
			  /* This is special; look at the production and how
			     parse_comsub sets token_to_read */
			  global_command = $1;
			  eof_encountered = 0;
			  YYACCEPT;
			}
	|	'\n'
			{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  if (parser_state & PST_CMDSUBST)
			    parser_state |= PST_EOFTOKEN;
			  YYACCEPT;
			}
	|	error '\n'
			{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  /* discard_parser_constructs (1); */
			  if (interactive && parse_and_execute_level == 0)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
	|	error yacc_EOF
			{
			  /* EOF after an error.  Do ignoreeof or not.  Really
			     only interesting in non-interactive shells */
			  global_command = (COMMAND *)NULL;
			  if (last_command_exit_value == 0)
			    last_command_exit_value = EX_BADUSAGE;	/* force error return */
			  if (interactive && parse_and_execute_level == 0)
			    {
			      handle_eof_input_unit ();
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
	|	error YYEOF
			{
			  global_command = (COMMAND *)NULL;
			  if (last_command_exit_value == 0)
			    last_command_exit_value = EX_BADUSAGE;	/* force error return */
			  if (interactive && parse_and_execute_level == 0)
			    {
			      handle_eof_input_unit ();
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
	|	yacc_EOF
			{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
	;

word_list:	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	|	word_list WORD
			{ $$ = make_word_list ($2, $1); }
	;

redirection:	'>' WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_output_direction, redir, 0);
			}
	|	'<' WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_input_direction, redir, 0);
			}
	|	NUMBER '>' WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_output_direction, redir, 0);
			}
	|	NUMBER '<' WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_input_direction, redir, 0);
			}
	|	REDIR_WORD '>' WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_output_direction, redir, REDIR_VARASSIGN);
			}
	|	REDIR_WORD '<' WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_input_direction, redir, REDIR_VARASSIGN);
			}
	|	GREATER_GREATER WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_appending_to, redir, 0);
			}
	|	NUMBER GREATER_GREATER WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_appending_to, redir, 0);
			}
	|	REDIR_WORD GREATER_GREATER WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_appending_to, redir, REDIR_VARASSIGN);
			}
	|	GREATER_BAR WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_output_force, redir, 0);
			}
	|	NUMBER GREATER_BAR WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_output_force, redir, 0);
			}
	|	REDIR_WORD GREATER_BAR WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_output_force, redir, REDIR_VARASSIGN);
			}
	|	LESS_GREATER WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_input_output, redir, 0);
			}
	|	NUMBER LESS_GREATER WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_input_output, redir, 0);
			}
	|	REDIR_WORD LESS_GREATER WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_input_output, redir, REDIR_VARASSIGN);
			}
	|	LESS_LESS WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_reading_until, redir, 0);
			  push_heredoc ($$);
			}
	|	NUMBER LESS_LESS WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_reading_until, redir, 0);
			  push_heredoc ($$);
			}
	|	REDIR_WORD LESS_LESS WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_reading_until, redir, REDIR_VARASSIGN);
			  push_heredoc ($$);
			}
	|	LESS_LESS_MINUS WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_deblank_reading_until, redir, 0);
			  push_heredoc ($$);
			}
	|	NUMBER LESS_LESS_MINUS WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_deblank_reading_until, redir, 0);
			  push_heredoc ($$);
			}
	|	REDIR_WORD  LESS_LESS_MINUS WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_deblank_reading_until, redir, REDIR_VARASSIGN);
			  push_heredoc ($$);
			}
	|	LESS_LESS_LESS WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_reading_string, redir, 0);
			}
	|	NUMBER LESS_LESS_LESS WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_reading_string, redir, 0);
			}
	|	REDIR_WORD LESS_LESS_LESS WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_reading_string, redir, REDIR_VARASSIGN);
			}
	|	LESS_AND NUMBER
			{
			  source.dest = 0;
			  redir.dest = $2;
			  $$ = make_redirection (source, r_duplicating_input, redir, 0);
			}
	|	NUMBER LESS_AND NUMBER
			{
			  source.dest = $1;
			  redir.dest = $3;
			  $$ = make_redirection (source, r_duplicating_input, redir, 0);
			}
	|	REDIR_WORD LESS_AND NUMBER
			{
			  source.filename = $1;
			  redir.dest = $3;
			  $$ = make_redirection (source, r_duplicating_input, redir, REDIR_VARASSIGN);
			}
	|	GREATER_AND NUMBER
			{
			  source.dest = 1;
			  redir.dest = $2;
			  $$ = make_redirection (source, r_duplicating_output, redir, 0);
			}
	|	NUMBER GREATER_AND NUMBER
			{
			  source.dest = $1;
			  redir.dest = $3;
			  $$ = make_redirection (source, r_duplicating_output, redir, 0);
			}
	|	REDIR_WORD GREATER_AND NUMBER
			{
			  source.filename = $1;
			  redir.dest = $3;
			  $$ = make_redirection (source, r_duplicating_output, redir, REDIR_VARASSIGN);
			}
	|	LESS_AND WORD
			{
			  source.dest = 0;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_duplicating_input_word, redir, 0);
			}
	|	NUMBER LESS_AND WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_duplicating_input_word, redir, 0);
			}
	|	REDIR_WORD LESS_AND WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_duplicating_input_word, redir, REDIR_VARASSIGN);
			}
	|	GREATER_AND WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_duplicating_output_word, redir, 0);
			}
	|	NUMBER GREATER_AND WORD
			{
			  source.dest = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_duplicating_output_word, redir, 0);
			}
	|	REDIR_WORD GREATER_AND WORD
			{
			  source.filename = $1;
			  redir.filename = $3;
			  $$ = make_redirection (source, r_duplicating_output_word, redir, REDIR_VARASSIGN);
			}
	|	GREATER_AND '-'
			{
			  source.dest = 1;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, 0);
			}
	|	NUMBER GREATER_AND '-'
			{
			  source.dest = $1;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, 0);
			}
	|	REDIR_WORD GREATER_AND '-'
			{
			  source.filename = $1;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, REDIR_VARASSIGN);
			}
	|	LESS_AND '-'
			{
			  source.dest = 0;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, 0);
			}
	|	NUMBER LESS_AND '-'
			{
			  source.dest = $1;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, 0);
			}
	|	REDIR_WORD LESS_AND '-'
			{
			  source.filename = $1;
			  redir.dest = 0;
			  $$ = make_redirection (source, r_close_this, redir, REDIR_VARASSIGN);
			}
	|	AND_GREATER WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_err_and_out, redir, 0);
			}
	|	AND_GREATER_GREATER WORD
			{
			  source.dest = 1;
			  redir.filename = $2;
			  $$ = make_redirection (source, r_append_err_and_out, redir, 0);
			}
	;

simple_command_element: WORD
			{ $$.word = $1; $$.redirect = 0; }
	|	ASSIGNMENT_WORD
			{ $$.word = $1; $$.redirect = 0; }
	|	redirection
			{ $$.redirect = $1; $$.word = 0; }
	;

redirection_list: redirection
			{
			  $$ = $1;
			}
	|	redirection_list redirection
			{
			  register REDIRECT *t;

			  for (t = $1; t->next; t = t->next)
			    ;
			  t->next = $2;
			  $$ = $1;
			}
	;

simple_command:	simple_command_element
			{ $$ = make_simple_command ($1, (COMMAND *)NULL); }
	|	simple_command simple_command_element
			{ $$ = make_simple_command ($2, $1); }
	;

command:	simple_command
			{ $$ = clean_simple_command ($1); }
	|	shell_command
			{ $$ = $1; }
	|	shell_command redirection_list
			{
			  COMMAND *tc;

			  tc = $1;
			  if (tc && tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = $2;
			    }
			  else if (tc)
			    tc->redirects = $2;
			  $$ = $1;
			}
	|	function_def
			{ $$ = $1; }
	|	coproc
			{ $$ = $1; }
	;

shell_command:	for_command
			{ $$ = $1; }
	|	case_command
			{ $$ = $1; }
 	|	WHILE compound_list DO compound_list DONE
			{ $$ = make_while_command ($2, $4); }
	|	UNTIL compound_list DO compound_list DONE
			{ $$ = make_until_command ($2, $4); }
	|	select_command
			{ $$ = $1; }
	|	if_command
			{ $$ = $1; }
	|	subshell
			{ $$ = $1; }
	|	group_command
			{ $$ = $1; }
	|	arith_command
			{ $$ = $1; }
	|	cond_command
			{ $$ = $1; }
	|	arith_for_command
			{ $$ = $1; }
	;

for_command:	FOR WORD newline_list DO compound_list DONE
			{
			  $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD newline_list '{' compound_list '}'
			{
			  $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD ';' newline_list DO compound_list DONE
			{
			  $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD ';' newline_list '{' compound_list '}'
			{
			  $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD newline_list IN word_list list_terminator newline_list DO compound_list DONE
			{
			  $$ = make_for_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD newline_list IN word_list list_terminator newline_list '{' compound_list '}'
			{
			  $$ = make_for_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD newline_list IN list_terminator newline_list DO compound_list DONE
			{
			  $$ = make_for_command ($2, (WORD_LIST *)NULL, $8, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	FOR WORD newline_list IN list_terminator newline_list '{' compound_list '}'
			{
			  $$ = make_for_command ($2, (WORD_LIST *)NULL, $8, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	;

arith_for_command:	FOR ARITH_FOR_EXPRS list_terminator newline_list DO compound_list DONE
				{
				  $$ = make_arith_for_command ($2, $6, arith_for_lineno);
				  if ($$ == 0) YYERROR;
				  if (word_top > 0) word_top--;
				}
	|		FOR ARITH_FOR_EXPRS list_terminator newline_list '{' compound_list '}'
				{
				  $$ = make_arith_for_command ($2, $6, arith_for_lineno);
				  if ($$ == 0) YYERROR;
				  if (word_top > 0) word_top--;
				}
	|		FOR ARITH_FOR_EXPRS DO compound_list DONE
				{
				  $$ = make_arith_for_command ($2, $4, arith_for_lineno);
				  if ($$ == 0) YYERROR;
				  if (word_top > 0) word_top--;
				}
	|		FOR ARITH_FOR_EXPRS '{' compound_list '}'
				{
				  $$ = make_arith_for_command ($2, $4, arith_for_lineno);
				  if ($$ == 0) YYERROR;
				  if (word_top > 0) word_top--;
				}
	;

select_command:	SELECT WORD newline_list DO compound_list DONE
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD newline_list '{' compound_list '}'
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD ';' newline_list DO compound_list DONE
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD ';' newline_list '{' compound_list '}'
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD newline_list IN word_list list_terminator newline_list DO compound_list DONE
			{
			  $$ = make_select_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD newline_list IN word_list list_terminator newline_list '{' compound_list '}'
			{
			  $$ = make_select_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD newline_list IN list_terminator newline_list DO compound_list DONE
			{
			  $$ = make_select_command ($2, (WORD_LIST *)NULL, $8, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	SELECT WORD newline_list IN list_terminator newline_list '{' compound_list '}'
			{
			  $$ = make_select_command ($2, (WORD_LIST *)NULL, $8, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	;

case_command:	CASE WORD newline_list IN newline_list ESAC
			{
			  $$ = make_case_command ($2, (PATTERN_LIST *)NULL, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	CASE WORD newline_list IN case_clause_sequence newline_list ESAC
			{
			  $$ = make_case_command ($2, $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	|	CASE WORD newline_list IN case_clause ESAC
			{
			  $$ = make_case_command ($2, $5, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
	;

function_def:	WORD '(' ')' newline_list function_body
			{ $$ = make_function_def ($1, $5, function_dstart, function_bstart); }
	|	FUNCTION WORD '(' ')' newline_list function_body
			{ $$ = make_function_def ($2, $6, function_dstart, function_bstart); }
	|	FUNCTION WORD function_body
			{ $$ = make_function_def ($2, $3, function_dstart, function_bstart); }
	|	FUNCTION WORD '\n' newline_list function_body
			{ $$ = make_function_def ($2, $5, function_dstart, function_bstart); }
	;

function_body:	shell_command
			{ $$ = $1; }
	|	shell_command redirection_list
			{
			  COMMAND *tc;

			  tc = $1;
			  /* According to Posix.2 3.9.5, redirections
			     specified after the body of a function should
			     be attached to the function and performed when
			     the function is executed, not as part of the
			     function definition command. */
			  /* XXX - I don't think it matters, but we might
			     want to change this in the future to avoid
			     problems differentiating between a function
			     definition with a redirection and a function
			     definition containing a single command with a
			     redirection.  The two are semantically equivalent,
			     though -- the only difference is in how the
			     command printing code displays the redirections. */
			  if (tc && tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = $2;
			    }
			  else if (tc)
			    tc->redirects = $2;
			  $$ = $1;
			}
	;

subshell:	'(' compound_list ')'
			{
			  $$ = make_subshell_command ($2);
			  $$->flags |= CMD_WANT_SUBSHELL;
			}
	;

comsub:		DOLPAREN compound_list ')'
			{
			  $$ = $2;
			}
	|	DOLPAREN newline_list ')'
			{
			  $$ = (COMMAND *)NULL;
			}
	;

coproc:		COPROC shell_command
			{
			  $$ = make_coproc_command ("COPROC", $2);
			  $$->flags |= CMD_WANT_SUBSHELL|CMD_COPROC_SUBSHELL;
			}
	|	COPROC shell_command redirection_list
			{
			  COMMAND *tc;

			  tc = $2;
			  if (tc && tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = $3;
			    }
			  else if (tc)
			    tc->redirects = $3;
			  $$ = make_coproc_command ("COPROC", $2);
			  $$->flags |= CMD_WANT_SUBSHELL|CMD_COPROC_SUBSHELL;
			}
	|	COPROC WORD shell_command
			{
			  $$ = make_coproc_command ($2->word, $3);
			  $$->flags |= CMD_WANT_SUBSHELL|CMD_COPROC_SUBSHELL;
			}
	|	COPROC WORD shell_command redirection_list
			{
			  COMMAND *tc;

			  tc = $3;
			  if (tc && tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = $4;
			    }
			  else if (tc)
			    tc->redirects = $4;
			  $$ = make_coproc_command ($2->word, $3);
			  $$->flags |= CMD_WANT_SUBSHELL|CMD_COPROC_SUBSHELL;
			}
	|	COPROC simple_command
			{
			  $$ = make_coproc_command ("COPROC", clean_simple_command ($2));
			  $$->flags |= CMD_WANT_SUBSHELL|CMD_COPROC_SUBSHELL;
			}
	;

if_command:	IF compound_list THEN compound_list FI
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	|	IF compound_list THEN compound_list ELSE compound_list FI
			{ $$ = make_if_command ($2, $4, $6); }
	|	IF compound_list THEN compound_list elif_clause FI
			{ $$ = make_if_command ($2, $4, $5); }
	;


group_command:	'{' compound_list '}'
			{ $$ = make_group_command ($2); }
	;

arith_command:	ARITH_CMD
			{ $$ = make_arith_command ($1); }
	;

cond_command:	COND_START COND_CMD COND_END
			{ $$ = $2; }
	; 

elif_clause:	ELIF compound_list THEN compound_list
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	|	ELIF compound_list THEN compound_list ELSE compound_list
			{ $$ = make_if_command ($2, $4, $6); }
	|	ELIF compound_list THEN compound_list elif_clause
			{ $$ = make_if_command ($2, $4, $5); }
	;

case_clause:	pattern_list
	|	case_clause_sequence pattern_list
			{ $2->next = $1; $$ = $2; }
	;

pattern_list:	newline_list pattern ')' compound_list
			{ $$ = make_pattern_list ($2, $4); }
	|	newline_list pattern ')' newline_list
			{ $$ = make_pattern_list ($2, (COMMAND *)NULL); }
	|	newline_list '(' pattern ')' compound_list
			{ $$ = make_pattern_list ($3, $5); }
	|	newline_list '(' pattern ')' newline_list
			{ $$ = make_pattern_list ($3, (COMMAND *)NULL); }
	;

case_clause_sequence:  pattern_list SEMI_SEMI
			{ $$ = $1; }
	|	case_clause_sequence pattern_list SEMI_SEMI
			{ $2->next = $1; $$ = $2; }
	|	pattern_list SEMI_AND
			{ $1->flags |= CASEPAT_FALLTHROUGH; $$ = $1; }
	|	case_clause_sequence pattern_list SEMI_AND
			{ $2->flags |= CASEPAT_FALLTHROUGH; $2->next = $1; $$ = $2; }
	|	pattern_list SEMI_SEMI_AND
			{ $1->flags |= CASEPAT_TESTNEXT; $$ = $1; }
	|	case_clause_sequence pattern_list SEMI_SEMI_AND
			{ $2->flags |= CASEPAT_TESTNEXT; $2->next = $1; $$ = $2; }	
	;

pattern:	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	|	pattern '|' WORD
			{ $$ = make_word_list ($3, $1); }
	;

/* A list allows leading or trailing newlines and
   newlines as operators (equivalent to semicolons).
   It must end with a newline or semicolon.
   Lists are used within commands such as if, for, while.  */

compound_list:	newline_list list0
			{
			  $$ = $2;
			  if (need_here_doc && last_read_token == '\n')
			    gather_here_documents ();
			 }
	|	newline_list list1
			{
			  $$ = $2;
			}
	;

list0:  	list1 '\n' newline_list
	|	list1 '&' newline_list
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, (COMMAND *)NULL, '&');
			  else
			    $$ = command_connect ($1, (COMMAND *)NULL, '&');
			}
	|	list1 ';' newline_list

	;

list1:		list1 AND_AND newline_list list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	list1 OR_OR newline_list list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	list1 '&' newline_list list1
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, $4, '&');
			  else
			    $$ = command_connect ($1, $4, '&');
			}
	|	list1 ';' newline_list list1
			{ $$ = command_connect ($1, $4, ';'); }
	|	list1 '\n' newline_list list1
			{
			  if (parser_state & PST_CMDSUBST)
			    $$ = command_connect ($1, $4, '\n');
			  else
			    $$ = command_connect ($1, $4, ';');
			}
	|	pipeline_command
			{ $$ = $1; }
	;

simple_list_terminator:	'\n'
	|	yacc_EOF
	;

list_terminator:'\n'
		{ $$ = '\n'; }
	|	';'
		{ $$ = ';'; }
	|	yacc_EOF
		{ $$ = yacc_EOF; }
	;

newline_list:
	|	newline_list '\n'
	;

/* A simple_list is a list that contains no significant newlines
   and no leading or trailing newlines.  Newlines are allowed
   only following operators, where they are not significant.

   This is what an inputunit consists of.  */

simple_list:	simple_list1
			{
			  $$ = $1;
			  if (need_here_doc)
			    gather_here_documents ();	/* XXX */
			  if ((parser_state & PST_CMDSUBST) && current_token == shell_eof_token)
			    {
INTERNAL_DEBUG (("LEGACY: parser: command substitution simple_list1 -> simple_list"));
			      global_command = $1;
			      eof_encountered = 0;
			      if (bash_input.type == st_string)
				rewind_input_string ();
			      YYACCEPT;
			    }
			}
	|	simple_list1 '&'
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, (COMMAND *)NULL, '&');
			  else
			    $$ = command_connect ($1, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents (); /* XXX */
			  if ((parser_state & PST_CMDSUBST) && current_token == shell_eof_token)
			    {
INTERNAL_DEBUG (("LEGACY: parser: command substitution simple_list1 '&' -> simple_list"));
			      global_command = $1;
			      eof_encountered = 0;
			      if (bash_input.type == st_string)
				rewind_input_string ();
			      YYACCEPT;
			    }
			}
	|	simple_list1 ';'
			{
			  $$ = $1;
			  if (need_here_doc)
			    gather_here_documents ();	/* XXX */
			  if ((parser_state & PST_CMDSUBST) && current_token == shell_eof_token)
			    {
INTERNAL_DEBUG (("LEGACY: parser: command substitution simple_list1 ';' -> simple_list"));
			      global_command = $1;
			      eof_encountered = 0;
			      if (bash_input.type == st_string)
				rewind_input_string ();
			      YYACCEPT;
			    }
			}
	;

simple_list1:	simple_list1 AND_AND newline_list simple_list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	simple_list1 OR_OR newline_list simple_list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	simple_list1 '&' simple_list1
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, $3, '&');
			  else
			    $$ = command_connect ($1, $3, '&');
			}
	|	simple_list1 ';' simple_list1
			{ $$ = command_connect ($1, $3, ';'); }

	|	pipeline_command
			{ $$ = $1; }
	;

pipeline_command: pipeline
			{ $$ = $1; }			
	|	BANG pipeline_command
			{
			  if ($2)
			    $2->flags ^= CMD_INVERT_RETURN;	/* toggle */
			  $$ = $2;
			}
	|	timespec pipeline_command
			{
			  if ($2)
			    $2->flags |= $1;
			  $$ = $2;
			}
	|	timespec list_terminator
			{
			  ELEMENT x;

			  /* Boy, this is unclean.  `time' by itself can
			     time a null command.  We cheat and push a
			     newline back if the list_terminator was a newline
			     to avoid the double-newline problem (one to
			     terminate this, one to terminate the command) */
			  x.word = 0;
			  x.redirect = 0;
			  $$ = make_simple_command (x, (COMMAND *)NULL);
			  $$->flags |= $1;
			  /* XXX - let's cheat and push a newline back */
			  if ($2 == '\n')
			    token_to_read = '\n';
			  else if ($2 == ';')
			    token_to_read = ';';
			  parser_state &= ~PST_REDIRLIST;	/* make_simple_command sets this */
			}
	|	BANG list_terminator
			{
			  ELEMENT x;

			  /* This is just as unclean.  Posix says that `!'
			     by itself should be equivalent to `false'.
			     We cheat and push a
			     newline back if the list_terminator was a newline
			     to avoid the double-newline problem (one to
			     terminate this, one to terminate the command) */
			  x.word = 0;
			  x.redirect = 0;
			  $$ = make_simple_command (x, (COMMAND *)NULL);
			  $$->flags |= CMD_INVERT_RETURN;
			  /* XXX - let's cheat and push a newline back */
			  if ($2 == '\n')
			    token_to_read = '\n';
			  if ($2 == ';')
			    token_to_read = ';';
			  parser_state &= ~PST_REDIRLIST;	/* make_simple_command sets this */
			}
	;

pipeline:	pipeline '|' newline_list pipeline
			{ $$ = command_connect ($1, $4, '|'); }
	|	pipeline BAR_AND newline_list pipeline
			{
			  /* Make cmd1 |& cmd2 equivalent to cmd1 2>&1 | cmd2 */
			  COMMAND *tc;
			  REDIRECTEE rd, sd;
			  REDIRECT *r;

			  tc = $1->type == cm_simple ? (COMMAND *)$1->value.Simple : $1;
			  sd.dest = 2;
			  rd.dest = 1;
			  r = make_redirection (sd, r_duplicating_output, rd, 0);
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = r;
			    }
			  else
			    tc->redirects = r;

			  $$ = command_connect ($1, $4, '|');
			}
	|	command
			{ $$ = $1; }
	;

timespec:	TIME
			{ $$ = CMD_TIME_PIPELINE; }
	|	TIME TIMEOPT
			{ $$ = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
	|	TIME TIMEIGN
			{ $$ = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
	|	TIME TIMEOPT TIMEIGN
			{ $$ = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
	;
%%

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_INITIAL_SIZE 496
#define TOKEN_DEFAULT_GROW_SIZE 512

/* Should we call prompt_again? */
#define SHOULD_PROMPT() \
  (interactive && (bash_input.type == st_stdin || bash_input.type == st_stream))

#if defined (ALIAS)
#  define expanding_alias() (pushed_string_list && pushed_string_list->expander)
#else
#  define expanding_alias() 0
#endif

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

#ifdef DEBUG
static void
debug_parser (i)
     int i;
{
#if YYDEBUG != 0
  yydebug = i;
  yyoutstream = stdout;
  yyerrstream = stderr;
#endif
}
#endif

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget, type, location) makes the function GET the
   installed function for getting the next character, makes UNGET the
   installed function for un-getting a character, sets the type of stream
   (either string or file) from TYPE, and makes LOCATION point to where
   the input is coming from. */

/* Unconditionally returns end-of-file. */
int
return_EOF ()
{
  return (EOF);
}

/* Variable containing the current get and unget functions.
   See ./input.h for a clearer description. */
BASH_INPUT bash_input;

/* Set all of the fields in BASH_INPUT to NULL.  Free bash_input.name if it
   is non-null, avoiding a memory leak. */
void
initialize_bash_input ()
{
  bash_input.type = st_none;
  FREE (bash_input.name);
  bash_input.name = (char *)NULL;
  bash_input.location.file = (FILE *)NULL;
  bash_input.location.string = (char *)NULL;
  bash_input.getter = (sh_cget_func_t *)NULL;
  bash_input.ungetter = (sh_cunget_func_t *)NULL;
}

/* Set the contents of the current bash input stream from
   GET, UNGET, TYPE, NAME, and LOCATION. */
void
init_yy_io (get, unget, type, name, location)
     sh_cget_func_t *get;
     sh_cunget_func_t *unget;
     enum stream_type type;
     const char *name;
     INPUT_STREAM location;
{
  bash_input.type = type;
  FREE (bash_input.name);
  bash_input.name = name ? savestring (name) : (char *)NULL;

  /* XXX */
#if defined (CRAY)
  memcpy((char *)&bash_input.location.string, (char *)&location.string, sizeof(location));
#else
  bash_input.location = location;
#endif
  bash_input.getter = get;
  bash_input.ungetter = unget;
}

char *
yy_input_name ()
{
  return (bash_input.name ? bash_input.name : "stdin");
}

/* Call this to get the next character of input. */
static int
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
static int
yy_ungetc (c)
     int c;
{
  return (*(bash_input.ungetter)) (c);
}

#if defined (BUFFERED_INPUT)
#ifdef INCLUDE_UNUSED
int
input_file_descriptor ()
{
  switch (bash_input.type)
    {
    case st_stream:
      return (fileno (bash_input.location.file));
    case st_bstream:
      return (bash_input.location.buffered_fd);
    case st_stdin:
    default:
      return (fileno (stdin));
    }
}
#endif
#endif /* BUFFERED_INPUT */

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#if defined (READLINE)
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int
yy_readline_get ()
{
  SigHandler *old_sigint;
  int line_len;
  unsigned char c;

  if (current_readline_line == 0)
    {
      if (bash_readline_initialized == 0)
	initialize_readline ();

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp, 0);
#endif /* JOB_CONTROL */

      old_sigint = IMPOSSIBLE_TRAP_HANDLER;
      if (signal_is_ignored (SIGINT) == 0)
	{
	  old_sigint = (SigHandler *)set_signal_handler (SIGINT, sigint_sighandler);
	}

      sh_unset_nodelay_mode (fileno (rl_instream));	/* just in case */
      current_readline_line = readline (current_readline_prompt ?
      					  current_readline_prompt : "");

      CHECK_TERMSIG;
      if (signal_is_ignored (SIGINT) == 0)
	{
	  if (old_sigint != IMPOSSIBLE_TRAP_HANDLER)
	    set_signal_handler (SIGINT, old_sigint);
	}

#if 0
      /* Reset the prompt to the decoded value of prompt_string_pointer. */
      reset_readline_prompt ();
#endif

      if (current_readline_line == 0)
	return (EOF);

      current_readline_line_index = 0;
      line_len = strlen (current_readline_line);

      current_readline_line = (char *)xrealloc (current_readline_line, 2 + line_len);
      current_readline_line[line_len++] = '\n';
      current_readline_line[line_len] = '\0';
    }

  if (current_readline_line[current_readline_line_index] == 0)
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      c = current_readline_line[current_readline_line_index++];
      return (c);
    }
}

static int
yy_readline_unget (c)
     int c;
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
  return (c);
}

void
with_input_from_stdin ()
{
  INPUT_STREAM location;

  if (bash_input.type != st_stdin && stream_on_stack (st_stdin) == 0)
    {
      location.string = current_readline_line;
      init_yy_io (yy_readline_get, yy_readline_unget,
		  st_stdin, "readline stdin", location);
    }
}

/* Will we be collecting another input line and printing a prompt? This uses
   different conditions than SHOULD_PROMPT(), since readline allows a user to
   embed a newline in the middle of the line it collects, which the parser
   will interpret as a line break and command delimiter. */
int
parser_will_prompt ()
{
  return (current_readline_line == 0 || current_readline_line[current_readline_line_index] == 0);
}
  
#else  /* !READLINE */

void
with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif	/* !READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

static int
yy_string_get ()
{
  register char *string;
  register unsigned char c;

  string = bash_input.location.string;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *string++;
      bash_input.location.string = string;
      return (c);
    }
  else
    return (EOF);
}

static int
yy_string_unget (c)
     int c;
{
  *(--bash_input.location.string) = c;
  return (c);
}

void
with_input_from_string (string, name)
     char *string;
     const char *name;
{
  INPUT_STREAM location;

  location.string = string;
  init_yy_io (yy_string_get, yy_string_unget, st_string, name, location);
}

/* Count the number of characters we've consumed from bash_input.location.string
   and read into shell_input_line, but have not returned from shell_getc.
   That is the true input location.  Rewind bash_input.location.string by
   that number of characters, so it points to the last character actually
   consumed by the parser. */
void
rewind_input_string ()
{
  int xchars;

  /* number of unconsumed characters in the input -- XXX need to take newlines
     into account, e.g., $(...\n) */
  xchars = shell_input_line_len - shell_input_line_index;
  if (bash_input.location.string[-1] == '\n')
    xchars++;

  /* XXX - how to reflect bash_input.location.string back to string passed to
     parse_and_execute or xparse_dolparen? xparse_dolparen needs to know how
     far into the string we parsed.  parse_and_execute knows where bash_input.
     location.string is, and how far from orig_string that is -- that's the
     number of characters the command consumed. */

  /* bash_input.location.string - xchars should be where we parsed to */
  /* need to do more validation on xchars value for sanity -- test cases. */
  bash_input.location.string -= xchars;
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

/* These two functions used to test the value of the HAVE_RESTARTABLE_SYSCALLS
   define, and just use getc/ungetc if it was defined, but since bash
   installs most of its signal handlers without the SA_RESTART flag, some
   signals received during a read(2) will not cause the read to be restarted.
   We will need to restart it ourselves. */

static int
yy_stream_get ()
{
  int result;

  result = EOF;
  if (bash_input.location.file)
    {
      /* XXX - don't need terminate_immediately; getc_with_restart checks
	 for terminating signals itself if read returns < 0 */
      result = getc_with_restart (bash_input.location.file);
    }
  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
  return (ungetc_with_restart (c, bash_input.location.file));
}

void
with_input_from_stream (stream, name)
     FILE *stream;
     const char *name;
{
  INPUT_STREAM location;

  location.file = stream;
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, name, location);
}

typedef struct stream_saver {
  struct stream_saver *next;
  BASH_INPUT bash_input;
  int line;
#if defined (BUFFERED_INPUT)
  BUFFERED_STREAM *bstream;
#endif /* BUFFERED_INPUT */
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

/* The line number offset set by assigning to LINENO.  Not currently used. */
int line_number_base = 0;

#if defined (COND_COMMAND)
static int cond_lineno;
static int cond_token;
#endif

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

void
push_stream (reset_lineno)
     int reset_lineno;
{
  STREAM_SAVER *saver = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));

  xbcopy ((char *)&bash_input, (char *)&(saver->bash_input), sizeof (BASH_INPUT));

#if defined (BUFFERED_INPUT)
  saver->bstream = (BUFFERED_STREAM *)NULL;
  /* If we have a buffered stream, clear out buffers[fd]. */
  if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
    saver->bstream = set_buffered_stream (bash_input.location.buffered_fd,
    					  (BUFFERED_STREAM *)NULL);
#endif /* BUFFERED_INPUT */

  saver->line = line_number;
  bash_input.name = (char *)NULL;
  saver->next = stream_list;
  stream_list = saver;
  EOF_Reached = 0;
  if (reset_lineno)
    line_number = 0;
}

void
pop_stream ()
{
  if (!stream_list)
    EOF_Reached = 1;
  else
    {
      STREAM_SAVER *saver = stream_list;

      EOF_Reached = 0;
      stream_list = stream_list->next;

      init_yy_io (saver->bash_input.getter,
		  saver->bash_input.ungetter,
		  saver->bash_input.type,
		  saver->bash_input.name,
		  saver->bash_input.location);

#if defined (BUFFERED_INPUT)
      /* If we have a buffered stream, restore buffers[fd]. */
      /* If the input file descriptor was changed while this was on the
	 save stack, update the buffered fd to the new file descriptor and
	 re-establish the buffer <-> bash_input fd correspondence. */
      if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
	{
	  if (bash_input_fd_changed)
	    {
	      bash_input_fd_changed = 0;
	      if (default_buffered_input >= 0)
		{
		  bash_input.location.buffered_fd = default_buffered_input;
		  saver->bstream->b_fd = default_buffered_input;
		  SET_CLOSE_ON_EXEC (default_buffered_input);
		}
	    }
	  /* XXX could free buffered stream returned as result here. */
	  set_buffered_stream (bash_input.location.buffered_fd, saver->bstream);
	}
#endif /* BUFFERED_INPUT */

      line_number = saver->line;

      FREE (saver->bash_input.name);
      free (saver);
    }
}

/* Return 1 if a stream of type TYPE is saved on the stack. */
int
stream_on_stack (type)
     enum stream_type type;
{
  register STREAM_SAVER *s;

  for (s = stream_list; s; s = s->next)
    if (s->bash_input.type == type)
      return 1;
  return 0;
}

/* Save the current token state and return it in a malloced array. */
int *
save_token_state ()
{
  int *ret;

  ret = (int *)xmalloc (4 * sizeof (int));
  ret[0] = last_read_token;
  ret[1] = token_before_that;
  ret[2] = two_tokens_ago;
  ret[3] = current_token;
  return ret;
}

void
restore_token_state (ts)
     int *ts;
{
  if (ts == 0)
    return;
  last_read_token = ts[0];
  token_before_that = ts[1];
  two_tokens_ago = ts[2];
  current_token = ts[3];
}

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list' is:
 *
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */

#define END_OF_ALIAS 0

/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

/*
 * Pushing and popping strings.  This works together with shell_getc to
 * implement alias expansion on a per-token basis.
 */

#define PSH_ALIAS	0x01
#define PSH_DPAREN	0x02
#define PSH_SOURCE	0x04
#define PSH_ARRAY	0x08

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
#if defined (ALIAS)
  alias_t *expander;   /* alias that caused this line to be pushed. */
#endif
  size_t saved_line_size, saved_line_index, saved_line_len;
  int saved_line_terminator;
  int flags;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, ap)
     char *s;
     int expand;
     alias_t *ap;
{
  STRING_SAVER *temp = (STRING_SAVER *)xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_len = shell_input_line_len;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
  temp->flags = 0;
#if defined (ALIAS)
  temp->expander = ap;
  if (ap)
    temp->flags = PSH_ALIAS;
#endif
  temp->next = pushed_string_list;
  pushed_string_list = temp;

#if defined (ALIAS)
  if (ap)
    ap->flags |= AL_BEINGEXPANDED;
#endif

  shell_input_line = s;
  shell_input_line_size = shell_input_line_len = STRLEN (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
#if 0
  parser_state &= ~PST_ALEXPNEXT;	/* XXX */
#endif

  set_line_mbstate ();
}

/*
 * Make the top of the pushed_string stack be the current shell input.
 * Only called when there is something on the stack.  Called from shell_getc
 * when it thinks it has consumed the string generated by an alias expansion
 * and needs to return to the original input line.
 */
static void
pop_string ()
{
  STRING_SAVER *t;

  FREE (shell_input_line);
  shell_input_line = pushed_string_list->saved_line;
  shell_input_line_index = pushed_string_list->saved_line_index;
  shell_input_line_size = pushed_string_list->saved_line_size;
  shell_input_line_len = pushed_string_list->saved_line_len;
  shell_input_line_terminator = pushed_string_list->saved_line_terminator;

#if defined (ALIAS)
  if (pushed_string_list->expand_alias)
    parser_state |= PST_ALEXPNEXT;
  else
    parser_state &= ~PST_ALEXPNEXT;
#endif

  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;

#if defined (ALIAS)
  if (t->expander)
    t->expander->flags &= ~AL_BEINGEXPANDED;
#endif

  free ((char *)t);

  set_line_mbstate ();
}

static void
free_string_list ()
{
  register STRING_SAVER *t, *t1;

  for (t = pushed_string_list; t; )
    {
      t1 = t->next;
      FREE (t->saved_line);
#if defined (ALIAS)
      if (t->expander)
	t->expander->flags &= ~AL_BEINGEXPANDED;
#endif
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

void
free_pushed_string_input ()
{
#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  free_string_list ();
#endif
}

int
parser_expanding_alias ()
{
  return (expanding_alias ());
}

void
parser_save_alias ()
{
#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  push_string ((char *)NULL, 0, (alias_t *)NULL);
  pushed_string_list->flags = PSH_SOURCE;	/* XXX - for now */
#else
  ;
#endif
}

void
parser_restore_alias ()
{
#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  if (pushed_string_list)
    pop_string ();
#else
  ;
#endif
}

#if defined (ALIAS)
/* Before freeing AP, make sure that there aren't any cases of pointer
   aliasing that could cause us to reference freed memory later on. */
void
clear_string_list_expander (ap)
     alias_t *ap;
{
  register STRING_SAVER *t;

  for (t = pushed_string_list; t; t = t->next)
    {
      if (t->expander && t->expander == ap)
	t->expander = 0;
    }
}
#endif

void
clear_shell_input_line ()
{
  if (shell_input_line)
    shell_input_line[shell_input_line_index = 0] = '\0';
}

/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL.  If REMOVE_QUOTED_NEWLINE
   is non-zero, we remove unquoted \<newline> pairs.  This is used by
   read_secondary_line to read here documents. */
static char *
read_a_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  static char *line_buffer = (char *)NULL;
  static int buffer_size = 0;
  int indx, c, peekc, pass_next;

#if defined (READLINE)
  if (no_line_editing && SHOULD_PROMPT ())
#else
  if (SHOULD_PROMPT ())
#endif
    print_prompt ();

  pass_next = indx = 0;
  while (1)
    {
      /* Allow immediate exit if interrupted during input. */
      QUIT;

      c = yy_getc ();

      /* Ignore null bytes in input. */
      if (c == 0)
	continue;

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  if (interactive && bash_input.type == st_stream)
	    clearerr (stdin);
	  if (indx == 0)
	    return ((char *)NULL);
	  c = '\n';
	}

      /* `+2' in case the final character in the buffer is a newline or we
	 have to handle CTLESC or CTLNUL. */
      RESIZE_MALLOCED_BUFFER (line_buffer, indx, 2, buffer_size, 128);

      /* IF REMOVE_QUOTED_NEWLINES is non-zero, we are reading a
	 here document with an unquoted delimiter.  In this case,
	 the line will be expanded as if it were in double quotes.
	 We allow a backslash to escape the next character, but we
	 need to treat the backslash specially only if a backslash
	 quoting a backslash-newline pair appears in the line. */
      if (pass_next)
	{
	  line_buffer[indx++] = c;
	  pass_next = 0;
	}
      else if (c == '\\' && remove_quoted_newline)
	{
	  QUIT;
	  peekc = yy_getc ();
	  if (peekc == '\n')
	    {
	      line_number++;
	      continue;	/* Make the unquoted \<newline> pair disappear. */
	    }
	  else
	    {
	      yy_ungetc (peekc);
	      pass_next = 1;
	      line_buffer[indx++] = c;		/* Preserve the backslash. */
	    }
	}
      else
	{
	  /* remove_quoted_newline is non-zero if the here-document delimiter
	     is unquoted. In this case, we will be expanding the lines and
	     need to make sure CTLESC and CTLNUL in the input are quoted. */
	  if (remove_quoted_newline && (c == CTLESC || c == CTLNUL))
	    line_buffer[indx++] = CTLESC;
	  line_buffer[indx++] = c;
	}

      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt.  This is used to read the lines of a here
   document.  REMOVE_QUOTED_NEWLINE is non-zero if we should remove
   newlines quoted with backslashes while reading the line.  It is
   non-zero unless the delimiter of the here document was quoted. */
char *
read_secondary_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  char *ret;
  int n, c;

  prompt_string_pointer = &ps2_prompt;
  if (SHOULD_PROMPT ())
    prompt_again (0);
  ret = read_a_line (remove_quoted_newline);
#if defined (HISTORY)
  if (ret && remember_on_history && (parser_state & PST_HEREDOC))
    {
      /* To make adding the here-document body right, we need to rely on
	 history_delimiting_chars() returning \n for the first line of the
	 here-document body and the null string for the second and subsequent
	 lines, so we avoid double newlines.
	 current_command_line_count == 2 for the first line of the body. */

      current_command_line_count++;
      maybe_add_history (ret);
    }
#endif /* HISTORY */
  return ret;
}

/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command. */
STRING_INT_ALIST word_token_alist[] = {
  { "if", IF },
  { "then", THEN },
  { "else", ELSE },
  { "elif", ELIF },
  { "fi", FI },
  { "case", CASE },
  { "esac", ESAC },
  { "for", FOR },
#if defined (SELECT_COMMAND)
  { "select", SELECT },
#endif
  { "while", WHILE },
  { "until", UNTIL },
  { "do", DO },
  { "done", DONE },
  { "in", IN },
  { "function", FUNCTION },
#if defined (COMMAND_TIMING)
  { "time", TIME },
#endif
  { "{", '{' },
  { "}", '}' },
  { "!", BANG },
#if defined (COND_COMMAND)
  { "[[", COND_START },
  { "]]", COND_END },
#endif
#if defined (COPROCESS_SUPPORT)
  { "coproc", COPROC },
#endif
  { (char *)NULL, 0}
};

/* other tokens that can be returned by read_token() */
STRING_INT_ALIST other_token_alist[] = {
  /* Multiple-character tokens with special values */
  { "--", TIMEIGN },
  { "-p", TIMEOPT },
  { "&&", AND_AND },
  { "||", OR_OR },
  { ">>", GREATER_GREATER },
  { "<<", LESS_LESS },
  { "<&", LESS_AND },
  { ">&", GREATER_AND },
  { ";;", SEMI_SEMI },
  { ";&", SEMI_AND },
  { ";;&", SEMI_SEMI_AND },
  { "<<-", LESS_LESS_MINUS },
  { "<<<", LESS_LESS_LESS },
  { "&>", AND_GREATER },
  { "&>>", AND_GREATER_GREATER },
  { "<>", LESS_GREATER },
  { ">|", GREATER_BAR },
  { "|&", BAR_AND },
  { "EOF", yacc_EOF },
  /* Tokens whose value is the character itself */
  { ">", '>' },
  { "<", '<' },
  { "-", '-' },
  { "{", '{' },
  { "}", '}' },
  { ";", ';' },
  { "(", '(' },
  { ")", ')' },
  { "|", '|' },
  { "&", '&' },
  { "newline", '\n' },
  { (char *)NULL, 0}
};

/* others not listed here:
	WORD			look at yylval.word
	ASSIGNMENT_WORD		look at yylval.word
	NUMBER			look at yylval.number
	ARITH_CMD		look at yylval.word_list
	ARITH_FOR_EXPRS		look at yylval.word_list
	COND_CMD		look at yylval.command
*/

/* These are used by read_token_word, but appear up here so that shell_getc
   can use them to decide when to add otherwise blank lines to the history. */

/* The primary delimiter stack. */
struct dstack dstack = {  (char *)NULL, 0, 0 };

/* A temporary delimiter stack to be used when decoding prompt strings.
   This is needed because command substitutions in prompt strings (e.g., PS2)
   can screw up the parser's quoting state. */
static struct dstack temp_dstack = { (char *)NULL, 0, 0 };

/* Macro for accessing the top delimiter on the stack.  Returns the
   delimiter or zero if none. */
#define current_delimiter(ds) \
  (ds.delimiter_depth ? ds.delimiters[ds.delimiter_depth - 1] : 0)

#define push_delimiter(ds, character) \
  do \
    { \
      if (ds.delimiter_depth + 2 > ds.delimiter_space) \
	ds.delimiters = (char *)xrealloc \
	  (ds.delimiters, (ds.delimiter_space += 10) * sizeof (char)); \
      ds.delimiters[ds.delimiter_depth] = character; \
      ds.delimiter_depth++; \
    } \
  while (0)

#define pop_delimiter(ds)	ds.delimiter_depth--

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line.  This is called by read_token when the shell is
   processing normal command input. */

/* This implements one-character lookahead/lookbehind across physical input
   lines, to avoid something being lost because it's pushed back with
   shell_ungetc when we're at the start of a line. */
static int eol_ungetc_lookahead = 0;

static int unquoted_backslash = 0;

static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  register int i;
  int c, truncating, last_was_backslash;
  unsigned char uc;

  QUIT;

  last_was_backslash = 0;
  if (sigwinch_received)
    {
      sigwinch_received = 0;
      get_new_window_size (0, (int *)0, (int *)0);
    }
      
  if (eol_ungetc_lookahead)
    {
      c = eol_ungetc_lookahead;
      eol_ungetc_lookahead = 0;
      return (c);
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS && !DPAREN_ARITHMETIC */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS && !DPAREN_ARITHMETIC */
    {
      line_number++;

      /* Let's not let one really really long line blow up memory allocation */
      if (shell_input_line && shell_input_line_size >= 32768)
	{
	  free (shell_input_line);
	  shell_input_line = 0;
	  shell_input_line_size = 0;
	}

    restart_read:

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      i = truncating = 0;
      shell_input_line_terminator = 0;

      /* If the shell is interactive, but not currently printing a prompt
         (interactive_shell && interactive == 0), we don't want to print
         notifies or cleanup the jobs -- we want to defer it until we do
         print the next prompt. */
      if (interactive_shell == 0 || SHOULD_PROMPT())
	{
#if defined (JOB_CONTROL)
      /* This can cause a problem when reading a command as the result
	 of a trap, when the trap is called from flush_child.  This call
	 had better not cause jobs to disappear from the job table in
	 that case, or we will have big trouble. */
	  notify_and_cleanup ();
#else /* !JOB_CONTROL */
	  cleanup_dead_jobs ();
#endif /* !JOB_CONTROL */
	}

#if defined (READLINE)
      if (no_line_editing && SHOULD_PROMPT())
#else
      if (SHOULD_PROMPT())
#endif
	print_prompt ();

      if (bash_input.type == st_stream)
	clearerr (stdin);

      while (1)
	{
	  c = yy_getc ();

	  /* Allow immediate exit if interrupted during input. */
	  QUIT;

	  if (c == '\0')
	    {
	      /* If we get EOS while parsing a string, treat it as EOF so we
		 don't just keep looping. Happens very rarely */
	      if (bash_input.type == st_string)
		{
		  if (i == 0)
		    shell_input_line_terminator = EOF;
		  shell_input_line[i] = '\0';
		  c = EOF;
		  break;
		}
	      continue;
	    }

	  /* Theoretical overflow */
	  /* If we can't put 256 bytes more into the buffer, allocate
	     everything we can and fill it as full as we can. */
	  /* XXX - we ignore rest of line using `truncating' flag */
	  if (shell_input_line_size > (SIZE_MAX - 256))
	    {
	      size_t n;

	      n = SIZE_MAX - i;	/* how much more can we put into the buffer? */
	      if (n <= 2)	/* we have to save 1 for the newline added below */
		{
		  if (truncating == 0)
		    internal_warning(_("shell_getc: shell_input_line_size (%zu) exceeds SIZE_MAX (%lu): line truncated"), shell_input_line_size, (unsigned long)SIZE_MAX);
		  shell_input_line[i] = '\0';
		  truncating = 1;
		}
	      if (shell_input_line_size < SIZE_MAX)
		{
		  shell_input_line_size = SIZE_MAX;
		  shell_input_line = xrealloc (shell_input_line, shell_input_line_size);
		}
	    }
	  else
	    RESIZE_MALLOCED_BUFFER (shell_input_line, i, 2, shell_input_line_size, 256);

	  if (c == EOF)
	    {
	      if (bash_input.type == st_stream)
		clearerr (stdin);

	      if (i == 0)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  if (truncating == 0 || c == '\n')
	    shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      current_command_line_count++;
	      break;
	    }

	  last_was_backslash = last_was_backslash == 0 && c == '\\';
	}

      shell_input_line_index = 0;
      shell_input_line_len = i;		/* == strlen (shell_input_line) */

      set_line_mbstate ();

#if defined (HISTORY)
      if (remember_on_history && shell_input_line && shell_input_line[0])
	{
	  char *expansions;
#  if defined (BANG_HISTORY)
	  /* If the current delimiter is a single quote, we should not be
	     performing history expansion, even if we're on a different
	     line from the original single quote. */
	  if (current_delimiter (dstack) == '\'')
	    history_quoting_state = '\'';
	  else if (current_delimiter (dstack) == '"')
	    history_quoting_state = '"';
	  else
	    history_quoting_state = 0;
#  endif
	  /* Calling with a third argument of 1 allows remember_on_history to
	     determine whether or not the line is saved to the history list */
	  expansions = pre_process_line (shell_input_line, 1, 1);
#  if defined (BANG_HISTORY)
	  history_quoting_state = 0;
#  endif
	  if (expansions != shell_input_line)
	    {
	      free (shell_input_line);
	      shell_input_line = expansions;
	      shell_input_line_len = shell_input_line ?
					strlen (shell_input_line) : 0;
	      if (shell_input_line_len == 0)
		current_command_line_count--;

	      /* We have to force the xrealloc below because we don't know
		 the true allocated size of shell_input_line anymore. */
	      shell_input_line_size = shell_input_line_len;

	      set_line_mbstate ();
	    }
	}
      /* Try to do something intelligent with blank lines encountered while
	 entering multi-line commands.  XXX - this is grotesque */
      else if (remember_on_history && shell_input_line &&
	       shell_input_line[0] == '\0' &&
	       current_command_line_count > 1)
	{
	  if (current_delimiter (dstack))
	    /* We know shell_input_line[0] == 0 and we're reading some sort of
	       quoted string.  This means we've got a line consisting of only
	       a newline in a quoted string.  We want to make sure this line
	       gets added to the history. */
	    maybe_add_history (shell_input_line);
	  else
	    {
	      char *hdcs;
	      hdcs = history_delimiting_chars (shell_input_line);
	      if (hdcs && hdcs[0] == ';')
		maybe_add_history (shell_input_line);
	    }
	}

#endif /* HISTORY */

      if (shell_input_line)
	{
	  /* Lines that signify the end of the shell's input should not be
	     echoed.  We should not echo lines while parsing command
	     substitutions with recursive calls into the parsing engine; those
	     should only be echoed once when we read the word.  That is the
	     reason for the test against shell_eof_token, which is set to a
	     right paren when parsing the contents of command substitutions. */
	  if (echo_input_at_read && (shell_input_line[0] ||
				       shell_input_line_terminator != EOF) &&
				     shell_eof_token == 0)
	    fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  if (SHOULD_PROMPT ())
	    prompt_again (0);
	  goto restart_read;
	}

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  if (shell_input_line_size < SIZE_MAX-3 && (shell_input_line_len+3 > shell_input_line_size))
	    shell_input_line = (char *)xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  /* Don't add a newline to a string that ends with a backslash if we're
	     going to be removing quoted newlines, since that will eat the
	     backslash.  Add another backslash instead (will be removed by
	     word expansion). */
	  if (bash_input.type == st_string && expanding_alias() == 0 && last_was_backslash && c == EOF && remove_quoted_newline)
	    shell_input_line[shell_input_line_len] = '\\';
	  else
	    shell_input_line[shell_input_line_len] = '\n';
	  shell_input_line[shell_input_line_len + 1] = '\0';

#if defined (HANDLE_MULTIBYTE)
	  /* This is kind of an abstraction violation, but there's no need to
	     go through the entire shell_input_line again with a call to
	     set_line_mbstate(). */
	  EXTEND_SHELL_INPUT_LINE_PROPERTY();
	  shell_input_line_property[shell_input_line_len] = 1;
#endif
	}
    }

next_alias_char:
  if (shell_input_line_index == 0)
    unquoted_backslash = 0;

  uc = shell_input_line[shell_input_line_index];

  if (uc)
    {
      unquoted_backslash = unquoted_backslash == 0 && uc == '\\';
      shell_input_line_index++;
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If UC is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  /* If pushed_string_list != 0 but pushed_string_list->expander == 0 (not
     currently tested) and the flags value is not PSH_SOURCE, we are not
     parsing an alias, we have just saved one (push_string, when called by
     the parse_dparen code) In this case, just go on as well.  The PSH_SOURCE
     case is handled below. */

  /* If we're at the end of an alias expansion add a space to make sure that
     the alias remains marked as being in use while we expand its last word.
     This makes sure that pop_string doesn't mark the alias as not in use
     before the string resulting from the alias expansion is tokenized and
     checked for alias expansion, preventing recursion.  At this point, the
     last character in shell_input_line is the last character of the alias
     expansion.  We test that last character to determine whether or not to
     return the space that will delimit the token and postpone the pop_string.
     This set of conditions duplicates what used to be in mk_alexpansion ()
     below, with the addition that we don't add a space if we're currently
     reading a quoted string or in a shell comment. */
#ifndef OLD_ALIAS_HACK
  if (uc == 0 && pushed_string_list && pushed_string_list->flags != PSH_SOURCE &&
      pushed_string_list->flags != PSH_DPAREN &&
      (parser_state & PST_COMMENT) == 0 &&
      (parser_state & PST_ENDALIAS) == 0 &&	/* only once */
      shell_input_line_index > 0 &&
      shellblank (shell_input_line[shell_input_line_index-1]) == 0 &&
      shell_input_line[shell_input_line_index-1] != '\n' &&
      unquoted_backslash == 0 &&
      shellmeta (shell_input_line[shell_input_line_index-1]) == 0 &&
      (current_delimiter (dstack) != '\'' && current_delimiter (dstack) != '"'))
    {
      parser_state |= PST_ENDALIAS;
      /* We need to do this to make sure last_shell_getc_is_singlebyte returns
	 true, since we are returning a single-byte space. */
      if (shell_input_line_index == shell_input_line_len && last_shell_getc_is_singlebyte == 0)
	{
#if 0
	  EXTEND_SHELL_INPUT_LINE_PROPERTY();
	  shell_input_line_property[shell_input_line_len++] = 1;
	  /* extend shell_input_line to accommodate the shell_ungetc that
	     read_token_word() will perform, since we're extending the index */
	  RESIZE_MALLOCED_BUFFER (shell_input_line, shell_input_line_index, 2, shell_input_line_size, 16);
          shell_input_line[++shell_input_line_index] = '\0';	/* XXX */
#else
	  shell_input_line_property[shell_input_line_index - 1] = 1;
#endif
	}
      return ' ';	/* END_ALIAS */
    }
#endif

pop_alias:
#endif /* ALIAS || DPAREN_ARITHMETIC */
  /* This case works for PSH_DPAREN as well as the shell_ungets() case that uses
     push_string */
  if (uc == 0 && pushed_string_list && pushed_string_list->flags != PSH_SOURCE)
    {
      parser_state &= ~PST_ENDALIAS;
      pop_string ();
      uc = shell_input_line[shell_input_line_index];
      if (uc)
	shell_input_line_index++;
    }

  if MBTEST(uc == '\\' && remove_quoted_newline && shell_input_line[shell_input_line_index] == '\n')
    {
	if (SHOULD_PROMPT ())
	  prompt_again (0);
	line_number++;

	/* What do we do here if we're expanding an alias whose definition
	   includes an escaped newline?  If that's the last character in the
	   alias expansion, we just pop the pushed string list (recall that
	   we inhibit the appending of a space if newline is the last
	   character).  If it's not the last character, we need to consume the
	   quoted newline and move to the next character in the expansion. */
#if defined (ALIAS)
	if (expanding_alias () && shell_input_line[shell_input_line_index+1] == '\0')
	  {
	    uc = 0;
	    goto pop_alias;
	  }
	else if (expanding_alias () && shell_input_line[shell_input_line_index+1] != '\0')
	  {
	    shell_input_line_index++;	/* skip newline */
	    goto next_alias_char;	/* and get next character */
	  }
	else
#endif 
	  goto restart_read;
    }

  if (uc == 0 && shell_input_line_terminator == EOF)
    return ((shell_input_line_index != 0) ? '\n' : EOF);

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* We already know that we are not parsing an alias expansion because of the
     check for expanding_alias() above.  This knows how parse_and_execute
     handles switching to st_string input while an alias is being expanded,
     hence the check for pushed_string_list without pushed_string_list->expander
     and the check for PSH_SOURCE as pushed_string_list->flags.
     parse_and_execute and parse_string both change the input type to st_string
     and place the string to be parsed and executed into location.string, so
     we should not stop reading that until the pointer is '\0'.
     The check for shell_input_line_terminator may be superfluous.

     This solves the problem of `.' inside a multi-line alias with embedded
     newlines executing things out of order. */
  if (uc == 0 && bash_input.type == st_string && *bash_input.location.string &&
      pushed_string_list && pushed_string_list->flags == PSH_SOURCE &&
      shell_input_line_terminator == 0)
    {
      shell_input_line_index = 0;
      goto restart_read;
    }
#endif

  return (uc);
}

/* Put C back into the input for the shell.  This might need changes for
   HANDLE_MULTIBYTE around EOLs.  Since we (currently) never push back a
   character different than we read, shell_input_line_property doesn't need
   to change when manipulating shell_input_line.  The define for
   last_shell_getc_is_singlebyte should take care of it, though. */
static void
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
  else
    eol_ungetc_lookahead = c;
}

/* Push S back into shell_input_line; updating shell_input_line_index */
void
shell_ungets (s)
     char *s;
{
  size_t slen, chars_left;

  slen = strlen (s);

  if (shell_input_line[shell_input_line_index] == '\0')
    {
      /* Easy, just overwrite shell_input_line. This is preferred because it
	 saves on set_line_mbstate () and other overhead like push_string */
      if (shell_input_line_size <= slen)
	RESIZE_MALLOCED_BUFFER (shell_input_line, shell_input_line_index, slen + 1, shell_input_line_size, 64);
      strcpy (shell_input_line, s);
      shell_input_line_index = 0;
      shell_input_line_len = slen;
      shell_input_line_terminator = 0;
    }
  else if (shell_input_line_index >= slen)
    {
      /* Just as easy, just back up shell_input_line_index, but it means we
	 will re-process some characters in set_line_mbstate(). Need to
	 watch pushing back newlines here. */
      while (slen > 0)
        shell_input_line[--shell_input_line_index] = s[--slen];
    }
  else if (s[slen - 1] == '\n')
    {
      push_string (savestring (s), 0, (alias_t *)NULL);
      /* push_string does set_line_mbstate () */
      return;
    }
  else
    {
      /* Harder case: pushing back input string that's longer than what we've
	 consumed from shell_input_line so far. */
      INTERNAL_DEBUG (("shell_ungets: not at end of shell_input_line"));

      chars_left = shell_input_line_len - shell_input_line_index;
      if (shell_input_line_size <= (slen + chars_left))
	RESIZE_MALLOCED_BUFFER (shell_input_line, shell_input_line_index, chars_left + slen + 1, shell_input_line_size, 64);
      memmove (shell_input_line + slen, shell_input_line + shell_input_line_index, shell_input_line_len - shell_input_line_index);
      strcpy (shell_input_line, s);
      shell_input_line_index = 0;
      shell_input_line_len = strlen (shell_input_line);	/* chars_left + slen? */
    }

#if defined (HANDLE_MULTIBYTE)
  set_line_mbstate ();	/* XXX */
#endif
}

char *
parser_remaining_input ()
{
  if (shell_input_line == 0)
    return 0;
  if ((int)shell_input_line_index < 0 || shell_input_line_index >= shell_input_line_len)
    return "";	/* XXX */
  return (shell_input_line + shell_input_line_index);
}

#ifdef INCLUDE_UNUSED
/* Back the input pointer up by one, effectively `ungetting' a character. */
static void
shell_ungetchar ()
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line_index--;
}
#endif

/* Discard input until CHARACTER is seen, then push that character back
   onto the input stream. */
static void
discard_until (character)
     int character;
{
  int c;

  while ((c = shell_getc (0)) != EOF && c != character)
    ;

  if (c != EOF)
    shell_ungetc (c);
}

void
execute_variable_command (command, vname)
     char *command, *vname;
{
  char *last_lastarg;
  sh_parser_state_t ps;

  save_parser_state (&ps);
  last_lastarg = get_string_value ("_");
  if (last_lastarg)
    last_lastarg = savestring (last_lastarg);

  parse_and_execute (savestring (command), vname, SEVAL_NONINT|SEVAL_NOHIST|SEVAL_NOOPTIMIZE);

  restore_parser_state (&ps);
  bind_variable ("_", last_lastarg, 0);
  FREE (last_lastarg);

  if (token_to_read == '\n')	/* reset_parser was called */
    token_to_read = 0;
}

void
push_token (x)
     int x;
{
  two_tokens_ago = token_before_that;
  token_before_that = last_read_token;
  last_read_token = current_token;

  current_token = x;
}

/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
static char *token = (char *)NULL;

/* Current size of the token buffer. */
static size_t token_buffer_size;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */
static int
yylex ()
{
  if (interactive && (current_token == 0 || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (prompt_is_ps1 && parse_and_execute_level == 0 && time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Avoid printing a prompt if we're not going to read anything, e.g.
	 after resetting the parser with read_token (RESET). */
      if (token_to_read == 0 && SHOULD_PROMPT ())
	prompt_again (0);
    }

  two_tokens_ago = token_before_that;
  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);

  if ((parser_state & PST_EOFTOKEN) && current_token == shell_eof_token)
    {
      /* placeholder for any special handling. */
      return (current_token);
    }

  if (current_token < 0)
#if defined (YYERRCODE) && !defined (YYUNDEF)
    current_token = EOF_Reached ? YYEOF : YYERRCODE;
#else
    current_token = EOF_Reached ? YYEOF : YYUNDEF;
#endif

  return (current_token);
}

/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int esacs_needed_count;

/* When non-zero, we can read IN as an acceptable token, regardless of how
   many newlines we read. */
static int expecting_in_token;

static void
push_heredoc (r)
     REDIRECT *r;
{
  if (need_here_doc >= HEREDOC_MAX)
    {
      last_command_exit_value = EX_BADUSAGE;
      need_here_doc = 0;
      report_syntax_error (_("maximum here-document count exceeded"));
      reset_parser ();
      exit_shell (last_command_exit_value);
    }
  redir_stack[need_here_doc++] = r;
}

void
gather_here_documents ()
{
  int r;

  r = 0;
  here_doc_first_line = 1;
  while (need_here_doc > 0)
    {
      parser_state |= PST_HEREDOC;
      make_here_document (redir_stack[r++], line_number);
      parser_state &= ~PST_HEREDOC;
      need_here_doc--;
      redir_stack[r - 1] = 0;		/* XXX */
    }
  here_doc_first_line = 0;		/* just in case */
}

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_count;

/* In the following three macros, `token' is always last_read_token */

/* Are we in the middle of parsing a redirection where we are about to read
   a word?  This is used to make sure alias expansion doesn't happen in the
   middle of a redirection, even though we're parsing a simple command. */
#define parsing_redirection(token) \
  (token == '<' || token == '>' || \
   token == GREATER_GREATER || token == GREATER_BAR || \
   token == LESS_GREATER || token == LESS_LESS_MINUS || \
   token == LESS_LESS || token == LESS_LESS_LESS || \
   token == LESS_AND || token == GREATER_AND || token == AND_GREATER)

/* Is `token' one that will allow a WORD to be read in a command position?
   We can read a simple command name on which we should attempt alias expansion
   or we can read an assignment statement. */
#define command_token_position(token) \
  (((token) == ASSIGNMENT_WORD) || \
   ((parser_state&PST_REDIRLIST) && parsing_redirection(token) == 0) || \
   ((token) != SEMI_SEMI && (token) != SEMI_AND && (token) != SEMI_SEMI_AND && reserved_word_acceptable(token)))

/* Are we in a position where we can read an assignment statement? */
#define assignment_acceptable(token) \
  (command_token_position(token) && ((parser_state & PST_CASEPAT) == 0))

/* Check to see if TOKEN is a reserved word and return the token
   value if it is. */
#define CHECK_FOR_RESERVED_WORD(tok) \
  do { \
    if (!dollar_present && !quoted && \
	reserved_word_acceptable (last_read_token)) \
      { \
	int i; \
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++) \
	  if (STREQ (tok, word_token_alist[i].word)) \
	    { \
	      if ((parser_state & PST_CASEPAT) && (word_token_alist[i].token != ESAC)) \
		break; \
	      if (word_token_alist[i].token == TIME && time_command_acceptable () == 0) \
		break; \
	      if ((parser_state & PST_CASEPAT) && last_read_token == '|' && word_token_alist[i].token == ESAC) \
		break; /* Posix grammar rule 4 */ \
	      if ((parser_state & PST_CASEPAT) && last_read_token == '(' && word_token_alist[i].token == ESAC) /*)*/ \
		break; /* phantom Posix grammar rule 4 */ \
	      if (word_token_alist[i].token == ESAC) { \
		parser_state &= ~(PST_CASEPAT|PST_CASESTMT); \
		esacs_needed_count--; \
	      } else if (word_token_alist[i].token == CASE) \
		parser_state |= PST_CASESTMT; \
	      else if (word_token_alist[i].token == COND_END) \
		parser_state &= ~(PST_CONDCMD|PST_CONDEXPR); \
	      else if (word_token_alist[i].token == COND_START) \
		parser_state |= PST_CONDCMD; \
	      else if (word_token_alist[i].token == '{') \
		open_brace_count++; \
	      else if (word_token_alist[i].token == '}' && open_brace_count) \
		open_brace_count--; \
	      return (word_token_alist[i].token); \
	    } \
      } \
  } while (0)

#if defined (ALIAS)

    /* OK, we have a token.  Let's try to alias expand it, if (and only if)
       it's eligible.

       It is eligible for expansion if EXPAND_ALIASES is set, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (parser_state & PST_CASEPAT). */

static char *
mk_alexpansion (s)
     char *s;
{
  int l;
  char *r;

  l = strlen (s);
  r = xmalloc (l + 2);
  strcpy (r, s);
#ifdef OLD_ALIAS_HACK
  /* If the last character in the alias is a newline, don't add a trailing
     space to the expansion.  Works with shell_getc above. */
  /* Need to do something about the case where the alias expansion contains
     an unmatched quoted string, since appending this space affects the
     subsequent output. */
  if (l > 0 && r[l - 1] != ' ' && r[l - 1] != '\n' && shellmeta(r[l - 1]) == 0)
    r[l++] = ' ';
#endif
  r[l] = '\0';
  return r;
}

static int
alias_expand_token (tokstr)
     char *tokstr;
{
  char *expanded;
  alias_t *ap;

#if 0
  if (((parser_state & PST_ALEXPNEXT) || command_token_position (last_read_token)) &&
	(parser_state & PST_CASEPAT) == 0)
#else
  if ((parser_state & PST_ALEXPNEXT) || assignment_acceptable (last_read_token))
#endif
    {
      ap = find_alias (tokstr);

      /* Currently expanding this token. */
      if (ap && (ap->flags & AL_BEINGEXPANDED))
	return (NO_EXPANSION);

#ifdef OLD_ALIAS_HACK
      /* mk_alexpansion puts an extra space on the end of the alias expansion,
	 so the lookahead by the parser works right (the alias needs to remain
	 `in use' while parsing its last word to avoid alias recursion for
	 something like "alias echo=echo").  If this gets changed, make sure
	 the code in shell_getc that deals with reaching the end of an
	 expanded alias is changed with it. */
#endif
      expanded = ap ? mk_alexpansion (ap->value) : (char *)NULL;

      if (expanded)
	{
	  push_string (expanded, ap->flags & AL_EXPANDNEXT, ap);
	  return (RE_READ_TOKEN);
	}
      else
	/* This is an eligible token that does not have an expansion. */
	return (NO_EXPANSION);
    }
  return (NO_EXPANSION);
}
#endif /* ALIAS */

static int
time_command_acceptable ()
{
#if defined (COMMAND_TIMING)
  int i;

  if (posixly_correct && shell_compatibility_level > 41)
    {
      /* Quick check of the rest of the line to find the next token.  If it
	 begins with a `-', Posix says to not return `time' as the token.
	 This was interp 267. */
      i = shell_input_line_index;
      while (i < shell_input_line_len && (shell_input_line[i] == ' ' || shell_input_line[i] == '\t'))
        i++;
      if (shell_input_line[i] == '-')
	return 0;
    }

  switch (last_read_token)
    {
    case 0:
    case ';':
    case '\n':
      if (token_before_that == '|')
	return (0);
      /* FALLTHROUGH */
    case AND_AND:
    case OR_OR:
    case '&':
    case WHILE:
    case DO:
    case UNTIL:
    case IF:
    case THEN:
    case ELIF:
    case ELSE:
    case '{':		/* } */
    case '(':		/* )( */
    case ')':		/* only valid in case statement */
    case BANG:		/* ! time pipeline */
    case TIME:		/* time time pipeline */
    case TIMEOPT:	/* time -p time pipeline */
    case TIMEIGN:	/* time -p -- ... */
    case DOLPAREN:
      return 1;
    default:
      return 0;
    }
#else
  return 0;
#endif /* COMMAND_TIMING */
}

/* Handle special cases of token recognition:
	IN is recognized if the last token was WORD and the token
	before that was FOR or CASE or SELECT.

	DO is recognized if the last token was WORD and the token
	before that was FOR or SELECT.

	ESAC is recognized if the last token caused `esacs_needed_count'
	to be set

	`{' is recognized if the last token as WORD and the token
	before that was FUNCTION, or if we just parsed an arithmetic
	`for' command.

	`}' is recognized if there is an unclosed `{' present.

	`-p' is returned as TIMEOPT if the last read token was TIME.
	`--' is returned as TIMEIGN if the last read token was TIME or TIMEOPT.

	']]' is returned as COND_END if the parser is currently parsing
	a conditional expression ((parser_state & PST_CONDEXPR) != 0)

	`time' is returned as TIME if and only if it is immediately
	preceded by one of `;', `\n', `||', `&&', or `&'.
*/

static int
special_case_tokens (tokstr)
     char *tokstr;
{
  /* Posix grammar rule 6 */
  if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
      ((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
      ((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
      (tokstr[0] == 'i' && tokstr[1] == 'n' && tokstr[2] == 0))
    {
      if (token_before_that == CASE)
	{
	  parser_state |= PST_CASEPAT;
	  esacs_needed_count++;
	}
      if (expecting_in_token)
	expecting_in_token--;
      return (IN);
    }

  /* XXX - leaving above code intact for now, but it should eventually be
     removed in favor of this clause. */
  /* Posix grammar rule 6 */
  if (expecting_in_token && (last_read_token == WORD || last_read_token == '\n') &&
      (tokstr[0] == 'i' && tokstr[1] == 'n' && tokstr[2] == 0))
    {
      if (parser_state & PST_CASESTMT)
	{
	  parser_state |= PST_CASEPAT;
	  esacs_needed_count++;
	}
      expecting_in_token--;
      return (IN);
    }
  /* Posix grammar rule 6, third word in FOR: for i; do command-list; done */
  else if (expecting_in_token && (last_read_token == '\n' || last_read_token == ';') &&
    (tokstr[0] == 'd' && tokstr[1] == 'o' && tokstr[2] == '\0'))
    {
      expecting_in_token--;
      return (DO);
    }

  /* for i do; command-list; done */
  if (last_read_token == WORD &&
#if defined (SELECT_COMMAND)
      (token_before_that == FOR || token_before_that == SELECT) &&
#else
      (token_before_that == FOR) &&
#endif
      (tokstr[0] == 'd' && tokstr[1] == 'o' && tokstr[2] == '\0'))
    {
      if (expecting_in_token)
	expecting_in_token--;
      return (DO);
    }

  /* Ditto for ESAC in the CASE case.
     Specifically, this handles "case word in esac", which is a legal
     construct, certainly because someone will pass an empty arg to the
     case construct, and we don't want it to barf.  Of course, we should
     insist that the case construct has at least one pattern in it, but
     the designers disagree. */
  if (esacs_needed_count)
    {
      if (last_read_token == IN && STREQ (tokstr, "esac"))
	{
	  esacs_needed_count--;
	  parser_state &= ~PST_CASEPAT;
	  return (ESAC);
	}
    }

  /* The start of a shell function definition. */
  if (parser_state & PST_ALLOWOPNBRC)
    {
      parser_state &= ~PST_ALLOWOPNBRC;
      if (tokstr[0] == '{' && tokstr[1] == '\0')		/* } */
	{
	  open_brace_count++;
	  function_bstart = line_number;
	  return ('{');					/* } */
	}
    }

  /* We allow a `do' after a for ((...)) without an intervening
     list_terminator */
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == 'd' && tokstr[1] == 'o' && !tokstr[2])
    return (DO);
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == '{' && tokstr[1] == '\0')	/* } */
    {
      open_brace_count++;
      return ('{');			/* } */
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && tokstr[0] == '}' && !tokstr[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

#if defined (COMMAND_TIMING)
  /* Handle -p after `time'. */
  if (last_read_token == TIME && tokstr[0] == '-' && tokstr[1] == 'p' && !tokstr[2])
    return (TIMEOPT);
  /* Handle -- after `time'. */
  if (last_read_token == TIME && tokstr[0] == '-' && tokstr[1] == '-' && !tokstr[2])
    return (TIMEIGN);
  /* Handle -- after `time -p'. */
  if (last_read_token == TIMEOPT && tokstr[0] == '-' && tokstr[1] == '-' && !tokstr[2])
    return (TIMEIGN);
#endif

#if defined (COND_COMMAND) /* [[ */
  if ((parser_state & PST_CONDEXPR) && tokstr[0] == ']' && tokstr[1] == ']' && tokstr[2] == '\0')
    return (COND_END);
#endif

  return (-1);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
void
reset_parser ()
{
  dstack.delimiter_depth = 0;	/* No delimiters found so far. */
  open_brace_count = 0;

#if defined (EXTENDED_GLOB)
  /* Reset to global value of extended glob */
  if (parser_state & (PST_EXTPAT|PST_CMDSUBST))
    extended_glob = extglob_flag;
#endif
  if (parser_state & (PST_CMDSUBST|PST_STRING))
    expand_aliases = expaliases_flag;

  parser_state = 0;
  here_doc_first_line = 0;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  if (pushed_string_list)
    free_string_list ();
#endif /* ALIAS || DPAREN_ARITHMETIC */

  /* This is where we resynchronize to the next newline on error/reset */
  if (shell_input_line)
    {
      free (shell_input_line);
      shell_input_line = (char *)NULL;
      shell_input_line_size = shell_input_line_index = 0;
    }

  FREE (word_desc_to_read);
  word_desc_to_read = (WORD_DESC *)NULL;

  eol_ungetc_lookahead = 0;

  /* added post-bash-5.1 */
  need_here_doc = 0;
  redir_stack[0] = 0;
  esacs_needed_count = expecting_in_token = 0;

  current_token = '\n';		/* XXX */
  last_read_token = '\n';
  token_to_read = '\n';
}

void
reset_readahead_token ()
{
  if (token_to_read == '\n')
    token_to_read = 0;
}

/* Read the next token.  Command can be READ (normal operation) or
   RESET (to normalize state). */
static int
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */

  if (command == RESET)
    {
      reset_parser ();
      return ('\n');
    }

  if (token_to_read)
    {
      result = token_to_read;
      if (token_to_read == WORD || token_to_read == ASSIGNMENT_WORD)
	{
	  yylval.word = word_desc_to_read;
	  word_desc_to_read = (WORD_DESC *)NULL;
	}
      token_to_read = 0;
      return (result);
    }

#if defined (COND_COMMAND)
  if ((parser_state & (PST_CONDCMD|PST_CONDEXPR)) == PST_CONDCMD)
    {
      cond_lineno = line_number;
      parser_state |= PST_CONDEXPR;
      yylval.command = parse_cond_command ();
      if (cond_token != COND_END)
	{
	  cond_error ();
	  return (-1);
	}
      token_to_read = COND_END;
      parser_state &= ~(PST_CONDEXPR|PST_CONDCMD);
      return (COND_CMD);
    }
#endif

#if defined (ALIAS)
  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
 re_read_token:
#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && shellblank (character))
    ;

  if (character == EOF)
    {
      EOF_Reached = 1;
      return (yacc_EOF);
    }

  /* If we hit the end of the string and we're not expanding an alias (e.g.,
     we are eval'ing a string that is an incomplete command), return EOF */
  if (character == '\0' && bash_input.type == st_string && expanding_alias() == 0)
    {
      INTERNAL_DEBUG (("shell_getc: bash_input.location.string = `%s'", bash_input.location.string));
      EOF_Reached = 1;
      return (yacc_EOF);
    }

  if MBTEST(character == '#' && (!interactive || interactive_comments))
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      parser_state |= PST_COMMENT;
      discard_until ('\n');
      shell_getc (0);
      parser_state &= ~PST_COMMENT;
      character = '\n';	/* this will take the next if statement and return. */
    }

  if MBTEST(character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	gather_here_documents ();

#if defined (ALIAS)
      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      parser_state &= ~PST_ASSIGNOK;

      return (character);
    }

  if (parser_state & PST_REGEXP)
    goto tokword;

  /* Shell meta-characters. */
  if MBTEST(shellmeta (character))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      parser_state &= ~PST_ASSIGNOK;

      /* If we are parsing a command substitution and we have read a character
	 that marks the end of it, don't bother to skip over quoted newlines
	 when we read the next token. We're just interested in a character
	 that will turn this into a two-character token, so we let the higher
	 layers deal with quoted newlines following the command substitution. */
      if ((parser_state & PST_CMDSUBST) && character == shell_eof_token)
	peek_char = shell_getc (0);
      else
	peek_char = shell_getc (1);

      if MBTEST(character == peek_char)
	{
	  switch (character)
	    {
	    case '<':
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	      peek_char = shell_getc (1);
	      if MBTEST(peek_char == '-')
		return (LESS_LESS_MINUS);
	      else if MBTEST(peek_char == '<')
		return (LESS_LESS_LESS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '>':
	      return (GREATER_GREATER);

	    case ';':
	      parser_state |= PST_CASEPAT;
#if defined (ALIAS)
	      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

	      peek_char = shell_getc (1);
	      if MBTEST(peek_char == '&')
		return (SEMI_SEMI_AND);
	      else
		{
		  shell_ungetc (peek_char);
		  return (SEMI_SEMI);
		}

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
	    case '(':		/* ) */
	      result = parse_dparen (character);
	      if (result == -2)
	        break;
	      else
	        return result;
#endif
	    }
	}
      else if MBTEST(character == '<' && peek_char == '&')
	return (LESS_AND);
      else if MBTEST(character == '>' && peek_char == '&')
	return (GREATER_AND);
      else if MBTEST(character == '<' && peek_char == '>')
	return (LESS_GREATER);
      else if MBTEST(character == '>' && peek_char == '|')
	return (GREATER_BAR);
      else if MBTEST(character == '&' && peek_char == '>')
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '>')
	    return (AND_GREATER_GREATER);
	  else
	    {
	      shell_ungetc (peek_char);
	      return (AND_GREATER);
	    }
	}
      else if MBTEST(character == '|' && peek_char == '&')
	return (BAR_AND);
      else if MBTEST(character == ';' && peek_char == '&')
	{
	  parser_state |= PST_CASEPAT;
#if defined (ALIAS)
	  parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	  return (SEMI_AND);
	}

      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if MBTEST(character == ')' && last_read_token == '(' && token_before_that == WORD)
	{
	  parser_state |= PST_ALLOWOPNBRC;
#if defined (ALIAS)
	  parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	  function_dstart = line_number;
	}

      /* case pattern lists may be preceded by an optional left paren.  If
	 we're not trying to parse a case pattern list, the left paren
	 indicates a subshell. */
      if MBTEST(character == '(' && (parser_state & PST_CASEPAT) == 0) /* ) */
	parser_state |= PST_SUBSHELL;
      /*(*/
      else if MBTEST((parser_state & PST_CASEPAT) && character == ')')
	parser_state &= ~PST_CASEPAT;
      /*(*/
      else if MBTEST((parser_state & PST_SUBSHELL) && character == ')')
	parser_state &= ~PST_SUBSHELL;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if MBTEST((character != '>' && character != '<') || peek_char != '(') /*)*/
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case.  Also <&N- (dup and close). */
  if MBTEST(character == '-' && (last_read_token == LESS_AND || last_read_token == GREATER_AND))
    return (character);

tokword:
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  result = read_token_word (character);
#if defined (ALIAS)
  if (result == RE_READ_TOKEN)
    goto re_read_token;
#endif
  return result;
}

/*
 * Match a $(...) or other grouping construct.  This has to handle embedded
 * quoted strings ('', ``, "") and nested constructs.  It also must handle
 * reprompting the user, if necessary, after reading a newline, and returning
 * correct error values if it reads EOF.
 */
#define P_FIRSTCLOSE	0x0001
#define P_ALLOWESC	0x0002
#define P_DQUOTE	0x0004
#define P_COMMAND	0x0008	/* parsing a command, so look for comments */
#define P_BACKQUOTE	0x0010	/* parsing a backquoted command substitution */
#define P_ARRAYSUB	0x0020	/* parsing a [...] array subscript for assignment */
#define P_DOLBRACE	0x0040	/* parsing a ${...} construct */
#define P_ARITH		0x0080	/* parsing a $(( )) arithmetic expansion */

/* Lexical state while parsing a grouping construct or $(...). */
#define LEX_WASDOL	0x0001
#define LEX_CKCOMMENT	0x0002
#define LEX_INCOMMENT	0x0004
#define LEX_PASSNEXT	0x0008
#define LEX_RESWDOK	0x0010
#define LEX_CKCASE	0x0020
#define LEX_INCASE	0x0040
#define LEX_INHEREDOC	0x0080
#define LEX_HEREDELIM	0x0100		/* reading here-doc delimiter */
#define LEX_STRIPDOC	0x0200		/* <<- strip tabs from here doc delim */
#define LEX_QUOTEDDOC	0x0400		/* here doc with quoted delim */
#define LEX_INWORD	0x0800
#define LEX_GTLT	0x1000
#define LEX_CKESAC	0x2000		/* check esac after in -- for later */
#define LEX_CASEWD	0x4000		/* word after case */
#define LEX_PATLIST	0x8000		/* case statement pattern list */

#define COMSUB_META(ch)		((ch) == ';' || (ch) == '&' || (ch) == '|')

#define CHECK_NESTRET_ERROR() \
  do { \
    if (nestret == &matched_pair_error) \
      { \
	free (ret); \
	return &matched_pair_error; \
      } \
  } while (0)

#define APPEND_NESTRET() \
  do { \
    if (nestlen) \
      { \
	RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64); \
	strcpy (ret + retind, nestret); \
	retind += nestlen; \
      } \
  } while (0)

static char matched_pair_error;

static char *
parse_matched_pair (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int count, ch, prevch, tflags;
  int nestlen, ttranslen, start_lineno;
  char *ret, *nestret, *ttrans;
  int retind, retsize, rflags;
  int dolbrace_state;

  dolbrace_state = (flags & P_DOLBRACE) ? DOLBRACE_PARAM : 0;

/*itrace("parse_matched_pair[%d]: open = %c close = %c flags = %d", line_number, open, close, flags);*/
  count = 1;
  tflags = 0;

  if ((flags & P_COMMAND) && qc != '`' && qc != '\'' && qc != '"' && (flags & P_DQUOTE) == 0)
    tflags |= LEX_CKCOMMENT;

  /* RFLAGS is the set of flags we want to pass to recursive calls. */
  rflags = (qc == '"') ? P_DQUOTE : (flags & P_DQUOTE);

  ret = (char *)xmalloc (retsize = 64);
  retind = 0;

  start_lineno = line_number;
  ch = EOF;		/* just in case */
  while (count)
    {
      prevch = ch;
      ch = shell_getc (qc != '\'' && (tflags & (LEX_PASSNEXT)) == 0);

      if (ch == EOF)
	{
	  free (ret);
	  parser_error (start_lineno, _("unexpected EOF while looking for matching `%c'"), close);
	  EOF_Reached = 1;	/* XXX */
	  parser_state |= PST_NOERROR;  /* avoid redundant error message */
	  return (&matched_pair_error);
	}

      /* Possible reprompting. */
      if MBTEST(ch == '\n' && SHOULD_PROMPT ())
	prompt_again (0);

      /* Don't bother counting parens or doing anything else if in a comment
	 or part of a case statement */
      if (tflags & LEX_INCOMMENT)
	{
	  /* Add this character. */
	  RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
	  ret[retind++] = ch;

	  if MBTEST(ch == '\n')
	    tflags &= ~LEX_INCOMMENT;

	  continue;
	}

      /* Not exactly right yet, should handle shell metacharacters, too.  If
	 any changes are made to this test, make analogous changes to subst.c:
	 extract_delimited_string(). */
      else if MBTEST((tflags & LEX_CKCOMMENT) && (tflags & LEX_INCOMMENT) == 0 && ch == '#' && (retind == 0 || ret[retind-1] == '\n' || shellblank (ret[retind - 1])))
	tflags |= LEX_INCOMMENT;

      if (tflags & LEX_PASSNEXT)		/* last char was backslash */
	{
	  tflags &= ~LEX_PASSNEXT;
	  /* XXX - PST_NOEXPAND? */
	  if MBTEST(qc != '\'' && ch == '\n')	/* double-quoted \<newline> disappears. */
	    {
	      if (retind > 0)
		retind--;	/* swallow previously-added backslash */
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  if MBTEST(ch == CTLESC)
	    ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      /* If we're reparsing the input (e.g., from parse_string_to_word_list),
	 we've already prepended CTLESC to single-quoted results of $'...'.
	 We may want to do this for other CTLESC-quoted characters in
	 reparse, too. */
      else if MBTEST((parser_state & PST_REPARSE) && open == '\'' && (ch == CTLESC || ch == CTLNUL))
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == CTLESC || ch == CTLNUL)	/* special shell escapes */
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == close)		/* ending delimiter */
	count--;
      /* handle nested ${...} specially. */
      else if MBTEST(open != close && (tflags & LEX_WASDOL) && open == '{' && ch == open) /* } */
	count++;
      else if MBTEST(((flags & P_FIRSTCLOSE) == 0) && ch == open)	/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      /* If we just read the ending character, don't bother continuing. */
      if (count == 0)
	break;

      if (open == '\'')			/* '' inside grouping construct */
	{
	  if MBTEST((flags & P_ALLOWESC) && ch == '\\')
	    tflags |= LEX_PASSNEXT;
	  continue;
	}

      if MBTEST(ch == '\\')			/* backslashes */
	tflags |= LEX_PASSNEXT;

      /* Based on which dolstate is currently in (param, op, or word),
	 decide what the op is.  We're really only concerned if it's % or
	 #, so we can turn on a flag that says whether or not we should
	 treat single quotes as special when inside a double-quoted
	 ${...}. This logic must agree with subst.c:extract_dollar_brace_string
	 since they share the same defines. */
      /* FLAG POSIX INTERP 221 */
      if (flags & P_DOLBRACE)
        {
          /* ${param%[%]word} */
	  if MBTEST(dolbrace_state == DOLBRACE_PARAM && ch == '%' && retind > 1)
	    dolbrace_state = DOLBRACE_QUOTE;
          /* ${param#[#]word} */
	  else if MBTEST(dolbrace_state == DOLBRACE_PARAM && ch == '#' && retind > 1)
	    dolbrace_state = DOLBRACE_QUOTE;
          /* ${param/[/]pat/rep} */
	  else if MBTEST(dolbrace_state == DOLBRACE_PARAM && ch == '/' && retind > 1)
	    dolbrace_state = DOLBRACE_QUOTE2;	/* XXX */
          /* ${param^[^]pat} */
	  else if MBTEST(dolbrace_state == DOLBRACE_PARAM && ch == '^' && retind > 1)
	    dolbrace_state = DOLBRACE_QUOTE;
          /* ${param,[,]pat} */
	  else if MBTEST(dolbrace_state == DOLBRACE_PARAM && ch == ',' && retind > 1)
	    dolbrace_state = DOLBRACE_QUOTE;
	  else if MBTEST(dolbrace_state == DOLBRACE_PARAM && strchr ("#%^,~:-=?+/", ch) != 0)
	    dolbrace_state = DOLBRACE_OP;
	  else if MBTEST(dolbrace_state == DOLBRACE_OP && strchr ("#%^,~:-=?+/", ch) == 0)
	    dolbrace_state = DOLBRACE_WORD;
        }

      /* The big hammer.  Single quotes aren't special in double quotes.  The
         problem is that Posix used to say the single quotes are semi-special:
         within a double-quoted ${...} construct "an even number of
         unescaped double-quotes or single-quotes, if any, shall occur." */
      /* This was changed in Austin Group Interp 221 */
      if MBTEST(posixly_correct && shell_compatibility_level > 41 && dolbrace_state != DOLBRACE_QUOTE && dolbrace_state != DOLBRACE_QUOTE2 && (flags & P_DQUOTE) && (flags & P_DOLBRACE) && ch == '\'')
	continue;

      /* Could also check open == '`' if we want to parse grouping constructs
	 inside old-style command substitution. */
      if (open != close)		/* a grouping construct */
	{
	  if MBTEST(shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      if MBTEST((tflags & LEX_WASDOL) && ch == '\'')	/* $'...' inside group */
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, P_ALLOWESC|rflags);
	      else
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, rflags);
	      pop_delimiter (dstack);
	      CHECK_NESTRET_ERROR ();

	      if MBTEST((tflags & LEX_WASDOL) && ch == '\'' && (extended_quote || (rflags & P_DQUOTE) == 0 || dolbrace_state == DOLBRACE_QUOTE || dolbrace_state == DOLBRACE_QUOTE2))
		{
		  /* Translate $'...' here. */
		  /* PST_NOEXPAND */
		  ttrans = ansiexpand (nestret, 0, nestlen - 1, &ttranslen);
		  free (nestret);

		  /* If we're parsing a double-quoted brace expansion and we are
		     not in a place where single quotes are treated specially,
		     make sure we single-quote the results of the ansi
		     expansion because quote removal should remove them later */
		  /* FLAG POSIX INTERP 221 */
		  if ((shell_compatibility_level > 42) && (rflags & P_DQUOTE) && (dolbrace_state == DOLBRACE_QUOTE2 || dolbrace_state == DOLBRACE_QUOTE) && (flags & P_DOLBRACE))
		    {
		      nestret = sh_single_quote (ttrans);
		      free (ttrans);
		      nestlen = strlen (nestret);
		    }
#if 0 /* TAG:bash-5.3 */
		  /* This single-quotes PARAM in ${PARAM OP WORD} when PARAM
		     contains a $'...' even when extended_quote is set. */
		  else if ((rflags & P_DQUOTE) && (dolbrace_state == DOLBRACE_PARAM) && (flags & P_DOLBRACE))
		    {
		      nestret = sh_single_quote (ttrans);
		      free (ttrans);
		      nestlen = strlen (nestret);
		    }
#endif
		  else if ((rflags & P_DQUOTE) == 0)
		    {
		      nestret = sh_single_quote (ttrans);
		      free (ttrans);
		      nestlen = strlen (nestret);
		    }
		  else
		    {
		      /* Should we quote CTLESC here? */
		      nestret = ttrans;
		      nestlen = ttranslen;
		    }
		  retind -= 2;		/* back up before the $' */
		}
#if defined (TRANSLATABLE_STRINGS)
	      else if MBTEST((tflags & LEX_WASDOL) && ch == '"' && (extended_quote || (rflags & P_DQUOTE) == 0))
		{
		  /* Locale expand $"..." here. */
		  /* PST_NOEXPAND */
		  ttrans = locale_expand (nestret, 0, nestlen - 1, start_lineno, &ttranslen);
		  free (nestret);

		  /* If we're supposed to single-quote translated strings,
		     check whether the translated result is different from
		     the original and single-quote the string if it is. */
		  if (singlequote_translations &&
		        ((nestlen - 1) != ttranslen || STREQN (nestret, ttrans, ttranslen) == 0))
		    {
		      if ((rflags & P_DQUOTE) == 0)
			nestret = sh_single_quote (ttrans);
		      else if ((rflags & P_DQUOTE) && (dolbrace_state == DOLBRACE_QUOTE2) && (flags & P_DOLBRACE))
			nestret = sh_single_quote (ttrans);
		      else
			/* single quotes aren't special, use backslash instead */
			nestret = sh_backslash_quote_for_double_quotes (ttrans, 0);
		    }
		  else
		    nestret = sh_mkdoublequoted (ttrans, ttranslen, 0);
		  free (ttrans);
		  nestlen = strlen (nestret);
		  retind -= 2;		/* back up before the $" */
		}
#endif /* TRANSLATABLE_STRINGS */

	      APPEND_NESTRET ();
	      FREE (nestret);
	    }
	  else if ((flags & (P_ARRAYSUB|P_DOLBRACE)) && (tflags & LEX_WASDOL) && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	    goto parse_dollar_word;
	  else if ((flags & P_ARITH) && (tflags & LEX_WASDOL) && ch == '(') /*)*/
	    /* $() inside $(( ))/$[ ] */
	    goto parse_dollar_word;
#if defined (PROCESS_SUBSTITUTION)
	  /* XXX - technically this should only be recognized at the start of
	     a word */
	  else if ((flags & (P_ARRAYSUB|P_DOLBRACE)) && (tflags & LEX_GTLT) && (ch == '('))	/* ) */
	    goto parse_dollar_word;
#endif
	}
      /* Parse an old-style command substitution within double quotes as a
	 single word. */
      /* XXX - sh and ksh93 don't do this - XXX */
      else if MBTEST(open == '"' && ch == '`')
	{
	  nestret = parse_matched_pair (0, '`', '`', &nestlen, rflags);

	  CHECK_NESTRET_ERROR ();
	  APPEND_NESTRET ();

	  FREE (nestret);
	}
      else if MBTEST(open != '`' && (tflags & LEX_WASDOL) && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	/* check for $(), $[], or ${} inside quoted string. */
	{
parse_dollar_word:
	  if (open == ch)	/* undo previous increment */
	    count--;
	  if (ch == '(')		/* ) */
	    nestret = parse_comsub (0, '(', ')', &nestlen, (rflags|P_COMMAND) & ~P_DQUOTE);
	  else if (ch == '{')		/* } */
	    nestret = parse_matched_pair (0, '{', '}', &nestlen, P_FIRSTCLOSE|P_DOLBRACE|rflags);
	  else if (ch == '[')		/* ] */
	    nestret = parse_matched_pair (0, '[', ']', &nestlen, rflags|P_ARITH);

	  CHECK_NESTRET_ERROR ();
	  APPEND_NESTRET ();

	  FREE (nestret);
	}
#if defined (PROCESS_SUBSTITUTION)
      if MBTEST((ch == '<' || ch == '>') && (tflags & LEX_GTLT) == 0)
	tflags |= LEX_GTLT;
      else
	tflags &= ~LEX_GTLT;
#endif
      if MBTEST(ch == '$' && (tflags & LEX_WASDOL) == 0)
	tflags |= LEX_WASDOL;
      else
	tflags &= ~LEX_WASDOL;
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
/*itrace("parse_matched_pair[%d]: returning %s", line_number, ret);*/
  return ret;
}

#if defined (DEBUG)
static void
dump_tflags (flags)
     int flags;
{
  int f;

  f = flags;
  fprintf (stderr, "%d -> ", f);
  if (f & LEX_WASDOL)
    {
      f &= ~LEX_WASDOL;
      fprintf (stderr, "LEX_WASDOL%s", f ? "|" : "");
    }
  if (f & LEX_CKCOMMENT)
    {
      f &= ~LEX_CKCOMMENT;
      fprintf (stderr, "LEX_CKCOMMENT%s", f ? "|" : "");
    }
  if (f & LEX_INCOMMENT)
    {
      f &= ~LEX_INCOMMENT;
      fprintf (stderr, "LEX_INCOMMENT%s", f ? "|" : "");
    }
  if (f & LEX_PASSNEXT)
    {
      f &= ~LEX_PASSNEXT;
      fprintf (stderr, "LEX_PASSNEXT%s", f ? "|" : "");
    }
  if (f & LEX_RESWDOK)
    {
      f &= ~LEX_RESWDOK;
      fprintf (stderr, "LEX_RESWDOK%s", f ? "|" : "");
    }
  if (f & LEX_CKCASE)
    {
      f &= ~LEX_CKCASE;
      fprintf (stderr, "LEX_CKCASE%s", f ? "|" : "");
    }
  if (f & LEX_CKESAC)
    {
      f &= ~LEX_CKESAC;
      fprintf (stderr, "LEX_CKESAC%s", f ? "|" : "");
    }
  if (f & LEX_INCASE)
    {
      f &= ~LEX_INCASE;
      fprintf (stderr, "LEX_INCASE%s", f ? "|" : "");
    }
  if (f & LEX_CASEWD)
    {
      f &= ~LEX_CASEWD;
      fprintf (stderr, "LEX_CASEWD%s", f ? "|" : "");
    }
  if (f & LEX_PATLIST)
    {
      f &= ~LEX_PATLIST;
      fprintf (stderr, "LEX_PATLIST%s", f ? "|" : "");
    }
  if (f & LEX_INHEREDOC)
    {
      f &= ~LEX_INHEREDOC;
      fprintf (stderr, "LEX_INHEREDOC%s", f ? "|" : "");
    }
  if (f & LEX_HEREDELIM)
    {
      f &= ~LEX_HEREDELIM;
      fprintf (stderr, "LEX_HEREDELIM%s", f ? "|" : "");
    }
  if (f & LEX_STRIPDOC)
    {
      f &= ~LEX_STRIPDOC;
      fprintf (stderr, "LEX_WASDOL%s", f ? "|" : "");
    }
  if (f & LEX_QUOTEDDOC)
    {
      f &= ~LEX_QUOTEDDOC;
      fprintf (stderr, "LEX_QUOTEDDOC%s", f ? "|" : "");
    }
  if (f & LEX_INWORD)
    {
      f &= ~LEX_INWORD;
      fprintf (stderr, "LEX_INWORD%s", f ? "|" : "");
    }

  fprintf (stderr, "\n");
  fflush (stderr);
}
#endif

/* Parse a $(...) command substitution.  This reads input from the current
   input stream. */
static char *
parse_comsub (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int peekc, r;
  int start_lineno, local_extglob, was_extpat;
  char *ret, *tcmd;
  int retlen;
  sh_parser_state_t ps;
  STRING_SAVER *saved_strings;
  COMMAND *saved_global, *parsed_command;

  /* Posix interp 217 says arithmetic expressions have precedence, so
     assume $(( introduces arithmetic expansion and parse accordingly. */
  if (open == '(')		/*)*/
    {
      peekc = shell_getc (1);
      shell_ungetc (peekc);
      if (peekc == '(')		/*)*/
	return (parse_matched_pair (qc, open, close, lenp, P_ARITH));
    }

/*itrace("parse_comsub: qc = `%c' open = %c close = %c", qc, open, close);*/

  /*debug_parser(1);*/
  start_lineno = line_number;

  save_parser_state (&ps);

  was_extpat = (parser_state & PST_EXTPAT);

  /* State flags we don't want to persist into command substitutions. */
  parser_state &= ~(PST_REGEXP|PST_EXTPAT|PST_CONDCMD|PST_CONDEXPR|PST_COMPASSIGN);
  /* Could do PST_CASESTMT too, but that also affects history. Setting
     expecting_in_token below should take care of the parsing requirements.
     Unsetting PST_REDIRLIST isn't strictly necessary because of how we set
     token_to_read below, but we do it anyway. */
  parser_state &= ~(PST_CASEPAT|PST_ALEXPNEXT|PST_SUBSHELL|PST_REDIRLIST);
  /* State flags we want to set for this run through the parser. */
  parser_state |= PST_CMDSUBST|PST_EOFTOKEN|PST_NOEXPAND;

  /* leave pushed_string_list alone, since we might need to consume characters
     from it to satisfy this command substitution (in some perverse case). */
  shell_eof_token = close;

  saved_global = global_command;		/* might not be necessary */
  global_command = (COMMAND *)NULL;

  /* These are reset by reset_parser() */
  need_here_doc = 0;
  esacs_needed_count = expecting_in_token = 0;

  /* We want to expand aliases on this pass if we're in posix mode, since the
     standard says you have to take aliases into account when looking for the
     terminating right paren. Otherwise, we defer until execution time for
     backwards compatibility. */
  if (expand_aliases)
    expand_aliases = posixly_correct != 0;
#if defined (EXTENDED_GLOB)
  /* If (parser_state & PST_EXTPAT), we're parsing an extended pattern for a
     conditional command and have already set extended_glob appropriately. */
  if (shell_compatibility_level <= 51 && was_extpat == 0)
    {
      local_extglob = extended_glob;
      extended_glob = 1;
    }
#endif

  current_token = '\n';				/* XXX */
  token_to_read = DOLPAREN;			/* let's trick the parser */

  r = yyparse ();

  if (need_here_doc > 0)
    {
      internal_warning ("command substitution: %d unterminated here-document%s", need_here_doc, (need_here_doc == 1) ? "" : "s");
      gather_here_documents ();	/* XXX check compatibility level? */
    }

#if defined (EXTENDED_GLOB)
  if (shell_compatibility_level <= 51 && was_extpat == 0)
    extended_glob = local_extglob;
#endif

  parsed_command = global_command;

  if (EOF_Reached)
    {
      shell_eof_token = ps.eof_token;
      expand_aliases = ps.expand_aliases;

      /* yyparse() has already called yyerror() and reset_parser() */
      parser_state |= PST_NOERROR;
      return (&matched_pair_error);
    }
  else if (r != 0)
    {
      /* parser_error (start_lineno, _("could not parse command substitution")); */
      /* Non-interactive shells exit on parse error in a command substitution. */
      if (last_command_exit_value == 0)
	last_command_exit_value = EXECUTION_FAILURE;
      set_exit_status (last_command_exit_value);
      if (interactive_shell == 0)
	jump_to_top_level (FORCE_EOF);	/* This is like reader_loop() */
      else
	{
	  shell_eof_token = ps.eof_token;
	  expand_aliases = ps.expand_aliases;

	  jump_to_top_level (DISCARD);	/* XXX - return (&matched_pair_error)? */
	}
    }

  if (current_token != shell_eof_token)
    {
INTERNAL_DEBUG(("current_token (%d) != shell_eof_token (%c)", current_token, shell_eof_token));
      token_to_read = current_token;

      /* If we get here we can check eof_encountered and if it's 1 but the
	 previous EOF_Reached test didn't succeed, we can assume that the shell
	 is interactive and ignoreeof is set. We might want to restore the
	 parser state in this case. */
      shell_eof_token = ps.eof_token;
      expand_aliases = ps.expand_aliases;

      return (&matched_pair_error);
    }

  /* We don't want to restore the old pushed string list, since we might have
     used it to consume additional input from an alias while parsing this
     command substitution. */
  saved_strings = pushed_string_list;
  restore_parser_state (&ps);
  pushed_string_list = saved_strings;

  tcmd = print_comsub (parsed_command);		/* returns static memory */
  retlen = strlen (tcmd);
  if (tcmd[0] == '(')			/* ) need a space to prevent arithmetic expansion */
    retlen++;
  ret = xmalloc (retlen + 2);
  if (tcmd[0] == '(')			/* ) */
    {
      ret[0] = ' ';
      strcpy (ret + 1, tcmd);
    }
  else
    strcpy (ret, tcmd);
  ret[retlen++] = ')';
  ret[retlen] = '\0';

  dispose_command (parsed_command);
  global_command = saved_global;

  if (lenp)
    *lenp = retlen;

/*itrace("parse_comsub:%d: returning `%s'", line_number, ret);*/
  return ret;
}

/* Recursively call the parser to parse a $(...) command substitution. This is
   called by the word expansion code and so does not have to reset as much
   parser state before calling yyparse(). */
char *
xparse_dolparen (base, string, indp, flags)
     char *base;
     char *string;
     int *indp;
     int flags;
{
  sh_parser_state_t ps;
  sh_input_line_state_t ls;
  int orig_ind, nc, sflags, start_lineno, local_extglob;
  char *ret, *ep, *ostring;

/*debug_parser(1);*/
  orig_ind = *indp;
  ostring = string;
  start_lineno = line_number;

  if (*string == 0)
    {
      if (flags & SX_NOALLOC) 
	return (char *)NULL;

      ret = xmalloc (1);
      ret[0] = '\0';
      return ret;
    }

/*itrace("xparse_dolparen: size = %d shell_input_line = `%s' string=`%s'", shell_input_line_size, shell_input_line, string);*/

  sflags = SEVAL_NONINT|SEVAL_NOHIST|SEVAL_NOFREE;
  if (flags & SX_NOLONGJMP)
    sflags |= SEVAL_NOLONGJMP;

  save_parser_state (&ps);
  save_input_line_state (&ls);

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  pushed_string_list = (STRING_SAVER *)NULL;
#endif
  /*(*/
  parser_state |= PST_CMDSUBST|PST_EOFTOKEN;	/* allow instant ')' */ /*(*/
  shell_eof_token = ')';
  if (flags & SX_COMPLETE)
    parser_state |= PST_NOERROR;

  /* Don't expand aliases on this pass at all. Either parse_comsub() does it
     at parse time, in which case this string already has aliases expanded,
     or command_substitute() does it in the child process executing the
     command substitution and we want to defer it completely until then. The
     old value will be restored by restore_parser_state(). */
  expand_aliases = 0;
#if defined (EXTENDED_GLOB)
  local_extglob = extended_glob;
#endif

  token_to_read = DOLPAREN;			/* let's trick the parser */

  nc = parse_string (string, "command substitution", sflags, (COMMAND **)NULL, &ep);

  /* Should we save and restore the bison/yacc lookahead token (yychar) here?
     Or only if it's not YYEMPTY? */
  if (current_token == shell_eof_token)
    yyclearin;		/* might want to clear lookahead token unconditionally */

  reset_parser ();	/* resets extended_glob too */
  /* reset_parser() clears shell_input_line and associated variables, including
     parser_state, so we want to reset things, then restore what we need. */
  restore_input_line_state (&ls);
  restore_parser_state (&ps);

#if defined (EXTENDED_GLOB)
  extended_glob = local_extglob;
#endif
  token_to_read = 0;

  /* If parse_string returns < 0, we need to jump to top level with the
     negative of the return value. We abandon the rest of this input line
     first */
  if (nc < 0)
    {
      clear_shell_input_line ();	/* XXX */
      if (bash_input.type != st_string)	/* paranoia */
	parser_state &= ~(PST_CMDSUBST|PST_EOFTOKEN);
      if ((flags & SX_NOLONGJMP) == 0)
	jump_to_top_level (-nc);	/* XXX */
    }

  /* Need to find how many characters parse_string() consumed, update
     *indp, if flags != 0, copy the portion of the string parsed into RET
     and return it.  If flags & 1 (SX_NOALLOC) we can return NULL. */

  /*(*/
  if (ep[-1] != ')')
    {
#if 0
      if (ep[-1] != '\n')
	itrace("xparse_dolparen:%d: ep[-1] != RPAREN (%d), ep = `%s'", line_number, ep[-1], ep);
#endif

      while (ep > ostring && ep[-1] == '\n') ep--;
    }

  nc = ep - ostring;
  *indp = ep - base - 1;

  /*((*/
#if 0
  if (base[*indp] != ')')
    itrace("xparse_dolparen:%d: base[%d] != RPAREN (%d), base = `%s'", line_number, *indp, base[*indp], base);
  if (*indp < orig_ind)
    itrace("xparse_dolparen:%d: *indp (%d) < orig_ind (%d), orig_string = `%s'", line_number, *indp, orig_ind, ostring);
#endif

  if (base[*indp] != ')' && (flags & SX_NOLONGJMP) == 0)
    {
      /*(*/
      if ((flags & SX_NOERROR) == 0)
	parser_error (start_lineno, _("unexpected EOF while looking for matching `%c'"), ')');
      jump_to_top_level (DISCARD);
    }

  if (flags & SX_NOALLOC) 
    return (char *)NULL;

  if (nc == 0)
    {
      ret = xmalloc (1);
      ret[0] = '\0';
    }
  else
    ret = substring (ostring, 0, nc - 1);

  return ret;
}

/* Recursively call the parser to parse the string from a $(...) command
   substitution to a COMMAND *. This is called from command_substitute() and
   has the same parser state constraints as xparse_dolparen(). */
COMMAND *
parse_string_to_command (string, flags)
     char *string;
     int flags;
{
  sh_parser_state_t ps;
  sh_input_line_state_t ls;
  int nc, sflags;
  size_t slen;
  char *ret, *ep;
  COMMAND *cmd;

  if (*string == 0)
    return (COMMAND *)NULL;

  ep = string;
  slen = STRLEN (string);

/*itrace("parse_string_to_command: size = %d shell_input_line = `%s' string=`%s'", shell_input_line_size, shell_input_line, string);*/

  sflags = SEVAL_NONINT|SEVAL_NOHIST|SEVAL_NOFREE;
  if (flags & SX_NOLONGJMP)
    sflags |= SEVAL_NOLONGJMP;

  save_parser_state (&ps);
  save_input_line_state (&ls);

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  pushed_string_list = (STRING_SAVER *)NULL;
#endif
  if (flags & SX_COMPLETE)
    parser_state |= PST_NOERROR;

  parser_state |= PST_STRING;
  expand_aliases = 0;

  cmd = 0;
  nc = parse_string (string, "command substitution", sflags, &cmd, &ep);

  reset_parser ();
  /* reset_parser() clears shell_input_line and associated variables, including
     parser_state, so we want to reset things, then restore what we need. */
  restore_input_line_state (&ls);
  restore_parser_state (&ps);

  /* If parse_string returns < 0, we need to jump to top level with the
     negative of the return value. We abandon the rest of this input line
     first */
  if (nc < 0)
    {
      clear_shell_input_line ();	/* XXX */
      if ((flags & SX_NOLONGJMP) == 0)
        jump_to_top_level (-nc);	/* XXX */
    }

  /* Need to check how many characters parse_string() consumed, make sure it's
     the entire string. */
  if (nc < slen)
    {
      dispose_command (cmd);
      return (COMMAND *)NULL;
    }

  return cmd;
}

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
/* Parse a double-paren construct.  It can be either an arithmetic
   command, an arithmetic `for' command, or a nested subshell.  Returns
   the parsed token, -1 on error, or -2 if we didn't do anything and
   should just go on. */
static int
parse_dparen (c)
     int c;
{
  int cmdtyp, sline;
  char *wval;
  WORD_DESC *wd;

#if defined (ARITH_FOR_COMMAND)
  if (last_read_token == FOR)
    {
      if (word_top < MAX_CASE_NEST)
	word_top++;
      arith_for_lineno = word_lineno[word_top] = line_number;
      cmdtyp = parse_arith_cmd (&wval, 0);
      if (cmdtyp == 1)
	{
	  wd = alloc_word_desc ();
	  wd->word = wval;
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  return (ARITH_FOR_EXPRS);
	}
      else
	return -1;		/* ERROR */
    }
#endif

#if defined (DPAREN_ARITHMETIC)
  if (reserved_word_acceptable (last_read_token))
    {
      sline = line_number;

      cmdtyp = parse_arith_cmd (&wval, 0);
      if (cmdtyp == 1)	/* arithmetic command */
	{
	  wd = alloc_word_desc ();
	  wd->word = wval;
	  wd->flags = W_QUOTED|W_NOSPLIT|W_NOGLOB|W_NOTILDE|W_NOPROCSUB;
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  return (ARITH_CMD);
	}
      else if (cmdtyp == 0)	/* nested subshell */
	{
	  push_string (wval, 0, (alias_t *)NULL);
	  pushed_string_list->flags = PSH_DPAREN;
	  if ((parser_state & PST_CASEPAT) == 0)
	    parser_state |= PST_SUBSHELL;
	  return (c);
	}
      else			/* ERROR */
	return -1;
    }
#endif

  return -2;			/* XXX */
}

/* We've seen a `(('.  Look for the matching `))'.  If we get it, return 1.
   If not, assume it's a nested subshell for backwards compatibility and
   return 0.  In any case, put the characters we've consumed into a locally-
   allocated buffer and make *ep point to that buffer.  Return -1 on an
   error, for example EOF. */
static int
parse_arith_cmd (ep, adddq)
     char **ep;
     int adddq;
{
  int exp_lineno, rval, c;
  char *ttok, *tokstr;
  int ttoklen;

  exp_lineno = line_number;
  ttok = parse_matched_pair (0, '(', ')', &ttoklen, P_ARITH);
  rval = 1;
  if (ttok == &matched_pair_error)
    return -1;
  /* Check that the next character is the closing right paren.  If
     not, this is a syntax error. ( */
  c = shell_getc (0);
  if MBTEST(c != ')')
    rval = 0;

  tokstr = (char *)xmalloc (ttoklen + 4);

  /* if ADDDQ != 0 then (( ... )) -> "..." */
  if (rval == 1 && adddq)	/* arith cmd, add double quotes */
    {
      tokstr[0] = '"';
      strncpy (tokstr + 1, ttok, ttoklen - 1);
      tokstr[ttoklen] = '"';
      tokstr[ttoklen+1] = '\0';
    }
  else if (rval == 1)		/* arith cmd, don't add double quotes */
    {
      strncpy (tokstr, ttok, ttoklen - 1);
      tokstr[ttoklen-1] = '\0';
    }
  else				/* nested subshell */
    {
      tokstr[0] = '(';
      strncpy (tokstr + 1, ttok, ttoklen - 1);
      tokstr[ttoklen] = ')';
      tokstr[ttoklen+1] = c;
      tokstr[ttoklen+2] = '\0';
    }

  *ep = tokstr;
  FREE (ttok);
  return rval;
}
#endif /* DPAREN_ARITHMETIC || ARITH_FOR_COMMAND */

#if defined (COND_COMMAND)
static void
cond_error ()
{
  char *etext;

  if (EOF_Reached && cond_token != COND_ERROR)		/* [[ */
    parser_error (cond_lineno, _("unexpected EOF while looking for `]]'"));
  else if (cond_token != COND_ERROR)
    {
      if (etext = error_token_from_token (cond_token))
	{
	  parser_error (cond_lineno, _("syntax error in conditional expression: unexpected token `%s'"), etext);
	  free (etext);
	}
      else
	parser_error (cond_lineno, _("syntax error in conditional expression"));
    }
}

static COND_COM *
cond_expr ()
{
  return (cond_or ());  
}

static COND_COM *
cond_or ()
{
  COND_COM *l, *r;

  l = cond_and ();
  if (cond_token == OR_OR)
    {
      r = cond_or ();
      l = make_cond_node (COND_OR, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static COND_COM *
cond_and ()
{
  COND_COM *l, *r;

  l = cond_term ();
  if (cond_token == AND_AND)
    {
      r = cond_and ();
      l = make_cond_node (COND_AND, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static int
cond_skip_newlines ()
{
  while ((cond_token = read_token (READ)) == '\n')
    {
      if (SHOULD_PROMPT ())
	prompt_again (0);
    }
  return (cond_token);
}

#define COND_RETURN_ERROR() \
  do { cond_token = COND_ERROR; return ((COND_COM *)NULL); } while (0)

static COND_COM *
cond_term ()
{
  WORD_DESC *op;
  COND_COM *term, *tleft, *tright;
  int tok, lineno, local_extglob;
  char *etext;

  /* Read a token.  It can be a left paren, a `!', a unary operator, or a
     word that should be the first argument of a binary operator.  Start by
     skipping newlines, since this is a compound command. */
  tok = cond_skip_newlines ();
  lineno = line_number;
  if (tok == COND_END)
    {
      COND_RETURN_ERROR ();
    }
  else if (tok == '(')
    {
      term = cond_expr ();
      if (cond_token != ')')
	{
	  if (term)
	    dispose_cond_node (term);		/* ( */
	  if (etext = error_token_from_token (cond_token))
	    {
	      parser_error (lineno, _("unexpected token `%s', expected `)'"), etext);
	      free (etext);
	    }
	  else
	    parser_error (lineno, _("expected `)'"));
	  COND_RETURN_ERROR ();
	}
      term = make_cond_node (COND_EXPR, (WORD_DESC *)NULL, term, (COND_COM *)NULL);
      (void)cond_skip_newlines ();
    }
  else if (tok == BANG || (tok == WORD && (yylval.word->word[0] == '!' && yylval.word->word[1] == '\0')))
    {
      if (tok == WORD)
	dispose_word (yylval.word);	/* not needed */
      term = cond_term ();
      if (term)
	term->flags ^= CMD_INVERT_RETURN;
    }
  else if (tok == WORD && yylval.word->word[0] == '-' && yylval.word->word[1] && yylval.word->word[2] == 0 && test_unop (yylval.word->word))
    {
      op = yylval.word;
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	}
      else
	{
	  dispose_word (op);
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected argument `%s' to conditional unary operator"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("unexpected argument to conditional unary operator"));
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else if (tok == WORD)		/* left argument to binary operator */
    {
      /* lhs */
      tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);

      /* binop */
      /* tok = cond_skip_newlines (); ? */
      tok = read_token (READ);
      if (tok == WORD && test_binop (yylval.word->word))
	{
	  op = yylval.word;
	  if (op->word[0] == '=' && (op->word[1] == '\0' || (op->word[1] == '=' && op->word[2] == '\0')))
	    parser_state |= PST_EXTPAT;
	  else if (op->word[0] == '!' && op->word[1] == '=' && op->word[2] == '\0')
	    parser_state |= PST_EXTPAT;
	}
#if defined (COND_REGEXP)
      else if (tok == WORD && STREQ (yylval.word->word, "=~"))
	{
	  op = yylval.word;
	  parser_state |= PST_REGEXP;
	}
#endif
      else if (tok == '<' || tok == '>')
	op = make_word_from_token (tok);  /* ( */
      /* There should be a check before blindly accepting the `)' that we have
	 seen the opening `('. */
      else if (tok == COND_END || tok == AND_AND || tok == OR_OR || tok == ')')
	{
	  /* Special case.  [[ x ]] is equivalent to [[ -n x ]], just like
	     the test command.  Similarly for [[ x && expr ]] or
	     [[ x || expr ]] or [[ (x) ]]. */
	  op = make_word ("-n");
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	  cond_token = tok;
	  return (term);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected token `%s', conditional binary operator expected"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("conditional binary operator expected"));
	  dispose_cond_node (tleft);
	  COND_RETURN_ERROR ();
	}

      /* rhs */
#if defined (EXTENDED_GLOB)
      local_extglob = extended_glob;
      if (parser_state & PST_EXTPAT)
	extended_glob = 1;
#endif
      tok = read_token (READ);
#if defined (EXTENDED_GLOB)
      if (parser_state & PST_EXTPAT)
	extended_glob = local_extglob;
#endif
      parser_state &= ~(PST_REGEXP|PST_EXTPAT);

      if (tok == WORD)
	{
	  tright = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_BINARY, op, tleft, tright);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected argument `%s' to conditional binary operator"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("unexpected argument to conditional binary operator"));
	  dispose_cond_node (tleft);
	  dispose_word (op);
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else
    {
      if (tok < 256)
	parser_error (line_number, _("unexpected token `%c' in conditional command"), tok);
      else if (etext = error_token_from_token (tok))
	{
	  parser_error (line_number, _("unexpected token `%s' in conditional command"), etext);
	  free (etext);
	}
      else
	parser_error (line_number, _("unexpected token %d in conditional command"), tok);
      COND_RETURN_ERROR ();
    }
  return (term);
}      

/* This is kind of bogus -- we slip a mini recursive-descent parser in
   here to handle the conditional statement syntax. */
static COMMAND *
parse_cond_command ()
{
  COND_COM *cexp;

  cexp = cond_expr ();
  return (make_cond_command (cexp));
}
#endif

#if defined (ARRAY_VARS)
/* When this is called, it's guaranteed that we don't care about anything
   in t beyond i.  We use a buffer with room for the characters we add just
   in case assignment() ends up doing something like parsing a command
   substitution that will reallocate atoken.  We don't want to write beyond
   the end of an allocated buffer. */
static int
token_is_assignment (t, i)
     char *t;
     int i;
{
  int r;
  char *atoken;

  atoken = xmalloc (i + 3);
  memcpy (atoken, t, i);
  atoken[i] = '=';
  atoken[i+1] = '\0';

  r = assignment (atoken, (parser_state & PST_COMPASSIGN) != 0);

  free (atoken);

  /* XXX - check that r == i to avoid returning false positive for
     t containing `=' before t[i]. */
  return (r > 0 && r == i);
}

/* XXX - possible changes here for `+=' */
static int
token_is_ident (t, i)
     char *t;
     int i;
{
  unsigned char c;
  int r;

  c = t[i];
  t[i] = '\0';
  r = legal_identifier (t);
  t[i] = c;
  return r;
}
#endif

static int
read_token_word (character)
     int character;
{
  /* The value for YYLVAL when a WORD is read. */
  WORD_DESC *the_word;

  /* Index into the token that we are building. */
  int token_index;

  /* ALL_DIGITS becomes zero when we see a non-digit. */
  int all_digit_token;

  /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
  int dollar_present;

  /* COMPOUND_ASSIGNMENT becomes non-zero if we are parsing a compound
     assignment. */
  int compound_assignment;

  /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
  int quoted;

  /* Non-zero means to ignore the value of the next character, and just
     to add it no matter what. */
  int pass_next_character;

  /* The current delimiting character. */
  int cd;
  int result, peek_char;
  char *ttok, *ttrans;
  int ttoklen, ttranslen;
  intmax_t lvalue;

  if (token_buffer_size < TOKEN_DEFAULT_INITIAL_SIZE)
    token = (char *)xrealloc (token, token_buffer_size = TOKEN_DEFAULT_INITIAL_SIZE);

  token_index = 0;
  all_digit_token = DIGIT (character);
  dollar_present = quoted = pass_next_character = compound_assignment = 0;

  for (;;)
    {
      if (character == EOF)
	goto got_token;

      if (pass_next_character)
	{
	  pass_next_character = 0;
	  goto got_escaped_character;
	}

      cd = current_delimiter (dstack);

      /* Handle backslashes.  Quote lots of things when not inside of
	 double-quotes, quote some things inside of double-quotes. */
      if MBTEST(character == '\\')
	{
	  if (parser_state & PST_NOEXPAND)
	    {
	      pass_next_character++;
	      quoted = 1;
	      goto got_character;
	    }
	      
	  peek_char = shell_getc (0);

	  /* Backslash-newline is ignored in all cases except
	     when quoted with single quotes. */
	  if MBTEST(peek_char == '\n')
	    {
	      character = '\n';
	      goto next_character;
	    }
	  else
	    {
	      shell_ungetc (peek_char);

	      /* If the next character is to be quoted, note it now. */
	      if MBTEST(cd == 0 || cd == '`' ||
		  (cd == '"' && peek_char >= 0 && (sh_syntaxtab[peek_char] & CBSDQUOTE)))
		pass_next_character++;

	      quoted = 1;
	      goto got_character;
	    }
	}

      /* Parse a matched pair of quote characters. */
      if MBTEST(shellquote (character))
	{
	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (character, character, character, &ttoklen, (character == '`') ? P_COMMAND : 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  all_digit_token = 0;
	  if (character != '`')
	    quoted = 1;
	  dollar_present |= (character == '"' && strchr (ttok, '$') != 0);
	  FREE (ttok);
	  goto next_character;
	}

#ifdef COND_REGEXP
      /* When parsing a regexp as a single word inside a conditional command,
	 we need to special-case characters special to both the shell and
	 regular expressions.  Right now, that is only '(' and '|'. */ /*)*/
      if MBTEST((parser_state & PST_REGEXP) && (character == '(' || character == '|'))		/*)*/
	{
	  if (character == '|')
	    goto got_character;

	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  FREE (ttok);
	  dollar_present = all_digit_token = 0;
	  goto next_character;
	}
#endif /* COND_REGEXP */

#ifdef EXTENDED_GLOB
      /* Parse a ksh-style extended pattern matching specification. */
      if MBTEST(extended_glob && PATTERN_CHAR (character))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = all_digit_token = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif /* EXTENDED_GLOB */

      /* If the delimiter character is not single quote, parse some of
	 the shell expansions that must be read as a single word. */
      if MBTEST(shellexp (character))
	{
	  peek_char = shell_getc (1);
	  /* $(...), <(...), >(...), $((...)), ${...}, and $[...] constructs */
	  if MBTEST(peek_char == '(' ||
		((peek_char == '{' || peek_char == '[') && character == '$'))	/* ) ] } */
	    {
	      if (peek_char == '{')		/* } */
		ttok = parse_matched_pair (cd, '{', '}', &ttoklen, P_FIRSTCLOSE|P_DOLBRACE);
	      else if (peek_char == '(')		/* ) */
		{
		  /* XXX - push and pop the `(' as a delimiter for use by
		     the command-oriented-history code.  This way newlines
		     appearing in the $(...) string get added to the
		     history literally rather than causing a possibly-
		     incorrect `;' to be added. ) */
		  push_delimiter (dstack, peek_char);
		  ttok = parse_comsub (cd, '(', ')', &ttoklen, P_COMMAND);
		  pop_delimiter (dstack);
		}
	      else
		ttok = parse_matched_pair (cd, '[', ']', &ttoklen, P_ARITH);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
#if defined (TRANSLATABLE_STRINGS)
	  else if MBTEST(character == '$' && (peek_char == '\'' || peek_char == '"'))
#else
	  else if MBTEST(character == '$' && peek_char == '\'')
#endif
	    {
	      int first_line;

	      first_line = line_number;
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (peek_char, peek_char, peek_char,
					 &ttoklen,
					 (peek_char == '\'') ? P_ALLOWESC : 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;
	      if (peek_char == '\'')
		{
		  /* PST_NOEXPAND */
		  ttrans = ansiexpand (ttok, 0, ttoklen - 1, &ttranslen);
		  free (ttok);

		  /* Insert the single quotes and correctly quote any
		     embedded single quotes (allowed because P_ALLOWESC was
		     passed to parse_matched_pair). */
		  ttok = sh_single_quote (ttrans);
		  free (ttrans);
		  ttranslen = strlen (ttok);
		  ttrans = ttok;
		}
#if defined (TRANSLATABLE_STRINGS)
	      else
		{
		  /* PST_NOEXPAND */
		  /* Try to locale-expand the converted string. */
		  ttrans = locale_expand (ttok, 0, ttoklen - 1, first_line, &ttranslen);
		  free (ttok);

		  /* Add the double quotes back (or single quotes if the user
		     has set that option). */
		  if (singlequote_translations &&
		        ((ttoklen - 1) != ttranslen || STREQN (ttok, ttrans, ttranslen) == 0))
		    ttok = sh_single_quote (ttrans);
		  else
		    ttok = sh_mkdoublequoted (ttrans, ttranslen, 0);

		  free (ttrans);
		  ttrans = ttok;
		  ttranslen = strlen (ttrans);
		}
#endif /* TRANSLATABLE_STRINGS */

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttranslen + 1,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttrans);
	      token_index += ttranslen;
	      FREE (ttrans);
	      quoted = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This could eventually be extended to recognize all of the
	     shell's single-character parameter expansions, and set flags.*/
	  else if MBTEST(character == '$' && peek_char == '$')
	    {
	      RESIZE_MALLOCED_BUFFER (token, token_index, 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = '$';
	      token[token_index++] = peek_char;
	      dollar_present = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible array subscript assignment; match [...].  If
	 parser_state&PST_COMPASSIGN, we need to parse [sub]=words treating
	 `sub' as if it were enclosed in double quotes. */
      else if MBTEST(character == '[' &&		/* ] */
		     ((token_index > 0 && assignment_acceptable (last_read_token) && token_is_ident (token, token_index)) ||
		      (token_index == 0 && (parser_state&PST_COMPASSIGN))))
        {
	  ttok = parse_matched_pair (cd, '[', ']', &ttoklen, P_ARRAYSUB);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size,
				  TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  FREE (ttok);
	  all_digit_token = 0;
	  goto next_character;
        }
      /* Identify possible compound array variable assignment. */
      else if MBTEST(character == '=' && token_index > 0 && (assignment_acceptable (last_read_token) || (parser_state & PST_ASSIGNOK)) && token_is_assignment (token, token_index))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      ttok = parse_compound_assignment (&ttoklen);

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 4,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);

	      token[token_index++] = '=';
	      token[token_index++] = '(';
	      if (ttok)
		{
		  strcpy (token + token_index, ttok);
		  token_index += ttoklen;
		}
	      token[token_index++] = ')';
	      FREE (ttok);
	      all_digit_token = 0;
	      compound_assignment = 1;
#if 1
	      goto next_character;
#else
	      goto got_token;		/* ksh93 seems to do this */
#endif
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif

      /* When not parsing a multi-character word construct, shell meta-
	 characters break words. */
      if MBTEST(shellbreak (character))
	{
	  shell_ungetc (character);
	  goto got_token;
	}

got_character:
      if MBTEST(character == CTLESC || character == CTLNUL)
	{
	  RESIZE_MALLOCED_BUFFER (token, token_index, 2, token_buffer_size,
				  TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = CTLESC;
	}
      else
got_escaped_character:
	RESIZE_MALLOCED_BUFFER (token, token_index, 1, token_buffer_size,
				TOKEN_DEFAULT_GROW_SIZE);

      token[token_index++] = character;

      all_digit_token &= DIGIT (character);
      dollar_present |= character == '$';

    next_character:
      if (character == '\n' && SHOULD_PROMPT ())
	prompt_again (0);

      /* We want to remove quoted newlines (that is, a \<newline> pair)
	 unless we are within single quotes or pass_next_character is
	 set (the shell equivalent of literal-next). */
      cd = current_delimiter (dstack);
      character = shell_getc (cd != '\'' && pass_next_character == 0);
    }	/* end for (;;) */

got_token:

  /* Calls to RESIZE_MALLOCED_BUFFER ensure there is sufficient room. */
  token[token_index] = '\0';

  /* Check to see what thing we should return.  If the last_read_token
     is a `<', or a `&', or the character which ended this token is
     a '>' or '<', then, and ONLY then, is this input token a NUMBER.
     Otherwise, it is just a word, and should be returned as such. */
  if MBTEST(all_digit_token && (character == '<' || character == '>' ||
		    last_read_token == LESS_AND ||
		    last_read_token == GREATER_AND))
      {
	if (legal_number (token, &lvalue) && (int)lvalue == lvalue)
	  {
	    yylval.number = lvalue;
	    return (NUMBER);
	  }
      }

  /* Check for special case tokens. */
  result = (last_shell_getc_is_singlebyte) ? special_case_tokens (token) : -1;
  if (result >= 0)
    return result;

#if defined (ALIAS)
  /* Posix.2 does not allow reserved words to be aliased, so check for all
     of them, including special cases, before expanding the current token
     as an alias. */
  if MBTEST(posixly_correct)
    CHECK_FOR_RESERVED_WORD (token);

  /* Aliases are expanded iff EXPAND_ALIASES is non-zero, and quoting
     inhibits alias expansion. */
  if (expand_aliases && quoted == 0)
    {
      result = alias_expand_token (token);
      if (result == RE_READ_TOKEN)
	return (RE_READ_TOKEN);
      else if (result == NO_EXPANSION)
	parser_state &= ~PST_ALEXPNEXT;
    }

  /* If not in Posix.2 mode, check for reserved words after alias
     expansion. */
  if MBTEST(posixly_correct == 0)
#endif
    CHECK_FOR_RESERVED_WORD (token);

  the_word = alloc_word_desc ();
  the_word->word = (char *)xmalloc (1 + token_index);
  the_word->flags = 0;
  strcpy (the_word->word, token);
  if (dollar_present)
    the_word->flags |= W_HASDOLLAR;
  if (quoted)
    the_word->flags |= W_QUOTED;		/*(*/
  if (compound_assignment && token[token_index-1] == ')')
    the_word->flags |= W_COMPASSIGN;
  /* A word is an assignment if it appears at the beginning of a
     simple command, or after another assignment word.  This is
     context-dependent, so it cannot be handled in the grammar. */
  if (assignment (token, (parser_state & PST_COMPASSIGN) != 0))
    {
      the_word->flags |= W_ASSIGNMENT;
      /* Don't perform word splitting on assignment statements. */
      if (assignment_acceptable (last_read_token) || (parser_state & PST_COMPASSIGN) != 0)
	{
	  the_word->flags |= W_NOSPLIT;
	  if (parser_state & PST_COMPASSIGN)
	    the_word->flags |= W_NOGLOB;	/* XXX - W_NOBRACE? */
	}
    }

  if (command_token_position (last_read_token))
    {
      struct builtin *b;
      b = builtin_address_internal (token, 0);
      if (b && (b->flags & ASSIGNMENT_BUILTIN))
	parser_state |= PST_ASSIGNOK;
      else if (STREQ (token, "eval") || STREQ (token, "let"))
	parser_state |= PST_ASSIGNOK;
    }

  yylval.word = the_word;

  /* should we check that quoted == 0 as well? */
  if MBTEST(token[0] == '{' && token[token_index-1] == '}' &&
      (character == '<' || character == '>'))
    {
      /* can use token; already copied to the_word */
      token[token_index-1] = '\0';
#if defined (ARRAY_VARS)
      if (legal_identifier (token+1) || valid_array_reference (token+1, 0))
#else
      if (legal_identifier (token+1))
#endif
	{
	  strcpy (the_word->word, token+1);
/* itrace("read_token_word: returning REDIR_WORD for %s", the_word->word); */
	  yylval.word = the_word;	/* accommodate recursive call */
	  return (REDIR_WORD);
	}
      else
        /* valid_array_reference can call the parser recursively; need to
	   make sure that yylval.word doesn't change if we are going to
	   return WORD or ASSIGNMENT_WORD */
        yylval.word = the_word;
    }

  result = ((the_word->flags & (W_ASSIGNMENT|W_NOSPLIT)) == (W_ASSIGNMENT|W_NOSPLIT))
		? ASSIGNMENT_WORD : WORD;

  switch (last_read_token)
    {
    case FUNCTION:
      parser_state |= PST_ALLOWOPNBRC;
      function_dstart = line_number;
      break;
    case CASE:
    case SELECT:
    case FOR:
      if (word_top < MAX_CASE_NEST)
	word_top++;
      word_lineno[word_top] = line_number;
      expecting_in_token++;
      break;
    }

  return (result);
}

/* Return 1 if TOKSYM is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (toksym)
     int toksym;
{
  switch (toksym)
    {
    case '\n':
    case ';':
    case '(':
    case ')':
    case '|':
    case '&':
    case '{':
    case '}':		/* XXX */
    case AND_AND:
    case ARITH_CMD:
    case BANG:
    case BAR_AND:
    case COND_END:
    case DO:
    case DONE:
    case ELIF:
    case ELSE:
    case ESAC:
    case FI:
    case IF:
    case OR_OR:
    case SEMI_SEMI:
    case SEMI_AND:
    case SEMI_SEMI_AND:
    case THEN:
    case TIME:
    case TIMEOPT:
    case TIMEIGN:
    case COPROC:
    case UNTIL:
    case WHILE:
    case 0:
    case DOLPAREN:
      return 1;
    default:
#if defined (COPROCESS_SUPPORT)
      if (last_read_token == WORD && token_before_that == COPROC)
	return 1;
#endif
      if (last_read_token == WORD && token_before_that == FUNCTION)
	return 1;
      return 0;
    }
}
    
/* Return the index of TOKEN in the alist of reserved words, or -1 if
   TOKEN is not a shell reserved word. */
int
find_reserved_word (tokstr)
     char *tokstr;
{
  int i;
  for (i = 0; word_token_alist[i].word; i++)
    if (STREQ (tokstr, word_token_alist[i].word))
      return i;
  return -1;
}

/* An interface to let the rest of the shell (primarily the completion
   system) know what the parser is expecting. */
int
parser_in_command_position ()
{
  return (command_token_position (last_read_token));
}

#if 0
#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
static void
reset_readline_prompt ()
{
  char *temp_prompt;

  if (prompt_string_pointer)
    {
      temp_prompt = (*prompt_string_pointer)
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

      if (temp_prompt == 0)
	{
	  temp_prompt = (char *)xmalloc (1);
	  temp_prompt[0] = '\0';
	}

      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
}
#endif /* READLINE */
#endif /* 0 */

#if defined (HISTORY)
/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
static const int no_semi_successors[] = {
  '\n', '{', '(', ')', ';', '&', '|',
  CASE, DO, ELSE, IF, SEMI_SEMI, SEMI_AND, SEMI_SEMI_AND, THEN, UNTIL,
  WHILE, AND_AND, OR_OR, IN,
  0
};

/* If we are not within a delimited expression, try to be smart
   about which separators can be semi-colons and which must be
   newlines.  Returns the string that should be added into the
   history entry.  LINE is the line we're about to add; it helps
   make some more intelligent decisions in certain cases. */
char *
history_delimiting_chars (line)
     const char *line;
{
  static int last_was_heredoc = 0;	/* was the last entry the start of a here document? */
  register int i;

  if ((parser_state & PST_HEREDOC) == 0)
    last_was_heredoc = 0;

  if (dstack.delimiter_depth != 0)
    return ("\n");

  /* We look for current_command_line_count == 2 because we are looking to
     add the first line of the body of the here document (the second line
     of the command).  We also keep LAST_WAS_HEREDOC as a private sentinel
     variable to note when we think we added the first line of a here doc
     (the one with a "<<" somewhere in it) */
  if (parser_state & PST_HEREDOC)
    {
      if (last_was_heredoc)
	{
	  last_was_heredoc = 0;
	  return "\n";
	}
      return (here_doc_first_line ? "\n" : "");
    }

  if (parser_state & PST_COMPASSIGN)
    return (" ");

  /* First, handle some special cases. */
  /*(*/
  /* If we just read `()', assume it's a function definition, and don't
     add a semicolon.  If the token before the `)' was not `(', and we're
     not in the midst of parsing a case statement, assume it's a
     parenthesized command and add the semicolon. */
  /*)(*/
  if (token_before_that == ')')
    {
      if (two_tokens_ago == '(')	/*)*/	/* function def */
	return " ";
      /* This does not work for subshells inside case statement
	 command lists.  It's a suboptimal solution. */
      else if (parser_state & PST_CASESTMT)	/* case statement pattern */
	return " ";
      else
	return "; ";				/* (...) subshell */
    }
  else if (token_before_that == WORD && two_tokens_ago == FUNCTION)
    return " ";		/* function def using `function name' without `()' */

  /* If we're not in a here document, but we think we're about to parse one,
     and we would otherwise return a `;', return a newline to delimit the
     line with the here-doc delimiter */
  else if ((parser_state & PST_HEREDOC) == 0 && current_command_line_count > 1 && last_read_token == '\n' && strstr (line, "<<"))
    {
      last_was_heredoc = 1;
      return "\n";
    }
  else if ((parser_state & PST_HEREDOC) == 0 && current_command_line_count > 1 && need_here_doc > 0)
    return "\n";
  else if (token_before_that == WORD && two_tokens_ago == FOR)
    {
      /* Tricky.  `for i\nin ...' should not have a semicolon, but
	 `for i\ndo ...' should.  We do what we can. */
      for (i = shell_input_line_index; whitespace (shell_input_line[i]); i++)
	;
      if (shell_input_line[i] && shell_input_line[i] == 'i' && shell_input_line[i+1] == 'n')
	return " ";
      return ";";
    }
  else if (two_tokens_ago == CASE && token_before_that == WORD && (parser_state & PST_CASESTMT))
    return " ";

  for (i = 0; no_semi_successors[i]; i++)
    {
      if (token_before_that == no_semi_successors[i])
	return (" ");
    }

  /* Assume that by this point we are reading lines in a multi-line command.
     If we have multiple consecutive blank lines we want to return only one
     semicolon. */
  if (line_isblank (line))
    return (current_command_line_count > 1 && last_read_token == '\n' && token_before_that != '\n') ? "; " : "";

  return ("; ");
}
#endif /* HISTORY */

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
static void
prompt_again (force)
     int force;
{
  char *temp_prompt;

  if (interactive == 0 || expanding_alias ())	/* XXX */
    return;

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  ps0_prompt = get_string_value ("PS0");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  temp_prompt = *prompt_string_pointer
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

  if (temp_prompt == 0)
    {
      temp_prompt = (char *)xmalloc (1);
      temp_prompt[0] = '\0';
    }

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

#if defined (READLINE)
  if (!no_line_editing)
    {
      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
  else
#endif	/* READLINE */
    {
      FREE (current_decoded_prompt);
      current_decoded_prompt = temp_prompt;
    }
}

int
get_current_prompt_level ()
{
  return ((current_prompt_string && current_prompt_string == ps2_prompt) ? 2 : 1);
}

void
set_current_prompt_level (x)
     int x;
{
  prompt_string_pointer = (x == 2) ? &ps2_prompt : &ps1_prompt;
  current_prompt_string = *prompt_string_pointer;
}
      
static void
print_prompt ()
{
  fprintf (stderr, "%s", current_decoded_prompt);
  fflush (stderr);
}

#if defined (HISTORY)
  /* The history library increments the history offset as soon as it stores
     the first line of a potentially multi-line command, so we compensate
     here by returning one fewer when appropriate. */
static int
prompt_history_number (pmt)
     char *pmt;
{
  int ret;

  ret = history_number ();
  if (ret == 1)
    return ret;

  if (pmt == ps1_prompt)	/* are we expanding $PS1? */
    return ret;
  else if (pmt == ps2_prompt && command_oriented_history == 0)
    return ret;			/* not command oriented history */
  else if (pmt == ps2_prompt && command_oriented_history && current_command_first_line_saved)
    return ret - 1;
  else
    return ret - 1;		/* PS0, PS4, ${var@P}, PS2 other cases */
}
#endif

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:

	\a	bell (ascii 07)
	\d	the date in Day Mon Date format
	\e	escape (ascii 033)
	\h	the hostname up to the first `.'
	\H	the hostname
	\j	the number of active jobs
	\l	the basename of the shell's tty device name
	\n	CRLF
	\r	CR
	\s	the name of the shell
	\t	the time in 24-hour hh:mm:ss format
	\T	the time in 12-hour hh:mm:ss format
	\@	the time in 12-hour hh:mm am/pm format
	\A	the time in 24-hour hh:mm format
	\D{fmt}	the result of passing FMT to strftime(3)
	\u	your username
	\v	the version of bash (e.g., 2.00)
	\V	the release of bash, version + patchlevel (e.g., 2.00.0)
	\w	the current working directory
	\W	the last element of $PWD
	\!	the history number of this command
	\#	the command number of this command
	\$	a $ or a # if you are root
	\nnn	character code nnn in octal
	\\	a backslash
	\[	begin a sequence of non-printing chars
	\]	end a sequence of non-printing chars
*/
#define PROMPT_GROWTH 48
char *
decode_prompt_string (string)
     char *string;
{
  WORD_LIST *list;
  char *result, *t, *orig_string;
  struct dstack save_dstack;
  int last_exit_value, last_comsub_pid;
#if defined (PROMPT_STRING_DECODE)
  size_t result_size;
  size_t result_index;
  int c, n, i;
  char *temp, *t_host, octal_string[4];
  struct tm *tm;  
  time_t the_time;
  char timebuf[128];
  char *timefmt;

  result = (char *)xmalloc (result_size = PROMPT_GROWTH);
  result[result_index = 0] = 0;
  temp = (char *)NULL;
  orig_string = string;

  while (c = *string++)
    {
      if (posixly_correct && c == '!')
	{
	  if (*string == '!')
	    {
	      temp = savestring ("!");
	      goto add_string;
	    }
	  else
	    {
#if !defined (HISTORY)
		temp = savestring ("1");
#else /* HISTORY */
		temp = itos (prompt_history_number (orig_string));
#endif /* HISTORY */
		string--;	/* add_string increments string again. */
		goto add_string;
	    }
	}
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      strncpy (octal_string, string, 3);
	      octal_string[3] = '\0';

	      n = read_octal (octal_string);
	      temp = (char *)xmalloc (3);

	      if (n == CTLESC || n == CTLNUL)
		{
		  temp[0] = CTLESC;
		  temp[1] = n;
		  temp[2] = '\0';
		}
	      else if (n == -1)
		{
		  temp[0] = '\\';
		  temp[1] = '\0';
		}
	      else
		{
		  temp[0] = n;
		  temp[1] = '\0';
		}

	      for (c = 0; n != -1 && c < 3 && ISOCTAL (*string); c++)
		string++;

	      c = 0;		/* tested at add_string: */
	      goto add_string;

	    case 'd':
	    case 't':
	    case 'T':
	    case '@':
	    case 'A':
	      /* Make the current time/date into a string. */
	      (void) time (&the_time);
#if defined (HAVE_TZSET)
	      sv_tz ("TZ");		/* XXX -- just make sure */
#endif
	      tm = localtime (&the_time);

	      if (c == 'd')
		n = strftime (timebuf, sizeof (timebuf), "%a %b %d", tm);
	      else if (c == 't')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M:%S", tm);
	      else if (c == 'T')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M:%S", tm);
	      else if (c == '@')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M %p", tm);
	      else if (c == 'A')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M", tm);

	      if (n == 0)
		timebuf[0] = '\0';
	      else
		timebuf[sizeof(timebuf) - 1] = '\0';

	      temp = savestring (timebuf);
	      goto add_string;

	    case 'D':		/* strftime format */
	      if (string[1] != '{')		/* } */
		goto not_escape;

	      (void) time (&the_time);
	      tm = localtime (&the_time);
	      string += 2;			/* skip { */
	      timefmt = xmalloc (strlen (string) + 3);
	      for (t = timefmt; *string && *string != '}'; )
		*t++ = *string++;
	      *t = '\0';
	      c = *string;	/* tested at add_string */
	      if (timefmt[0] == '\0')
		{
		  timefmt[0] = '%';
		  timefmt[1] = 'X';	/* locale-specific current time */
		  timefmt[2] = '\0';
		}
	      n = strftime (timebuf, sizeof (timebuf), timefmt, tm);
	      free (timefmt);

	      if (n == 0)
		timebuf[0] = '\0';
	      else
		timebuf[sizeof(timebuf) - 1] = '\0';

	      if (promptvars || posixly_correct)
		/* Make sure that expand_prompt_string is called with a
		   second argument of Q_DOUBLE_QUOTES if we use this
		   function here. */
		temp = sh_backslash_quote_for_double_quotes (timebuf, 0);
	      else
		temp = savestring (timebuf);
	      goto add_string;
	      
	    case 'n':
	      temp = (char *)xmalloc (3);
	      temp[0] = no_line_editing ? '\n' : '\r';
	      temp[1] = no_line_editing ? '\0' : '\n';
	      temp[2] = '\0';
	      goto add_string;

	    case 's':
	      temp = base_pathname (shell_name);
	      /* Try to quote anything the user can set in the file system */
	      if (promptvars || posixly_correct)
		{
		  char *t;
		  t = sh_strvis (temp);
		  temp = sh_backslash_quote_for_double_quotes (t, 0);
		  free (t);
		}
	      else
		temp = sh_strvis (temp);
	      goto add_string;

	    case 'v':
	    case 'V':
	      temp = (char *)xmalloc (16);
	      if (c == 'v')
		strcpy (temp, dist_version);
	      else
		sprintf (temp, "%s.%d", dist_version, patch_level);
	      goto add_string;

	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more efficient. */
		char t_string[PATH_MAX];
		int tlen;

		temp = get_string_value ("PWD");

		if (temp == 0)
		  {
		    if (getcwd (t_string, sizeof(t_string)) == 0)
		      {
			t_string[0] = '.';
			tlen = 1;
		      }
		    else
		      tlen = strlen (t_string);
		  }
		else
		  {
		    tlen = sizeof (t_string) - 1;
		    strncpy (t_string, temp, tlen);
		  }
		t_string[tlen] = '\0';

#if defined (MACOSX)
		/* Convert from "fs" format to "input" format */
		temp = fnx_fromfs (t_string, strlen (t_string));
		if (temp != t_string)
		  strcpy (t_string, temp);
#endif

#define ROOT_PATH(x)	((x)[0] == '/' && (x)[1] == 0)
#define DOUBLE_SLASH_ROOT(x)	((x)[0] == '/' && (x)[1] == '/' && (x)[2] == 0)
		/* Abbreviate \W as ~ if $PWD == $HOME */
		if (c == 'W' && (((t = get_string_value ("HOME")) == 0) || STREQ (t, t_string) == 0))
		  {
		    if (ROOT_PATH (t_string) == 0 && DOUBLE_SLASH_ROOT (t_string) == 0)
		      {
			t = strrchr (t_string, '/');
			if (t)
			  memmove (t_string, t + 1, strlen (t));	/* strlen(t) to copy NULL */
		      }
		  }
#undef ROOT_PATH
#undef DOUBLE_SLASH_ROOT
		else
		  {
		    /* polite_directory_format is guaranteed to return a string
		       no longer than PATH_MAX - 1 characters. */
		    temp = polite_directory_format (t_string);
		    if (temp != t_string)
		      strcpy (t_string, temp);
		  }

		temp = trim_pathname (t_string, PATH_MAX - 1);
		/* If we're going to be expanding the prompt string later,
		   quote the directory name. */
		if (promptvars || posixly_correct)
		  /* Make sure that expand_prompt_string is called with a
		     second argument of Q_DOUBLE_QUOTES if we use this
		     function here. */
		  {
		    char *t;
		    t = sh_strvis (t_string);
		    temp = sh_backslash_quote_for_double_quotes (t, 0);
		    free (t);
		  }
		else
		  temp = sh_strvis (t_string);

		goto add_string;
	      }

	    case 'u':
	      if (current_user.user_name == 0)
		get_current_user_info ();
	      temp = savestring (current_user.user_name);
	      goto add_string;

	    case 'h':
	    case 'H':
	      t_host = savestring (current_host_name);
	      if (c == 'h' && (t = (char *)strchr (t_host, '.')))
		*t = '\0';
	      if (promptvars || posixly_correct)
		/* Make sure that expand_prompt_string is called with a
		   second argument of Q_DOUBLE_QUOTES if we use this
		   function here. */
		temp = sh_backslash_quote_for_double_quotes (t_host, 0);
	      else
		temp = savestring (t_host);
	      free (t_host);
	      goto add_string;

	    case '#':
	      n = current_command_number;
	      /* If we have already incremented current_command_number (PS4,
		 ${var@P}), compensate */
	      if (orig_string != ps0_prompt && orig_string != ps1_prompt && orig_string != ps2_prompt)
		n--;
	      temp = itos (n);
	      goto add_string;

	    case '!':
#if !defined (HISTORY)
	      temp = savestring ("1");
#else /* HISTORY */
	      temp = itos (prompt_history_number (orig_string));
#endif /* HISTORY */
	      goto add_string;

	    case '$':
	      t = temp = (char *)xmalloc (3);
	      if ((promptvars || posixly_correct) && (current_user.euid != 0))
		*t++ = '\\';
	      *t++ = current_user.euid == 0 ? '#' : '$';
	      *t = '\0';
	      goto add_string;

	    case 'j':
	      temp = itos (count_all_jobs ());
	      goto add_string;

	    case 'l':
#if defined (HAVE_TTYNAME)
	      temp = (char *)ttyname (fileno (stdin));
	      t = temp ? base_pathname (temp) : "tty";
	      temp = savestring (t);
#else
	      temp = savestring ("tty");
#endif /* !HAVE_TTYNAME */
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      if (no_line_editing)
		{
		  string++;
		  break;
		}
	      temp = (char *)xmalloc (3);
	      n = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      i = 0;
	      if (n == CTLESC || n == CTLNUL)
		temp[i++] = CTLESC;
	      temp[i++] = n;
	      temp[i] = '\0';
	      goto add_string;
#endif /* READLINE */

	    case '\\':
	    case 'a':
	    case 'e':
	    case 'r':
	      temp = (char *)xmalloc (2);
	      if (c == 'a')
		temp[0] = '\07';
	      else if (c == 'e')
		temp[0] = '\033';
	      else if (c == 'r')
		temp[0] = '\r';
	      else			/* (c == '\\') */
	        temp[0] = c;
	      temp[1] = '\0';
	      goto add_string;

	    default:
not_escape:
	      temp = (char *)xmalloc (3);
	      temp[0] = '\\';
	      temp[1] = c;
	      temp[2] = '\0';

	    add_string:
	      if (c)
		string++;
	      result =
		sub_append_string (temp, result, &result_index, &result_size);
	      temp = (char *)NULL; /* Freed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  RESIZE_MALLOCED_BUFFER (result, result_index, 3, result_size, PROMPT_GROWTH);
	  /* dequote_string should take care of removing this if we are not
	     performing the rest of the word expansions. */
	  if (c == CTLESC || c == CTLNUL)
	    result[result_index++] = CTLESC;
	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }
#else /* !PROMPT_STRING_DECODE */
  result = savestring (string);
#endif /* !PROMPT_STRING_DECODE */

  /* Save the delimiter stack and point `dstack' to temp space so any
     command substitutions in the prompt string won't result in screwing
     up the parser's quoting state. */
  save_dstack = dstack;
  dstack = temp_dstack;
  dstack.delimiter_depth = 0;

  /* Perform variable and parameter expansion and command substitution on
     the prompt string. */
  if (promptvars || posixly_correct)
    {
      last_exit_value = last_command_exit_value;
      last_comsub_pid = last_command_subst_pid;
      list = expand_prompt_string (result, Q_DOUBLE_QUOTES, 0);
      free (result);
      result = string_list (list);
      dispose_words (list);
      last_command_exit_value = last_exit_value;
      last_command_subst_pid = last_comsub_pid;
    }
  else
    {
      t = dequote_string (result);
      free (result);
      result = t;
    }

  dstack = save_dstack;

  return (result);
}

/************************************************
 *						*
 *		ERROR HANDLING			*
 *						*
 ************************************************/

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
int
yyerror (msg)
     const char *msg;
{
  if ((parser_state & PST_NOERROR) == 0)
    report_syntax_error ((char *)NULL);
  reset_parser ();
  return (0);
}

static char *
error_token_from_token (tok)
     int tok;
{
  char *t;

  if (t = find_token_in_alist (tok, word_token_alist, 0))
    return t;

  if (t = find_token_in_alist (tok, other_token_alist, 0))
    return t;

  t = (char *)NULL;
  /* This stuff is dicy and needs closer inspection */
  switch (current_token)
    {
    case WORD:
    case ASSIGNMENT_WORD:
      if (yylval.word)
	t = savestring (yylval.word->word);
      break;
    case NUMBER:
      t = itos (yylval.number);
      break;
    case ARITH_CMD:
      if (yylval.word_list)
        t = string_list (yylval.word_list);
      break;
    case ARITH_FOR_EXPRS:
      if (yylval.word_list)
	t = string_list_internal (yylval.word_list, " ; ");
      break;
    case COND_CMD:
      t = (char *)NULL;		/* punt */
      break;
    }

  return t;
}

static char *
error_token_from_text ()
{
  char *msg, *t;
  int token_end, i;

  t = shell_input_line;
  i = shell_input_line_index;
  token_end = 0;
  msg = (char *)NULL;

  if (i && t[i] == '\0')
    i--;

  while (i && (whitespace (t[i]) || t[i] == '\n'))
    i--;

  if (i)
    token_end = i + 1;

  while (i && (member (t[i], " \n\t;|&") == 0))
    i--;

  while (i != token_end && (whitespace (t[i]) || t[i] == '\n'))
    i++;

  /* Return our idea of the offending token. */
  if (token_end || (i == 0 && token_end == 0))
    {
      if (token_end)
	msg = substring (t, i, token_end);
      else	/* one-character token */
	{
	  msg = (char *)xmalloc (2);
	  msg[0] = t[i];
	  msg[1] = '\0';
	}
    }

  return (msg);
}

static void
print_offending_line ()
{
  char *msg;
  int token_end;

  msg = savestring (shell_input_line);
  token_end = strlen (msg);
  while (token_end && msg[token_end - 1] == '\n')
    msg[--token_end] = '\0';

  parser_error (line_number, "`%s'", msg);
  free (msg);
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  char *msg, *p;

  if (message)
    {
      parser_error (line_number, "%s", message);
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
      last_command_exit_value = (executing_builtin && parse_and_execute_level) ? EX_BADSYNTAX : EX_BADUSAGE;
      set_pipestatus_from_exit (last_command_exit_value);
      return;
    }

  /* If the line of input we're reading is not null, try to find the
     objectionable token.  First, try to figure out what token the
     parser's complaining about by looking at current_token. */
  if (current_token != 0 && EOF_Reached == 0 && (msg = error_token_from_token (current_token)))
    {
      if (ansic_shouldquote (msg))
	{
	  p = ansic_quote (msg, 0, NULL);
	  free (msg);
	  msg = p;
	}
      parser_error (line_number, _("syntax error near unexpected token `%s'"), msg);
      free (msg);

      if (interactive == 0)
	print_offending_line ();

      last_command_exit_value = (executing_builtin && parse_and_execute_level) ? EX_BADSYNTAX : EX_BADUSAGE;
      set_pipestatus_from_exit (last_command_exit_value);
      return;
    }

  /* If looking at the current token doesn't prove fruitful, try to find the
     offending token by analyzing the text of the input line near the current
     input line index and report what we find. */
  if (shell_input_line && *shell_input_line)
    {
      msg = error_token_from_text ();
      if (msg)
	{
	  parser_error (line_number, _("syntax error near `%s'"), msg);
	  free (msg);
	}

      /* If not interactive, print the line containing the error. */
      if (interactive == 0)
        print_offending_line ();
    }
  else
    {
      if (EOF_Reached && shell_eof_token && current_token != shell_eof_token)
	parser_error (line_number, _("unexpected EOF while looking for matching `%c'"), shell_eof_token);
      else
	{
	  msg = EOF_Reached ? _("syntax error: unexpected end of file") : _("syntax error");
	  parser_error (line_number, "%s", msg);
	}

      /* When the shell is interactive, this file uses EOF_Reached
	 only for error reporting.  Other mechanisms are used to
	 decide whether or not to exit. */
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
    }

  last_command_exit_value = (executing_builtin && parse_and_execute_level) ? EX_BADSYNTAX : EX_BADUSAGE;
  set_pipestatus_from_exit (last_command_exit_value);
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command.) */
static void
discard_parser_constructs (error_p)
     int error_p;
{
}

/************************************************
 *						*
 *		EOF HANDLING			*
 *						*
 ************************************************/

/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* A flag denoting whether or not ignoreeof is set. */
int ignoreeof = 0;

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
static void
handle_eof_input_unit ()
{
  if (interactive)
    {
      /* shell.c may use this to decide whether or not to write out the
	 history, among other things.  We use it only for error reporting
	 in this file. */
      if (EOF_Reached)
	EOF_Reached = 0;

      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (ignoreeof)
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, _("Use \"%s\" to leave the shell.\n"),
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      /* Reset the parsing state. */
	      last_read_token = current_token = '\n';
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again (0);
	      return;
	    }
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();

      last_shell_builtin = this_shell_builtin;
      this_shell_builtin = exit_builtin;
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}

/************************************************
 *						*
 *	STRING PARSING FUNCTIONS		*
 *						*
 ************************************************/

/* It's very important that these two functions treat the characters
   between ( and ) identically. */

static WORD_LIST parse_string_error;

/* Take a string and run it through the shell parser, returning the
   resultant word list.  Used by compound array assignment. */
WORD_LIST *
parse_string_to_word_list (s, flags, whom)
     char *s;
     int flags;
     const char *whom;
{
  WORD_LIST *wl;
  int tok, orig_current_token, orig_line_number;
  int orig_parser_state;
  sh_parser_state_t ps;
  int ea;

  orig_line_number = line_number;
  save_parser_state (&ps);

#if defined (HISTORY)
  bash_history_disable ();
#endif

  push_stream (1);
  if (ea = expanding_alias ())
    parser_save_alias ();

  /* WORD to avoid parsing reserved words as themselves and just parse them as
     WORDs. */
  last_read_token = WORD;

  current_command_line_count = 0;
  echo_input_at_read = expand_aliases = 0;

  with_input_from_string (s, whom);
  wl = (WORD_LIST *)NULL;

  if (flags & 1)
    {
      orig_parser_state = parser_state;		/* XXX - not needed? */
      /* State flags we don't want to persist into compound assignments. */
      parser_state &= ~PST_NOEXPAND;	/* parse_comsub sentinel */
      /* State flags we want to set for this run through the tokenizer. */
      parser_state |= PST_COMPASSIGN|PST_REPARSE|PST_STRING;
    }

  while ((tok = read_token (READ)) != yacc_EOF)
    {
      if (tok == '\n' && *bash_input.location.string == '\0')
	break;
      if (tok == '\n')		/* Allow newlines in compound assignments */
	continue;
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  line_number = orig_line_number + line_number - 1;
	  orig_current_token = current_token;
	  current_token = tok;
	  yyerror (NULL);	/* does the right thing */
	  current_token = orig_current_token;
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }
  
  last_read_token = '\n';
  pop_stream ();

  if (ea)
    parser_restore_alias ();

  restore_parser_state (&ps);

  if (flags & 1)
    parser_state = orig_parser_state;	/* XXX - not needed? */

  if (wl == &parse_string_error)
    {
      set_exit_status (EXECUTION_FAILURE);
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  return (REVERSE_LIST (wl, WORD_LIST *));
}

static char *
parse_compound_assignment (retlenp)
     int *retlenp;
{
  WORD_LIST *wl, *rl;
  int tok, orig_line_number, assignok;
  sh_parser_state_t ps;
  char *ret;

  orig_line_number = line_number;
  save_parser_state (&ps);

  /* WORD to avoid parsing reserved words as themselves and just parse them as
     WORDs. Plus it means we won't be in a command position and so alias
     expansion won't happen. */
  last_read_token = WORD;

  token = (char *)NULL;
  token_buffer_size = 0;
  wl = (WORD_LIST *)NULL;	/* ( */

  assignok = parser_state&PST_ASSIGNOK;		/* XXX */

  /* State flags we don't want to persist into compound assignments. */
  parser_state &= ~(PST_NOEXPAND|PST_CONDCMD|PST_CONDEXPR|PST_REGEXP|PST_EXTPAT);
  /* State flags we want to set for this run through the tokenizer. */
  parser_state |= PST_COMPASSIGN;

  esacs_needed_count = expecting_in_token = 0;

  while ((tok = read_token (READ)) != ')')
    {
      if (tok == '\n')			/* Allow newlines in compound assignments */
	{
	  if (SHOULD_PROMPT ())
	    prompt_again (0);
	  continue;
	}
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  current_token = tok;	/* for error reporting */
	  if (tok == yacc_EOF)	/* ( */
	    parser_error (orig_line_number, _("unexpected EOF while looking for matching `)'"));
	  else
	    yyerror(NULL);	/* does the right thing */
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }

  restore_parser_state (&ps);

  if (wl == &parse_string_error)
    {
      set_exit_status (EXECUTION_FAILURE);
      last_read_token = '\n';	/* XXX */
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  if (wl)
    {
      rl = REVERSE_LIST (wl, WORD_LIST *);
      ret = string_list (rl);
      dispose_words (rl);
    }
  else
    ret = (char *)NULL;

  if (retlenp)
    *retlenp = (ret && *ret) ? strlen (ret) : 0;

  if (assignok)
    parser_state |= PST_ASSIGNOK;

  return ret;
}

/************************************************
 *						*
 *   SAVING AND RESTORING PARTIAL PARSE STATE   *
 *						*
 ************************************************/

sh_parser_state_t *
save_parser_state (ps)
     sh_parser_state_t *ps;
{
  if (ps == 0)
    ps = (sh_parser_state_t *)xmalloc (sizeof (sh_parser_state_t));
  if (ps == 0)
    return ((sh_parser_state_t *)NULL);

  ps->parser_state = parser_state;
  ps->token_state = save_token_state ();

  ps->input_line_terminator = shell_input_line_terminator;
  ps->eof_encountered = eof_encountered;
  ps->eol_lookahead = eol_ungetc_lookahead;

  ps->prompt_string_pointer = prompt_string_pointer;

  ps->current_command_line_count = current_command_line_count;

#if defined (HISTORY)
  ps->remember_on_history = remember_on_history;
#  if defined (BANG_HISTORY)
  ps->history_expansion_inhibited = history_expansion_inhibited;
#  endif
#endif

  ps->last_command_exit_value = last_command_exit_value;
#if defined (ARRAY_VARS)
  ps->pipestatus = save_pipestatus_array ();
#endif
    
  ps->last_shell_builtin = last_shell_builtin;
  ps->this_shell_builtin = this_shell_builtin;

  ps->expand_aliases = expand_aliases;
  ps->echo_input_at_read = echo_input_at_read;
  ps->need_here_doc = need_here_doc;
  ps->here_doc_first_line = here_doc_first_line;

  ps->esacs_needed = esacs_needed_count;
  ps->expecting_in = expecting_in_token;

  if (need_here_doc == 0)
    ps->redir_stack[0] = 0;
  else
    memcpy (ps->redir_stack, redir_stack, sizeof (redir_stack[0]) * HEREDOC_MAX);

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  ps->pushed_strings = pushed_string_list;
#endif

  ps->eof_token = shell_eof_token;
  ps->token = token;
  ps->token_buffer_size = token_buffer_size;
  /* Force reallocation on next call to read_token_word */
  token = 0;
  token_buffer_size = 0;

  return (ps);
}

void
restore_parser_state (ps)
     sh_parser_state_t *ps;
{
  int i;

  if (ps == 0)
    return;

  parser_state = ps->parser_state;
  if (ps->token_state)
    {
      restore_token_state (ps->token_state);
      free (ps->token_state);
    }

  shell_input_line_terminator = ps->input_line_terminator;
  eof_encountered = ps->eof_encountered;
  eol_ungetc_lookahead = ps->eol_lookahead;

  prompt_string_pointer = ps->prompt_string_pointer;

  current_command_line_count = ps->current_command_line_count;

#if defined (HISTORY)
  remember_on_history = ps->remember_on_history;
#  if defined (BANG_HISTORY)
  history_expansion_inhibited = ps->history_expansion_inhibited;
#  endif
#endif

  last_command_exit_value = ps->last_command_exit_value;
#if defined (ARRAY_VARS)
  restore_pipestatus_array (ps->pipestatus);
#endif

  last_shell_builtin = ps->last_shell_builtin;
  this_shell_builtin = ps->this_shell_builtin;

  expand_aliases = ps->expand_aliases;
  echo_input_at_read = ps->echo_input_at_read;
  need_here_doc = ps->need_here_doc;
  here_doc_first_line = ps->here_doc_first_line;

  esacs_needed_count = ps->esacs_needed;
  expecting_in_token = ps->expecting_in;

#if 0
  for (i = 0; i < HEREDOC_MAX; i++)
    redir_stack[i] = ps->redir_stack[i];
#else
  if (need_here_doc == 0)
    redir_stack[0] = 0;
  else
    memcpy (redir_stack, ps->redir_stack, sizeof (redir_stack[0]) * HEREDOC_MAX);
#endif

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  pushed_string_list = (STRING_SAVER *)ps->pushed_strings;
#endif

  FREE (token);
  token = ps->token;
  token_buffer_size = ps->token_buffer_size;
  shell_eof_token = ps->eof_token;
}

sh_input_line_state_t *
save_input_line_state (ls)
     sh_input_line_state_t *ls;
{
  if (ls == 0)
    ls = (sh_input_line_state_t *)xmalloc (sizeof (sh_input_line_state_t));
  if (ls == 0)
    return ((sh_input_line_state_t *)NULL);

  ls->input_line = shell_input_line;
  ls->input_line_size = shell_input_line_size;
  ls->input_line_len = shell_input_line_len;
  ls->input_line_index = shell_input_line_index;

#if defined (HANDLE_MULTIBYTE)
  ls->input_property = shell_input_line_property;
  ls->input_propsize = shell_input_line_propsize;
#endif

  /* force reallocation */
  shell_input_line = 0;
  shell_input_line_size = shell_input_line_len = shell_input_line_index = 0;

#if defined (HANDLE_MULTIBYTE)
  shell_input_line_property = 0;
  shell_input_line_propsize = 0;
#endif

  return ls;
}

void
restore_input_line_state (ls)
     sh_input_line_state_t *ls;
{
  FREE (shell_input_line);
  shell_input_line = ls->input_line;
  shell_input_line_size = ls->input_line_size;
  shell_input_line_len = ls->input_line_len;
  shell_input_line_index = ls->input_line_index;

#if defined (HANDLE_MULTIBYTE)
  FREE (shell_input_line_property);
  shell_input_line_property = ls->input_property;
  shell_input_line_propsize = ls->input_propsize;
#endif

#if 0
  set_line_mbstate ();
#endif
}

/************************************************
 *						*
 *	MULTIBYTE CHARACTER HANDLING		*
 *						*
 ************************************************/

#if defined (HANDLE_MULTIBYTE)

/* We don't let the property buffer get larger than this unless the line is */
#define MAX_PROPSIZE 32768

static void
set_line_mbstate ()
{
  int c;
  size_t i, previ, len;
  mbstate_t mbs, prevs;
  size_t mbclen;
  int ilen;

  if (shell_input_line == NULL)
    return;
  len = STRLEN (shell_input_line);	/* XXX - shell_input_line_len ? */
  if (len == 0)
    return;
  if (shell_input_line_propsize >= MAX_PROPSIZE && len < MAX_PROPSIZE>>1)
    {
      free (shell_input_line_property);
      shell_input_line_property = 0;
      shell_input_line_propsize = 0;
    }
  if (len+1 > shell_input_line_propsize)
    {
      shell_input_line_propsize = len + 1;
      shell_input_line_property = (char *)xrealloc (shell_input_line_property, shell_input_line_propsize);
    }

  if (locale_mb_cur_max == 1)
    {
      memset (shell_input_line_property, 1, len);
      return;
    }

  /* XXX - use whether or not we are in a UTF-8 locale to avoid calls to
     mbrlen */
  if (locale_utf8locale == 0)
    memset (&prevs, '\0', sizeof (mbstate_t));

  for (i = previ = 0; i < len; i++)
    {
      if (locale_utf8locale == 0)
	mbs = prevs;

      c = shell_input_line[i];
      if (c == EOF)
	{
	  size_t j;
	  for (j = i; j < len; j++)
	    shell_input_line_property[j] = 1;
	  break;
	}

      if (locale_utf8locale)
	{
	  if ((unsigned char)shell_input_line[previ] < 128)	/* i != previ */
	    mbclen = 1;
	  else
	    {
	      ilen = utf8_mblen (shell_input_line + previ, i - previ + 1);
	      mbclen = (ilen == -1) ? (size_t)-1
				    : ((ilen == -2) ? (size_t)-2 : (size_t)ilen);
	    }
	}
      else
	mbclen = mbrlen (shell_input_line + previ, i - previ + 1, &mbs);

      if (mbclen == 1 || mbclen == (size_t)-1)
	{
	  mbclen = 1;
	  previ = i + 1;
	}
      else if (mbclen == (size_t)-2)
        mbclen = 0;
      else if (mbclen > 1)
	{
	  mbclen = 0;
	  previ = i + 1;
	  if (locale_utf8locale == 0)
	    prevs = mbs;
	}
      else
	{
	  size_t j;
	  for (j = i; j < len; j++)
	    shell_input_line_property[j] = 1;
	  break;
	}

      shell_input_line_property[i] = mbclen;
    }
}
#endif /* HANDLE_MULTIBYTE */
