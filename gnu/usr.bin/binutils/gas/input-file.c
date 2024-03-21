/* input_file.c - Deal with Input Files -
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Confines all details of reading source bytes to this module.
   All O/S specific crocks should live here.
   What we lose in "efficiency" we gain in modularity.
   Note we don't need to #include the "as.h" file. No common coupling!  */

#include "as.h"
#include "input-file.h"
#include "safe-ctype.h"

/* This variable is non-zero if the file currently being read should be
   preprocessed by app.  It is zero if the file can be read straight in.  */
int preprocess = 0;

/* This code opens a file, then delivers BUFFER_SIZE character
   chunks of the file on demand.
   BUFFER_SIZE is supposed to be a number chosen for speed.
   The caller only asks once what BUFFER_SIZE is, and asks before
   the nature of the input files (if any) is known.  */

#define BUFFER_SIZE (32 * 1024)

/* We use static data: the data area is not sharable.  */

static FILE *f_in;
static const char *file_name;

/* Struct for saving the state of this module for file includes.  */
struct saved_file
  {
    FILE * f_in;
    const char * file_name;
    int    preprocess;
    char * app_save;
  };

/* These hooks accommodate most operating systems.  */

void
input_file_begin (void)
{
  f_in = (FILE *) 0;
}

void
input_file_end (void)
{
}

/* Return BUFFER_SIZE.  */
size_t
input_file_buffer_size (void)
{
  return (BUFFER_SIZE);
}

/* Push the state of our input, returning a pointer to saved info that
   can be restored with input_file_pop ().  */

char *
input_file_push (void)
{
  struct saved_file *saved;

  saved = XNEW (struct saved_file);

  saved->f_in = f_in;
  saved->file_name = file_name;
  saved->preprocess = preprocess;
  if (preprocess)
    saved->app_save = app_push ();

  /* Initialize for new file.  */
  input_file_begin ();

  return (char *) saved;
}

void
input_file_pop (char *arg)
{
  struct saved_file *saved = (struct saved_file *) arg;

  input_file_end ();		/* Close out old file.  */

  f_in = saved->f_in;
  file_name = saved->file_name;
  preprocess = saved->preprocess;
  if (preprocess)
    app_pop (saved->app_save);

  free (arg);
}

/* Open the specified file, "" means stdin.  Filename must not be null.  */

void
input_file_open (const char *filename,
		 int pre)
{
  int c;
  char buf[80];

  preprocess = pre;

  gas_assert (filename != 0);	/* Filename may not be NULL.  */
  if (filename[0])
    {
      f_in = fopen (filename, FOPEN_RT);
      file_name = filename;
    }
  else
    {
      /* Use stdin for the input file.  */
      f_in = stdin;
      /* For error messages.  */
      file_name = _("{standard input}");
    }

  if (f_in == NULL)
    {
      as_bad (_("can't open %s for reading: %s"),
	      file_name, xstrerror (errno));
      return;
    }

  c = getc (f_in);

  if (ferror (f_in))
    {
      as_bad (_("can't read from %s: %s"),
	      file_name, xstrerror (errno));

      fclose (f_in);
      f_in = NULL;
      return;
    }

  /* Check for an empty input file.  */
  if (feof (f_in))
    {
      fclose (f_in);
      f_in = NULL;
      return;
    }
  gas_assert (c != EOF);

  if (c == '#')
    {
      /* Begins with comment, may not want to preprocess.  */
      c = getc (f_in);
      if (c == 'N')
	{
	  char *p = fgets (buf, sizeof (buf), f_in);
	  if (p && startswith (p, "O_APP") && ISSPACE (p[5]))
	    preprocess = 0;
	  if (!p || !strchr (p, '\n'))
	    ungetc ('#', f_in);
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == 'A')
	{
	  char *p = fgets (buf, sizeof (buf), f_in);
	  if (p && startswith (p, "PP") && ISSPACE (p[2]))
	    preprocess = 1;
	  if (!p || !strchr (p, '\n'))
	    ungetc ('#', f_in);
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == '\n')
	ungetc ('\n', f_in);
      else
	ungetc ('#', f_in);
    }
  else
    ungetc (c, f_in);
}

/* Close input file.  */

void
input_file_close (void)
{
  /* Don't close a null file pointer.  */
  if (f_in != NULL)
    fclose (f_in);

  f_in = 0;
}

/* This function is passed to do_scrub_chars.  */

static size_t
input_file_get (char *buf, size_t buflen)
{
  size_t size;

  if (feof (f_in))
    return 0;

  size = fread (buf, sizeof (char), buflen, f_in);
  if (ferror (f_in))
    as_bad (_("can't read from %s: %s"), file_name, xstrerror (errno));
  return size;
}

/* Read a buffer from the input file.  */

char *
input_file_give_next_buffer (char *where /* Where to place 1st character of new buffer.  */)
{
  char *return_value;		/* -> Last char of what we read, + 1.  */
  size_t size;

  if (f_in == (FILE *) 0)
    return 0;
  /* fflush (stdin); could be done here if you want to synchronise
     stdin and stdout, for the case where our input file is stdin.
     Since the assembler shouldn't do any output to stdout, we
     don't bother to synch output and input.  */
  if (preprocess)
    size = do_scrub_chars (input_file_get, where, BUFFER_SIZE);
  else
    size = input_file_get (where, BUFFER_SIZE);

  if (size)
    return_value = where + size;
  else
    {
      if (fclose (f_in))
	as_warn (_("can't close %s: %s"), file_name, xstrerror (errno));

      f_in = (FILE *) 0;
      return_value = 0;
    }

  return return_value;
}
