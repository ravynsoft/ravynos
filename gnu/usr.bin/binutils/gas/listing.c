/* listing.c - maintain assembly listings
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

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

/* Contributed by Steve Chamberlain <sac@cygnus.com>

 A listing page looks like:

 LISTING_HEADER  sourcefilename pagenumber
 TITLE LINE
 SUBTITLE LINE
 linenumber address data  source
 linenumber address data  source
 linenumber address data  source
 linenumber address data  source

 If not overridden, the listing commands are:

 .title  "stuff"
 	Put "stuff" onto the title line
 .sbttl  "stuff"
        Put stuff onto the subtitle line

  If these commands come within 10 lines of the top of the page, they
  will affect the page they are on, as well as any subsequent page

 .eject
 	Throw a page
 .list
 	Increment the enable listing counter
 .nolist
 	Decrement the enable listing counter

 .psize Y[,X]
 	Set the paper size to X wide and Y high. Setting a psize Y of
	zero will suppress form feeds except where demanded by .eject

 If the counter goes below zero, listing is suppressed.

 Listings are a maintained by read calling various listing_<foo>
 functions.  What happens most is that the macro NO_LISTING is not
 defined (from the Makefile), then the macro LISTING_NEWLINE expands
 into a call to listing_newline.  The call is done from read.c, every
 time it sees a newline, and -l is on the command line.

 The function listing_newline remembers the frag associated with the
 newline, and creates a new frag - note that this is wasteful, but not
 a big deal, since listing slows things down a lot anyway.  The
 function also remembers when the filename changes.

 When all the input has finished, and gas has had a chance to settle
 down, the listing is output. This is done by running down the list of
 frag/source file records, and opening the files as needed and printing
 out the bytes and chars associated with them.

 The only things which the architecture can change about the listing
 are defined in these macros:

 LISTING_HEADER		The name of the architecture
 LISTING_WORD_SIZE      The make of the number of bytes in a word, this determines
 			the clumping of the output data. eg a value of
			2 makes words look like 1234 5678, whilst 1
			would make the same value look like 12 34 56
			78
 LISTING_LHS_WIDTH      Number of words of above size for the lhs

 LISTING_LHS_WIDTH_SECOND   Number of words for the data on the lhs
 			for the second line

 LISTING_LHS_CONT_LINES	Max number of lines to use up for a continuation
 LISTING_RHS_WIDTH      Number of chars from the input file to print
                        on a line.  */

#include "as.h"
#include "filenames.h"
#include "safe-ctype.h"
#include "input-file.h"
#include "subsegs.h"
#include "bfdver.h"
#include <time.h>
#include <stdarg.h>

#ifndef NO_LISTING

#ifndef LISTING_HEADER
#define LISTING_HEADER "GAS LISTING"
#endif
#ifndef LISTING_WORD_SIZE
#define LISTING_WORD_SIZE 4
#endif
#ifndef LISTING_LHS_WIDTH
#define LISTING_LHS_WIDTH ((LISTING_WORD_SIZE) > 4 ? 1 : 4 / (LISTING_WORD_SIZE))
#endif
#ifndef LISTING_LHS_WIDTH_SECOND
#define LISTING_LHS_WIDTH_SECOND LISTING_LHS_WIDTH
#endif
#ifndef LISTING_RHS_WIDTH
#define LISTING_RHS_WIDTH 100
#endif
#ifndef LISTING_LHS_CONT_LINES
#define LISTING_LHS_CONT_LINES 4
#endif
#define MAX_DATELEN 30

/* This structure remembers which .s were used.  */
typedef struct file_info_struct
{
  struct file_info_struct * next;
  char *                    filename;
  long                      pos;
  unsigned int              linenum;
  int                       at_end;
} file_info_type;

enum edict_enum
{
  EDICT_NONE,
  EDICT_SBTTL,
  EDICT_TITLE,
  EDICT_NOLIST,
  EDICT_LIST,
  EDICT_NOLIST_NEXT,
  EDICT_EJECT
};


struct list_message
{
  char *message;
  struct list_message *next;
};

/* This structure remembers which line from which file goes into which
   frag.  */
struct list_info_struct
{
  /* Frag which this line of source is nearest to.  */
  fragS *frag;

  /* The actual line in the source file.  */
  unsigned int line;

  /* Pointer to the file info struct for the file which this line
     belongs to.  */
  file_info_type *file;

  /* The expanded text of any macro that may have been executing.  */
  char *line_contents;

  /* Next in list.  */
  struct list_info_struct *next;

  /* Pointer to the file info struct for the high level language
     source line that belongs here.  */
  file_info_type *hll_file;

  /* High level language source line.  */
  unsigned int hll_line;

  /* Pointers to linked list of messages associated with this line.  */
  struct list_message *messages, *last_message;

  enum edict_enum edict;
  char *edict_arg;

  /* Nonzero if this line is to be omitted because it contains
     debugging information.  This can become a flags field if we come
     up with more information to store here.  */
  int debugging;
};

typedef struct list_info_struct list_info_type;

int listing_lhs_width        = LISTING_LHS_WIDTH;
int listing_lhs_width_second = LISTING_LHS_WIDTH_SECOND;
int listing_lhs_cont_lines   = LISTING_LHS_CONT_LINES;
int listing_rhs_width        = LISTING_RHS_WIDTH;

