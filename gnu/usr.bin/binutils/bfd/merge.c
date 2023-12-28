/* SEC_MERGE support.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Written by Jakub Jelinek <jakub@redhat.com>.

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


/* This file contains support for merging duplicate entities within sections,
   as used in ELF SHF_MERGE.  */

#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "elf-bfd.h"
#include "libbfd.h"
#include "objalloc.h"
#include "libiberty.h"

/* We partition all mergable input sections into sets of similar
   characteristics.  These sets are the unit of merging.  All content
   of the input sections is scanned and inserted into a hash table.
   We also remember an input-offset to entry mapping per input section, but
   the content itself is removed.  After everything is read in we assign
   output offsets to all hash entries, and when relocations are processed we
   lookup the given input offset per input-section, get the matching entry
   and its output offset (possibly adjusted for offset pointing into the
   middle of an entry).

   The input-offset-to-entry mapping (in map_ofs/map) is sorted, so in principle
   we could binary search it, but that's not cache-friendly and it's faster
   to add another lookup structure that gets us very near the correct
   entry in just one step (that's what ofstolowbound is for) and do a linear
   search from there.  */

/* An entry in the section merge hash table.  */

struct sec_merge_hash_entry
{
  /* Length of this entry.  This includes the zero terminator.  */
  unsigned int len;
  /* Start of this string needs to be aligned to
     alignment octets (not 1 << align).  */
  unsigned int alignment;
  union
  {
    /* Index within the merged section.  */
    bfd_size_type index;
    /* Entry this is a suffix of (if alignment is 0).  */
    struct sec_merge_hash_entry *suffix;
  } u;
  /* Next entity in the hash table (in order of entering).  */
  struct sec_merge_hash_entry *next;
  char str[1];
};

/* The section merge hash table.  */

struct sec_merge_hash
{
  struct bfd_hash_table table;
  /* Next available index.  */
  bfd_size_type size;
  /* First entity in the SEC_MERGE sections of this type.  */
  struct sec_merge_hash_entry *first;
  /* Last entity in the SEC_MERGE sections of this type.  */
  struct sec_merge_hash_entry *last;
  /* Entity size.  */
  unsigned int entsize;
  /* Are entries fixed size or zero terminated strings?  */
  bool strings;
  /* struct-of-array variant of all entries in the hash-table: */
  unsigned int nbuckets;
  /* We keep hash-code and length of entry together in a separate
     array in such a way that it can be checked with just a single memory
     reference.  In this way we don't need indirect access to the entries
     in the normal case.  keys_lens[i] is 'hashcode << 32) | len' for entry
     i (which is pointed to be values[i]).  */
  uint64_t *key_lens;
  struct sec_merge_hash_entry **values;
};

struct sec_merge_sec_info;

/* Information per merged blob.  This is the unit of merging and is
   related to (multiple) input sections of similar characteristics
   (alignment, entity size, strings or blobs).  */
struct sec_merge_info
{
  /* Chain of sec_merge_infos.  */
  struct sec_merge_info *next;
  /* Chain of sec_merge_sec_infos.  This first one will be the representative
     section that conceptually collects all merged content.  */
  struct sec_merge_sec_info *chain;
  struct sec_merge_sec_info **last;
  /* A hash table used to hold section content.  */
  struct sec_merge_hash *htab;
};

/* Offset into input mergable sections are represented by this type.
   Note how doesn't support crazy large mergable sections.  */
typedef uint32_t mapofs_type;

/* Given a sec_merge_sec_info S this gives the input offset of the IDX's
   recorded entry.  */
#define MAP_OFS(S,IDX) (S)->map_ofs[IDX]
/* And this gives the output offset (in the merged blob representing
   this S.  */
#define MAP_IDX(S,IDX) (S)->map[IDX].idx
/* For quick lookup of output offset given an input offset we store
   an array mapping intput-offset / OFSDIV to entry index.
   16 is better than 8, 32 is roughly same as 16, but uses less memory, so
   we use that. */
#define OFSDIV 32

