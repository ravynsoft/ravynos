/* This is the Assembler Pre-Processor
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

/* App, the assembler pre-processor.  This pre-processor strips out excess
   spaces, turns single-quoted characters into a decimal constant, and turns
   # <number> <filename> <garbage> into a .line <number>;.file <filename> pair.
   This needs better error-handling.
 */
#include <stdio.h>
#include <string.h>
#include "as.h"
#include "md.h"
#include "app.h"
#include "messages.h"

FILE *scrub_file = NULL;
char *scrub_string = NULL;
char *scrub_last_string = NULL;

#ifdef NeXT_MOD	/* .include feature */
/* These are moved out of do_scrub() so save_scrub_context() can save them */
static int state;
#ifdef I386
static int substate = 0;
#endif
static int old_state;
static char *out_string;
static char out_buf[20];
static int add_newlines = 0;
#endif /* NeXT_MOD .include feature */

static char	lex [256];
static char	symbol_chars[] = 
	"$._ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

#define LEX_IS_SYMBOL_COMPONENT		(1)
#define LEX_IS_WHITESPACE		(2)
#define LEX_IS_LINE_SEPERATOR		(4)
#define LEX_IS_COMMENT_START		(8)	/* JF added these two */
#define LEX_IS_LINE_COMMENT_START	(16)
#define IS_SYMBOL_COMPONENT(c)		(lex [c] & LEX_IS_SYMBOL_COMPONENT)
#define IS_WHITESPACE(c)		(lex [c] & LEX_IS_WHITESPACE)
#define IS_LINE_SEPERATOR(c)		(lex [c] & LEX_IS_LINE_SEPERATOR)
#define IS_COMMENT(c)			(lex [c] & LEX_IS_COMMENT_START)
#define IS_LINE_COMMENT(c)		(lex [c] & LEX_IS_LINE_COMMENT_START)

void
do_scrub_begin(
void)
{
    char *p;
    const char *q;

	memset(lex, '\0', sizeof(lex));		/* Trust NOBODY! */
	lex [' ']		|= LEX_IS_WHITESPACE;
	lex ['\t']		|= LEX_IS_WHITESPACE;
	lex ['\r']		|= LEX_IS_WHITESPACE;
	for (p =symbol_chars;*p;++p)
		lex [(int)*p] |= LEX_IS_SYMBOL_COMPONENT;
	lex ['\n']		|= LEX_IS_LINE_SEPERATOR;
#ifdef PPC
	if(flagseen[(int)'p'] == TRUE)
	    lex ['\r']		|= LEX_IS_LINE_SEPERATOR;
#endif /* PPC */
#ifndef DONTDEF
#ifdef NeXT_MOD
	/*
	 * This DOES not cause ':' to be a LINE SEPERATOR but does make the
	 * second if logic after flushchar: in do_scrub_next_char() to handle
	 * "foo :" and strip the blanks.  This is the way has always been and
	 * must be this way to work.
	 */
#endif /* NeXT_MOD */
	lex [':']		|= LEX_IS_LINE_SEPERATOR;
#endif /* !defined(DONTDEF) */

#if defined(M88K) || defined(PPC) || defined(HPPA)
#ifdef PPC
	if(flagseen[(int)'p'] == FALSE)
#endif /* PPC */
	    lex ['@']		|= LEX_IS_LINE_SEPERATOR;
#else
	lex [';']		|= LEX_IS_LINE_SEPERATOR;
#endif
	for (q=md_comment_chars;*q;q++)
		lex[(int)*q] |= LEX_IS_COMMENT_START;
	for (q=md_line_comment_chars;*q;q++)
		lex[(int)*q] |= LEX_IS_LINE_COMMENT_START;
}

static inline int
scrub_from_string(
void)
{
	return scrub_string == scrub_last_string ? EOF : *scrub_string++;
}

static inline void
scrub_to_string(
int ch)
{
	*--scrub_string = ch;
}

