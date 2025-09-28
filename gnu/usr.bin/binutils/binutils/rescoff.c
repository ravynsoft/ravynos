/* rescoff.c -- read and write resources in Windows COFF files.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.
   Rewritten by Kai Tietz, Onevision.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file contains function that read and write Windows resources
   in COFF files.  */

#include "sysdep.h"
#include "bfd.h"
#include "bucomm.h"
#include "libiberty.h"
#include "windres.h"

#include <assert.h>

/* In order to use the address of a resource data entry, we need to
   get the image base of the file.  Right now we extract it from
   internal BFD information.  FIXME.  */

#include "coff/internal.h"
#include "libcoff.h"

/* Information we extract from the file.  */

struct coff_file_info
{
  /* File name.  */
  const char *filename;
  /* Data read from the file.  */
  const bfd_byte *data;
  /* End of data read from file.  */
  const bfd_byte *data_end;
  /* Address of the resource section minus the image base of the file.  */
  rc_uint_type secaddr;
};

/* A resource directory table in a COFF file.  */

struct __attribute__ ((__packed__)) extern_res_directory
{
  /* Characteristics.  */
  bfd_byte characteristics[4];
  /* Time stamp.  */
  bfd_byte time[4];
  /* Major version number.  */
  bfd_byte major[2];
  /* Minor version number.  */
  bfd_byte minor[2];
  /* Number of named directory entries.  */
  bfd_byte name_count[2];
  /* Number of directory entries with IDs.  */
  bfd_byte id_count[2];
};

/* A resource directory entry in a COFF file.  */

struct extern_res_entry
{
  /* Name or ID.  */
  bfd_byte name[4];
  /* Address of resource entry or subdirectory.  */
  bfd_byte rva[4];
};

/* A resource data entry in a COFF file.  */

struct extern_res_data
{
  /* Address of resource data.  This is apparently a file relative
     address, rather than a section offset.  */
  bfd_byte rva[4];
  /* Size of resource data.  */
  bfd_byte size[4];
  /* Code page.  */
  bfd_byte codepage[4];
  /* Reserved.  */
  bfd_byte reserved[4];
};

/* Local functions.  */

static void overrun (const struct coff_file_info *, const char *);
static rc_res_directory *read_coff_res_dir (windres_bfd *, const bfd_byte *,
					    const struct coff_file_info *,
					    const rc_res_id *, int);
static rc_res_resource *read_coff_data_entry (windres_bfd *, const bfd_byte *,
					      const struct coff_file_info *,
					      const rc_res_id *);

/* Read the resources in a COFF file.  */

rc_res_directory *
read_coff_rsrc (const char *filename, const char *target)
{
  rc_res_directory *ret;
  bfd *abfd;
  windres_bfd wrbfd;
  char **matching;
  asection *sec;
  bfd_size_type size;
  bfd_byte *data;
  struct coff_file_info flaginfo;

  if (filename == NULL)
    fatal (_("filename required for COFF input"));

  abfd = bfd_openr (filename, target);
  if (abfd == NULL)
    bfd_fatal (filename);

  if (! bfd_check_format_matches (abfd, bfd_object, &matching))
    {
      bfd_nonfatal (bfd_get_filename (abfd));
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	list_matching_formats (matching);
      xexit (1);
    }

  sec = bfd_get_section_by_name (abfd, ".rsrc");
  if (sec == NULL)
    {
      fatal (_("%s: no resource section"), filename);
    }

  set_windres_bfd (&wrbfd, abfd, sec, WR_KIND_BFD);
  size = bfd_section_size (sec);
  /* PR 17512: file: 1b25ba5d
     The call to get_file_size here may be expensive
     but there is no other way to determine if the section size
     is reasonable.  */
  if (size > (bfd_size_type) get_file_size (filename))
    fatal (_("%s: .rsrc section is bigger than the file!"), filename);

  data = (bfd_byte *) res_alloc (size);
  get_windres_bfd_content (&wrbfd, data, 0, size);

  flaginfo.filename = filename;
  flaginfo.data = data;
  flaginfo.data_end = data + size;
  flaginfo.secaddr = (bfd_section_vma (sec)
		      - pe_data (abfd)->pe_opthdr.ImageBase);

  /* Now just read in the top level resource directory.  Note that we
     don't free data, since we create resource entries that point into
     it.  If we ever want to free up the resource information we read,
     this will have to be cleaned up.  */

  ret = read_coff_res_dir (&wrbfd, data, &flaginfo, (const rc_res_id *) NULL, 0);

  bfd_close (abfd);

  return ret;
}

