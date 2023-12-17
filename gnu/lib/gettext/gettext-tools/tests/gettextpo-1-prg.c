/* Test of public API for GNU gettext PO files.
   Copyright (C) 2010, 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2010.

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

#include "gettext-po.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use the system functions, not the gnulib overrides in this file.  */
#undef fflush
#undef fprintf
#undef free
#undef printf
#undef strdup

#define ASSERT(expr) \
  do                                                                         \
    {                                                                        \
      if (!(expr))                                                           \
        {                                                                    \
          fprintf (stderr, "%s:%d: assertion failed\n",                      \
                   __FILE__, __LINE__);                                      \
          fflush (stderr);                                                   \
          abort ();                                                          \
        }                                                                    \
    }                                                                        \
  while (0)

static char *
xstrdup (const char *s)
{
  char *result = strdup (s);
  if (result == NULL)
    {
      fprintf (stderr, "memory exhausted\n");
      fflush (stderr);
      exit (1);
    }
  return result;
}

static int num_errors;

static void
my_xerror (int severity,
           po_message_t message,
           const char *filename, size_t lineno, size_t column,
           int multiline_p, const char *message_text)
{
  printf ("xerror called:\n  %s\n", message_text);
  if (severity == PO_SEVERITY_FATAL_ERROR)
    abort ();
  num_errors++;
}

  /* Signal a problem that refers to two messages.
     Similar to two calls to xerror.
     If possible, a "..." can be appended to MESSAGE_TEXT1 and prepended to
     MESSAGE_TEXT2.  */
static void
my_xerror2 (int severity,
            po_message_t message1,
            const char *filename1, size_t lineno1, size_t column1,
            int multiline_p1, const char *message_text1,
            po_message_t message2,
            const char *filename2, size_t lineno2, size_t column2,
            int multiline_p2, const char *message_text2)
{
  printf ("xerror2 called:\n  %s\n  %s\n", message_text1, message_text2);
  if (severity == PO_SEVERITY_FATAL_ERROR)
    abort ();
  num_errors++;
}

static const struct po_xerror_handler my_xerror_handler =
  {
    my_xerror,
    my_xerror2
  };