struct list_info_struct *        listing_tail;

static file_info_type *          file_info_head;
static file_info_type *          last_open_file_info;
static FILE *                    last_open_file;
static struct list_info_struct * head;
static int                       paper_width = 200;
static int                       paper_height = 60;

extern int                       listing;

/* File to output listings to.  */
static FILE *list_file;

/* This static array is used to keep the text of data to be printed
   before the start of the line.  */

#define MAX_BYTES							\
  (((LISTING_WORD_SIZE * 2) + 1) * listing_lhs_width			\
   + ((((LISTING_WORD_SIZE * 2) + 1) * listing_lhs_width_second)	\
      * listing_lhs_cont_lines)						\
   + 20)

static char *data_buffer;

/* Prototypes.  */
static void listing_message (const char *, const char *);
static file_info_type *file_info (const char *);
static void new_frag (void);
static void listing_page (list_info_type *);
static unsigned int calc_hex (list_info_type *);
static void print_lines (list_info_type *, unsigned int, const char *,
			 unsigned int);
static void list_symbol_table (void);
static int debugging_pseudo (list_info_type *, const char *);
static void listing_listing (char *);

static void
listing_message (const char *name, const char *message)
{
  if (listing_tail != (list_info_type *) NULL)
    {
      char *n = concat (name, message, (char *) NULL);
      struct list_message *lm = XNEW (struct list_message);
      lm->message = n;
      lm->next = NULL;

      if (listing_tail->last_message)
	listing_tail->last_message->next = lm;
      else
	listing_tail->messages = lm;
      listing_tail->last_message = lm;
    }
}

void
listing_warning (const char *message)
{
  listing_message (_("Warning: "), message);
}

void
listing_error (const char *message)
{
  listing_message (_("Error: "), message);
}

static file_info_type *
file_info (const char *file_name)
{
  /* Find an entry with this file name.  */
  file_info_type *p = file_info_head;

  while (p != (file_info_type *) NULL)
    {
      if (filename_cmp (p->filename, file_name) == 0)
	return p;
      p = p->next;
    }

  /* Make new entry.  */
  p = XNEW (file_info_type);
  p->next = file_info_head;
  file_info_head = p;
  p->filename = xstrdup (file_name);
  p->pos = 0;
  p->linenum = 0;
  p->at_end = 0;

  return p;
}

static void
new_frag (void)
{
  frag_wane (frag_now);
  frag_new (0);
}

void
listing_newline (char *ps)
{
  const char *file;
  unsigned int line;
  static unsigned int last_line = 0xffff;
  static const char *last_file = NULL;
  list_info_type *new_i = NULL;

  if (listing == 0)
    return;

  if (now_seg == absolute_section)
    return;

#ifdef OBJ_ELF
  /* In ELF, anything in a section beginning with .debug or .line is
     considered to be debugging information.  This includes the
     statement which switches us into the debugging section, which we
     can only set after we are already in the debugging section.  */
  if ((listing & LISTING_NODEBUG) != 0
      && listing_tail != NULL
      && ! listing_tail->debugging)
    {
      const char *segname;

      segname = segment_name (now_seg);
      if (startswith (segname, ".debug")
	  || startswith (segname, ".line"))
	listing_tail->debugging = 1;
    }
#endif

  /* PR 21977 - use the physical file name not the logical one unless high
     level source files are being included in the listing.  */
  if (listing & LISTING_HLL)
    file = as_where (&line);
  else
    file = as_where_physical (&line);

  if (ps == NULL)
    {
      if (line == last_line
	  && !(last_file && file && filename_cmp (file, last_file)))
	return;

      new_i = XNEW (list_info_type);

      /* Detect if we are reading from stdin by examining the file
	 name returned by as_where().

	 [FIXME: We rely upon the name in the strcmp below being the
	 same as the one used by input_scrub_new_file(), if that is
	 not true, then this code will fail].

	 If we are reading from stdin, then we need to save each input
	 line here (assuming of course that we actually have a line of
	 input to read), so that it can be displayed in the listing
	 that is produced at the end of the assembly.  */
      if (strcmp (file, _("{standard input}")) == 0
	  && input_line_pointer != NULL)
	{
	  char *copy, *src, *dest;
	  int len;
	  int seen_quote = 0;
	  int seen_slash = 0;

	  for (copy = input_line_pointer;
	       *copy && (seen_quote
			 || is_end_of_line [(unsigned char) *copy] != 1);
	       copy++)
	    {
	      if (seen_slash)
		seen_slash = 0;
	      else if (*copy == '\\')
		seen_slash = 1;
	      else if (*copy == '"')
		seen_quote = !seen_quote;
	    }

	  len = copy - input_line_pointer + 1;

	  copy = XNEWVEC (char, len);

	  src = input_line_pointer;
	  dest = copy;

	  while (--len)
	    {
	      unsigned char c = *src++;

	      /* Omit control characters in the listing.  */
	      if (!ISCNTRL (c))
		*dest++ = c;
	    }

	  *dest = 0;

	  new_i->line_contents = copy;
	}
      else
	new_i->line_contents = NULL;
    }
  else
    {
      new_i = XNEW (list_info_type);
      new_i->line_contents = ps;
    }

  last_line = line;
  last_file = file;

  new_frag ();

  if (listing_tail)
    listing_tail->next = new_i;
  else
    head = new_i;

  listing_tail = new_i;

  new_i->frag = frag_now;
  new_i->line = line;
  new_i->file = file_info (file);
  new_i->next = (list_info_type *) NULL;
  new_i->messages = NULL;
  new_i->last_message = NULL;
  new_i->edict = EDICT_NONE;
  new_i->hll_file = (file_info_type *) NULL;
  new_i->hll_line = 0;
  new_i->debugging = 0;

  new_frag ();

#ifdef OBJ_ELF
  /* In ELF, anything in a section beginning with .debug or .line is
     considered to be debugging information.  */
  if ((listing & LISTING_NODEBUG) != 0)
    {
      const char *segname;

      segname = segment_name (now_seg);
      if (startswith (segname, ".debug")
	  || startswith (segname, ".line"))
	new_i->debugging = 1;
    }
#endif
}

