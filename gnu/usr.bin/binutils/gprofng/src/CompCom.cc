/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/param.h>

#include "demangle.h"
#include "gp-defs.h"
#include "StringBuilder.h"
#include "CompCom.h"
#include "Elf.h"
#include "util.h"
#include "i18n.h"
#include "comp_com.c"

CompComment::CompComment (Elf *_elf, int _compcom)
{
  elf = _elf;
  compcom = _compcom;
  elf_cls = elf->elf_getclass ();
}

CompComment::~CompComment () { }

int
CompComment::get_align (int64_t offset, int align)
{
  int val = (int) (offset % align);
  if (val)
    val = align - val;
  return val;
}

/*
 * Preprocesses the header structure, builds a table of messages with the line
 * numbers, PCoffsets, original index, and compmsg pointer for each message.
 * If the show_bits field is not in the message, this routine would fill it in
 * from the mapping from COMPMSG_ID
 */
int
CompComment::compcom_open (CheckSrcName check_src)
{
  if (check_src == NULL)
    return 0;
  Elf_Data *data = elf->elf_getdata (compcom);
  uint64_t b_offset = data->d_off;
  if (get_align (b_offset, 4))   // not align 4
    return 0;
  char *CommData = (char *) data->d_buf;
  uint64_t offset = b_offset;
  for (uint64_t e_offset = b_offset + data->d_size; offset < e_offset;)
    {
      offset += get_align (offset, (int) data->d_align);
      if (offset >= e_offset)
	return 0;
      compcomhdr *hdr = (compcomhdr *) (CommData + (offset - b_offset));
      int hdr_msgcount = elf->decode (hdr->msgcount);
      int hdr_srcname = elf->decode (hdr->srcname);
      int hdr_stringlen = elf->decode (hdr->stringlen);
      int hdr_paramcount = elf->decode (hdr->paramcount);
      size_t length = sizeof (compcomhdr) + hdr_msgcount * sizeof (compmsg) +
	      hdr_paramcount * sizeof (int32_t);
      if (offset + length + hdr_stringlen > e_offset || hdr_srcname < 0
	  || hdr_srcname >= hdr_stringlen)
	return 0;

      // check source file
      char *src_name = (char *) (((char*) hdr) + length + hdr_srcname);
      if (check_src (src_name))
	{
	  msgs = (compmsg *) (((char *) hdr) + sizeof (compcomhdr));
	  params = (int32_t *) ((char *) msgs + hdr_msgcount * sizeof (compmsg));
	  strs = (char *) ((char *) params + hdr_paramcount * sizeof (int32_t));

	  // initialize the I18N/L10N strings & set the visible table
	  ccm_vis_init ();
	  return hdr_msgcount;
	}
      offset += (length + hdr_stringlen);
    }
  return 0;
}

char *
CompComment::get_demangle_name (char *fname)
{
  if (*fname == '_')
    return cplus_demangle (fname, DMGL_PARAMS);
  return NULL;
}

/*
 * takes the message, and returns the I18N string for the message.
 */
