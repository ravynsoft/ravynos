/* Writing C# satellite assemblies.
   Copyright (C) 2003-2010, 2016, 2018-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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
#include <alloca.h>

/* Specification.  */
#include "write-csharp.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#if !defined S_ISDIR && defined S_IFDIR
# define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#if !S_IRUSR && S_IREAD
# define S_IRUSR S_IREAD
#endif
#if !S_IRUSR
# define S_IRUSR 00400
#endif
#if !S_IWUSR && S_IWRITE
# define S_IWUSR S_IWRITE
#endif
#if !S_IWUSR
# define S_IWUSR 00200
#endif
#if !S_IXUSR && S_IEXEC
# define S_IXUSR S_IEXEC
#endif
#if !S_IXUSR
# define S_IXUSR 00100
#endif
#if !S_IRGRP
# define S_IRGRP (S_IRUSR >> 3)
#endif
#if !S_IWGRP
# define S_IWGRP (S_IWUSR >> 3)
#endif
#if !S_IXGRP
# define S_IXGRP (S_IXUSR >> 3)
#endif
#if !S_IROTH
# define S_IROTH (S_IRUSR >> 6)
#endif
#if !S_IWOTH
# define S_IWOTH (S_IWUSR >> 6)
#endif
#if !S_IXOTH
# define S_IXOTH (S_IXUSR >> 6)
#endif

#include "attribute.h"
#include "c-ctype.h"
#include "relocatable.h"
#include "error.h"
#include "xerror.h"
#include "csharpcomp.h"
#include "message.h"
#include "msgfmt.h"
#include "msgl-iconv.h"
#include "msgl-header.h"
#include "plural-exp.h"
#include "po-charset.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "concat-filename.h"
#include "fwriteerror.h"
#include "clean-temp.h"
#include "unistr.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Convert a resource name to a class name.
   Return a nonempty string consisting of alphanumerics and underscores
   and starting with a letter or underscore.  */
static char *
construct_class_name (const char *resource_name)
{
  /* This code must be kept consistent with intl.cs, function
     GettextResourceManager.ConstructClassName.  */
  /* We could just return an arbitrary fixed class name, like "Messages",
     assuming that every assembly will only ever contain one
     GettextResourceSet subclass, but this assumption would break the day
     we want to support multi-domain PO files in the same format...  */
  bool valid;
  const char *p;

  /* Test for a valid ASCII identifier:
     - nonempty,
     - first character is A..Za..z_ - see x-csharp.c:is_identifier_start.
     - next characters are A..Za..z_0..9 - see x-csharp.c:is_identifier_part.
   */
  valid = (resource_name[0] != '\0');
  for (p = resource_name; valid && *p != '\0'; p++)
    {
      char c = *p;
      if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_')
            || (p > resource_name && c >= '0' && c <= '9')))
        valid = false;
    }
  if (valid)
    return xstrdup (resource_name);
  else
    {
      static const char hexdigit[] = "0123456789abcdef";
      const char *str = resource_name;
      const char *str_limit = str + strlen (str);
      char *class_name = XNMALLOC (12 + 6 * (str_limit - str) + 1, char);
      char *b;

      b = class_name;
      memcpy (b, "__UESCAPED__", 12); b += 12;
      while (str < str_limit)
        {
          ucs4_t uc;
          str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
          if (uc >= 0x10000)
            {
              *b++ = '_';
              *b++ = 'U';
              *b++ = hexdigit[(uc >> 28) & 0x0f];
              *b++ = hexdigit[(uc >> 24) & 0x0f];
              *b++ = hexdigit[(uc >> 20) & 0x0f];
              *b++ = hexdigit[(uc >> 16) & 0x0f];
              *b++ = hexdigit[(uc >> 12) & 0x0f];
              *b++ = hexdigit[(uc >> 8) & 0x0f];
              *b++ = hexdigit[(uc >> 4) & 0x0f];
              *b++ = hexdigit[uc & 0x0f];
            }
          else if (!((uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z')
                     || (uc >= '0' && uc <= '9')))
            {
              *b++ = '_';
              *b++ = 'u';
              *b++ = hexdigit[(uc >> 12) & 0x0f];
              *b++ = hexdigit[(uc >> 8) & 0x0f];
              *b++ = hexdigit[(uc >> 4) & 0x0f];
              *b++ = hexdigit[uc & 0x0f];
            }
          else
            *b++ = uc;
        }
      *b++ = '\0';
      return (char *) xrealloc (class_name, b - class_name);
    }
}


