/* Opening CTF files.
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
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <elf.h>
#include "swap.h"
#include <bfd.h>
#include <zlib.h>

static const ctf_dmodel_t _libctf_models[] = {
  {"ILP32", CTF_MODEL_ILP32, 4, 1, 2, 4, 4},
  {"LP64", CTF_MODEL_LP64, 8, 1, 2, 4, 8},
  {NULL, 0, 0, 0, 0, 0, 0}
};

const char _CTF_SECTION[] = ".ctf";
const char _CTF_NULLSTR[] = "";

/* Version-sensitive accessors.  */

static uint32_t
get_kind_v1 (uint32_t info)
{
  return (CTF_V1_INFO_KIND (info));
}

static uint32_t
get_root_v1 (uint32_t info)
{
  return (CTF_V1_INFO_ISROOT (info));
}

static uint32_t
get_vlen_v1 (uint32_t info)
{
  return (CTF_V1_INFO_VLEN (info));
}

static uint32_t
get_kind_v2 (uint32_t info)
{
  return (CTF_V2_INFO_KIND (info));
}

static uint32_t
get_root_v2 (uint32_t info)
{
  return (CTF_V2_INFO_ISROOT (info));
}

static uint32_t
get_vlen_v2 (uint32_t info)
{
  return (CTF_V2_INFO_VLEN (info));
}

static inline ssize_t
get_ctt_size_common (const ctf_dict_t *fp _libctf_unused_,
		     const ctf_type_t *tp _libctf_unused_,
		     ssize_t *sizep, ssize_t *incrementp, size_t lsize,
		     size_t csize, size_t ctf_type_size,
		     size_t ctf_stype_size, size_t ctf_lsize_sent)
{
  ssize_t size, increment;

  if (csize == ctf_lsize_sent)
    {
      size = lsize;
      increment = ctf_type_size;
    }
  else
    {
      size = csize;
      increment = ctf_stype_size;
    }

  if (sizep)
    *sizep = size;
  if (incrementp)
    *incrementp = increment;

  return size;
}

static ssize_t
get_ctt_size_v1 (const ctf_dict_t *fp, const ctf_type_t *tp,
		 ssize_t *sizep, ssize_t *incrementp)
{
  ctf_type_v1_t *t1p = (ctf_type_v1_t *) tp;

  return (get_ctt_size_common (fp, tp, sizep, incrementp,
			       CTF_TYPE_LSIZE (t1p), t1p->ctt_size,
			       sizeof (ctf_type_v1_t), sizeof (ctf_stype_v1_t),
			       CTF_LSIZE_SENT_V1));
}

/* Return the size that a v1 will be once it is converted to v2.  */

static ssize_t
get_ctt_size_v2_unconverted (const ctf_dict_t *fp, const ctf_type_t *tp,
			     ssize_t *sizep, ssize_t *incrementp)
{
  ctf_type_v1_t *t1p = (ctf_type_v1_t *) tp;

  return (get_ctt_size_common (fp, tp, sizep, incrementp,
			       CTF_TYPE_LSIZE (t1p), t1p->ctt_size,
			       sizeof (ctf_type_t), sizeof (ctf_stype_t),
			       CTF_LSIZE_SENT));
}

static ssize_t
get_ctt_size_v2 (const ctf_dict_t *fp, const ctf_type_t *tp,
		 ssize_t *sizep, ssize_t *incrementp)
{
  return (get_ctt_size_common (fp, tp, sizep, incrementp,
			       CTF_TYPE_LSIZE (tp), tp->ctt_size,
			       sizeof (ctf_type_t), sizeof (ctf_stype_t),
			       CTF_LSIZE_SENT));
}

static ssize_t
get_vbytes_common (ctf_dict_t *fp, unsigned short kind,
		   ssize_t size _libctf_unused_, size_t vlen)
{
  switch (kind)
    {
    case CTF_K_INTEGER:
    case CTF_K_FLOAT:
      return (sizeof (uint32_t));
    case CTF_K_SLICE:
      return (sizeof (ctf_slice_t));
    case CTF_K_ENUM:
      return (sizeof (ctf_enum_t) * vlen);
    case CTF_K_FORWARD:
    case CTF_K_UNKNOWN:
    case CTF_K_POINTER:
    case CTF_K_TYPEDEF:
    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
      return 0;
    default:
      ctf_set_errno (fp, ECTF_CORRUPT);
      ctf_err_warn (fp, 0, 0, _("detected invalid CTF kind: %x"), kind);
      return -1;
    }
}

static ssize_t
get_vbytes_v1 (ctf_dict_t *fp, unsigned short kind, ssize_t size, size_t vlen)
{
  switch (kind)
    {
    case CTF_K_ARRAY:
      return (sizeof (ctf_array_v1_t));
    case CTF_K_FUNCTION:
      return (sizeof (unsigned short) * (vlen + (vlen & 1)));
    case CTF_K_STRUCT:
    case CTF_K_UNION:
      if (size < CTF_LSTRUCT_THRESH_V1)
	return (sizeof (ctf_member_v1_t) * vlen);
      else
	return (sizeof (ctf_lmember_v1_t) * vlen);
    }

  return (get_vbytes_common (fp, kind, size, vlen));
}

static ssize_t
get_vbytes_v2 (ctf_dict_t *fp, unsigned short kind, ssize_t size, size_t vlen)
{
  switch (kind)
    {
    case CTF_K_ARRAY:
      return (sizeof (ctf_array_t));
    case CTF_K_FUNCTION:
      return (sizeof (uint32_t) * (vlen + (vlen & 1)));
    case CTF_K_STRUCT:
    case CTF_K_UNION:
      if (size < CTF_LSTRUCT_THRESH)
	return (sizeof (ctf_member_t) * vlen);
      else
	return (sizeof (ctf_lmember_t) * vlen);
    }

  return (get_vbytes_common (fp, kind, size, vlen));
}

static const ctf_dictops_t ctf_dictops[] = {
  {NULL, NULL, NULL, NULL, NULL},
  /* CTF_VERSION_1 */
  {get_kind_v1, get_root_v1, get_vlen_v1, get_ctt_size_v1, get_vbytes_v1},
  /* CTF_VERSION_1_UPGRADED_3 */
  {get_kind_v2, get_root_v2, get_vlen_v2, get_ctt_size_v2, get_vbytes_v2},
  /* CTF_VERSION_2 */
  {get_kind_v2, get_root_v2, get_vlen_v2, get_ctt_size_v2, get_vbytes_v2},
  /* CTF_VERSION_3, identical to 2: only new type kinds */
  {get_kind_v2, get_root_v2, get_vlen_v2, get_ctt_size_v2, get_vbytes_v2},
};

/* Initialize the symtab translation table as appropriate for its indexing
   state.  For unindexed symtypetabs, fill each entry with the offset of the CTF
   type or function data corresponding to each STT_FUNC or STT_OBJECT entry in
   the symbol table.  For indexed symtypetabs, do nothing: the needed
   initialization for indexed lookups may be quite expensive, so it is done only
   as needed, when lookups happen.  (In particular, the majority of indexed
   symtypetabs come from the compiler, and all the linker does is iteration over
   all entries, which doesn't need this initialization.)

   The SP symbol table section may be NULL if there is no symtab.

   If init_symtab works on one call, it cannot fail on future calls to the same
   fp: ctf_symsect_endianness relies on this.  */

static int
init_symtab (ctf_dict_t *fp, const ctf_header_t *hp, const ctf_sect_t *sp)
{
  const unsigned char *symp;
  int skip_func_info = 0;
  int i;
  uint32_t *xp = fp->ctf_sxlate;
  uint32_t *xend = PTR_ADD (xp, fp->ctf_nsyms);

  uint32_t objtoff = hp->cth_objtoff;
  uint32_t funcoff = hp->cth_funcoff;

  /* If the CTF_F_NEWFUNCINFO flag is not set, pretend the func info section
     is empty: this compiler is too old to emit a function info section we
     understand.  */

  if (!(hp->cth_flags & CTF_F_NEWFUNCINFO))
    skip_func_info = 1;

  if (hp->cth_objtidxoff < hp->cth_funcidxoff)
    fp->ctf_objtidx_names = (uint32_t *) (fp->ctf_buf + hp->cth_objtidxoff);
  if (hp->cth_funcidxoff < hp->cth_varoff && !skip_func_info)
    fp->ctf_funcidx_names = (uint32_t *) (fp->ctf_buf + hp->cth_funcidxoff);

  /* Don't bother doing the rest if everything is indexed, or if we don't have a
     symbol table: we will never use it.  */
  if ((fp->ctf_objtidx_names && fp->ctf_funcidx_names) || !sp || !sp->cts_data)
    return 0;

  /* The CTF data object and function type sections are ordered to match the
     relative order of the respective symbol types in the symtab, unless there
     is an index section, in which case the order is arbitrary and the index
     gives the mapping.  If no type information is available for a symbol table
     entry, a pad is inserted in the CTF section.  As a further optimization,
     anonymous or undefined symbols are omitted from the CTF data.  If an
     index is available for function symbols but not object symbols, or vice
     versa, we populate the xslate table for the unindexed symbols only.  */

  for (i = 0, symp = sp->cts_data; xp < xend; xp++, symp += sp->cts_entsize,
	 i++)
    {
      ctf_link_sym_t sym;

      switch (sp->cts_entsize)
	{
	case sizeof (Elf64_Sym):
	  {
	    const Elf64_Sym *symp64 = (Elf64_Sym *) (uintptr_t) symp;
	    ctf_elf64_to_link_sym (fp, &sym, symp64, i);
	  }
	  break;
	case sizeof (Elf32_Sym):
	  {
	    const Elf32_Sym *symp32 = (Elf32_Sym *) (uintptr_t) symp;
	    ctf_elf32_to_link_sym (fp, &sym, symp32, i);
	  }
	  break;
	default:
	  return ECTF_SYMTAB;
	}

      /* This call may be led astray if our idea of the symtab's endianness is
	 wrong, but when this is fixed by a call to ctf_symsect_endianness,
	 init_symtab will be called again with the right endianness in
	 force.  */
      if (ctf_symtab_skippable (&sym))
	{
	  *xp = -1u;
	  continue;
	}

      switch (sym.st_type)
	{
	case STT_OBJECT:
	  if (fp->ctf_objtidx_names || objtoff >= hp->cth_funcoff)
	    {
	      *xp = -1u;
	      break;
	    }

	  *xp = objtoff;
	  objtoff += sizeof (uint32_t);
	  break;

	case STT_FUNC:
	  if (fp->ctf_funcidx_names || funcoff >= hp->cth_objtidxoff
	      || skip_func_info)
	    {
	      *xp = -1u;
	      break;
	    }

	  *xp = funcoff;
	  funcoff += sizeof (uint32_t);
	  break;

	default:
	  *xp = -1u;
	  break;
	}
    }

  ctf_dprintf ("loaded %lu symtab entries\n", fp->ctf_nsyms);
  return 0;
}