/* Attach all current frags to the previous line instead of the
   current line.  This is called by the MIPS backend when it discovers
   that it needs to add some NOP instructions; the added NOP
   instructions should go with the instruction that has the delay, not
   with the new instruction.  */

void
listing_prev_line (void)
{
  list_info_type *l;
  fragS *f;

  if (head == (list_info_type *) NULL
      || head == listing_tail)
    return;

  new_frag ();

  for (l = head; l->next != listing_tail; l = l->next)
    ;

  for (f = frchain_now->frch_root; f != (fragS *) NULL; f = f->fr_next)
    if (f->line == listing_tail)
      f->line = l;

  listing_tail->frag = frag_now;
  new_frag ();
}

/* This function returns the next source line from the file supplied,
   truncated to size.  It appends a fake line to the end of each input
   file to make using the returned buffer simpler.  */

static const char *
buffer_line (file_info_type *file, char *line, unsigned int size)
{
  unsigned int count = 0;
  int c;
  char *p = line;

  /* If we couldn't open the file, return an empty line.  */
  if (file->at_end)
    return "";

  /* Check the cache and see if we last used this file.  */
  if (!last_open_file_info || file != last_open_file_info)
    {
      if (last_open_file)
	{
	  last_open_file_info->pos = ftell (last_open_file);
	  fclose (last_open_file);
	}

      /* Open the file in the binary mode so that ftell above can
	 return a reliable value that we can feed to fseek below.  */
      last_open_file_info = file;
      last_open_file = fopen (file->filename, FOPEN_RB);
      if (last_open_file == NULL)
	{
	  file->at_end = 1;
	  return "";
	}

      /* Seek to where we were last time this file was open.  */
      if (file->pos)
	fseek (last_open_file, file->pos, SEEK_SET);
    }

  c = fgetc (last_open_file);

  while (c != EOF && c != '\n' && c != '\r')
    {
      if (++count < size)
	*p++ = c;
      c = fgetc (last_open_file);
    }

  /* If '\r' is followed by '\n', swallow that.  Likewise, if '\n'
     is followed by '\r', swallow that as well.  */
  if (c == '\r' || c == '\n')
    {
      int next = fgetc (last_open_file);

      if ((c == '\r' && next != '\n')
	  || (c == '\n' && next != '\r'))
	ungetc (next, last_open_file);
    }

  if (c == EOF)
    {
      file->at_end = 1;
      if (count + 3 < size)
	{
	  *p++ = '.';
	  *p++ = '.';
	  *p++ = '.';
	}
    }
  file->linenum++;
  *p++ = 0;
  return line;
}


/* This function rewinds the requested file back to the line requested,
   reads it in again into the buffer provided and then restores the file
   back to its original location.  */