char *
CompComment::compcom_format (int index, compmsg *msg, int &visible)
{
  compmsg *p = msgs + index;
  msg->instaddr = elf->decode (p->instaddr);
  msg->lineno = elf->decode (p->lineno);
  msg->msg_type = elf->decode (p->msg_type);
  msg->nparam = elf->decode (p->nparam);
  msg->param_index = elf->decode (p->param_index);

  int vindex = ccm_vis_index (msg->msg_type);
  char *mbuf;
  Ccm_Primtype_t prim_ty;
  visible = ccm_attrs[vindex].vis;
  if (ccm_attrs[vindex].msg == NULL)
    {
      /* Print CCM_UNKNOWN message */
      int uindex = ccm_vis_index (CCM_UNKNOWN);
      visible = ccm_attrs[uindex].vis;
      return dbe_sprintf (ccm_attrs[uindex].msg, vindex);
    }

  /*
   * Construct the output buffer based on the primitive types of the
   * message parameters.
   *
   * Parameter lists have to be handled carefully -- the 1 parameter
   * is built up of all the elements separated by ", ".
   *
   * Old way: Case by message format string.
   */
  int *ind = params + msg->param_index;
  int plist_idx = ccm_paramlist_index (msg->msg_type);
  if (plist_idx <= 0)
    {
      /* No parameter list to handle; 0 parameters case is handled */

      enum
      {
	MAX_COMPCOM_ARGS = 13
      };
      char *parms[MAX_COMPCOM_ARGS];
      if (msg->nparam >= MAX_COMPCOM_ARGS)
	{
	  fprintf (stderr,
		   GTXT ("Warning: improperly formatted compiler commentary message (%d parameters >= %d);\n  please report this bug against the compiler\n"),
		   msg->nparam, MAX_COMPCOM_ARGS);
	  return NULL;
	}
      for (int i = 0; i < MAX_COMPCOM_ARGS; i++)
	parms[i] = NULL; // initialize array
      int prm_cnt = ccm_num_params (msg->msg_type);
      if (prm_cnt != msg->nparam)
	{
	  fprintf (stderr,
		   GTXT ("Warning, improperly formatted compiler commentary message (parameter count mismatch = %d, param# = %d, msg_type = %x, `%s');\n  please report this bug against the compiler\n"),
		   prm_cnt, msg->nparam, msg->msg_type, ccm_attrs[vindex].msg);
	  return NULL;
	}
      for (int i = 0; i < msg->nparam; i++)
	{
	  /* Parameters in message-type numbered from '1' */
	  prim_ty = ccm_param_primtype (msg->msg_type, i + 1);
	  if (prim_ty == CCM_PRIMTYPE_INTEGER)
	    {
	      unsigned long v = elf->decode (ind[i]);
	      parms[i] = (char*) v;
	    }
	  else if (prim_ty == CCM_PRIMTYPE_STRING)
	    {
	      char *fname = strs + elf->decode (ind[i]);
	      char *demName = get_demangle_name (fname);
	      parms[i] = demName ? demName : dbe_strdup (fname);
	    }
	  else if (prim_ty == CCM_PRIMTYPE_HEXSTRING)
	    parms[i] = dbe_sprintf (elf_cls == ELFCLASS32 ? NTXT ("0x%08llx") : NTXT ("0x%016llx"),
				    (unsigned long long) msg->instaddr);
	  else
	    {
	      fprintf (stderr,
		       GTXT ("Warning, improperly formatted compiler commentary message (unexpected primitive type %d);\n  please report this bug against the compiler\n"),
		       prim_ty);
	      // Dummy code to avoid compiler's warning: static function ccm_param_hightype is not used
	      Ccm_Hitype_t hightype = CCM_HITYPE_NONE;
	      if (hightype != CCM_HITYPE_NONE)
		hightype = ccm_param_hightype (msg->msg_type, i + 1);
	      return NULL;
	    }
	}

      /*
       * Must make sure to pass _ALL_ params; may pass more because
       * the format won't access the 'extra' parameters if all the
       * rules for messages have been followed.
       */
      mbuf = dbe_sprintf (ccm_attrs[vindex].msg, parms[0], parms[1], parms[2],
			  parms[3], parms[4], parms[5], parms[6], parms[7],
			  parms[8], parms[9], parms[10], parms[11]);
      // Cleanup allocated memory.
      for (int i = 0; i < msg->nparam; i++)
	{
	  prim_ty = ccm_param_primtype (msg->msg_type, i + 1);
	  if (prim_ty == CCM_PRIMTYPE_STRING || prim_ty == CCM_PRIMTYPE_HEXSTRING)
	    free (parms[i]);
	}
    }
  else
    {
      /*
       * Parameter list messages never have 0 parameters; the
       * primitive type for the parameter list elements is always
       * the same.  And as of 22-Sep-2006, it was always
       * CCM_PRIMTYPE_STRING.
       *
       * Account for different bases of parameter indices and
       * 'nparam' count (1 and 0, respectively).
       */
      char *parms[3];
      if (plist_idx > (int) ((sizeof (parms) / sizeof (char*))))
	{
	  fprintf (stderr,
		   GTXT ("Warning: improperly formatted compiler commentary message (msg->nparam=%d plist_idx=%d);\n  please report this bug against the compiler\n"),
		   msg->nparam, plist_idx);
	  return NULL;
	}
      for (size_t i = 0; i < (sizeof (parms) / sizeof (char*)); i++)
	parms[i] = NULL; // initialize array

      StringBuilder sb;
      prim_ty = ccm_param_primtype (msg->msg_type, plist_idx);
      for (int i = plist_idx - 1; i < msg->nparam; i++)
	{
	  if (i != plist_idx - 1)
	    sb.append (GTXT (", "));
	  if (prim_ty == CCM_PRIMTYPE_INTEGER)
	    sb.append (elf->decode (ind[i]));
	  else if (prim_ty == CCM_PRIMTYPE_STRING)
	    {
	      char *fname = strs + elf->decode (ind[i]);
	      char *demName = get_demangle_name (fname);
	      if (demName)
		{
		  sb.append (demName);
		  delete demName;
		}
	      else
		sb.append (fname);
	    }
	  else if (prim_ty == CCM_PRIMTYPE_HEXSTRING)
	    sb.appendf (elf_cls == ELFCLASS32 ? NTXT ("0x%08llx") : NTXT ("0x%016llx"),
			(unsigned long long) msg->instaddr);
	}
      parms[plist_idx - 1] = sb.toString ();

      for (int i = 0; i < plist_idx - 1; i++)
	{
	  prim_ty = ccm_param_primtype (msg->msg_type, i + 1);
	  if (prim_ty == CCM_PRIMTYPE_INTEGER)
	    {
	      unsigned long v = elf->decode (ind[i]);
	      parms[i] = (char*) v;
	    }
	  else if (prim_ty == CCM_PRIMTYPE_STRING)
	    {
	      char *fname = strs + elf->decode (ind[i]);
	      char *demName = get_demangle_name (fname);
	      parms[i] = demName ? demName : dbe_strdup (fname);
	    }
	  else if (prim_ty == CCM_PRIMTYPE_HEXSTRING)
	    parms[i] = dbe_sprintf (elf_cls == ELFCLASS32 ? NTXT ("0x%08llx") : NTXT ("0x%016llx"),
				    (unsigned long long) msg->instaddr);
	  else
	    {
	      fprintf (stderr,
		       GTXT ("Warning, improperly formatted compiler commentary message (unexpected primitive type %d);\n  please report this bug against the compiler\n"),
		       prim_ty);
	      return NULL;
	    }
	}

      /*
       * We have reduced the parameter list to a single string (as
       * the printf format specifier requires), so only have
       * 'plist_idx' parameters.
       */
      mbuf = dbe_sprintf (ccm_attrs[vindex].msg, parms[0], parms[1], parms[2]);

      // Cleanup allocated memory.
      free (parms[plist_idx - 1]);
      for (int i = 0; i < plist_idx - 1; i++)
	{
	  prim_ty = ccm_param_primtype (msg->msg_type, i + 1);
	  if (prim_ty == CCM_PRIMTYPE_STRING)
	    free (parms[i]);
	}
    }
  return mbuf;
}