int
do_scrub_next_char(
FILE *fp)
{
	/* State 0: beginning of normal line
		1: After first whitespace on normal line (flush more white)
		2: After first non-white on normal line (keep 1white)
		3: after second white on normal line (flush white)
		4: after putting out a .line, put out digits
		5: parsing a string, then go to old-state
		6: putting out \ escape in a "d string.
		7: After putting out a .file, put out string.
		8: After putting out a .file string, flush until newline.
	        FROM line 358
		9: After seeing symbol char in state 3 (keep 1white after symchar)
	       10: After seeing whitespace in state 9 (keep white before symchar)
		-1: output string in out_string and go to the state in old_state
		-2: flush text until a '*' '/' is seen, then go to state old_state
	*/

			       
  /* FROM line 379 */
  /* I added states 9 and 10 because the MIPS ECOFF assembler uses
     constructs like ``.loc 1 20''.  This was turning into ``.loc
     120''.  States 9 and 10 ensure that a space is never dropped in
     between characters which could appear in an identifier.  Ian
     Taylor, ian@cygnus.com.  */

#ifndef NeXT_MOD	/* .include feature */
	static state;
	static old_state;
	static char *out_string;
	static char out_buf[20];
	static add_newlines = 0;
#endif /* NeXT_MOD .include feature */
	int ch;

	if(state==-1) {
		ch= *out_string++;
		if(*out_string==0) {
			state=old_state;
			old_state=3;
		}
		return ch;
	}
	if(state==-2) {
		for(;;) {
			do ch=getc_unlocked(fp);
			while(ch!=EOF && ch!='\n' && ch!='*');
			if(ch=='\n' || ch==EOF)
				return ch;
			 ch=getc_unlocked(fp);
			 if(ch==EOF || ch=='/')
			 	break;
			ungetc(ch, fp);
		}
		state=old_state;
		return ' ';
	}
	if(state==4) {
		ch=getc_unlocked(fp);
		if(ch==EOF || (ch>='0' && ch<='9'))
			return ch;
		else {
			while(ch!=EOF && IS_WHITESPACE(ch))
				ch=getc_unlocked(fp);
			if(ch=='"') {
				ungetc(ch, fp);
#if defined(M88K) || defined(PPC) || defined(HPPA)
				out_string="@ .file ";
#else
				out_string="; .file ";
#endif
				old_state=7;
				state= -1;
				return *out_string++;
			} else {
				while(ch!=EOF && ch!='\n')
					ch=getc_unlocked(fp);
#ifdef NeXT_MOD
				/* bug fix for bug #8918, which was when
				 * a full line comment line this:
				 * # 40 MP1 = M + 1
				 * got confused with a cpp output like:
				 * # 1 "hello.c" 1
				 */
				state = 0;
#endif /* NeXT_MOD */
				return ch;
			}
		}
	}
	if(state==5) {
		ch=getc_unlocked(fp);
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE && ch=='\'') {
			state=old_state;
			return '\'';
		} else
#endif /* PPC */
		if(ch=='"') {
			state=old_state;
			return '"';
		} else if(ch=='\\') {
			state=6;
			return ch;
		} else if(ch==EOF) {
 			state=old_state;
			ungetc('\n', fp);
#ifdef PPC
			if(flagseen[(int)'p'] == TRUE){
			    as_warn("End of file in string: inserted '\''");
			    return '\'';
			}
#endif /* PPC */
			as_warn("End of file in string: inserted '\"'");
			return '"';
		} else {
			return ch;
		}
	}
	if(state==6) {
		state=5;
		ch=getc_unlocked(fp);
		switch(ch) {
			/* This is neet.  Turn "string
			   more string" into "string\n  more string"
			 */
		case '\n':
			ungetc('n', fp);
			add_newlines++;
			return '\\';

		case '\'':
		case '"':
		case '\\':
		case 'b':
		case 'f':
		case 'n':
		case 'r':
		case 't':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			break;
		default:
			as_warn("Unknown escape '\\%c' in string: Ignored",ch);
			break;

		case EOF:
			as_warn("End of file in string: '\"' inserted");
			return '"';
		}
		return ch;
	}

	if(state==7) {
		ch=getc_unlocked(fp);
		state=5;
		old_state=8;
		return ch;
	}

	if(state==8) {
		do ch= getc_unlocked(fp);
		while(ch!='\n');
		state=0;
#ifdef I386
		substate = 0;
#endif
		return ch;
	}

 flushchar:
	ch=getc_unlocked(fp);
	switch(ch) {
	case ' ':
	case '\t':
		do ch=getc_unlocked(fp);
		while(ch!=EOF && IS_WHITESPACE(ch));
		if(ch==EOF)
			return ch;
		if(IS_COMMENT(ch) || (state==0 && IS_LINE_COMMENT(ch)) || ch=='/' || IS_LINE_SEPERATOR(ch)) {
			ungetc(ch, fp);
			goto flushchar;
		}
		ungetc(ch, fp);
		if(state==0 || state==2) {
#ifdef I386
			if(state == 2){
			    if(substate == 0){
				/* if in state 2 don't change to state 3
				   the first time, and leave white space
				   after the first two tokens */
				substate = 1;
				return ' ';
			    }
			    substate = 0;
			}
#endif
			state++;
			return ' ';
		}
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE && state == 3){
			return ' ';
		}
