/* BFD back-end for VMS archive files.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

   This file is part of BFD, the Binary File Descriptor library.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "safe-ctype.h"
#include "bfdver.h"
#include "libiberty.h"
#include "vms.h"
#include "vms/lbr.h"
#include "vms/dcx.h"

/* The standard VMS disk block size.  */
#ifndef VMS_BLOCK_SIZE
#define VMS_BLOCK_SIZE 512
#endif

/* Maximum key length (which is also the maximum symbol length in archive).  */
#define MAX_KEYLEN 128
#define MAX_EKEYLEN 1024

/* DCX Submaps.  */

struct dcxsbm_desc
{
  unsigned char min_char;
  unsigned char max_char;
  unsigned char *flags;
  unsigned char *nodes;
  unsigned short *next;
};

/* Kind of library.  Used to filter in archive_p.  */

enum vms_lib_kind
  {
    vms_lib_vax,
    vms_lib_alpha,
    vms_lib_ia64,
    vms_lib_txt
  };

/* Back-end private data.  */

struct lib_tdata
{
  /* Standard tdata for an archive.  But we don't use many fields.  */
  struct artdata artdata;

  /* Major version.  */
  unsigned char ver;

  /* Type of the archive.  */
  unsigned char type;

  /* Kind of archive.  Summary of its type.  */
  enum vms_lib_kind kind;

  /* Total size of the mhd (element header).  */
  unsigned int mhd_size;

  /* Creation date.  */
  unsigned int credat_lo;
  unsigned int credat_hi;

  /* Vector of modules (archive elements), already sorted.  */
  unsigned int nbr_modules;
  struct carsym *modules;
  bfd **cache;

  /* DCX (decompression) data.  */
  unsigned int nbr_dcxsbm;
  struct dcxsbm_desc *dcxsbm;
};

#define bfd_libdata(bfd) ((struct lib_tdata *)((bfd)->tdata.any))

/* End-Of-Text pattern.  This is a special record to mark the end of file.  */

static const unsigned char eotdesc[] = { 0x03, 0x00, 0x77, 0x00, 0x77, 0x00 };

/* Describe the current state of carsym entries while building the archive
   table of content.  Things are simple with Alpha archives as the number
   of entries is known, but with IA64 archives a entry can make a reference
   to severals members.  Therefore we must be able to extend the table on the
   fly, but it should be allocated on the bfd - which doesn't support realloc.
   To reduce the overhead, the table is initially allocated in the BFD's
   objalloc and extended if necessary on the heap.  In the later case, it
   is finally copied to the BFD's objalloc so that it will automatically be
   freed.  */

struct carsym_mem
{
  /* The table of content.  */
  struct carsym *idx;

  /* Number of entries used in the table.  */
  unsigned int nbr;

  /* Maximum number of entries.  */
  unsigned int max;

  /* Do not allocate more that this number of entries.  */
  unsigned int limit;

  /* If true, the table was reallocated on the heap.  If false, it is still
     in the BFD's objalloc.  */
  bool realloced;
};

/* Simply add a name to the index.  */

static bool
vms_add_index (struct carsym_mem *cs, char *name,
	       unsigned int idx_vbn, unsigned int idx_off)
{
  if (cs->nbr == cs->max)
    {
      struct carsym *n;
      size_t amt;

      if (cs->max > -33u / 2 || cs->max >= cs->limit)
	{
	  bfd_set_error (bfd_error_file_too_big);
	  return false;
	}
      cs->max = 2 * cs->max + 32;
      if (cs->max > cs->limit)
	cs->max = cs->limit;
      if (_bfd_mul_overflow (cs->max, sizeof (struct carsym), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  return false;
	}

      if (!cs->realloced)
	{
	  n = bfd_malloc (amt);
	  if (n == NULL)
	    return false;
	  memcpy (n, cs->idx, cs->nbr * sizeof (struct carsym));
	  /* And unfortunately we can't free cs->idx.  */
	}
      else
	{
	  n = bfd_realloc_or_free (cs->idx, amt);
	  if (n == NULL)
	    return false;
	}
      cs->idx = n;
      cs->realloced = true;
    }
  cs->idx[cs->nbr].file_offset = (idx_vbn - 1) * VMS_BLOCK_SIZE + idx_off;
  cs->idx[cs->nbr].name = name;
  cs->nbr++;
  return true;
}

/* Follow all member of a lns list (pointed by RFA) and add indexes for
   NAME.  Return FALSE in case of error.  */

static bool
vms_add_indexes_from_list (bfd *abfd, struct carsym_mem *cs, char *name,
			   struct vms_rfa *rfa)
{
  struct vms_lns lns;
  unsigned int vbn;
  file_ptr off;

  while (1)
    {
      vbn = bfd_getl32 (rfa->vbn);
      if (vbn == 0)
	return true;

      /* Read the LHS.  */
      off = (vbn - 1) * VMS_BLOCK_SIZE + bfd_getl16 (rfa->offset);
      if (bfd_seek (abfd, off, SEEK_SET) != 0
	  || bfd_bread (&lns, sizeof (lns), abfd) != sizeof (lns))
	return false;

      if (!vms_add_index (cs, name,
			  bfd_getl32 (lns.modrfa.vbn),
			  bfd_getl16 (lns.modrfa.offset)))
	return false;

      rfa = &lns.nxtrfa;
    }
}

/* Read block VBN from ABFD and store it into BLK.  Return FALSE in case of error.  */

static bool
vms_read_block (bfd *abfd, unsigned int vbn, void *blk)
{
  file_ptr off;

  off = (vbn - 1) * VMS_BLOCK_SIZE;
  if (bfd_seek (abfd, off, SEEK_SET) != 0
      || bfd_bread (blk, VMS_BLOCK_SIZE, abfd) != VMS_BLOCK_SIZE)
    return false;

  return true;
}

/* Write the content of BLK to block VBN of ABFD.  Return FALSE in case of error.  */

static bool
vms_write_block (bfd *abfd, unsigned int vbn, void *blk)
{
  file_ptr off;

  off = (vbn - 1) * VMS_BLOCK_SIZE;
  if (bfd_seek (abfd, off, SEEK_SET) != 0
      || bfd_bwrite (blk, VMS_BLOCK_SIZE, abfd) != VMS_BLOCK_SIZE)
    return false;

  return true;
}

/* Read index block VBN and put the entry in **IDX (which is updated).
   If the entry is indirect, recurse.  */

static bool
vms_traverse_index (bfd *abfd, unsigned int vbn, struct carsym_mem *cs,
		    unsigned int recur_count)
{
  struct vms_indexdef indexdef;
  file_ptr off;
  unsigned char *p;
  unsigned char *endp;
  unsigned int n;

  if (recur_count == 100)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Read the index block.  */
  BFD_ASSERT (sizeof (indexdef) == VMS_BLOCK_SIZE);
  if (!vms_read_block (abfd, vbn, &indexdef))
    return false;

  /* Traverse it.  */
  p = &indexdef.keys[0];
  n = bfd_getl16 (indexdef.used);
  if (n > sizeof (indexdef.keys))
    return false;
  endp = p + n;
  while (p < endp)
    {
      unsigned int idx_vbn;
      unsigned int idx_off;
      unsigned int keylen;
      unsigned char *keyname;
      unsigned int flags;

      /* Extract key length.  */
      if (bfd_libdata (abfd)->ver == LBR_MAJORID
	  && offsetof (struct vms_idx, keyname) <= (size_t) (endp - p))
	{
	  struct vms_idx *ridx = (struct vms_idx *)p;

	  idx_vbn = bfd_getl32 (ridx->rfa.vbn);
	  idx_off = bfd_getl16 (ridx->rfa.offset);

	  keylen = ridx->keylen;
	  flags = 0;
	  keyname = ridx->keyname;
	}
      else if (bfd_libdata (abfd)->ver == LBR_ELFMAJORID
	       && offsetof (struct vms_elfidx, keyname) <= (size_t) (endp - p))
	{
	  struct vms_elfidx *ridx = (struct vms_elfidx *)p;

	  idx_vbn = bfd_getl32 (ridx->rfa.vbn);
	  idx_off = bfd_getl16 (ridx->rfa.offset);

	  keylen = bfd_getl16 (ridx->keylen);
	  flags = ridx->flags;
	  keyname = ridx->keyname;
	}
      else
	return false;

      /* Illegal value.  */
      if (idx_vbn == 0)
	return false;

      /* Point to the next index entry.  */
      p = keyname + keylen;
      if (p > endp)
	return false;

      if (idx_off == RFADEF__C_INDEX)
	{
	  /* Indirect entry.  Recurse.  */
	  if (!vms_traverse_index (abfd, idx_vbn, cs, recur_count + 1))
	    return false;
	}
      else
	{
	  /* Add a new entry.  */
	  char *name;

	  if (flags & ELFIDX__SYMESC)
	    {
	      /* Extended key name.  */
	      unsigned int noff = 0;
	      unsigned int koff;
	      unsigned int kvbn;
	      struct vms_kbn *kbn;
	      unsigned char kblk[VMS_BLOCK_SIZE];

	      /* Sanity check.  */
	      if (keylen != sizeof (struct vms_kbn))
		return false;

	      kbn = (struct vms_kbn *)keyname;
	      keylen = bfd_getl16 (kbn->keylen);

	      name = bfd_alloc (abfd, keylen + 1);
	      if (name == NULL)
		return false;
	      kvbn = bfd_getl32 (kbn->rfa.vbn);
	      koff = bfd_getl16 (kbn->rfa.offset);

	      /* Read the key, chunk by chunk.  */
	      do
		{
		  unsigned int klen;

		  if (!vms_read_block (abfd, kvbn, kblk))
		    return false;
		  if (koff > sizeof (kblk) - sizeof (struct vms_kbn))
		    return false;
		  kbn = (struct vms_kbn *)(kblk + koff);
		  klen = bfd_getl16 (kbn->keylen);
		  if (klen > sizeof (kblk) - sizeof (struct vms_kbn) - koff)
		    return false;
		  kvbn = bfd_getl32 (kbn->rfa.vbn);
		  koff = bfd_getl16 (kbn->rfa.offset);

		  if (noff + klen > keylen)
		    return false;
		  memcpy (name + noff, kbn + 1, klen);
		  noff += klen;
		}
	      while (kvbn != 0);

	      /* Sanity check.  */
	      if (noff != keylen)
		return false;
	    }
	  else
	    {
	      /* Usual key name.  */
	      name = bfd_alloc (abfd, keylen + 1);
	      if (name == NULL)
		return false;

	      memcpy (name, keyname, keylen);
	    }
	  name[keylen] = 0;

	  if (flags & ELFIDX__LISTRFA)
	    {
	      struct vms_lhs lhs;

	      /* Read the LHS.  */
	      off = (idx_vbn - 1) * VMS_BLOCK_SIZE + idx_off;
	      if (bfd_seek (abfd, off, SEEK_SET) != 0
		  || bfd_bread (&lhs, sizeof (lhs), abfd) != sizeof (lhs))
		return false;

	      /* These extra entries may cause reallocation of CS.  */
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.ng_g_rfa))
		return false;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.ng_wk_rfa))
		return false;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.g_g_rfa))
		return false;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.g_wk_rfa))
		return false;
	    }
	  else
	    {
	      if (!vms_add_index (cs, name, idx_vbn, idx_off))
		return false;
	    }
	}
    }

  return true;
}