/* Reset the CTF base pointer and derive the buf pointer from it, initializing
   everything in the ctf_dict that depends on the base or buf pointers.

   The original gap between the buf and base pointers, if any -- the original,
   unconverted CTF header -- is kept, but its contents are not specified and are
   never used.  */

static void
ctf_set_base (ctf_dict_t *fp, const ctf_header_t *hp, unsigned char *base)
{
  fp->ctf_buf = base + (fp->ctf_buf - fp->ctf_base);
  fp->ctf_base = base;
  fp->ctf_vars = (ctf_varent_t *) ((const char *) fp->ctf_buf +
				   hp->cth_varoff);
  fp->ctf_nvars = (hp->cth_typeoff - hp->cth_varoff) / sizeof (ctf_varent_t);

  fp->ctf_str[CTF_STRTAB_0].cts_strs = (const char *) fp->ctf_buf
    + hp->cth_stroff;
  fp->ctf_str[CTF_STRTAB_0].cts_len = hp->cth_strlen;

  /* If we have a parent dict name and label, store the relocated string
     pointers in the CTF dict for easy access later. */

  /* Note: before conversion, these will be set to values that will be
     immediately invalidated by the conversion process, but the conversion
     process will call ctf_set_base() again to fix things up.  */

  if (hp->cth_parlabel != 0)
    fp->ctf_parlabel = ctf_strptr (fp, hp->cth_parlabel);
  if (hp->cth_parname != 0)
    fp->ctf_parname = ctf_strptr (fp, hp->cth_parname);
  if (hp->cth_cuname != 0)
    fp->ctf_cuname = ctf_strptr (fp, hp->cth_cuname);

  if (fp->ctf_cuname)
    ctf_dprintf ("ctf_set_base: CU name %s\n", fp->ctf_cuname);
  if (fp->ctf_parname)
    ctf_dprintf ("ctf_set_base: parent name %s (label %s)\n",
	       fp->ctf_parname,
	       fp->ctf_parlabel ? fp->ctf_parlabel : "<NULL>");
}

/* Set the version of the CTF file. */

/* When this is reset, LCTF_* changes behaviour, but there is no guarantee that
   the variable data list associated with each type has been upgraded: the
   caller must ensure this has been done in advance.  */

static void
ctf_set_version (ctf_dict_t *fp, ctf_header_t *cth, int ctf_version)
{
  fp->ctf_version = ctf_version;
  cth->cth_version = ctf_version;
  fp->ctf_dictops = &ctf_dictops[ctf_version];
}


/* Upgrade the header to CTF_VERSION_3.  The upgrade is done in-place.  */
static void
upgrade_header (ctf_header_t *hp)
{
  ctf_header_v2_t *oldhp = (ctf_header_v2_t *) hp;

  hp->cth_strlen = oldhp->cth_strlen;
  hp->cth_stroff = oldhp->cth_stroff;
  hp->cth_typeoff = oldhp->cth_typeoff;
  hp->cth_varoff = oldhp->cth_varoff;
  hp->cth_funcidxoff = hp->cth_varoff;		/* No index sections.  */
  hp->cth_objtidxoff = hp->cth_funcidxoff;
  hp->cth_funcoff = oldhp->cth_funcoff;
  hp->cth_objtoff = oldhp->cth_objtoff;
  hp->cth_lbloff = oldhp->cth_lbloff;
  hp->cth_cuname = 0;				/* No CU name.  */
}

/* Upgrade the type table to CTF_VERSION_3 (really CTF_VERSION_1_UPGRADED_3)
   from CTF_VERSION_1.

   The upgrade is not done in-place: the ctf_base is moved.  ctf_strptr() must
   not be called before reallocation is complete.

   Sections not checked here due to nonexistence or nonpopulated state in older
   formats: objtidx, funcidx.

   Type kinds not checked here due to nonexistence in older formats:
      CTF_K_SLICE.  */
