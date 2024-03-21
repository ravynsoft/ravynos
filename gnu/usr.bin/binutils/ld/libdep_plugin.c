/* libdeps plugin for the GNU linker.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

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
#if BFD_SUPPORTS_PLUGINS
#include "plugin-api.h"

#include <ctype.h> /* For isspace.  */

extern enum ld_plugin_status onload (struct ld_plugin_tv *tv);

/* Helper for calling plugin api message function.  */
#define TV_MESSAGE if (tv_message) (*tv_message)

/* Function pointers to cache hooks passed at onload time.  */
static ld_plugin_register_claim_file tv_register_claim_file = 0;
static ld_plugin_register_all_symbols_read tv_register_all_symbols_read = 0;
static ld_plugin_register_cleanup tv_register_cleanup = 0;
static ld_plugin_message tv_message = 0;
static ld_plugin_add_input_library tv_add_input_library = 0;
static ld_plugin_set_extra_library_path tv_set_extra_library_path = 0;

/* Handle/record information received in a transfer vector entry.  */
static enum ld_plugin_status
parse_tv_tag (struct ld_plugin_tv *tv)
{
#define SETVAR(x) x = tv->tv_u.x
  switch (tv->tv_tag)
    {
      case LDPT_REGISTER_CLAIM_FILE_HOOK:
	SETVAR(tv_register_claim_file);
	break;
      case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
	SETVAR(tv_register_all_symbols_read);
	break;
      case LDPT_REGISTER_CLEANUP_HOOK:
	SETVAR(tv_register_cleanup);
	break;
      case LDPT_MESSAGE:
	SETVAR(tv_message);
	break;
      case LDPT_ADD_INPUT_LIBRARY:
	SETVAR(tv_add_input_library);
	break;
      case LDPT_SET_EXTRA_LIBRARY_PATH:
	SETVAR(tv_set_extra_library_path);
	break;
      default:
	break;
    }
#undef SETVAR
  return LDPS_OK;
}

/* Defs for archive parsing.  */
#define ARMAGSIZE	8
typedef struct arhdr
{
  char ar_name[16];
  char ar_date[12];
  char ar_uid[6];
  char ar_gid[6];
  char ar_mode[8];
  char ar_size[10];
  char ar_fmag[2];
} arhdr;

typedef struct linerec
{
  struct linerec *next;
  char line[];
} linerec;

#define LIBDEPS "__.LIBDEP/ "

static linerec *line_head, **line_tail = &line_head;

static enum ld_plugin_status
get_libdeps (int fd)
{
  arhdr ah;
  int len;
  unsigned long mlen;
  size_t amt;
  linerec *lr;
  enum ld_plugin_status rc = LDPS_NO_SYMS;

  lseek (fd, ARMAGSIZE, SEEK_SET);
  for (;;)
    {
      len = read (fd, (void *) &ah, sizeof (ah));
      if (len != sizeof (ah))
	break;
      mlen = strtoul (ah.ar_size, NULL, 10);
      if (!mlen || strncmp (ah.ar_name, LIBDEPS, sizeof (LIBDEPS)-1))
	{
	  lseek (fd, mlen, SEEK_CUR);
	  continue;
	}
      amt = mlen + sizeof (linerec);
      if (amt <= mlen)
	return LDPS_ERR;
      lr = malloc (amt);
      if (!lr)
	return LDPS_ERR;
      lr->next = NULL;
      len = read (fd, lr->line, mlen);
      lr->line[mlen-1] = '\0';
      *line_tail = lr;
      line_tail = &lr->next;
      rc = LDPS_OK;
      break;
    }
  return rc;
}