static void
rebuffer_line (file_info_type *  file,
	       unsigned int      linenum,
	       char *            buffer,
	       unsigned int      size)
{
  unsigned int count = 0;
  unsigned int current_line;
  char * p = buffer;
  long pos;
  long pos2;
  int c;
  bool found = false;

  /* Sanity checks.  */
  if (file == NULL || buffer == NULL || size <= 1 || file->linenum <= linenum)
    return;

  /* Check the cache and see if we last used this file.  */
  if (last_open_file_info == NULL || file != last_open_file_info)
    {
      if (last_open_file)
	{
	  last_open_file_info->pos = ftell (last_open_file);
	  fclose (last_open_file);
	}

      /* Open the file in the binary mode so that ftell above can
	 return a reliable value that we can feed to fseek below.  */
      last_open_file_info = file;
      last_open_file = fopen (file->filename, FOPEN_RB);
      if (last_open_file == NULL)
	{
	  file->at_end = 1;
	  return;
	}

      /* Seek to where we were last time this file was open.  */
      if (file->pos)
	fseek (last_open_file, file->pos, SEEK_SET);
    }

  /* Remember where we are in the current file.  */
  pos2 = pos = ftell (last_open_file);
  if (pos < 3)
    return;
  current_line = file->linenum;

  /* Leave room for the nul at the end of the buffer.  */
  size -= 1;
  buffer[size] = 0;

  /* Increment the current line count by one.
     This is to allow for the fact that we are searching for the
     start of a previous line, but we do this by detecting end-of-line
     character(s) not start-of-line characters.  */
  ++ current_line;

  while (pos2 > 0 && ! found)
    {
      char * ptr;

      /* Move backwards through the file, looking for earlier lines.  */
      pos2 = (long) size > pos2 ? 0 : pos2 - size;
      fseek (last_open_file, pos2, SEEK_SET);

      /* Our caller has kindly provided us with a buffer, so we use it.  */
      if (fread (buffer, 1, size, last_open_file) != size)
	{
	  as_warn (_("unable to rebuffer file: %s\n"), file->filename);
	  return;
	}

      for (ptr = buffer + size; ptr >= buffer; -- ptr)
	{
	  if (*ptr == '\n')
	    {
	      -- current_line;

	      if (current_line == linenum)
		{
		  /* We have found the start of the line we seek.  */
		  found = true;

		  /* FIXME: We could skip the read-in-the-line code
		     below if we know that we already have the whole
		     line in the buffer.  */

		  /* Advance pos2 to the newline character we have just located.  */
		  pos2 += (ptr - buffer);

		  /* Skip the newline and, if present, the carriage return.  */
		  if (ptr + 1 == buffer + size)
		    {
		      ++pos2;
		      if (fgetc (last_open_file) == '\r')
			++ pos2;
		    }
		  else
		    pos2 += (ptr[1] == '\r' ? 2 : 1);

		  /* Move the file pointer to this location.  */
		  fseek (last_open_file, pos2, SEEK_SET);
		  break;
		}
	    }
	}
    }

  /* Read in the line.  */
  c = fgetc (last_open_file);

  while (c != EOF && c != '\n' && c != '\r')
    {
      if (count < size)
	*p++ = c;
      count++;

      c = fgetc (last_open_file);
    }

  /* If '\r' is followed by '\n', swallow that.  Likewise, if '\n'
     is followed by '\r', swallow that as well.  */
  if (c == '\r' || c == '\n')
    {
      int next = fgetc (last_open_file);

      if ((c == '\r' && next != '\n')
	  || (c == '\n' && next != '\r'))
	ungetc (next, last_open_file);
    }

  /* Terminate the line.  */
  *p++ = 0;

  /* Reset the file position.  */
  fseek (last_open_file, pos, SEEK_SET);
}

static const char *fn;
static unsigned int eject;	/* Eject pending.  */
static unsigned int page;	/* Current page number.  */
static const char *title;	/* Current title.  */
static const char *subtitle;	/* Current subtitle.  */
static unsigned int on_page;	/* Number of lines printed on current page.  */

static void
listing_page (list_info_type *list)
{
  /* Grope around, see if we can see a title or subtitle edict coming up
     soon.  (we look down 10 lines of the page and see if it's there)  */
  if ((eject || (on_page >= (unsigned int) paper_height))
      && paper_height != 0)
    {
      unsigned int c = 10;
      int had_title = 0;
      int had_subtitle = 0;

      page++;

      while (c != 0 && list)
	{
	  if (list->edict == EDICT_SBTTL && !had_subtitle)
	    {
	      had_subtitle = 1;
	      subtitle = list->edict_arg;
	    }
	  if (list->edict == EDICT_TITLE && !had_title)
	    {
	      had_title = 1;
	      title = list->edict_arg;
	    }
	  list = list->next;
	  c--;
	}

      if (page > 1)
	{
	  fprintf (list_file, "\f");
	}

      fprintf (list_file, "%s %s \t\t\tpage %d\n", LISTING_HEADER, fn, page);
      fprintf (list_file, "%s\n", title);
      fprintf (list_file, "%s\n", subtitle);
      on_page = 3;
      eject = 0;
    }
}

/* Print a line into the list_file.  Update the line count
   and if necessary start a new page.  */

static void
emit_line (list_info_type * list, const char * format, ...)
{
  va_list args;

  va_start (args, format);

  vfprintf (list_file, format, args);
  on_page++;
  listing_page (list);

  va_end (args);
}

static unsigned int
calc_hex (list_info_type *list)
{
  int data_buffer_size;
  list_info_type *first = list;
  unsigned int address = ~(unsigned int) 0;
  fragS *frag;
  fragS *frag_ptr;
  unsigned int octet_in_frag;

  /* Find first frag which says it belongs to this line.  */
  frag = list->frag;
  while (frag && frag->line != list)
    frag = frag->fr_next;

  frag_ptr = frag;

  data_buffer_size = 0;

  /* Dump all the frags which belong to this line.  */
  while (frag_ptr != (fragS *) NULL && frag_ptr->line == first)
    {
      /* Print as many bytes from the fixed part as is sensible.  */
      octet_in_frag = 0;
      while (octet_in_frag < frag_ptr->fr_fix
	     && data_buffer_size < MAX_BYTES - 3)
	{
	  if (address == ~(unsigned int) 0)
	    address = frag_ptr->fr_address / OCTETS_PER_BYTE;

	  sprintf (data_buffer + data_buffer_size,
		   "%02X",
		   (frag_ptr->fr_literal[octet_in_frag]) & 0xff);
	  data_buffer_size += 2;
	  octet_in_frag++;
	}
      if (frag_ptr->fr_type == rs_fill)
	{
	  unsigned int var_rep_max = octet_in_frag;
	  unsigned int var_rep_idx = octet_in_frag;

	  /* Print as many bytes from the variable part as is sensible.  */
	  while ((octet_in_frag
		  < frag_ptr->fr_fix + frag_ptr->fr_var * frag_ptr->fr_offset)
		 && data_buffer_size < MAX_BYTES - 3)
	    {
	      if (address == ~(unsigned int) 0)
		address = frag_ptr->fr_address / OCTETS_PER_BYTE;

	      sprintf (data_buffer + data_buffer_size,
		       "%02X",
		       (frag_ptr->fr_literal[var_rep_idx]) & 0xff);
	      data_buffer_size += 2;

	      var_rep_idx++;
	      octet_in_frag++;

	      if (var_rep_idx >= frag_ptr->fr_fix + frag_ptr->fr_var)
		var_rep_idx = var_rep_max;
	    }
	}

      frag_ptr = frag_ptr->fr_next;
    }
  data_buffer[data_buffer_size] = '\0';
  return address;
}