/* Give an error if we are out of bounds.  */

static void
overrun (const struct coff_file_info *flaginfo, const char *msg)
{
  fatal (_("%s: %s: address out of bounds"), flaginfo->filename, msg);
}

/* Read a resource directory.  */

static rc_res_directory *
read_coff_res_dir (windres_bfd *wrbfd, const bfd_byte *data,
		   const struct coff_file_info *flaginfo,
		   const rc_res_id *type, int level)
{
  const struct extern_res_directory *erd;
  rc_res_directory *rd;
  int name_count, id_count, i;
  rc_res_entry **pp;
  const struct extern_res_entry *ere;

  /* PR 17512: file: 09d80f53.
     Whilst in theory resources can nest to any level, in practice
     Microsoft only defines 3 levels.  Corrupt files however might
     claim to use more.  */
  if (level > 4)
    overrun (flaginfo, _("Resources nest too deep"));

  if ((size_t) (flaginfo->data_end - data) < sizeof (struct extern_res_directory))
    overrun (flaginfo, _("directory"));

  erd = (const struct extern_res_directory *) data;

  rd = (rc_res_directory *) res_alloc (sizeof (rc_res_directory));
  rd->characteristics = windres_get_32 (wrbfd, erd->characteristics, 4);
  rd->time = windres_get_32 (wrbfd, erd->time, 4);
  rd->major = windres_get_16 (wrbfd, erd->major, 2);
  rd->minor = windres_get_16 (wrbfd, erd->minor, 2);
  rd->entries = NULL;

  name_count = windres_get_16 (wrbfd, erd->name_count, 2);
  id_count = windres_get_16 (wrbfd, erd->id_count, 2);

  pp = &rd->entries;

  /* The resource directory entries immediately follow the directory
     table.  */
  ere = (const struct extern_res_entry *) (erd + 1);

  for (i = 0; i < name_count; i++, ere++)
    {
      rc_uint_type name, rva;
      rc_res_entry *re;
      const bfd_byte *ers;
      int length, j;

      if ((const bfd_byte *) ere >= flaginfo->data_end)
	overrun (flaginfo, _("named directory entry"));

      name = windres_get_32 (wrbfd, ere->name, 4);
      rva = windres_get_32 (wrbfd, ere->rva, 4);

      /* For some reason the high bit in NAME is set.  */
      name &=~ 0x80000000;

      if (name > (rc_uint_type) (flaginfo->data_end - flaginfo->data))
	overrun (flaginfo, _("directory entry name"));

      ers = flaginfo->data + name;

      re = (rc_res_entry *) res_alloc (sizeof *re);
      re->next = NULL;
      re->id.named = 1;
      length = windres_get_16 (wrbfd, ers, 2);
      re->id.u.n.length = length;
      re->id.u.n.name = (unichar *) res_alloc (length * sizeof (unichar));
      for (j = 0; j < length; j++)
	{
	  /* PR 17512: file: 05dc4a16.  */
	  if (length < 0 || ers >= flaginfo->data_end || ers + j * 2 + 4 >= flaginfo->data_end)
	    overrun (flaginfo, _("resource name"));
	  re->id.u.n.name[j] = windres_get_16 (wrbfd, ers + j * 2 + 2, 2);
	}

      if (level == 0)
	type = &re->id;

      if ((rva & 0x80000000) != 0)
	{
	  rva &=~ 0x80000000;
	  if (rva >= (rc_uint_type) (flaginfo->data_end - flaginfo->data))
	    overrun (flaginfo, _("named subdirectory"));
	  re->subdir = 1;
	  re->u.dir = read_coff_res_dir (wrbfd, flaginfo->data + rva, flaginfo, type,
					 level + 1);
	}
      else
	{
	  if (rva >= (rc_uint_type) (flaginfo->data_end - flaginfo->data))
	    overrun (flaginfo, _("named resource"));
	  re->subdir = 0;
	  re->u.res = read_coff_data_entry (wrbfd, flaginfo->data + rva, flaginfo, type);
	}

      *pp = re;
      pp = &re->next;
    }

  for (i = 0; i < id_count; i++, ere++)
    {
      unsigned long name, rva;
      rc_res_entry *re;

      if ((const bfd_byte *) ere >= flaginfo->data_end)
	overrun (flaginfo, _("ID directory entry"));

      name = windres_get_32 (wrbfd, ere->name, 4);
      rva = windres_get_32 (wrbfd, ere->rva, 4);

      re = (rc_res_entry *) res_alloc (sizeof *re);
      re->next = NULL;
      re->id.named = 0;
      re->id.u.id = name;

      if (level == 0)
	type = &re->id;

      if ((rva & 0x80000000) != 0)
	{
	  rva &=~ 0x80000000;
	  if (rva >= (rc_uint_type) (flaginfo->data_end - flaginfo->data))
	    overrun (flaginfo, _("ID subdirectory"));
	  re->subdir = 1;
	  re->u.dir = read_coff_res_dir (wrbfd, flaginfo->data + rva, flaginfo, type,
					 level + 1);
	}
      else
	{
	  if (rva >= (rc_uint_type) (flaginfo->data_end - flaginfo->data))
	    overrun (flaginfo, _("ID resource"));
	  re->subdir = 0;
	  re->u.res = read_coff_data_entry (wrbfd, flaginfo->data + rva, flaginfo, type);
	}

      *pp = re;
      pp = &re->next;
    }

  return rd;
}

