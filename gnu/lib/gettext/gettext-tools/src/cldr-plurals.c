/* Unicode CLDR plural rule parser and converter
   Copyright (C) 2015, 2018-2023 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

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

#include "basename-lgpl.h"
#include "cldr-plural-exp.h"
#include "closeout.h"
#include "c-ctype.h"
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include "gettext.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <locale.h>
#include "progname.h"
#include "propername.h"
#include "relocatable.h"
#include <stdlib.h>
#include <string.h>
#include "xalloc.h"

#define _(s) gettext(s)


static char *
extract_rules (FILE *fp,
               const char *real_filename, const char *logical_filename,
               const char *locale)
{
  xmlDocPtr doc;
  xmlNodePtr node, n;
  size_t locale_length;
  char *buffer = NULL, *p;
  size_t bufmax = 0;
  size_t buflen = 0;

  doc = xmlReadFd (fileno (fp), logical_filename, NULL,
                   XML_PARSE_NONET
                   | XML_PARSE_NOWARNING
                   | XML_PARSE_NOBLANKS);
  if (doc == NULL)
    error (EXIT_FAILURE, 0, _("Could not parse file %s as XML"), logical_filename);

  node = xmlDocGetRootElement (doc);
  if (!node || !xmlStrEqual (node->name, BAD_CAST "supplementalData"))
    {
      error_at_line (0, 0,
                     logical_filename,
                     xmlGetLineNo (node),
                     _("The root element must be <%s>"),
                     "supplementalData");
      goto out;
    }

  for (n = node->children; n; n = n->next)
    {
      if (n->type == XML_ELEMENT_NODE
          && xmlStrEqual (n->name, BAD_CAST "plurals"))
        break;
    }
  if (!n)
    {
      error (0, 0, _("The element <%s> does not contain a <%s> element"),
             "supplementalData", "plurals");
      goto out;
    }

  locale_length = strlen (locale);
  for (n = n->children; n; n = n->next)
    {
      xmlChar *locales;
      xmlChar *cp;
      xmlNodePtr n2;
      bool found = false;

      if (n->type != XML_ELEMENT_NODE
          || !xmlStrEqual (n->name, BAD_CAST "pluralRules"))
        continue;

      if (!xmlHasProp (n, BAD_CAST "locales"))
        {
          error_at_line (0, 0,
                         logical_filename,
                         xmlGetLineNo (n),
                         _("The element <%s> does not have attribute <%s>"),
                         "pluralRules", "locales");
          continue;
        }

      cp = locales = xmlGetProp (n, BAD_CAST "locales");
      while (*cp != '\0')
        {
          while (c_isspace (*cp))
            cp++;
          if (xmlStrncmp (cp, BAD_CAST locale, locale_length) == 0
              && (*(cp + locale_length) == '\0'
                  || c_isspace (*(cp + locale_length))))
            {
              found = true;
              break;
            }
          while (*cp && !c_isspace (*cp))
            cp++;
        }
      xmlFree (locales);

      if (!found)
        continue;

      for (n2 = n->children; n2; n2 = n2->next)
        {
          xmlChar *count;
          xmlChar *content;
          size_t length;

          if (n2->type != XML_ELEMENT_NODE
              || !xmlStrEqual (n2->name, BAD_CAST "pluralRule"))
            continue;

          if (!xmlHasProp (n2, BAD_CAST "count"))
            {
              error_at_line (0, 0,
                             logical_filename,
                             xmlGetLineNo (n2),
                             _("The element <%s> does not have attribute <%s>"),
                             "pluralRule", "count");
              break;
            }

          count = xmlGetProp (n2, BAD_CAST "count");
          content = xmlNodeGetContent (n2);
          length = xmlStrlen (count) + strlen (": ")
            + xmlStrlen (content) + strlen ("; ");

          if (buflen + length + 1 > bufmax)
            {
              bufmax *= 2;
              if (bufmax < buflen + length + 1)
                bufmax = buflen + length + 1;
              buffer = (char *) xrealloc (buffer, bufmax);
            }

          sprintf (buffer + buflen, "%s: %s; ", count, content);
          xmlFree (count);
          xmlFree (content);

          buflen += length;
        }
    }

  if (buffer)
    {
      /* Scrub the last semicolon, if any.  */
      p = strrchr (buffer, ';');
      if (p)
        *p = '\0';
    }

 out:
  xmlFreeDoc (doc);
  return buffer;
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
Usage: %s [OPTION...] [LOCALE RULES]...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Extract or convert Unicode CLDR plural rules.\n\
\n\
If both LOCALE and RULES are specified, it reads CLDR plural rules for\n\
LOCALE from RULES and print them in a form suitable for gettext use.\n\
If no argument is given, it reads CLDR plural rules from the standard input.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n\
Similarly for optional arguments.\n\
"));
      printf ("\n");
      printf (_("\
  -c, --cldr                  print plural rules in the CLDR format\n"));
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

/* Long options.  */
static const struct option long_options[] =
{
  { "cldr", no_argument, NULL, 'c' },
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};

int
main (int argc, char **argv)
{
  bool opt_cldr_format = false;
  bool do_help = false;
  bool do_version = false;
  int optchar;

  /* Set program name for messages.  */
  set_program_name (argv[0]);

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  while ((optchar = getopt_long (argc, argv, "chV", long_options, NULL)) != EOF)
    switch (optchar)
      {
      case '\0':                /* Long option.  */
        break;

      case 'c':
        opt_cldr_format = true;
        break;

      case 'h':
        do_help = true;
        break;

      case 'V':
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
              "2015-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Daiki Ueno"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  if (argc == optind + 2)
    {
      /* Two arguments: Read CLDR rules from a file.  */
      const char *locale = argv[optind];
      const char *logical_filename = argv[optind + 1];
      char *extracted_rules;
      FILE *fp;

      LIBXML_TEST_VERSION

      fp = fopen (logical_filename, "r");
      if (fp == NULL)
        error (1, 0, _("%s cannot be read"), logical_filename);

      extracted_rules = extract_rules (fp, logical_filename, logical_filename,
                                       locale);
      fclose (fp);
      if (extracted_rules == NULL)
        error (1, 0, _("cannot extract rules for %s"), locale);

      if (opt_cldr_format)
        printf ("%s\n", extracted_rules);
      else
        {
          struct cldr_plural_rule_list_ty *result;

          result = cldr_plural_parse (extracted_rules);
          if (result == NULL)
            error (1, 0, _("cannot parse CLDR rule"));

          cldr_plural_rule_list_print (result, stdout);
          cldr_plural_rule_list_free (result);
        }
      free (extracted_rules);
    }
  else if (argc == optind)
    {
      /* No argument: Read CLDR rules from standard input.  */
      char *line = NULL;
      size_t line_size = 0;
      for (;;)
        {
          int line_len;
          struct cldr_plural_rule_list_ty *result;

          line_len = getline (&line, &line_size, stdin);
          if (line_len < 0)
            break;
          if (line_len > 0 && line[line_len - 1] == '\n')
            line[--line_len] = '\0';

          result = cldr_plural_parse (line);
          if (result)
            {
              cldr_plural_rule_list_print (result, stdout);
              cldr_plural_rule_list_free (result);
            }
        }

      free (line);
    }
  else
    {
      error (1, 0, _("extra operand %s"), argv[optind]);
    }

  return 0;
}