/* Information per input merge section.  */
struct sec_merge_sec_info
{
  /* Chain of sec_merge_sec_infos.  */
  struct sec_merge_sec_info *next;
  /* The corresponding section.  */
  asection *sec;
  /* Pointer to merge_info pointing to us.  */
  void **psecinfo;
  /* The merge entity this is a part of.  */
  struct sec_merge_info *sinfo;
  /* The section associated with sinfo (i.e. the representative section).
     Same as sinfo->chain->sec, but faster to access in the hot function.  */
  asection *reprsec;
  /* First string in this section.  */
  struct sec_merge_hash_entry *first_str;
  /* Sparse mapping from input offset to entry covering that offset:  */
  unsigned int noffsetmap;  /* Number of these mappings.  */
  mapofs_type *map_ofs;     /* Input offset.  */
  union {
      struct sec_merge_hash_entry *entry;  /* Covering hash entry ... */
      bfd_size_type idx;                   /* ... or destination offset.  */
  } *map;
  /* Quick access: index into map_ofs[].  ofstolowbound[o / OFSDIV]=I is
     such that map_ofs[I] is the smallest offset higher that
     rounddown(o, OFSDIV) (and hence I-1 is the largest entry whose offset is
     smaller or equal to o/OFSDIV*OFSDIV).  */
  unsigned int *ofstolowbound;
  int fast_state;
};


/* Given a merge hash table TABLE and a number of entries to be
   ADDED, possibly resize the table for this to fit without further
   resizing.  */

static bool
sec_merge_maybe_resize (struct sec_merge_hash *table, unsigned added)
{
  struct bfd_hash_table *bfdtab = &table->table;
  if (bfdtab->count + added > table->nbuckets * 2 / 3)
    {
      unsigned i;
      unsigned long newnb = table->nbuckets * 2;
      struct sec_merge_hash_entry **newv;
      uint64_t *newl;
      unsigned long alloc;

      while (bfdtab->count + added > newnb * 2 / 3)
	{
	  newnb *= 2;
	  if (!newnb)
	    return false;
	}

      alloc = newnb * sizeof (newl[0]);
      if (alloc / sizeof (newl[0]) != newnb)
	return false;
      newl = objalloc_alloc ((struct objalloc *) table->table.memory, alloc);
      if (newl == NULL)
	return false;
      memset (newl, 0, alloc);
      alloc = newnb * sizeof (newv[0]);
      if (alloc / sizeof (newv[0]) != newnb)
	return false;
      newv = objalloc_alloc ((struct objalloc *) table->table.memory, alloc);
      if (newv == NULL)
	return false;
      memset (newv, 0, alloc);

      for (i = 0; i < table->nbuckets; i++)
	{
	  struct sec_merge_hash_entry *v = table->values[i];
	  if (v)
	    {
	      uint32_t thishash = table->key_lens[i] >> 32;
	      unsigned idx = thishash & (newnb - 1);
	      while (newv[idx])
		idx = (idx + 1) & (newnb - 1);
	      newl[idx] = table->key_lens[i];
	      newv[idx] = v;
	    }
	}

      table->key_lens = newl;
      table->values = newv;
      table->nbuckets = newnb;
    }
  return true;
}

/* Insert STRING (actually a byte blob of length LEN, with pre-computed
   HASH and bucket _INDEX) into our hash TABLE.  */

static struct sec_merge_hash_entry *
sec_merge_hash_insert (struct sec_merge_hash *table,
		 const char *string,
		 uint64_t hash, unsigned int len, unsigned int _index)
{
  struct bfd_hash_table *bfdtab = &table->table;
  struct sec_merge_hash_entry *hashp;

  hashp = (struct sec_merge_hash_entry *)
      bfd_hash_allocate (bfdtab, len + sizeof (struct sec_merge_hash_entry));
  if (hashp == NULL)
    return NULL;

  memcpy (hashp->str, string, len);
  hashp->len = len;
  hashp->alignment = 0;
  hashp->u.suffix = NULL;
  hashp->next = NULL;
  // We must not need resizing, otherwise _index is wrong
  BFD_ASSERT (bfdtab->count + 1 <= table->nbuckets * 2 / 3);
  bfdtab->count++;
  table->key_lens[_index] = (hash << 32) | (uint32_t)len;
  table->values[_index] = hashp;

  return hashp;
}

/* Read four bytes from *STR, interpret it as 32bit unsigned little
   endian value and return that.  */

