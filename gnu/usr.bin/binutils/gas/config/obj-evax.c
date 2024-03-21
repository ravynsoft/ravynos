/* obj-evax.c - EVAX (openVMS/Alpha) object file format.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   Contributed by Klaus Kämpf (kkaempf@progis.de) of
     proGIS Software, Aachen, Germany.
   Extensively enhanced by Douglas Rupp of AdaCore.

   This file is part of GAS, the GNU Assembler

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#define OBJ_HEADER "obj-evax.h"

#include "as.h"
#include "bfd.h"
#include "vms.h"
#include "subsegs.h"
#include "safe-ctype.h"

static void s_evax_weak (int);

const pseudo_typeS obj_pseudo_table[] =
{
  { "weak", s_evax_weak, 0},
  {0, 0, 0},
};				/* obj_pseudo_table */

void obj_read_begin_hook () {}

/* Handle the weak specific pseudo-op.  */

static void
s_evax_weak (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  do
    {
      c = get_symbol_name (&name);
      symbolP = symbol_find_or_make (name);
      (void) restore_line_pointer (c);
      SKIP_WHITESPACE ();
      S_SET_WEAK (symbolP);
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

void
evax_symbol_new_hook (symbolS *sym)
{
  struct evax_private_udata_struct *udata;

  udata = XNEW (struct evax_private_udata_struct);

  udata->bsym = symbol_get_bfdsym (sym);
  udata->enbsym = NULL;
  udata->origname = xstrdup (S_GET_NAME (sym));
  udata->lkindex = 0;
  symbol_get_bfdsym(sym)->udata.p = udata;
}

void
evax_frob_symbol (symbolS *sym, int *punt)
{
  const char *symname = S_GET_NAME (sym);
  int symlen = strlen (symname);
  asymbol *symbol = symbol_get_bfdsym (sym);

  if (symlen > 4
      && strcmp (symname + symlen - 4, "..en") == 0
      && S_GET_SEGMENT (sym) == undefined_section)
    {
      symbol_clear_used_in_reloc (sym);
      *punt = 1;
    }

  else if ((symbol->flags & BSF_GLOBAL) && (symbol->flags & BSF_FUNCTION))
    {
      struct evax_private_udata_struct *udata
	= (struct evax_private_udata_struct *)symbol->udata.p;

      /* Fix up equates of function definitions.  */
      while (udata->enbsym == NULL)
	{
	  /* ??? Equates have been resolved at this point so their
	     expression is O_constant; but they previously were
	     O_symbol and we hope the equated symbol is still there.  */
	  sym = symbol_get_value_expression (sym)->X_add_symbol;
	  if (sym == NULL)
            {
              as_bad (_("no entry symbol for global function '%s'"), symname);
              return;
            }
	  symbol = symbol_get_bfdsym (sym);
	  udata->enbsym
	    = ((struct evax_private_udata_struct *)symbol->udata.p)->enbsym;
	}
    }
}

void
evax_frob_file_before_adjust (void)
{
  struct alpha_linkage_fixups *l;
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  segment_info_type *seginfo;
  int linkage_index = 1;

  subseg_set (alpha_link_section, 0);
  seginfo = seg_info (alpha_link_section);

  /* Handle .linkage fixups.  */
  for (l = alpha_linkage_fixup_root; l != NULL; l = l->next)
    {
      if (S_GET_SEGMENT (l->fixp->fx_addsy) == alpha_link_section)
	{
          /* The symbol is defined in the file.  The linkage entry decays to
             two relocs.  */
	  symbolS *entry_sym;
	  fixS *fixpentry, *fixppdesc, *fixtail;

	  fixtail = seginfo->fix_tail;

	  /* Replace the linkage with the local symbols */
	  entry_sym = symbol_find
	    (((struct evax_private_udata_struct *)symbol_get_bfdsym (l->fixp->fx_addsy)->udata.p)->enbsym->name);
	  if (!entry_sym)
	    abort ();
	  fixpentry = fix_new (l->fixp->fx_frag, l->fixp->fx_where, 8,
			       entry_sym, l->fixp->fx_offset, 0,
			       BFD_RELOC_64);
	  fixppdesc = fix_new (l->fixp->fx_frag, l->fixp->fx_where + 8, 8,
			       l->fixp->fx_addsy, l->fixp->fx_offset, 0,
			       BFD_RELOC_64);
	  l->fixp->fx_size = 0;
	  l->fixp->fx_done = 1;

	  /* If not already at the tail, splice the new fixups into
	     the chain right after the one we are nulling out */
	  if (fixtail != l->fixp)
	    {
	      fixppdesc->fx_next = l->fixp->fx_next;
	      l->fixp->fx_next = fixpentry;
	      fixtail->fx_next = 0;
	      seginfo->fix_tail = fixtail;
	    }
	}
      else
	{
          /* Assign a linkage index.  */
	  ((struct evax_private_udata_struct *)
	   symbol_get_bfdsym (l->label)->udata.p)->lkindex = linkage_index;

	  l->fixp->fx_addnumber = linkage_index;

	  linkage_index += 2;
	}
    }

  subseg_set (current_section, current_subsec);
}

void
evax_frob_file_before_fix (void)
{
  /* Now that the fixups are done earlier, we need to transfer the values
     into the BFD symbols before calling fix_segment (ideally should not
     be done also later).  */
  if (symbol_rootP)
    {
      symbolS *symp;

      /* Set the value into the BFD symbol.  Up til now the value
	 has only been kept in the gas symbolS struct.  */
      for (symp = symbol_rootP; symp; symp = symbol_next (symp))
	symbol_get_bfdsym (symp)->value = S_GET_VALUE (symp);
    }
}

/* The length is computed from the maximum allowable length of 64 less the
   4 character ..xx extension that must be preserved (removed before
   crunching and appended back on afterwards).  The $<nnn>.. prefix is
   also removed and prepened back on, but doesn't enter into the length
   computation because symbols with that prefix are always resolved
   by the assembler and will never appear in the symbol table. At least
   I hope that's true, TBD.  */
#define MAX_LABEL_LENGTH 60

static char *shorten_identifier (char *);
static int is_truncated_identifier (char *);

char *
evax_shorten_name (char *id)
{
  int prefix_dotdot = 0;
  char prefix [64];
  int len = strlen (id);
  int suffix_dotdot = len;
  char suffix [64];
  char *base_id;

  /* This test may be too conservative.  */
  if (len <= MAX_LABEL_LENGTH)
    return id;

  suffix [0] = 0;
  prefix [0] = 0;

  /* Check for ..xx suffix and save it.  */
  if (startswith (&id[len-4], ".."))
    {
      suffix_dotdot = len - 4;
      strncpy (suffix, &id[len-4], 4);
      suffix [4] = 0;
    }

  /* Check for $<nnn>.. prefix and save it.  */
  if ((id[0] == '$') && ISDIGIT (id[1]))
    {
      int i;

      for (i=2; i < len; i++)
        {
	  if (!ISDIGIT (id[i]))
            {
	      if (id[i] == '.' && id [i+1] == '.')
                 {
                   prefix_dotdot = i+2;
                   strncpy (prefix, id, prefix_dotdot);
                   prefix [prefix_dotdot] = 0;
                 }
               break;
            }
        }
    }

  /* We only need worry about crunching the base symbol.  */
  base_id = xmemdup0 (&id[prefix_dotdot], suffix_dotdot - prefix_dotdot);

  if (strlen (base_id) > MAX_LABEL_LENGTH)
    {
      char new_id [4096];
      char *return_id;

      strcpy (new_id, base_id);

      /* Shorten it.  */
      strcpy (new_id, shorten_identifier (new_id));

      /* Prepend back the prefix if there was one.  */
      if (prefix_dotdot)
        {
          memmove (&new_id [prefix_dotdot], new_id, strlen (new_id) + 1);
          strncpy (new_id, prefix, prefix_dotdot);
        }

      /* Append back the suffix if there was one.  */
      if (strlen (suffix))
	strcat (new_id, suffix);

      /* Save it on the heap and return.  */
      return_id = xstrdup (new_id);

      return return_id;
    }
  else
    return id;
}

/* The code below implements a mechanism for truncating long
   identifiers to an arbitrary length (set by MAX_LABEL_LENGTH).

   It attempts to make each truncated identifier unique by replacing
   part of the identifier with an encoded 32-bit CRC and an associated
   checksum (the checksum is used as a way to determine that the name
   was truncated).

   Note that both a portion of the start and of the end of the
   identifier may be kept.  The macro ID_SUFFIX_LENGTH will return the
   number of characters in the suffix of the identifier that should be
   kept.

   The portion of the identifier that is going to be removed is
   checksummed.  The checksum is then encoded as a 5-character string,
   the characters of which are then summed.  This sum is then encoded
   as a 3-character string.  Finally, the original length of the
   identifier is encoded as a 3-character string.

   These three strings are then concatenated together (along with an _h
   which further designates that the name was truncated):

   "original_identifier"_haaaaabbbccc

   aaaaa = 32-bit CRC
   bbb = length of original identifier
   ccc = sum of 32-bit CRC characters

   The resulting identifier will be MAX_LABEL_LENGTH characters long.

   */


/* Table used to convert an integer into a string.  */

static const unsigned char codings[] = {
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_'};

/* Table used by decode_16 () to convert an encoded string back into
   an integer.  */
static unsigned char decodings[256];

/* Table used by the crc32 function to calculate the checksum.  */
static unsigned int crc32_table[256] = {0, 0};

/* Given a string in BUF, calculate a 32-bit CRC for it.

   This is used as a reasonably unique hash for the given string.  */

static unsigned int
crc32 (unsigned char *buf, int len)
{
  unsigned int crc = 0xffffffff;

  if (! crc32_table[1])
    {
      /* Initialize the CRC table and the decoding table. */
      unsigned int i, j;
      unsigned int c;

      for (i = 0; i < 256; i++)
	{
	  for (c = i << 24, j = 8; j > 0; --j)
	    c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
	  crc32_table[i] = c;
	  decodings[i] = 0;
	}
      for (i = 0; i < ARRAY_SIZE (codings); i++)
	decodings[codings[i]] = i;
    }

  while (len--)
    {
      crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *buf];
      buf++;
    }
  return crc;
}

/* Encode the lower 32 bits of VALUE as a 5-character string.  */

static unsigned char *
encode_32 (unsigned int value)
{
  static unsigned char res[6];
  int x;

  res[5] = 0;
  for(x = 0; x < 5; x++)
    {
      res[x] = codings[value % ARRAY_SIZE (codings)];
      value = value / ARRAY_SIZE (codings);
    }
  return res;
}

/* Encode the lower 16 bits of VALUE as a 3-character string.  */

static unsigned char *
encode_16 (unsigned int value)
{
  static unsigned char res[4];
  int x;

  res[3] = 0;
  for(x = 0; x < 3; x++)
    {
      res[x] = codings[value % ARRAY_SIZE (codings)];
      value = value / ARRAY_SIZE (codings);
    }
  return res;
}

/* Convert the encoded string obtained from encode_16 () back into a
   16-bit integer.  */

static int
decode_16 (const unsigned char *string)
{
  return (decodings[string[2]] * ARRAY_SIZE (codings) * ARRAY_SIZE (codings)
	  + decodings[string[1]] * ARRAY_SIZE (codings)
	  + decodings[string[0]]);
}

/* ID_SUFFIX_LENGTH is used to determine how many characters in the
   suffix of the identifier are to be preserved, if any.  */

#ifndef ID_SUFFIX_LENGTH
#define ID_SUFFIX_LENGTH(ID) (0)
#endif

/* Return a reasonably-unique version of NAME that is less than or
   equal to MAX_LABEL_LENGTH characters long.  The string returned from
   this function may be a copy of NAME; the function will never
   actually modify the contents of NAME.  */

static char newname[MAX_LABEL_LENGTH + 1];

static char *
shorten_identifier (char *name)
{
  int crc, len, sum, x, final_len;
  unsigned char *crc_chars;
  int suffix_length = ID_SUFFIX_LENGTH (name);

  if ((len = strlen (name)) <= MAX_LABEL_LENGTH)
    return name;

  final_len = MAX_LABEL_LENGTH - 2 - 5 - 3 - 3 - suffix_length;
  crc = crc32 ((unsigned char *) name + final_len,
	       len - final_len - suffix_length);
  crc_chars = encode_32 (crc);
  sum = 0;
  for (x = 0; x < 5; x++)
    sum += crc_chars [x];
  strncpy (newname, name, final_len);
  newname [MAX_LABEL_LENGTH] = 0;
  /* Now append the suffix of the original identifier, if any.  */
  if (suffix_length)
    strncpy (newname + MAX_LABEL_LENGTH - suffix_length,
	     name + len - suffix_length,
	     suffix_length);
  memcpy (newname + final_len, "_h", 2);
  memcpy (newname + final_len + 2 , crc_chars, 5);
  memcpy (newname + final_len + 2 + 5, encode_16 (len), 3);
  memcpy (newname + final_len + 2 + 5 + 3, encode_16 (sum), 3);
  if (!is_truncated_identifier (newname))
    abort ();
  return newname;
}

/* Determine whether or not ID is a truncated identifier, and return a
   non-zero value if it is.  */

static int
is_truncated_identifier (char *id)
{
  unsigned char *ptr;
  int len = strlen (id);
  /* If it's not exactly MAX_LABEL_LENGTH characters long, it can't be
     a truncated identifier.  */
  if (len != MAX_LABEL_LENGTH)
    return 0;

  /* Start scanning backwards for a _h.  */
  len = len - 3 - 3 - 5 - 2;
  ptr = (unsigned char *) id + len;
  while (ptr >= (unsigned char *) id)
    {
      if (ptr[0] == '_' && ptr[1] == 'h')
	{
	  /* Now see if the sum encoded in the identifier matches.  */
	  int x, sum;
	  sum = 0;
	  for (x = 0; x < 5; x++)
	    sum += ptr[x + 2];
	  /* If it matches, this is probably a truncated identifier.  */
	  if (sum == decode_16 (ptr + 5 + 2 + 3))
	    return 1;
	}
      ptr--;
    }
  return 0;
}

/* end of obj-evax.c */