static void
print_lines (list_info_type *list, unsigned int lineno,
	     const char *string, unsigned int address)
{
  unsigned int idx;
  unsigned int nchars;
  unsigned int lines;
  unsigned int octet_in_word = 0;
  char *src = data_buffer;
  int cur;
  struct list_message *msg;

  /* Print the stuff on the first line.  */
  listing_page (list);
  nchars = (LISTING_WORD_SIZE * 2 + 1) * listing_lhs_width;

  /* Print the hex for the first line.  */
  if (address == ~(unsigned int) 0)
    {
      fprintf (list_file, "% 4d     ", lineno);
      for (idx = 0; idx < nchars; idx++)
	fprintf (list_file, " ");

      emit_line (NULL, "\t%s\n", string ? string : "");
      return;
    }

  if (had_errors ())
    fprintf (list_file, "% 4d ???? ", lineno);
  else
    fprintf (list_file, "% 4d %04x ", lineno, address);

  /* And the data to go along with it.  */
  idx = 0;
  cur = 0;
  while (src[cur] && idx < nchars)
    {
      int offset;
      offset = cur;
      fprintf (list_file, "%c%c", src[offset], src[offset + 1]);
      cur += 2;
      octet_in_word++;

      if (octet_in_word == LISTING_WORD_SIZE)
	{
	  fprintf (list_file, " ");
	  idx++;
	  octet_in_word = 0;
	}

      idx += 2;
    }

  for (; idx < nchars; idx++)
    fprintf (list_file, " ");

  emit_line (list, "\t%s\n", string ? string : "");

  for (msg = list->messages; msg; msg = msg->next)
    emit_line (list, "****  %s\n", msg->message);

  for (lines = 0;
       lines < (unsigned int) listing_lhs_cont_lines
	 && src[cur];
       lines++)
    {
      nchars = ((LISTING_WORD_SIZE * 2) + 1) * listing_lhs_width_second - 1;
      idx = 0;

      /* Print any more lines of data, but more compactly.  */
      fprintf (list_file, "% 4d      ", lineno);

      while (src[cur] && idx < nchars)
	{
	  int offset;
	  offset = cur;
	  fprintf (list_file, "%c%c", src[offset], src[offset + 1]);
	  cur += 2;
	  idx += 2;
	  octet_in_word++;

	  if (octet_in_word == LISTING_WORD_SIZE)
	    {
	      fprintf (list_file, " ");
	      idx++;
	      octet_in_word = 0;
	    }
	}

      emit_line (list, "\n");
    }
}

static void
list_symbol_table (void)
{
  extern symbolS *symbol_rootP;
  int got_some = 0;

  symbolS *ptr;
  eject = 1;
  listing_page (NULL);

  for (ptr = symbol_rootP; ptr != (symbolS *) NULL; ptr = symbol_next (ptr))
    {
      if (SEG_NORMAL (S_GET_SEGMENT (ptr))
	  || S_GET_SEGMENT (ptr) == absolute_section)
	{
	  /* Don't report section symbols.  They are not interesting.  */
	  if (symbol_section_p (ptr))
	    continue;

	  if (S_GET_NAME (ptr))
	    {
	      char buf[30];
	      valueT val = S_GET_VALUE (ptr);

	      bfd_sprintf_vma (stdoutput, buf, val);
	      if (!got_some)
		{
		  fprintf (list_file, "DEFINED SYMBOLS\n");
		  on_page++;
		  got_some = 1;
		}

	      if (symbol_get_frag (ptr) && symbol_get_frag (ptr)->line)
		{
		  fprintf (list_file, "%20s:%-5d  %s:%s %s\n",
			   symbol_get_frag (ptr)->line->file->filename,
			   symbol_get_frag (ptr)->line->line,
			   segment_name (S_GET_SEGMENT (ptr)),
			   buf, S_GET_NAME (ptr));
		}
	      else
		{
		  fprintf (list_file, "%33s:%s %s\n",
			   segment_name (S_GET_SEGMENT (ptr)),
			   buf, S_GET_NAME (ptr));
		}

	      on_page++;
	      listing_page (NULL);
	    }
	}

    }
  if (!got_some)
    {
      fprintf (list_file, "NO DEFINED SYMBOLS\n");
      on_page++;
    }
  emit_line (NULL, "\n");

  got_some = 0;

  for (ptr = symbol_rootP; ptr != (symbolS *) NULL; ptr = symbol_next (ptr))
    {
      if (S_GET_NAME (ptr) && strlen (S_GET_NAME (ptr)) != 0)
	{
	  if (S_GET_SEGMENT (ptr) == undefined_section)
	    {
	      if (!got_some)
		{
		  got_some = 1;

		  emit_line (NULL, "UNDEFINED SYMBOLS\n");
		}

	      emit_line (NULL, "%s\n", S_GET_NAME (ptr));
	    }
	}
    }

  if (!got_some)
    emit_line (NULL, "NO UNDEFINED SYMBOLS\n");
}

