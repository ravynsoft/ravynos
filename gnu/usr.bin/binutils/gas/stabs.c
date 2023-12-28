/* Generic stabs parsing for gas.
   Copyright (C) 1989-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "filenames.h"
#include "obstack.h"
#include "subsegs.h"
#include "ecoff.h"

/* We need this, despite the apparent object format dependency, since
   it defines stab types, which all object formats can use now.  */

#include "aout/stab_gnu.h"

/* Holds whether the assembler is generating stabs line debugging
   information or not.  Potentially used by md_cleanup function.  */

int outputting_stabs_line_debug = 0;

static void generate_asm_file (int, const char *);

/* Allow backends to override the names used for the stab sections.  */
#ifndef STAB_SECTION_NAME
#define STAB_SECTION_NAME ".stab"
#endif

#ifndef STAB_STRING_SECTION_NAME
#define STAB_STRING_SECTION_NAME ".stabstr"
#endif

/* Label at start of current function if we're in the middle of a
   .func function, in which case stabs_generate_asm_lineno emits
   function relative line number stabs.  Otherwise it emits line
   number stabs with absolute addresses.  Note that both cases only
   apply to assembler code assembled with -gstabs.  */
static const char *current_function_label;

/* Current stab section when SEPARATE_STAB_SECTIONS.  */
static segT cached_sec;

/* State used by generate_asm_file.  */
static char *last_asm_file;
static int file_label_count;

/* State used by stabs_generate_asm_lineno.  */
static int line_label_count;
static unsigned int prev_lineno;
static char *prev_line_file;

/* State used by stabs_generate_asm_func.  */
static bool void_emitted_p;

/* State used by stabs_generate_asm_endfunc.  */
static int endfunc_label_count;

/*
 * Handle .stabX directives, which used to be open-coded.
 * So much creeping featurism overloaded the semantics that we decided
 * to put all .stabX thinking in one place. Here.
 *
 * We try to make any .stabX directive legal. Other people's AS will often
 * do assembly-time consistency checks: eg assigning meaning to n_type bits
 * and "protecting" you from setting them to certain values. (They also zero
 * certain bits before emitting symbols. Tut tut.)
 *
 * If an expression is not absolute we either gripe or use the relocation
 * information. Other people's assemblers silently forget information they
 * don't need and invent information they need that you didn't supply.
 */

/*
 * Build a string dictionary entry for a .stabX symbol.
 * The symbol is added to the .<secname>str section.
 */

#ifndef SEPARATE_STAB_SECTIONS
#define SEPARATE_STAB_SECTIONS 0
#endif

unsigned int
get_stab_string_offset (const char *string, const char *stabstr_secname,
			bool free_stabstr_secname)
{
  unsigned int length;
  unsigned int retval;
  segT save_seg;
  subsegT save_subseg;
  segT seg;
  char *p;

  if (! SEPARATE_STAB_SECTIONS)
    abort ();

  length = strlen (string);

  save_seg = now_seg;
  save_subseg = now_subseg;

  /* Create the stab string section, if it doesn't already exist.  */
  seg = subseg_new (stabstr_secname, 0);
  if (free_stabstr_secname && seg->name != stabstr_secname)
    free ((char *) stabstr_secname);

  retval = seg_info (seg)->stabu.stab_string_size;
  if (retval <= 0)
    {
      /* Make sure the first string is empty.  */
      p = frag_more (1);
      *p = 0;
      retval = seg_info (seg)->stabu.stab_string_size = 1;
      bfd_set_section_flags (seg, SEC_READONLY | SEC_DEBUGGING);
    }

  if (length > 0)
    {				/* Ordinary case.  */
      p = frag_more (length + 1);
      strcpy (p, string);

      seg_info (seg)->stabu.stab_string_size += length + 1;
    }
  else
    retval = 0;

  subseg_set (save_seg, save_subseg);

  return retval;
}

#ifdef AOUT_STABS
#ifndef OBJ_PROCESS_STAB
#define OBJ_PROCESS_STAB(SEG,W,S,T,O,D)	aout_process_stab(W,S,T,O,D)
#endif

