/* input_scrub.c - layer between app and the rest of the world
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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libc.h>
#ifdef NeXT_MOD	/* .include feature */
#include <sys/file.h>
#include <sys/param.h>
#endif /* NeXT_MOD .include feature */
#include "as.h"
#include "read.h"
#include "input-file.h"
#include "input-scrub.h"
#include "app.h"
#include "xmalloc.h"
#include "messages.h"

/*
 * O/S independent module to supply buffers of sanitised source code
 * to rest of assembler. We get raw input data of some length.
 * Also looks after line numbers, for e.g. error messages.
 * This module used to do the sanitising, but now a pre-processor program
 * (app) does that job so this module is degenerate.
 * Now input is pre-sanitised, so we only worry about finding the
 * last partial line. A buffer of full lines is returned to caller.
 * The last partial line begins the next buffer we build and return to caller.
 * The buffer returned to caller is preceeded by BEFORE_STRING and followed
 * by AFTER_STRING. The last character before AFTER_STRING is a newline.
 */

/*
 * We expect the following sanitation has already been done.
 *
 * No comments, reduce a comment to a space.
 * Reduce a tab to a space unless it is 1st char of line.
 * All multiple tabs and spaces collapsed into 1 char. Tab only
 *   legal if 1st char of line.
 * # line file statements converted to .line x;.file y; statements.
 * Escaped newlines at end of line: remove them but add as many newlines
 *   to end of statement as you removed in the middle, to synch line numbers.
 */

#define BEFORE_STRING ("\n")
#define AFTER_STRING ("\0")	/* memcpy of 0 chars might choke. */
#define BEFORE_SIZE (1)
#define AFTER_SIZE  (1)		/* includes the \0 */

static char *	buffer_start;	/* -> 1st char of full buffer area. */
static char *	partial_where;	/* -> after last full line in buffer. */
static int	partial_size;	/* >=0. Number of chars in partial line in buffer. */
static char	save_source [AFTER_SIZE];
				/* Because we need AFTER_STRING just after last */
				/* full line, it clobbers 1st part of partial */
				/* line. So we preserve 1st part of partial */
				/* line here. */
static int	buffer_length;	/* What is the largest size buffer that */
				/* input_file_give_next_buffer() could */
				/* return to us? */

/*
We never have more than one source file open at once.
We may, however, read more than 1 source file in an assembly.
NULL means we have no file open right now.
*/


/*
We must track the physical file and line number for error messages.
We also track a "logical" file and line number corresponding to (C?)
compiler source line numbers.
Whenever we open a file we must fill in physical_input_file. So if it is NULL
we have not opened any files yet.
*/

int doing_include = FALSE; /* TRUE when we are processing a .include */

char *physical_input_file = NULL;
char *logical_input_file = NULL;
char *layout_file = NULL;
char *input_dir = NULL;

line_numberT physical_input_line = 0;
line_numberT logical_input_line = 0;
line_numberT layout_line = 0;


void
input_scrub_begin(
void)
{
  know( strlen(BEFORE_STRING)    == BEFORE_SIZE );
  know( strlen(AFTER_STRING) + 1 ==  AFTER_SIZE );

  input_file_begin ();

  buffer_length = input_file_buffer_size ();

  buffer_start = xmalloc ((size_t)(BEFORE_SIZE + buffer_length + buffer_length + AFTER_SIZE));
  memcpy(buffer_start, BEFORE_STRING, (int)BEFORE_SIZE);

  /* Line number things. */
  logical_input_line = 0;
  logical_input_file = (char *)NULL;
  physical_input_file = NULL;	/* No file read yet. */
  do_scrub_begin();
}

void
input_scrub_end(
void)
{
  input_file_end ();
}

char *				/* Return start of caller's part of buffer. */
input_scrub_new_file(
char *filename)
{
  input_file_open (filename, !flagseen['f']);
  physical_input_file = filename[0] ? filename : "{standard input}";
  physical_input_line = 0;

  if (filename[0])
    {
      char *p;
      long len;

      p = strrchr(filename, '/');
      if (p != NULL && p[1] != '\0')
	{
	  len = p - filename + 1;
	  input_dir = xmalloc(len + 1);
	  strncpy(input_dir, filename, len);
	}
    }

  partial_size = 0;
  return (buffer_start + BEFORE_SIZE);
}