/* Write a string in C# Unicode notation to the given stream.  */
static void
write_csharp_string (FILE *stream, const char *str)
{
  static const char hexdigit[] = "0123456789abcdef";
  const char *str_limit = str + strlen (str);

  fprintf (stream, "\"");
  while (str < str_limit)
    {
      ucs4_t uc;
      str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
      if (uc == 0x0000)
        fprintf (stream, "\\0");
      else if (uc == 0x0007)
        fprintf (stream, "\\a");
      else if (uc == 0x0008)
        fprintf (stream, "\\b");
      else if (uc == 0x0009)
        fprintf (stream, "\\t");
      else if (uc == 0x000a)
        fprintf (stream, "\\n");
      else if (uc == 0x000b)
        fprintf (stream, "\\v");
      else if (uc == 0x000c)
        fprintf (stream, "\\f");
      else if (uc == 0x000d)
        fprintf (stream, "\\r");
      else if (uc == 0x0022)
        fprintf (stream, "\\\"");
      else if (uc == 0x005c)
        fprintf (stream, "\\\\");
      else if (uc >= 0x0020 && uc < 0x007f)
        fprintf (stream, "%c", (int) uc);
      else if (uc < 0x10000)
        fprintf (stream, "\\u%c%c%c%c",
                 hexdigit[(uc >> 12) & 0x0f], hexdigit[(uc >> 8) & 0x0f],
                 hexdigit[(uc >> 4) & 0x0f], hexdigit[uc & 0x0f]);
      else
        fprintf (stream, "\\U%c%c%c%c%c%c%c%c",
                 hexdigit[(uc >> 28) & 0x0f], hexdigit[(uc >> 24) & 0x0f],
                 hexdigit[(uc >> 20) & 0x0f], hexdigit[(uc >> 16) & 0x0f],
                 hexdigit[(uc >> 12) & 0x0f], hexdigit[(uc >> 8) & 0x0f],
                 hexdigit[(uc >> 4) & 0x0f], hexdigit[uc & 0x0f]);
    }
  fprintf (stream, "\"");
}


/* Write a (msgctxt, msgid) pair as a string in C# Unicode notation to the
   given stream.  */
static void
write_csharp_msgid (FILE *stream, message_ty *mp)
{
  const char *msgctxt = mp->msgctxt;
  const char *msgid = mp->msgid;

  if (msgctxt == NULL)
    write_csharp_string (stream, msgid);
  else
    {
      size_t msgctxt_len = strlen (msgctxt);
      size_t msgid_len = strlen (msgid);
      size_t combined_len = msgctxt_len + 1 + msgid_len;
      char *combined;

      combined = (char *) xmalloca (combined_len + 1);
      memcpy (combined, msgctxt, msgctxt_len);
      combined[msgctxt_len] = MSGCTXT_SEPARATOR;
      memcpy (combined + msgctxt_len + 1, msgid, msgid_len + 1);

      write_csharp_string (stream, combined);

      freea (combined);
    }
}


/* Write C# code that returns the value for a message.  If the message
   has plural forms, it is an expression of type System.String[], otherwise it
   is an expression of type System.String.  */
static void
write_csharp_msgstr (FILE *stream, message_ty *mp)
{
  if (mp->msgid_plural != NULL)
    {
      bool first;
      const char *p;

      fprintf (stream, "new System.String[] { ");
      for (p = mp->msgstr, first = true;
           p < mp->msgstr + mp->msgstr_len;
           p += strlen (p) + 1, first = false)
        {
          if (!first)
            fprintf (stream, ", ");
          write_csharp_string (stream, p);
        }
      fprintf (stream, " }");
    }
  else
    {
      if (mp->msgstr_len != strlen (mp->msgstr) + 1)
        abort ();

      write_csharp_string (stream, mp->msgstr);
    }
}


/* Tests whether a plural expression, evaluated according to the C rules,
   can only produce the values 0 and 1.  */
static bool
is_expression_boolean (struct expression *exp)
{
  switch (exp->operation)
    {
    case var:
    case mult:
    case divide:
    case module:
    case plus:
    case minus:
      return false;
    case lnot:
    case less_than:
    case greater_than:
    case less_or_equal:
    case greater_or_equal:
    case equal:
    case not_equal:
    case land:
    case lor:
      return true;
    case num:
      return (exp->val.num == 0 || exp->val.num == 1);
    case qmop:
      return is_expression_boolean (exp->val.args[1])
             && is_expression_boolean (exp->val.args[2]);
    default:
      abort ();
    }
}


/* Write C# code that evaluates a plural expression according to the C rules.
   The variable is called 'n'.  */