/* Here instead of obj-aout.c because other formats use it too.  */
void
aout_process_stab (int what, const char *string, int type, int other, int desc)
{
  /* Put the stab information in the symbol table.  */
  symbolS *symbol;

  /* Create the symbol now, but only insert it into the symbol chain
     after any symbols mentioned in the value expression get into the
     symbol chain.  This is to avoid "continuation symbols" (where one
     ends in "\" and the debug info is continued in the next .stabs
     directive) from being separated by other random symbols.  */
  symbol = symbol_create (string, undefined_section, &zero_address_frag, 0);
  if (what == 's' || what == 'n')
    {
      /* Pick up the value from the input line.  */
      pseudo_set (symbol);
    }
  else
    {
      /* .stabd sets the name to NULL.  Why?  */
      S_SET_NAME (symbol, NULL);
      symbol_set_frag (symbol, frag_now);
      S_SET_VALUE (symbol, (valueT) frag_now_fix ());
    }

  symbol_append (symbol, symbol_lastP, &symbol_rootP, &symbol_lastP);

  symbol_get_bfdsym (symbol)->flags |= BSF_DEBUGGING;

  S_SET_TYPE (symbol, type);
  S_SET_OTHER (symbol, other);
  S_SET_DESC (symbol, desc);
}
#endif

/* This can handle different kinds of stabs (s,n,d) and different
   kinds of stab sections.  If STAB_SECNAME_OBSTACK_END is non-NULL,
   then STAB_SECNAME and STABSTR_SECNAME will be freed if possible
   before this function returns (the former by obstack_free).  */