/* Turn a string into an argvec.  */
static char **
str2vec (char *in)
{
  char **res;
  char *s, *first, *end;
  char *sq, *dq;
  int i;

  end = in + strlen (in);
  s = in;
  while (isspace ((unsigned char) *s)) s++;
  first = s;

  i = 1;
  while ((s = strchr (s, ' ')))
    {
      s++;
      i++;
    }
  res = (char **)malloc ((i+1) * sizeof (char *));
  if (!res)
    return res;

  i = 0;
  sq = NULL;
  dq = NULL;
  res[0] = first;
  for (s = first; *s; s++)
    {
      if (*s == '\\')
	{
	  memmove (s, s+1, end-s-1);
	  end--;
	}
      if (isspace ((unsigned char) *s))
	{
	  if (sq || dq)
	    continue;
	  *s++ = '\0';
	  while (isspace ((unsigned char) *s)) s++;
	  if (*s)
	    res[++i] = s;
	}
      if (*s == '\'' && !dq)
	{
	  if (sq)
	    {
	      memmove (sq, sq+1, s-sq-1);
	      memmove (s-2, s+1, end-s-1);
	      end -= 2;
	      s--;
	      sq = NULL;
	    }
	  else
	    {
	      sq = s;
	    }
	}
      if (*s == '"' && !sq)
	{
	  if (dq)
	    {
	      memmove (dq, dq+1, s-dq-1);
	      memmove (s-2, s+1, end-s-1);
	      end -= 2;
	      s--;
	      dq = NULL;
	    }
	  else
	    {
	      dq = s;
	    }
	}
    }
  res[++i] = NULL;
  return res;
}

static char *prevfile;

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
onclaim_file (const struct ld_plugin_input_file *file, int *claimed)
{
  enum ld_plugin_status rv;

  *claimed = 0;

  /* If we've already seen this file, ignore it.  */
  if (prevfile && !strcmp (file->name, prevfile))
    return LDPS_OK;

  /* If it's not an archive member, ignore it.  */
  if (!file->offset)
    return LDPS_OK;

  if (prevfile)
    free (prevfile);

  prevfile = strdup (file->name);
  if (!prevfile)
    return LDPS_ERR;

  /* This hook only gets called on actual object files.
   * We have to examine the archive ourselves, to find
   * our LIBDEPS member.  */
  rv = get_libdeps (file->fd);
  if (rv == LDPS_ERR)
    return rv;

  if (rv == LDPS_OK)
    {
      linerec *lr = (linerec *)line_tail;
      /* Inform the user/testsuite.  */
      TV_MESSAGE (LDPL_INFO, "got deps for library %s: %s",
		  file->name, lr->line);
      fflush (NULL);
    }

  return LDPS_OK;
}

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
onall_symbols_read (void)
{
  linerec *lr;
  char **vec;
  enum ld_plugin_status rv = LDPS_OK;

  while ((lr = line_head))
    {
      line_head = lr->next;
      vec = str2vec (lr->line);
      if (vec)
	{
	  int i;
	  for (i = 0; vec[i]; i++)
	    {
	      if (vec[i][0] != '-')
		{
		  TV_MESSAGE (LDPL_WARNING, "ignoring libdep argument %s",
			      vec[i]);
		  fflush (NULL);
		  continue;
		}
	      if (vec[i][1] == 'l')
		rv = tv_add_input_library (vec[i]+2);
	      else if (vec[i][1] == 'L')
		rv = tv_set_extra_library_path (vec[i]+2);
	      else
		{
		  TV_MESSAGE (LDPL_WARNING, "ignoring libdep argument %s",
			      vec[i]);
		  fflush (NULL);
		}
	      if (rv != LDPS_OK)
		break;
	    }
	  free (vec);
	}
      free (lr);
    }
  line_tail = NULL;
  return rv;
}

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
oncleanup (void)
{
  if (prevfile)
    {
      free (prevfile);
      prevfile = NULL;
    }
  if (line_head)
    {
      linerec *lr;
      while ((lr = line_head))
	{
	  line_head = lr->next;
	  free (lr);
	}
      line_tail = NULL;
    }
  return LDPS_OK;
}

/* Standard plugin API entry point.  */
enum ld_plugin_status
onload (struct ld_plugin_tv *tv)
{
  enum ld_plugin_status rv;

  /* This plugin requires a valid tv array.  */
  if (!tv)
    return LDPS_ERR;

  /* First entry should always be LDPT_MESSAGE, letting us get
     hold of it easily so we can send output straight away.  */
  if (tv[0].tv_tag == LDPT_MESSAGE)
    tv_message = tv[0].tv_u.tv_message;

  do
    if ((rv = parse_tv_tag (tv)) != LDPS_OK)
      return rv;
  while ((tv++)->tv_tag != LDPT_NULL);

  /* Register hooks.  */
  if (tv_register_claim_file
      && tv_register_all_symbols_read
      && tv_register_cleanup)
    {
      (*tv_register_claim_file) (onclaim_file);
      (*tv_register_all_symbols_read) (onall_symbols_read);
      (*tv_register_cleanup) (oncleanup);
    }
  fflush (NULL);
  return LDPS_OK;
}
#endif /* BFD_SUPPORTS_PLUGINS */
