/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2010, 2012, 2016, 2018-2023 Free Software Foundation, Inc.
   This file was written by Peter Miller <millerp@canb.auug.org.au>

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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "noreturn.h"
#include "closeout.h"
#include "dir-list.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "message.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "xmalloca.h"
#include "po-charset.h"
#include "msgl-iconv.h"
#include "msgl-fsearch.h"
#include "c-strstr.h"
#include "c-strcase.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Apply the .pot file to each of the domains in the PO file.  */
static bool multi_domain_mode = false;

/* Determines whether to use fuzzy matching.  */
static bool use_fuzzy_matching = true;

/* Whether to consider fuzzy messages as translations.  */
static bool include_fuzzies = false;

/* Whether to consider untranslated messages as translations.  */
static bool include_untranslated = false;

/* Long options.  */
static const struct option long_options[] =
{
  { "directory", required_argument, NULL, 'D' },
  { "help", no_argument, NULL, 'h' },
  { "multi-domain", no_argument, NULL, 'm' },
  { "no-fuzzy-matching", no_argument, NULL, 'N' },
  { "properties-input", no_argument, NULL, 'P' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 1 },
  { "use-fuzzy", no_argument, NULL, CHAR_MAX + 2 },
  { "use-untranslated", no_argument, NULL, CHAR_MAX + 3 },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void compare (const char *fn1, const char *fn2,
                     catalog_input_format_ty input_syntax);


int
main (int argc, char *argv[])
{
  int optchar;
  bool do_help;
  bool do_version;
  catalog_input_format_ty input_syntax = &input_format_po;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;
  gram_max_allowed_errors = UINT_MAX;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  do_help = false;
  do_version = false;
  while ((optchar = getopt_long (argc, argv, "D:hmNPV", long_options, NULL))
         != EOF)
    switch (optchar)
      {
      case '\0':                /* long option */
        break;

      case 'D':
        dir_list_append (optarg);
        break;

      case 'h':
        do_help = true;
        break;

      case 'm':
        multi_domain_mode = true;
        break;

      case 'N':
        use_fuzzy_matching = false;
        break;

      case 'P':
        input_syntax = &input_format_properties;
        break;

      case 'V':
        do_version = true;
        break;

      case CHAR_MAX + 1:        /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 2:        /* --use-fuzzy */
        include_fuzzies = true;
        break;

      case CHAR_MAX + 3:        /* --use-untranslated */
        include_untranslated = true;
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
              "1995-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Peter Miller"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test whether we have an .po file name as argument.  */
  if (optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input files given"));
      usage (EXIT_FAILURE);
    }
  if (optind + 2 != argc)
    {
      error (EXIT_SUCCESS, 0, _("exactly 2 input files required"));
      usage (EXIT_FAILURE);
    }

  /* compare the two files */
  compare (argv[optind], argv[optind + 1], input_syntax);
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
Usage: %s [OPTION] def.po ref.pot\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Compare two Uniforum style .po files to check that both contain the same\n\
set of msgid strings.  The def.po file is an existing PO file with the\n\
translations.  The ref.pot file is the last created PO file, or a PO Template\n\
file (generally created by xgettext).  This is useful for checking that\n\
you have translated each and every message in your program.  Where an exact\n\
match cannot be found, fuzzy matching is used to produce better diagnostics.\n\
"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  def.po                      translations\n"));
      printf (_("\
  ref.pot                     references to the sources\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf ("\n");
      printf (_("\
Operation modifiers:\n"));
      printf (_("\
  -m, --multi-domain          apply ref.pot to each of the domains in def.po\n"));
      printf (_("\
  -N, --no-fuzzy-matching     do not use fuzzy matching\n"));
      printf (_("\
      --use-fuzzy             consider fuzzy entries\n"));
      printf (_("\
      --use-untranslated      consider untranslated entries\n"));
      printf ("\n");
      printf (_("\
Input file syntax:\n"));
      printf (_("\
  -P, --properties-input      input files are in Java .properties syntax\n"));
      printf (_("\
      --stringtable-input     input files are in NeXTstep/GNUstep .strings\n\
                              syntax\n"));
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


/* Return true if a message should be kept.  */
static bool
is_message_selected (const message_ty *mp)
{
  /* Always keep the header entry.  */
  if (is_header (mp))
    return true;

  return !mp->obsolete;
}


/* Remove obsolete messages from a message list.  Return the modified list.  */
static msgdomain_list_ty *
remove_obsoletes (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    message_list_remove_if_not (mdlp->item[k]->messages, is_message_selected);

  return mdlp;
}


static void
match_domain (const char *fn1, const char *fn2,
              message_list_ty *defmlp, message_fuzzy_index_ty **defmlp_findex,
              const char *def_canon_charset,
              message_list_ty *refmlp,
              int *nerrors)
{
  size_t j;

  for (j = 0; j < refmlp->nitems; j++)
    {
      message_ty *refmsg;
      message_ty *defmsg;

      refmsg = refmlp->item[j];

      /* See if it is in the other file.  */
      defmsg = message_list_search (defmlp, refmsg->msgctxt, refmsg->msgid);
      if (defmsg)
        {
          if (!include_untranslated && defmsg->msgstr[0] == '\0')
            {
              (*nerrors)++;
              po_gram_error_at_line (&defmsg->pos,
                                     _("this message is untranslated"));
            }
          else if (!include_fuzzies && defmsg->is_fuzzy && !is_header (defmsg))
            {
              (*nerrors)++;
              po_gram_error_at_line (&defmsg->pos,
                                     _("this message needs to be reviewed by the translator"));
            }
          else
            defmsg->used = 1;
        }
      else
        {
          /* If the message was not defined at all, try to find a very
             similar message, it could be a typo, or the suggestion may
             help.  */
          (*nerrors)++;
          if (use_fuzzy_matching)
            {
              if (false)
                {
                  /* Old, slow code.  */
                  defmsg =
                    message_list_search_fuzzy (defmlp,
                                               refmsg->msgctxt, refmsg->msgid);
                }
              else
                {
                  /* Speedup through early abort in fstrcmp(), combined with
                     pre-sorting of the messages through a hashed index.  */
                  /* Create the fuzzy index lazily.  */
                  if (*defmlp_findex == NULL)
                    *defmlp_findex =
                      message_fuzzy_index_alloc (defmlp, def_canon_charset);
                  defmsg =
                    message_fuzzy_index_search (*defmlp_findex,
                                                refmsg->msgctxt, refmsg->msgid,
                                                FUZZY_THRESHOLD, false);
                }
            }
          else
            defmsg = NULL;
          if (defmsg)
            {
              po_gram_error_at_line (&refmsg->pos,
                                     _("this message is used but not defined..."));
              error_message_count--;
              po_gram_error_at_line (&defmsg->pos,
                                     _("...but this definition is similar"));
              defmsg->used = 1;
            }
          else
            po_gram_error_at_line (&refmsg->pos,
                                   _("this message is used but not defined in %s"),
                                   fn1);
        }
    }
}


static void
compare (const char *fn1, const char *fn2, catalog_input_format_ty input_syntax)
{
  msgdomain_list_ty *def;
  msgdomain_list_ty *ref;
  int nerrors;
  size_t j, k;
  const char *def_canon_charset;
  message_list_ty *empty_list;

  /* This is the master file, created by a human.  */
  def = remove_obsoletes (read_catalog_file (fn1, input_syntax));

  /* This is the generated file, created by groping the sources with
     the xgettext program.  */
  ref = remove_obsoletes (read_catalog_file (fn2, input_syntax));

  /* The references file can be either in ASCII or in UTF-8.  If it is
     in UTF-8, we have to convert the definitions to UTF-8 as well.  */
  {
    bool was_utf8 = false;
    for (k = 0; k < ref->nitems; k++)
      {
        message_list_ty *mlp = ref->item[k]->messages;

        for (j = 0; j < mlp->nitems; j++)
          if (is_header (mlp->item[j]) /* && !mlp->item[j]->obsolete */)
            {
              const char *header = mlp->item[j]->msgstr;

              if (header != NULL)
                {
                  const char *charsetstr = c_strstr (header, "charset=");

                  if (charsetstr != NULL)
                    {
                      size_t len;

                      charsetstr += strlen ("charset=");
                      len = strcspn (charsetstr, " \t\n");
                      if (len == strlen ("UTF-8")
                          && c_strncasecmp (charsetstr, "UTF-8", len) == 0)
                        was_utf8 = true;
                    }
                }
            }
        }
    if (was_utf8)
      def = iconv_msgdomain_list (def, po_charset_utf8, true, fn1);
  }

  /* Determine canonicalized encoding name of the definitions now, after
     conversion.  Only used for fuzzy matching.  */
  if (use_fuzzy_matching)
    {
      def_canon_charset = def->encoding;
      if (def_canon_charset == NULL)
        {
          char *charset = NULL;

          /* Get the encoding of the definitions file.  */
          for (k = 0; k < def->nitems; k++)
            {
              message_list_ty *mlp = def->item[k]->messages;

              for (j = 0; j < mlp->nitems; j++)
                if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
                  {
                    const char *header = mlp->item[j]->msgstr;

                    if (header != NULL)
                      {
                        const char *charsetstr = c_strstr (header, "charset=");

                        if (charsetstr != NULL)
                          {
                            size_t len;

                            charsetstr += strlen ("charset=");
                            len = strcspn (charsetstr, " \t\n");
                            charset = (char *) xmalloca (len + 1);
                            memcpy (charset, charsetstr, len);
                            charset[len] = '\0';
                            break;
                          }
                      }
                  }
              if (charset != NULL)
                break;
            }
          if (charset != NULL)
            def_canon_charset = po_charset_canonicalize (charset);
          if (def_canon_charset == NULL)
            /* Unspecified encoding.  Assume unibyte encoding.  */
            def_canon_charset = po_charset_ascii;
        }
    }
  else
    def_canon_charset = NULL;

  empty_list = message_list_alloc (false);

  /* Every entry in the xgettext generated file must be matched by a
     (single) entry in the human created file.  */
  nerrors = 0;
  if (!multi_domain_mode)
    for (k = 0; k < ref->nitems; k++)
      {
        const char *domain = ref->item[k]->domain;
        message_list_ty *refmlp = ref->item[k]->messages;
        message_list_ty *defmlp;
        message_fuzzy_index_ty *defmlp_findex;

        defmlp = msgdomain_list_sublist (def, domain, false);
        if (defmlp == NULL)
          defmlp = empty_list;

        defmlp_findex = NULL;

        match_domain (fn1, fn2, defmlp, &defmlp_findex, def_canon_charset,
                      refmlp, &nerrors);

        if (defmlp_findex != NULL)
          message_fuzzy_index_free (defmlp_findex);
      }
  else
    {
      /* Apply the references messages in the default domain to each of
         the definition domains.  */
      message_list_ty *refmlp = ref->item[0]->messages;

      for (k = 0; k < def->nitems; k++)
        {
          message_list_ty *defmlp = def->item[k]->messages;

          /* Ignore the default message domain if it has no messages.  */
          if (k > 0 || defmlp->nitems > 0)
            {
              message_fuzzy_index_ty *defmlp_findex = NULL;

              match_domain (fn1, fn2, defmlp, &defmlp_findex, def_canon_charset,
                            refmlp, &nerrors);

              if (defmlp_findex != NULL)
                message_fuzzy_index_free (defmlp_findex);
            }
        }
    }

  /* Look for messages in the definition file, which are not present
     in the reference file, indicating messages which defined but not
     used in the program.  */
  for (k = 0; k < def->nitems; ++k)
    {
      message_list_ty *defmlp = def->item[k]->messages;

      for (j = 0; j < defmlp->nitems; j++)
        {
          message_ty *defmsg = defmlp->item[j];

          if (!defmsg->used)
            po_gram_error_at_line (&defmsg->pos,
                                   _("warning: this message is not used"));
        }
    }

  /* Exit with status 1 on any error.  */
  if (nerrors > 0)
    error (EXIT_FAILURE, 0,
           ngettext ("found %d fatal error", "found %d fatal errors", nerrors),
           nerrors);
}
