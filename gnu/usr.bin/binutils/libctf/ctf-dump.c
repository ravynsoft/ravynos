/* Textual dumping of CTF data.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <string.h>

#define str_append(s, a) ctf_str_append_noerr (s, a)

/* One item to be dumped, in string form.  */

typedef struct ctf_dump_item
{
  ctf_list_t cdi_list;
  char *cdi_item;
} ctf_dump_item_t;

/* Cross-call state for dumping.  Basically just enough to track the section in
   use and a list of return strings.  */

struct ctf_dump_state
{
  ctf_sect_names_t cds_sect;
  ctf_dict_t *cds_fp;
  ctf_dump_item_t *cds_current;
  ctf_list_t cds_items;
};

/* Cross-call state for ctf_dump_member. */

typedef struct ctf_dump_membstate
{
  char **cdm_str;
  ctf_dict_t *cdm_fp;
  const char *cdm_toplevel_indent;
} ctf_dump_membstate_t;

static int
ctf_dump_append (ctf_dump_state_t *state, char *str)
{
  ctf_dump_item_t *cdi;

  if ((cdi = malloc (sizeof (struct ctf_dump_item))) == NULL)
    return (ctf_set_errno (state->cds_fp, ENOMEM));

  cdi->cdi_item = str;
  ctf_list_append (&state->cds_items, cdi);
  return 0;
}

static void
ctf_dump_free (ctf_dump_state_t *state)
{
  ctf_dump_item_t *cdi, *next_cdi;

  if (state == NULL)
    return;

  for (cdi = ctf_list_next (&state->cds_items); cdi != NULL;
       cdi = next_cdi)
    {
      free (cdi->cdi_item);
      next_cdi = ctf_list_next (cdi);
      free (cdi);
    }
}

/* Return a dump for a single type, without member info: but do optionally show
   the type's references.  */

#define CTF_FT_REFS     0x2 	/* Print referenced types.  */
#define CTF_FT_BITFIELD 0x4	/* Print :BITS if a bitfield.  */
#define CTF_FT_ID       0x8	/* Print "ID: " in front of type IDs.  */

