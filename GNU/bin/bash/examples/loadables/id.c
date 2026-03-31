/*
 * id - POSIX.2 user identity
 *
 * (INCOMPLETE -- supplementary groups for other users not yet done)
 *
 * usage: id [-Ggu] [-nr] [user]
 *
 * The default output format looks something like:
 *	uid=xxx(chet) gid=xx groups=aa(aname), bb(bname), cc(cname)
 */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <stdio.h>
#include "bashtypes.h"
#include <pwd.h>
#include <grp.h>
#include "bashansi.h"

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#else
#  include <sys/param.h>
#endif

#if !defined (HAVE_GETPW_DECLS)
extern struct passwd *getpwuid ();
#endif
extern struct group *getgrgid ();

#include "shell.h"
#include "builtins.h"
#include "stdc.h"
#include "common.h"
#include "bashgetopt.h"

#define ID_ALLGROUPS	0x001		/* -G */
#define ID_GIDONLY	0x002		/* -g */
#define ID_USENAME	0x004		/* -n */
#define ID_USEREAL	0x008		/* -r */
#define ID_USERONLY	0x010		/* -u */

#define ID_FLAGSET(s)	((id_flags & (s)) != 0)

static int id_flags;

static uid_t ruid, euid;
static gid_t rgid, egid;

static char *id_user;

static int inituser ();

static int id_pruser ();
static int id_prgrp ();
static int id_prgroups ();
static int id_prall ();

int
id_builtin (list)
     WORD_LIST *list;
{
  int opt;
  char *user;

  id_flags = 0;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "Ggnru")) != -1)
    {
      switch (opt)
	{
	case 'G': id_flags |= ID_ALLGROUPS; break;
	case 'g': id_flags |= ID_GIDONLY; break;
	case 'n': id_flags |= ID_USENAME; break;
	case 'r': id_flags |= ID_USEREAL; break;
	case 'u': id_flags |= ID_USERONLY; break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  user = list ? list->word->word : (char *)NULL;

  /* Check for some invalid option combinations */
  opt = ID_FLAGSET (ID_ALLGROUPS) + ID_FLAGSET (ID_GIDONLY) + ID_FLAGSET (ID_USERONLY);
  if (opt > 1 || (opt == 0 && ((id_flags & (ID_USEREAL|ID_USENAME)) != 0)))
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (list && list->next)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (inituser (user) < 0)
    return (EXECUTION_FAILURE);

  opt = 0;
  if (id_flags & ID_USERONLY)
    opt += id_pruser ((id_flags & ID_USEREAL) ? ruid : euid);
  else if (id_flags & ID_GIDONLY)
    opt += id_prgrp ((id_flags & ID_USEREAL) ? rgid : egid);
  else if (id_flags & ID_ALLGROUPS)
    opt += id_prgroups (user);
  else
    opt += id_prall (user);
  putchar ('\n');
  fflush (stdout);

  return (opt == 0 ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

static int
inituser (uname)
     char *uname;
{
  struct passwd *pwd;

  if (uname)
    {
      pwd = getpwnam (uname);
      if (pwd == 0)
	{
	  builtin_error ("%s: no such user", uname);
	  return -1;
	}
      ruid = euid = pwd->pw_uid;
      rgid = egid = pwd->pw_gid;
    }
  else
    {
      ruid = current_user.uid;
      euid = current_user.euid;
      rgid = current_user.gid;
      egid = current_user.egid;
    }
  return 0;
}

/* Print the name or value of user ID UID. */
static int
id_pruser (uid)
     int uid;
{
  struct passwd *pwd = NULL;
  int r;

  r = 0;
  if (id_flags & ID_USENAME)
    {
      pwd = getpwuid (uid);
      if (pwd == NULL)
        r = 1;
    }
  if (pwd)
    printf ("%s", pwd->pw_name);
  else
    printf ("%u", (unsigned) uid);
      
  return r;
}

/* Print the name or value of group ID GID. */

static int
id_prgrp (gid)
     int gid;
{
  struct group *grp = NULL;
  int r;

  r = 0;
  if (id_flags & ID_USENAME)
    {
      grp = getgrgid (gid);
      if (grp == NULL)
	r = 1;
    }

  if (grp)
    printf ("%s", grp->gr_name);
  else
    printf ("%u", (unsigned) gid);

  return r;
}

static int
id_prgroups (uname)
     char *uname;
{
  int *glist, ng, i, r;

  r = 0;
  id_prgrp (rgid);
  if (egid != rgid)
    {
      putchar (' ');
      id_prgrp (egid);
    }

  if (uname)
    {
      builtin_error ("supplementary groups for other users not yet implemented");
      glist = (int *)NULL;
      ng = 0;
      r = 1;
    }
  else
    glist = get_group_array (&ng);

  for (i = 0; i < ng; i++)
    if (glist[i] != rgid && glist[i] != egid)
      {
	putchar (' ');
	id_prgrp (glist[i]);
      }
  
  return r;
}

static int
id_prall (uname)
     char *uname;
{
  int r, i, ng, *glist;
  struct passwd *pwd;
  struct group *grp;

  r = 0;
  printf ("uid=%u", (unsigned) ruid);
  pwd = getpwuid (ruid);
  if (pwd == NULL)
    r = 1;
  else
    printf ("(%s)", pwd->pw_name);

  printf (" gid=%u", (unsigned) rgid);
  grp = getgrgid (rgid);
  if (grp == NULL)
    r = 1;
  else
    printf ("(%s)", grp->gr_name);

  if (euid != ruid)
    { 
      printf (" euid=%u", (unsigned) euid);
      pwd = getpwuid (euid);
      if (pwd == NULL)
	r = 1;
      else 
	printf ("(%s)", pwd->pw_name);
    }

  if (egid != rgid) 
    {
      printf (" egid=%u", (unsigned) egid);
      grp = getgrgid (egid);
      if (grp == NULL)
	r = 1;
      else
	printf ("(%s)", grp->gr_name);
    }

  if (uname)
    {
      builtin_error ("supplementary groups for other users not yet implemented");
      glist = (int *)NULL;
      ng = 0;
      r = 1;
    }
  else
    glist = get_group_array (&ng);

  if (ng > 0)
    printf (" groups=");
  for (i = 0; i < ng; i++)
    {
      if (i > 0)
	printf (", ");
      printf ("%u", (unsigned) glist[i]);
      grp = getgrgid (glist[i]);
      if (grp == NULL)
	r = 1;
      else
	printf ("(%s)", grp->gr_name);
    }

  return r;
}

char *id_doc[] = {
	"Display information about user."
	"",
	"Return information about user identity",
	(char *)NULL
};

struct builtin id_struct = {
	"id",
	id_builtin,
	BUILTIN_ENABLED,
	id_doc,
	"id [user]\n\tid -G [-n] [user]\n\tid -g [-nr] [user]\n\tid -u [-nr] [user]",
	0
};