static int
upgrade_types_v1 (ctf_dict_t *fp, ctf_header_t *cth)
{
  const ctf_type_v1_t *tbuf;
  const ctf_type_v1_t *tend;
  unsigned char *ctf_base, *old_ctf_base = (unsigned char *) fp->ctf_dynbase;
  ctf_type_t *t2buf;

  ssize_t increase = 0, size, increment, v2increment, vbytes, v2bytes;
  const ctf_type_v1_t *tp;
  ctf_type_t *t2p;

  tbuf = (ctf_type_v1_t *) (fp->ctf_buf + cth->cth_typeoff);
  tend = (ctf_type_v1_t *) (fp->ctf_buf + cth->cth_stroff);

  /* Much like init_types(), this is a two-pass process.

     First, figure out the new type-section size needed.  (It is possible,
     in theory, for it to be less than the old size, but this is very
     unlikely.  It cannot be so small that cth_typeoff ends up of negative
     size.  We validate this with an assertion below.)

     We must cater not only for changes in vlen and types sizes but also
     for changes in 'increment', which happen because v2 places some types
     into ctf_stype_t where v1 would be forced to use the larger non-stype.  */

  for (tp = tbuf; tp < tend;
       tp = (ctf_type_v1_t *) ((uintptr_t) tp + increment + vbytes))
    {
      unsigned short kind = CTF_V1_INFO_KIND (tp->ctt_info);
      unsigned long vlen = CTF_V1_INFO_VLEN (tp->ctt_info);

      size = get_ctt_size_v1 (fp, (const ctf_type_t *) tp, NULL, &increment);
      vbytes = get_vbytes_v1 (fp, kind, size, vlen);

      get_ctt_size_v2_unconverted (fp, (const ctf_type_t *) tp, NULL,
				   &v2increment);
      v2bytes = get_vbytes_v2 (fp, kind, size, vlen);

      if ((vbytes < 0) || (size < 0))
	return ECTF_CORRUPT;

      increase += v2increment - increment;	/* May be negative.  */
      increase += v2bytes - vbytes;
    }

  /* Allocate enough room for the new buffer, then copy everything but the type
     section into place, and reset the base accordingly.  Leave the version
     number unchanged, so that LCTF_INFO_* still works on the
     as-yet-untranslated type info.  */

  if ((ctf_base = malloc (fp->ctf_size + increase)) == NULL)
    return ECTF_ZALLOC;

  /* Start at ctf_buf, not ctf_base, to squeeze out the original header: we
     never use it and it is unconverted.  */

  memcpy (ctf_base, fp->ctf_buf, cth->cth_typeoff);
  memcpy (ctf_base + cth->cth_stroff + increase,
	  fp->ctf_buf + cth->cth_stroff, cth->cth_strlen);

  memset (ctf_base + cth->cth_typeoff, 0, cth->cth_stroff - cth->cth_typeoff
	  + increase);

  cth->cth_stroff += increase;
  fp->ctf_size += increase;
  assert (cth->cth_stroff >= cth->cth_typeoff);
  fp->ctf_base = ctf_base;
  fp->ctf_buf = ctf_base;
  fp->ctf_dynbase = ctf_base;
  ctf_set_base (fp, cth, ctf_base);

  t2buf = (ctf_type_t *) (fp->ctf_buf + cth->cth_typeoff);

  /* Iterate through all the types again, upgrading them.

     Everything that hasn't changed can just be outright memcpy()ed.
     Things that have changed need field-by-field consideration.  */

  for (tp = tbuf, t2p = t2buf; tp < tend;
       tp = (ctf_type_v1_t *) ((uintptr_t) tp + increment + vbytes),
       t2p = (ctf_type_t *) ((uintptr_t) t2p + v2increment + v2bytes))
    {
      unsigned short kind = CTF_V1_INFO_KIND (tp->ctt_info);
      int isroot = CTF_V1_INFO_ISROOT (tp->ctt_info);
      unsigned long vlen = CTF_V1_INFO_VLEN (tp->ctt_info);
      ssize_t v2size;
      void *vdata, *v2data;

      size = get_ctt_size_v1 (fp, (const ctf_type_t *) tp, NULL, &increment);
      vbytes = get_vbytes_v1 (fp, kind, size, vlen);

      t2p->ctt_name = tp->ctt_name;
      t2p->ctt_info = CTF_TYPE_INFO (kind, isroot, vlen);

      switch (kind)
	{
	case CTF_K_FUNCTION:
	case CTF_K_FORWARD:
	case CTF_K_TYPEDEF:
	case CTF_K_POINTER:
	case CTF_K_VOLATILE:
	case CTF_K_CONST:
	case CTF_K_RESTRICT:
	  t2p->ctt_type = tp->ctt_type;
	  break;
	case CTF_K_INTEGER:
	case CTF_K_FLOAT:
	case CTF_K_ARRAY:
	case CTF_K_STRUCT:
	case CTF_K_UNION:
	case CTF_K_ENUM:
	case CTF_K_UNKNOWN:
	  if ((size_t) size <= CTF_MAX_SIZE)
	    t2p->ctt_size = size;
	  else
	    {
	      t2p->ctt_lsizehi = CTF_SIZE_TO_LSIZE_HI (size);
	      t2p->ctt_lsizelo = CTF_SIZE_TO_LSIZE_LO (size);
	    }
	  break;
	}

      v2size = get_ctt_size_v2 (fp, t2p, NULL, &v2increment);
      v2bytes = get_vbytes_v2 (fp, kind, v2size, vlen);

      /* Catch out-of-sync get_ctt_size_*().  The count goes wrong if
	 these are not identical (and having them different makes no
	 sense semantically).  */

      assert (size == v2size);

      /* Now the varlen info.  */

      vdata = (void *) ((uintptr_t) tp + increment);
      v2data = (void *) ((uintptr_t) t2p + v2increment);

      switch (kind)
	{
	case CTF_K_ARRAY:
	  {
	    const ctf_array_v1_t *ap = (const ctf_array_v1_t *) vdata;
	    ctf_array_t *a2p = (ctf_array_t *) v2data;

	    a2p->cta_contents = ap->cta_contents;
	    a2p->cta_index = ap->cta_index;
	    a2p->cta_nelems = ap->cta_nelems;
	    break;
	  }
	case CTF_K_STRUCT:
	case CTF_K_UNION:
	  {
	    ctf_member_t tmp;
	    const ctf_member_v1_t *m1 = (const ctf_member_v1_t *) vdata;
	    const ctf_lmember_v1_t *lm1 = (const ctf_lmember_v1_t *) m1;
	    ctf_member_t *m2 = (ctf_member_t *) v2data;
	    ctf_lmember_t *lm2 = (ctf_lmember_t *) m2;
	    unsigned long i;

	    /* We walk all four pointers forward, but only reference the two
	       that are valid for the given size, to avoid quadruplicating all
	       the code.  */

	    for (i = vlen; i != 0; i--, m1++, lm1++, m2++, lm2++)
	      {
		size_t offset;
		if (size < CTF_LSTRUCT_THRESH_V1)
		  {
		    offset = m1->ctm_offset;
		    tmp.ctm_name = m1->ctm_name;
		    tmp.ctm_type = m1->ctm_type;
		  }
		else
		  {
		    offset = CTF_LMEM_OFFSET (lm1);
		    tmp.ctm_name = lm1->ctlm_name;
		    tmp.ctm_type = lm1->ctlm_type;
		  }
		if (size < CTF_LSTRUCT_THRESH)
		  {
		    m2->ctm_name = tmp.ctm_name;
		    m2->ctm_type = tmp.ctm_type;
		    m2->ctm_offset = offset;
		  }
		else
		  {
		    lm2->ctlm_name = tmp.ctm_name;
		    lm2->ctlm_type = tmp.ctm_type;
		    lm2->ctlm_offsethi = CTF_OFFSET_TO_LMEMHI (offset);
		    lm2->ctlm_offsetlo = CTF_OFFSET_TO_LMEMLO (offset);
		  }
	      }
	    break;
	  }
	case CTF_K_FUNCTION:
	  {
	    unsigned long i;
	    unsigned short *a1 = (unsigned short *) vdata;
	    uint32_t *a2 = (uint32_t *) v2data;

	    for (i = vlen; i != 0; i--, a1++, a2++)
	      *a2 = *a1;
	  }
	/* FALLTHRU */
	default:
	  /* Catch out-of-sync get_vbytes_*().  */
	  assert (vbytes == v2bytes);
	  memcpy (v2data, vdata, vbytes);
	}
    }

  /* Verify that the entire region was converted.  If not, we are either
     converting too much, or too little (leading to a buffer overrun either here
     or at read time, in init_types().) */

  assert ((size_t) t2p - (size_t) fp->ctf_buf == cth->cth_stroff);

  ctf_set_version (fp, cth, CTF_VERSION_1_UPGRADED_3);
  free (old_ctf_base);

  return 0;
}

/* Upgrade from any earlier version.  */
static int
upgrade_types (ctf_dict_t *fp, ctf_header_t *cth)
{
  switch (cth->cth_version)
    {
      /* v1 requires a full pass and reformatting.  */
    case CTF_VERSION_1:
      upgrade_types_v1 (fp, cth);
      /* FALLTHRU */
      /* Already-converted v1 is just like later versions except that its
	 parent/child boundary is unchanged (and much lower).  */

    case CTF_VERSION_1_UPGRADED_3:
      fp->ctf_parmax = CTF_MAX_PTYPE_V1;

      /* v2 is just the same as v3 except for new types and sections:
	 no upgrading required. */
    case CTF_VERSION_2: ;
      /* FALLTHRU */
    }
  return 0;
}

/* Initialize the type ID translation table with the byte offset of each type,
   and initialize the hash tables of each named type.  Upgrade the type table to
   the latest supported representation in the process, if needed, and if this
   recension of libctf supports upgrading.  */