/* Read a resource data entry.  */

static rc_res_resource *
read_coff_data_entry (windres_bfd *wrbfd, const bfd_byte *data,
		      const struct coff_file_info *flaginfo,
		      const rc_res_id *type)
{
  const struct extern_res_data *erd;
  rc_res_resource *r;
  rc_uint_type size, rva;
  const bfd_byte *resdata;

  if (type == NULL)
    fatal (_("resource type unknown"));

  if ((size_t) (flaginfo->data_end - data) < sizeof (struct extern_res_data))
    overrun (flaginfo, _("data entry"));

  erd = (const struct extern_res_data *) data;

  size = windres_get_32 (wrbfd, erd->size, 4);
  rva = windres_get_32 (wrbfd, erd->rva, 4);
  if (rva < flaginfo->secaddr
      || rva - flaginfo->secaddr >= (rc_uint_type) (flaginfo->data_end - flaginfo->data))
    overrun (flaginfo, _("resource data"));

  resdata = flaginfo->data + (rva - flaginfo->secaddr);

  if (size > (rc_uint_type) (flaginfo->data_end - resdata))
    overrun (flaginfo, _("resource data size"));

  r = bin_to_res (wrbfd, *type, resdata, size);

  memset (&r->res_info, 0, sizeof (rc_res_res_info));
  r->coff_info.codepage = windres_get_32 (wrbfd, erd->codepage, 4);
  r->coff_info.reserved = windres_get_32 (wrbfd, erd->reserved, 4);

  return r;
}

/* This structure is used to build a list of bindata structures.  */

struct bindata_build
{
  /* The data.  */
  bindata *d;
  /* The last structure we have added to the list.  */
  bindata *last;
  /* The size of the list as a whole.  */
  unsigned long length;
};

struct coff_res_data_build
{
  /* The data.  */
  coff_res_data *d;
  /* The last structure we have added to the list.  */
  coff_res_data *last;
  /* The size of the list as a whole.  */
  unsigned long length;
};

/* This structure keeps track of information as we build the directory
   tree.  */

struct coff_write_info
{
  /* These fields are based on the BFD.  */
  /* The BFD itself.  */
  windres_bfd *wrbfd;
  /* Pointer to section symbol used to build RVA relocs.  */
  asymbol **sympp;

  /* These fields are computed initially, and then not changed.  */
  /* Length of directory tables and entries.  */
  unsigned long dirsize;
  /* Length of directory entry strings.  */
  unsigned long dirstrsize;
  /* Length of resource data entries.  */
  unsigned long dataentsize;

  /* These fields are updated as we add data.  */
  /* Directory tables and entries.  */
  struct bindata_build dirs;
  /* Directory entry strings.  */
  struct bindata_build dirstrs;
  /* Resource data entries.  */
  struct bindata_build dataents;
  /* Actual resource data.  */
  struct coff_res_data_build resources;
  /* Relocations.  */
  arelent **relocs;
  /* Number of relocations.  */
  unsigned int reloc_count;
};

