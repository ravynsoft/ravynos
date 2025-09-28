/* Test of exact or abbreviated match search.
   Copyright (C) 1990, 1998-1999, 2001-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2007, based on test code
   by David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include <config.h>

/* As of GCC 11.2.1, gcc -Wanalyzer-too-complex reports that main's
   use of CHECK macros expands to code that is too complicated for gcc
   -fanalyzer.  Suppress the resulting bogus warnings.  */
#if 10 <= __GNUC__
# pragma GCC diagnostic ignored "-Wanalyzer-null-argument"
#endif

#include "argmatch.h"

#include <stdlib.h>

#include "macros.h"

# define N_(Msgid) (Msgid)

/* Some packages define ARGMATCH_DIE and ARGMATCH_DIE_DECL in <config.h>, and
   thus must link with a definition of that function.  Provide it here.  */
#ifdef ARGMATCH_DIE_DECL

_Noreturn ARGMATCH_DIE_DECL;
ARGMATCH_DIE_DECL { exit (1); }

#endif

enum backup_type
{
  no_backups,
  simple_backups,
  numbered_existing_backups,
  numbered_backups
};

static const char *const backup_args[] =
{
  "no", "none", "off",
  "simple", "never", "single",
  "existing", "nil", "numbered-existing",
  "numbered", "t", "newstyle",
  NULL
};

static const enum backup_type backup_vals[] =
{
  no_backups, no_backups, no_backups,
  simple_backups, simple_backups, simple_backups,
  numbered_existing_backups, numbered_existing_backups, numbered_existing_backups,
  numbered_backups, numbered_backups, numbered_backups
};

ARGMATCH_DEFINE_GROUP(backup, enum backup_type)

static const argmatch_backup_doc argmatch_backup_docs[] =
{
  { "no",       N_("never make backups (even if --backup is given)") },
  { "numbered", N_("make numbered backups") },
  { "existing", N_("numbered if numbered backups exist, simple otherwise") },
  { "simple",   N_("always make simple backups") },
  { NULL, NULL }
};

static const argmatch_backup_arg argmatch_backup_args[] =
{
  { "no",                no_backups },
  { "none",              no_backups },
  { "off",               no_backups },
  { "simple",            simple_backups },
  { "never",             simple_backups },
  { "single",            simple_backups },
  { "existing",          numbered_existing_backups },
  { "nil",               numbered_existing_backups },
  { "numbered-existing", numbered_existing_backups },
  { "numbered",          numbered_backups },
  { "t",                 numbered_backups },
  { "newstyle",          numbered_backups },
  { NULL, no_backups }
};

const argmatch_backup_group_type argmatch_backup_group =
{
  argmatch_backup_args,
  argmatch_backup_docs,
  N_("\
The backup suffix is '~', unless set with --suffix or SIMPLE_BACKUP_SUFFIX.\n\
The version control method may be selected via the --backup option or through\n\
the VERSION_CONTROL environment variable.  Here are the values:\n"),
  NULL
};

int
main (int argc, char *argv[])
{
#define CHECK(Input, Output)                                            \
  do {                                                                  \
    ASSERT (ARGMATCH (Input, backup_args, backup_vals) == Output);      \
    ASSERT (argmatch_backup_choice (Input) == Output);                  \
    if (0 <= Output)                                                    \
      {                                                                 \
        enum backup_type val                                            \
          = argmatch_backup_args[Output < 0 ? 0 : Output].val;          \
        ASSERT (*argmatch_backup_value ("test", Input) == val);         \
        ASSERT (*argmatch_backup_value ("test",                         \
                                        argmatch_backup_argument (&val)) \
                == val);                                                \
      }                                                                 \
  } while (0)

#define CHECK_EXACT(Input, Output)                                      \
  do {                                                                  \
    ASSERT (ARGMATCH_EXACT (Input, backup_args) == Output);             \
  } while (0)

  /* Not found.  */
  CHECK ("klingon", -1);
  CHECK_EXACT ("klingon", -1);

  /* Exact match.  */
  CHECK ("none", 1);
  CHECK_EXACT ("none", 1);
  CHECK ("nil", 7);
  CHECK_EXACT ("nil", 7);

  /* Too long.  */
  CHECK ("nilpotent", -1);
  CHECK_EXACT ("nilpotent", -1);

  /* Abbreviated.  */
  CHECK ("simpl", 3);
  CHECK_EXACT ("simpl", -1);
  CHECK ("simp", 3);
  CHECK_EXACT ("simp", -1);
  CHECK ("sim", 3);
  CHECK_EXACT ("sim", -1);

  /* Exact match and abbreviated.  */
  CHECK ("numbered", 9);
  CHECK_EXACT ("numbered", 9);
  CHECK ("numbere", -2);
  CHECK_EXACT ("numbere", -1);
  CHECK ("number", -2);
  CHECK_EXACT ("number", -1);
  CHECK ("numbe", -2);
  CHECK_EXACT ("numbe", -1);
  CHECK ("numb", -2);
  CHECK_EXACT ("numb", -1);
  CHECK ("num", -2);
  CHECK_EXACT ("num", -1);
  CHECK ("nu", -2);
  CHECK_EXACT ("nu", -1);
  CHECK ("n", -2);
  CHECK_EXACT ("n", -1);

  /* Ambiguous abbreviated.  */
  CHECK ("ne", -2);
  CHECK_EXACT ("ne", -1);

  /* Ambiguous abbreviated, but same value ("single" and "simple").  */
  CHECK ("si", 3);
  CHECK_EXACT ("si", -1);
  CHECK ("s", 3);
  CHECK_EXACT ("s", -1);

#undef CHECK
#undef CHECK_EXACT

  argmatch_backup_usage (stdout);

  return 0;
}