static void
s_stab_generic (int what,
		const char *stab_secname,
		const char *stabstr_secname,
		const char *stab_secname_obstack_end)
{
  long longint;
  const char *string;
  char *saved_string_obstack_end;
  int type;
  int other;
  int desc;

  /* The general format is:
     .stabs "STRING",TYPE,OTHER,DESC,VALUE
     .stabn TYPE,OTHER,DESC,VALUE
     .stabd TYPE,OTHER,DESC
     At this point input_line_pointer points after the pseudo-op and
     any trailing whitespace.  The argument what is one of 's', 'n' or
     'd' indicating which type of .stab this is.  */

  if (what != 's')
    {
      string = "";
      saved_string_obstack_end = 0;
    }
  else
    {
      int length;

      string = demand_copy_C_string (&length);
      if (string == NULL)
	{
	  as_warn (_(".stab%c: missing string"), what);
	  ignore_rest_of_line ();
	  return;
	}
      /* FIXME: We should probably find some other temporary storage
	 for string, rather than leaking memory if someone else
	 happens to use the notes obstack.  */
      saved_string_obstack_end = obstack_next_free (&notes);
      SKIP_WHITESPACE ();
      if (*input_line_pointer == ',')
	input_line_pointer++;
      else
	{
	  as_warn (_(".stab%c: missing comma"), what);
	  ignore_rest_of_line ();
	  return;
	}
    }

  if (get_absolute_expression_and_terminator (&longint) != ',')
    {
      as_warn (_(".stab%c: missing comma"), what);
      ignore_rest_of_line ();
      return;
    }
  type = longint;

  if (get_absolute_expression_and_terminator (&longint) != ',')
    {
      as_warn (_(".stab%c: missing comma"), what);
      ignore_rest_of_line ();
      return;
    }
  other = longint;

  desc = get_absolute_expression ();

  if ((desc > 0xffff) || (desc < -0x8000))
    /* This could happen for example with a source file with a huge
       number of lines.  The only cure is to use a different debug
       format, probably DWARF.  */
    as_warn (_(".stab%c: description field '%x' too big, try a different debug format"),
	     what, desc);

  if (what == 's' || what == 'n')
    {
      if (*input_line_pointer != ',')
	{
	  as_warn (_(".stab%c: missing comma"), what);
	  ignore_rest_of_line ();
	  return;
	}
      input_line_pointer++;
      SKIP_WHITESPACE ();
    }

#ifdef TC_PPC
#ifdef OBJ_ELF
  /* Solaris on PowerPC has decided that .stabd can take 4 arguments, so if we were
     given 4 arguments, make it a .stabn */
  else if (what == 'd')
    {
      char *save_location = input_line_pointer;

      SKIP_WHITESPACE ();
      if (*input_line_pointer == ',')
	{
	  input_line_pointer++;
	  what = 'n';
	}
      else
	input_line_pointer = save_location;
    }
#endif /* OBJ_ELF */
#endif /* TC_PPC */

#ifndef NO_LISTING
  if (listing)
    {
      switch (type)
	{
	case N_SLINE:
	  listing_source_line ((unsigned int) desc);
	  break;
	case N_SO:
	case N_SOL:
	  listing_source_file (string);
	  break;
	}
    }
#endif /* ! NO_LISTING */

  /* We have now gathered the type, other, and desc information.  For
     .stabs or .stabn, input_line_pointer is now pointing at the
     value.  */

  if (SEPARATE_STAB_SECTIONS)
    /* Output the stab information in a separate section.  This is used
       at least for COFF and ELF.  */
    {
      segT saved_seg = now_seg;
      subsegT saved_subseg = now_subseg;
      fragS *saved_frag = frag_now;
      valueT dot;
      segT seg;
      unsigned int stroff;
      char *p;

      dot = frag_now_fix ();

#ifdef md_flush_pending_output
      md_flush_pending_output ();
#endif

      if (cached_sec && strcmp (cached_sec->name, stab_secname) == 0)
	{
	  seg = cached_sec;
	  subseg_set (seg, 0);
	}
      else
	{
	  seg = subseg_new (stab_secname, 0);
	  cached_sec = seg;
	}

      if (! seg_info (seg)->hadone)
	{
	  bfd_set_section_flags (seg,
				 SEC_READONLY | SEC_RELOC | SEC_DEBUGGING);
#ifdef INIT_STAB_SECTION
	  INIT_STAB_SECTION (seg);
#endif
	  seg_info (seg)->hadone = 1;
	}

      stroff = get_stab_string_offset (string, stabstr_secname,
				       stab_secname_obstack_end != NULL);

      /* Release the string, if nobody else has used the obstack.  */
      if (saved_string_obstack_end != NULL
	  && saved_string_obstack_end == obstack_next_free (&notes))
	obstack_free (&notes, string);
      /* Similarly for the section name.  This must be done before
	 creating symbols below, which uses the notes obstack.  */
      if (seg->name != stab_secname
	  && stab_secname_obstack_end != NULL
	  && stab_secname_obstack_end == obstack_next_free (&notes))
	obstack_free (&notes, stab_secname);

      /* At least for now, stabs in a special stab section are always
	 output as 12 byte blocks of information.  */
      p = frag_more (8);
      md_number_to_chars (p, (valueT) stroff, 4);
      md_number_to_chars (p + 4, (valueT) type, 1);
      md_number_to_chars (p + 5, (valueT) other, 1);
      md_number_to_chars (p + 6, (valueT) desc, 2);

      if (what == 's' || what == 'n')
	{
	  /* Pick up the value from the input line.  */
	  cons (4);
	  input_line_pointer--;
	}
      else
	{
	  symbolS *symbol;
	  expressionS exp;

	  /* Arrange for a value representing the current location.  */
	  symbol = symbol_temp_new (saved_seg, saved_frag, dot);

	  exp.X_op = O_symbol;
	  exp.X_add_symbol = symbol;
	  exp.X_add_number = 0;

	  emit_expr (&exp, 4);
	}

#ifdef OBJ_PROCESS_STAB
      OBJ_PROCESS_STAB (seg, what, string, type, other, desc);
#endif

      subseg_set (saved_seg, saved_subseg);
    }
  else
    {
      if (stab_secname_obstack_end != NULL)
	{
	  free ((char *) stabstr_secname);
	  if (stab_secname_obstack_end == obstack_next_free (&notes))
	    obstack_free (&notes, stab_secname);
	}
#ifdef OBJ_PROCESS_STAB
      OBJ_PROCESS_STAB (0, what, string, type, other, desc);
#else
      abort ();
#endif
    }

  demand_empty_rest_of_line ();
}