/* Read index #IDX, which must have NBREL entries.  */

static struct carsym *
vms_lib_read_index (bfd *abfd, int idx, unsigned int *nbrel)
{
  struct vms_idd idd;
  unsigned int flags;
  unsigned int vbn;
  ufile_ptr filesize;
  size_t amt;
  struct carsym *csbuf;
  struct carsym_mem csm;

  /* Read index desription.  */
  if (bfd_seek (abfd, LHD_IDXDESC + idx * IDD_LENGTH, SEEK_SET) != 0
      || bfd_bread (&idd, sizeof (idd), abfd) != sizeof (idd))
    return NULL;

  /* Sanity checks.  */
  flags = bfd_getl16 (idd.flags);
  if (!(flags & IDD__FLAGS_ASCII)
      || !(flags & IDD__FLAGS_VARLENIDX))
    return NULL;

  filesize = bfd_get_file_size (abfd);
  csm.nbr = 0;
  csm.max = *nbrel;
  csm.limit = -1u;
  csm.realloced = false;
  if (filesize != 0)
    {
      /* Put an upper bound based on a file full of single char keys.
	 This is to prevent fuzzed binary silliness.  It is easily
	 possible to set up loops over file blocks that add syms
	 without end.  */
      if (filesize / (sizeof (struct vms_rfa) + 2) <= -1u)
	csm.limit = filesize / (sizeof (struct vms_rfa) + 2);
    }
  if (csm.max > csm.limit)
    csm.max = csm.limit;
  if (_bfd_mul_overflow (csm.max, sizeof (struct carsym), &amt))
    return NULL;
  csm.idx = csbuf = bfd_alloc (abfd, amt);
  if (csm.idx == NULL)
    return NULL;

  /* Note: if the index is empty, there is no block to traverse.  */
  vbn = bfd_getl32 (idd.vbn);
  if (vbn != 0 && !vms_traverse_index (abfd, vbn, &csm, 0))
    {
      if (csm.realloced)
	free (csm.idx);

      /* Note: in case of error, we can free what was allocated on the
	 BFD's objalloc.  */
      bfd_release (abfd, csbuf);
      return NULL;
    }

  if (csm.realloced)
    {
      /* There are more entries than the first estimate.  Allocate on
	 the BFD's objalloc.  */
      csbuf = bfd_alloc (abfd, csm.nbr * sizeof (struct carsym));
      if (csbuf == NULL)
	{
	  free (csm.idx);
	  return NULL;
	}
      memcpy (csbuf, csm.idx, csm.nbr * sizeof (struct carsym));
      free (csm.idx);
      csm.idx = csbuf;
    }
  *nbrel = csm.nbr;
  return csm.idx;
}

/* Standard function.  */

static bfd_cleanup
_bfd_vms_lib_archive_p (bfd *abfd, enum vms_lib_kind kind)
{
  struct vms_lhd lhd;
  unsigned int sanity;
  unsigned int majorid;
  struct lib_tdata *tdata_hold;
  struct lib_tdata *tdata;
  unsigned int dcxvbn;
  unsigned int nbr_ent;

  /* Read header.  */
  if (bfd_bread (&lhd, sizeof (lhd), abfd) != sizeof (lhd))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Check sanity (= magic) number.  */
  sanity = bfd_getl32 (lhd.sanity);
  if (!(sanity == LHD_SANEID3
	|| sanity == LHD_SANEID6
	|| sanity == LHD_SANEID_DCX))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }
  majorid = bfd_getl32 (lhd.majorid);

  /* Check archive kind.  */
  switch (kind)
    {
    case vms_lib_alpha:
      if ((lhd.type != LBR__C_TYP_EOBJ && lhd.type != LBR__C_TYP_ESHSTB)
	  || majorid != LBR_MAJORID
	  || lhd.nindex != 2)
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
      break;
    case vms_lib_ia64:
      if ((lhd.type != LBR__C_TYP_IOBJ && lhd.type != LBR__C_TYP_ISHSTB)
	  || majorid != LBR_ELFMAJORID
	  || lhd.nindex != 2)
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
      break;
    case vms_lib_txt:
      if ((lhd.type != LBR__C_TYP_TXT
	   && lhd.type != LBR__C_TYP_MLB
	   && lhd.type != LBR__C_TYP_HLP)
	  || majorid != LBR_MAJORID
	  || lhd.nindex != 1)
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
      break;
    default:
      abort ();
    }

  /* Allocate and initialize private data.  */
  tdata_hold = bfd_libdata (abfd);
  tdata = (struct lib_tdata *) bfd_zalloc (abfd, sizeof (struct lib_tdata));
  if (tdata == NULL)
    return NULL;
  abfd->tdata.any = (void *)tdata;
  tdata->ver = majorid;
  tdata->mhd_size = MHD__C_USRDAT + lhd.mhdusz;
  tdata->type = lhd.type;
  tdata->kind = kind;
  tdata->credat_lo = bfd_getl32 (lhd.credat + 0);
  tdata->credat_hi = bfd_getl32 (lhd.credat + 4);

  /* Read indexes.  */
  tdata->nbr_modules = bfd_getl32 (lhd.modcnt);
  tdata->artdata.symdef_count = bfd_getl32 (lhd.idxcnt) - tdata->nbr_modules;
  nbr_ent = tdata->nbr_modules;
  tdata->modules = vms_lib_read_index (abfd, 0, &nbr_ent);
  if (tdata->modules == NULL || nbr_ent != tdata->nbr_modules)
    goto err;
  if (lhd.nindex == 2)
    {
      nbr_ent = tdata->artdata.symdef_count;
      tdata->artdata.symdefs = vms_lib_read_index (abfd, 1, &nbr_ent);
      if (tdata->artdata.symdefs == NULL)
	goto err;
      /* Only IA64 archives may have more entries in the index that what
	 was declared.  */
      if (nbr_ent != tdata->artdata.symdef_count
	  && kind != vms_lib_ia64)
	goto err;
      tdata->artdata.symdef_count = nbr_ent;
    }
  tdata->cache = bfd_zalloc (abfd, sizeof (bfd *) * tdata->nbr_modules);
  if (tdata->cache == NULL)
    goto err;

  /* Read DCX submaps.  */
  dcxvbn = bfd_getl32 (lhd.dcxmapvbn);
  if (dcxvbn != 0)
    {
      unsigned char buf_reclen[4];
      unsigned int reclen;
      unsigned char *buf;
      struct vms_dcxmap *map;
      unsigned int sbm_off;
      unsigned int i;

      if (bfd_seek (abfd, (dcxvbn - 1) * VMS_BLOCK_SIZE, SEEK_SET) != 0
	  || bfd_bread (buf_reclen, sizeof (buf_reclen), abfd)
	  != sizeof (buf_reclen))
	goto err;
      reclen = bfd_getl32 (buf_reclen);
      if (reclen < sizeof (struct vms_dcxmap))
	goto err;
      buf = _bfd_malloc_and_read (abfd, reclen, reclen);
      if (buf == NULL)
	goto err;
      map = (struct vms_dcxmap *)buf;
      tdata->nbr_dcxsbm = bfd_getl16 (map->nsubs);
      sbm_off = bfd_getl16 (map->sub0);
      tdata->dcxsbm = (struct dcxsbm_desc *)bfd_alloc
	(abfd, tdata->nbr_dcxsbm * sizeof (struct dcxsbm_desc));
      for (i = 0; i < tdata->nbr_dcxsbm; i++)
	{
	  struct vms_dcxsbm *sbm;
	  struct dcxsbm_desc *sbmdesc = &tdata->dcxsbm[i];
	  unsigned int sbm_len;
	  unsigned int sbm_sz;
	  unsigned int off;
	  unsigned char *buf1;
	  unsigned int l, j;

	  if (sbm_off > reclen
	      || reclen - sbm_off < sizeof (struct vms_dcxsbm))
	    {
	    err_free_buf:
	      free (buf);
	      goto err;
	    }
	  sbm = (struct vms_dcxsbm *) (buf + sbm_off);
	  sbm_sz = bfd_getl16 (sbm->size);
	  sbm_off += sbm_sz;
	  if (sbm_off > reclen)
	    goto err_free_buf;

	  sbmdesc->min_char = sbm->min_char;
	  BFD_ASSERT (sbmdesc->min_char == 0);
	  sbmdesc->max_char = sbm->max_char;
	  sbm_len = sbmdesc->max_char - sbmdesc->min_char + 1;
	  l = (2 * sbm_len + 7) / 8;
	  if (sbm_sz < sizeof (struct vms_dcxsbm) + l + sbm_len
	      || (tdata->nbr_dcxsbm > 1
		  && sbm_sz < sizeof (struct vms_dcxsbm) + l + 3 * sbm_len))
	    goto err_free_buf;
	  sbmdesc->flags = (unsigned char *)bfd_alloc (abfd, l);
	  off = bfd_getl16 (sbm->flags);
	  if (off > sbm_sz
	      || sbm_sz - off < l)
	    goto err_free_buf;
	  memcpy (sbmdesc->flags, (bfd_byte *) sbm + off, l);
	  sbmdesc->nodes = (unsigned char *)bfd_alloc (abfd, 2 * sbm_len);
	  off = bfd_getl16 (sbm->nodes);
	  if (off > sbm_sz
	      || sbm_sz - off < 2 * sbm_len)
	    goto err_free_buf;
	  memcpy (sbmdesc->nodes, (bfd_byte *) sbm + off, 2 * sbm_len);
	  off = bfd_getl16 (sbm->next);
	  if (off != 0)
	    {
	      if (off > sbm_sz
		  || sbm_sz - off < 2 * sbm_len)
		goto err_free_buf;
	      /* Read the 'next' array.  */
	      sbmdesc->next = (unsigned short *) bfd_alloc (abfd, 2 * sbm_len);
	      buf1 = (bfd_byte *) sbm + off;
	      for (j = 0; j < sbm_len; j++)
		sbmdesc->next[j] = bfd_getl16 (buf1 + j * 2);
	    }
	  else
	    {
	      /* There is no next array if there is only one submap.  */
	      BFD_ASSERT (tdata->nbr_dcxsbm == 1);
	      sbmdesc->next = NULL;
	    }
	}
      free (buf);
    }
  else
    {
      tdata->nbr_dcxsbm = 0;
    }

  /* The map is always present.  Also mark shared image library.  */
  abfd->has_armap = true;
  if (tdata->type == LBR__C_TYP_ESHSTB || tdata->type == LBR__C_TYP_ISHSTB)
    abfd->is_thin_archive = true;

  return _bfd_no_cleanup;

 err:
  bfd_release (abfd, tdata);
  abfd->tdata.any = (void *)tdata_hold;
  return NULL;
}