static void
write_csharp_expression (FILE *stream, const struct expression *exp, bool as_boolean)
{
  /* We use parentheses everywhere.  This frees us from tracking the priority
     of arithmetic operators.  */
  if (as_boolean)
    {
      /* Emit a C# expression of type 'bool'.  */
      switch (exp->operation)
        {
        case num:
          fprintf (stream, "%s", exp->val.num ? "true" : "false");
          return;
        case lnot:
          fprintf (stream, "(!");
          write_csharp_expression (stream, exp->val.args[0], true);
          fprintf (stream, ")");
          return;
        case less_than:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " < ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case greater_than:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " > ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case less_or_equal:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " <= ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case greater_or_equal:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " >= ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case equal:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " == ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case not_equal:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " != ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case land:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], true);
          fprintf (stream, " && ");
          write_csharp_expression (stream, exp->val.args[1], true);
          fprintf (stream, ")");
          return;
        case lor:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], true);
          fprintf (stream, " || ");
          write_csharp_expression (stream, exp->val.args[1], true);
          fprintf (stream, ")");
          return;
        case qmop:
          if (is_expression_boolean (exp->val.args[1])
              && is_expression_boolean (exp->val.args[2]))
            {
              fprintf (stream, "(");
              write_csharp_expression (stream, exp->val.args[0], true);
              fprintf (stream, " ? ");
              write_csharp_expression (stream, exp->val.args[1], true);
              fprintf (stream, " : ");
              write_csharp_expression (stream, exp->val.args[2], true);
              fprintf (stream, ")");
              return;
            }
          FALLTHROUGH;
        case var:
        case mult:
        case divide:
        case module:
        case plus:
        case minus:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp, false);
          fprintf (stream, " != 0)");
          return;
        default:
          abort ();
        }
    }
  else
    {
      /* Emit a C# expression of type 'long'.  */
      switch (exp->operation)
        {
        case var:
          fprintf (stream, "n");
          return;
        case num:
          fprintf (stream, "%lu", exp->val.num);
          return;
        case mult:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " * ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case divide:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " / ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case module:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " %% ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case plus:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " + ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case minus:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], false);
          fprintf (stream, " - ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, ")");
          return;
        case qmop:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp->val.args[0], true);
          fprintf (stream, " ? ");
          write_csharp_expression (stream, exp->val.args[1], false);
          fprintf (stream, " : ");
          write_csharp_expression (stream, exp->val.args[2], false);
          fprintf (stream, ")");
          return;
        case lnot:
        case less_than:
        case greater_than:
        case less_or_equal:
        case greater_or_equal:
        case equal:
        case not_equal:
        case land:
        case lor:
          fprintf (stream, "(");
          write_csharp_expression (stream, exp, true);
          fprintf (stream, " ? 1 : 0)");
          return;
        default:
          abort ();
        }
    }
}


/* Write the C# code for the GettextResourceSet subclass to the given stream.
   Note that we use fully qualified class names and no "using" statements,
   because applications can have their own classes called X.Y.Hashtable or
   X.Y.String.  */
