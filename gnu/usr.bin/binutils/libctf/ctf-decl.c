/* C declarator syntax glue.
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

/* CTF Declaration Stack

   In order to implement ctf_type_name(), we must convert a type graph back
   into a C type declaration.  Unfortunately, a type graph represents a storage
   class ordering of the type whereas a type declaration must obey the C rules
   for operator precedence, and the two orderings are frequently in conflict.
   For example, consider these CTF type graphs and their C declarations:

   CTF_K_POINTER -> CTF_K_FUNCTION -> CTF_K_INTEGER  : int (*)()
   CTF_K_POINTER -> CTF_K_ARRAY -> CTF_K_INTEGER     : int (*)[]

   In each case, parentheses are used to raise operator * to higher lexical
   precedence, so the string form of the C declaration cannot be constructed by
   walking the type graph links and forming the string from left to right.

   The functions in this file build a set of stacks from the type graph nodes
   corresponding to the C operator precedence levels in the appropriate order.
   The code in ctf_type_name() can then iterate over the levels and nodes in
   lexical precedence order and construct the final C declaration string.  */

#include <ctf-impl.h>
#include <string.h>

void
ctf_decl_init (ctf_decl_t *cd)
{
  int i;

  memset (cd, 0, sizeof (ctf_decl_t));

  for (i = CTF_PREC_BASE; i < CTF_PREC_MAX; i++)
    cd->cd_order[i] = CTF_PREC_BASE - 1;

  cd->cd_qualp = CTF_PREC_BASE;
  cd->cd_ordp = CTF_PREC_BASE;
}

void
ctf_decl_fini (ctf_decl_t *cd)
{
  ctf_decl_node_t *cdp, *ndp;
  int i;

  for (i = CTF_PREC_BASE; i < CTF_PREC_MAX; i++)
    {
      for (cdp = ctf_list_next (&cd->cd_nodes[i]); cdp != NULL; cdp = ndp)
	{
	  ndp = ctf_list_next (cdp);
	  free (cdp);
	}
    }
  free (cd->cd_buf);
}

void
ctf_decl_push (ctf_decl_t *cd, ctf_dict_t *fp, ctf_id_t type)
{
  ctf_decl_node_t *cdp;
  ctf_decl_prec_t prec;
  uint32_t kind, n = 1;
  int is_qual = 0;

  const ctf_type_t *tp;
  ctf_arinfo_t ar;

  if ((tp = ctf_lookup_by_id (&fp, type)) == NULL)
    {
      cd->cd_err = fp->ctf_errno;
      return;
    }

  switch (kind = LCTF_INFO_KIND (fp, tp->ctt_info))
    {
    case CTF_K_ARRAY:
      (void) ctf_array_info (fp, type, &ar);
      ctf_decl_push (cd, fp, ar.ctr_contents);
      n = ar.ctr_nelems;
      prec = CTF_PREC_ARRAY;
      break;

    case CTF_K_TYPEDEF:
      if (ctf_strptr (fp, tp->ctt_name)[0] == '\0')
	{
	  ctf_decl_push (cd, fp, tp->ctt_type);
	  return;
	}
      prec = CTF_PREC_BASE;
      break;

    case CTF_K_FUNCTION:
      ctf_decl_push (cd, fp, tp->ctt_type);
      prec = CTF_PREC_FUNCTION;
      break;

    case CTF_K_POINTER:
      ctf_decl_push (cd, fp, tp->ctt_type);
      prec = CTF_PREC_POINTER;
      break;

    case CTF_K_SLICE:
      /* Slices themselves have no print representation and should not appear in
	 the decl stack.  */
      ctf_decl_push (cd, fp, ctf_type_reference (fp, type));
      return;

    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
      ctf_decl_push (cd, fp, tp->ctt_type);
      prec = cd->cd_qualp;
      is_qual++;
      break;

    default:
      prec = CTF_PREC_BASE;
    }

  if ((cdp = malloc (sizeof (ctf_decl_node_t))) == NULL)
    {
      cd->cd_err = EAGAIN;
      return;
    }

  cdp->cd_type = type;
  cdp->cd_kind = kind;
  cdp->cd_n = n;

  if (ctf_list_next (&cd->cd_nodes[prec]) == NULL)
    cd->cd_order[prec] = cd->cd_ordp++;

  /* Reset cd_qualp to the highest precedence level that we've seen so
     far that can be qualified (CTF_PREC_BASE or CTF_PREC_POINTER).  */

  if (prec > cd->cd_qualp && prec < CTF_PREC_ARRAY)
    cd->cd_qualp = prec;

  /* By convention qualifiers of base types precede the type specifier (e.g.
     const int vs. int const) even though the two forms are equivalent.  */

  if (is_qual && prec == CTF_PREC_BASE)
    ctf_list_prepend (&cd->cd_nodes[prec], cdp);
  else
    ctf_list_append (&cd->cd_nodes[prec], cdp);
}

_libctf_printflike_ (2, 3)
void ctf_decl_sprintf (ctf_decl_t *cd, const char *format, ...)
{
  va_list ap;
  char *str;
  int n;

  if (cd->cd_enomem)
    return;

  va_start (ap, format);
  n = vasprintf (&str, format, ap);
  va_end (ap);

  if (n > 0)
    {
      char *newbuf;
      if ((newbuf = ctf_str_append (cd->cd_buf, str)) != NULL)
	cd->cd_buf = newbuf;
    }

  /* Sticky error condition.  */
  if (n < 0 || cd->cd_buf == NULL)
    {
      free (cd->cd_buf);
      cd->cd_buf = NULL;
      cd->cd_enomem = 1;
    }

  free (str);
}

char *ctf_decl_buf (ctf_decl_t *cd)
{
  char *buf = cd->cd_buf;
  cd->cd_buf = NULL;
  return buf;
}