static inline uint32_t
hash_read32 (const char *str)
{
  uint32_t i;
  /* All reasonable compilers will inline this memcpy and generate optimal
     code on architectures that support unaligned (4-byte) accesses.  */
  memcpy(&i, str, 4);
#ifdef WORDS_BIGENDIAN
  i = (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
#endif
  return i;
}

/* Calculate and return a hashvalue of the bytes at STR[0..LEN-1].
   All non-zero lengths and all alignments are supported.

   This is somewhat similar to xxh3 (of xxhash), but restricted to 32bit.
   On cc1 strings this has quite similar statistic properties, and we
   don't need to jump through hoops to get fast 64x64->128 mults,
   or 64bit arith on 32 bit hosts.  We also don't care for seeds
   or secrets.  They improve mixing very little.  */

static uint32_t
hash_blob (const char *str, unsigned int len)
{
  uint32_t ret = 0;
  uint32_t mul = (1 << 0) +  (1 << 2) + (1 << 3) + (1 << 5) + (1 << 7);
  mul += (1 << 11) + (1 << 13) + (1 << 17) + (0 << 19) + (1 << 23) + (1 << 29);
  mul += (1u << 31);
  if (len >= 8)
    {
      uint32_t acc = len * 0x9e3779b1;
      while (len >= 8)
	{
	  uint32_t i1 = hash_read32  (str) ^ (0x396cfeb8 + 1*len);
	  uint32_t i2 = hash_read32  (str + 4) ^ (0xbe4ba423 + 1*len);
	  str += 8;
	  len -= 8;
	  uint64_t m = (uint64_t)i1 * i2;
	  acc += (uint32_t)m ^ (uint32_t)(m >> 32);
	}
      acc = acc ^ (acc >> 7);
      uint64_t r = (uint64_t)mul * acc;
      ret = (uint32_t)r ^ (uint32_t)(r >> 32);
      if (len == 0)
	goto end;
    }
  if (len >= 4)
    {
      uint32_t i1 = hash_read32  (str);
      uint32_t i2 = hash_read32  (str + len - 4);
      i1 = ((i1 + len) ^ (i1 >> 7));
      i2 = i2 ^ (i2 >> 7);
      uint64_t r = (uint64_t)mul * i1 + i2;
      ret += r ^ (r >> 32);
    }
  else
    {
      /* Cleverly read in 1 to 3 bytes without further conditionals.  */
      unsigned char c1 = str[0];
      unsigned char c2 = str[len >> 1];
      unsigned char c3 = str[len - 1];
      uint32_t i1 = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24)
		     | ((uint32_t) c3) | (len << 8);
      i1 = i1 ^ (i1 >> 7);
      uint64_t r = (uint64_t)mul * i1;
      ret += r ^ (r >> 32);
    }
end:
  return ret;
}

/* Given a hash TABLE, return the hash of STRING (a blob described
   according to info in TABLE, either a character string, or some fixed
   size entity) and set *PLEN to the length of this blob.  */

static uint32_t
hashit (struct sec_merge_hash *table, const char *string, unsigned int *plen)
{
  const unsigned char *s;
  uint32_t hash;
  unsigned int len, i;

  s = (const unsigned char *) string;
  if (table->strings)
    {
      if (table->entsize == 1)
	len = strlen (string) + 1;
      else
	{
	  len = 0;
	  for (;;)
	    {
	      for (i = 0; i < table->entsize; ++i)
		if (s[i] != '\0')
		  break;
	      if (i == table->entsize)
		break;
	      s += table->entsize;
	      ++len;
	    }
	  len *= table->entsize;
	  len += table->entsize;
	}
    }
  else
    len = table->entsize;
  hash = hash_blob (string, len);
  *plen = len;
  return hash;
}

/* Lookup or insert a blob STRING (of length LEN, precomputed HASH and
   input ALIGNMENT) into TABLE.  Return the found or new hash table entry.  */

static struct sec_merge_hash_entry *
sec_merge_hash_lookup (struct sec_merge_hash *table, const char *string,
		       unsigned int len, uint64_t hash,
		       unsigned int alignment)
{
  struct sec_merge_hash_entry *hashp;
  unsigned int _index;

  /*printf ("YYY insert 0x%x into %u buckets (%s)\n",
	  (unsigned)hash, (unsigned)table->nbuckets, string);*/
  uint64_t *key_lens = table->key_lens;
  struct sec_merge_hash_entry **values = table->values;
  uint64_t hlen = (hash << 32) | (uint32_t)len;
  unsigned int nbuckets = table->nbuckets;
  _index = hash & (nbuckets - 1);
  while (1)
    {
      uint64_t candlen = key_lens[_index];
      if (candlen == hlen
	  && !memcmp (values[_index]->str, string, len))
	{
	  hashp = values[_index];
	  if (hashp->alignment < alignment)
	    hashp->alignment = alignment;
	  return hashp;
	}
      if (!(candlen & (uint32_t)-1))
	break;
      _index = (_index + 1) & (nbuckets - 1);
    }

  hashp = sec_merge_hash_insert (table, string, hash, len, _index);
  if (hashp == NULL)
    return NULL;
  hashp->alignment = alignment;

  table->size++;
  BFD_ASSERT (table->size == table->table.count);
  if (table->first == NULL)
    table->first = hashp;
  else
    table->last->next = hashp;
  table->last = hashp;

  return hashp;
}

/* Create a new hash table.  */