static int
init_types (ctf_dict_t *fp, ctf_header_t *cth)
{
  const ctf_type_t *tbuf;
  const ctf_type_t *tend;

  unsigned long pop[CTF_K_MAX + 1] = { 0 };
  const ctf_type_t *tp;
  uint32_t id;
  uint32_t *xp;

  /* We determine whether the dict is a child or a parent based on the value of
     cth_parname.  */

  int child = cth->cth_parname != 0;
  int nlstructs = 0, nlunions = 0;
  int err;

  assert (!(fp->ctf_flags & LCTF_RDWR));

  if (_libctf_unlikely_ (fp->ctf_version == CTF_VERSION_1))
    {
      int err;
      if ((err = upgrade_types (fp, cth)) != 0)
	return err;				/* Upgrade failed.  */
    }

  tbuf = (ctf_type_t *) (fp->ctf_buf + cth->cth_typeoff);
  tend = (ctf_type_t *) (fp->ctf_buf + cth->cth_stroff);

  /* We make two passes through the entire type section.  In this first
     pass, we count the number of each type and the total number of types.  */

  for (tp = tbuf; tp < tend; fp->ctf_typemax++)
    {
      unsigned short kind = LCTF_INFO_KIND (fp, tp->ctt_info);
      unsigned long vlen = LCTF_INFO_VLEN (fp, tp->ctt_info);
      ssize_t size, increment, vbytes;

      (void) ctf_get_ctt_size (fp, tp, &size, &increment);
      vbytes = LCTF_VBYTES (fp, kind, size, vlen);

      if (vbytes < 0)
	return ECTF_CORRUPT;

      /* For forward declarations, ctt_type is the CTF_K_* kind for the tag,
	 so bump that population count too.  */
      if (kind == CTF_K_FORWARD)
	pop[tp->ctt_type]++;

      tp = (ctf_type_t *) ((uintptr_t) tp + increment + vbytes);
      pop[kind]++;
    }

  if (child)
    {
      ctf_dprintf ("CTF dict %p is a child\n", (void *) fp);
      fp->ctf_flags |= LCTF_CHILD;
    }
  else
    ctf_dprintf ("CTF dict %p is a parent\n", (void *) fp);

  /* Now that we've counted up the number of each type, we can allocate
     the hash tables, type translation table, and pointer table.  */

  if ((fp->ctf_structs.ctn_readonly
       = ctf_hash_create (pop[CTF_K_STRUCT], ctf_hash_string,
			  ctf_hash_eq_string)) == NULL)
    return ENOMEM;

  if ((fp->ctf_unions.ctn_readonly
       = ctf_hash_create (pop[CTF_K_UNION], ctf_hash_string,
			  ctf_hash_eq_string)) == NULL)
    return ENOMEM;

  if ((fp->ctf_enums.ctn_readonly
       = ctf_hash_create (pop[CTF_K_ENUM], ctf_hash_string,
			  ctf_hash_eq_string)) == NULL)
    return ENOMEM;

  if ((fp->ctf_names.ctn_readonly
       = ctf_hash_create (pop[CTF_K_UNKNOWN] +
			  pop[CTF_K_INTEGER] +
			  pop[CTF_K_FLOAT] +
			  pop[CTF_K_FUNCTION] +
			  pop[CTF_K_TYPEDEF] +
			  pop[CTF_K_POINTER] +
			  pop[CTF_K_VOLATILE] +
			  pop[CTF_K_CONST] +
			  pop[CTF_K_RESTRICT],
			  ctf_hash_string,
			  ctf_hash_eq_string)) == NULL)
    return ENOMEM;

  fp->ctf_txlate = malloc (sizeof (uint32_t) * (fp->ctf_typemax + 1));
  fp->ctf_ptrtab_len = fp->ctf_typemax + 1;
  fp->ctf_ptrtab = malloc (sizeof (uint32_t) * fp->ctf_ptrtab_len);

  if (fp->ctf_txlate == NULL || fp->ctf_ptrtab == NULL)
    return ENOMEM;		/* Memory allocation failed.  */

  xp = fp->ctf_txlate;
  *xp++ = 0;			/* Type id 0 is used as a sentinel value.  */

  memset (fp->ctf_txlate, 0, sizeof (uint32_t) * (fp->ctf_typemax + 1));
  memset (fp->ctf_ptrtab, 0, sizeof (uint32_t) * (fp->ctf_typemax + 1));

  /* In the second pass through the types, we fill in each entry of the
     type and pointer tables and add names to the appropriate hashes.  */

  for (id = 1, tp = tbuf; tp < tend; xp++, id++)
    {
      unsigned short kind = LCTF_INFO_KIND (fp, tp->ctt_info);
      unsigned short isroot = LCTF_INFO_ISROOT (fp, tp->ctt_info);
      unsigned long vlen = LCTF_INFO_VLEN (fp, tp->ctt_info);
      ssize_t size, increment, vbytes;

      const char *name;

      (void) ctf_get_ctt_size (fp, tp, &size, &increment);
      name = ctf_strptr (fp, tp->ctt_name);
      /* Cannot fail: shielded by call in loop above.  */
      vbytes = LCTF_VBYTES (fp, kind, size, vlen);

      switch (kind)
	{
	case CTF_K_UNKNOWN:
	case CTF_K_INTEGER:
	case CTF_K_FLOAT:
	  /* Names are reused by bit-fields, which are differentiated by their
	     encodings, and so typically we'd record only the first instance of
	     a given intrinsic.  However, we replace an existing type with a
	     root-visible version so that we can be sure to find it when
	     checking for conflicting definitions in ctf_add_type().  */

	  if (((ctf_hash_lookup_type (fp->ctf_names.ctn_readonly,
				      fp, name)) == 0)
	      || isroot)
	    {
	      err = ctf_hash_define_type (fp->ctf_names.ctn_readonly, fp,
					  LCTF_INDEX_TO_TYPE (fp, id, child),
					  tp->ctt_name);
	      if (err != 0)
		return err;
	    }
	  break;

	  /* These kinds have no name, so do not need interning into any
	     hashtables.  */
	case CTF_K_ARRAY:
	case CTF_K_SLICE:
	  break;

	case CTF_K_FUNCTION:
	  if (!isroot)
	    break;

	  err = ctf_hash_insert_type (fp->ctf_names.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);
	  if (err != 0)
	    return err;
	  break;

	case CTF_K_STRUCT:
	  if (size >= CTF_LSTRUCT_THRESH)
	    nlstructs++;

	  if (!isroot)
	    break;

	  err = ctf_hash_define_type (fp->ctf_structs.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);

	  if (err != 0)
	    return err;

	  break;

	case CTF_K_UNION:
	  if (size >= CTF_LSTRUCT_THRESH)
	    nlunions++;

	  if (!isroot)
	    break;

	  err = ctf_hash_define_type (fp->ctf_unions.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);

	  if (err != 0)
	    return err;
	  break;

	case CTF_K_ENUM:
	  if (!isroot)
	    break;

	  err = ctf_hash_define_type (fp->ctf_enums.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);

	  if (err != 0)
	    return err;
	  break;

	case CTF_K_TYPEDEF:
	  if (!isroot)
	    break;

	  err = ctf_hash_insert_type (fp->ctf_names.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);
	  if (err != 0)
	    return err;
	  break;

	case CTF_K_FORWARD:
	  {
	    ctf_names_t *np = ctf_name_table (fp, tp->ctt_type);

	    if (!isroot)
	      break;

	    /* Only insert forward tags into the given hash if the type or tag
	       name is not already present.  */
	    if (ctf_hash_lookup_type (np->ctn_readonly, fp, name) == 0)
	      {
		err = ctf_hash_insert_type (np->ctn_readonly, fp,
					    LCTF_INDEX_TO_TYPE (fp, id, child),
					    tp->ctt_name);
		if (err != 0)
		  return err;
	      }
	    break;
	  }

	case CTF_K_POINTER:
	  /* If the type referenced by the pointer is in this CTF dict, then
	     store the index of the pointer type in fp->ctf_ptrtab[ index of
	     referenced type ].  */

	  if (LCTF_TYPE_ISCHILD (fp, tp->ctt_type) == child
	      && LCTF_TYPE_TO_INDEX (fp, tp->ctt_type) <= fp->ctf_typemax)
	    fp->ctf_ptrtab[LCTF_TYPE_TO_INDEX (fp, tp->ctt_type)] = id;
	 /*FALLTHRU*/

	case CTF_K_VOLATILE:
	case CTF_K_CONST:
	case CTF_K_RESTRICT:
	  if (!isroot)
	    break;

	  err = ctf_hash_insert_type (fp->ctf_names.ctn_readonly, fp,
				      LCTF_INDEX_TO_TYPE (fp, id, child),
				      tp->ctt_name);
	  if (err != 0)
	    return err;
	  break;
	default:
	  ctf_err_warn (fp, 0, ECTF_CORRUPT,
			_("init_types(): unhandled CTF kind: %x"), kind);
	  return ECTF_CORRUPT;
	}

      *xp = (uint32_t) ((uintptr_t) tp - (uintptr_t) fp->ctf_buf);
      tp = (ctf_type_t *) ((uintptr_t) tp + increment + vbytes);
    }

  ctf_dprintf ("%lu total types processed\n", fp->ctf_typemax);
  ctf_dprintf ("%u enum names hashed\n",
	       ctf_hash_size (fp->ctf_enums.ctn_readonly));
  ctf_dprintf ("%u struct names hashed (%d long)\n",
	       ctf_hash_size (fp->ctf_structs.ctn_readonly), nlstructs);
  ctf_dprintf ("%u union names hashed (%d long)\n",
	       ctf_hash_size (fp->ctf_unions.ctn_readonly), nlunions);
  ctf_dprintf ("%u base type names hashed\n",
	       ctf_hash_size (fp->ctf_names.ctn_readonly));

  return 0;
}

/* Endianness-flipping routines.

   We flip everything, mindlessly, even 1-byte entities, so that future
   expansions do not require changes to this code.  */

/* Flip the endianness of the CTF header.  */

void
ctf_flip_header (ctf_header_t *cth)
{
  swap_thing (cth->cth_preamble.ctp_magic);
  swap_thing (cth->cth_preamble.ctp_version);
  swap_thing (cth->cth_preamble.ctp_flags);
  swap_thing (cth->cth_parlabel);
  swap_thing (cth->cth_parname);
  swap_thing (cth->cth_cuname);
  swap_thing (cth->cth_objtoff);
  swap_thing (cth->cth_funcoff);
  swap_thing (cth->cth_objtidxoff);
  swap_thing (cth->cth_funcidxoff);
  swap_thing (cth->cth_varoff);
  swap_thing (cth->cth_typeoff);
  swap_thing (cth->cth_stroff);
  swap_thing (cth->cth_strlen);
}

/* Flip the endianness of the label section, an array of ctf_lblent_t.  */

static void
flip_lbls (void *start, size_t len)
{
  ctf_lblent_t *lbl = start;
  ssize_t i;

  for (i = len / sizeof (struct ctf_lblent); i > 0; lbl++, i--)
    {
      swap_thing (lbl->ctl_label);
      swap_thing (lbl->ctl_type);
    }
}

/* Flip the endianness of the data-object or function sections or their indexes,
   all arrays of uint32_t.  */

static void
flip_objts (void *start, size_t len)
{
  uint32_t *obj = start;
  ssize_t i;

  for (i = len / sizeof (uint32_t); i > 0; obj++, i--)
      swap_thing (*obj);
}

/* Flip the endianness of the variable section, an array of ctf_varent_t.  */

static void
flip_vars (void *start, size_t len)
{
  ctf_varent_t *var = start;
  ssize_t i;

  for (i = len / sizeof (struct ctf_varent); i > 0; var++, i--)
    {
      swap_thing (var->ctv_name);
      swap_thing (var->ctv_type);
    }
}

/* Flip the endianness of the type section, a tagged array of ctf_type or
   ctf_stype followed by variable data.  */