#endif
		/* FROM line 900 */
		if (state == 9)
		  state = 10;
		goto flushchar;

	case '/':
		ch=getc_unlocked(fp);
		if(ch=='*') {
			for(;;) {
				do {
					ch=getc_unlocked(fp);
					if(ch=='\n')
						add_newlines++;
				} while(ch!=EOF && ch!='*');
				ch=getc_unlocked(fp);
				if(ch==EOF || ch=='/')
					break;
				ungetc(ch, fp);
			}
			if(ch==EOF)
				as_warn("End of file in '/' '*' string: */ inserted");

			ungetc(' ', fp);
			goto flushchar;
		} else {
#if defined(I860) || defined(M88K) || defined(PPC) || defined(I386) || \
    defined(HPPA) || defined (SPARC)
		  if (ch == '/') {
		    do {
		      ch=getc_unlocked(fp);
		    } while (ch != EOF && (ch != '\n'));
		    if (ch == EOF)
		      as_warn("End of file before newline in // comment");
		    if ( ch == '\n' )	/* Push NL back so we can complete state */
		    	ungetc(ch, fp);
		    goto flushchar;
		  }
#endif
			if(IS_COMMENT('/') || (state==0 && IS_LINE_COMMENT('/'))) {
				ungetc(ch, fp);
				ch='/';
				goto deal_misc;
			}
			if(ch!=EOF)
				ungetc(ch, fp);
			return '/';
		}
		break;

	case '"':
		old_state=state;
		state=5;
		return '"';
		break;

	case '\'':
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE){
			old_state=state;
			state=5;
			return '\'';
			break;
		}
#endif
		ch=getc_unlocked(fp);
		if(ch==EOF) {
			as_warn("End-of-file after a ': \\000 inserted");
			ch=0;
		}
		sprintf(out_buf,"(%d)",ch&0xff);
		old_state=state;
		state= -1;
		out_string=out_buf;
		return *out_string++;

	case ':':
		if(state!=3) {
			state=0;
#ifdef I386
			substate = 0;
#endif
		}
		return ch;

	case '\n':
		if(add_newlines) {
			--add_newlines;
			ungetc(ch, fp);
		}
	/* Fall through.  */
#if defined(M88K) || defined(PPC) || defined(HPPA)
	case '@':
#else
	case ';':
#endif
		state=0;
#ifdef I386
		substate = 0;