static struct sec_merge_hash *
sec_merge_init (unsigned int entsize, bool strings)
{
  struct sec_merge_hash *table;

  table = (struct sec_merge_hash *) bfd_malloc (sizeof (struct sec_merge_hash));
  if (table == NULL)
    return NULL;

  if (! bfd_hash_table_init_n (&table->table, NULL,
			       sizeof (struct sec_merge_hash_entry), 0x2000))
    {
      free (table);
      return NULL;
    }

  table->size = 0;
  table->first = NULL;
  table->last = NULL;
  table->entsize = entsize;
  table->strings = strings;

  table->nbuckets = 0x2000;
  table->key_lens = objalloc_alloc ((struct objalloc *) table->table.memory,
				table->nbuckets * sizeof (table->key_lens[0]));
  memset (table->key_lens, 0, table->nbuckets * sizeof (table->key_lens[0]));
  table->values = objalloc_alloc ((struct objalloc *) table->table.memory,
				table->nbuckets * sizeof (table->values[0]));
  memset (table->values, 0, table->nbuckets * sizeof (table->values[0]));

  return table;
}

/* Append the tuple of input-offset O corresponding
   to hash table ENTRY into SECINFO, such that we later may lookup the
   entry just by O.  */

static bool
append_offsetmap (struct sec_merge_sec_info *secinfo,
		  mapofs_type o,
		  struct sec_merge_hash_entry *entry)
{
  if ((secinfo->noffsetmap & 2047) == 0)
    {
      bfd_size_type amt;
      amt = (secinfo->noffsetmap + 2048);
      secinfo->map_ofs = bfd_realloc (secinfo->map_ofs,
				      amt * sizeof(secinfo->map_ofs[0]));
      if (!secinfo->map_ofs)
	return false;
      secinfo->map = bfd_realloc (secinfo->map, amt * sizeof(secinfo->map[0]));
      if (!secinfo->map)
	return false;
    }
  unsigned int i = secinfo->noffsetmap++;
  MAP_OFS(secinfo, i) = o;
  secinfo->map[i].entry = entry;
  return true;
}

/* Prepare the input-offset-to-entry tables after output offsets are
   determined.  */

static void
prepare_offsetmap (struct sec_merge_sec_info *secinfo)
{
  unsigned int noffsetmap = secinfo->noffsetmap;
  unsigned int i, lbi;
  bfd_size_type l, sz, amt;

  secinfo->fast_state = 1;

  for (i = 0; i < noffsetmap; i++)
    MAP_IDX(secinfo, i) = secinfo->map[i].entry->u.index;

  sz = secinfo->sec->rawsize;
  amt = (sz / OFSDIV + 1) * sizeof (secinfo->ofstolowbound[0]);
  secinfo->ofstolowbound = bfd_zmalloc (amt);
  if (!secinfo->ofstolowbound)
    return;
  for (l = lbi = 0; l < sz; l += OFSDIV)
    {
      /* No need for bounds checking on lbi, as we've added a sentinel that's
	 larger than any offset.  */
      while (MAP_OFS(secinfo, lbi) <= l)
	lbi++;
      //BFD_ASSERT ((l / OFSDIV) <= (i / OFSDIV));
      secinfo->ofstolowbound[l / OFSDIV] = lbi;
    }
  secinfo->fast_state = 2;
}

static bool
sec_merge_emit (bfd *abfd, struct sec_merge_sec_info *secinfo,
		unsigned char *contents)
{
  struct sec_merge_hash_entry *entry = secinfo->first_str;
  asection *sec = secinfo->sec;
  file_ptr offset = sec->output_offset;
  char *pad = NULL;
  bfd_size_type off = 0;
  unsigned int opb = bfd_octets_per_byte (abfd, sec);
  int alignment_power = sec->output_section->alignment_power * opb;
  bfd_size_type pad_len;  /* Octets.  */

  /* FIXME: If alignment_power is 0 then really we should scan the
     entry list for the largest required alignment and use that.  */
  pad_len = alignment_power ? ((bfd_size_type) 1 << alignment_power) : 16;

  pad = (char *) bfd_zmalloc (pad_len);
  if (pad == NULL)
    return false;

  for (; entry != NULL; entry = entry->next)
    {
      const char *str;
      bfd_size_type len;

      if (!entry->len)
	continue;
      BFD_ASSERT (entry->alignment);
      len = -off & (entry->alignment - 1);
      if (len != 0)
	{
	  BFD_ASSERT (len <= pad_len);
	  if (contents)
	    {
	      memcpy (contents + offset, pad, len);
	      offset += len;
	    }
	  else if (bfd_bwrite (pad, len, abfd) != len)
	    goto err;
	  off += len;
	}

      str = entry->str;
      len = entry->len;

      if (contents)
	{
	  memcpy (contents + offset, str, len);
	  offset += len;
	}
      else if (bfd_bwrite (str, len, abfd) != len)
	goto err;

      off += len;
    }
  BFD_ASSERT (!entry);

  /* Trailing alignment needed?  */
  off = sec->size - off;
  if (1 && off != 0)
    {
      BFD_ASSERT (off <= pad_len);
      if (contents)
	memcpy (contents + offset, pad, off);
      else if (bfd_bwrite (pad, off, abfd) != off)
	goto err;
    }

  free (pad);
  return true;

 err:
  free (pad);
  return false;
}