static char *
ctf_dump_format_type (ctf_dict_t *fp, ctf_id_t id, int flag)
{
  ctf_id_t new_id;
  char *str = NULL, *bit = NULL, *buf = NULL;

  ctf_set_errno (fp, 0);
  new_id = id;
  do
    {
      ctf_encoding_t ep;
      ctf_arinfo_t ar;
      int kind, unsliced_kind;
      ssize_t size, align;
      const char *nonroot_leader = "";
      const char *nonroot_trailer = "";
      const char *idstr = "";

      id = new_id;
      if (flag == CTF_ADD_NONROOT)
	{
	  nonroot_leader = "{";
	  nonroot_trailer = "}";
	}

      buf = ctf_type_aname (fp, id);
      if (!buf)
	{
	  if (id == 0 || ctf_errno (fp) == ECTF_NONREPRESENTABLE)
	    {
	      ctf_set_errno (fp, ECTF_NONREPRESENTABLE);
	      str = str_append (str, " (type not represented in CTF)");
	      return str;
	    }

	  goto err;
	}

      if (flag & CTF_FT_ID)
	idstr = "ID ";
      if (asprintf (&bit, "%s%s0x%lx: (kind %i) ", nonroot_leader, idstr,
		    id, ctf_type_kind (fp, id)) < 0)
	goto oom;
      str = str_append (str, bit);
      free (bit);
      bit = NULL;

      if (buf[0] != '\0')
	str = str_append (str, buf);

      free (buf);
      buf = NULL;

      unsliced_kind = ctf_type_kind_unsliced (fp, id);
      kind = ctf_type_kind (fp, id);

      /* Report encodings of everything with an encoding other than enums:
	 base-type enums cannot have a nonzero cte_offset or cte_bits value.
	 (Slices of them can, but they are of kind CTF_K_SLICE.)  */
      if (unsliced_kind != CTF_K_ENUM && ctf_type_encoding (fp, id, &ep) == 0)
	{
	  if ((ssize_t) ep.cte_bits != ctf_type_size (fp, id) * CHAR_BIT
	      && flag & CTF_FT_BITFIELD)
	    {
	      if (asprintf (&bit, ":%i", ep.cte_bits) < 0)
		goto oom;
	      str = str_append (str, bit);
	      free (bit);
	      bit = NULL;
	    }

	  if ((ssize_t) ep.cte_bits != ctf_type_size (fp, id) * CHAR_BIT
	      || ep.cte_offset != 0)
	    {
	      const char *slice = "";

	      if (unsliced_kind == CTF_K_SLICE)
		slice = "slice ";

	      if (asprintf (&bit, " [%s0x%x:0x%x]",
			    slice, ep.cte_offset, ep.cte_bits) < 0)
		goto oom;
	      str = str_append (str, bit);
	      free (bit);
	      bit = NULL;
	    }

	  if (asprintf (&bit, " (format 0x%x)", ep.cte_format) < 0)
	    goto oom;
	  str = str_append (str, bit);
	  free (bit);
	  bit = NULL;
	}

      size = ctf_type_size (fp, id);
      if (kind != CTF_K_FUNCTION && size >= 0)
	{
	  if (asprintf (&bit, " (size 0x%lx)", (unsigned long int) size) < 0)
	    goto oom;

	  str = str_append (str, bit);
	  free (bit);
	  bit = NULL;
	}

      align = ctf_type_align (fp, id);
      if (align >= 0)
	{
	  if (asprintf (&bit, " (aligned at 0x%lx)",
			(unsigned long int) align) < 0)
	    goto oom;

	  str = str_append (str, bit);
	  free (bit);
	  bit = NULL;
	}

      if (nonroot_trailer[0] != 0)
	str = str_append (str, nonroot_trailer);

      /* Just exit after one iteration if we are not showing the types this type
	 references.  */
      if (!(flag & CTF_FT_REFS))
	return str;

      /* Keep going as long as this type references another.  We consider arrays
	 to "reference" their element type. */

      if (kind == CTF_K_ARRAY)
	{
	  if (ctf_array_info (fp, id, &ar) < 0)
	    goto err;
	  new_id = ar.ctr_contents;
	}
      else
	new_id = ctf_type_reference (fp, id);
      if (new_id != CTF_ERR)
	str = str_append (str, " -> ");
    }
  while (new_id != CTF_ERR);

  if (ctf_errno (fp) != ECTF_NOTREF)
    {
      free (str);
      return NULL;
    }

  return str;

 oom:
  ctf_set_errno (fp, errno);
 err:
  ctf_err_warn (fp, 1, 0, _("cannot format name dumping type 0x%lx"), id);
  free (buf);
  free (str);
  free (bit);
  return NULL;
}

/* Dump one string field from the file header into the cds_items.  */
static int
ctf_dump_header_strfield (ctf_dict_t *fp, ctf_dump_state_t *state,
			  const char *name, uint32_t value)
{
  char *str;
  if (value)
    {
      if (asprintf (&str, "%s: %s\n", name, ctf_strptr (fp, value)) < 0)
	goto err;
      ctf_dump_append (state, str);
    }
  return 0;

 err:
  return (ctf_set_errno (fp, errno));
}

/* Dump one section-offset field from the file header into the cds_items.  */
static int
ctf_dump_header_sectfield (ctf_dict_t *fp, ctf_dump_state_t *state,
			   const char *sect, uint32_t off, uint32_t nextoff)
{
  char *str;
  if (nextoff - off)
    {
      if (asprintf (&str, "%s:\t0x%lx -- 0x%lx (0x%lx bytes)\n", sect,
		    (unsigned long) off, (unsigned long) (nextoff - 1),
		    (unsigned long) (nextoff - off)) < 0)
	goto err;
      ctf_dump_append (state, str);
    }
  return 0;

 err:
  return (ctf_set_errno (fp, errno));
}