typedef struct cached_line
{
  file_info_type * file;
  unsigned int     line;
  char             buffer [LISTING_RHS_WIDTH];
} cached_line;

static void
print_source (file_info_type *  current_file,
	      list_info_type *  list,
	      unsigned int      width)
{
#define NUM_CACHE_LINES  3
  static cached_line cached_lines[NUM_CACHE_LINES];
  static int next_free_line = 0;
  cached_line * cache = NULL;

  if (current_file->linenum > list->hll_line
      && list->hll_line > 0)
    {
      /* This can happen with modern optimizing compilers.  The source
	 lines from the high level language input program are split up
	 and interleaved, meaning the line number we want to display
	 (list->hll_line) can have already been displayed.  We have
	 three choices:

	   a. Do nothing, since we have already displayed the source
	      line.  This was the old behaviour.

	   b. Display the particular line requested again, but only
	      that line.  This is the new behaviour.

	   c. Display the particular line requested again and reset
	      the current_file->line_num value so that we redisplay
	      all the following lines as well the next time we
	      encounter a larger line number.  */
      int i;

      /* Check the cache, maybe we already have the line saved.  */
      for (i = 0; i < NUM_CACHE_LINES; i++)
	if (cached_lines[i].file == current_file
	    && cached_lines[i].line == list->hll_line)
	  {
	    cache = cached_lines + i;
	    break;
	  }

      if (i == NUM_CACHE_LINES)
	{
	  cache = cached_lines + next_free_line;
	  next_free_line ++;
	  if (next_free_line == NUM_CACHE_LINES)
	    next_free_line = 0;

	  cache->file = current_file;
	  cache->line = list->hll_line;
	  cache->buffer[0] = 0;
	  rebuffer_line (current_file, cache->line, cache->buffer, width);
	}

      emit_line (list, "%4u:%-13s **** %s\n",
		 cache->line, cache->file->filename, cache->buffer);
      return;
    }

  if (!current_file->at_end)
    {
      int num_lines_shown = 0;

      while (current_file->linenum < list->hll_line
	     && !current_file->at_end)
	{
	  const char *p;

	  cache = cached_lines + next_free_line;
	  cache->file = current_file;
	  cache->line = current_file->linenum + 1;
	  cache->buffer[0] = 0;
	  p = buffer_line (current_file, cache->buffer, width);

	  /* Cache optimization:  If printing a group of lines
	     cache the first and last lines in the group.  */
	  if (num_lines_shown == 0)
	    {
	      next_free_line ++;
	      if (next_free_line == NUM_CACHE_LINES)
		next_free_line = 0;
	    }

	  emit_line (list, "%4u:%-13s **** %s\n",
		     cache->line, cache->file->filename, p);
	  num_lines_shown ++;
	}
    }
}

/* Sometimes the user doesn't want to be bothered by the debugging
   records inserted by the compiler, see if the line is suspicious.  */

static int
debugging_pseudo (list_info_type *list, const char *line)
{
#ifdef OBJ_ELF
  static int in_debug;
  int was_debug;
#endif

  if (list->debugging)
    {
#ifdef OBJ_ELF
      in_debug = 1;
#endif
      return 1;
    }
#ifdef OBJ_ELF
  was_debug = in_debug;
  in_debug = 0;
#endif

  while (ISSPACE (*line))
    line++;

  if (*line != '.')
    {
#ifdef OBJ_ELF
      /* The ELF compiler sometimes emits blank lines after switching
         out of a debugging section.  If the next line drops us back
         into debugging information, then don't print the blank line.
         This is a hack for a particular compiler behaviour, not a
         general case.  */
      if (was_debug
	  && *line == '\0'
	  && list->next != NULL
	  && list->next->debugging)
	{
	  in_debug = 1;
	  return 1;
	}
#endif

      return 0;
    }

  line++;

  if (startswith (line, "def"))
    return 1;
  if (startswith (line, "val"))
    return 1;
  if (startswith (line, "scl"))
    return 1;
  if (startswith (line, "line"))
    return 1;
  if (startswith (line, "endef"))
    return 1;
  if (startswith (line, "ln"))
    return 1;
  if (startswith (line, "type"))
    return 1;
  if (startswith (line, "size"))
    return 1;
  if (startswith (line, "dim"))
    return 1;
  if (startswith (line, "tag"))
    return 1;
  if (startswith (line, "stabs"))
    return 1;
  if (startswith (line, "stabn"))
    return 1;

  return 0;
}