/* Register a SEC_MERGE section as a candidate for merging.
   This function is called for all non-dynamic SEC_MERGE input sections.  */

bool
_bfd_add_merge_section (bfd *abfd, void **psinfo, asection *sec,
			void **psecinfo)
{
  struct sec_merge_info *sinfo;
  struct sec_merge_sec_info *secinfo;
  asection *repr;
  unsigned int alignment_power;  /* Octets.  */
  unsigned int align;            /* Octets.  */
  unsigned int opb = bfd_octets_per_byte (abfd, sec);

  if ((abfd->flags & DYNAMIC) != 0
      || (sec->flags & SEC_MERGE) == 0)
    abort ();

  if (sec->size == 0
      || (sec->flags & SEC_EXCLUDE) != 0
      || sec->entsize == 0)
    return true;

  if (sec->size % sec->entsize != 0)
    return true;

  if ((sec->flags & SEC_RELOC) != 0)
    {
      /* We aren't prepared to handle relocations in merged sections.  */
      return true;
    }

  if (sec->size > (mapofs_type)-1)
    {
      /* Input offsets must be representable by mapofs_type.  */
      return true;
    }

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
  alignment_power = sec->alignment_power * opb;
  if (alignment_power >= sizeof (align) * CHAR_BIT)
    return true;

  align = 1u << alignment_power;
  if ((sec->entsize < align
       && ((sec->entsize & (sec->entsize - 1))
	   || !(sec->flags & SEC_STRINGS)))
      || (sec->entsize > align
	  && (sec->entsize & (align - 1))))
    {
      /* Sanity check.  If string character size is smaller than
	 alignment, then we require character size to be a power
	 of 2, otherwise character size must be integer multiple
	 of alignment.  For non-string constants, alignment must
	 be smaller than or equal to entity size and entity size
	 must be integer multiple of alignment.  */
      return true;
    }

  /* Initialize the descriptor for this input section.  */

  *psecinfo = secinfo = bfd_zalloc (abfd, sizeof (*secinfo));
  if (*psecinfo == NULL)
    goto error_return;

  secinfo->sec = sec;
  secinfo->psecinfo = psecinfo;

  /* Search for a matching output merged section.  */
  for (sinfo = (struct sec_merge_info *) *psinfo; sinfo; sinfo = sinfo->next)
    if (sinfo->chain
	&& (repr = sinfo->chain->sec)
	&& ! ((repr->flags ^ sec->flags) & (SEC_MERGE | SEC_STRINGS))
	&& repr->entsize == sec->entsize
	&& repr->alignment_power == sec->alignment_power
	&& repr->output_section == sec->output_section)
      break;

  if (sinfo == NULL)
    {
      /* Initialize the information we need to keep track of.  */
      sinfo = (struct sec_merge_info *)
	  bfd_alloc (abfd, sizeof (struct sec_merge_info));
      if (sinfo == NULL)
	goto error_return;
      sinfo->next = (struct sec_merge_info *) *psinfo;
      sinfo->chain = NULL;
      sinfo->last = &sinfo->chain;
      *psinfo = sinfo;
      sinfo->htab = sec_merge_init (sec->entsize, (sec->flags & SEC_STRINGS));
      if (sinfo->htab == NULL)
	goto error_return;
    }

  *sinfo->last = secinfo;
  sinfo->last = &secinfo->next;

  secinfo->sinfo = sinfo;
  secinfo->reprsec = sinfo->chain->sec;

  return true;

 error_return:
  *psecinfo = NULL;
  return false;
}

/* Record one whole input section (described by SECINFO) into the hash table
   SINFO.  */

static bool
record_section (struct sec_merge_info *sinfo,
		struct sec_merge_sec_info *secinfo)
{
  asection *sec = secinfo->sec;
  struct sec_merge_hash_entry *entry;
  unsigned char *p, *end;
  bfd_vma mask, eltalign;
  unsigned int align;
  bfd_size_type amt;
  bfd_byte *contents;