/* Dump the file header into the cds_items.  */
static int
ctf_dump_header (ctf_dict_t *fp, ctf_dump_state_t *state)
{
  char *str;
  char *flagstr = NULL;
  const ctf_header_t *hp = fp->ctf_header;
  const char *vertab[] =
    {
     NULL, "CTF_VERSION_1",
     "CTF_VERSION_1_UPGRADED_3 (latest format, version 1 type "
     "boundaries)",
     "CTF_VERSION_2",
     "CTF_VERSION_3", NULL
    };
  const char *verstr = NULL;

  if (asprintf (&str, "Magic number: 0x%x\n", hp->cth_magic) < 0)
      goto err;
  ctf_dump_append (state, str);

  if (hp->cth_version <= CTF_VERSION)
    verstr = vertab[hp->cth_version];

  if (verstr == NULL)
    verstr = "(not a valid version)";

  if (asprintf (&str, "Version: %i (%s)\n", hp->cth_version,
		verstr) < 0)
    goto err;
  ctf_dump_append (state, str);

  /* Everything else is only printed if present.  */

  /* The flags are unusual in that they represent the ctf_dict_t *in memory*:
     flags representing compression, etc, are turned off as the file is
     decompressed.  So we store a copy of the flags before they are changed, for
     the dumper.  */

  if (fp->ctf_openflags > 0)
    {
      if (asprintf (&flagstr, "%s%s%s%s%s%s%s",
		    fp->ctf_openflags & CTF_F_COMPRESS
		    ? "CTF_F_COMPRESS": "",
		    (fp->ctf_openflags & CTF_F_COMPRESS)
		    && (fp->ctf_openflags & ~CTF_F_COMPRESS)
		    ? ", " : "",
		    fp->ctf_openflags & CTF_F_NEWFUNCINFO
		    ? "CTF_F_NEWFUNCINFO" : "",
		    (fp->ctf_openflags & (CTF_F_COMPRESS | CTF_F_NEWFUNCINFO))
		    && (fp->ctf_openflags & ~(CTF_F_COMPRESS | CTF_F_NEWFUNCINFO))
		    ? ", " : "",
		    fp->ctf_openflags & CTF_F_IDXSORTED
		    ? "CTF_F_IDXSORTED" : "",
		    fp->ctf_openflags & (CTF_F_COMPRESS | CTF_F_NEWFUNCINFO
					 | CTF_F_IDXSORTED)
		    && (fp->ctf_openflags & ~(CTF_F_COMPRESS | CTF_F_NEWFUNCINFO
					      | CTF_F_IDXSORTED))
		    ? ", " : "",
		    fp->ctf_openflags & CTF_F_DYNSTR
		    ? "CTF_F_DYNSTR" : "") < 0)
	goto err;

      if (asprintf (&str, "Flags: 0x%x (%s)", fp->ctf_openflags, flagstr) < 0)
	goto err;
      ctf_dump_append (state, str);
    }

  if (ctf_dump_header_strfield (fp, state, "Parent label",
				hp->cth_parlabel) < 0)
    goto err;

  if (ctf_dump_header_strfield (fp, state, "Parent name", hp->cth_parname) < 0)
    goto err;

  if (ctf_dump_header_strfield (fp, state, "Compilation unit name",
				hp->cth_cuname) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Label section", hp->cth_lbloff,
				 hp->cth_objtoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Data object section",
				 hp->cth_objtoff, hp->cth_funcoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Function info section",
				 hp->cth_funcoff, hp->cth_objtidxoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Object index section",
				 hp->cth_objtidxoff, hp->cth_funcidxoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Function index section",
				 hp->cth_funcidxoff, hp->cth_varoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Variable section",
				 hp->cth_varoff, hp->cth_typeoff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "Type section",
				 hp->cth_typeoff, hp->cth_stroff) < 0)
    goto err;

  if (ctf_dump_header_sectfield (fp, state, "String section", hp->cth_stroff,
				 hp->cth_stroff + hp->cth_strlen + 1) < 0)
    goto err;

  return 0;
 err:
  free (flagstr);
  return (ctf_set_errno (fp, errno));
}

/* Dump a single label into the cds_items.  */

