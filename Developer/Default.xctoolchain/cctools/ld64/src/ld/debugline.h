/*
 * Copyright (c) 2006 Apple, Inc. All rights reserved.
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
#ifndef __DEBUG_LINE_H__
#define __DEBUG_LINE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Information about a line.
   DIRECTORY is to be ignored if FILENAME is absolute.  
   PC will be relative to the file the debug_line section is in.  */
struct line_info
{
  uint64_t file;
  int64_t line;
  uint64_t col;
  uint64_t pc;
  int end_of_sequence;
};

/* Opaque status structure for the line readers.  */
struct line_reader_data;

/* Create a line_reader_data, given address and size of the debug_line section.
   SIZE may be (size_t)-1 if unknown, although this suppresses checking
   for an incorrectly large size in the debug_line section.
   LITTLE_ENDIAN is set if the debug_line section is for a little-endian
   machine.
   Returns NULL on error.  */
struct line_reader_data * line_open (const uint8_t * debug_line,
				     size_t debug_line_size,
				     int little_endian);

/* The STOP parameter to line_next is one of line_stop_{file,line,col},
   perhaps ORed with line_stop_pc; or line_stop_atend, or line_stop_always.  */
enum line_stop_constants {
  line_stop_atend = 0, /* Stop only at the end of a sequence.  */
  line_stop_file = 1,  /* Stop if DIRECTORY or FILENAME change.  */
  line_stop_line = 2,  /* Stop if LINE, DIRECTORY, or FILENAME change.  */
  line_stop_col = 3,   /* Stop if COL, LINE, DIRECTORY, or FILENAME change.  */
  line_stop_pos_mask = 3,
  line_stop_pc = 4,    /* Stop if PC changes.  */
  line_stop_always = 8 /* Stop always.  */
};

/* Either return FALSE on an error, in which case the line_reader_data
   may be invalid and should be passed immediately to line_free; or
   fill RESULT with the first 'interesting' line, as determined by STOP.
   The last line data in a sequence is always considered 'interesting'.  */
int line_next (struct line_reader_data * lnd,
		struct line_info * result,
		enum line_stop_constants stop);

/* Find the region (START->pc through END->pc) in the debug_line
   information which contains PC.  This routine starts searching at
   the current position (which is returned as END), and will go all
   the way around the debug_line information.  It will return false if
   an error occurs or if there is no matching region; these may be
   distinguished by looking at START->end_of_sequence, which will be
   false on error and true if there was no matching region.
   You could write this routine using line_next, but this version
   will be slightly more efficient, and of course more convenient.  */

int line_find_addr (struct line_reader_data * lnd,
		     struct line_info * start,
		     struct line_info * end,
		     uint64_t pc);

/* Return TRUE if there is more line data to be fetched.
   If line_next has not been called or it has been called but did not
   set END_OF_SEQUENCE, you can assume there is more line data,
   but it's safe to call this routine anyway.  */
int line_at_eof (struct line_reader_data * lnd);

/* Return the pathname of the file in S, or NULL on error. 
   The result will have been allocated with malloc.  */
char * line_file (struct line_reader_data *lnd, uint64_t file);

/* Reset the line_reader_data: go back to the beginning.  */
void line_reset (struct line_reader_data * lnd);

/* Free a line_reader_data structure.  */
void line_free (struct line_reader_data * lnd);

#ifdef __cplusplus
}
#endif

#endif // __DEBUG_LINE_H__