  amt = sec->size;
  if (sec->flags & SEC_STRINGS)
    /* Some versions of gcc may emit a string without a zero terminator.
       See http://gcc.gnu.org/ml/gcc-patches/2006-06/msg01004.html
       Allocate space for an extra zero.  */
    amt += sec->entsize;
  contents = bfd_malloc (amt);
  if (!contents)
    goto error_return;

  /* Slurp in all section contents (possibly decompressing it).  */
  sec->rawsize = sec->size;
  if (sec->flags & SEC_STRINGS)
    memset (contents + sec->size, 0, sec->entsize);
  if (! bfd_get_full_section_contents (sec->owner, sec, &contents))
    goto error_return;

  /* Now populate the hash table and offset mapping.  */

  /* Presize the hash table for what we're going to add.  We overestimate
     quite a bit, but if it turns out to be too much then other sections
     merged into this area will make use of that as well.  */
  if (!sec_merge_maybe_resize (sinfo->htab, 1 + sec->size / 2))
    {
      bfd_set_error (bfd_error_no_memory);
      goto error_return;
    }

  /* Walk through the contents, calculate hashes and length of all
     blobs (strings or fixed-size entries) we find and fill the
     hash and offset tables.  */
  align = sec->alignment_power;
  mask = ((bfd_vma) 1 << align) - 1;
  end = contents + sec->size;
  for (p = contents; p < end;)
    {
      unsigned len;
      uint32_t hash = hashit (sinfo->htab, (char*) p, &len);
      unsigned int ofs = p - contents;
      eltalign = ofs;
      eltalign = ((eltalign ^ (eltalign - 1)) + 1) >> 1;
      if (!eltalign || eltalign > mask)
	eltalign = mask + 1;
      entry = sec_merge_hash_lookup (sinfo->htab, (char *) p, len, hash,
				     (unsigned) eltalign);
      if (! entry)
	goto error_return;
      if (! append_offsetmap (secinfo, ofs, entry))
	goto error_return;
      p += len;
    }

  /* Add a sentinel element that's conceptually behind all others.  */
  append_offsetmap (secinfo, sec->size, NULL);
  /* But don't count it.  */
  secinfo->noffsetmap--;

  free (contents);
  contents = NULL;
  /*printf ("ZZZ %s:%s %u entries\n", sec->owner->filename, sec->name,
	  (unsigned)secinfo->noffsetmap);*/

  return true;

 error_return:
  free (contents);
  contents = NULL;
  for (secinfo = sinfo->chain; secinfo; secinfo = secinfo->next)
    *secinfo->psecinfo = NULL;
  return false;
}

/* qsort comparison function.  Won't ever return zero as all entries
   differ, so there is no issue with qsort stability here.  */

static int
strrevcmp (const void *a, const void *b)
{
  struct sec_merge_hash_entry *A = *(struct sec_merge_hash_entry **) a;
  struct sec_merge_hash_entry *B = *(struct sec_merge_hash_entry **) b;
  unsigned int lenA = A->len;
  unsigned int lenB = B->len;
  const unsigned char *s = (const unsigned char *) A->str + lenA - 1;
  const unsigned char *t = (const unsigned char *) B->str + lenB - 1;
  int l = lenA < lenB ? lenA : lenB;

  while (l)
    {
      if (*s != *t)
	return (int) *s - (int) *t;
      s--;
      t--;
      l--;
    }
  return lenA - lenB;
}

/* Like strrevcmp, but for the case where all strings have the same
   alignment > entsize.  */

static int
strrevcmp_align (const void *a, const void *b)
{
  struct sec_merge_hash_entry *A = *(struct sec_merge_hash_entry **) a;
  struct sec_merge_hash_entry *B = *(struct sec_merge_hash_entry **) b;
  unsigned int lenA = A->len;
  unsigned int lenB = B->len;
  const unsigned char *s = (const unsigned char *) A->str + lenA - 1;
  const unsigned char *t = (const unsigned char *) B->str + lenB - 1;
  int l = lenA < lenB ? lenA : lenB;
  int tail_align = (lenA & (A->alignment - 1)) - (lenB & (A->alignment - 1));

  if (tail_align != 0)
    return tail_align;

  while (l)
    {
      if (*s != *t)
	return (int) *s - (int) *t;
      s--;
      t--;
      l--;
    }
  return lenA - lenB;
}

static inline int
is_suffix (const struct sec_merge_hash_entry *A,
	   const struct sec_merge_hash_entry *B)
{
  if (A->len <= B->len)
    /* B cannot be a suffix of A unless A is equal to B, which is guaranteed
       not to be equal by the hash table.  */
    return 0;

  return memcmp (A->str + (A->len - B->len),
		 B->str, B->len) == 0;
}