static void coff_bin_sizes (const rc_res_directory *, struct coff_write_info *);
static bfd_byte *coff_alloc (struct bindata_build *, rc_uint_type);
static void coff_to_bin
  (const rc_res_directory *, struct coff_write_info *);
static void coff_res_to_bin
  (const rc_res_resource *, struct coff_write_info *);

/* Write resources to a COFF file.  RESOURCES should already be
   sorted.

   Right now we always create a new file.  Someday we should also
   offer the ability to merge resources into an existing file.  This
   would require doing the basic work of objcopy, just modifying or
   adding the .rsrc section.  */

void
write_coff_file (const char *filename, const char *target,
		 const rc_res_directory *resources)
{
  bfd *abfd;
  asection *sec;
  struct coff_write_info cwi;
  windres_bfd wrbfd;
  bindata *d;
  coff_res_data *rd;
  unsigned long length, offset;

  if (filename == NULL)
    fatal (_("filename required for COFF output"));

  abfd = bfd_openw (filename, target);
  if (abfd == NULL)
    bfd_fatal (filename);

  if (! bfd_set_format (abfd, bfd_object))
    bfd_fatal ("bfd_set_format");

#if defined DLLTOOL_SH
  if (! bfd_set_arch_mach (abfd, bfd_arch_sh, 0))
    bfd_fatal ("bfd_set_arch_mach(sh)");
#elif defined DLLTOOL_MIPS
  if (! bfd_set_arch_mach (abfd, bfd_arch_mips, 0))
    bfd_fatal ("bfd_set_arch_mach(mips)");
#elif defined DLLTOOL_ARM
  if (! bfd_set_arch_mach (abfd, bfd_arch_arm, 0))
    bfd_fatal ("bfd_set_arch_mach(arm)");
#elif defined DLLTOOL_AARCH64
  if (! bfd_set_arch_mach (abfd, bfd_arch_aarch64, 0))
    bfd_fatal ("bfd_set_arch_mach(aarch64)");
#else
  /* FIXME: This is obviously i386 specific.  */
  if (! bfd_set_arch_mach (abfd, bfd_arch_i386, 0))
    bfd_fatal ("bfd_set_arch_mach(i386)");
#endif

  if (! bfd_set_file_flags (abfd, HAS_SYMS | HAS_RELOC))
    bfd_fatal ("bfd_set_file_flags");

  sec = bfd_make_section_with_flags (abfd, ".rsrc",
				     (SEC_HAS_CONTENTS | SEC_ALLOC
				      | SEC_LOAD | SEC_DATA | SEC_READONLY));
  if (sec == NULL)
    bfd_fatal ("bfd_make_section");

  if (! bfd_set_symtab (abfd, sec->symbol_ptr_ptr, 1))
    bfd_fatal ("bfd_set_symtab");

  /* Requiring this is probably a bug in BFD.  */
  sec->output_section = sec;

  /* The order of data in the .rsrc section is
       resource directory tables and entries
       resource directory strings
       resource data entries
       actual resource data

     We build these different types of data in different lists.  */

  set_windres_bfd (&wrbfd, abfd, sec, WR_KIND_BFD);

  cwi.wrbfd = &wrbfd;
  cwi.sympp = sec->symbol_ptr_ptr;
  cwi.dirsize = 0;
  cwi.dirstrsize = 0;
  cwi.dataentsize = 0;
  cwi.dirs.d = NULL;
  cwi.dirs.last = NULL;
  cwi.dirs.length = 0;
  cwi.dirstrs.d = NULL;
  cwi.dirstrs.last = NULL;
  cwi.dirstrs.length = 0;
  cwi.dataents.d = NULL;
  cwi.dataents.last = NULL;
  cwi.dataents.length = 0;
  cwi.resources.d = NULL;
  cwi.resources.last = NULL;
  cwi.resources.length = 0;
  cwi.relocs = NULL;
  cwi.reloc_count = 0;

  /* Work out the sizes of the resource directory entries, so that we
     know the various offsets we will need.  */
  coff_bin_sizes (resources, &cwi);

  /* Force the directory strings to be 64 bit aligned.  Every other
     structure is 64 bit aligned anyhow.  */
  cwi.dirstrsize = (cwi.dirstrsize + 7) & ~7;

  /* Actually convert the resources to binary.  */
  coff_to_bin (resources, &cwi);

  /* Add another few bytes to the directory strings if needed for
     alignment.  */
  if ((cwi.dirstrs.length & 7) != 0)
    {
      rc_uint_type pad = 8 - (cwi.dirstrs.length & 7);
      bfd_byte *ex;

      ex = coff_alloc (& cwi.dirstrs, pad);
      memset (ex, 0, pad);
    }

  /* Make sure that the data we built came out to the same size as we
     calculated initially.  */
  assert (cwi.dirs.length == cwi.dirsize);
  assert (cwi.dirstrs.length == cwi.dirstrsize);
  assert (cwi.dataents.length == cwi.dataentsize);

  length = (cwi.dirsize
	    + cwi.dirstrsize
	    + cwi.dataentsize
	    + cwi.resources.length);

  if (!bfd_set_section_size (sec, length))
    bfd_fatal ("bfd_set_section_size");

  bfd_set_reloc (abfd, sec, cwi.relocs, cwi.reloc_count);

  offset = 0;
  for (d = cwi.dirs.d; d != NULL; d = d->next)
    {
      if (! bfd_set_section_contents (abfd, sec, d->data, offset, d->length))
	bfd_fatal ("bfd_set_section_contents");
      offset += d->length;
    }
  for (d = cwi.dirstrs.d; d != NULL; d = d->next)
    {
      set_windres_bfd_content (&wrbfd, d->data, offset, d->length);
      offset += d->length;
    }
  for (d = cwi.dataents.d; d != NULL; d = d->next)
    {
      set_windres_bfd_content (&wrbfd, d->data, offset, d->length);
      offset += d->length;
    }
  for (rd = cwi.resources.d; rd != NULL; rd = rd->next)
    {
      res_to_bin (cwi.wrbfd, (rc_uint_type) offset, rd->res);
      offset += rd->length;
    }

  assert (offset == length);

  if (! bfd_close (abfd))
    bfd_fatal ("bfd_close");

  /* We allocated the relocs array using malloc.  */
  free (cwi.relocs);
}