static int
flip_types (ctf_dict_t *fp, void *start, size_t len, int to_foreign)
{
  ctf_type_t *t = start;

  while ((uintptr_t) t < ((uintptr_t) start) + len)
    {
      uint32_t kind;
      size_t size;
      uint32_t vlen;
      size_t vbytes;

      if (to_foreign)
	{
	  kind = CTF_V2_INFO_KIND (t->ctt_info);
	  size = t->ctt_size;
	  vlen = CTF_V2_INFO_VLEN (t->ctt_info);
	  vbytes = get_vbytes_v2 (fp, kind, size, vlen);
	}

      swap_thing (t->ctt_name);
      swap_thing (t->ctt_info);
      swap_thing (t->ctt_size);

      if (!to_foreign)
	{
	  kind = CTF_V2_INFO_KIND (t->ctt_info);
	  size = t->ctt_size;
	  vlen = CTF_V2_INFO_VLEN (t->ctt_info);
	  vbytes = get_vbytes_v2 (fp, kind, size, vlen);
	}

      if (_libctf_unlikely_ (size == CTF_LSIZE_SENT))
	{
	  if (to_foreign)
	    size = CTF_TYPE_LSIZE (t);

	  swap_thing (t->ctt_lsizehi);
	  swap_thing (t->ctt_lsizelo);

	  if (!to_foreign)
	    size = CTF_TYPE_LSIZE (t);

	  t = (ctf_type_t *) ((uintptr_t) t + sizeof (ctf_type_t));
	}
      else
	t = (ctf_type_t *) ((uintptr_t) t + sizeof (ctf_stype_t));

      switch (kind)
	{
	case CTF_K_FORWARD:
	case CTF_K_UNKNOWN:
	case CTF_K_POINTER:
	case CTF_K_TYPEDEF:
	case CTF_K_VOLATILE:
	case CTF_K_CONST:
	case CTF_K_RESTRICT:
	  /* These types have no vlen data to swap.  */
	  assert (vbytes == 0);
	  break;

	case CTF_K_INTEGER:
	case CTF_K_FLOAT:
	  {
	    /* These types have a single uint32_t.  */

	    uint32_t *item = (uint32_t *) t;

	    swap_thing (*item);
	    break;
	  }

	case CTF_K_FUNCTION:
	  {
	    /* This type has a bunch of uint32_ts.  */

	    uint32_t *item = (uint32_t *) t;
	    ssize_t i;

	    for (i = vlen; i > 0; item++, i--)
	      swap_thing (*item);
	    break;
	  }

	case CTF_K_ARRAY:
	  {
	    /* This has a single ctf_array_t.  */

	    ctf_array_t *a = (ctf_array_t *) t;

	    assert (vbytes == sizeof (ctf_array_t));
	    swap_thing (a->cta_contents);
	    swap_thing (a->cta_index);
	    swap_thing (a->cta_nelems);

	    break;
	  }

	case CTF_K_SLICE:
	  {
	    /* This has a single ctf_slice_t.  */

	    ctf_slice_t *s = (ctf_slice_t *) t;

	    assert (vbytes == sizeof (ctf_slice_t));
	    swap_thing (s->cts_type);
	    swap_thing (s->cts_offset);
	    swap_thing (s->cts_bits);

	    break;
	  }

	case CTF_K_STRUCT:
	case CTF_K_UNION:
	  {
	    /* This has an array of ctf_member or ctf_lmember, depending on
	       size.  We could consider it to be a simple array of uint32_t,
	       but for safety's sake in case these structures ever acquire
	       non-uint32_t members, do it member by member.  */

	    if (_libctf_unlikely_ (size >= CTF_LSTRUCT_THRESH))
	      {
		ctf_lmember_t *lm = (ctf_lmember_t *) t;
		ssize_t i;
		for (i = vlen; i > 0; i--, lm++)
		  {
		    swap_thing (lm->ctlm_name);
		    swap_thing (lm->ctlm_offsethi);
		    swap_thing (lm->ctlm_type);
		    swap_thing (lm->ctlm_offsetlo);
		  }
	      }
	    else
	      {
		ctf_member_t *m = (ctf_member_t *) t;
		ssize_t i;
		for (i = vlen; i > 0; i--, m++)
		  {
		    swap_thing (m->ctm_name);
		    swap_thing (m->ctm_offset);
		    swap_thing (m->ctm_type);
		  }
	      }
	    break;
	  }

	case CTF_K_ENUM:
	  {
	    /* This has an array of ctf_enum_t.  */

	    ctf_enum_t *item = (ctf_enum_t *) t;
	    ssize_t i;

	    for (i = vlen; i > 0; item++, i--)
	      {
		swap_thing (item->cte_name);
		swap_thing (item->cte_value);
	      }
	    break;
	  }
	default:
	  ctf_err_warn (fp, 0, ECTF_CORRUPT,
			_("unhandled CTF kind in endianness conversion: %x"),
			kind);
	  return ECTF_CORRUPT;
	}

      t = (ctf_type_t *) ((uintptr_t) t + vbytes);
    }

  return 0;
}

/* Flip the endianness of BUF, given the offsets in the (already endian-
   converted) CTH.  If TO_FOREIGN is set, flip to foreign-endianness; if not,
   flip away.

   All of this stuff happens before the header is fully initialized, so the
   LCTF_*() macros cannot be used yet.  Since we do not try to endian-convert v1
   data, this is no real loss.  */

int
ctf_flip (ctf_dict_t *fp, ctf_header_t *cth, unsigned char *buf,
	  int to_foreign)
{
  ctf_dprintf("flipping endianness\n");

  flip_lbls (buf + cth->cth_lbloff, cth->cth_objtoff - cth->cth_lbloff);
  flip_objts (buf + cth->cth_objtoff, cth->cth_funcoff - cth->cth_objtoff);
  flip_objts (buf + cth->cth_funcoff, cth->cth_objtidxoff - cth->cth_funcoff);
  flip_objts (buf + cth->cth_objtidxoff, cth->cth_funcidxoff - cth->cth_objtidxoff);
  flip_objts (buf + cth->cth_funcidxoff, cth->cth_varoff - cth->cth_funcidxoff);
  flip_vars (buf + cth->cth_varoff, cth->cth_typeoff - cth->cth_varoff);
  return flip_types (fp, buf + cth->cth_typeoff,
		     cth->cth_stroff - cth->cth_typeoff, to_foreign);
}

/* Set up the ctl hashes in a ctf_dict_t.  Called by both writable and
   non-writable dictionary initialization.  */
void ctf_set_ctl_hashes (ctf_dict_t *fp)
{
  /* Initialize the ctf_lookup_by_name top-level dictionary.  We keep an
     array of type name prefixes and the corresponding ctf_hash to use.  */
  fp->ctf_lookups[0].ctl_prefix = "struct";
  fp->ctf_lookups[0].ctl_len = strlen (fp->ctf_lookups[0].ctl_prefix);
  fp->ctf_lookups[0].ctl_hash = &fp->ctf_structs;
  fp->ctf_lookups[1].ctl_prefix = "union";
  fp->ctf_lookups[1].ctl_len = strlen (fp->ctf_lookups[1].ctl_prefix);
  fp->ctf_lookups[1].ctl_hash = &fp->ctf_unions;
  fp->ctf_lookups[2].ctl_prefix = "enum";
  fp->ctf_lookups[2].ctl_len = strlen (fp->ctf_lookups[2].ctl_prefix);
  fp->ctf_lookups[2].ctl_hash = &fp->ctf_enums;
  fp->ctf_lookups[3].ctl_prefix = _CTF_NULLSTR;
  fp->ctf_lookups[3].ctl_len = strlen (fp->ctf_lookups[3].ctl_prefix);
  fp->ctf_lookups[3].ctl_hash = &fp->ctf_names;
  fp->ctf_lookups[4].ctl_prefix = NULL;
  fp->ctf_lookups[4].ctl_len = 0;
  fp->ctf_lookups[4].ctl_hash = NULL;
}

/* Open a CTF file, mocking up a suitable ctf_sect.  */

ctf_dict_t *ctf_simple_open (const char *ctfsect, size_t ctfsect_size,
			     const char *symsect, size_t symsect_size,
			     size_t symsect_entsize,
			     const char *strsect, size_t strsect_size,
			     int *errp)
{
  return ctf_simple_open_internal (ctfsect, ctfsect_size, symsect, symsect_size,
				   symsect_entsize, strsect, strsect_size, NULL,
				   0, errp);
}

/* Open a CTF file, mocking up a suitable ctf_sect and overriding the external
   strtab with a synthetic one.  */

ctf_dict_t *ctf_simple_open_internal (const char *ctfsect, size_t ctfsect_size,
				      const char *symsect, size_t symsect_size,
				      size_t symsect_entsize,
				      const char *strsect, size_t strsect_size,
				      ctf_dynhash_t *syn_strtab, int writable,
				      int *errp)
{
  ctf_sect_t skeleton;

  ctf_sect_t ctf_sect, sym_sect, str_sect;
  ctf_sect_t *ctfsectp = NULL;
  ctf_sect_t *symsectp = NULL;
  ctf_sect_t *strsectp = NULL;

  skeleton.cts_name = _CTF_SECTION;
  skeleton.cts_entsize = 1;

  if (ctfsect)
    {
      memcpy (&ctf_sect, &skeleton, sizeof (struct ctf_sect));
      ctf_sect.cts_data = ctfsect;
      ctf_sect.cts_size = ctfsect_size;
      ctfsectp = &ctf_sect;
    }

  if (symsect)
    {
      memcpy (&sym_sect, &skeleton, sizeof (struct ctf_sect));
      sym_sect.cts_data = symsect;
      sym_sect.cts_size = symsect_size;
      sym_sect.cts_entsize = symsect_entsize;
      symsectp = &sym_sect;
    }

  if (strsect)
    {
      memcpy (&str_sect, &skeleton, sizeof (struct ctf_sect));
      str_sect.cts_data = strsect;
      str_sect.cts_size = strsect_size;
      strsectp = &str_sect;
    }

  return ctf_bufopen_internal (ctfsectp, symsectp, strsectp, syn_strtab,
			       writable, errp);
}

/* Decode the specified CTF buffer and optional symbol table, and create a new
   CTF dict representing the symbolic debugging information.  This code can
   be used directly by the debugger, or it can be used as the engine for
   ctf_fdopen() or ctf_open(), below.  */

ctf_dict_t *
ctf_bufopen (const ctf_sect_t *ctfsect, const ctf_sect_t *symsect,
	     const ctf_sect_t *strsect, int *errp)
{
  return ctf_bufopen_internal (ctfsect, symsect, strsect, NULL, 0, errp);
}