/*
 * input_scrub_next_buffer()
 *
 * The input parameter **bufp is used to return the address of a new or the
 * previous buffer containing the characters to parse.
 *
 * This uses the static variables declared in this file (previouly set up by
 * input_scrub_begin() and previous calls to itself).  The buffer is created
 * with twice the buffer_length plus the "BEFORE" and "AFTER" bytes.
 * So there maybe some characters from a partial line following the last
 * newline in the buffer.
 *
 * It returns a pointer into the buffer as the buffer_limit to the last
 * character of the last line in the buffer (before the last newline, where the
 * parsing stops).
 */
char *
input_scrub_next_buffer(
char **bufp)
{
  register char *	limit;	/* -> just after last char of buffer. */
  int give_next_size;

  if (partial_size)
    {
      memcpy(buffer_start + BEFORE_SIZE, partial_where, (int)partial_size);
      memcpy(buffer_start + BEFORE_SIZE, save_source, (int)AFTER_SIZE);
    }
get_more:
  limit = input_file_give_next_buffer(
		buffer_start + BEFORE_SIZE + partial_size,
		&give_next_size);
  if (limit)
    {
      register char *	p;	/* Find last newline. */
      for (p = limit;   * -- p != '\n';   )
	{
	}
      ++ p;
      if (p <= buffer_start + BEFORE_SIZE)
	{
	  long new;
	
	  new = limit - (buffer_start + BEFORE_SIZE + partial_size);
	  partial_size += new;

	  /*
	   * If there is enough room left in this buffer for what
	   * input_file_give_next_buffer() will need don't reallocate as we
	   * could run out of memory needlessly.
	   */
	  if((BEFORE_SIZE + buffer_length * 2) - (limit - buffer_start) >
	     give_next_size)
	      goto get_more;

  	  buffer_length = buffer_length * 2;
  	  buffer_start = xrealloc (buffer_start,
				   (size_t)(BEFORE_SIZE + buffer_length +
					  buffer_length + AFTER_SIZE));
	  *bufp = buffer_start + BEFORE_SIZE;
	  goto get_more;
	}
      partial_where = p;
      partial_size = (int)(limit - p);
      memcpy(save_source, partial_where, (int)AFTER_SIZE);
      memcpy(partial_where, AFTER_STRING, (int)AFTER_SIZE);
    }
  else
    {
      partial_where = 0;
      if (partial_size > 0)
	{
	  as_warn( "Partial line at end of file ignored" );
	}
    }
  return (partial_where);
}

/*
 * The remaining part of this file deals with line numbers, error
 * messages and so on.
 */


/*
 * seen_at_least_1_file() returns TRUE if we opened any file.
 */
int
seen_at_least_1_file(
void)
{
  return (physical_input_file != NULL);
}

void
bump_line_counters(
void)
{
  ++ physical_input_line;
  ++ logical_input_line;
}

/*
 *			new_logical_line()
 *
 * Tells us what the new logical line number and file are.
 * If the line_number is <0, we don't change the current logical line number.
 * If the fname is NULL, we don't change the current logical file name.
 */
void
new_logical_line(
char *fname,		/* DON'T destroy it! We point to it! */
int line_number)
{
  if ( fname )
    {
      logical_input_file = fname;
    }
  if ( line_number >= 0 )
    {
      logical_input_line = line_number;
    }
}

/*
 *			a s _ w h e r e ( )
 *
 * Write a line to stderr locating where we are in reading
 * input source files.
 * As a sop to the debugger of AS, pretty-print the offending line.
 */