/* Standard function for alpha libraries.  */

bfd_cleanup
_bfd_vms_lib_alpha_archive_p (bfd *abfd)
{
  return _bfd_vms_lib_archive_p (abfd, vms_lib_alpha);
}

/* Standard function for ia64 libraries.  */

bfd_cleanup
_bfd_vms_lib_ia64_archive_p (bfd *abfd)
{
  return _bfd_vms_lib_archive_p (abfd, vms_lib_ia64);
}

/* Standard function for text libraries.  */

static bfd_cleanup
_bfd_vms_lib_txt_archive_p (bfd *abfd)
{
  return _bfd_vms_lib_archive_p (abfd, vms_lib_txt);
}

/* Standard bfd function.  */

static bool
_bfd_vms_lib_mkarchive (bfd *abfd, enum vms_lib_kind kind)
{
  struct lib_tdata *tdata;

  tdata = (struct lib_tdata *) bfd_zalloc (abfd, sizeof (struct lib_tdata));
  if (tdata == NULL)
    return false;

  abfd->tdata.any = (void *)tdata;
  vms_get_time (&tdata->credat_hi, &tdata->credat_lo);

  tdata->kind = kind;
  switch (kind)
    {
    case vms_lib_alpha:
      tdata->ver = LBR_MAJORID;
      tdata->mhd_size = offsetof (struct vms_mhd, pad1);
      tdata->type = LBR__C_TYP_EOBJ;
      break;
    case vms_lib_ia64:
      tdata->ver = LBR_ELFMAJORID;
      tdata->mhd_size = sizeof (struct vms_mhd);
      tdata->type = LBR__C_TYP_IOBJ;
      break;
    default:
      abort ();
    }

  tdata->nbr_modules = 0;
  tdata->artdata.symdef_count = 0;
  tdata->modules = NULL;
  tdata->artdata.symdefs = NULL;
  tdata->cache = NULL;

  return true;
}

bool
_bfd_vms_lib_alpha_mkarchive (bfd *abfd)
{
  return _bfd_vms_lib_mkarchive (abfd, vms_lib_alpha);
}

bool
_bfd_vms_lib_ia64_mkarchive (bfd *abfd)
{
  return _bfd_vms_lib_mkarchive (abfd, vms_lib_ia64);
}

/* Find NAME in the symbol index.  Return the index.  */

symindex
_bfd_vms_lib_find_symbol (bfd *abfd, const char *name)
{
  struct lib_tdata *tdata = bfd_libdata (abfd);
  carsym *syms = tdata->artdata.symdefs;
  int lo, hi;

  /* Open-coded binary search for speed.  */
  lo = 0;
  hi = tdata->artdata.symdef_count - 1;

  while (lo <= hi)
    {
      int mid = lo + (hi - lo) / 2;
      int diff;

      diff = (char)(name[0] - syms[mid].name[0]);
      if (diff == 0)
	diff = strcmp (name, syms[mid].name);
      if (diff == 0)
	return mid;
      else if (diff < 0)
	hi = mid - 1;
      else
	lo = mid + 1;
    }
  return BFD_NO_MORE_SYMBOLS;
}

/* IO vector for archive member.  Need that because members are not linearly
   stored in archives.  */

struct vms_lib_iovec
{
  /* Current offset.  */
  ufile_ptr where;

  /* Length of the module, when known.  */
  ufile_ptr file_len;

  /* Current position in the record from bfd_bread point of view (ie, after
     decompression).  0 means that no data byte have been read, -2 and -1
     are reserved for the length word.  */
  int rec_pos;
#define REC_POS_NL   -4
#define REC_POS_PAD  -3
#define REC_POS_LEN0 -2
#define REC_POS_LEN1 -1

  /* Record length.  */
  unsigned short rec_len;
  /* Number of bytes to read in the current record.  */
  unsigned short rec_rem;
  /* Offset of the next block.  */
  file_ptr next_block;
  /* Current *data* offset in the data block.  */
  unsigned short blk_off;

  /* Offset of the first block.  Extracted from the index.  */
  file_ptr first_block;

  /* Initial next_block.  Extracted when the MHD is read.  */
  file_ptr init_next_block;
  /* Initial blk_off, once the MHD is read.  */
  unsigned short init_blk_off;

  /* Used to store any 3 byte record, which could be the EOF pattern.  */
  unsigned char pattern[4];

  /* DCX.  */
  struct dcxsbm_desc *dcxsbms;
  /* Current submap.  */
  struct dcxsbm_desc *dcx_sbm;
  /* Current offset in the submap.  */
  unsigned int dcx_offset;
  int dcx_pos;

  /* Compressed buffer.  */
  unsigned char *dcx_buf;
  /* Size of the buffer.  Used to resize.  */
  unsigned int dcx_max;
  /* Number of valid bytes in the buffer.  */
  unsigned int dcx_rlen;
};

/* Return the current position.  */

static file_ptr
vms_lib_btell (struct bfd *abfd)
{
  struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;
  return vec->where;
}

/* Read the header of the next data block if all bytes of the current block
   have been read.  */

static bool
vms_lib_read_block (struct bfd *abfd)
{
  struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;

  if (vec->blk_off == DATA__LENGTH)
    {
      unsigned char hdr[DATA__DATA];

      /* Read next block.  */
      if (bfd_seek (abfd->my_archive, vec->next_block, SEEK_SET) != 0)
	return false;
      if (bfd_bread (hdr, sizeof (hdr), abfd->my_archive) != sizeof (hdr))
	return false;
      vec->next_block = (bfd_getl32 (hdr + 2) - 1) * VMS_BLOCK_SIZE;
      vec->blk_off = sizeof (hdr);
    }
  return true;
}

/* Read NBYTES from ABFD into BUF if not NULL.  If BUF is NULL, bytes are
   not stored.  Read linearly from the library, but handle blocks.  This
   function does not handle records nor EOF.  */