/* Like ctf_bufopen, but overriding the external strtab with a synthetic one.  */

ctf_dict_t *
ctf_bufopen_internal (const ctf_sect_t *ctfsect, const ctf_sect_t *symsect,
		      const ctf_sect_t *strsect, ctf_dynhash_t *syn_strtab,
		      int writable, int *errp)
{
  const ctf_preamble_t *pp;
  size_t hdrsz = sizeof (ctf_header_t);
  ctf_header_t *hp;
  ctf_dict_t *fp;
  int foreign_endian = 0;
  int err;

  libctf_init_debug();

  if ((ctfsect == NULL) || ((symsect != NULL) &&
			    ((strsect == NULL) && syn_strtab == NULL)))
    return (ctf_set_open_errno (errp, EINVAL));

  if (symsect != NULL && symsect->cts_entsize != sizeof (Elf32_Sym) &&
      symsect->cts_entsize != sizeof (Elf64_Sym))
    return (ctf_set_open_errno (errp, ECTF_SYMTAB));

  if (symsect != NULL && symsect->cts_data == NULL)
    return (ctf_set_open_errno (errp, ECTF_SYMBAD));

  if (strsect != NULL && strsect->cts_data == NULL)
    return (ctf_set_open_errno (errp, ECTF_STRBAD));

  if (ctfsect->cts_data == NULL
      || ctfsect->cts_size < sizeof (ctf_preamble_t))
    return (ctf_set_open_errno (errp, ECTF_NOCTFBUF));

  pp = (const ctf_preamble_t *) ctfsect->cts_data;

  ctf_dprintf ("ctf_bufopen: magic=0x%x version=%u\n",
	       pp->ctp_magic, pp->ctp_version);

  /* Validate each part of the CTF header.

     First, we validate the preamble (common to all versions).  At that point,
     we know the endianness and specific header version, and can validate the
     version-specific parts including section offsets and alignments.

     We specifically do not support foreign-endian old versions.  */

  if (_libctf_unlikely_ (pp->ctp_magic != CTF_MAGIC))
    {
      if (pp->ctp_magic == bswap_16 (CTF_MAGIC))
	{
	  if (pp->ctp_version != CTF_VERSION_3)
	    return (ctf_set_open_errno (errp, ECTF_CTFVERS));
	  foreign_endian = 1;
	}
      else
	return (ctf_set_open_errno (errp, ECTF_NOCTFBUF));
    }

  if (_libctf_unlikely_ ((pp->ctp_version < CTF_VERSION_1)
			 || (pp->ctp_version > CTF_VERSION_3)))
    return (ctf_set_open_errno (errp, ECTF_CTFVERS));

  if ((symsect != NULL) && (pp->ctp_version < CTF_VERSION_2))
    {
      /* The symtab can contain function entries which contain embedded ctf
	 info.  We do not support dynamically upgrading such entries (none
	 should exist in any case, since dwarf2ctf does not create them).  */

      ctf_err_warn (NULL, 0, ECTF_NOTSUP, _("ctf_bufopen: CTF version %d "
					    "symsect not supported"),
		    pp->ctp_version);
      return (ctf_set_open_errno (errp, ECTF_NOTSUP));
    }

  if (pp->ctp_version < CTF_VERSION_3)
    hdrsz = sizeof (ctf_header_v2_t);

  if (_libctf_unlikely_ (pp->ctp_flags > CTF_F_MAX))
    {
      ctf_err_warn (NULL, 0, ECTF_FLAGS, _("ctf_bufopen: invalid header "
					   "flags: %x"),
		    (unsigned int) pp->ctp_flags);
      return (ctf_set_open_errno (errp, ECTF_FLAGS));
    }

  if (ctfsect->cts_size < hdrsz)
    return (ctf_set_open_errno (errp, ECTF_NOCTFBUF));

  if ((fp = malloc (sizeof (ctf_dict_t))) == NULL)
    return (ctf_set_open_errno (errp, ENOMEM));

  memset (fp, 0, sizeof (ctf_dict_t));

  if (writable)
    fp->ctf_flags |= LCTF_RDWR;

  if ((fp->ctf_header = malloc (sizeof (struct ctf_header))) == NULL)
    {
      free (fp);
      return (ctf_set_open_errno (errp, ENOMEM));
    }
  hp = fp->ctf_header;
  memcpy (hp, ctfsect->cts_data, hdrsz);
  if (pp->ctp_version < CTF_VERSION_3)
    upgrade_header (hp);

  if (foreign_endian)
    ctf_flip_header (hp);
  fp->ctf_openflags = hp->cth_flags;
  fp->ctf_size = hp->cth_stroff + hp->cth_strlen;

  ctf_dprintf ("ctf_bufopen: uncompressed size=%lu\n",
	       (unsigned long) fp->ctf_size);

  if (hp->cth_lbloff > fp->ctf_size || hp->cth_objtoff > fp->ctf_size
      || hp->cth_funcoff > fp->ctf_size || hp->cth_objtidxoff > fp->ctf_size
      || hp->cth_funcidxoff > fp->ctf_size || hp->cth_typeoff > fp->ctf_size
      || hp->cth_stroff > fp->ctf_size)
    {
      ctf_err_warn (NULL, 0, ECTF_CORRUPT, _("header offset exceeds CTF size"));
      return (ctf_set_open_errno (errp, ECTF_CORRUPT));
    }

  if (hp->cth_lbloff > hp->cth_objtoff
      || hp->cth_objtoff > hp->cth_funcoff
      || hp->cth_funcoff > hp->cth_typeoff
      || hp->cth_funcoff > hp->cth_objtidxoff
      || hp->cth_objtidxoff > hp->cth_funcidxoff
      || hp->cth_funcidxoff > hp->cth_varoff
      || hp->cth_varoff > hp->cth_typeoff || hp->cth_typeoff > hp->cth_stroff)
    {
      ctf_err_warn (NULL, 0, ECTF_CORRUPT, _("overlapping CTF sections"));
      return (ctf_set_open_errno (errp, ECTF_CORRUPT));
    }

  if ((hp->cth_lbloff & 3) || (hp->cth_objtoff & 2)
      || (hp->cth_funcoff & 2) || (hp->cth_objtidxoff & 2)
      || (hp->cth_funcidxoff & 2) || (hp->cth_varoff & 3)
      || (hp->cth_typeoff & 3))
    {
      ctf_err_warn (NULL, 0, ECTF_CORRUPT,
		    _("CTF sections not properly aligned"));
      return (ctf_set_open_errno (errp, ECTF_CORRUPT));
    }

  /* This invariant will be lifted in v4, but for now it is true.  */

  if ((hp->cth_funcidxoff - hp->cth_objtidxoff != 0) &&
      (hp->cth_funcidxoff - hp->cth_objtidxoff
       != hp->cth_funcoff - hp->cth_objtoff))
    {
      ctf_err_warn (NULL, 0, ECTF_CORRUPT,
		    _("Object index section is neither empty nor the "
		      "same length as the object section: %u versus %u "
		      "bytes"), hp->cth_funcoff - hp->cth_objtoff,
		    hp->cth_funcidxoff - hp->cth_objtidxoff);
      return (ctf_set_open_errno (errp, ECTF_CORRUPT));
    }

  if ((hp->cth_varoff - hp->cth_funcidxoff != 0) &&
      (hp->cth_varoff - hp->cth_funcidxoff
       != hp->cth_objtidxoff - hp->cth_funcoff) &&
      (hp->cth_flags & CTF_F_NEWFUNCINFO))
    {
      ctf_err_warn (NULL, 0, ECTF_CORRUPT,
		    _("Function index section is neither empty nor the "
		      "same length as the function section: %u versus %u "
		      "bytes"), hp->cth_objtidxoff - hp->cth_funcoff,
		    hp->cth_varoff - hp->cth_funcidxoff);
      return (ctf_set_open_errno (errp, ECTF_CORRUPT));
    }

  /* Once everything is determined to be valid, attempt to decompress the CTF
     data buffer if it is compressed, or copy it into new storage if it is not
     compressed but needs endian-flipping.  Otherwise we just put the data
     section's buffer pointer into ctf_buf, below.  */

  /* Note: if this is a v1 buffer, it will be reallocated and expanded by
     init_types().  */

  if (hp->cth_flags & CTF_F_COMPRESS)
    {
      size_t srclen;
      uLongf dstlen;
      const void *src;
      int rc = Z_OK;

      /* We are allocating this ourselves, so we can drop the ctf header
	 copy in favour of ctf->ctf_header.  */

      if ((fp->ctf_base = malloc (fp->ctf_size)) == NULL)
	{
	  err = ECTF_ZALLOC;
	  goto bad;
	}
      fp->ctf_dynbase = fp->ctf_base;
      hp->cth_flags &= ~CTF_F_COMPRESS;

      src = (unsigned char *) ctfsect->cts_data + hdrsz;
      srclen = ctfsect->cts_size - hdrsz;
      dstlen = fp->ctf_size;
      fp->ctf_buf = fp->ctf_base;

      if ((rc = uncompress (fp->ctf_base, &dstlen, src, srclen)) != Z_OK)
	{
	  ctf_err_warn (NULL, 0, ECTF_DECOMPRESS, _("zlib inflate err: %s"),
			zError (rc));
	  err = ECTF_DECOMPRESS;
	  goto bad;
	}

      if ((size_t) dstlen != fp->ctf_size)
	{
	  ctf_err_warn (NULL, 0, ECTF_CORRUPT,
			_("zlib inflate short: got %lu of %lu bytes"),
			(unsigned long) dstlen, (unsigned long) fp->ctf_size);
	  err = ECTF_CORRUPT;
	  goto bad;
	}
    }
  else
    {
      if (_libctf_unlikely_ (ctfsect->cts_size < hdrsz + fp->ctf_size))
	{
	  ctf_err_warn (NULL, 0, ECTF_CORRUPT,
			_("%lu byte long CTF dictionary overruns %lu byte long CTF section"),
			(unsigned long) ctfsect->cts_size,
			(unsigned long) (hdrsz + fp->ctf_size));
	  err = ECTF_CORRUPT;
	  goto bad;
	}

      if (foreign_endian)
	{
	  if ((fp->ctf_base = malloc (fp->ctf_size)) == NULL)
	    {
	      err = ECTF_ZALLOC;
	      goto bad;
	    }
	  fp->ctf_dynbase = fp->ctf_base;
	  memcpy (fp->ctf_base, ((unsigned char *) ctfsect->cts_data) + hdrsz,
		  fp->ctf_size);
	  fp->ctf_buf = fp->ctf_base;
	}
      else
	{
	  /* We are just using the section passed in -- but its header may
	     be an old version.  Point ctf_buf past the old header, and
	     never touch it again.  */
	  fp->ctf_base = (unsigned char *) ctfsect->cts_data;
	  fp->ctf_dynbase = NULL;
	  fp->ctf_buf = fp->ctf_base + hdrsz;
	}
    }

  /* Once we have uncompressed and validated the CTF data buffer, we can
     proceed with initializing the ctf_dict_t we allocated above.

     Nothing that depends on buf or base should be set directly in this function
     before the init_types() call, because it may be reallocated during
     transparent upgrade if this recension of libctf is so configured: see
     ctf_set_base().  */

  ctf_set_version (fp, hp, hp->cth_version);
  if (ctf_str_create_atoms (fp) < 0)
    {
      err = ENOMEM;
      goto bad;
    }

  fp->ctf_parmax = CTF_MAX_PTYPE;
  memcpy (&fp->ctf_data, ctfsect, sizeof (ctf_sect_t));

  if (symsect != NULL)
    {
      memcpy (&fp->ctf_symtab, symsect, sizeof (ctf_sect_t));
      memcpy (&fp->ctf_strtab, strsect, sizeof (ctf_sect_t));
    }

  if (fp->ctf_data.cts_name != NULL)
    if ((fp->ctf_data.cts_name = strdup (fp->ctf_data.cts_name)) == NULL)
      {
	err = ENOMEM;
	goto bad;
      }
  if (fp->ctf_symtab.cts_name != NULL)
    if ((fp->ctf_symtab.cts_name = strdup (fp->ctf_symtab.cts_name)) == NULL)
      {
	err = ENOMEM;
	goto bad;
      }
  if (fp->ctf_strtab.cts_name != NULL)
    if ((fp->ctf_strtab.cts_name = strdup (fp->ctf_strtab.cts_name)) == NULL)
      {
	err = ENOMEM;
	goto bad;
      }

  if (fp->ctf_data.cts_name == NULL)
    fp->ctf_data.cts_name = _CTF_NULLSTR;
  if (fp->ctf_symtab.cts_name == NULL)
    fp->ctf_symtab.cts_name = _CTF_NULLSTR;
  if (fp->ctf_strtab.cts_name == NULL)
    fp->ctf_strtab.cts_name = _CTF_NULLSTR;

  if (strsect != NULL)
    {
      fp->ctf_str[CTF_STRTAB_1].cts_strs = strsect->cts_data;
      fp->ctf_str[CTF_STRTAB_1].cts_len = strsect->cts_size;
    }
  fp->ctf_syn_ext_strtab = syn_strtab;

  if (foreign_endian &&
      (err = ctf_flip (fp, hp, fp->ctf_buf, 0)) != 0)
    {
      /* We can be certain that ctf_flip() will have endian-flipped everything
	 other than the types table when we return.  In particular the header
	 is fine, so set it, to allow freeing to use the usual code path.  */

      ctf_set_base (fp, hp, fp->ctf_base);
      goto bad;
    }

  ctf_set_base (fp, hp, fp->ctf_base);

  /* No need to do anything else for dynamic dicts: they do not support symbol
     lookups, and the type table is maintained in the dthashes.  */
  if (fp->ctf_flags & LCTF_RDWR)
    {
      fp->ctf_refcnt = 1;
      return fp;
    }

  if ((err = init_types (fp, hp)) != 0)
    goto bad;

  /* Allocate and initialize the symtab translation table, pointed to by
     ctf_sxlate, and the corresponding index sections.  This table may be too
     large for the actual size of the object and function info sections: if so,
     ctf_nsyms will be adjusted and the excess will never be used.  It's
     possible to do indexed symbol lookups even without a symbol table, so check
     even in that case.  Initially, we assume the symtab is native-endian: if it
     isn't, the caller will inform us later by calling ctf_symsect_endianness.  */
#ifdef WORDS_BIGENDIAN
  fp->ctf_symsect_little_endian = 0;
#else
  fp->ctf_symsect_little_endian = 1;
#endif

  if (symsect != NULL)
    {
      fp->ctf_nsyms = symsect->cts_size / symsect->cts_entsize;
      fp->ctf_sxlate = malloc (fp->ctf_nsyms * sizeof (uint32_t));

      if (fp->ctf_sxlate == NULL)
	{
	  err = ENOMEM;
	  goto bad;
	}
    }

  if ((err = init_symtab (fp, hp, symsect)) != 0)
    goto bad;

  ctf_set_ctl_hashes (fp);

  if (symsect != NULL)
    {
      if (symsect->cts_entsize == sizeof (Elf64_Sym))
	(void) ctf_setmodel (fp, CTF_MODEL_LP64);
      else
	(void) ctf_setmodel (fp, CTF_MODEL_ILP32);
    }
  else
    (void) ctf_setmodel (fp, CTF_MODEL_NATIVE);

  fp->ctf_refcnt = 1;
  return fp;

bad:
  ctf_set_open_errno (errp, err);
  ctf_err_warn_to_open (fp);
  ctf_dict_close (fp);
  return NULL;
}