#endif
		return ch;

	default:
	deal_misc:
		/* FROM line 1234 */
		if (IS_SYMBOL_COMPONENT (ch))
		  {
		    if (state == 10)
		      {
			/* This is a symbol character following another symbol
			   character, with whitespace in between.  We skipped
			   the whitespace earlier, so output it now.  */
			ungetc(ch, fp);
			state = 3;
			ch = ' ';
			return ch;
		      }

		    if (state == 3)
		      state = 9;
		  }
		/* FROM line 1321 */
		else if (state == 9)
		  {
		    /* FROM line 1324 */
		    state = 3;
		  }
		else if (state == 10)
		  /* FROM line 1345 */
		  state = 3;

		if(state==0 && IS_LINE_COMMENT(ch)) {
			do ch=getc_unlocked(fp);
			while(ch!=EOF && IS_WHITESPACE(ch));
			if(ch==EOF) {
				as_warn("EOF in comment:  Newline inserted");
				return '\n';
			}
			if(ch<'0' || ch>'9') {
				if(ch!='\n'){
					do ch=getc_unlocked(fp);
					while(ch!=EOF && ch!='\n');
				}
				if(ch==EOF)
					as_warn("EOF in Comment: Newline inserted");
				state=0;
#ifdef I386
				substate = 0;
#endif
				return '\n';
			}
			ungetc(ch, fp);
			old_state=4;
			state= -1;
			out_string=".line ";
			return *out_string++;

		} else if(IS_COMMENT(ch)) {
			do ch=getc_unlocked(fp);
			while(ch!=EOF && ch!='\n');
			if(ch==EOF)
				as_warn("EOF in comment:  Newline inserted");
			state=0;
#ifdef I386
			substate = 0;
#endif
			return '\n';

		} else if(state==0) {
			state=2;
			return ch;
		} else if(state==1) {
			state=2;
			return ch;
		} else {
			return ch;

		}
	case EOF:
		if(state==0)
			return ch;
		as_warn("End-of-File not at end of a line");
	}
	return -1;
}

