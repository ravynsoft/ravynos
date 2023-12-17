/* Pass translations to a subprocess.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
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
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "noreturn.h"
#include "closeout.h"
#include "dir-list.h"
#include "error.h"
#include "xvasprintf.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "message.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "msgl-charset.h"
#include "xalloc.h"
#include "full-write.h"
#include "findprog.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "xsetenv.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)

#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif


/* Name of the subprogram.  */
static const char *sub_name;

/* Pathname of the subprogram.  */
static const char *sub_path;

/* Argument list for the subprogram.  */
static const char **sub_argv;
static int sub_argc;

static bool newline;

/* Maximum exit code encountered.  */
static int exitcode;

/* Long options.  */
static const struct option long_options[] =
{
  { "directory", required_argument, NULL, 'D' },
  { "help", no_argument, NULL, 'h' },
  { "input", required_argument, NULL, 'i' },
  { "newline", no_argument, NULL, CHAR_MAX + 2 },
  { "properties-input", no_argument, NULL, 'P' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 1 },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void process_msgdomain_list (const msgdomain_list_ty *mdlp);


int
main (int argc, char **argv)
{
  int opt;
  bool do_help;
  bool do_version;
  const char *input_file;
  msgdomain_list_ty *result;
  catalog_input_format_ty input_syntax = &input_format_po;
  size_t i;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;
  input_file = NULL;

  /* The '+' in the options string causes option parsing to terminate when
     the first non-option, i.e. the subprogram name, is encountered.  */
  while ((opt = getopt_long (argc, argv, "+D:hi:PV", long_options, NULL))
         != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;

      case 'D':
        dir_list_append (optarg);
        break;

      case 'h':
        do_help = true;
        break;

      case 'i':
        if (input_file != NULL)
          {
            error (EXIT_SUCCESS, 0, _("at most one input file allowed"));
            usage (EXIT_FAILURE);
          }
        input_file = optarg;
        break;

      case 'P':
        input_syntax = &input_format_properties;
        break;

      case 'V':
        do_version = true;
        break;

      case CHAR_MAX + 1: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 2: /* --newline */
        newline = true;
        break;

      default:
        usage (EXIT_FAILURE);
        break;
      }

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
              "2001-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test for the subprogram name.  */
  if (optind == argc)
    error (EXIT_FAILURE, 0, _("missing command name"));
  sub_name = argv[optind];

  /* Build argument list for the program.  */
  sub_argc = argc - optind;
  sub_argv = XNMALLOC (sub_argc + 1, const char *);
  for (i = 0; i < sub_argc; i++)
    sub_argv[i] = argv[optind + i];
  sub_argv[i] = NULL;

  /* By default, input comes from standard input.  */
  if (input_file == NULL)
    input_file = "-";

  /* Read input file.  */
  result = read_catalog_file (input_file, input_syntax);

  if (strcmp (sub_name, "0") != 0)
    {
      /* Warn if the current locale is not suitable for this PO file.  */
      compare_po_locale_charsets (result);

      /* Block SIGPIPE for this process and for the subprocesses.
         The subprogram may have side effects (additionally to producing some
         output), therefore if there are no readers on stdout, processing of the
         strings must continue nevertheless.  */
      {
        sigset_t sigpipe_set;

        sigemptyset (&sigpipe_set);
        sigaddset (&sigpipe_set, SIGPIPE);
        sigprocmask (SIG_UNBLOCK, &sigpipe_set, NULL);
      }

      /* Attempt to locate the program.
         This is an optimization, to avoid that spawn/exec searches the PATH
         on every call.  */
      sub_path = find_in_path (sub_name);

      /* Finish argument list for the program.  */
      sub_argv[0] = sub_path;
    }

  exitcode = 0; /* = EXIT_SUCCESS */

  /* Apply the subprogram.  */
  process_msgdomain_list (result);

  exit (exitcode);
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
Usage: %s [OPTION] COMMAND [COMMAND-OPTION]\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Applies a command to all translations of a translation catalog.\n\
The COMMAND can be any program that reads a translation from standard\n\
input.  It is invoked once for each translation.  Its output becomes\n\
msgexec's output.  msgexec's return code is the maximum return code\n\
across all invocations.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
A special builtin command called '0' outputs the translation, followed by a\n\
null byte.  The output of \"msgexec 0\" is suitable as input for \"xargs -0\".\n\
"));
      printf ("\n");
      printf (_("\
Command input:\n"));
      printf (_("\
  --newline                   add newline at the end of input\n"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  -i, --input=INPUTFILE       input PO file\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf (_("\
If no input file is given or if it is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Input file syntax:\n"));
      printf (_("\
  -P, --properties-input      input file is in Java .properties syntax\n"));
      printf (_("\
      --stringtable-input     input file is in NeXTstep/GNUstep .strings syntax\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
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


#ifdef EINTR

/* EINTR handling for close().
   These functions can return -1/EINTR even though we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

static inline int
nonintr_close (int fd)
{
  int retval;

  do
    retval = close (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef close
#define close nonintr_close

#endif


/* Pipe a string STR of size LEN bytes to the subprogram.
   The byte after STR is known to be a '\0' byte.  */
static void
process_string (const message_ty *mp, const char *str, size_t len)
{
  if (strcmp (sub_name, "0") == 0)
    {
      /* Built-in command "0".  */
      if (full_write (STDOUT_FILENO, str, len + 1) < len + 1)
        error (EXIT_FAILURE, errno, _("write to stdout failed"));
    }
  else
    {
      /* General command.  */
      char *location;
      pid_t child;
      int fd[1];
      void (*orig_sigpipe_handler)(int);
      int exitstatus;
      char *newstr;

      /* Set environment variables for the subprocess.
         Note: These environment variables, especially MSGEXEC_MSGCTXT and
         MSGEXEC_MSGID, may contain non-ASCII characters.  The subprocess
         may not interpret these values correctly if the locale encoding is
         different from the PO file's encoding.  We warned about this situation,
         above.
         On Unix, this problem is often harmless.  On Windows, however, - both
         native Windows and Cygwin - the values of environment variables *must*
         be in the encoding that is the value of GetACP(), because the system
         may convert the environment from char** to wchar_t** before spawning
         the subprocess and back from wchar_t** to char** in the subprocess,
         and it does so using the GetACP() codepage.  */
      if (mp->msgctxt != NULL)
        xsetenv ("MSGEXEC_MSGCTXT", mp->msgctxt, 1);
      else
        unsetenv ("MSGEXEC_MSGCTXT");
      xsetenv ("MSGEXEC_MSGID", mp->msgid, 1);
      if (mp->msgid_plural != NULL)
        xsetenv ("MSGEXEC_MSGID_PLURAL", mp->msgid_plural, 1);
      else
        unsetenv ("MSGEXEC_MSGID_PLURAL");
      location = xasprintf ("%s:%ld", mp->pos.file_name,
                            (long) mp->pos.line_number);
      xsetenv ("MSGEXEC_LOCATION", location, 1);
      free (location);
      if (mp->prev_msgctxt != NULL)
        xsetenv ("MSGEXEC_PREV_MSGCTXT", mp->prev_msgctxt, 1);
      else
        unsetenv ("MSGEXEC_PREV_MSGCTXT");
      if (mp->prev_msgid != NULL)
        xsetenv ("MSGEXEC_PREV_MSGID", mp->prev_msgid, 1);
      else
        unsetenv ("MSGEXEC_PREV_MSGID");
      if (mp->prev_msgid_plural != NULL)
        xsetenv ("MSGEXEC_PREV_MSGID_PLURAL", mp->prev_msgid_plural, 1);
      else
        unsetenv ("MSGEXEC_PREV_MSGID_PLURAL");

      /* Open a pipe to a subprocess.  */
      child = create_pipe_out (sub_name, sub_path, sub_argv, NULL,
                               NULL, false, true, true, fd);

      /* Ignore SIGPIPE here.  We don't care if the subprocesses terminates
         successfully without having read all of the input that we feed it.  */
      orig_sigpipe_handler = signal (SIGPIPE, SIG_IGN);

      if (newline)
        {
          newstr = XNMALLOC (len + 1, char);
          memcpy (newstr, str, len);
          newstr[len++] = '\n';
        }
      else
        newstr = (char *) str;

      if (full_write (fd[0], newstr, len) < len)
        if (errno != EPIPE)
          error (EXIT_FAILURE, errno,
                 _("write to %s subprocess failed"), sub_name);

      if (newstr != str)
        free (newstr);

      close (fd[0]);

      signal (SIGPIPE, orig_sigpipe_handler);

      /* Remove zombie process from process list, and retrieve exit status.  */
      /* FIXME: Should ignore_sigpipe be set to true here? It depends on the
         semantics of the subprogram...  */
      exitstatus =
        wait_subprocess (child, sub_name, false, false, true, true, NULL);
      if (exitcode < exitstatus)
        exitcode = exitstatus;
    }
}


static void
process_message (const message_ty *mp)
{
  const char *msgstr = mp->msgstr;
  size_t msgstr_len = mp->msgstr_len;
  const char *p;
  size_t k;

  /* Process each NUL delimited substring separately.  */
  for (p = msgstr, k = 0; p < msgstr + msgstr_len; k++)
    {
      size_t length = strlen (p);

      if (mp->msgid_plural != NULL)
        {
          char *plural_form_string = xasprintf ("%lu", (unsigned long) k);

          xsetenv ("MSGEXEC_PLURAL_FORM", plural_form_string, 1);
          free (plural_form_string);
        }
      else
        unsetenv ("MSGEXEC_PLURAL_FORM");
      process_string (mp, p, length);

      p += length + 1;
    }
}


static void
process_message_list (const message_list_ty *mlp)
{
  size_t j;

  for (j = 0; j < mlp->nitems; j++)
    process_message (mlp->item[j]);
}


static void
process_msgdomain_list (const msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    process_message_list (mdlp->item[k]->messages);
}