/* Bump the refcount on the specified CTF dict, to allow export of ctf_dict_t's
   from iterators that open and close the ctf_dict_t around the loop.  (This
   does not extend their lifetime beyond that of the ctf_archive_t in which they
   are contained.)  */

void
ctf_ref (ctf_dict_t *fp)
{
  fp->ctf_refcnt++;
}

/* Close the specified CTF dict and free associated data structures.  Note that
   ctf_dict_close() is a reference counted operation: if the specified file is
   the parent of other active dict, its reference count will be greater than one
   and it will be freed later when no active children exist.  */

void
ctf_dict_close (ctf_dict_t *fp)
{
  ctf_dtdef_t *dtd, *ntd;
  ctf_dvdef_t *dvd, *nvd;
  ctf_in_flight_dynsym_t *did, *nid;
  ctf_err_warning_t *err, *nerr;

  if (fp == NULL)
    return;		   /* Allow ctf_dict_close(NULL) to simplify caller code.  */

  ctf_dprintf ("ctf_dict_close(%p) refcnt=%u\n", (void *) fp, fp->ctf_refcnt);

  if (fp->ctf_refcnt > 1)
    {
      fp->ctf_refcnt--;
      return;
    }

  /* It is possible to recurse back in here, notably if dicts in the
     ctf_link_inputs or ctf_link_outputs cite this dict as a parent without
     using ctf_import_unref.  Do nothing in that case.  */
  if (fp->ctf_refcnt == 0)
    return;

  fp->ctf_refcnt--;
  free (fp->ctf_dyncuname);
  free (fp->ctf_dynparname);
  if (fp->ctf_parent && !fp->ctf_parent_unreffed)
    ctf_dict_close (fp->ctf_parent);

  for (dtd = ctf_list_next (&fp->ctf_dtdefs); dtd != NULL; dtd = ntd)
    {
      ntd = ctf_list_next (dtd);
      ctf_dtd_delete (fp, dtd);
    }
  ctf_dynhash_destroy (fp->ctf_dthash);
  if (fp->ctf_flags & LCTF_RDWR)
    {
      ctf_dynhash_destroy (fp->ctf_structs.ctn_writable);
      ctf_dynhash_destroy (fp->ctf_unions.ctn_writable);
      ctf_dynhash_destroy (fp->ctf_enums.ctn_writable);
      ctf_dynhash_destroy (fp->ctf_names.ctn_writable);
    }
  else
    {
      ctf_hash_destroy (fp->ctf_structs.ctn_readonly);
      ctf_hash_destroy (fp->ctf_unions.ctn_readonly);
      ctf_hash_destroy (fp->ctf_enums.ctn_readonly);
      ctf_hash_destroy (fp->ctf_names.ctn_readonly);
    }

  for (dvd = ctf_list_next (&fp->ctf_dvdefs); dvd != NULL; dvd = nvd)
    {
      nvd = ctf_list_next (dvd);
      ctf_dvd_delete (fp, dvd);
    }
  ctf_dynhash_destroy (fp->ctf_dvhash);

  ctf_dynhash_destroy (fp->ctf_symhash);
  free (fp->ctf_funcidx_sxlate);
  free (fp->ctf_objtidx_sxlate);
  ctf_dynhash_destroy (fp->ctf_objthash);
  ctf_dynhash_destroy (fp->ctf_funchash);
  free (fp->ctf_dynsymidx);
  ctf_dynhash_destroy (fp->ctf_dynsyms);
  for (did = ctf_list_next (&fp->ctf_in_flight_dynsyms); did != NULL; did = nid)
    {
      nid = ctf_list_next (did);
      ctf_list_delete (&fp->ctf_in_flight_dynsyms, did);
      free (did);
    }

  ctf_str_free_atoms (fp);
  free (fp->ctf_tmp_typeslice);

  if (fp->ctf_data.cts_name != _CTF_NULLSTR)
    free ((char *) fp->ctf_data.cts_name);

  if (fp->ctf_symtab.cts_name != _CTF_NULLSTR)
    free ((char *) fp->ctf_symtab.cts_name);

  if (fp->ctf_strtab.cts_name != _CTF_NULLSTR)
    free ((char *) fp->ctf_strtab.cts_name);
  else if (fp->ctf_data_mmapped)
    ctf_munmap (fp->ctf_data_mmapped, fp->ctf_data_mmapped_len);

  free (fp->ctf_dynbase);

  ctf_dynhash_destroy (fp->ctf_syn_ext_strtab);
  ctf_dynhash_destroy (fp->ctf_link_inputs);
  ctf_dynhash_destroy (fp->ctf_link_outputs);
  ctf_dynhash_destroy (fp->ctf_link_type_mapping);
  ctf_dynhash_destroy (fp->ctf_link_in_cu_mapping);
  ctf_dynhash_destroy (fp->ctf_link_out_cu_mapping);
  ctf_dynhash_destroy (fp->ctf_add_processing);
  ctf_dedup_fini (fp, NULL, 0);
  ctf_dynset_destroy (fp->ctf_dedup_atoms_alloc);

  for (err = ctf_list_next (&fp->ctf_errs_warnings); err != NULL; err = nerr)
    {
      nerr = ctf_list_next (err);
      ctf_list_delete (&fp->ctf_errs_warnings, err);
      free (err->cew_text);
      free (err);
    }

  free (fp->ctf_sxlate);
  free (fp->ctf_txlate);
  free (fp->ctf_ptrtab);
  free (fp->ctf_pptrtab);

  free (fp->ctf_header);
  free (fp);
}