/* Regular stab directive.  */

void
s_stab (int what)
{
  s_stab_generic (what, STAB_SECTION_NAME, STAB_STRING_SECTION_NAME, NULL);
}

/* "Extended stabs", used in Solaris only now.  */

void
s_xstab (int what)
{
  int length;
  char *stab_secname, *stabstr_secname, *stab_secname_obstack_end;

  stab_secname = demand_copy_C_string (&length);
  stab_secname_obstack_end = obstack_next_free (&notes);
  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    input_line_pointer++;
  else
    {
      as_bad (_("comma missing in .xstabs"));
      ignore_rest_of_line ();
      return;
    }

  /* To get the name of the stab string section, simply add "str" to
     the stab section name.  */
  stabstr_secname = concat (stab_secname, "str", (char *) NULL);
  s_stab_generic (what, stab_secname, stabstr_secname,
		  stab_secname_obstack_end);
}

#ifdef S_SET_DESC

/* Frob invented at RMS' request. Set the n_desc of a symbol.  */

void
s_desc (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  char *p;
  symbolS *symbolP;
  int temp;

  c = get_symbol_name (&name);
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE_AFTER_NAME ();
  if (*input_line_pointer != ',')
    {
      *p = 0;
      as_bad (_("expected comma after \"%s\""), name);
      *p = c;
      ignore_rest_of_line ();
    }
  else
    {
      input_line_pointer++;
      temp = get_absolute_expression ();
      *p = 0;
      symbolP = symbol_find_or_make (name);
      *p = c;
      S_SET_DESC (symbolP, temp);
    }
  demand_empty_rest_of_line ();
}				/* s_desc() */

#endif /* defined (S_SET_DESC) */

/* Generate stabs debugging information to denote the main source file.  */

void
stabs_generate_asm_file (void)
{
  const char *file;
  unsigned int lineno;

  file = as_where (&lineno);
  if (use_gnu_debug_info_extensions)
    {
      char *dir;
      char *dir2;

      dir = remap_debug_filename (getpwd ());
      dir2 = concat (dir, "/", NULL);
      generate_asm_file (N_SO, dir2);
      free (dir2);
      free (dir);
    }
  generate_asm_file (N_SO, file);
}

/* Generate stabs debugging information to denote the source file.
   TYPE is one of N_SO, N_SOL.  */

static void
generate_asm_file (int type, const char *file)
{
  char sym[30];
  char *buf;
  const char *tmp = file;
  const char *file_endp = file + strlen (file);
  char *bufp;

  if (last_asm_file != NULL
      && filename_cmp (last_asm_file, file) == 0)
    return;

  /* Rather than try to do this in some efficient fashion, we just
     generate a string and then parse it again.  That lets us use the
     existing stabs hook, which expect to see a string, rather than
     inventing new ones.  */
  sprintf (sym, "%sF%d", FAKE_LABEL_NAME, file_label_count);
  ++file_label_count;

  /* Allocate enough space for the file name (possibly extended with
     doubled up backslashes), the symbol name, and the other characters
     that make up a stabs file directive.  */
  bufp = buf = XNEWVEC (char, 2 * strlen (file) + strlen (sym) + 12);

  *bufp++ = '"';

  while (tmp < file_endp)
    {
      const char *bslash = strchr (tmp, '\\');
      size_t len = bslash != NULL ? bslash - tmp + 1 : file_endp - tmp;

      /* Double all backslashes, since demand_copy_C_string (used by
	 s_stab to extract the part in quotes) will try to replace them as
	 escape sequences.  backslash may appear in a filespec.  */
      memcpy (bufp, tmp, len);

      tmp += len;
      bufp += len;

      if (bslash != NULL)
	*bufp++ = '\\';
    }

  sprintf (bufp, "\",%d,0,0,%s\n", type, sym);

  temp_ilp (buf);
  s_stab ('s');
  restore_ilp ();

  colon (sym);

  free (last_asm_file);
  last_asm_file = xstrdup (file);

  free (buf);
}

