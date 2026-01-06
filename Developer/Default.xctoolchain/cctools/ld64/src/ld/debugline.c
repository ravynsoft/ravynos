/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef KLD
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "dwarf2.h"
#include "debugline.h"

struct line_reader_data 
{
  bool little_endian;
  
  /* From the line number information header.  */
  uint8_t minimum_instruction_length;
  int8_t line_base;
  uint8_t line_range;
  uint8_t opcode_base;
  const uint8_t * standard_opcode_lengths;
  size_t numdir;
  const uint8_t * * dirnames;
  size_t numfile_orig;
  size_t numfile;  /* As updated during execution of the table.  */
  const uint8_t * * filenames;

  /* Current position in the line table.  */
  const uint8_t * cpos;
  /* End of this part of the line table.  */
  const uint8_t * end;
  /* Start of the line table.  */
  const uint8_t * init;

  struct line_info cur;
};

/* Read in a word of fixed size, which may be unaligned, in the
   appropriate endianness.  */
#define read_16(p) (lnd->little_endian		\
		    ? ((p)[1] << 8 | (p)[0])	\
		    : ((p)[0] << 8 | (p)[1]))
#define read_32(p) (lnd->little_endian					    \
		    ? ((p)[3] << 24 | (p)[2] << 16 | (p)[1] << 8 | (p)[0])  \
		    : ((p)[0] << 24 | (p)[1] << 16 | (p)[2] << 8 | (p)[3]))
#define read_64(p) (lnd->little_endian					    \
		    ? ((uint64_t) (p)[7] << 56 | (uint64_t) (p)[6] << 48    \
		       | (uint64_t) (p)[5] << 40 | (uint64_t) (p)[4] << 32  \
		       | (uint64_t) (p)[3] << 24 | (uint64_t) (p)[2] << 16u \
		       | (uint64_t) (p)[1] << 8 | (uint64_t) (p)[0])	    \
		    : ((uint64_t) (p)[0] << 56 | (uint64_t) (p)[1] << 48    \
		       | (uint64_t) (p)[2] << 40 | (uint64_t) (p)[3] << 32  \
		       | (uint64_t) (p)[4] << 24 | (uint64_t) (p)[5] << 16u \
		       | (uint64_t) (p)[6] << 8 | (uint64_t) (p)[7]))

/* Skip over a LEB128 value (signed or unsigned).  */
static void
skip_leb128 (struct line_reader_data * leb)
{
  while (leb->cpos != leb->end && *leb->cpos >= 0x80)
    leb->cpos++;
  if (leb->cpos != leb->end)
    leb->cpos++;
}

/* Read a ULEB128 into a 64-bit word.  Return (uint64_t)-1 on overflow
   or error.  On overflow, skip past the rest of the uleb128.  */
static uint64_t
read_uleb128 (struct line_reader_data * leb)
{
  uint64_t result = 0;
  int bit = 0;
  
  do  {
    uint64_t b;
    
    if (leb->cpos == leb->end)
      return (uint64_t) -1;
  
    b = *leb->cpos & 0x7f;
    
    if (bit >= 64 || b << bit >> bit != b)
      result = (uint64_t) -1;
    else {
      result |= b << bit;
      bit += 7;
    }
  } while (*leb->cpos++ >= 0x80);
  return result;
}


/* Read a SLEB128 into a 64-bit word.  Return 0 on overflow or error
   (which is not very helpful).  On overflow, skip past the rest of
   the SLEB128.  For negative numbers, this actually overflows when
   under -2^62, but since this is used for line numbers that ought to
   be OK...  */
static int64_t
read_sleb128 (struct line_reader_data * leb)
{
  const uint8_t * start_pos = leb->cpos;
  uint64_t v = read_uleb128 (leb);
  uint64_t signbit;
  
  if (v >= 1ull << 63)
    return 0;
  if (leb->cpos - start_pos > 9)
    return v;

  signbit = 1ull << ((leb->cpos - start_pos) * 7 - 1);

  return v | -(v & signbit);
}

/* Free a line_reader_data structure.  */
void
line_free (struct line_reader_data * lnd)
{
  if (! lnd)
    return;
  if (lnd->dirnames)
    free (lnd->dirnames);
  if (lnd->filenames)
    free (lnd->filenames);
  free (lnd);
}

/* Return the pathname of the file in S, or NULL on error. 
   The result will have been allocated with malloc.  */