static int
ctf_dump_label (const char *name, const ctf_lblinfo_t *info,
		void *arg)
{
  char *str;
  char *typestr;
  ctf_dump_state_t *state = arg;

  if (asprintf (&str, "%s -> ", name) < 0)
    return (ctf_set_errno (state->cds_fp, errno));

  if ((typestr = ctf_dump_format_type (state->cds_fp, info->ctb_type,
				       CTF_ADD_ROOT | CTF_FT_REFS)) == NULL)
    {
      free (str);
      return 0;				/* Swallow the error.  */
    }

  str = str_append (str, typestr);
  free (typestr);

  ctf_dump_append (state, str);
  return 0;
}

/* Dump all the object or function entries into the cds_items.  */

static int
ctf_dump_objts (ctf_dict_t *fp, ctf_dump_state_t *state, int functions)
{
  const char *name;
  ctf_id_t id;
  ctf_next_t *i = NULL;
  char *str = NULL;

  if ((functions && fp->ctf_funcidx_names)
      || (!functions && fp->ctf_objtidx_names))
    str = str_append (str, _("Section is indexed.\n"));
  else if (fp->ctf_symtab.cts_data == NULL)
    str = str_append (str, _("No symbol table.\n"));

  while ((id = ctf_symbol_next (fp, &i, &name, functions)) != CTF_ERR)
    {
      char *typestr = NULL;

      /* Emit the name, if we know it.  No trailing space: ctf_dump_format_type
	 has a leading one.   */
      if (name)
	{
	  if (asprintf (&str, "%s -> ", name) < 0)
	    goto oom;
	}
      else
	str = xstrdup ("");

      if ((typestr = ctf_dump_format_type (state->cds_fp, id,
					   CTF_ADD_ROOT | CTF_FT_REFS)) == NULL)
	{
	  ctf_dump_append (state, str);
	  continue;				/* Swallow the error.  */
	}

      str = str_append (str, typestr);
      free (typestr);
      ctf_dump_append (state, str);
      continue;

    oom:
      ctf_set_errno (fp, ENOMEM);
      ctf_next_destroy (i);
      return -1;
    }
  return 0;
}

/* Dump a single variable into the cds_items.  */
static int
ctf_dump_var (const char *name, ctf_id_t type, void *arg)
{
  char *str;
  char *typestr;
  ctf_dump_state_t *state = arg;

  if (asprintf (&str, "%s -> ", name) < 0)
    return (ctf_set_errno (state->cds_fp, errno));

  if ((typestr = ctf_dump_format_type (state->cds_fp, type,
				       CTF_ADD_ROOT | CTF_FT_REFS)) == NULL)
    {
      free (str);
      return 0;			/* Swallow the error.  */
    }

  str = str_append (str, typestr);
  free (typestr);

  ctf_dump_append (state, str);
  return 0;
}

/* Dump a single struct/union member into the string in the membstate.  */
static int
ctf_dump_member (const char *name, ctf_id_t id, unsigned long offset,
		 int depth, void *arg)
{
  ctf_dump_membstate_t *state = arg;
  char *typestr = NULL;
  char *bit = NULL;

  /* The struct/union itself has already been printed.  */
  if (depth == 0)
    return 0;

  if (asprintf (&bit, "%s%*s", state->cdm_toplevel_indent, (depth-1)*4, "") < 0)
    goto oom;
  *state->cdm_str = str_append (*state->cdm_str, bit);
  free (bit);

  if ((typestr = ctf_dump_format_type (state->cdm_fp, id,
				       CTF_ADD_ROOT | CTF_FT_BITFIELD
				       | CTF_FT_ID)) == NULL)
    return -1;				/* errno is set for us.  */

  if (asprintf (&bit, "[0x%lx] %s: %s\n", offset, name, typestr) < 0)
    goto oom;

  *state->cdm_str = str_append (*state->cdm_str, bit);
  free (typestr);
  free (bit);
  typestr = NULL;
  bit = NULL;

  return 0;

 oom:
  free (typestr);
  free (bit);
  return (ctf_set_errno (state->cdm_fp, errno));
}

/* Report the number of digits in the hexadecimal representation of a type
   ID.  */

static int
type_hex_digits (ctf_id_t id)
{
  int i = 0;

  if (id == 0)
    return 1;

  for (; id > 0; id >>= 4, i++);
  return i;
}