/* Backward compatibility.  */
void
ctf_file_close (ctf_file_t *fp)
{
  ctf_dict_close (fp);
}

/* The converse of ctf_open().  ctf_open() disguises whatever it opens as an
   archive, so closing one is just like closing an archive.  */
void
ctf_close (ctf_archive_t *arc)
{
  ctf_arc_close (arc);
}

/* Get the CTF archive from which this ctf_dict_t is derived.  */
ctf_archive_t *
ctf_get_arc (const ctf_dict_t *fp)
{
  return fp->ctf_archive;
}

/* Return the ctfsect out of the core ctf_impl.  Useful for freeing the
   ctfsect's data * after ctf_dict_close(), which is why we return the actual
   structure, not a pointer to it, since that is likely to become a pointer to
   freed data before the return value is used under the expected use case of
   ctf_getsect()/ ctf_dict_close()/free().  */
ctf_sect_t
ctf_getdatasect (const ctf_dict_t *fp)
{
  return fp->ctf_data;
}

ctf_sect_t
ctf_getsymsect (const ctf_dict_t *fp)
{
  return fp->ctf_symtab;
}

ctf_sect_t
ctf_getstrsect (const ctf_dict_t *fp)
{
  return fp->ctf_strtab;
}

/* Set the endianness of the symbol table attached to FP.  */
void
ctf_symsect_endianness (ctf_dict_t *fp, int little_endian)
{
  int old_endianness = fp->ctf_symsect_little_endian;

  fp->ctf_symsect_little_endian = !!little_endian;

  /* If we already have a symtab translation table, we need to repopulate it if
     our idea of the endianness has changed.  */

  if (old_endianness != fp->ctf_symsect_little_endian
      && fp->ctf_sxlate != NULL && fp->ctf_symtab.cts_data != NULL)
    assert (init_symtab (fp, fp->ctf_header, &fp->ctf_symtab) == 0);
}

/* Return the CTF handle for the parent CTF dict, if one exists.  Otherwise
   return NULL to indicate this dict has no imported parent.  */
ctf_dict_t *
ctf_parent_dict (ctf_dict_t *fp)
{
  return fp->ctf_parent;
}

/* Backward compatibility.  */
ctf_dict_t *
ctf_parent_file (ctf_dict_t *fp)
{
  return ctf_parent_dict (fp);
}

/* Return the name of the parent CTF dict, if one exists, or NULL otherwise.  */
const char *
ctf_parent_name (ctf_dict_t *fp)
{
  return fp->ctf_parname;
}

/* Set the parent name.  It is an error to call this routine without calling
   ctf_import() at some point.  */
int
ctf_parent_name_set (ctf_dict_t *fp, const char *name)
{
  if (fp->ctf_dynparname != NULL)
    free (fp->ctf_dynparname);

  if ((fp->ctf_dynparname = strdup (name)) == NULL)
    return (ctf_set_errno (fp, ENOMEM));
  fp->ctf_parname = fp->ctf_dynparname;
  return 0;
}

/* Return the name of the compilation unit this CTF file applies to.  Usually
   non-NULL only for non-parent dicts.  */
const char *
ctf_cuname (ctf_dict_t *fp)
{
  return fp->ctf_cuname;
}

/* Set the compilation unit name.  */
int
ctf_cuname_set (ctf_dict_t *fp, const char *name)
{
  if (fp->ctf_dyncuname != NULL)
    free (fp->ctf_dyncuname);

  if ((fp->ctf_dyncuname = strdup (name)) == NULL)
    return (ctf_set_errno (fp, ENOMEM));
  fp->ctf_cuname = fp->ctf_dyncuname;
  return 0;
}

/* Import the types from the specified parent dict by storing a pointer to it in
   ctf_parent and incrementing its reference count.  Only one parent is allowed:
   if a parent already exists, it is replaced by the new parent.  The pptrtab
   is wiped, and will be refreshed by the next ctf_lookup_by_name call.  */
int
ctf_import (ctf_dict_t *fp, ctf_dict_t *pfp)
{
  if (fp == NULL || fp == pfp || (pfp != NULL && pfp->ctf_refcnt == 0))
    return (ctf_set_errno (fp, EINVAL));

  if (pfp != NULL && pfp->ctf_dmodel != fp->ctf_dmodel)
    return (ctf_set_errno (fp, ECTF_DMODEL));

  if (fp->ctf_parent && !fp->ctf_parent_unreffed)
    ctf_dict_close (fp->ctf_parent);
  fp->ctf_parent = NULL;

  free (fp->ctf_pptrtab);
  fp->ctf_pptrtab = NULL;
  fp->ctf_pptrtab_len = 0;
  fp->ctf_pptrtab_typemax = 0;

  if (pfp != NULL)
    {
      int err;

      if (fp->ctf_parname == NULL)
	if ((err = ctf_parent_name_set (fp, "PARENT")) < 0)
	  return err;

      fp->ctf_flags |= LCTF_CHILD;
      pfp->ctf_refcnt++;
      fp->ctf_parent_unreffed = 0;
    }

  fp->ctf_parent = pfp;
  return 0;
}

/* Like ctf_import, but does not increment the refcount on the imported parent
   or close it at any point: as a result it can go away at any time and the
   caller must do all freeing itself.  Used internally to avoid refcount
   loops.  */
int
ctf_import_unref (ctf_dict_t *fp, ctf_dict_t *pfp)
{
  if (fp == NULL || fp == pfp || (pfp != NULL && pfp->ctf_refcnt == 0))
    return (ctf_set_errno (fp, EINVAL));

  if (pfp != NULL && pfp->ctf_dmodel != fp->ctf_dmodel)
    return (ctf_set_errno (fp, ECTF_DMODEL));

  if (fp->ctf_parent && !fp->ctf_parent_unreffed)
    ctf_dict_close (fp->ctf_parent);
  fp->ctf_parent = NULL;

  free (fp->ctf_pptrtab);
  fp->ctf_pptrtab = NULL;
  fp->ctf_pptrtab_len = 0;
  fp->ctf_pptrtab_typemax = 0;
  if (pfp != NULL)
    {
      int err;

      if (fp->ctf_parname == NULL)
	if ((err = ctf_parent_name_set (fp, "PARENT")) < 0)
	  return err;

      fp->ctf_flags |= LCTF_CHILD;
      fp->ctf_parent_unreffed = 1;
    }

  fp->ctf_parent = pfp;
  return 0;
}

/* Set the data model constant for the CTF dict.  */
int
ctf_setmodel (ctf_dict_t *fp, int model)
{
  const ctf_dmodel_t *dp;

  for (dp = _libctf_models; dp->ctd_name != NULL; dp++)
    {
      if (dp->ctd_code == model)
	{
	  fp->ctf_dmodel = dp;
	  return 0;
	}
    }

  return (ctf_set_errno (fp, EINVAL));
}

/* Return the data model constant for the CTF dict.  */
int
ctf_getmodel (ctf_dict_t *fp)
{
  return fp->ctf_dmodel->ctd_code;
}

/* The caller can hang an arbitrary pointer off each ctf_dict_t using this
   function.  */
void
ctf_setspecific (ctf_dict_t *fp, void *data)
{
  fp->ctf_specific = data;
}

/* Retrieve the arbitrary pointer again.  */
void *
ctf_getspecific (ctf_dict_t *fp)
{
  return fp->ctf_specific;
}