/* This is a helper function for _bfd_merge_sections.  It attempts to
   merge strings matching suffixes of longer strings.  */
static struct sec_merge_sec_info *
merge_strings (struct sec_merge_info *sinfo)
{
  struct sec_merge_hash_entry **array, **a, *e;
  struct sec_merge_sec_info *secinfo;
  bfd_size_type size, amt;
  unsigned int alignment = 0;

  /* Now sort the strings */
  amt = sinfo->htab->size * sizeof (struct sec_merge_hash_entry *);
  array = (struct sec_merge_hash_entry **) bfd_malloc (amt);
  if (array == NULL)
    return NULL;

  for (e = sinfo->htab->first, a = array; e; e = e->next)
    if (e->alignment)
      {
	*a++ = e;
	/* Adjust the length to not include the zero terminator.  */
	e->len -= sinfo->htab->entsize;
	if (alignment != e->alignment)
	  {
	    if (alignment == 0)
	      alignment = e->alignment;
	    else
	      alignment = (unsigned) -1;
	  }
      }

  sinfo->htab->size = a - array;
  if (sinfo->htab->size != 0)
    {
      qsort (array, (size_t) sinfo->htab->size,
	     sizeof (struct sec_merge_hash_entry *),
	     (alignment != (unsigned) -1 && alignment > sinfo->htab->entsize
	      ? strrevcmp_align : strrevcmp));

      /* Loop over the sorted array and merge suffixes */
      e = *--a;
      e->len += sinfo->htab->entsize;
      while (--a >= array)
	{
	  struct sec_merge_hash_entry *cmp = *a;

	  cmp->len += sinfo->htab->entsize;
	  if (e->alignment >= cmp->alignment
	      && !((e->len - cmp->len) & (cmp->alignment - 1))
	      && is_suffix (e, cmp))
	    {
	      cmp->u.suffix = e;
	      cmp->alignment = 0;
	    }
	  else
	    e = cmp;
	}
    }

  free (array);

  /* Now assign positions to the strings we want to keep.  */
  size = 0;
  secinfo = sinfo->chain;
  for (e = sinfo->htab->first; e; e = e->next)
    {
      if (e->alignment)
	{
	  size = (size + e->alignment - 1) & ~((bfd_vma) e->alignment - 1);
	  e->u.index = size;
	  size += e->len;
	}
    }
  secinfo->sec->size = size;

  /* And now adjust the rest, removing them from the chain (but not hashtable)
     at the same time.  */
  for (a = &sinfo->htab->first, e = *a; e; e = e->next)
    if (e->alignment)
      a = &e->next;
    else
      {
	*a = e->next;
	if (e->len)
	  {
	    e->alignment = e->u.suffix->alignment;
	    e->u.index = e->u.suffix->u.index + (e->u.suffix->len - e->len);
	  }
      }

  BFD_ASSERT (!secinfo->first_str);
  secinfo->first_str = sinfo->htab->first;

  return secinfo;
}

/* This function is called once after all SEC_MERGE sections are registered
   with _bfd_merge_section.  */

bool
_bfd_merge_sections (bfd *abfd,
		     struct bfd_link_info *info ATTRIBUTE_UNUSED,
		     void *xsinfo,
		     void (*remove_hook) (bfd *, asection *))
{
  struct sec_merge_info *sinfo;

  for (sinfo = (struct sec_merge_info *) xsinfo; sinfo; sinfo = sinfo->next)
    {
      struct sec_merge_sec_info *secinfo;
      bfd_size_type align;  /* Bytes.  */

      if (! sinfo->chain)
	continue;

      /* Record the sections into the hash table.  */
      align = 1;
      for (secinfo = sinfo->chain; secinfo; secinfo = secinfo->next)
	if (secinfo->sec->flags & SEC_EXCLUDE)
	  {
	    *secinfo->psecinfo = NULL;
	    if (remove_hook)
	      (*remove_hook) (abfd, secinfo->sec);
	  }
	else
	  {
	    if (!record_section (sinfo, secinfo))
	      return false;
	    if (align)
	      {
		unsigned int opb = bfd_octets_per_byte (abfd, secinfo->sec);

		align = (bfd_size_type) 1 << secinfo->sec->alignment_power;
		if (((secinfo->sec->size / opb) & (align - 1)) != 0)
		  align = 0;
	      }
	  }

      if (sinfo->htab->first == NULL)
	continue;

      if (sinfo->htab->strings)
	{
	  secinfo = merge_strings (sinfo);
	  if (!secinfo)
	    return false;
	}
      else
	{
	  struct sec_merge_hash_entry *e = sinfo->htab->first;
	  bfd_size_type size = 0;  /* Octets.  */

	  /* Things are much simpler for non-strings.
	     Just assign them slots in the section.  */
	  secinfo = sinfo->chain;
	  BFD_ASSERT (!secinfo->first_str);
	  secinfo->first_str = e;
	  for (e = sinfo->htab->first; e; e = e->next)
	    {
	      if (e->alignment)
		{
		  size = (size + e->alignment - 1)
			 & ~((bfd_vma) e->alignment - 1);
		  e->u.index = size;
		  size += e->len;
		}
	    }
	  secinfo->sec->size = size;
	}

      /* If the input sections were padded according to their alignments,
	 then pad the output too.  */
      if (align)
	secinfo->sec->size = (secinfo->sec->size + align - 1) & -align;

      /* Finally remove all input sections which have not made it into
	 the hash table at all.  */
      for (secinfo = sinfo->chain; secinfo; secinfo = secinfo->next)
	if (secinfo->first_str == NULL)
	  secinfo->sec->flags |= SEC_EXCLUDE | SEC_KEEP;
    }

  return true;
}