/* Work out the sizes of the various fixed size resource directory
   entries.  This updates fields in CWI.  */

static void
coff_bin_sizes (const rc_res_directory *resdir,
		struct coff_write_info *cwi)
{
  const rc_res_entry *re;

  cwi->dirsize += sizeof (struct extern_res_directory);

  for (re = resdir->entries; re != NULL; re = re->next)
    {
      cwi->dirsize += sizeof (struct extern_res_entry);

      if (re->id.named)
	cwi->dirstrsize += re->id.u.n.length * 2 + 2;

      if (re->subdir)
	coff_bin_sizes (re->u.dir, cwi);
      else
	cwi->dataentsize += sizeof (struct extern_res_data);
    }
}

/* Allocate data for a particular list.  */

static bfd_byte *
coff_alloc (struct bindata_build *bb, rc_uint_type size)
{
  bindata *d;

  d = (bindata *) reswr_alloc (sizeof (bindata));

  d->next = NULL;
  d->data = (bfd_byte *) reswr_alloc (size);
  d->length = size;

  if (bb->d == NULL)
    bb->d = d;
  else
    bb->last->next = d;
  bb->last = d;
  bb->length += size;

  return d->data;
}

/* Convert the resource directory RESDIR to binary.  */

static void
coff_to_bin (const rc_res_directory *resdir, struct coff_write_info *cwi)
{
  struct extern_res_directory *erd;
  int ci, cn;
  const rc_res_entry *e;
  struct extern_res_entry *ere;

  /* Write out the directory table.  */

  erd = ((struct extern_res_directory *)
	 coff_alloc (&cwi->dirs, sizeof (*erd)));

  windres_put_32 (cwi->wrbfd, erd->characteristics, resdir->characteristics);
  windres_put_32 (cwi->wrbfd, erd->time, resdir->time);
  windres_put_16 (cwi->wrbfd, erd->major, resdir->major);
  windres_put_16 (cwi->wrbfd, erd->minor, resdir->minor);

  ci = 0;
  cn = 0;
  for (e = resdir->entries; e != NULL; e = e->next)
    {
      if (e->id.named)
	++cn;
      else
	++ci;
    }

  windres_put_16 (cwi->wrbfd, erd->name_count, cn);
  windres_put_16 (cwi->wrbfd, erd->id_count, ci);

  /* Write out the data entries.  Note that we allocate space for all
     the entries before writing them out.  That permits a recursive
     call to work correctly when writing out subdirectories.  */

  ere = ((struct extern_res_entry *)
	 coff_alloc (&cwi->dirs, (ci + cn) * sizeof (*ere)));
  for (e = resdir->entries; e != NULL; e = e->next, ere++)
    {
      if (! e->id.named)
	windres_put_32 (cwi->wrbfd, ere->name, e->id.u.id);
      else
	{
	  bfd_byte *str;
	  rc_uint_type i;

	  /* For some reason existing files seem to have the high bit
             set on the address of the name, although that is not
             documented.  */
	  windres_put_32 (cwi->wrbfd, ere->name,
		     0x80000000 | (cwi->dirsize + cwi->dirstrs.length));

	  str = coff_alloc (&cwi->dirstrs, e->id.u.n.length * 2 + 2);
	  windres_put_16 (cwi->wrbfd, str, e->id.u.n.length);
	  for (i = 0; i < e->id.u.n.length; i++)
	    windres_put_16 (cwi->wrbfd, str + (i + 1) * sizeof (unichar), e->id.u.n.name[i]);
	}

      if (e->subdir)
	{
	  windres_put_32 (cwi->wrbfd, ere->rva, 0x80000000 | cwi->dirs.length);
	  coff_to_bin (e->u.dir, cwi);
	}
      else
	{
	  windres_put_32 (cwi->wrbfd, ere->rva,
		     cwi->dirsize + cwi->dirstrsize + cwi->dataents.length);

	  coff_res_to_bin (e->u.res, cwi);
	}
    }
}