char *
line_file (struct line_reader_data *lnd, uint64_t n)
{
  const uint8_t * prev_pos = lnd->cpos;
  size_t filelen, dirlen;
  uint64_t dir;
  char * result;
  const char * dirpath;

  /* I'm not sure if this is actually an error.  */
  if (n == 0
      || n > lnd->numfile)
    return NULL;

  filelen = strlen ((const char *)lnd->filenames[n - 1]);
  lnd->cpos = lnd->filenames[n - 1] + filelen + 1;
  dir = read_uleb128 (lnd);
  lnd->cpos = prev_pos;
  if (dir == 0
      || lnd->filenames[n - 1][0] == '/')
    return strdup ((const char *)lnd->filenames[n - 1]);
  else if (dir > lnd->numdir)
    return NULL;

  dirpath = (const char *)lnd->dirnames[dir - 1];
  dirlen = strlen (dirpath);
  if ( dirpath[dirlen-1] == '/' )
    --dirlen;
  if ( (dirpath[dirlen-1] == '.') && (dirpath[dirlen-2] == '/') )
    dirlen -= 2;
  result = malloc (dirlen + filelen + 2);
  memcpy (result, dirpath, dirlen);
  result[dirlen] = '/';
  memcpy (result + dirlen + 1, lnd->filenames[n - 1], filelen);
  result[dirlen + 1 + filelen] = '\0';
  return result;
}

/* Initialize a state S.  Return FALSE on error.  */

static void
init_state (struct line_info *s)
{
  s->file = 1;
  s->line = 1;
  s->col = 0;
  s->pc = 0;
  s->end_of_sequence = false;
}

/* Read a debug_line section.  */

struct line_reader_data *
line_open (const uint8_t * debug_line, size_t debug_line_size,
	   int little_endian)
{
  struct line_reader_data * lnd = NULL;
  bool dwarf_size_64;

  uint64_t lnd_length, header_length;
  const uint8_t * table_start;

  if (debug_line_size < 12)
    return NULL;
  
  lnd = malloc (sizeof (struct line_reader_data));
  if (! lnd)
    return NULL;
  bzero(lnd, sizeof(struct line_reader_data));
  
  lnd->little_endian = little_endian;
  lnd->cpos = debug_line;

  lnd_length = read_32 (lnd->cpos);
  lnd->cpos += 4;
  if (lnd_length == 0xffffffff)
    {
      lnd_length = read_64 (lnd->cpos);
      lnd->cpos += 8;
      dwarf_size_64 = true;
    }
  else if (lnd_length > 0xfffffff0)
    /* Not a format we understand.  */
    goto error;
  else
    dwarf_size_64 = false;

  if (debug_line_size < lnd_length + (dwarf_size_64 ? 12 : 4)
      || lnd_length < (dwarf_size_64 ? 15 : 11))
    /* Too small.  */
    goto error;
  
  if (read_16 (lnd->cpos) != 2)
    /* Unknown line number format.  */
    goto error;
  lnd->cpos += 2;

  header_length = dwarf_size_64 ? (uint64_t)read_64(lnd->cpos) : (uint64_t)read_32(lnd->cpos);
  lnd->cpos += dwarf_size_64 ? 8 : 4;
  if (lnd_length < header_length + (lnd->cpos - debug_line)
      || header_length < 7)
    goto error;

  lnd->minimum_instruction_length = lnd->cpos[0];
  /* Ignore default_is_stmt.  */
  lnd->line_base = lnd->cpos[2];
  lnd->line_range = lnd->cpos[3];
  lnd->opcode_base = lnd->cpos[4];

  if (lnd->opcode_base == 0)
    /* Every valid line number program must use at least opcode 0
       for DW_LNE_end_sequence.  */
    goto error;

  lnd->standard_opcode_lengths = lnd->cpos + 5;
  if (header_length < (uint64_t)(5 + (lnd->opcode_base - 1)))
    /* Header not long enough.  */
    goto error;
  lnd->cpos += 5 + lnd->opcode_base - 1;
  lnd->end = debug_line + header_length + (dwarf_size_64 ? 22 : 10);
  
  /* Make table of offsets to directory names.  */
  table_start = lnd->cpos;
  lnd->numdir = 0;
  while (lnd->cpos != lnd->end && *lnd->cpos)
    {
      lnd->cpos = memchr (lnd->cpos, 0, lnd->end - lnd->cpos);
      if (! lnd->cpos)
	goto error;
      lnd->cpos++;
      lnd->numdir++;
    }
  if (lnd->cpos == lnd->end)
    goto error;
  lnd->dirnames = malloc (lnd->numdir * sizeof (const uint8_t *));
  if (! lnd->dirnames)
    goto error;
  lnd->numdir = 0;
  lnd->cpos = table_start;
  while (*lnd->cpos)
    {
      lnd->dirnames[lnd->numdir++] = lnd->cpos;
      lnd->cpos = memchr (lnd->cpos, 0, lnd->end - lnd->cpos) + 1;
    }
  lnd->cpos++;
  
  /* Make table of offsets to file entries.  */
  table_start = lnd->cpos;
  lnd->numfile = 0;
  while (lnd->cpos != lnd->end && *lnd->cpos)
    {
      lnd->cpos = memchr (lnd->cpos, 0, lnd->end - lnd->cpos);
      if (! lnd->cpos)
	goto error;
      lnd->cpos++;
      skip_leb128 (lnd);
      skip_leb128 (lnd);
      skip_leb128 (lnd);
      lnd->numfile++;
    }
  if (lnd->cpos == lnd->end)
    goto error;
  lnd->filenames = malloc (lnd->numfile * sizeof (const uint8_t *));
  if (! lnd->filenames)
    goto error;
  lnd->numfile = 0;
  lnd->cpos = table_start;
  while (*lnd->cpos)
    {
      lnd->filenames[lnd->numfile++] = lnd->cpos;
      lnd->cpos = memchr (lnd->cpos, 0, lnd->end - lnd->cpos) + 1;
      skip_leb128 (lnd);
      skip_leb128 (lnd);
      skip_leb128 (lnd);
    }
  lnd->cpos++;
  
  lnd->numfile_orig = lnd->numfile;
  lnd->cpos = lnd->init = lnd->end;
  lnd->end = debug_line + lnd_length + (dwarf_size_64 ? 12 : 4);

  init_state (&lnd->cur);

  return lnd;
  
 error:
  line_free (lnd);
  return NULL;
}

