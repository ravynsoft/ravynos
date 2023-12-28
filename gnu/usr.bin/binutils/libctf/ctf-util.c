/* Miscellaneous utilities.
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
#include "ctf-endian.h"

/* Simple doubly-linked list append routine.  This implementation assumes that
   each list element contains an embedded ctf_list_t as the first member.
   An additional ctf_list_t is used to store the head (l_next) and tail
   (l_prev) pointers.  The current head and tail list elements have their
   previous and next pointers set to NULL, respectively.  */

void
ctf_list_append (ctf_list_t *lp, void *newp)
{
  ctf_list_t *p = lp->l_prev;	/* p = tail list element.  */
  ctf_list_t *q = newp;		/* q = new list element.  */

  lp->l_prev = q;
  q->l_prev = p;
  q->l_next = NULL;

  if (p != NULL)
    p->l_next = q;
  else
    lp->l_next = q;
}

/* Prepend the specified existing element to the given ctf_list_t.  The
   existing pointer should be pointing at a struct with embedded ctf_list_t.  */

void
ctf_list_prepend (ctf_list_t * lp, void *newp)
{
  ctf_list_t *p = newp;		/* p = new list element.  */
  ctf_list_t *q = lp->l_next;	/* q = head list element.  */

  lp->l_next = p;
  p->l_prev = NULL;
  p->l_next = q;

  if (q != NULL)
    q->l_prev = p;
  else
    lp->l_prev = p;
}

/* Delete the specified existing element from the given ctf_list_t.  The
   existing pointer should be pointing at a struct with embedded ctf_list_t.  */

void
ctf_list_delete (ctf_list_t *lp, void *existing)
{
  ctf_list_t *p = existing;

  if (p->l_prev != NULL)
    p->l_prev->l_next = p->l_next;
  else
    lp->l_next = p->l_next;

  if (p->l_next != NULL)
    p->l_next->l_prev = p->l_prev;
  else
    lp->l_prev = p->l_prev;
}

/* Return 1 if the list is empty.  */

int
ctf_list_empty_p (ctf_list_t *lp)
{
  return (lp->l_next == NULL && lp->l_prev == NULL);
}

/* Splice one entire list onto the end of another one.  The existing list is
   emptied.  */

void
ctf_list_splice (ctf_list_t *lp, ctf_list_t *append)
{
  if (ctf_list_empty_p (append))
    return;

  if (lp->l_prev != NULL)
    lp->l_prev->l_next = append->l_next;
  else
    lp->l_next = append->l_next;

  append->l_next->l_prev = lp->l_prev;
  lp->l_prev = append->l_prev;
  append->l_next = NULL;
  append->l_prev = NULL;
}

/* Convert a 32-bit ELF symbol to a ctf_link_sym_t.  */

ctf_link_sym_t *
ctf_elf32_to_link_sym (ctf_dict_t *fp, ctf_link_sym_t *dst, const Elf32_Sym *src,
		       uint32_t symidx)
{
  Elf32_Sym tmp;
  int needs_flipping = 0;

#ifdef WORDS_BIGENDIAN
  if (fp->ctf_symsect_little_endian)
    needs_flipping = 1;
#else
  if (!fp->ctf_symsect_little_endian)
    needs_flipping = 1;
#endif

  memcpy (&tmp, src, sizeof (Elf32_Sym));
  if (needs_flipping)
    {
      swap_thing (tmp.st_name);
      swap_thing (tmp.st_size);
      swap_thing (tmp.st_shndx);
      swap_thing (tmp.st_value);
    }
  /* The name must be in the external string table.  */
  if (tmp.st_name < fp->ctf_str[CTF_STRTAB_1].cts_len)
    dst->st_name = (const char *) fp->ctf_str[CTF_STRTAB_1].cts_strs + tmp.st_name;
  else
    dst->st_name = _CTF_NULLSTR;
  dst->st_nameidx_set = 0;
  dst->st_symidx = symidx;
  dst->st_shndx = tmp.st_shndx;
  dst->st_type = ELF32_ST_TYPE (tmp.st_info);
  dst->st_value = tmp.st_value;

  return dst;
}

/* Convert a 64-bit ELF symbol to a ctf_link_sym_t.  */

