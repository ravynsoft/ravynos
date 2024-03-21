/* Test of sh-quote module.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2012.  */

#include <config.h>

/* Specification.  */
#include "sh-quote.h"

#include <limits.h>
#include <string.h>

#include "macros.h"

static void
check_one (const char *input, const char *expected)
{
  char buf[1000];
  size_t output_len;
  char *output;
  char *bufend;

  output_len = shell_quote_length (input);

  output = shell_quote (input);
  ASSERT (strlen (output) == output_len);

  ASSERT (output_len <= sizeof (buf) - 2);
  memset (buf, '\0', output_len + 1);
  buf[output_len + 1] = '%';
  bufend = shell_quote_copy (buf, input);
  ASSERT (bufend == buf + output_len);
  ASSERT (memcmp (buf, output, output_len + 1) == 0);
  ASSERT (buf[output_len + 1] == '%');

  ASSERT (strcmp (output, expected) == 0);

  free (output);
}

int
main (void)
{
  /* Check the shell_quote_length, shell_quote_copy, shell_quote functions.  */
  {
    int c;

    /* Empty argument.  */
    check_one ("", "''");

    /* Identifier or number.  */
    check_one ("foo", "foo");
    check_one ("phr0ck", "phr0ck");

    /* Whitespace would be interpreted as argument separator by the shell.  */
    check_one ("foo\tbar", "'foo\tbar'");
    check_one ("foo\nbar", "'foo\nbar'");
    check_one ("foo\rbar", "'foo\rbar'");
    check_one ("foo bar", "'foo bar'");

    /* '!' at the beginning of argv[0] would introduce a negated command.  */
    check_one ("!foo", "'!foo'");

    /* '"' would be interpreted as the start of a string.  */
    check_one ("\"foo\"bar", "'\"foo\"bar'");

    /* '#' at the beginning of an argument would be interpreted as the start
       of a comment.  */
    check_one ("#foo", "'#foo'");

    /* '$' at the beginning of an argument would be interpreted as a variable
       reference.  */
    check_one ("$foo", "'$foo'");

    /* '&' at the beginning of an argument would be interpreted as a background
       task indicator.  */
    check_one ("&", "'&'");

    /* "'" would be interpreted as the start of a string.  */
    check_one ("'foo'bar", "\"'foo'bar\"");

    /* '(' at the beginning of argv[0] would introduce a subshell command.  */
    check_one ("(", "'('");

    /* ')' at the beginning of an argument would be interpreted as the end of
       the command.  */
    check_one (")", "')'");

    /* '*' would be interpreted as a wildcard character.  */
    check_one ("*", "'*'");
    check_one ("*foo", "'*foo'");

    /* ';' at the beginning of an argument would be interpreted as an empty
       statement in argv[0] and as the end of the command otherwise.  */
    check_one (";", "';'");
    check_one ("foo;", "'foo;'");

    /* '<' would be interpreted as a redirection of stdin.  */
    check_one ("<", "'<'");

    /* '=' inside argv[0] would be interpreted as an environment variable
       assignment.  */
    check_one ("foo=bar", "'foo=bar'");

    /* '>' would be interpreted as a redirection of stdout.  */
    check_one (">", "'>'");

    /* '?' would be interpreted as a wildcard character.  */
    check_one ("?", "'?'");
    check_one ("foo?bar", "'foo?bar'");

    /* '^' would be interpreted in old /bin/sh, e.g. SunOS 4.1.4.  */
    check_one ("^", "'^'");

    /* "[...]" would be interpreted as a wildcard pattern.  */
    check_one ("[", "'['");
    check_one ("]", "]"); /* or "']'" */

    /* '\' would be interpreted as an escape character.  */
    check_one ("\\foo", "'\\foo'");

    /* '`' would be interpreted as the start of a command substitution.  */
    check_one ("`foo", "'`foo'");

    /* '{' at the beginning of argv[0] would introduce a complex command.  */
    check_one ("{", "'{'");

    /* '|' at the beginning of an argument would be interpreted as a pipe
       between commands.  */
    check_one ("|", "'|'");

    /* '}' at the beginning of an argument would be interpreted as the end of
       the command.  */
    check_one ("}", "'}'");

    /* '~' at the beginning of an argument would be interpreted as a reference
       to a user's home directory.  */
    check_one ("~", "'~'");
    check_one ("~foo", "'~foo'");

    /* A string that contains both ' and ".  */
    check_one ("foo'bar\"baz", "'foo'\\''bar\"baz'"); /* or "\"foo'bar\\\"baz\"" */

    /* All other characters don't need quoting.  */
    for (c = 1; c <= UCHAR_MAX; c++)
      if (strchr ("\t\n\r !\"#$&'()*;<=>?^[\\]`{|}~", c) == NULL)
        {
          char s[5];
          s[0] = 'a';
          s[1] = (char) c;
          s[2] = 'z';
          s[3] = (char) c;
          s[4] = '\0';

          check_one (s, s);
        }
  }

  /* Check the shell_quote_argv function.  */
  {
    const char *argv[1];
    char *result;
    argv[0] = NULL;
    result = shell_quote_argv (argv);
    ASSERT (strcmp (result, "") == 0);
    free (result);
  }
  {
    const char *argv[2];
    char *result;
    argv[0] = "foo bar/baz";
    argv[1] = NULL;
    result = shell_quote_argv (argv);
    ASSERT (strcmp (result, "'foo bar/baz'") == 0); /* or "\"foo bar/baz\"" */
    free (result);
  }
  {
    const char *argv[3];
    char *result;
    argv[0] = "foo bar/baz";
    argv[1] = "$";
    argv[2] = NULL;
    result = shell_quote_argv (argv);
    ASSERT (strcmp (result, "'foo bar/baz' '$'") == 0); /* or "\"foo bar/baz\" \"\\$\"" */
    free (result);
  }

  return 0;
}