/* Dump a single type into the cds_items.  */
static int
ctf_dump_type (ctf_id_t id, int flag, void *arg)
{
  char *str;
  char *indent;
  ctf_dump_state_t *state = arg;
  ctf_dump_membstate_t membstate = { &str, state->cds_fp, NULL };

  /* Indent neatly.  */
  if (asprintf (&indent, "    %*s", type_hex_digits (id), "") < 0)
    return (ctf_set_errno (state->cds_fp, ENOMEM));

  /* Dump the type itself.  */
  if ((str = ctf_dump_format_type (state->cds_fp, id,
				   flag | CTF_FT_REFS)) == NULL)
    goto err;
  str = str_append (str, "\n");

  membstate.cdm_toplevel_indent = indent;

  /* Member dumping for structs, unions...  */
  if (ctf_type_kind (state->cds_fp, id) == CTF_K_STRUCT
      || ctf_type_kind (state->cds_fp, id) == CTF_K_UNION)
    {
      if ((ctf_type_visit (state->cds_fp, id, ctf_dump_member, &membstate)) < 0)
	{
	  if (id == 0 || ctf_errno (state->cds_fp) == ECTF_NONREPRESENTABLE)
	    {
	      ctf_dump_append (state, str);
	      return 0;
	    }
	  ctf_err_warn (state->cds_fp, 1, ctf_errno (state->cds_fp),
			_("cannot visit members dumping type 0x%lx"), id);
	  goto err;
	}
    }

  /* ... and enums, for which we dump the first and last few members and skip
     the ones in the middle.  */
  if (ctf_type_kind (state->cds_fp, id) == CTF_K_ENUM)
    {
      int enum_count = ctf_member_count (state->cds_fp, id);
      ctf_next_t *it = NULL;
      int i = 0;
      const char *enumerand;
      char *bit;
      int value;

      while ((enumerand = ctf_enum_next (state->cds_fp, id,
					 &it, &value)) != NULL)
	{
	  i++;
	  if ((i > 5) && (i < enum_count - 4))
	    continue;

	  str = str_append (str, indent);

	  if (asprintf (&bit, "%s: %i\n", enumerand, value) < 0)
	    {
	      ctf_next_destroy (it);
	      goto oom;
	    }
	  str = str_append (str, bit);
	  free (bit);

	  if ((i == 5) && (enum_count > 10))
	    {
	      str = str_append (str, indent);
	      str = str_append (str, "...\n");
	    }
	}
      if (ctf_errno (state->cds_fp) != ECTF_NEXT_END)
	{
	  ctf_err_warn (state->cds_fp, 1, ctf_errno (state->cds_fp),
			_("cannot visit enumerands dumping type 0x%lx"), id);
	  goto err;
	}
    }

  ctf_dump_append (state, str);
  free (indent);

  return 0;

 err:
  free (indent);
  free (str);

  /* Swallow the error: don't cause an error in one type to abort all
     type dumping.  */
  return 0;

 oom:
  free (indent);
  free (str);
  return ctf_set_errno (state->cds_fp, ENOMEM);
}

/* Dump the string table into the cds_items.  */

static int
ctf_dump_str (ctf_dict_t *fp, ctf_dump_state_t *state)
{
  const char *s = fp->ctf_str[CTF_STRTAB_0].cts_strs;

  for (; s < fp->ctf_str[CTF_STRTAB_0].cts_strs +
	 fp->ctf_str[CTF_STRTAB_0].cts_len;)
    {
      char *str;
      if (asprintf (&str, "0x%lx: %s",
		    (unsigned long) (s - fp->ctf_str[CTF_STRTAB_0].cts_strs),
		    s) < 0)
	return (ctf_set_errno (fp, errno));
      ctf_dump_append (state, str);
      s += strlen (s) + 1;
    }

  return 0;
}

/* Dump a particular section of a CTF file, in textual form.  Call with a
   pointer to a NULL STATE: each call emits a dynamically allocated string
   containing a description of one entity in the specified section, in order.
   Only the first call (with a NULL state) may vary SECT.  Once the CTF section
   has been entirely dumped, the call returns NULL and frees and annuls the
   STATE, ready for another section to be dumped.  The returned textual content
   may span multiple lines: between each call the FUNC is called with one
   textual line at a time, and should return a suitably decorated line (it can
   allocate a new one and return it if it likes).  */

