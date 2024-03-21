/* Get the contents of an URL.
   Copyright (C) 2001-2003, 2005-2010, 2012, 2017-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
# include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#include "noreturn.h"
#include "closeout.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "full-write.h"
#include "execute.h"
#include "javaexec.h"
#include "binary-io.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)

#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif


/* Only high-level toolkits, written in languages with exception handling,
   have an URL datatype and operations to fetch an URL's contents.  Such
   toolkits are Java (class java.net.URL), Qt (classes QUrl and QUrlOperator).
   We use the Java toolkit.
   Note that this program doesn't handle redirection pages; programs which
   wish to process HTML redirection tags need to include a HTML parser,
   and only full-fledged browsers like w3m, lynx, links have have both
   an URL fetcher (which covers at least the protocols "http", "ftp", "file")
   and a HTML parser.  [Well, this is not true: libxml2 and Java (see
   <http://java.sun.com/products/jfc/tsc/articles/bookmarks/>) also contain
   HTML parsers.]  */


/* Whether to output something on standard error.
   This is true by default, because the user should know why we are trying to
   establish an internet connection.  Also, users get confused if a program
   produces no output for more than 10 seconds for no apparent reason.  */
static bool verbose = true;

/* Long options.  */
static const struct option long_options[] =
{
  { "help", no_argument, NULL, 'h' },
  { "quiet", no_argument, NULL, 'q' },
  { "silent", no_argument, NULL, 'q' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void fetch (const char *url, const char *file);


int
main (int argc, char *argv[])
{
  int optchar;
  bool do_help;
  bool do_version;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;

  /* Parse command line options.  */
  while ((optchar = getopt_long (argc, argv, "hqV", long_options, NULL)) != EOF)
    switch (optchar)
    {
    case '\0':          /* Long option.  */
      break;
    case 'h':           /* --help */
      do_help = true;
      break;
    case 'q':           /* --quiet / --silent */
      verbose = false;
      break;
    case 'V':           /* --version */
      do_version = true;
      break;
    default:
      usage (EXIT_FAILURE);
      /* NOTREACHED */
    }

  /* Version information requested.  */
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
              "2001-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test argument count.  */
  if (optind + 2 != argc)
    error (EXIT_FAILURE, 0, _("expected two arguments"));

  /* Fetch the contents.  */
  fetch (argv[optind], argv[optind + 1]);

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
      printf (_("\
Usage: %s [OPTION] URL FILE\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Fetches and outputs the contents of an URL.  If the URL cannot be accessed,\n\
the locally accessible FILE is used instead.\n\
"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
      printf (_("\
  -q, --quiet, --silent       suppress progress indicators\n"));
      printf ("\n");
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

/* Copy a file's contents to stdout.  */
static void
cat_file (const char *src_filename)
{
  int src_fd;
  char buf[4096];
  const int buf_size = sizeof (buf);

  src_fd = open (src_filename, O_RDONLY | O_BINARY);
  if (src_fd < 0)
    error (EXIT_FAILURE, errno, _("error while opening \"%s\" for reading"),
           src_filename);

  for (;;)
    {
      ssize_t n_read = read (src_fd, buf, buf_size);
      if (n_read < 0)
        {
#ifdef EINTR
          if (errno == EINTR)
            continue;
#endif
          error (EXIT_FAILURE, errno, _("error reading \"%s\""), src_filename);
        }
      if (n_read == 0)
        break;

      if (full_write (STDOUT_FILENO, buf, n_read) < n_read)
        error (EXIT_FAILURE, errno, _("error writing stdout"));
    }

  if (close (src_fd) < 0)
    error (EXIT_FAILURE, errno, _("error after reading \"%s\""), src_filename);
}

#if USEJAVA

/* Exit code of the Java program.  */
static int java_exitcode;

static bool
execute_it (const char *progname,
            const char *prog_path, const char * const *prog_argv,
            void *private_data)
{
  (void) private_data;

  java_exitcode =
    execute (progname, prog_path, prog_argv, NULL,
             true, true, false, false, true, false, NULL);
  /* Exit code 0 means success, 2 means timed out.  */
  return !(java_exitcode == 0 || java_exitcode == 2);
}

#endif

/* Fetch the URL.  Upon error, use the FILE as fallback.  */
static void
fetch (const char *url, const char *file)
{
  if (verbose)
    {
      fprintf (stderr, _("Retrieving %s..."), url);
      fflush (stderr);
    }

#if USEJAVA
  /* First try: using Java.  */
  {
    const char *class_name = "gnu.gettext.GetURL";
    const char *gettextjar;
    const char *args[2];

    /* Make it possible to override the gettext.jar location.  This is
       necessary for running the testsuite before "make install".  */
    gettextjar = getenv ("GETTEXTJAR");
    if (gettextjar == NULL || gettextjar[0] == '\0')
      gettextjar = relocate (GETTEXTJAR);

    /* Prepare arguments.  */
    args[0] = url;
    args[1] = NULL;

    /* Fetch the URL's contents.  */
    java_exitcode = 127;
    if (!execute_java_class (class_name, &gettextjar, 1, true, NULL,
                             args,
                             false, true,
                             execute_it, NULL))
      {
        if (verbose)
          {
            if (java_exitcode == 0)
              fprintf (stderr, _(" done.\n"));
            else if (java_exitcode == 2)
              fprintf (stderr, _(" timed out.\n"));
          }
        return;
      }
  }
#endif

  /* Second try: using "wget -q -O - -T 30 url".  */
  {
    static bool wget_tested;
    static bool wget_present;

    if (!wget_tested)
      {
        /* Test for presence of wget: "wget --version > /dev/null"  */
        const char *argv[3];
        int exitstatus;

        argv[0] = "wget";
        argv[1] = "--version";
        argv[2] = NULL;
        exitstatus = execute ("wget", "wget", argv, NULL,
                              false, false, true, true, true, false, NULL);
        wget_present = (exitstatus == 0);
        wget_tested = true;
      }

    if (wget_present)
      {
        const char *argv[10];
        int exitstatus;

        argv[0] = "wget";
        argv[1] = "--quiet";
        argv[2] = "--output-document"; argv[3] = "-";
        argv[4] = "--timeout"; argv[5] = "30";
        argv[6] = "--user-agent"; argv[7] = "urlget";
        argv[8] = url;
        argv[9] = NULL;
        exitstatus = execute ("wget", "wget", argv, NULL,
                              true, false, false, false, true, false, NULL);
        if (exitstatus != 127)
          {
            if (exitstatus != 0)
              goto failed;
            if (verbose)
              fprintf (stderr, _(" done.\n"));
            return;
          }
      }
  }

  /* Third try: using "lynx -source url".  */
  {
    static bool lynx_tested;
    static bool lynx_present;

    if (!lynx_tested)
      {
        /* Test for presence of lynx: "lynx --version > /dev/null"  */
        const char *argv[3];
        int exitstatus;

        argv[0] = "lynx";
        argv[1] = "--version";
        argv[2] = NULL;
        exitstatus = execute ("lynx", "lynx", argv, NULL,
                              false, false, true, true, true, false, NULL);
        lynx_present = (exitstatus == 0);
        lynx_tested = true;
      }

    if (lynx_present)
      {
        const char *argv[5];
        int exitstatus;

        argv[0] = "lynx";
        argv[1] = "-useragent=urlget";
        argv[2] = "-source";
        argv[3] = url;
        argv[4] = NULL;
        exitstatus = execute ("lynx", "lynx", argv, NULL,
                              true, false, false, false, true, false, NULL);
        if (exitstatus != 127)
          {
            if (exitstatus != 0)
              goto failed;
            if (verbose)
              fprintf (stderr, _(" done.\n"));
            return;
          }
      }
  }

  /* Fourth try: using "curl --silent url".  */
  {
    static bool curl_tested;
    static bool curl_present;

    if (!curl_tested)
      {
        /* Test for presence of curl: "curl --version > /dev/null"  */
        const char *argv[3];
        int exitstatus;

        argv[0] = "curl";
        argv[1] = "--version";
        argv[2] = NULL;
        exitstatus = execute ("curl", "curl", argv, NULL,
                              false, false, true, true, true, false, NULL);
        curl_present = (exitstatus == 0 || exitstatus == 2);
        curl_tested = true;
      }

    if (curl_present)
      {
        const char *argv[6];
        int exitstatus;

        argv[0] = "curl";
        argv[1] = "--silent";
        argv[2] = "--user-agent"; argv[3] = "urlget";
        argv[4] = url;
        argv[5] = NULL;
        exitstatus = execute ("curl", "curl", argv, NULL,
                              true, false, false, false, true, false, NULL);
        if (exitstatus != 127)
          {
            if (exitstatus != 0)
              goto failed;
            if (verbose)
              fprintf (stderr, _(" done.\n"));
            return;
          }
      }
  }

 failed:
  if (verbose)
    fprintf (stderr, _(" failed.\n"));
  /* Use the file as fallback.  */
  cat_file (file);
}