static file_ptr
vms_lib_bread_raw (struct bfd *abfd, unsigned char *buf, file_ptr nbytes)
{
  struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;
  file_ptr res;

  res = 0;
  while (nbytes > 0)
    {
      unsigned int l;

      /* Be sure the current data block is read.  */
      if (!vms_lib_read_block (abfd))
	return -1;

      /* Do not read past the data block, do not read more than requested.  */
      l = DATA__LENGTH - vec->blk_off;
      if (l > nbytes)
	l = nbytes;
      if (l == 0)
	return 0;
      if (buf != NULL)
	{
	  /* Really read into BUF.  */
	  if (bfd_bread (buf, l, abfd->my_archive) != l)
	    return -1;
	}
      else
	{
	  /* Make as if we are reading.  */
	  if (bfd_seek (abfd->my_archive, l, SEEK_CUR) != 0)
	    return -1;
	}

      if (buf != NULL)
	buf += l;
      vec->blk_off += l;
      nbytes -= l;
      res += l;
    }
  return res;
}

/* Decompress NBYTES from VEC.  Store the bytes into BUF if not NULL.  */

static file_ptr
vms_lib_dcx (struct vms_lib_iovec *vec, unsigned char *buf, file_ptr nbytes)
{
  struct dcxsbm_desc *sbm;
  unsigned int i;
  unsigned int offset;
  unsigned int j;
  file_ptr res = 0;

  /* The loop below expect to deliver at least one byte.  */
  if (nbytes == 0)
    return 0;

  /* Get the current state.  */
  sbm = vec->dcx_sbm;
  offset = vec->dcx_offset;
  j = vec->dcx_pos & 7;

  for (i = vec->dcx_pos >> 3; i < vec->dcx_rlen; i++)
    {
      unsigned char b = vec->dcx_buf[i];

      for (; j < 8; j++)
	{
	  if (b & (1 << j))
	    offset++;
	  if (!(sbm->flags[offset >> 3] & (1 << (offset & 7))))
	    {
	      unsigned int n_offset = sbm->nodes[offset];
	      if (n_offset == 0)
		{
		  /* End of buffer.  Stay where we are.  */
		  vec->dcx_pos = (i << 3) + j;
		  if (b & (1 << j))
		    offset--;
		  vec->dcx_offset = offset;
		  vec->dcx_sbm = sbm;
		  return res;
		}
	      offset = 2 * n_offset;
	    }
	  else
	    {
	      unsigned char v = sbm->nodes[offset];

	      if (sbm->next != NULL)
		sbm = vec->dcxsbms + sbm->next[v];
	      offset = 0;
	      res++;

	      if (buf)
		{
		  *buf++ = v;
		  nbytes--;

		  if (nbytes == 0)
		    {
		      vec->dcx_pos = (i << 3) + j + 1;
		      vec->dcx_offset = offset;
		      vec->dcx_sbm = sbm;

		      return res;
		    }
		}
	    }
	}
      j = 0;
    }
  return -1;
}

/* Standard IOVEC function.  */

static file_ptr
vms_lib_bread (struct bfd *abfd, void *vbuf, file_ptr nbytes)
{
  struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;
  file_ptr res;
  file_ptr chunk;
  unsigned char *buf = (unsigned char *)vbuf;

  /* Do not read past the end.  */
  if (vec->where >= vec->file_len)
    return 0;

  res = 0;
  while (nbytes > 0)
    {
      if (vec->rec_rem == 0)
	{
	  unsigned char blen[2];

	  /* Read record length.  */
	  if (vms_lib_bread_raw (abfd, blen, sizeof (blen)) != sizeof (blen))
	    return -1;
	  vec->rec_len = bfd_getl16 (blen);
	  if (bfd_libdata (abfd->my_archive)->kind == vms_lib_txt)
	    {
	      /* Discard record size and align byte.  */
	      vec->rec_pos = 0;
	      vec->rec_rem = vec->rec_len;
	    }
	  else
	    {
	      /* Prepend record size.  */
	      vec->rec_pos = REC_POS_LEN0;
	      vec->rec_rem = (vec->rec_len + 1) & ~1;	/* With align byte.  */
	    }
	  if (vec->rec_len == 3)
	    {
	      /* Possibly end of file.  Check the pattern.  */
	      if (vms_lib_bread_raw (abfd, vec->pattern, 4) != 4)
		return -1;
	      if (!memcmp (vec->pattern, eotdesc + 2, 3))
		{
		  /* This is really an EOF.  */
		  vec->where += res;
		  vec->file_len = vec->where;
		  return res;
		}
	    }

	  if (vec->dcxsbms != NULL)
	    {
	      /* This is a compressed member.  */
	      unsigned int len;
	      file_ptr elen;

	      /* Be sure there is enough room for the expansion.  */
	      len = (vec->rec_len + 1) & ~1;
	      if (len > vec->dcx_max)
		{
		  while (len > vec->dcx_max)
		    vec->dcx_max *= 2;
		  vec->dcx_buf = bfd_alloc (abfd, vec->dcx_max);
		  if (vec->dcx_buf == NULL)
		    return -1;
		}

	      /* Read the compressed record.  */
	      vec->dcx_rlen = len;
	      if (vec->rec_len == 3)
		{
		  /* Already read.  */
		  memcpy (vec->dcx_buf, vec->pattern, 3);
		}
	      else
		{
		  elen = vms_lib_bread_raw (abfd, vec->dcx_buf, len);
		  if (elen != len)
		    return -1;
		}

	      /* Dummy expansion to get the expanded length.  */
	      vec->dcx_offset = 0;
	      vec->dcx_sbm = vec->dcxsbms;
	      vec->dcx_pos = 0;
	      elen = vms_lib_dcx (vec, NULL, 0x10000);
	      if (elen < 0)
		return -1;
	      vec->rec_len = elen;
	      vec->rec_rem = elen;

	      /* Reset the state.  */
	      vec->dcx_offset = 0;
	      vec->dcx_sbm = vec->dcxsbms;
	      vec->dcx_pos = 0;
	    }
	}
      if (vec->rec_pos < 0)
	{
	  unsigned char c;
	  switch (vec->rec_pos)
	    {
	    case REC_POS_LEN0:
	      c = vec->rec_len & 0xff;
	      vec->rec_pos = REC_POS_LEN1;
	      break;
	    case REC_POS_LEN1:
	      c = (vec->rec_len >> 8) & 0xff;
	      vec->rec_pos = 0;
	      break;
	    case REC_POS_PAD:
	      c = 0;
	      vec->rec_rem = 0;
	      break;
	    case REC_POS_NL:
	      c = '\n';
	      vec->rec_rem = 0;
	      break;
	    default:
	      abort ();
	    }
	  if (buf != NULL)
	    {
	      *buf = c;
	      buf++;
	    }
	  nbytes--;
	  res++;
	  continue;
	}

      if (nbytes > vec->rec_rem)
	chunk = vec->rec_rem;
      else
	chunk = nbytes;

      if (vec->dcxsbms != NULL)
	{
	  /* Optimize the stat() case: no need to decompress again as we
	     know the length.  */
	  if (!(buf == NULL && chunk == vec->rec_rem))
	    chunk = vms_lib_dcx (vec, buf, chunk);
	}
      else
	{
	  if (vec->rec_len == 3)
	    {
	      if (buf != NULL)
		memcpy (buf, vec->pattern + vec->rec_pos, chunk);
	    }
	  else
	    chunk = vms_lib_bread_raw (abfd, buf, chunk);
	}
      if (chunk < 0)
	return -1;
      res += chunk;
      if (buf != NULL)
	buf += chunk;
      nbytes -= chunk;
      vec->rec_pos += chunk;
      vec->rec_rem -= chunk;

      if (vec->rec_rem == 0)
	{
	  /* End of record reached.  */
	  if (bfd_libdata (abfd->my_archive)->kind == vms_lib_txt)
	    {
	      if ((vec->rec_len & 1) == 1
		  && vec->rec_len != 3
		  && vec->dcxsbms == NULL)
		{
		  /* Eat the pad byte.  */
		  unsigned char pad;
		  if (vms_lib_bread_raw (abfd, &pad, 1) != 1)
		    return -1;
		}
	      vec->rec_pos = REC_POS_NL;
	      vec->rec_rem = 1;
	    }
	  else
	    {
	      if ((vec->rec_len & 1) == 1 && vec->dcxsbms != NULL)
		{
		  vec->rec_pos = REC_POS_PAD;
		  vec->rec_rem = 1;
		}
	    }
	}
    }
  vec->where += res;
  return res;
}

/* Standard function, but we currently only handle the rewind case.  */

static int
vms_lib_bseek (struct bfd *abfd, file_ptr offset, int whence)
{
  struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;

  if (whence == SEEK_SET && offset == 0)
    {
      vec->where = 0;
      vec->rec_rem = 0;
      vec->dcx_pos = -1;
      vec->blk_off = vec->init_blk_off;
      vec->next_block = vec->init_next_block;

      if (bfd_seek (abfd->my_archive, vec->first_block, SEEK_SET) != 0)
	return -1;
    }
  else
    abort ();
  return 0;
}

static file_ptr
vms_lib_bwrite (struct bfd *abfd ATTRIBUTE_UNUSED,
	      const void *where ATTRIBUTE_UNUSED,
	      file_ptr nbytes ATTRIBUTE_UNUSED)
{
  return -1;
}

static int
vms_lib_bclose (struct bfd *abfd)
{
  abfd->iostream = NULL;
  return 0;
}