/* Write out the merged section.  */

bool
_bfd_write_merged_section (bfd *output_bfd, asection *sec, void *psecinfo)
{
  struct sec_merge_sec_info *secinfo;
  file_ptr pos;
  unsigned char *contents;
  Elf_Internal_Shdr *hdr;

  secinfo = (struct sec_merge_sec_info *) psecinfo;

  if (!secinfo)
    return false;

  if (secinfo->first_str == NULL)
    return true;

  /* FIXME: octets_per_byte.  */
  hdr = &elf_section_data (sec->output_section)->this_hdr;
  if (hdr->sh_offset == (file_ptr) -1)
    {
      /* We must compress this section.  Write output to the
	 buffer.  */
      contents = hdr->contents;
      if (contents == NULL)
	abort ();
    }
  else
    {
      contents = NULL;
      pos = sec->output_section->filepos + sec->output_offset;
      if (bfd_seek (output_bfd, pos, SEEK_SET) != 0)
	return false;
    }

  BFD_ASSERT (sec == secinfo->sec);
  BFD_ASSERT (secinfo == secinfo->sinfo->chain);
  if (! sec_merge_emit (output_bfd, secinfo, contents))
    return false;

  return true;
}

/* Adjust an address in the SEC_MERGE section.  Given OFFSET within
   *PSEC, this returns the new offset in the adjusted SEC_MERGE
   section and writes the new section back into *PSEC.  */

bfd_vma
_bfd_merged_section_offset (bfd *output_bfd ATTRIBUTE_UNUSED, asection **psec,
			    void *psecinfo, bfd_vma offset)
{
  struct sec_merge_sec_info *secinfo;
  asection *sec = *psec;

  secinfo = (struct sec_merge_sec_info *) psecinfo;

  if (!secinfo)
    return offset;

  if (offset >= sec->rawsize)
    {
      if (offset > sec->rawsize)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: access beyond end of merged section (%" PRId64 ")"),
	   sec->owner, (int64_t) offset);
      return secinfo->first_str ? sec->size : 0;
    }

  if (secinfo->fast_state != 2)
    {
      if (!secinfo->fast_state)
	prepare_offsetmap (secinfo);
      if (secinfo->fast_state != 2)
	return offset;
    }

  long lb = secinfo->ofstolowbound[offset / OFSDIV];
  *psec = secinfo->reprsec;

  /* No need for bounds checking on lb, as we've added a sentinel that's
     larger than any offset.  */
  while (MAP_OFS(secinfo, lb) <= offset)
    lb++;
  lb--;

  /*printf ("YYY (%s:%s):%u -> (%s):%u\n",
	  sec->owner->filename, sec->name, (unsigned)offset,
	  (*psec)->name, (unsigned)lb);*/
  return MAP_IDX(secinfo, lb) + offset - MAP_OFS(secinfo, lb);
}

/* Tidy up when done.  */

void
_bfd_merge_sections_free (void *xsinfo)
{
  struct sec_merge_info *sinfo;

  for (sinfo = (struct sec_merge_info *) xsinfo; sinfo; sinfo = sinfo->next)
    {
      struct sec_merge_sec_info *secinfo;
      for (secinfo = sinfo->chain; secinfo; secinfo = secinfo->next)
	{
	  free (secinfo->ofstolowbound);
	  free (secinfo->map);
	  free (secinfo->map_ofs);
	}
      bfd_hash_table_free (&sinfo->htab->table);
      free (sinfo->htab);
    }
}