/* Reset back to the beginning.  */
void
line_reset (struct line_reader_data * lnd)
{
  lnd->cpos = lnd->init;
  lnd->numfile = lnd->numfile_orig;
  init_state (&lnd->cur);
}

/* Is there no more line data available?  */
int
line_at_eof (struct line_reader_data * lnd)
{
  return lnd->cpos == lnd->end;
}

static const bool verbose = false;

static bool
next_state (struct line_reader_data *lnd)
{
  if (lnd->cur.end_of_sequence)
    init_state (&lnd->cur);

  for (;;)
    {
      uint8_t op;
      uint64_t tmp;
      
      if (lnd->cpos == lnd->end)
	return false;
      op = *lnd->cpos++;
      if (op >= lnd->opcode_base)
	{
	  op -= lnd->opcode_base;
	  
	  lnd->cur.line += op % lnd->line_range + lnd->line_base;
	  lnd->cur.pc += (op / lnd->line_range 
			  * lnd->minimum_instruction_length);
	  return true;
	}
      else switch (op)
	{
	case DW_LNS_extended_op:
	  {
	    uint64_t sz = read_uleb128 (lnd);
	    const uint8_t * eop = lnd->cpos;
	    
	    if ((uint64_t)(lnd->end - eop) < sz || sz == 0)
	      return false;
	    lnd->cpos += sz;
	    switch (*eop++)
	      {
	      case DW_LNE_end_sequence:
	      if (verbose) fprintf(stderr, "DW_LNE_end_sequence\n");
		lnd->cur.end_of_sequence = true;
		return true;
		
	      case DW_LNE_set_address:
		if (sz == 9) {
		  lnd->cur.pc = read_64 (eop);
	          if (verbose) fprintf(stderr, "DW_LNE_set_address(0x%08llX)\n", lnd->cur.pc);
		}
		else if (sz == 5) {
		  lnd->cur.pc = read_32 (eop);
	          if (verbose) fprintf(stderr, "DW_LNE_set_address(0x%08llX)\n", lnd->cur.pc);
		}
		else
		  return false;
		break;
		
	      case DW_LNE_define_file:
		{
	          if (verbose) fprintf(stderr, "DW_LNE_define_file\n");
		  const uint8_t * * filenames;
		  filenames = realloc 
		    (lnd->filenames, 
		     (lnd->numfile + 1) * sizeof (const uint8_t *));
		  if (! filenames)
		    return false;
		  /* Check for zero-termination.  */
		  if (! memchr (eop, 0, lnd->cpos - eop))
		    return false;
		  filenames[lnd->numfile++] = eop;
		  lnd->filenames = filenames;

		  /* There's other data here, like file sizes and modification
		     times, but we don't need to read it so skip it.  */
		}
		break;
		
	      default:
		/* Don't understand it, so skip it.  */
		if (verbose) fprintf(stderr, "DW_LNS_extended_op unknown\n");
		break;
	      }
	    break;
	  }
	  
	case DW_LNS_copy:
	  if (verbose) fprintf(stderr, "DW_LNS_copy\n");
	  return true;
	case DW_LNS_advance_pc:
	  tmp = read_uleb128 (lnd);
	  if (tmp == (uint64_t) -1)
	    return false;
	  lnd->cur.pc += tmp * lnd->minimum_instruction_length;
	  if (verbose) fprintf(stderr, "DW_LNS_advance_pc(0x%08llX)\n", lnd->cur.pc);
	  break;
	case DW_LNS_advance_line:
	  lnd->cur.line += read_sleb128 (lnd);
	  if (verbose) fprintf(stderr, "DW_LNS_advance_line(%lld)\n", lnd->cur.line);
	  break;
	case DW_LNS_set_file:
	  if (verbose) fprintf(stderr, "DW_LNS_set_file\n");
	  lnd->cur.file = read_uleb128 (lnd);
	  break;
	case DW_LNS_set_column:
	  if (verbose) fprintf(stderr, "DW_LNS_set_column\n");
	  lnd->cur.col = read_uleb128 (lnd);
	  break;
	case DW_LNS_const_add_pc:
	  if (verbose) fprintf(stderr, "DW_LNS_const_add_pc\n");
	  lnd->cur.pc += ((255 - lnd->opcode_base) / lnd->line_range
			  * lnd->minimum_instruction_length);
	  break;
	case DW_LNS_fixed_advance_pc:
	  if (verbose) fprintf(stderr, "DW_LNS_fixed_advance_pc\n");
	  if (lnd->end - lnd->cpos < 2)
	    return false;
	  lnd->cur.pc += read_16 (lnd->cpos);
	  lnd->cpos += 2;
	  break;
	default:
	  {
	    /* Don't know what it is, so skip it.  */
	    if (verbose) fprintf(stderr, "unknown opcode\n");
	    int i;
	    for (i = 0; i < lnd->standard_opcode_lengths[op - 1]; i++)
	      skip_leb128 (lnd);
	    break;
	  }
	}
    }
}