static void
write_csharp_code (FILE *stream, const char *culture_name, const char *class_name, message_list_ty *mlp)
{
  const char *last_dot;
  const char *class_name_last_part;
  unsigned int plurals;
  size_t j;

  fprintf (stream,
           "/* Automatically generated by GNU msgfmt.  Do not modify!  */\n");

  /* We chose to use a "using" statement here, to avoid a bug in the pnet-0.6.0
     compiler.  */
  fprintf (stream, "using GNU.Gettext;\n");

  /* Assign a strong name to the assembly, so that two different localizations
     of the same domain can be loaded one after the other.  This strong name
     tells the Global Assembly Cache that they are meant to be different.  */
  fprintf (stream, "[assembly: System.Reflection.AssemblyCulture(");
  write_csharp_string (stream, culture_name);
  fprintf (stream, ")]\n");

  last_dot = strrchr (class_name, '.');
  if (last_dot != NULL)
    {
      fprintf (stream, "namespace ");
      fwrite (class_name, 1, last_dot - class_name, stream);
      fprintf (stream, " {\n");
      class_name_last_part = last_dot + 1;
    }
  else
    class_name_last_part = class_name;
  fprintf (stream, "public class %s : GettextResourceSet {\n",
           class_name_last_part);

  /* Determine whether there are plural messages.  */
  plurals = 0;
  for (j = 0; j < mlp->nitems; j++)
    if (mlp->item[j]->msgid_plural != NULL)
      plurals++;

  /* Emit the constructor.  */
  fprintf (stream, "  public %s ()\n", class_name_last_part);
  fprintf (stream, "    : base () {\n");
  fprintf (stream, "  }\n");

  /* Emit the TableInitialized field.  */
  fprintf (stream, "  private bool TableInitialized;\n");

  /* Emit the ReadResources method.  */
  fprintf (stream, "  protected override void ReadResources () {\n");
  /* In some implementations, such as mono < 2009-02-27, the ReadResources
     method is called just once, when Table == null.  In other implementations,
     such as mono >= 2009-02-27, it is called at every GetObject call, and it
     is responsible for doing the initialization only once, even when called
     simultaneously from multiple threads.  */
  fprintf (stream, "    if (!TableInitialized) {\n");
  fprintf (stream, "      lock (this) {\n");
  fprintf (stream, "        if (!TableInitialized) {\n");
  /* In some implementations, the ResourceSet constructor initializes Table
     before calling ReadResources().  In other implementations, the
     ReadResources() method is expected to initialize the Table.  */
  fprintf (stream, "          if (Table == null)\n");
  fprintf (stream, "            Table = new System.Collections.Hashtable();\n");
  fprintf (stream, "          System.Collections.Hashtable t = Table;\n");
  for (j = 0; j < mlp->nitems; j++)
    {
      fprintf (stream, "          t.Add(");
      write_csharp_msgid (stream, mlp->item[j]);
      fprintf (stream, ",");
      write_csharp_msgstr (stream, mlp->item[j]);
      fprintf (stream, ");\n");
    }
  fprintf (stream, "          TableInitialized = true;\n");
  fprintf (stream, "        }\n");
  fprintf (stream, "      }\n");
  fprintf (stream, "    }\n");
  fprintf (stream, "  }\n");

  /* Emit the msgid_plural strings.  Only used by msgunfmt.  */
  if (plurals)
    {
      fprintf (stream, "  public static System.Collections.Hashtable GetMsgidPluralTable () {\n");
      fprintf (stream, "    System.Collections.Hashtable t = new System.Collections.Hashtable();\n");
      for (j = 0; j < mlp->nitems; j++)
        if (mlp->item[j]->msgid_plural != NULL)
          {
            fprintf (stream, "    t.Add(");
            write_csharp_msgid (stream, mlp->item[j]);
            fprintf (stream, ",");
            write_csharp_string (stream, mlp->item[j]->msgid_plural);
            fprintf (stream, ");\n");
          }
      fprintf (stream, "    return t;\n");
      fprintf (stream, "  }\n");
    }

  /* Emit the PluralEval function.  It is a subroutine for GetPluralString.  */
  if (plurals)
    {
      message_ty *header_entry;
      const struct expression *plural;
      unsigned long int nplurals;

      header_entry = message_list_search (mlp, NULL, "");
      extract_plural_expression (header_entry ? header_entry->msgstr : NULL,
                                 &plural, &nplurals);

      fprintf (stream, "  protected override long PluralEval (long n) {\n");
      fprintf (stream, "    return ");
      write_csharp_expression (stream, plural, false);
      fprintf (stream, ";\n");
      fprintf (stream, "  }\n");
    }

  /* Terminate the class.  */
  fprintf (stream, "}\n");

  if (last_dot != NULL)
    /* Terminate the namespace.  */
    fprintf (stream, "}\n");
}