/* Convert the resource RES to binary.  */

static void
coff_res_to_bin (const rc_res_resource *res, struct coff_write_info *cwi)
{
  arelent *r;
  struct extern_res_data *erd;
  coff_res_data *d;

  /* For some reason, although every other address is a section
     offset, the address of the resource data itself is an RVA.  That
     means that we need to generate a relocation for it.  We allocate
     the relocs array using malloc so that we can use realloc.  FIXME:
     This relocation handling is correct for the i386, but probably
     not for any other target.  */

  r = (arelent *) reswr_alloc (sizeof (arelent));
  r->sym_ptr_ptr = cwi->sympp;
  r->address = cwi->dirsize + cwi->dirstrsize + cwi->dataents.length;
  r->addend = 0;
  r->howto = bfd_reloc_type_lookup (WR_BFD (cwi->wrbfd), BFD_RELOC_RVA);
  if (r->howto == NULL)
    bfd_fatal (_("can't get BFD_RELOC_RVA relocation type"));

  cwi->relocs = xrealloc (cwi->relocs,
			  (cwi->reloc_count + 2) * sizeof (arelent *));
  cwi->relocs[cwi->reloc_count] = r;
  cwi->relocs[cwi->reloc_count + 1] = NULL;
  ++cwi->reloc_count;

  erd = (struct extern_res_data *) coff_alloc (&cwi->dataents, sizeof (*erd));

  windres_put_32 (cwi->wrbfd, erd->rva,
	     (cwi->dirsize
	      + cwi->dirstrsize
	      + cwi->dataentsize
	      + cwi->resources.length));
  windres_put_32 (cwi->wrbfd, erd->codepage, res->coff_info.codepage);
  windres_put_32 (cwi->wrbfd, erd->reserved, res->coff_info.reserved);

  d = (coff_res_data *) reswr_alloc (sizeof (coff_res_data));
  d->length = res_to_bin (NULL, (rc_uint_type) 0, res);
  d->res = res;
  d->next = NULL;

  if (cwi->resources.d == NULL)
    cwi->resources.d = d;
  else
    cwi->resources.last->next = d;

  cwi->resources.last = d;
  cwi->resources.length += (d->length + 7) & ~7;

  windres_put_32 (cwi->wrbfd, erd->size, d->length);

  /* Force the next resource to have 64 bit alignment.  */
  d->length = (d->length + 7) & ~7;
}