int
do_scrub_next_char_from_string(void)
{
	/* State 0: beginning of normal line
		1: After first whitespace on normal line (flush more white)
		2: After first non-white on normal line (keep 1white)
		3: after second white on normal line (flush white)
		4: after putting out a .line, put out digits
		5: parsing a string, then go to old-state
		6: putting out \ escape in a "d string.
		7: After putting out a .file, put out string.
		8: After putting out a .file string, flush until newline.
		-1: output string in out_string and go to the state in old_state
		-2: flush text until a '*' '/' is seen, then go to state old_state
	*/

#ifndef NeXT_MOD	/* .include feature */
	static state;
	static old_state;
	static char *out_string;
	static char out_buf[20];
	static add_newlines = 0;
#endif /* NeXT_MOD .include feature */
	int ch;

	if(state==-1) {
		ch= *out_string++;
		if(*out_string==0) {
			state=old_state;
			old_state=3;
		}
		return ch;
	}
	if(state==-2) {
		for(;;) {
			do ch=scrub_from_string();
			while(ch!=EOF && ch!='\n' && ch!='*');
			if(ch=='\n' || ch==EOF)
				return ch;
			 ch=scrub_from_string();
			 if(ch==EOF || ch=='/')
			 	break;
			scrub_to_string(ch);
		}
		state=old_state;
		return ' ';
	}
	if(state==4) {
		ch=scrub_from_string();
		if(ch==EOF || (ch>='0' && ch<='9'))
			return ch;
		else {
			while(ch!=EOF && IS_WHITESPACE(ch))
				ch=scrub_from_string();
			if(ch=='"') {
				scrub_to_string(ch);
#if defined(M88K) || defined(PPC) || defined(HPPA)
				out_string="@ .file ";
#else
				out_string="; .file ";
#endif
				old_state=7;
				state= -1;
				return *out_string++;
			} else {
				while(ch!=EOF && ch!='\n')
					ch=scrub_from_string();
#ifdef NeXT_MOD
				/* bug fix for bug #8918, which was when
				 * a full line comment line this:
				 * # 40 MP1 = M + 1
				 * got confused with a cpp output like:
				 * # 1 "hello.c" 1
				 */
				state = 0;
#endif /* NeXT_MOD */
				return ch;
			}
		}
	}
	if(state==5) {
		ch=scrub_from_string();
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE && ch=='\'') {
			state=old_state;
			return '\'';
		} else
#endif /* PPC */
		if(ch=='"') {
			state=old_state;
			return '"';
		} else if(ch=='\\') {
			state=6;
			return ch;
		} else if(ch==EOF) {
 			state=old_state;
			scrub_to_string('\n');
#ifdef PPC
			if(flagseen[(int)'p'] == TRUE){
			    as_warn("End of file in string: inserted '\''");
			    return '\'';
			}
#endif /* PPC */
			as_warn("End of file in string: inserted '\"'");
			return '"';
		} else {
			return ch;
		}
	}
	if(state==6) {
		state=5;
		ch=scrub_from_string();
		switch(ch) {
			/* This is neet.  Turn "string
			   more string" into "string\n  more string"
			 */
		case '\n':
			scrub_to_string('n');
			add_newlines++;
			return '\\';

		case '"':
		case '\\':
		case 'b':
		case 'f':
		case 'n':
		case 'r':
		case 't':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			break;
		default:
			as_warn("Unknown escape '\\%c' in string: Ignored",ch);
			break;

		case EOF:
			as_warn("End of file in string: '\"' inserted");
			return '"';
		}
		return ch;
	}

	if(state==7) {
		ch=scrub_from_string();
		state=5;
		old_state=8;
		return ch;
	}

	if(state==8) {
		do ch= scrub_from_string();
		while(ch!='\n');
		state=0;
#ifdef I386
		substate = 0;
#endif
		return ch;
	}

 flushchar:
	ch=scrub_from_string();
	switch(ch) {
	case ' ':
	case '\t':
		do ch=scrub_from_string();
		while(ch!=EOF && IS_WHITESPACE(ch));
		if(ch==EOF)
			return ch;
		if(IS_COMMENT(ch) || (state==0 && IS_LINE_COMMENT(ch)) || ch=='/' || IS_LINE_SEPERATOR(ch)) {
			scrub_to_string(ch);
			goto flushchar;
		}
		scrub_to_string(ch);
		if(state==0 || state==2) {
			state++;
			return ' ';
		}
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE && state == 3){
			return ' ';
		}
#endif
		else goto flushchar;

	case '/':
		ch=scrub_from_string();
		if(ch=='*') {
			for(;;) {
				do {
					ch=scrub_from_string();
					if(ch=='\n')
						add_newlines++;
				} while(ch!=EOF && ch!='*');
				ch=scrub_from_string();
				if(ch==EOF || ch=='/')
					break;
				scrub_to_string(ch);
			}
			if(ch==EOF)
				as_warn("End of file in '/' '*' string: */ inserted");

			scrub_to_string(' ');
			goto flushchar;
		} else {
#if defined(I860) || defined(M88K) || defined(PPC) || defined(I386) || \
    defined(HPPA) || defined (SPARC)
		  if (ch == '/') {
		    do {
		      ch=scrub_from_string();
		    } while (ch != EOF && (ch != '\n'));
		    if (ch == EOF)
		      as_warn("End of file before newline in // comment");
		    if ( ch == '\n' )	/* Push NL back so we can complete state */
		    	scrub_to_string(ch);
		    goto flushchar;
		  }
#endif
			if(IS_COMMENT('/') || (state==0 && IS_LINE_COMMENT('/'))) {
				scrub_to_string(ch);
				ch='/';
				goto deal_misc;
			}
			if(ch!=EOF)
				scrub_to_string(ch);
			return '/';
		}
		break;

	case '"':
		old_state=state;
		state=5;
		return '"';
		break;

	case '\'':
#ifdef PPC
		if(flagseen[(int)'p'] == TRUE){
			old_state=state;
			state=5;
			return '\'';
			break;
		}