int
main (int argc, char *argv[])
{
  const char *input_filename;

  ASSERT (argc == 2);
  input_filename = argv[1];

  /* Test LIBGETTEXTPO_VERSION.  */
  {
    enum { version = LIBGETTEXTPO_VERSION };
  }

  /* Test libgettextpo_version.  */
  ASSERT (libgettextpo_version == LIBGETTEXTPO_VERSION);

  /* Test po_file_read.  */
  {
    po_file_t file = po_file_read ("/nonexist/ent", &my_xerror_handler);
    ASSERT (file == NULL);
  }

  {
    po_file_t file = po_file_read (input_filename, &my_xerror_handler);
    ASSERT (file != NULL);

    /* Test po_file_domains.  */
    {
      const char * const * domains = po_file_domains (file);
      ASSERT (domains[0] != NULL);
      ASSERT (strcmp (domains[0], "messages") == 0);
      ASSERT (domains[1] == NULL);
    }

    /* Test po_file_write.  */
    ASSERT (po_file_write (file, "gtpo-1-copied.po", &my_xerror_handler)
            == file);

    /* Test po_file_domain_header.  */
    {
      static const char expected[] =
        "Project-Id-Version: libgettextpo 0.18.1\n"
        "Report-Msgid-Bugs-To: bug-gnu-gettext@gnu.org\n"
        "POT-Creation-Date: 2010-06-04 01:57+0200\n"
        "PO-Revision-Date: 2010-06-05 14:39+0200\n"
        "Last-Translator: Bruno Haible <bruno@clisp.org>\n"
        "Language-Team: German <translation-team-de@lists.sourceforge.net>\n"
        "Language: de\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain; charset=UTF-8\n"
        "Content-Transfer-Encoding: 8bit\n"
        "Plural-Forms: nplurals=2; plural=(n != 1);\n";
      const char *header;

      header = po_file_domain_header (file, NULL);
      ASSERT (header != NULL);
      ASSERT (strcmp (header, expected) == 0);

      header = po_file_domain_header (file, "messages");
      ASSERT (header != NULL);
      ASSERT (strcmp (header, expected) == 0);

      header = po_file_domain_header (file, "anything");
      ASSERT (header == NULL);

      /* Test po_header_field.  */
      {
        char *value;

        header = po_file_domain_header (file, NULL);

        value = po_header_field (header, "Report-Msgid-Bugs-To");
        ASSERT (value != NULL);
        ASSERT (strcmp (value, "bug-gnu-gettext@gnu.org") == 0);

        value = po_header_field (header, "X-Generator");
        ASSERT (value == NULL);
      }

      /* Test po_header_set_field.  */
      {
        char *augmented_header;
        const char *augmented_expected;

        header = po_file_domain_header (file, NULL);
        augmented_header =
          po_header_set_field (header, "Last-Translator",
                               "Karl Eichwalder <ke@suse.de>");
        augmented_expected =
          "Project-Id-Version: libgettextpo 0.18.1\n"
          "Report-Msgid-Bugs-To: bug-gnu-gettext@gnu.org\n"
          "POT-Creation-Date: 2010-06-04 01:57+0200\n"
          "PO-Revision-Date: 2010-06-05 14:39+0200\n"
          "Last-Translator: Karl Eichwalder <ke@suse.de>\n"
          "Language-Team: German <translation-team-de@lists.sourceforge.net>\n"
          "Language: de\n"
          "MIME-Version: 1.0\n"
          "Content-Type: text/plain; charset=UTF-8\n"
          "Content-Transfer-Encoding: 8bit\n"
          "Plural-Forms: nplurals=2; plural=(n != 1);\n";
        ASSERT (strcmp (augmented_header, augmented_expected) == 0);
        free (augmented_header);

        /* Verify that there was no side effect.  */
        ASSERT (strcmp (header, expected) == 0);
        ASSERT (strcmp (po_file_domain_header (file, NULL), expected) == 0);

        header = po_file_domain_header (file, NULL);
        augmented_header =
          po_header_set_field (header, "X-Generator", "KBabel 1.11.4");
        augmented_expected =
          "Project-Id-Version: libgettextpo 0.18.1\n"
          "Report-Msgid-Bugs-To: bug-gnu-gettext@gnu.org\n"
          "POT-Creation-Date: 2010-06-04 01:57+0200\n"
          "PO-Revision-Date: 2010-06-05 14:39+0200\n"
          "Last-Translator: Bruno Haible <bruno@clisp.org>\n"
          "Language-Team: German <translation-team-de@lists.sourceforge.net>\n"
          "Language: de\n"
          "MIME-Version: 1.0\n"
          "Content-Type: text/plain; charset=UTF-8\n"
          "Content-Transfer-Encoding: 8bit\n"
          "Plural-Forms: nplurals=2; plural=(n != 1);\n"
          "X-Generator: KBabel 1.11.4\n";
        ASSERT (strcmp (augmented_header, augmented_expected) == 0);
        free (augmented_header);

        /* Verify that there was no side effect.  */
        ASSERT (strcmp (header, expected) == 0);
        ASSERT (strcmp (po_file_domain_header (file, NULL), expected) == 0);
      }
    }

    /* Test po_message_iterator.  */
    {
      po_message_iterator_t iter = po_message_iterator (file, NULL);
      int min;
      int max;

      /* Test po_next_message and the po_message_* accessors.  */
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (po_message_msgctxt (msg) == NULL);
        ASSERT (strcmp (po_message_msgid (msg), "") == 0);
        ASSERT (po_message_msgid_plural (msg) == NULL);
        ASSERT (strcmp (po_message_msgstr (msg),
                        po_file_domain_header (file, NULL)) == 0);
        ASSERT (po_message_msgstr_plural (msg, 0) == NULL);
        ASSERT (strcmp (po_message_comments (msg),
                        "Test case for the libgettextpo library.\n") == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg), "") == 0);
        ASSERT (po_message_filepos (msg, 0) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (!po_message_is_obsolete (msg));
        ASSERT (!po_message_is_fuzzy (msg));
        ASSERT (!po_message_is_format (msg, "c-format"));
        ASSERT (!po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (po_message_msgctxt (msg) == NULL);
        ASSERT (strcmp (po_message_msgid (msg),
                        "cannot restore fd %d: dup2 failed") == 0);
        ASSERT (po_message_msgid_plural (msg) == NULL);
        ASSERT (strcmp (po_message_msgstr (msg),
                        "Ausgabedatei \302\273%s\302\253 kann nicht erstellt werden")
                == 0);
        ASSERT (po_message_msgstr_plural (msg, 0) == NULL);
        ASSERT (strcmp (po_message_comments (msg), "") == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg), "") == 0);
        {
          po_filepos_t pos = po_message_filepos (msg, 0);
          ASSERT (pos != NULL);
          ASSERT (strcmp (po_filepos_file (pos), "gnulib-lib/w32spawn.h") == 0);
          ASSERT (po_filepos_start_line (pos) == 81);
        }
        ASSERT (po_message_filepos (msg, 1) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (!po_message_is_obsolete (msg));
        ASSERT (po_message_is_fuzzy (msg));
        ASSERT (po_message_is_format (msg, "c-format"));
        ASSERT (!po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (po_message_msgctxt (msg) == NULL);
        ASSERT (strcmp (po_message_msgid (msg), "%s subprocess") == 0);
        ASSERT (po_message_msgid_plural (msg) == NULL);
        ASSERT (strcmp (po_message_msgstr (msg), "Subproze\303\237 %s") == 0);
        ASSERT (po_message_msgstr_plural (msg, 0) == NULL);
        ASSERT (strcmp (po_message_comments (msg), "") == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg), "") == 0);
        {
          po_filepos_t pos = po_message_filepos (msg, 0);
          ASSERT (pos != NULL);
          ASSERT (strcmp (po_filepos_file (pos), "gnulib-lib/wait-process.c")
                  == 0);
          ASSERT (po_filepos_start_line (pos) == 223);
        }
        {
          po_filepos_t pos = po_message_filepos (msg, 1);
          ASSERT (pos != NULL);
          ASSERT (strcmp (po_filepos_file (pos), "gnulib-lib/wait-process.c")
                  == 0);
          ASSERT (po_filepos_start_line (pos) == 255);
        }
        {
          po_filepos_t pos = po_message_filepos (msg, 2);
          ASSERT (pos != NULL);
          ASSERT (strcmp (po_filepos_file (pos), "gnulib-lib/wait-process.c")
                  == 0);
          ASSERT (po_filepos_start_line (pos) == 317);
        }
        ASSERT (po_message_filepos (msg, 3) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (!po_message_is_obsolete (msg));
        ASSERT (!po_message_is_fuzzy (msg));
        ASSERT (po_message_is_format (msg, "c-format"));
        ASSERT (!po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (strcmp (po_message_msgctxt (msg), "Lock state") == 0);
        ASSERT (strcmp (po_message_msgid (msg), "Open") == 0);
        ASSERT (po_message_msgid_plural (msg) == NULL);
        ASSERT (strcmp (po_message_msgstr (msg), "Ge\303\266ffnet") == 0);
        ASSERT (po_message_msgstr_plural (msg, 0) == NULL);
        ASSERT (strcmp (po_message_comments (msg),
                        "Adjektiv, kein ganzer Satz!\n") == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg),
                        "Denote a lock's state\n") == 0);
        ASSERT (po_message_filepos (msg, 0) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (!po_message_is_obsolete (msg));
        ASSERT (!po_message_is_fuzzy (msg));
        ASSERT (!po_message_is_format (msg, "c-format"));
        ASSERT (!po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (po_message_msgctxt (msg) == NULL);
        ASSERT (strcmp (po_message_msgid (msg), "a bottle of wine") == 0);
        ASSERT (strcmp (po_message_msgid_plural (msg),
                        "{0,number} bottles of wine") == 0);
        ASSERT (strcmp (po_message_msgstr (msg), "eine Flasche Wein") == 0);
        ASSERT (strcmp (po_message_msgstr_plural (msg, 0),
                        "eine Flasche Wein") == 0);
        ASSERT (strcmp (po_message_msgstr_plural (msg, 1),
                        "{0,number} Weinflaschen") == 0);
        ASSERT (po_message_msgstr_plural (msg, 2) == NULL);
        ASSERT (po_message_msgstr_plural (msg, 100000000) == NULL);
        ASSERT (po_message_msgstr_plural (msg, -1) == NULL);
        ASSERT (strcmp (po_message_comments (msg),
                        "Franz\303\266sische Weine sind die besten der Welt.\n")
                == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg), "") == 0);
        ASSERT (po_message_filepos (msg, 0) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (!po_message_is_obsolete (msg));
        ASSERT (!po_message_is_fuzzy (msg));
        ASSERT (!po_message_is_format (msg, "c-format"));
        ASSERT (po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (strcmp (po_message_msgctxt (msg), "Lock state") == 0);
        ASSERT (strcmp (po_message_msgid (msg), "Closed") == 0);
        ASSERT (po_message_msgid_plural (msg) == NULL);
        ASSERT (strcmp (po_message_msgstr (msg), "Geschlossen") == 0);
        ASSERT (po_message_msgstr_plural (msg, 0) == NULL);
        ASSERT (strcmp (po_message_comments (msg), "") == 0);
        ASSERT (strcmp (po_message_extracted_comments (msg),
                        "Denote a lock's state\n") == 0);
        ASSERT (po_message_filepos (msg, 0) == NULL);
        ASSERT (po_message_prev_msgctxt (msg) == NULL);
        ASSERT (po_message_prev_msgid (msg) == NULL);
        ASSERT (po_message_prev_msgid_plural (msg) == NULL);
        ASSERT (po_message_is_obsolete (msg));
        ASSERT (!po_message_is_fuzzy (msg));
        ASSERT (!po_message_is_format (msg, "c-format"));
        ASSERT (!po_message_is_format (msg, "java-format"));
        ASSERT (!po_message_is_range (msg, &min, &max));
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg == NULL);
      }

      /* Test po_message_iterator_free.  */
      po_message_iterator_free (iter);
    }

    /* Test po_file_check_all.  */
    num_errors = 0;
    po_file_check_all (file, &my_xerror_handler);
    ASSERT (num_errors == 0);

    /* Test po_file_free.  */
    po_file_free (file);
  }

  /* Test po_file_create.  */
  {
    po_file_t file = po_file_create ();

    {
      po_message_iterator_t iter = po_message_iterator (file, NULL);

      /* Test po_message_insert, po_message_create, and the po_message_set_*
         setters.  Check that the string arguments are copied.  */
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          static const char header[] =
            "Project-Id-Version: libgettextpo 0.18.1\n"
            "Report-Msgid-Bugs-To: bug-gnu-gettext@gnu.org\n"
            "POT-Creation-Date: 2010-06-04 01:57+0200\n"
            "PO-Revision-Date: 2010-06-05 14:39+0200\n"
            "Last-Translator: Bruno Haible <bruno@clisp.org>\n"
            "Language-Team: German <translation-team-de@lists.sourceforge.net>\n"
            "Language: de\n"
            "MIME-Version: 1.0\n"
            "Content-Type: text/plain; charset=UTF-8\n"
            "Content-Transfer-Encoding: 8bit\n"
            "Plural-Forms: nplurals=2; plural=(n != 1);\n";
          char *arg = xstrdup (header);
          po_message_set_msgstr (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Test case for the libgettextpo library.\n");
          po_message_set_comments (msg, arg);
          free (arg);
        }
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("cannot restore fd %d: dup2 failed");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          char *arg =
            xstrdup ("Ausgabedatei \302\273%s\302\253 kann nicht erstellt werden");
          po_message_set_msgstr (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("gnulib-lib/w32spawn.h");
          po_message_add_filepos (msg, arg, 81);
          free (arg);
        }
        po_message_set_fuzzy (msg, 1);
        po_message_set_format (msg, "c-format", 1);
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("%s subprocess");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Subproze\303\237 %s");
          po_message_set_msgstr (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("gnulib-lib/wait-process.c");
          po_message_add_filepos (msg, arg, 223);
          free (arg);
        }
        {
          char *arg = xstrdup ("gnulib-lib/wait-process.c");
          po_message_add_filepos (msg, arg, 255);
          free (arg);
        }
        {
          char *arg = xstrdup ("gnulib-lib/wait-process.c");
          po_message_add_filepos (msg, arg, 317);
          free (arg);
        }
        po_message_set_format (msg, "c-format", 1);
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("Lock state");
          po_message_set_msgctxt (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Open");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Ge\303\266ffnet");
          po_message_set_msgstr (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Adjektiv, kein ganzer Satz!\n");
          po_message_set_comments (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Denote a lock's state\n");
          po_message_set_extracted_comments (msg, arg);
          free (arg);
        }
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("a bottle of wine");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("{0,number} bottles of wine");
          po_message_set_msgid_plural (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("eine Flasche Wein");
          po_message_set_msgstr_plural (msg, 0, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("{0,number} Weinflaschen");
          po_message_set_msgstr_plural (msg, 1, arg);
          free (arg);
        }
        {
          char *arg =
            xstrdup ("Franz\303\266sische Weine sind die besten der Welt.\n");
          po_message_set_comments (msg, arg);
          free (arg);
        }
        po_message_set_format (msg, "java-format", 1);
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        {
          char *arg = xstrdup ("Lock state");
          po_message_set_msgctxt (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Closed");
          po_message_set_msgid (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Geschlossen");
          po_message_set_msgstr (msg, arg);
          free (arg);
        }
        {
          char *arg = xstrdup ("Denote a lock's state\n");
          po_message_set_extracted_comments (msg, arg);
          free (arg);
        }
        po_message_set_obsolete (msg, 1);
        po_message_insert (iter, msg);
      }

      po_message_iterator_free (iter);
    }

    /* Test po_file_write.  */
    ASSERT (po_file_write (file, "gtpo-1-created.po", &my_xerror_handler)
            == file);
  }

  /* Test po_next_message after po_message_insert.  */
  {
    po_file_t file = po_file_create ();
    {
      po_message_iterator_t iter = po_message_iterator (file, NULL);
      {
        po_message_t msg = po_message_create ();
        po_message_set_msgid (msg, "");
        po_message_insert (iter, msg);
      }
      {
        po_message_t msg = po_message_create ();
        po_message_set_msgid (msg, "Closed");
        po_message_insert (iter, msg);
      }
      po_message_iterator_free (iter);
    }
    {
      po_message_iterator_t iter = po_message_iterator (file, NULL);
      po_next_message (iter);
      {
        po_message_t msg = po_message_create ();
        po_message_set_msgid (msg, "Open");
        po_message_insert (iter, msg);
      }
      po_message_iterator_free (iter);
    }
    {
      po_message_iterator_t iter = po_message_iterator (file, NULL);
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (strcmp (po_message_msgid (msg), "") == 0);
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (strcmp (po_message_msgid (msg), "Open") == 0);
      }
      {
        po_message_t msg = po_next_message (iter);
        ASSERT (msg != NULL);
        ASSERT (strcmp (po_message_msgid (msg), "Closed") == 0);
      }
      ASSERT (po_next_message (iter) == NULL);
      po_message_iterator_free (iter);
    }
    po_file_free (file);
  }

  /* Test po_message_set_msgctxt.  */
  {
    po_message_t msg = po_message_create ();
    po_message_set_msgctxt (msg, "Menu");
    ASSERT (strcmp (po_message_msgctxt (msg), "Menu") == 0);
    po_message_set_msgctxt (msg, "Demo");
    ASSERT (strcmp (po_message_msgctxt (msg), "Demo") == 0);
    po_message_set_msgctxt (msg, NULL);
    ASSERT (po_message_msgctxt (msg) == NULL);
  }

  /* Test po_message_set_msgid.  */
  {
    po_message_t msg = po_message_create ();
    po_message_set_msgid (msg, "operation failed");
    ASSERT (strcmp (po_message_msgid (msg), "operation failed") == 0);
    po_message_set_msgid (msg, "operation succeeded");
    ASSERT (strcmp (po_message_msgid (msg), "operation succeeded") == 0);
  }

  /* Test po_message_set_msgid_plural.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (po_message_msgid_plural (msg) == NULL);
    po_message_set_msgid (msg, "an error");
    ASSERT (strcmp (po_message_msgid (msg), "an error") == 0);
    ASSERT (po_message_msgid_plural (msg) == NULL);
    po_message_set_msgid_plural (msg, "%u errors");
    ASSERT (strcmp (po_message_msgid (msg), "an error") == 0);
    ASSERT (strcmp (po_message_msgid_plural (msg), "%u errors") == 0);
    po_message_set_msgid_plural (msg, NULL);
    ASSERT (strcmp (po_message_msgid (msg), "an error") == 0);
    ASSERT (po_message_msgid_plural (msg) == NULL);
  }

  /* Test po_message_set_msgstr.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (strcmp (po_message_msgstr (msg), "") == 0);
    po_message_set_msgid (msg, "an error");
    ASSERT (strcmp (po_message_msgstr (msg), "") == 0);
    po_message_set_msgstr (msg, "une erreur");
    ASSERT (strcmp (po_message_msgstr (msg), "une erreur") == 0);
    po_message_set_msgstr (msg, "catastrophe");
    ASSERT (strcmp (po_message_msgstr (msg), "catastrophe") == 0);
  }

  /* Test po_message_set_msgstr_plural.  */
  {
    po_message_t msg = po_message_create ();
    po_message_set_msgid (msg, "an error");
    po_message_set_msgid_plural (msg, "%u errors");
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "") == 0);
    ASSERT (po_message_msgstr_plural (msg, 1) == NULL);
    po_message_set_msgstr_plural (msg, 1, "%u erreurs");
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 1), "%u erreurs") == 0);
    ASSERT (po_message_msgstr_plural (msg, 2) == NULL);
    po_message_set_msgstr_plural (msg, 0, "une erreur");
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "une erreur") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 1), "%u erreurs") == 0);
    ASSERT (po_message_msgstr_plural (msg, 2) == NULL);
    po_message_set_msgstr_plural (msg, 1, "des erreurs");
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "une erreur") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 1), "des erreurs") == 0);
    ASSERT (po_message_msgstr_plural (msg, 2) == NULL);
    po_message_set_msgstr_plural (msg, 2, "beaucoup d'erreurs");
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "une erreur") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 1), "des erreurs") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 2), "beaucoup d'erreurs")
            == 0);
    ASSERT (po_message_msgstr_plural (msg, 3) == NULL);
    po_message_set_msgstr_plural (msg, 2, NULL);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 0), "une erreur") == 0);
    ASSERT (strcmp (po_message_msgstr_plural (msg, 1), "des erreurs") == 0);
    ASSERT (po_message_msgstr_plural (msg, 2) == NULL);
    ASSERT (po_message_msgstr_plural (msg, 3) == NULL);
  }

  /* Test po_message_set_comments.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (strcmp (po_message_comments (msg), "") == 0);
    po_message_set_comments (msg, "Not clear.");
    ASSERT (strcmp (po_message_comments (msg), "Not clear.\n") == 0);
    po_message_set_comments (msg, "To be reviewed.\n");
    ASSERT (strcmp (po_message_comments (msg), "To be reviewed.\n") == 0);
  }

  /* Test po_message_set_extracted_comments.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (strcmp (po_message_extracted_comments (msg), "") == 0);
    po_message_set_extracted_comments (msg, "Translate carefully.");
    ASSERT (strcmp (po_message_extracted_comments (msg),
                    "Translate carefully.\n") == 0);
    po_message_set_extracted_comments (msg, "Translate very\ncarefully!\n");
    ASSERT (strcmp (po_message_extracted_comments (msg),
                    "Translate very\ncarefully!\n") == 0);
  }

  /* Test po_message_add_filepos, po_message_remove_filepos.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (po_message_filepos (msg, 0) == NULL);
    po_message_remove_filepos (msg, 2);
    ASSERT (po_message_filepos (msg, 0) == NULL);
    {
      char *arg = xstrdup ("hello.c");
      po_message_add_filepos (msg, arg, 81);
      free (arg);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 0);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 81);
    }
    ASSERT (po_message_filepos (msg, 1) == NULL);
    /* Adding the same filepos once again has no effect.  */
    {
      char *arg = xstrdup ("hello.c");
      po_message_add_filepos (msg, arg, 81);
      free (arg);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 0);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 81);
    }
    ASSERT (po_message_filepos (msg, 1) == NULL);
    {
      char *arg = xstrdup ("hello.c");
      po_message_add_filepos (msg, arg, 1024);
      free (arg);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 0);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 81);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 1);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 1024);
    }
    ASSERT (po_message_filepos (msg, 2) == NULL);
    {
      char *arg = xstrdup ("../src/bar.c");
      po_message_add_filepos (msg, arg, 17);
      free (arg);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 0);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 81);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 1);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 1024);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 2);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "../src/bar.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 17);
    }
    ASSERT (po_message_filepos (msg, 3) == NULL);
    po_message_remove_filepos (msg, 1);
    {
      po_filepos_t pos = po_message_filepos (msg, 0);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "hello.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 81);
    }
    {
      po_filepos_t pos = po_message_filepos (msg, 1);
      ASSERT (pos != NULL);
      ASSERT (strcmp (po_filepos_file (pos), "../src/bar.c") == 0);
      ASSERT (po_filepos_start_line (pos) == 17);
    }
    ASSERT (po_message_filepos (msg, 2) == NULL);
    ASSERT (po_message_filepos (msg, 3) == NULL);
  }

  /* Test po_message_set_prev_msgctxt.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (po_message_prev_msgctxt (msg) == NULL);
    po_message_set_prev_msgctxt (msg, "Menu");
    ASSERT (strcmp (po_message_prev_msgctxt (msg), "Menu") == 0);
    po_message_set_prev_msgctxt (msg, "Demo");
    ASSERT (strcmp (po_message_prev_msgctxt (msg), "Demo") == 0);
    po_message_set_prev_msgctxt (msg, NULL);
    ASSERT (po_message_prev_msgctxt (msg) == NULL);
  }

  /* Test po_message_set_prev_msgid.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (po_message_prev_msgid (msg) == NULL);
    po_message_set_prev_msgid (msg, "operation failed");
    ASSERT (strcmp (po_message_prev_msgid (msg), "operation failed") == 0);
    po_message_set_prev_msgid (msg, "operation succeeded");
    ASSERT (strcmp (po_message_prev_msgid (msg), "operation succeeded") == 0);
  }

  /* Test po_message_set_prev_msgid_plural.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (po_message_prev_msgid_plural (msg) == NULL);
    po_message_set_prev_msgid (msg, "an error");
    ASSERT (strcmp (po_message_prev_msgid (msg), "an error") == 0);
    ASSERT (po_message_prev_msgid_plural (msg) == NULL);
    po_message_set_prev_msgid_plural (msg, "%u errors");
    ASSERT (strcmp (po_message_prev_msgid (msg), "an error") == 0);
    ASSERT (strcmp (po_message_prev_msgid_plural (msg), "%u errors") == 0);
    po_message_set_prev_msgid_plural (msg, NULL);
    ASSERT (strcmp (po_message_prev_msgid (msg), "an error") == 0);
    ASSERT (po_message_prev_msgid_plural (msg) == NULL);
  }

  /* Test po_message_set_obsolete.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (!po_message_is_obsolete (msg));
    po_message_set_obsolete (msg, 1);
    ASSERT (po_message_is_obsolete (msg));
    po_message_set_obsolete (msg, 1);
    ASSERT (po_message_is_obsolete (msg));
    po_message_set_obsolete (msg, 0);
    ASSERT (!po_message_is_obsolete (msg));
  }

  /* Test po_message_set_fuzzy.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (!po_message_is_fuzzy (msg));
    po_message_set_fuzzy (msg, 1);
    ASSERT (po_message_is_fuzzy (msg));
    po_message_set_fuzzy (msg, 1);
    ASSERT (po_message_is_fuzzy (msg));
    po_message_set_fuzzy (msg, 0);
    ASSERT (!po_message_is_fuzzy (msg));
  }

  /* Test po_message_set_format.  */
  {
    po_message_t msg = po_message_create ();
    ASSERT (!po_message_is_format (msg, "c-format"));
    ASSERT (!po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
    po_message_set_format (msg, "c-format", 1);
    ASSERT (po_message_is_format (msg, "c-format"));
    ASSERT (!po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
    po_message_set_format (msg, "c-format", 1);
    ASSERT (po_message_is_format (msg, "c-format"));
    ASSERT (!po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
    po_message_set_format (msg, "java-format", 1);
    ASSERT (po_message_is_format (msg, "c-format"));
    ASSERT (po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
    po_message_set_format (msg, "c-format", 0);
    ASSERT (!po_message_is_format (msg, "c-format"));
    ASSERT (po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
    po_message_set_format (msg, "xyzzy-format", 1);
    ASSERT (!po_message_is_format (msg, "c-format"));
    ASSERT (po_message_is_format (msg, "java-format"));
    ASSERT (!po_message_is_format (msg, "xyzzy-format"));
  }

  /* Test po_message_set_range.  */
  {
    po_message_t msg = po_message_create ();
    int min;
    int max;
    ASSERT (!po_message_is_range (msg, &min, &max));
    po_message_set_range (msg, 1, 100);
    ASSERT (po_message_is_range (msg, &min, &max));
    ASSERT (min == 1);
    ASSERT (max == 100);
    po_message_set_range (msg, 5, 1000);
    ASSERT (po_message_is_range (msg, &min, &max));
    ASSERT (min == 5);
    ASSERT (max == 1000);
    po_message_set_range (msg, -1, -1);
    ASSERT (!po_message_is_range (msg, &min, &max));
  }

  /* Test po_message_check_all.  */
  {
    po_file_t file = po_file_create ();
    po_message_iterator_t iter = po_message_iterator (file, NULL);
    {
      po_message_t msg = po_message_create ();
      po_message_set_msgid (msg, "cannot write %s");
      num_errors = 0;
      po_message_check_all (msg, iter, &my_xerror_handler);
      ASSERT (num_errors == 0);
    }
    {
      po_message_t msg = po_message_create ();
      po_message_set_msgid (msg, "an error");
      po_message_set_msgid_plural (msg, "%u errors\n");
      num_errors = 0;
      po_message_check_all (msg, iter, &my_xerror_handler);
      ASSERT (num_errors == 0);
      po_message_set_msgstr (msg, "Fehler");
      num_errors = 0;
      po_message_check_all (msg, iter, &my_xerror_handler);
      ASSERT (num_errors == 2);
    }
    {
      po_message_t msg = po_message_create ();
      po_message_set_msgid (msg, "");
      po_message_set_msgstr (msg,
        "Content-Type: text/plain; charset=CHARSET\n"
        "Content-Transfer-Encoding: 8bit\n");
      num_errors = 0;
      po_message_check_all (msg, iter, &my_xerror_handler);
      ASSERT (num_errors == 7);
    }
    {
      po_message_t msg = po_message_create ();
      po_message_set_msgid (msg, "encountered %d errors");
      po_message_set_format (msg, "c-format", 1);
      po_message_set_msgstr (msg, "rencontr\303\251 %ld erreurs");
      num_errors = 0;
      po_message_check_all (msg, iter, &my_xerror_handler);
      ASSERT (num_errors == 1);
    }
    po_message_iterator_free (iter);
    po_file_free (file);
  }

  /* Test po_message_check_format.  */
  {
    po_message_t msg = po_message_create ();
    po_message_set_msgid (msg, "encountered %d errors");
    po_message_set_format (msg, "c-format", 1);
    po_message_set_msgstr (msg, "rencontr\303\251 %ld erreurs");
    num_errors = 0;
    po_message_check_format (msg, &my_xerror_handler);
    ASSERT (num_errors == 1);
  }

  /* Test po_format_list.  */
  {
    const char * const *format_types = po_format_list ();

    ASSERT (strcmp (format_types[0], "c-format") == 0);

    while (*format_types != NULL)
      format_types++;
  }

  /* Test po_format_pretty_name.  */
  ASSERT (strcmp (po_format_pretty_name ("c-format"), "C") == 0);
  ASSERT (strcmp (po_format_pretty_name ("csharp-format"), "C#") == 0);
  ASSERT (po_format_pretty_name ("xyzzy-format") == NULL);

  return 0;
}