/* Set RESULT to the next 'interesting' line state, as indicated
   by STOP, or return FALSE on error.  The final (end-of-sequence)
   line state is always considered interesting.  */
int
line_next (struct line_reader_data * lnd,
	   struct line_info * result,
	   enum line_stop_constants stop)
{
  for (;;)
    {
      struct line_info prev = lnd->cur;

      if (! next_state (lnd))
	return false;

      if (lnd->cur.end_of_sequence)
	break;
      if (stop == line_stop_always)
	break;
      if ((stop & line_stop_pc) && lnd->cur.pc != prev.pc)
	break;
      if ((stop & line_stop_pos_mask) && lnd->cur.file != prev.file)
	break;
      if ((stop & line_stop_pos_mask) >= line_stop_line
	  && lnd->cur.line != prev.line)
	break;
      if ((stop & line_stop_pos_mask) >= line_stop_col
	  && lnd->cur.col != prev.col)
	break;
    }
  *result = lnd->cur;
  return true;
}

/* Find the region (START->pc through END->pc) in the debug_line
   information which contains PC.  This routine starts searching at
   the current position (which is returned as END), and will go all
   the way around the debug_line information.  It will return false if
   an error occurs or if there is no matching region; these may be
   distinguished by looking at START->end_of_sequence, which will be
   false on error and true if there was no matching region.
   You could write this routine using line_next, but this version
   will be slightly more efficient, and of course more convenient.  */

int
line_find_addr (struct line_reader_data * lnd,
		struct line_info * start,
		struct line_info * end,
		uint64_t pc)
{
  const uint8_t * startpos;
  struct line_info prev;

  if (lnd->cur.end_of_sequence && lnd->cpos == lnd->end)
    line_reset (lnd);

  startpos = lnd->cpos;

  do {
    prev = lnd->cur;
    if (! next_state (lnd))
      {
	start->end_of_sequence = false;
	return false;
      }
    if (lnd->cur.end_of_sequence && lnd->cpos == lnd->end)
      line_reset (lnd);
    if (lnd->cpos == startpos)
      {
	start->end_of_sequence = true;
	return false;
      }
  } while (lnd->cur.pc <= pc || prev.pc > pc || prev.end_of_sequence);
  *start = prev;
  *end = lnd->cur;
  return true;
}
#endif /* ! KLD */