char *
ctf_dump (ctf_dict_t *fp, ctf_dump_state_t **statep, ctf_sect_names_t sect,
	  ctf_dump_decorate_f *func, void *arg)
{
  char *str;
  char *line;
  ctf_dump_state_t *state = NULL;

  if (*statep == NULL)
    {
      /* Data collection.  Transforming a call-at-a-time iterator into a
	 return-at-a-time iterator in a language without call/cc is annoying. It
	 is easiest to simply collect everything at once and then return it bit
	 by bit.  The first call will take (much) longer than otherwise, but the
	 amortized time needed is the same.  */

      if ((*statep = malloc (sizeof (struct ctf_dump_state))) == NULL)
	{
	  ctf_set_errno (fp, ENOMEM);
	  goto end;
	}
      state = *statep;

      memset (state, 0, sizeof (struct ctf_dump_state));
      state->cds_fp = fp;
      state->cds_sect = sect;

      switch (sect)
	{
	case CTF_SECT_HEADER:
	  ctf_dump_header (fp, state);
	  break;
	case CTF_SECT_LABEL:
	  if (ctf_label_iter (fp, ctf_dump_label, state) < 0)
	    {
	      if (ctf_errno (fp) != ECTF_NOLABELDATA)
		goto end;		/* errno is set for us.  */
	      ctf_set_errno (fp, 0);
	    }
	  break;
	case CTF_SECT_OBJT:
	  if (ctf_dump_objts (fp, state, 0) < 0)
	    goto end;			/* errno is set for us.  */
	  break;
	case CTF_SECT_FUNC:
	  if (ctf_dump_objts (fp, state, 1) < 0)
	    goto end;			/* errno is set for us.  */
	  break;
	case CTF_SECT_VAR:
	  if (ctf_variable_iter (fp, ctf_dump_var, state) < 0)
	    goto end;			/* errno is set for us.  */
	  break;
	case CTF_SECT_TYPE:
	  if (ctf_type_iter_all (fp, ctf_dump_type, state) < 0)
	    goto end;			/* errno is set for us.  */
	  break;
	case CTF_SECT_STR:
	  ctf_dump_str (fp, state);
	  break;
	default:
	  ctf_set_errno (fp, ECTF_DUMPSECTUNKNOWN);
	  goto end;
	}
    }
  else
    {
      state = *statep;

      if (state->cds_sect != sect)
	{
	  ctf_set_errno (fp, ECTF_DUMPSECTCHANGED);
	  goto end;
	}
    }

  if (state->cds_current == NULL)
    state->cds_current = ctf_list_next (&state->cds_items);
  else
    state->cds_current = ctf_list_next (state->cds_current);

  if (state->cds_current == NULL)
    goto end;

  /* Hookery.  There is some extra complexity to preserve linefeeds within each
     item while removing linefeeds at the end.  */
  if (func)
    {
      size_t len;

      str = NULL;
      for (line = state->cds_current->cdi_item; line && *line; )
	{
	  char *nline = line;
	  char *ret;

	  nline = strchr (line, '\n');
	  if (nline)
	    nline[0] = '\0';

	  ret = func (sect, line, arg);
	  str = str_append (str, ret);
	  str = str_append (str, "\n");
	  if (ret != line)
	    free (ret);

	  if (nline)
	    {
	      nline[0] = '\n';
	      nline++;
	    }

	  line = nline;
	}

      len = strlen (str);

      if (str[len-1] == '\n')
	str[len-1] = '\0';
    }
  else
    {
      str = strdup (state->cds_current->cdi_item);
      if (!str)
	{
	  ctf_set_errno (fp, ENOMEM);
	  return str;
	}
    }

  ctf_set_errno (fp, 0);
  return str;

 end:
  ctf_dump_free (state);
  free (state);
  ctf_set_errno (fp, 0);
  *statep = NULL;
  return NULL;
}