void
as_where(
void)
{
  char *p;
  line_numberT line;

  if (physical_input_file)
    {				/* we tried to read SOME source */
      if (input_file_is_open())
	{			/* we can still read lines from source */
		p = logical_input_file ? logical_input_file : physical_input_file;
		line = logical_input_line ? logical_input_line : physical_input_line;
		fprintf(stderr,"%s:%u:", p, line);
	}
      else
	{
	p = logical_input_file ? logical_input_file : physical_input_file;
	line = logical_input_line ? logical_input_line : physical_input_line;
	if(layout_line != 0)
	    fprintf (stderr,"%s:%u:", layout_file, layout_line);
	else
	    fprintf (stderr,"%s:unknown:", p);
	}
    }
  else
    {
    }
}

void
as_file_and_line(
char **file_ret,
unsigned int *line_ret)
{
  *file_ret = NULL;
  *line_ret = 0;
  if (physical_input_file)
    {				/* we tried to read SOME source */
      *file_ret = logical_input_file ? 
		  logical_input_file : physical_input_file;
      if (input_file_is_open())
	{			/* we can still read lines from source */
	   *line_ret = logical_input_line ?
		       logical_input_line : physical_input_line;
	}
    }
}

/*
 * as_where_ProjectBuilder() returns the fileName, directory, and line number
 * to be used to tell ProjectBuilder where the error is.  Note that the '/'
 * between fileName and directory does not appear in what is returned.
 */
void
as_where_ProjectBuilder(
char **fileName,
char **directory,
int *line)
{
    char *p, *q;
    static char directory_buf[MAXPATHLEN];

	getcwd(directory_buf, sizeof(directory_buf)); /* cctools-port: getwd() -> getcwd() */
	*fileName = NULL;
	*directory = directory_buf;
	*line = 0;

	if(physical_input_file){
	    p = logical_input_file ?
		logical_input_file : physical_input_file;
	    if(input_file_is_open()){
		*line = logical_input_line ?
		        logical_input_line : physical_input_line;
	    }
	    *fileName = p;
	    q = strrchr(p, '/');
	    if(q == NULL)
		return;
	    *fileName = p + 1;
	    strncat(directory_buf, p, q - p);
	}
}

/*
 *			a s _ p e r r o r
 *
 * Like perror(3), but with more info.
 */
void
as_perror(
char *gripe,		/* Unpunctuated error theme. */
char *filename)
{
  fprintf (stderr,"as:file(%s) %s! ",
	   filename, gripe
	   );
  int xerrno = errno;
  char errbuf[64];
  if (strerror_r (errno, errbuf, sizeof(errbuf))) /* cctools-port: if (errno > sys_nerr) -> strerror_r() */
    {
      fprintf (stderr, "Unknown error #%d.", xerrno); /* cctools-port: errno -> xerrno */
    }
  else
    {
      fprintf (stderr, "%s.", errbuf); /* cctools-port: sys_errlist [errno] -> errbuf */
    }
  (void)putc('\n', stderr);
  errno = 0;			/* After reporting, clear it. */
  if (input_file_is_open())	/* RMS says don't mention line # if not needed. */
    {
      as_where();
    }
  bad_error = 1;
}