/* Generate stabs debugging information for the current line.  This is
   used to produce debugging information for an assembler file.  */

void
stabs_generate_asm_lineno (void)
{
  const char *file;
  unsigned int lineno;
  char *buf;
  char sym[30];

  /* Rather than try to do this in some efficient fashion, we just
     generate a string and then parse it again.  That lets us use the
     existing stabs hook, which expect to see a string, rather than
     inventing new ones.  */

  file = as_where (&lineno);

  /* Don't emit sequences of stabs for the same line.  */
  if (prev_line_file != NULL
      && filename_cmp (file, prev_line_file) == 0)
    {
      if (lineno == prev_lineno)
	/* Same file/line as last time.  */
	return;
    }
  else
    {
      /* Remember file/line for next time.  */
      free (prev_line_file);
      prev_line_file = xstrdup (file);
    }

  prev_lineno = lineno;

  /* Let the world know that we are in the middle of generating a
     piece of stabs line debugging information.  */
  outputting_stabs_line_debug = 1;

  generate_asm_file (N_SOL, file);

  sprintf (sym, "%sL%d", FAKE_LABEL_NAME, line_label_count);
  ++line_label_count;

  if (current_function_label)
    {
      buf = XNEWVEC (char, 100 + strlen (current_function_label));
      sprintf (buf, "%d,0,%d,%s-%s\n", N_SLINE, lineno,
	       sym, current_function_label);
    }
  else
    {
      buf = XNEWVEC (char, 100);
      sprintf (buf, "%d,0,%d,%s\n", N_SLINE, lineno, sym);
    }

  temp_ilp (buf);
  s_stab ('n');
  restore_ilp ();

  colon (sym);

  outputting_stabs_line_debug = 0;
  free (buf);
}

/* Emit a function stab.
   All assembler functions are assumed to have return type `void'.  */

void
stabs_generate_asm_func (const char *funcname, const char *startlabname)
{
  char *buf;
  unsigned int lineno;

  if (! void_emitted_p)
    {
      temp_ilp ((char *) "\"void:t1=1\",128,0,0,0");
      s_stab ('s');
      restore_ilp ();
      void_emitted_p = true;
    }

  as_where (&lineno);
  if (asprintf (&buf, "\"%s:F1\",%d,0,%d,%s",
		funcname, N_FUN, lineno + 1, startlabname) == -1)
    as_fatal ("%s", xstrerror (errno));

  temp_ilp (buf);
  s_stab ('s');
  restore_ilp ();
  free (buf);

  free ((char *) current_function_label);
  current_function_label = xstrdup (startlabname);
}

/* Emit a stab to record the end of a function.  */

void
stabs_generate_asm_endfunc (const char *funcname ATTRIBUTE_UNUSED,
			    const char *startlabname)
{
  char *buf;
  char sym[30];

  sprintf (sym, "%sendfunc%d", FAKE_LABEL_NAME, endfunc_label_count);
  ++endfunc_label_count;
  colon (sym);

  if (asprintf (&buf, "\"\",%d,0,0,%s-%s", N_FUN, sym, startlabname) == -1)
    as_fatal ("%s", xstrerror (errno));

  temp_ilp (buf);
  s_stab ('s');
  restore_ilp ();
  free (buf);

  free ((char *) current_function_label);
  current_function_label = NULL;
}

void
stabs_begin (void)
{
  current_function_label = NULL;
  cached_sec = NULL;
  last_asm_file = NULL;
  file_label_count = 0;
  line_label_count = 0;
  prev_lineno = -1u;
  prev_line_file = NULL;
  void_emitted_p = false;
  endfunc_label_count = 0;
}

void
stabs_end (void)
{
  free ((char *) current_function_label);
  free (last_asm_file);
  free (prev_line_file);
}
