/* input_file.c - Deal with Input Files -
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

/*
 * Confines all details of reading source bytes to this module.
 * All O/S specific crocks should live here.
 * What we lose in "efficiency" we gain in modularity.
 * Note we don't need to #include the "as.h" file. No common coupling!
 */

#define NDEBUG		/* JF remove asserts */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <assert.h>
#include <libc.h>
#include "input-file.h"
#include "xmalloc.h"
#include "input-scrub.h"
#include "app.h"
#include "as.h"

/*
 * This variable is non-zero if the file currently being read should be
 * preprocessed by app.  It is zero if the file can be read straight in.
 */
#ifndef SUSPECT
static
#endif
int preprocess = 0;

/*
 * This code opens a file, then delivers BUFFER_SIZE character
 * chunks of the file on demand.
 * BUFFER_SIZE is supposed to be a number chosen for speed.
 * The caller only asks once what BUFFER_SIZE is, and asks before
 * the nature of the input files (if any) is known.
 */

#define BUFFER_SIZE (64 * 1024)

#ifndef NeXT_MOD
static char in_buf[BUFFER_SIZE];
#endif /* !defined(NeXT_MOD) */

/*
 * We use static data: the data area is not sharable.
 */

FILE *f_in = NULL;	/* JF do things the RIGHT way */
/* static JF remove static so app.c can use file_name */
char *file_name = NULL;

/* These hooks accomodate most operating systems. */

void
input_file_begin(
void)
{
  /* file_handle = -1; */
  f_in = (FILE *)0;
}

void
input_file_end(
void)
{
}

int				/* Return BUFFER_SIZE. */
input_file_buffer_size(
void)
{
  return (BUFFER_SIZE);
}

int
input_file_is_open(
void)
{
  /* return (file_handle >= 0); */
  return f_in!=(FILE *)0;
}

void
input_file_open(
char *filename,	/* "" means use stdin. Must not be 0. */
int pre)
{
	int	c;
	char	buf[80];

	preprocess = pre;

	assert( filename != 0 );	/* Filename may not be NULL. */
	if (filename [0]) {	/* We have a file name. Suck it and see. */
		f_in=fopen(filename,"r");
		file_name=filename;
	} else {			/* use stdin for the input file. */
		f_in = stdin;
		file_name = "{standard input}"; /* For error messages. */
	}
	if (f_in==(FILE *)0) {
		as_perror ("Can't open source file for input", file_name);
		return;
	}
#ifdef NeXT_MOD	/* .include feature */
	setbuffer(f_in, xmalloc(BUFFER_SIZE), BUFFER_SIZE);
#else
	setbuffer(f_in,in_buf,BUFFER_SIZE);
#endif
	c=getc_unlocked(f_in);
	if(c=='#') {	/* Begins with comment, may not want to preprocess */
		c=getc_unlocked(f_in);
		if(c=='N') {
			fgets(buf,80,f_in);
			if(!strcmp(buf,"O_APP\n"))
				preprocess=0;
			if(!index(buf,'\n'))
				ungetc('#',f_in);	/* It was longer */
			else
				ungetc('\n',f_in);
		} else if(c=='\n')
			ungetc('\n',f_in);
		else
			ungetc('#',f_in);
	} else
		ungetc(c,f_in);
}

char *
input_file_give_next_buffer(
char *where,	/* Where to place 1st character of new buffer. */
int *give_next_size)
{
  char *	return_value;	/* -> Last char of what we read, + 1. */
  register size_t	size;

  *give_next_size = BUFFER_SIZE;
  if (f_in == (FILE *)0)
      return 0;
      /*
       * fflush (stdin); could be done here if you want to synchronise
       * stdin and stdout, for the case where our input file is stdin.
       * Since the assembler shouldn't do any output to stdout, we
       * don't bother to synch output and input.
       */
  /* size = read (file_handle, where, BUFFER_SIZE); */
  if(preprocess) {
	char *p;
	int n;
	int ch;

	scrub_file=f_in;
	for(p=where,n=BUFFER_SIZE;n;--n) {
		ch=do_scrub_next_char(scrub_file);
		if(ch==EOF)
			break;
		*p++=ch;
	}
	size=BUFFER_SIZE-n;
  } else
	size= fread(where,sizeof(char),BUFFER_SIZE,f_in);
  if (size < 0)
    {
      as_perror ("Can't read source file: end-of-file faked.", file_name);
      size = 0;
    }
  if (size)
    return_value = where + size;
  else
    {
#ifdef NeXT_MOD	/* .include feature */
#ifdef __OPENSTEP__
      if(doing_include)
	free (f_in->_base);
#endif /* defined(__OPENSTEP__) */
#endif /* NeXT_MOD .include feature */
      if (fclose (f_in))
	as_perror ("Can't close source file -- continuing", file_name);
      f_in = (FILE *)0;
      return_value = 0;
    }
  return (return_value);
}

/* end: input_file.c */