static void
listing_listing (char *name ATTRIBUTE_UNUSED)
{
  list_info_type *list = head;
  file_info_type *current_hll_file = (file_info_type *) NULL;
  char *buffer;
  const char *p;
  int show_listing = 1;
  unsigned int width;

  buffer = XNEWVEC (char, listing_rhs_width);
  data_buffer = XNEWVEC (char, MAX_BYTES);
  eject = 1;
  list = head->next;

  while (list)
    {
      unsigned int list_line;

      width = listing_rhs_width > paper_width ? paper_width :
	listing_rhs_width;

      list_line = list->line;
      switch (list->edict)
	{
	case EDICT_LIST:
	  /* Skip all lines up to the current.  */
	  list_line--;
	  break;
	case EDICT_NOLIST:
	  show_listing--;
	  break;
	case EDICT_NOLIST_NEXT:
	  if (show_listing == 0)
	    list_line--;
	  break;
	case EDICT_EJECT:
	  break;
	case EDICT_NONE:
	  break;
	case EDICT_TITLE:
	  title = list->edict_arg;
	  break;
	case EDICT_SBTTL:
	  subtitle = list->edict_arg;
	  break;
	default:
	  abort ();
	}

      if (show_listing <= 0)
	{
	  while (list->file->linenum < list_line
		 && !list->file->at_end)
	    p = buffer_line (list->file, buffer, width);
	}

      if (list->edict == EDICT_LIST
	  || (list->edict == EDICT_NOLIST_NEXT && show_listing == 0))
	{
	  /* Enable listing for the single line that caused the enable.  */
	  list_line++;
	  show_listing++;
	}

      if (show_listing > 0)
	{
	  /* Scan down the list and print all the stuff which can be done
	     with this line (or lines).  */
	  if (list->hll_file)
	    current_hll_file = list->hll_file;

	  if (current_hll_file && list->hll_line && (listing & LISTING_HLL))
	    print_source (current_hll_file, list, width);

	  if (!list->line_contents || list->file->linenum)
	    {
	      while (list->file->linenum < list_line
		     && !list->file->at_end)
		{
		  unsigned int address;

		  p = buffer_line (list->file, buffer, width);

		  if (list->file->linenum < list_line)
		    address = ~(unsigned int) 0;
		  else
		    address = calc_hex (list);

		  if (!((listing & LISTING_NODEBUG)
			&& debugging_pseudo (list, p)))
		    print_lines (list, list->file->linenum, p, address);
		}
	    }

	  if (list->line_contents)
	    {
	      if (!((listing & LISTING_NODEBUG)
		    && debugging_pseudo (list, list->line_contents)))
		print_lines (list, list->line, list->line_contents,
			     calc_hex (list));

	      free (list->line_contents);
	      list->line_contents = NULL;
	    }

	  if (list->edict == EDICT_EJECT)
	    eject = 1;
	}

      if (list->edict == EDICT_NOLIST_NEXT && show_listing == 1)
	--show_listing;

      list = list->next;
    }

  free (buffer);
  free (data_buffer);
  data_buffer = NULL;
}

/* Print time stamp in ISO format:  yyyy-mm-ddThh:mm:ss.ss+/-zzzz.  */

static void
print_timestamp (void)
{
  const time_t now = time (NULL);
  struct tm * timestamp;
  char stampstr[MAX_DATELEN];

  /* Any portable way to obtain subsecond values???  */
  timestamp = localtime (&now);
  strftime (stampstr, MAX_DATELEN, "%Y-%m-%dT%H:%M:%S.000%z", timestamp);
  fprintf (list_file, _("\n time stamp    \t: %s\n\n"), stampstr);
}

static void
print_single_option (char * opt, int *pos)
{
  int opt_len = strlen (opt);

   if ((*pos + opt_len) < paper_width)
     {
        fprintf (list_file, _("%s "), opt);
        *pos = *pos + opt_len;
     }
   else
     {
        fprintf (list_file, _("\n\t%s "), opt);
        *pos = opt_len;
     }
}

/* Print options passed to as.  */

static void
print_options (char ** argv)
{
  const char *field_name = _("\n options passed\t: ");
  int pos = strlen (field_name);
  char **p;

  fputs (field_name, list_file);
  for (p = &argv[1]; *p != NULL; p++)
    if (**p == '-')
      {
        /* Ignore these.  */
        if (strcmp (*p, "-o") == 0)
          {
            if (p[1] != NULL)
              p++;
            continue;
          }
        if (strcmp (*p, "-v") == 0)
          continue;

        print_single_option (*p, &pos);
      }
}

/* Print a first section with basic info like file names, as version,
   options passed, target, and timestamp.
   The format of this section is as follows:

   AS VERSION

   fieldname TAB ':' fieldcontents
  { TAB fieldcontents-cont }  */

static void
listing_general_info (char ** argv)
{
  /* Print the stuff on the first line.  */
  eject = 1;
  listing_page (NULL);

  fprintf (list_file,
           _(" GNU assembler version %s (%s)\n\t using BFD version %s."),
           VERSION, TARGET_ALIAS, BFD_VERSION_STRING);
  print_options (argv);
  fprintf (list_file, _("\n input file    \t: %s"), fn);
  fprintf (list_file, _("\n output file   \t: %s"), out_file_name);
  fprintf (list_file, _("\n target        \t: %s"), TARGET_CANONICAL);
  print_timestamp ();
}