#ifdef NeXT_MOD	/* .include feature */
/* DJA -- added for .include pseudo op support */
char *
find_an_include_file(
char *no_path_name)
{
  /* cctools-port: added static keyword */
  static char				name_buffer [MAXPATHLEN];
  register struct directory_stack	* the_path_pointer;
  register char				* whole_file_name;

 /*
  * figure out what directory the file name is in.
  */
  whole_file_name = no_path_name;
  if (access(whole_file_name, R_OK))
    {
      whole_file_name = name_buffer;
      if (no_path_name[0] != '/' && input_dir != NULL)
	{
	  if (strlen (input_dir) + strlen (no_path_name) >= MAXPATHLEN)
	    as_fatal ("include file name too long: \"%s%s\"", input_dir, no_path_name);
	  else
	    {
	      strcpy (whole_file_name, input_dir);
	      strcat (whole_file_name, no_path_name);
	      if (!access(whole_file_name, R_OK))
	        goto found;
	    }
        }
      the_path_pointer = include;
      while (the_path_pointer)
        {
	  if (strlen (the_path_pointer->fname) + (strlen (no_path_name)) >= MAXPATHLEN)
	    as_fatal ("include file name too long: \"%s%s\"", the_path_pointer->fname, no_path_name);
	  else
	    {
	      *whole_file_name = '\0';
	      strcpy (whole_file_name, the_path_pointer->fname);
	      strcat (whole_file_name, "/");
	      strcat (whole_file_name, no_path_name);
	      if (!access(whole_file_name, R_OK))
	        goto found;
	    }
	  the_path_pointer = the_path_pointer->next;
	}
      the_path_pointer = include_defaults;
      while (the_path_pointer->fname != NULL)
        {
	  if (strlen (the_path_pointer->fname) + (strlen (no_path_name)) >= MAXPATHLEN)
	    as_fatal ("include file name too long: \"%s%s\"", the_path_pointer->fname, no_path_name);
	  else
	    {
	      *whole_file_name = '\0';
	      strcpy (whole_file_name, the_path_pointer->fname);
	      strcat (whole_file_name, "/");
	      strcat (whole_file_name, no_path_name);
	      if (!access(whole_file_name, R_OK))
	        goto found;
	    }
	  the_path_pointer++;
	}
      as_fatal ("Couldn't find the include file: \"%s\"", no_path_name);
      return (NULL);
    }
found:
  return (whole_file_name);
}

void
read_an_include_file(
char *no_path_name)
{
  char		      			* buffer;
  char		      			* last_buffer_limit;
  char	   	      			* last_buffer_start;
  int					  last_doing_include;
  FILE	  	      			* last_f_in;
  char	  	      			* last_file_name;
  char		      			* last_input_line_pointer;
  char	   	      			* last_logical_input_file;
  line_numberT				  last_logical_input_line;
  int	 				  last_partial_size;
  char	  	      			* last_partial_where;
  char	 	      			* last_physical_input_file;
  line_numberT				  last_physical_input_line;
  char	 				  last_save_source [AFTER_SIZE];
  int					  last_buffer_length;
#if 0
  char					* last_save_buffer;
#endif
  scrub_context_data			  scrub_context;
  register char				* whole_file_name;

 /*
  * figure out what directory the file name is in.
  */
  whole_file_name = find_an_include_file (no_path_name);

 /*
  * save a copy of the file state for a recursive call to read a file
  */
  last_buffer_limit = buffer_limit;
  last_buffer_start = buffer_start;
  last_doing_include = doing_include;
  last_f_in = f_in;
  last_file_name = file_name;
  last_input_line_pointer = input_line_pointer;
  last_logical_input_file = logical_input_file;
  last_logical_input_line = logical_input_line;
  last_partial_size = partial_size;
  last_partial_where = partial_where;
  last_physical_input_file = physical_input_file;
  last_physical_input_line = physical_input_line;
  memcpy(last_save_source, save_source, sizeof (save_source));
  last_buffer_length = buffer_length;
  save_scrub_context (&scrub_context);
 /*
  * set up for another file
  */
  partial_size = 0;
  doing_include = TRUE;
  input_scrub_begin ();
  buffer = input_scrub_new_file (whole_file_name);
  if (f_in != (FILE *)0)
    read_a_source_file(buffer);

  xfree (buffer_start);
 /*
  * restore the file state
  */
  buffer_limit = last_buffer_limit;
  buffer_start = last_buffer_start;
  doing_include = last_doing_include;
  f_in = last_f_in;
  file_name = last_file_name;
  input_line_pointer = last_input_line_pointer;
  logical_input_file = last_logical_input_file;
  logical_input_line = last_logical_input_line;
  partial_size = last_partial_size;
  partial_where = last_partial_where;
  physical_input_file = last_physical_input_file;
  physical_input_line = last_physical_input_line;
  memcpy(save_source, last_save_source, sizeof (save_source));
  buffer_length = last_buffer_length;
  restore_scrub_context (&scrub_context);
} /* read_an_include_file */
#endif /* NeXT_MOD .include feature */
