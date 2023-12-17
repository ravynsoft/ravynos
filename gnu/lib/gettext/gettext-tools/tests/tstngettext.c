/* ngettext - retrieve plural form strings from message catalog and print them.
   Copyright (C) 1995-1997, 2000-2007, 2012, 2018-2020 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#include "attribute.h"
#include "noreturn.h"
#include "closeout.h"
#include "error.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "propername.h"
#include "xsetenv.h"

/* Make sure we use the included libintl, not the system's one. */
#undef _LIBINTL_H
#include "libgnuintl.h"

#if defined _WIN32 && !defined __CYGWIN__
# undef setlocale
# define setlocale fake_setlocale
extern char *setlocale (int category, const char *locale);
#endif

#define _(str) gettext (str)

/* Long options.  */
static const struct option long_options[] =
{
  { "domain", required_argument, NULL, 'd' },
  { "env", required_argument, NULL, '=' },
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};

/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int __status);

int
main (int argc, char *argv[])
{
  int optchar;
  const char *msgid;
  const char *msgid_plural;
  const char *count;
  unsigned long n;

  /* Default values for command line options.  */
  bool do_help = false;
  bool do_version = false;
  bool environ_changed = false;
  const char *domain = getenv ("TEXTDOMAIN");
  const char *domaindir = getenv ("TEXTDOMAINDIR");

  /* Set program name for message texts.  */
  set_program_name (argv[0]);

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Parse command line options.  */
  while ((optchar = getopt_long (argc, argv, "+d:hV", long_options, NULL))
         != EOF)
    switch (optchar)
    {
    case '\0':          /* Long option.  */
      break;
    case 'd':
      domain = optarg;
      break;
    case 'h':
      do_help = true;
      break;
    case 'V':
      do_version = true;
      break;
    case '=':
      {
        /* Undocumented option --env sets an environment variable.  */
        char *separator = strchr (optarg, '=');
        if (separator != NULL)
          {
            *separator = '\0';
            xsetenv (optarg, separator + 1, 1);
            environ_changed = true;
            break;
          }
      }
      FALLTHROUGH;
    default:
      usage (EXIT_FAILURE);
    }

  if (environ_changed)
    /* Set locale again via LC_ALL.  */
    setlocale (LC_ALL, "");

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "1995-1997, 2000-2006", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Ulrich Drepper"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* More optional command line options.  */
  if (argc - optind <= 2)
    error (EXIT_FAILURE, 0, _("missing arguments"));

  /* Now the mandatory command line options.  */
  msgid = argv[optind++];
  msgid_plural = argv[optind++];

  /* If no domain name is given we print the original string.
     We mark this assigning NULL to domain.  */
  if (domain == NULL || domain[0] == '\0')
    domain = NULL;
  else
    /* Bind domain to appropriate directory.  */
    if (domaindir != NULL && domaindir[0] != '\0')
      bindtextdomain (domain, domaindir);

  /* To speed up the plural-2 test, we accept more than one COUNT in one
     call.  */
  while (optind < argc)
    {
      count = argv[optind++];

      {
        char *endp;
        unsigned long tmp_val;

        errno = 0;
        tmp_val = strtoul (count, &endp, 10);
        if (errno == 0 && count[0] != '\0' && endp[0] == '\0')
          n = tmp_val;
        else
          /* When COUNT is not valid, use plural.  */
          n = 99;
      }

      /* If no domain name is given we don't translate, and we use English
         plural form handling.  */
      if (domain == NULL)
        fputs (n == 1 ? msgid : msgid_plural, stdout);
      else
        /* Write out the result.  */
        fputs (dngettext (domain, msgid, msgid_plural, n), stdout);
    }

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      /* xgettext: no-wrap */
      printf (_("\
Usage: %s [OPTION] MSGID MSGID-PLURAL COUNT...\n\
  -d, --domain=TEXTDOMAIN   retrieve translated message from TEXTDOMAIN\n\
  -h, --help                display this help and exit\n\
  -V, --version             display version information and exit\n\
  MSGID MSGID-PLURAL        translate MSGID (singular) / MSGID-PLURAL (plural)\n\
  COUNT                     choose singular/plural form based on this value\n"),
              program_name);
      /* xgettext: no-wrap */
      printf (_("\
\n\
If the TEXTDOMAIN parameter is not given, the domain is determined from the\n\
environment variable TEXTDOMAIN.  If the message catalog is not found in the\n\
regular directory, another location can be specified with the environment\n\
variable TEXTDOMAINDIR.\n\
Standard search directory: %s\n"), LOCALEDIR);
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf(_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
             "https://savannah.gnu.org/projects/gettext",
             "bug-gettext@gnu.org");
    }

  exit (status);
}