void
listing_print (char *name, char **argv)
{
  int using_stdout;

  title = "";
  subtitle = "";

  if (name == NULL)
    {
      list_file = stdout;
      using_stdout = 1;
    }
  else
    {
      list_file = fopen (name, FOPEN_WT);
      if (list_file != NULL)
	using_stdout = 0;
      else
	{
	  as_warn (_("can't open %s: %s"), name, xstrerror (errno));
	  list_file = stdout;
	  using_stdout = 1;
	}
    }

  if (listing & LISTING_NOFORM)
    paper_height = 0;

  if (listing & LISTING_GENERAL)
    listing_general_info (argv);

  if (listing & LISTING_LISTING)
    listing_listing (name);

  if (listing & LISTING_SYMBOLS)
    list_symbol_table ();

  if (! using_stdout)
    {
      if (fclose (list_file) == EOF)
	as_warn (_("can't close %s: %s"), name, xstrerror (errno));
    }

  if (last_open_file)
    fclose (last_open_file);
}

void
listing_file (const char *name)
{
  fn = name;
}

void
listing_eject (int ignore ATTRIBUTE_UNUSED)
{
  if (listing)
    listing_tail->edict = EDICT_EJECT;
}

/* Turn listing on or off.  An argument of 0 means to turn off
   listing.  An argument of 1 means to turn on listing.  An argument
   of 2 means to turn off listing, but as of the next line; that is,
   the current line should be listed, but the next line should not.  */

void
listing_list (int on)
{
  if (listing)
    {
      switch (on)
	{
	case 0:
	  if (listing_tail->edict == EDICT_LIST)
	    listing_tail->edict = EDICT_NONE;
	  else
	    listing_tail->edict = EDICT_NOLIST;
	  break;
	case 1:
	  if (listing_tail->edict == EDICT_NOLIST
	      || listing_tail->edict == EDICT_NOLIST_NEXT)
	    listing_tail->edict = EDICT_NONE;
	  else
	    listing_tail->edict = EDICT_LIST;
	  break;
	case 2:
	  listing_tail->edict = EDICT_NOLIST_NEXT;
	  break;
	default:
	  abort ();
	}
    }
}

void
listing_psize (int width_only)
{
  if (! width_only)
    {
      paper_height = get_absolute_expression ();

      if (paper_height < 0 || paper_height > 1000)
	{
	  paper_height = 0;
	  as_warn (_("strange paper height, set to no form"));
	}

      if (*input_line_pointer != ',')
	{
	  demand_empty_rest_of_line ();
	  return;
	}

      ++input_line_pointer;
    }

  {
    expressionS exp;

    (void) expression_and_evaluate (& exp);

    if (exp.X_op == O_constant)
      {
	offsetT new_width = exp.X_add_number;

	if (new_width > 7)
	  paper_width = new_width;
	else
	  as_bad (_("new paper width is too small"));
      }
    else if (exp.X_op != O_absent)
      as_bad (_("bad or irreducible expression for paper width"));
    else
      as_bad (_("missing expression for paper width"));
  }

  demand_empty_rest_of_line ();
}

void
listing_nopage (int ignore ATTRIBUTE_UNUSED)
{
  paper_height = 0;
}

void
listing_title (int depth)
{
  int quoted;
  char *start;
  char *ttl;
  unsigned int length;

  SKIP_WHITESPACE ();
  if (*input_line_pointer != '\"')
    quoted = 0;
  else
    {
      quoted = 1;
      ++input_line_pointer;
    }

  start = input_line_pointer;

  while (*input_line_pointer)
    {
      if (quoted
	  ? *input_line_pointer == '\"'
	  : is_end_of_line[(unsigned char) *input_line_pointer])
	{
	  if (listing)
	    {
	      length = input_line_pointer - start;
	      ttl = xmemdup0 (start, length);
	      listing_tail->edict = depth ? EDICT_SBTTL : EDICT_TITLE;
	      listing_tail->edict_arg = ttl;
	    }
	  if (quoted)
	    input_line_pointer++;
	  demand_empty_rest_of_line ();
	  return;
	}
      else if (*input_line_pointer == '\n')
	{
	  as_bad (_("new line in title"));
	  demand_empty_rest_of_line ();
	  return;
	}
      else
	{
	  input_line_pointer++;
	}
    }
}

void
listing_source_line (unsigned int line)
{
  if (listing)
    {
      new_frag ();
      listing_tail->hll_line = line;
      new_frag ();
    }
}

void
listing_source_file (const char *file)
{
  if (listing)
    listing_tail->hll_file = file_info (file);
}

#else

/* Dummy functions for when compiled without listing enabled.  */

void
listing_list (int on)
{
  s_ignore (0);
}

void
listing_eject (int ignore)
{
  s_ignore (0);
}

void
listing_psize (int ignore)
{
  s_ignore (0);
}

void
listing_nopage (int ignore)
{
  s_ignore (0);
}

void
listing_title (int depth)
{
  s_ignore (0);
}

void
listing_file (const char *name)
{
}

void
listing_newline (char *name)
{
}

void
listing_source_line (unsigned int n)
{
}

void
listing_source_file (const char *n)
{
}

#endif