#endif
		ch=scrub_from_string();
		if(ch==EOF) {
			as_warn("End-of-file after a ': \\000 inserted");
			ch=0;
		}
		sprintf(out_buf,"(%d)",ch&0xff);
		old_state=state;
		state= -1;
		out_string=out_buf;
		return *out_string++;

	case ':':
		if(state!=3) {
			state=0;
#ifdef I386
			substate = 0;
#endif
		}
		return ch;

	case '\n':
		if(add_newlines) {
			--add_newlines;
			scrub_to_string(ch);
		}
	/* Fall through.  */
#if defined(M88K) || defined(PPC) || defined(HPPA)
	case '@':
#else
	case ';':
#endif
		state=0;
#ifdef I386
		substate = 0;
#endif
		return ch;

	default:
	deal_misc:
		if(state==0 && IS_LINE_COMMENT(ch)) {
			do ch=scrub_from_string();
			while(ch!=EOF && IS_WHITESPACE(ch));
			if(ch==EOF) {
				as_warn("EOF in comment:  Newline inserted");
				return '\n';
			}
			if(ch<'0' || ch>'9') {
				if(ch!='\n'){
					do ch=scrub_from_string();
					while(ch!=EOF && ch!='\n');
				}
				if(ch==EOF)
					as_warn("EOF in Comment: Newline inserted");
				state=0;
#ifdef I386
				substate = 0;
#endif
				return '\n';
			}
			scrub_to_string(ch);
			old_state=4;
			state= -1;
			out_string=".line ";
			return *out_string++;

		} else if(IS_COMMENT(ch)) {
			do ch=scrub_from_string();
			while(ch!=EOF && ch!='\n');
			if(ch==EOF)
				as_warn("EOF in comment:  Newline inserted");
			state=0;
#ifdef I386
	    		substate = 0;
#endif
			return '\n';

		} else if(state==0) {
			state=2;
			return ch;
		} else if(state==1) {
			state=2;
			return ch;
		} else {
			return ch;

		}
	case EOF:
		if(state==0)
			return ch;
		as_warn("End-of-File not at end of a line");
	}
	return -1;
}


#ifdef NeXT_MOD	/* .include feature */
void
save_scrub_context(
scrub_context_data *save_buffer_ptr)
{
	save_buffer_ptr->last_scrub_file = scrub_file;
	save_buffer_ptr->last_state = state;
	save_buffer_ptr->last_old_state = old_state;
	save_buffer_ptr->last_out_string = out_string;
	memcpy(save_buffer_ptr->last_out_buf, out_buf, sizeof(out_buf));
	save_buffer_ptr->last_add_newlines = add_newlines;

	state = 0;
	old_state = 0;
	out_string = NULL;
	memset(out_buf, '\0', sizeof(out_buf));
	add_newlines = 0;
}

void
restore_scrub_context(
scrub_context_data *save_buffer_ptr)
{
	scrub_file = save_buffer_ptr->last_scrub_file;
	state = save_buffer_ptr->last_state;
	old_state = save_buffer_ptr->last_old_state;
	out_string = save_buffer_ptr->last_out_string;
	memcpy(out_buf, save_buffer_ptr->last_out_buf, sizeof(out_buf));
	add_newlines = save_buffer_ptr->last_add_newlines;
}
#endif /* NeXT_MOD .include feature */

#ifdef TEST
#include <stdarg.h>

const char md_comment_chars[] = "|";
const char md_line_comment_chars[] = "#";

#ifdef PPC
/* ['x'] TRUE if "-x" seen. */
char flagseen[128] = { 0 };
#endif

int
main(
int argc,
char *argv[],
char *envp[])
{
    int ch;
#ifdef PPC
	if(argc > 1 && strncmp(argv[1], "-p", 2) == 0)
	    flagseen[(int)'p'] = TRUE;
#endif

	while((ch = do_scrub_next_char(stdin)) != EOF)
	    putc(ch, stdout);
	return(0);
}

void
as_warn(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}
#endif /* TEST */