static int
vms_lib_bflush (struct bfd *abfd ATTRIBUTE_UNUSED)
{
  return 0;
}

static int
vms_lib_bstat (struct bfd *abfd ATTRIBUTE_UNUSED,
	       struct stat *sb ATTRIBUTE_UNUSED)
{
  /* Not supported.  */
  return 0;
}

static void *
vms_lib_bmmap (struct bfd *abfd ATTRIBUTE_UNUSED,
	       void *addr ATTRIBUTE_UNUSED,
	       bfd_size_type len ATTRIBUTE_UNUSED,
	       int prot ATTRIBUTE_UNUSED,
	       int flags ATTRIBUTE_UNUSED,
	       file_ptr offset ATTRIBUTE_UNUSED,
	       void **map_addr ATTRIBUTE_UNUSED,
	       bfd_size_type *map_len ATTRIBUTE_UNUSED)
{
  return (void *) -1;
}

static const struct bfd_iovec vms_lib_iovec = {
  &vms_lib_bread, &vms_lib_bwrite, &vms_lib_btell, &vms_lib_bseek,
  &vms_lib_bclose, &vms_lib_bflush, &vms_lib_bstat, &vms_lib_bmmap
};

/* Open a library module.  FILEPOS is the position of the module header.  */

static bool
vms_lib_bopen (bfd *el, file_ptr filepos)
{
  struct vms_lib_iovec *vec;
  unsigned char buf[256];
  struct vms_mhd *mhd;
  struct lib_tdata *tdata = bfd_libdata (el->my_archive);
  unsigned int len;

  /* Allocate and initialized the iovec.  */
  vec = bfd_zalloc (el, sizeof (*vec));
  if (vec == NULL)
    return false;

  el->iostream = vec;
  el->iovec = &vms_lib_iovec;

  /* File length is not known.  */
  vec->file_len = -1;

  /* Read the first data block.  */
  vec->next_block = filepos & ~(VMS_BLOCK_SIZE - 1);
  vec->blk_off = DATA__LENGTH;
  if (!vms_lib_read_block (el))
    return false;

  /* Prepare to read the first record.  */
  vec->blk_off = filepos & (VMS_BLOCK_SIZE - 1);
  vec->rec_rem = 0;
  if (bfd_seek (el->my_archive, filepos, SEEK_SET) != 0)
    return false;

  /* Read Record length + MHD + align byte.  */
  len = tdata->mhd_size;
  if (vms_lib_bread_raw (el, buf, 2) != 2)
    return false;
  if (bfd_getl16 (buf) != len)
    return false;
  len = (len + 1) & ~1;
  BFD_ASSERT (len <= sizeof (buf));
  if (vms_lib_bread_raw (el, buf, len) != len)
    return false;

  /* Get info from mhd.  */
  mhd = (struct vms_mhd *)buf;
  /* Check id.  */
  if (mhd->id != MHD__C_MHDID)
    return false;
  if (len >= MHD__C_MHDLEN + 1)
    el->selective_search = (mhd->objstat & MHD__M_SELSRC) ? 1 : 0;
  el->mtime = vms_rawtime_to_time_t (mhd->datim);
  el->mtime_set = true;

  /* Reinit the iovec so that seek() will point to the first record after
     the mhd.  */
  vec->where = 0;
  vec->init_blk_off = vec->blk_off;
  vec->init_next_block = vec->next_block;
  vec->first_block = bfd_tell (el->my_archive);
  vec->dcxsbms = bfd_libdata (el->my_archive)->dcxsbm;

  if (vec->dcxsbms != NULL)
    {
      /* Handle DCX.  */
      vec->dcx_max = 10 * 1024;
      vec->dcx_buf = bfd_alloc (el, vec->dcx_max);
      vec->dcx_pos = -1;
      if (vec->dcx_buf == NULL)
	return -1;
    }
  return true;
}

/* Get member MODIDX.  Return NULL in case of error.  */

static bfd *
_bfd_vms_lib_get_module (bfd *abfd, unsigned int modidx)
{
  struct lib_tdata *tdata = bfd_libdata (abfd);
  bfd *res;
  file_ptr file_off;
  const char *name;
  char *newname;
  size_t namelen;

  /* Sanity check.  */
  if (modidx >= tdata->nbr_modules)
    return NULL;

  /* Already loaded.  */
  if (tdata->cache[modidx])
    return tdata->cache[modidx];

  /* Build it.  */
  file_off = tdata->modules[modidx].file_offset;
  if (tdata->type != LBR__C_TYP_IOBJ)
    {
      res = _bfd_create_empty_archive_element_shell (abfd);
      if (res == NULL)
	return NULL;

      /* Special reader to deal with data blocks.  */
      if (!vms_lib_bopen (res, file_off))
	return NULL;
    }
  else
    {
      char buf[256];
      struct vms_mhd *mhd;
      struct areltdata *arelt;

      /* Sanity check.  The MHD must be big enough to contain module size.  */
      if (tdata->mhd_size < offsetof (struct vms_mhd, modsize) + 4)
	return NULL;

      /* Read the MHD now.  */
      if (bfd_seek (abfd, file_off, SEEK_SET) != 0)
	return NULL;
      if (bfd_bread (buf, tdata->mhd_size, abfd) != tdata->mhd_size)
	return NULL;

      mhd = (struct vms_mhd *) buf;
      if (mhd->id != MHD__C_MHDID)
	return NULL;

      res = _bfd_create_empty_archive_element_shell (abfd);
      if (res == NULL)
	return NULL;
      arelt = bfd_zmalloc (sizeof (*arelt));
      if (arelt == NULL)
	{
	  bfd_close (res);
	  return NULL;
	}
      res->arelt_data = arelt;

      /* Get info from mhd.  */
      if (tdata->mhd_size >= offsetof (struct vms_mhd, objstat) + 1)
	res->selective_search = (mhd->objstat & MHD__M_SELSRC) ? 1 : 0;
      res->mtime = vms_rawtime_to_time_t (mhd->datim);
      res->mtime_set = true;

      arelt->parsed_size = bfd_getl32 (mhd->modsize);

      /* No need for a special reader as members are stored linearly.
	 Just skip the MHD.  */
      res->origin = file_off + tdata->mhd_size;
    }

  /* Set filename.  */
  name = tdata->modules[modidx].name;
  namelen = strlen (name);
  newname = bfd_malloc (namelen + 4 + 1);
  if (newname == NULL)
    {
      bfd_close (res);
      return NULL;
    }
  strcpy (newname, name);
  switch (tdata->type)
    {
    case LBR__C_TYP_IOBJ:
    case LBR__C_TYP_EOBJ:
      /* For object archives, append .obj to mimic standard behaviour.  */
      strcpy (newname + namelen, ".obj");
      break;
    default:
      break;
    }
  bfd_set_filename (res, newname);
  free (newname);
  if (bfd_get_filename (res) == NULL)
    {
      bfd_close (res);
      return NULL;
    }

  tdata->cache[modidx] = res;

  return res;
}

/* Standard function: get member at IDX.  */

bfd *
_bfd_vms_lib_get_elt_at_index (bfd *abfd, symindex symidx)
{
  struct lib_tdata *tdata = bfd_libdata (abfd);
  file_ptr file_off;
  unsigned int modidx;

  /* Check symidx.  */
  if (symidx > tdata->artdata.symdef_count)
    return NULL;
  file_off = tdata->artdata.symdefs[symidx].file_offset;

  /* Linear-scan.  */
  for (modidx = 0; modidx < tdata->nbr_modules; modidx++)
    {
      if (tdata->modules[modidx].file_offset == file_off)
	break;
    }
  if (modidx >= tdata->nbr_modules)
    return NULL;

  return _bfd_vms_lib_get_module (abfd, modidx);
}

/* Elements of an imagelib are stubs.  You can get the real image with this
   function.  */

bfd *
_bfd_vms_lib_get_imagelib_file (bfd *el)
{
  bfd *archive = el->my_archive;
  const char *modname = bfd_get_filename (el);
  int modlen = strlen (modname);
  char *filename;
  int j;
  bfd *res;

  /* Convert module name to lower case and append '.exe'.  */
  filename = bfd_alloc (el, modlen + 5);
  if (filename == NULL)
    return NULL;
  for (j = 0; j < modlen; j++)
    if (ISALPHA (modname[j]))
      filename[j] = TOLOWER (modname[j]);
    else
      filename[j] = modname[j];
  memcpy (filename + modlen, ".exe", 5);

  filename = _bfd_append_relative_path (archive, filename);
  if (filename == NULL)
    return NULL;
  res = bfd_openr (filename, NULL);

  if (res == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler(_("could not open shared image '%s' from '%s'"),
			 filename, bfd_get_filename (archive));
      bfd_release (archive, filename);
      return NULL;
    }

  /* FIXME: put it in a cache ?  */
  return res;
}

/* Standard function.  */

bfd *
_bfd_vms_lib_openr_next_archived_file (bfd *archive,
				       bfd *last_file)
{
  unsigned int idx;
  bfd *res;

  if (!last_file)
    idx = 0;
  else
    idx = last_file->proxy_origin + 1;

  if (idx >= bfd_libdata (archive)->nbr_modules)
    {
      bfd_set_error (bfd_error_no_more_archived_files);
      return NULL;
    }

  res = _bfd_vms_lib_get_module (archive, idx);
  if (res == NULL)
    return res;
  res->proxy_origin = idx;
  return res;
}