ctf_link_sym_t *
ctf_elf64_to_link_sym (ctf_dict_t *fp, ctf_link_sym_t *dst, const Elf64_Sym *src,
		       uint32_t symidx)
{
  Elf64_Sym tmp;
  int needs_flipping = 0;

#ifdef WORDS_BIGENDIAN
  if (fp->ctf_symsect_little_endian)
    needs_flipping = 1;
#else
  if (!fp->ctf_symsect_little_endian)
    needs_flipping = 1;
#endif

  memcpy (&tmp, src, sizeof (Elf64_Sym));
  if (needs_flipping)
    {
      swap_thing (tmp.st_name);
      swap_thing (tmp.st_size);
      swap_thing (tmp.st_shndx);
      swap_thing (tmp.st_value);
    }

  /* The name must be in the external string table.  */
  if (tmp.st_name < fp->ctf_str[CTF_STRTAB_1].cts_len)
    dst->st_name = (const char *) fp->ctf_str[CTF_STRTAB_1].cts_strs + tmp.st_name;
  else
    dst->st_name = _CTF_NULLSTR;
  dst->st_nameidx_set = 0;
  dst->st_symidx = symidx;
  dst->st_shndx = tmp.st_shndx;
  dst->st_type = ELF32_ST_TYPE (tmp.st_info);

  /* We only care if the value is zero, so avoid nonzeroes turning into
     zeroes.  */
  if (_libctf_unlikely_ (tmp.st_value != 0 && ((uint32_t) tmp.st_value == 0)))
    dst->st_value = 1;
  else
    dst->st_value = (uint32_t) tmp.st_value;

  return dst;
}

/* A string appender working on dynamic strings.  Returns NULL on OOM.  */

char *
ctf_str_append (char *s, const char *append)
{
  size_t s_len = 0;

  if (append == NULL)
    return s;

  if (s != NULL)
    s_len = strlen (s);

  size_t append_len = strlen (append);

  if ((s = realloc (s, s_len + append_len + 1)) == NULL)
    return NULL;

  memcpy (s + s_len, append, append_len);
  s[s_len + append_len] = '\0';

  return s;
}

/* A version of ctf_str_append that returns the old string on OOM.  */

char *
ctf_str_append_noerr (char *s, const char *append)
{
  char *new_s;

  new_s = ctf_str_append (s, append);
  if (!new_s)
    return s;
  return new_s;
}

/* A realloc() that fails noisily if called with any ctf_str_num_users.  */
void *
ctf_realloc (ctf_dict_t *fp, void *ptr, size_t size)
{
  if (fp->ctf_str_num_refs > 0)
    {
      ctf_dprintf ("%p: attempt to realloc() string table with %lu active refs\n",
		   (void *) fp, (unsigned long) fp->ctf_str_num_refs);
      return NULL;
    }
  return realloc (ptr, size);
}

/* Store the specified error code into errp if it is non-NULL, and then
   return NULL for the benefit of the caller.  */

void *
ctf_set_open_errno (int *errp, int error)
{
  if (errp != NULL)
    *errp = error;
  return NULL;
}

/* Store the specified error code into the CTF dict, and then return CTF_ERR /
   -1 for the benefit of the caller. */

unsigned long
ctf_set_errno (ctf_dict_t *fp, int err)
{
  fp->ctf_errno = err;
  return CTF_ERR;
}

/* Create a ctf_next_t.  */

ctf_next_t *
ctf_next_create (void)
{
  return calloc (1, sizeof (struct ctf_next));
}

/* Destroy a ctf_next_t, for early exit from iterators.  */

void
ctf_next_destroy (ctf_next_t *i)
{
  if (i == NULL)
    return;

  if (i->ctn_iter_fun == (void (*) (void)) ctf_dynhash_next_sorted)
    free (i->u.ctn_sorted_hkv);
  if (i->ctn_next)
    ctf_next_destroy (i->ctn_next);
  free (i);
}

/* Copy a ctf_next_t.  */

ctf_next_t *
ctf_next_copy (ctf_next_t *i)
{
  ctf_next_t *i2;

  if ((i2 = ctf_next_create()) == NULL)
    return NULL;
  memcpy (i2, i, sizeof (struct ctf_next));

  if (i2->ctn_iter_fun == (void (*) (void)) ctf_dynhash_next_sorted)
    {
      size_t els = ctf_dynhash_elements ((ctf_dynhash_t *) i->cu.ctn_h);
      if ((i2->u.ctn_sorted_hkv = calloc (els, sizeof (ctf_next_hkv_t))) == NULL)
	{
	  free (i2);
	  return NULL;
	}
      memcpy (i2->u.ctn_sorted_hkv, i->u.ctn_sorted_hkv,
	      els * sizeof (ctf_next_hkv_t));
    }
  return i2;
}