int
msgdomain_write_csharp (message_list_ty *mlp, const char *canon_encoding,
                        const char *resource_name, const char *locale_name,
                        const char *directory)
{
  int retval;
  struct temp_dir *tmpdir;
  char *culture_name;
  char *output_file;
  char *class_name;
  char *csharp_file_name;
  FILE *csharp_file;
  const char *gettextlibdir;
  const char *csharp_sources[1];
  const char *libdirs[1];
  const char *libraries[1];

  /* If no entry for this resource/domain, don't even create the file.  */
  if (mlp->nitems == 0)
    return 0;

  retval = 1;

  /* Convert the messages to Unicode.  */
  iconv_message_list (mlp, canon_encoding, po_charset_utf8, NULL);

  /* Support for "reproducible builds": Delete information that may vary
     between builds in the same conditions.  */
  message_list_delete_header_field (mlp, "POT-Creation-Date:");

  /* Create a temporary directory where we can put the C# file.
     A simple temporary file would also be possible but would require us to
     define our own variant of mkstemp(): On one hand the functions mktemp(),
     tmpnam(), tempnam() present a security risk, and on the other hand the
     function mkstemp() doesn't allow to specify a fixed suffix of the file.
     It is simpler to create a temporary directory.  */
  tmpdir = create_temp_dir ("msg", NULL, false);
  if (tmpdir == NULL)
    goto quit1;

  /* Assign a default value to the resource name.  */
  if (resource_name == NULL)
    resource_name = "Messages";

  /* Convert the locale name to a .NET specific culture name.  */
  culture_name = xstrdup (locale_name);
  {
    char *p;
    for (p = culture_name; *p != '\0'; p++)
      if (*p == '_')
        *p = '-';
    if (strncmp (culture_name, "sr-CS", 5) == 0)
      memcpy (culture_name, "sr-SP", 5);
    p = strchr (culture_name, '@');
    if (p != NULL)
      {
        if (strcmp (p, "@latin") == 0)
          strcpy (p, "-Latn");
        else if (strcmp (p, "@cyrillic") == 0)
          strcpy (p, "-Cyrl");
      }
    if (strcmp (culture_name, "sr-SP") == 0)
      {
        free (culture_name);
        culture_name = xstrdup ("sr-SP-Latn");
      }
    else if (strcmp (culture_name, "uz-UZ") == 0)
      {
        free (culture_name);
        culture_name = xstrdup ("uz-UZ-Latn");
      }
  }

  /* Compute the output file name.  This code must be kept consistent with
     intl.cs, function GetSatelliteAssembly().  */
  {
    char *output_dir = xconcatenated_filename (directory, culture_name, NULL);
    struct stat statbuf;

    /* Try to create the output directory if it does not yet exist.  */
    if (stat (output_dir, &statbuf) < 0 && errno == ENOENT)
      if (mkdir (output_dir, S_IRUSR | S_IWUSR | S_IXUSR
                             | S_IRGRP | S_IWGRP | S_IXGRP
                             | S_IROTH | S_IWOTH | S_IXOTH) < 0)
        {
          error (0, errno, _("failed to create directory \"%s\""), output_dir);
          free (output_dir);
          goto quit2;
        }

    output_file =
      xconcatenated_filename (output_dir, resource_name, ".resources.dll");

    free (output_dir);
  }

  /* Compute the class name.  This code must be kept consistent with intl.cs,
     function InstantiateResourceSet().  */
  {
    char *class_name_part1 = construct_class_name (resource_name);
    char *p;

    class_name =
      XNMALLOC (strlen (class_name_part1) + 1 + strlen (culture_name) + 1, char);
    sprintf (class_name, "%s_%s", class_name_part1, culture_name);
    for (p = class_name + strlen (class_name_part1) + 1; *p != '\0'; p++)
      if (*p == '-')
        *p = '_';
    free (class_name_part1);
  }

  /* Compute the temporary C# file name.  It must end in ".cs", so that
     the C# compiler recognizes that it is C# source code.  */
  csharp_file_name =
    xconcatenated_filename (tmpdir->dir_name, "resset.cs", NULL);

  /* Create the C# file.  */
  register_temp_file (tmpdir, csharp_file_name);
  csharp_file = fopen_temp (csharp_file_name, "w", false);
  if (csharp_file == NULL)
    {
      error (0, errno, _("failed to create \"%s\""), csharp_file_name);
      unregister_temp_file (tmpdir, csharp_file_name);
      goto quit3;
    }

  write_csharp_code (csharp_file, culture_name, class_name, mlp);

  if (fwriteerror_temp (csharp_file))
    {
      error (0, errno, _("error while writing \"%s\" file"), csharp_file_name);
      goto quit3;
    }

  /* Make it possible to override the .dll location.  This is
     necessary for running the testsuite before "make install".  */
  gettextlibdir = getenv ("GETTEXTCSHARPLIBDIR");
  if (gettextlibdir == NULL || gettextlibdir[0] == '\0')
    gettextlibdir = relocate (LIBDIR);

  /* Compile the C# file to a .dll file.  */
  csharp_sources[0] = csharp_file_name;
  libdirs[0] = gettextlibdir;
  libraries[0] = "GNU.Gettext";
  if (compile_csharp_class (csharp_sources, 1, libdirs, 1, libraries, 1,
                            output_file, true, false, verbose > 0))
    {
      if (!verbose)
        error (0, 0, _("compilation of C# class failed, please try --verbose"));
      else
        error (0, 0, _("compilation of C# class failed"));
      goto quit3;
    }

  retval = 0;

 quit3:
  free (csharp_file_name);
  free (class_name);
  free (output_file);
 quit2:
  free (culture_name);
  cleanup_temp_dir (tmpdir);
 quit1:
  return retval;
}