/* Standard function.  Just compute the length.  */

int
_bfd_vms_lib_generic_stat_arch_elt (bfd *abfd, struct stat *st)
{
  struct lib_tdata *tdata;

  /* Sanity check.  */
  if (abfd->my_archive == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  tdata = bfd_libdata (abfd->my_archive);
  if (tdata->type != LBR__C_TYP_IOBJ)
    {
      struct vms_lib_iovec *vec = (struct vms_lib_iovec *) abfd->iostream;

      if (vec->file_len == (ufile_ptr)-1)
	{
	  if (vms_lib_bseek (abfd, 0, SEEK_SET) != 0)
	    return -1;

	  /* Compute length.  */
	  while (vms_lib_bread (abfd, NULL, 1 << 20) > 0)
	    ;
	}
      st->st_size = vec->file_len;
    }
  else
    {
      st->st_size = ((struct areltdata *)abfd->arelt_data)->parsed_size;
    }

  if (abfd->mtime_set)
    st->st_mtime = abfd->mtime;
  else
    st->st_mtime = 0;
  st->st_uid = 0;
  st->st_gid = 0;
  st->st_mode = 0644;

  return 0;
}

/* Internal representation of an index entry.  */

struct lib_index
{
  /* Corresponding archive member.  */
  bfd *abfd;

  /* Number of reference to this entry.  */
  unsigned int ref;

  /* Length of the key.  */
  unsigned short namlen;

  /* Key.  */
  const char *name;
};

/* Used to sort index entries.  */

static int
lib_index_cmp (const void *lv, const void *rv)
{
  const struct lib_index *l = lv;
  const struct lib_index *r = rv;

  return strcmp (l->name, r->name);
}

/* Maximum number of index blocks level.  */

#define MAX_LEVEL 10

/* Get the size of an index entry.  */

static unsigned int
get_idxlen (struct lib_index *idx, bool is_elfidx)
{
  if (is_elfidx)
    {
      /* 9 is the size of struct vms_elfidx without keyname.  */
      if (idx->namlen > MAX_KEYLEN)
	return 9 + sizeof (struct vms_kbn);
      else
	return 9 + idx->namlen;
    }
  else
    {
      /* 7 is the size of struct vms_idx without keyname.  */
      return 7 + idx->namlen;
    }
}

/* Write the index composed by NBR symbols contained in IDX.
   VBN is the first vbn to be used, and will contain on return the last vbn.
   Can be called with ABFD set to NULL just to size the index.
   If not null, TOPVBN will be assigned to the vbn of the root index tree.
   IS_ELFIDX is true for elfidx (ie ia64) indexes layout.
   Return TRUE on success.  */

static bool
vms_write_index (bfd *abfd,
		 struct lib_index *idx, unsigned int nbr, unsigned int *vbn,
		 unsigned int *topvbn, bool is_elfidx)
{
  /* The index is organized as a tree.  This function implements a naive
     algorithm to balance the tree: it fills the leaves, and create a new
     branch when all upper leaves and branches are full.  We only keep in
     memory a path to the current leaf.  */
  unsigned int i;
  int j;
  int level;
  /* Disk blocks for the current path.  */
  struct vms_indexdef *rblk[MAX_LEVEL];
  /* Info on the current blocks.  */
  struct idxblk
  {
    unsigned int vbn;		/* VBN of the block.  */
    /* The last entry is identified so that it could be copied to the
       parent block.  */
    unsigned short len;		/* Length up to the last entry.  */
    unsigned short lastlen;	/* Length of the last entry.  */
  } blk[MAX_LEVEL];

  /* The kbn blocks are used to store long symbol names.  */
  unsigned int kbn_sz = 0;   /* Number of bytes available in the kbn block.  */
  unsigned int kbn_vbn = 0;  /* VBN of the kbn block.  */
  unsigned char *kbn_blk = NULL; /* Contents of the kbn block.  */

  if (nbr == 0)
    {
      /* No entries.  Very easy to handle.  */
      if (topvbn != NULL)
	*topvbn = 0;
      return true;
    }

  if (abfd == NULL)
    {
      /* Sort the index the first time this function is called.  */
      qsort (idx, nbr, sizeof (struct lib_index), lib_index_cmp);
    }

  /* Allocate first index block.  */
  level = 1;
  if (abfd != NULL)
    rblk[0] = bfd_zmalloc (sizeof (struct vms_indexdef));
  blk[0].vbn = (*vbn)++;
  blk[0].len = 0;
  blk[0].lastlen = 0;

  for (i = 0; i < nbr; i++, idx++)
    {
      unsigned int idxlen;
      int flush = 0;
      unsigned int key_vbn = 0;
      unsigned int key_off = 0;

      idxlen = get_idxlen (idx, is_elfidx);

      if (is_elfidx && idx->namlen > MAX_KEYLEN)
	{
	  /* If the key (ie name) is too long, write it in the kbn block.  */
	  unsigned int kl = idx->namlen;
	  unsigned int kl_chunk;
	  const char *key = idx->name;

	  /* Write the key in the kbn, chunk after chunk.  */
	  do
	    {
	      if (kbn_sz < sizeof (struct vms_kbn))
		{
		  /* Not enough room in the kbn block.  */
		  if (abfd != NULL)
		    {
		      /* Write it to the disk (if there is one).  */
		      if (kbn_vbn != 0)
			{
			  if (!vms_write_block (abfd, kbn_vbn, kbn_blk))
			    goto err;
			}
		      else
			{
			  kbn_blk = bfd_malloc (VMS_BLOCK_SIZE);
			  if (kbn_blk == NULL)
			    goto err;
			}
		      *(unsigned short *)kbn_blk = 0;
		    }
		  /* Allocate a new block for the keys.  */
		  kbn_vbn = (*vbn)++;
		  kbn_sz = VMS_BLOCK_SIZE - 2;
		}
	      /* Size of the chunk written to the current key block.  */
	      if (kl + sizeof (struct vms_kbn) > kbn_sz)
		kl_chunk = kbn_sz - sizeof (struct vms_kbn);
	      else
		kl_chunk = kl;

	      if (kbn_blk != NULL)
		{
		  struct vms_kbn *kbn;

		  kbn = (struct vms_kbn *)(kbn_blk + VMS_BLOCK_SIZE - kbn_sz);

		  if (key_vbn == 0)
		    {
		      /* Save the rfa of the first chunk.  */
		      key_vbn = kbn_vbn;
		      key_off = VMS_BLOCK_SIZE - kbn_sz;
		    }

		  bfd_putl16 (kl_chunk, kbn->keylen);
		  if (kl_chunk == kl)
		    {
		      /* No next chunk.  */
		      bfd_putl32 (0, kbn->rfa.vbn);
		      bfd_putl16 (0, kbn->rfa.offset);
		    }
		  else
		    {
		      /* Next chunk will be at the start of the next block.  */
		      bfd_putl32 (*vbn, kbn->rfa.vbn);
		      bfd_putl16 (2, kbn->rfa.offset);
		    }
		  memcpy ((char *)(kbn + 1), key, kl_chunk);
		  key += kl_chunk;
		}
	      kl -= kl_chunk;
	      kl_chunk = (kl_chunk + 1) & ~1;	  /* Always align.  */
	      kbn_sz -= kl_chunk + sizeof (struct vms_kbn);
	    }
	  while (kl > 0);
	}

      /* Check if a block might overflow.  In this case we will flush this
	 block and all the blocks below it.  */
      for (j = 0; j < level; j++)
	if (blk[j].len + blk[j].lastlen + idxlen > INDEXDEF__BLKSIZ)
	  flush = j + 1;

      for (j = 0; j < level; j++)
	{
	  if (j < flush)
	    {
	      /* There is not enough room to write the new entry in this
		 block or in a parent block.  */

	      if (j + 1 == level)
		{
		  BFD_ASSERT (level < MAX_LEVEL);

		  /* Need to create a parent.  */
		  if (abfd != NULL)
		    {
		      rblk[level] = bfd_zmalloc (sizeof (struct vms_indexdef));
		      bfd_putl32 (*vbn, rblk[j]->parent);
		    }
		  blk[level].vbn = (*vbn)++;
		  blk[level].len = 0;
		  blk[level].lastlen = blk[j].lastlen;

		  level++;
		}

	      /* Update parent block: write the last entry from the current
		 block.  */
	      if (abfd != NULL)
		{
		  struct vms_rfa *rfa;

		  /* Pointer to the last entry in parent block.  */
		  rfa = (struct vms_rfa *)(rblk[j + 1]->keys + blk[j + 1].len);

		  /* Copy the whole entry.  */
		  BFD_ASSERT (blk[j + 1].lastlen == blk[j].lastlen);
		  memcpy (rfa, rblk[j]->keys + blk[j].len, blk[j].lastlen);
		  /* Fix the entry (which in always the first field of an
		     entry.  */
		  bfd_putl32 (blk[j].vbn, rfa->vbn);
		  bfd_putl16 (RFADEF__C_INDEX, rfa->offset);
		}

	      if (j + 1 == flush)
		{
		  /* And allocate it.  Do it only on the block that won't be
		     flushed (so that the parent of the parent can be
		     updated too).  */
		  blk[j + 1].len += blk[j + 1].lastlen;
		  blk[j + 1].lastlen = 0;
		}

	      /* Write this block on the disk.  */
	      if (abfd != NULL)
		{
		  bfd_putl16 (blk[j].len + blk[j].lastlen, rblk[j]->used);
		  if (!vms_write_block (abfd, blk[j].vbn, rblk[j]))
		    goto err;
		}

	      /* Reset this block.  */
	      blk[j].len = 0;
	      blk[j].lastlen = 0;
	      blk[j].vbn = (*vbn)++;
	    }

	  /* Append it to the block.  */
	  if (j == 0)
	    {
	      /* Keep the previous last entry.  */
	      blk[j].len += blk[j].lastlen;

	      if (abfd != NULL)
		{
		  struct vms_rfa *rfa;

		  rfa = (struct vms_rfa *)(rblk[j]->keys + blk[j].len);
		  bfd_putl32 ((idx->abfd->proxy_origin / VMS_BLOCK_SIZE) + 1,
			      rfa->vbn);
		  bfd_putl16
		    ((idx->abfd->proxy_origin % VMS_BLOCK_SIZE)
		     + (is_elfidx ? 0 : DATA__DATA),
		     rfa->offset);

		  if (is_elfidx)
		    {
		      /* Use elfidx format.  */
		      struct vms_elfidx *en = (struct vms_elfidx *)rfa;

		      en->flags = 0;
		      if (key_vbn != 0)
			{
			  /* Long symbol name.  */
			  struct vms_kbn *k = (struct vms_kbn *)(en->keyname);
			  bfd_putl16 (sizeof (struct vms_kbn), en->keylen);
			  bfd_putl16 (idx->namlen, k->keylen);
			  bfd_putl32 (key_vbn, k->rfa.vbn);
			  bfd_putl16 (key_off, k->rfa.offset);
			  en->flags |= ELFIDX__SYMESC;
			}
		      else
			{
			  bfd_putl16 (idx->namlen, en->keylen);
			  memcpy (en->keyname, idx->name, idx->namlen);
			}
		    }
		  else
		    {
		      /* Use idx format.  */
		      struct vms_idx *en = (struct vms_idx *)rfa;
		      en->keylen = idx->namlen;
		      memcpy (en->keyname, idx->name, idx->namlen);
		    }
		}
	    }
	  /* The last added key can now be the last one all blocks in the
	     path.  */
	  blk[j].lastlen = idxlen;
	}
    }

  /* Save VBN of the root.  */
  if (topvbn != NULL)
    *topvbn = blk[level - 1].vbn;

  if (abfd == NULL)
    return true;

  /* Flush.  */
  for (j = 1; j < level; j++)
    {
      /* Update parent block: write the new entry.  */
      unsigned char *en;
      unsigned char *par;
      struct vms_rfa *rfa;

      en = rblk[j - 1]->keys + blk[j - 1].len;
      par = rblk[j]->keys + blk[j].len;
      BFD_ASSERT (blk[j].lastlen == blk[j - 1].lastlen);
      memcpy (par, en, blk[j - 1].lastlen);
      rfa = (struct vms_rfa *)par;
      bfd_putl32 (blk[j - 1].vbn, rfa->vbn);
      bfd_putl16 (RFADEF__C_INDEX, rfa->offset);
    }

  for (j = 0; j < level; j++)
    {
      /* Write this block on the disk.  */
      bfd_putl16 (blk[j].len + blk[j].lastlen, rblk[j]->used);
      if (!vms_write_block (abfd, blk[j].vbn, rblk[j]))
	goto err;

      free (rblk[j]);
      rblk[j] = NULL;
    }

  /* Write the last kbn (if any).  */
  if (kbn_vbn != 0)
    {
      if (!vms_write_block (abfd, kbn_vbn, kbn_blk))
	goto err;
      free (kbn_blk);
    }

  return true;

 err:
  if (abfd != NULL)
    {
      for (j = 0; j < level; j++)
	free (rblk[j]);
      free (kbn_blk);
    }
  return false;
}

/* Append data to the data block DATA.  Force write if PAD is true.  */

static bool
vms_write_data_block (bfd *arch, struct vms_datadef *data, file_ptr *off,
		      const unsigned char *buf, unsigned int len, int pad)
{
  while (len > 0 || pad)
    {
      unsigned int doff = *off & (VMS_BLOCK_SIZE - 1);
      unsigned int remlen = (DATA__LENGTH - DATA__DATA) - doff;
      unsigned int l;

      l = (len > remlen) ? remlen : len;
      memcpy (data->data + doff, buf, l);
      buf += l;
      len -= l;
      doff += l;
      *off += l;

      if (doff == (DATA__LENGTH - DATA__DATA) || (len == 0 && pad))
	{
	  data->recs = 0;
	  data->fill_1 = 0;
	  bfd_putl32 ((*off / VMS_BLOCK_SIZE) + 2, data->link);

	  if (bfd_bwrite (data, sizeof (*data), arch) != sizeof (*data))
	    return false;

	  *off += DATA__LENGTH - doff;

	  if (len == 0)
	    break;
	}
    }
  return true;
}

/* Build the symbols index.  */

static bool
_bfd_vms_lib_build_map (unsigned int nbr_modules,
			struct lib_index *modules,
			unsigned int *res_cnt,
			struct lib_index **res)
{
  unsigned int i;
  asymbol **syms = NULL;
  long syms_max = 0;
  struct lib_index *map = NULL;
  unsigned int map_max = 1024;		/* Fine initial default.  */
  unsigned int map_count = 0;

  map = (struct lib_index *) bfd_malloc (map_max * sizeof (struct lib_index));
  if (map == NULL)
    goto error_return;

  /* Gather symbols.  */
  for (i = 0; i < nbr_modules; i++)
    {
      long storage;
      long symcount;
      long src_count;
      bfd *current = modules[i].abfd;

      if ((bfd_get_file_flags (current) & HAS_SYMS) == 0)
	continue;

      storage = bfd_get_symtab_upper_bound (current);
      if (storage < 0)
	goto error_return;

      if (storage != 0)
	{
	  if (storage > syms_max)
	    {
	      free (syms);
	      syms_max = storage;
	      syms = (asymbol **) bfd_malloc (syms_max);
	      if (syms == NULL)
		goto error_return;
	    }
	  symcount = bfd_canonicalize_symtab (current, syms);
	  if (symcount < 0)
	    goto error_return;

	  /* Now map over all the symbols, picking out the ones we
	     want.  */
	  for (src_count = 0; src_count < symcount; src_count++)
	    {
	      flagword flags = (syms[src_count])->flags;
	      asection *sec = syms[src_count]->section;

	      if ((flags & BSF_GLOBAL
		   || flags & BSF_WEAK
		   || flags & BSF_INDIRECT
		   || bfd_is_com_section (sec))
		  && ! bfd_is_und_section (sec))
		{
		  struct lib_index *new_map;

		  /* This symbol will go into the archive header.  */
		  if (map_count == map_max)
		    {
		      map_max *= 2;
		      new_map = (struct lib_index *)
			bfd_realloc (map, map_max * sizeof (struct lib_index));
		      if (new_map == NULL)
			goto error_return;
		      map = new_map;
		    }

		  map[map_count].abfd = current;
		  map[map_count].namlen = strlen (syms[src_count]->name);
		  map[map_count].name = syms[src_count]->name;
		  map_count++;
		  modules[i].ref++;
		}
	    }
	}
    }

  *res_cnt = map_count;
  *res = map;
  free (syms);
  return true;

 error_return:
  free (syms);
  free (map);
  return false;
}

/* Do the hard work: write an archive on the disk.  */

bool
_bfd_vms_lib_write_archive_contents (bfd *arch)
{
  bfd *current;
  unsigned int nbr_modules;
  struct lib_index *modules;
  unsigned int nbr_symbols;
  struct lib_index *symbols = NULL;
  struct lib_tdata *tdata = bfd_libdata (arch);
  unsigned int i;
  file_ptr off;
  unsigned int nbr_mod_iblk;
  unsigned int nbr_sym_iblk;
  unsigned int vbn;
  unsigned int mod_idx_vbn;
  unsigned int sym_idx_vbn;
  bool is_elfidx = tdata->kind == vms_lib_ia64;
  unsigned int max_keylen = is_elfidx ? MAX_EKEYLEN : MAX_KEYLEN;

  /* Count the number of modules (and do a first sanity check).  */
  nbr_modules = 0;
  for (current = arch->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      /* This check is checking the bfds for the objects we're reading
	 from (which are usually either an object file or archive on
	 disk), not the archive entries we're writing to.  We don't
	 actually create bfds for the archive members, we just copy
	 them byte-wise when we write out the archive.  */
      if (bfd_write_p (current) || !bfd_check_format (current, bfd_object))
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  goto input_err;
	}

      nbr_modules++;
    }

  /* Build the modules list.  */
  BFD_ASSERT (tdata->modules == NULL);
  modules = bfd_alloc (arch, nbr_modules * sizeof (struct lib_index));
  if (modules == NULL)
    return false;

  for (current = arch->archive_head, i = 0;
       current != NULL;
       current = current->archive_next, i++)
    {
      unsigned int nl;

      modules[i].abfd = current;
      modules[i].name = vms_get_module_name (bfd_get_filename (current), false);
      modules[i].ref = 1;

      /* FIXME: silently truncate long names ?  */
      nl = strlen (modules[i].name);
      modules[i].namlen = (nl > max_keylen ? max_keylen : nl);
    }

  /* Create the module index.  */
  vbn = 0;
  if (!vms_write_index (NULL, modules, nbr_modules, &vbn, NULL, is_elfidx))
    return false;
  nbr_mod_iblk = vbn;

  /* Create symbol index.  */
  if (!_bfd_vms_lib_build_map (nbr_modules, modules, &nbr_symbols, &symbols))
    goto err;

  vbn = 0;
  if (!vms_write_index (NULL, symbols, nbr_symbols, &vbn, NULL, is_elfidx))
    goto err;
  nbr_sym_iblk = vbn;

  /* Write modules and remember their position.  */
  off = (1 + nbr_mod_iblk + nbr_sym_iblk) * VMS_BLOCK_SIZE;

  if (bfd_seek (arch, off, SEEK_SET) != 0)
    goto err;

  for (i = 0; i < nbr_modules; i++)
    {
      struct vms_datadef data;
      unsigned char blk[VMS_BLOCK_SIZE];
      struct vms_mhd *mhd;
      unsigned int sz;

      current = modules[i].abfd;
      current->proxy_origin = off;

      if (is_elfidx)
	sz = 0;
      else
	{
	  /* Write the MHD as a record (ie, size first).  */
	  sz = 2;
	  bfd_putl16 (tdata->mhd_size, blk);
	}
      mhd = (struct vms_mhd *)(blk + sz);
      memset (mhd, 0, sizeof (struct vms_mhd));
      mhd->lbrflag = 0;
      mhd->id = MHD__C_MHDID;
      mhd->objidlng = 4;
      memcpy (mhd->objid, "V1.0", 4);
      bfd_putl32 (modules[i].ref, mhd->refcnt);
      /* FIXME: datim.  */

      sz += tdata->mhd_size;
      sz = (sz + 1) & ~1;

      /* Rewind the member to be put into the archive.  */
      if (bfd_seek (current, 0, SEEK_SET) != 0)
	goto input_err;

      /* Copy the member into the archive.  */
      if (is_elfidx)
	{
	  unsigned int modsize = 0;
	  bfd_size_type amt;
	  file_ptr off_hdr = off;

	  /* Read to complete the first block.  */
	  amt = bfd_bread (blk + sz, VMS_BLOCK_SIZE - sz, current);
	  if (amt == (bfd_size_type)-1)
	    goto input_err;
	  modsize = amt;
	  if (amt < VMS_BLOCK_SIZE - sz)
	    {
	      /* The member size is less than a block.  Pad the block.  */
	      memset (blk + sz + amt, 0, VMS_BLOCK_SIZE - sz - amt);
	    }
	  bfd_putl32 (modsize, mhd->modsize);

	  /* Write the first block (which contains an mhd).  */
	  if (bfd_bwrite (blk, VMS_BLOCK_SIZE, arch) != VMS_BLOCK_SIZE)
	    goto input_err;
	  off += VMS_BLOCK_SIZE;

	  if (amt == VMS_BLOCK_SIZE - sz)
	    {
	      /* Copy the remaining.  */
	      char buffer[DEFAULT_BUFFERSIZE];

	      while (1)
		{
		  amt = bfd_bread (buffer, sizeof (buffer), current);
		  if (amt == (bfd_size_type)-1)
		    goto input_err;
		  if (amt == 0)
		    break;
		  modsize += amt;
		  if (amt != sizeof (buffer))
		    {
		      /* Clear the padding.  */
		      memset (buffer + amt, 0, sizeof (buffer) - amt);
		      amt = (amt + VMS_BLOCK_SIZE) & ~(VMS_BLOCK_SIZE - 1);
		    }
		  if (bfd_bwrite (buffer, amt, arch) != amt)
		    goto input_err;
		  off += amt;
		}

	      /* Now that the size is known, write the first block (again).  */
	      bfd_putl32 (modsize, mhd->modsize);
	      if (bfd_seek (arch, off_hdr, SEEK_SET) != 0
		  || bfd_bwrite (blk, VMS_BLOCK_SIZE, arch) != VMS_BLOCK_SIZE)
		goto input_err;
	      if (bfd_seek (arch, off, SEEK_SET) != 0)
		goto input_err;
	    }
	}
      else
	{
	  /* Write the MHD.  */
	  if (!vms_write_data_block (arch, &data, &off, blk, sz, 0))
	    goto input_err;

	  /* Write the member.  */
	  while (1)
	    {
	      sz = bfd_bread (blk, sizeof (blk), current);
	      if (sz == 0)
		break;
	      if (!vms_write_data_block (arch, &data, &off, blk, sz, 0))
		goto input_err;
	    }

	  /* Write the end of module marker.  */
	  if (!vms_write_data_block (arch, &data, &off,
				     eotdesc, sizeof (eotdesc), 1))
	    goto input_err;
	}
    }

  /* Write the indexes.  */
  vbn = 2;
  if (!vms_write_index (arch, modules, nbr_modules, &vbn, &mod_idx_vbn,
			is_elfidx))
    goto err;
  if (!vms_write_index (arch, symbols, nbr_symbols, &vbn, &sym_idx_vbn,
			is_elfidx))
    goto err;

  /* Write libary header.  */
  {
    unsigned char blk[VMS_BLOCK_SIZE];
    struct vms_lhd *lhd = (struct vms_lhd *)blk;
    struct vms_idd *idd = (struct vms_idd *)(blk + sizeof (*lhd));
    unsigned int idd_flags;
    unsigned int saneid;

    memset (blk, 0, sizeof (blk));

    lhd->type = tdata->type;
    lhd->nindex = 2;
    switch (tdata->kind)
      {
      case vms_lib_alpha:
	saneid = LHD_SANEID3;
	break;
      case vms_lib_ia64:
	saneid = LHD_SANEID6;
	break;
      default:
	abort ();
      }
    bfd_putl32 (saneid, lhd->sanity);
    bfd_putl16 (tdata->ver, lhd->majorid);
    bfd_putl16 (0, lhd->minorid);
    snprintf ((char *)lhd->lbrver + 1, sizeof (lhd->lbrver) - 1,
	      "GNU ar %u.%u.%u",
	      (unsigned)(BFD_VERSION / 100000000UL),
	      (unsigned)(BFD_VERSION / 1000000UL) % 100,
	      (unsigned)(BFD_VERSION / 10000UL) % 100);
    lhd->lbrver[sizeof (lhd->lbrver) - 1] = 0;
    lhd->lbrver[0] = strlen ((char *)lhd->lbrver + 1);

    bfd_putl32 (tdata->credat_lo, lhd->credat + 0);
    bfd_putl32 (tdata->credat_hi, lhd->credat + 4);
    vms_raw_get_time (lhd->updtim);

    lhd->mhdusz = tdata->mhd_size - MHD__C_USRDAT;

    bfd_putl32 (nbr_modules + nbr_symbols, lhd->idxcnt);
    bfd_putl32 (nbr_modules, lhd->modcnt);
    bfd_putl32 (nbr_modules, lhd->modhdrs);

    /* Number of blocks for index.  */
    bfd_putl32 (nbr_mod_iblk + nbr_sym_iblk, lhd->idxblks);
    bfd_putl32 (vbn - 1, lhd->hipreal);
    bfd_putl32 (vbn - 1, lhd->hiprusd);

    /* VBN of the next free block.  */
    bfd_putl32 ((off / VMS_BLOCK_SIZE) + 1, lhd->nextvbn);
    bfd_putl32 ((off / VMS_BLOCK_SIZE) + 1, lhd->nextrfa + 0);
    bfd_putl16 (0, lhd->nextrfa + 4);

    /* First index (modules name).  */
    idd_flags = IDD__FLAGS_ASCII | IDD__FLAGS_VARLENIDX
      | IDD__FLAGS_NOCASECMP | IDD__FLAGS_NOCASENTR;
    bfd_putl16 (idd_flags, idd->flags);
    bfd_putl16 (max_keylen + 1, idd->keylen);
    bfd_putl16 (mod_idx_vbn, idd->vbn);
    idd++;

    /* Second index (symbols name).  */
    bfd_putl16 (idd_flags, idd->flags);
    bfd_putl16 (max_keylen + 1, idd->keylen);
    bfd_putl16 (sym_idx_vbn, idd->vbn);
    idd++;

    if (!vms_write_block (arch, 1, blk))
      goto err;
  }

  free (symbols);
  return true;

 input_err:
  bfd_set_input_error (current, bfd_get_error ());
 err:
  free (symbols);
  return false;
}

/* Add a target for text library.  This costs almost nothing and is useful to
   read VMS library on the host.  */

const bfd_target alpha_vms_lib_txt_vec =
{
  "vms-libtxt",			/* Name.  */
  bfd_target_unknown_flavour,
  BFD_ENDIAN_UNKNOWN,		/* byteorder */
  BFD_ENDIAN_UNKNOWN,		/* header_byteorder */
  0,				/* Object flags.  */
  0,				/* Sect flags.  */
  0,				/* symbol_leading_char.  */
  ' ',				/* ar_pad_char.  */
  15,				/* ar_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,
  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    _bfd_dummy_target,
    _bfd_vms_lib_txt_archive_p,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error
  },
  BFD_JUMP_TABLE_GENERIC (_bfd_generic),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_vms_lib),
  BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (_bfd_nowrite),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};
