/* Test of command line argument processing.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2009.  */

static int a_seen;
static int b_seen;
static int q_seen;

static const struct option long_options_required[] =
  {
    { "alpha",    no_argument,       NULL, 'a' },
    { "beta",     no_argument,       &b_seen, 1 },
    { "prune",    required_argument, NULL, 'p' },
    { "quetsche", required_argument, &q_seen, 1 },
    { "xtremely-",no_argument,       NULL, 1003 },
    { "xtra",     no_argument,       NULL, 1001 },
    { "xtreme",   no_argument,       NULL, 1002 },
    { "xtremely", no_argument,       NULL, 1003 },
    { NULL,       0,                 NULL, 0 }
  };

static const struct option long_options_optional[] =
  {
    { "alpha",    no_argument,       NULL, 'a' },
    { "beta",     no_argument,       &b_seen, 1 },
    { "prune",    optional_argument, NULL, 'p' },
    { "quetsche", optional_argument, &q_seen, 1 },
    { NULL,       0,                 NULL, 0 }
  };

static void
getopt_long_loop (int argc, const char **argv,
                  const char *options, const struct option *long_options,
                  const char **p_value, const char **q_value,
                  int *non_options_count, const char **non_options,
                  int *unrecognized)
{
  int option_index = -1;
  int c;

  opterr = 0;
  q_seen = 0;
  while ((c = getopt_long (argc, (char **) argv, options, long_options,
                           &option_index))
         != -1)
    {
      switch (c)
        {
        case 0:
          /* An option with a non-NULL flag pointer was processed.  */
          if (q_seen)
            *q_value = optarg;
          break;
        case 'a':
          a_seen++;
          break;
        case 'b':
          b_seen = 1;
          break;
        case 'p':
          *p_value = optarg;
          break;
        case 'q':
          *q_value = optarg;
          break;
        case '\1':
          /* Must only happen with option '-' at the beginning.  */
          ASSERT (options[0] == '-');
          non_options[(*non_options_count)++] = optarg;
          break;
        case ':':
          /* Must only happen with option ':' at the beginning.  */
          ASSERT (options[0] == ':'
                  || ((options[0] == '-' || options[0] == '+')
                      && options[1] == ':'));
          FALLTHROUGH;
        case '?':
          *unrecognized = optopt;
          break;
        default:
          *unrecognized = c;
          break;
        }
    }
}

/* Reduce casting, so we can use string literals elsewhere.
   getopt_long takes an array of char*, but luckily does not modify
   those elements, so we can pass const char*.  */
static int
do_getopt_long (int argc, const char **argv, const char *shortopts,
                const struct option *longopts, int *longind)
{
  return getopt_long (argc, (char **) argv, shortopts, longopts, longind);
}

static void
test_getopt_long (void)
{
  int start;

  /* Test disambiguation of options.  */
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--x";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xt";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtr";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtra";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == 1001);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtre";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtrem";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtreme";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == 1002);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtremel";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == 1003);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--xtremely";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "ab", long_options_required, &option_index);
    ASSERT (c == 1003);
  }

  /* Check that -W handles unknown options.  */
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-W";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "W;", long_options_required, &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 'W');
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-Wunknown";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "W;", long_options_required, &option_index);
    /* glibc and BSD behave differently here, but for now, we allow
       both behaviors since W support is not frequently used.  */
    if (c == '?')
      {
        ASSERT (optopt == 0);
        ASSERT (optarg == NULL);
      }
    else
      {
        ASSERT (c == 'W');
        ASSERT (strcmp (optarg, "unknown") == 0);
      }
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-W";
    argv[argc++] = "unknown";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "W;", long_options_required, &option_index);
    /* glibc and BSD behave differently here, but for now, we allow
       both behaviors since W support is not frequently used.  */
    if (c == '?')
      {
        ASSERT (optopt == 0);
        ASSERT (optarg == NULL);
      }
    else
      {
        ASSERT (c == 'W');
        ASSERT (strcmp (optarg, "unknown") == 0);
      }
  }

  /* Test that 'W' does not dump core:
     https://sourceware.org/bugzilla/show_bug.cgi?id=12922  */
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-W";
    argv[argc++] = "dummy";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long (argc, argv, "W;", NULL, &option_index);
    ASSERT (c == 'W');
    ASSERT (optind == 2);
  }

  /* Test processing of boolean short options.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-a";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-b";
      argv[argc++] = "-a";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-ba";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-ab";
      argv[argc++] = "-a";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 2);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }

  /* Test processing of boolean long options.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--alpha";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--beta";
      argv[argc++] = "--alpha";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--alpha";
      argv[argc++] = "--beta";
      argv[argc++] = "--alpha";
      argv[argc++] = "--beta";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 2);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }

  /* Test processing of boolean long options via -W.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Walpha";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abW;", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-W";
      argv[argc++] = "beta";
      argv[argc++] = "-W";
      argv[argc++] = "alpha";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "aW;b", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Walpha";
      argv[argc++] = "-Wbeta";
      argv[argc++] = "-Walpha";
      argv[argc++] = "-Wbeta";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "W;ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 2);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }

  /* Test processing of short options with arguments.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-pfoo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-ab";
      argv[argc++] = "-q";
      argv[argc++] = "baz";
      argv[argc++] = "-pfoo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value != NULL && strcmp (q_value, "baz") == 0);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }

  /* Test processing of long options with arguments.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-ab";
      argv[argc++] = "--q";
      argv[argc++] = "baz";
      argv[argc++] = "--p=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value != NULL && strcmp (q_value, "baz") == 0);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }

  /* Test processing of long options with arguments via -W.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Wp=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:q:W;", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-W";
      argv[argc++] = "p";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p:W;q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-ab";
      argv[argc++] = "-Wq";
      argv[argc++] = "baz";
      argv[argc++] = "-W";
      argv[argc++] = "p=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "W;abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value != NULL && strcmp (q_value, "baz") == 0);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 6);
    }

  /* Test processing of short options with optional arguments.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-pfoo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }

  /* Test processing of long options with optional arguments.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p=";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && *p_value == '\0');
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "--p";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }

  /* Test processing of long options with optional arguments via -W.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Wp=foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::W;", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Wp";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::q::W;", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-Wp=";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "W;p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && *p_value == '\0');
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-W";
      argv[argc++] = "p=";
      argv[argc++] = "foo";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "W;p::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && *p_value == '\0');
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 3);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-W";
      argv[argc++] = "p";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "W;abp::q::", long_options_optional,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      /* ASSERT (p_value == NULL); */
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }

  /* Check that invalid options are recognized.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "-x";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 'x');
      ASSERT (optind == 5);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "-:";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == ':');
      ASSERT (optind == 5);
    }

  /* Check that unexpected arguments are recognized.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "--a=";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 'a');
      ASSERT (optind == 4);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "foo";
      argv[argc++] = "--b=";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "foo") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      /* When flag is non-zero, glibc sets optopt anyway, but BSD
         leaves optopt unchanged.  */
      ASSERT (unrecognized == 1 || unrecognized == 0);
      ASSERT (optind == 4);
    }

  /* Check that by default, non-options arguments are moved to the end.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "-p") == 0);
      ASSERT (strcmp (argv[2], "billy") == 0);
      ASSERT (strcmp (argv[3], "-a") == 0);
      ASSERT (strcmp (argv[4], "donald") == 0);
      ASSERT (strcmp (argv[5], "duck") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }

  /* Check that '--' ends the argument processing.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[20];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "--";
      argv[argc++] = "-b";
      argv[argc++] = "foo";
      argv[argc++] = "-q";
      argv[argc++] = "johnny";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "-p") == 0);
      ASSERT (strcmp (argv[2], "billy") == 0);
      ASSERT (strcmp (argv[3], "-a") == 0);
      ASSERT (strcmp (argv[4], "--") == 0);
      ASSERT (strcmp (argv[5], "donald") == 0);
      ASSERT (strcmp (argv[6], "duck") == 0);
      ASSERT (strcmp (argv[7], "-b") == 0);
      ASSERT (strcmp (argv[8], "foo") == 0);
      ASSERT (strcmp (argv[9], "-q") == 0);
      ASSERT (strcmp (argv[10], "johnny") == 0);
      ASSERT (strcmp (argv[11], "bar") == 0);
      ASSERT (argv[12] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 5);
    }

  /* Check that the '-' flag causes non-options to be returned in order.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "-abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "donald") == 0);
      ASSERT (strcmp (argv[2], "-p") == 0);
      ASSERT (strcmp (argv[3], "billy") == 0);
      ASSERT (strcmp (argv[4], "duck") == 0);
      ASSERT (strcmp (argv[5], "-a") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 3);
      ASSERT (strcmp (non_options[0], "donald") == 0);
      ASSERT (strcmp (non_options[1], "duck") == 0);
      ASSERT (strcmp (non_options[2], "bar") == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 7);
    }

  /* Check that '--' ends the argument processing.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[20];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "--";
      argv[argc++] = "-b";
      argv[argc++] = "foo";
      argv[argc++] = "-q";
      argv[argc++] = "johnny";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "-abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "donald") == 0);
      ASSERT (strcmp (argv[2], "-p") == 0);
      ASSERT (strcmp (argv[3], "billy") == 0);
      ASSERT (strcmp (argv[4], "duck") == 0);
      ASSERT (strcmp (argv[5], "-a") == 0);
      ASSERT (strcmp (argv[6], "--") == 0);
      ASSERT (strcmp (argv[7], "-b") == 0);
      ASSERT (strcmp (argv[8], "foo") == 0);
      ASSERT (strcmp (argv[9], "-q") == 0);
      ASSERT (strcmp (argv[10], "johnny") == 0);
      ASSERT (strcmp (argv[11], "bar") == 0);
      ASSERT (argv[12] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      if (non_options_count == 2)
      {
        /* glibc behaviour.  */
        ASSERT (non_options_count == 2);
        ASSERT (strcmp (non_options[0], "donald") == 0);
        ASSERT (strcmp (non_options[1], "duck") == 0);
        ASSERT (unrecognized == 0);
        ASSERT (optind == 7);
      }
      else
      {
        /* Another valid behaviour.  */
        ASSERT (non_options_count == 7);
        ASSERT (strcmp (non_options[0], "donald") == 0);
        ASSERT (strcmp (non_options[1], "duck") == 0);
        ASSERT (strcmp (non_options[2], "-b") == 0);
        ASSERT (strcmp (non_options[3], "foo") == 0);
        ASSERT (strcmp (non_options[4], "-q") == 0);
        ASSERT (strcmp (non_options[5], "johnny") == 0);
        ASSERT (strcmp (non_options[6], "bar") == 0);
        ASSERT (unrecognized == 0);
        ASSERT (optind == 12);
      }
    }

  /* Check that the '-' flag has to come first.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:-", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "-p") == 0);
      ASSERT (strcmp (argv[2], "billy") == 0);
      ASSERT (strcmp (argv[3], "-a") == 0);
      ASSERT (strcmp (argv[4], "donald") == 0);
      ASSERT (strcmp (argv[5], "duck") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }

  /* Check that the '+' flag causes the first non-option to terminate the
     loop.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "+abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "donald") == 0);
      ASSERT (strcmp (argv[2], "-p") == 0);
      ASSERT (strcmp (argv[3], "billy") == 0);
      ASSERT (strcmp (argv[4], "duck") == 0);
      ASSERT (strcmp (argv[5], "-a") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 1);
    }
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-+";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "+abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == '+');
      ASSERT (optind == 2);
    }

  /* Check that '--' ends the argument processing.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[20];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "--";
      argv[argc++] = "-b";
      argv[argc++] = "foo";
      argv[argc++] = "-q";
      argv[argc++] = "johnny";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "+abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "donald") == 0);
      ASSERT (strcmp (argv[2], "-p") == 0);
      ASSERT (strcmp (argv[3], "billy") == 0);
      ASSERT (strcmp (argv[4], "duck") == 0);
      ASSERT (strcmp (argv[5], "-a") == 0);
      ASSERT (strcmp (argv[6], "--") == 0);
      ASSERT (strcmp (argv[7], "-b") == 0);
      ASSERT (strcmp (argv[8], "foo") == 0);
      ASSERT (strcmp (argv[9], "-q") == 0);
      ASSERT (strcmp (argv[10], "johnny") == 0);
      ASSERT (strcmp (argv[11], "bar") == 0);
      ASSERT (argv[12] == NULL);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 1);
    }

  /* Check that the '+' flag has to come first.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:+", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "-p") == 0);
      ASSERT (strcmp (argv[2], "billy") == 0);
      ASSERT (strcmp (argv[3], "-a") == 0);
      ASSERT (strcmp (argv[4], "donald") == 0);
      ASSERT (strcmp (argv[5], "duck") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 0);
      ASSERT (p_value != NULL && strcmp (p_value, "billy") == 0);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }
}

/* Test behavior of getopt_long when POSIXLY_CORRECT is set in the
   environment.  Options with optional arguments should not change
   behavior just because of an environment variable.
   https://lists.gnu.org/r/bug-m4/2006-09/msg00028.html  */
static void
test_getopt_long_posix (void)
{
  int start;

  /* Check that POSIXLY_CORRECT stops parsing the same as leading '+'.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "donald";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc++] = "duck";
      argv[argc++] = "-a";
      argv[argc++] = "bar";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "abp:q:", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (strcmp (argv[0], "program") == 0);
      ASSERT (strcmp (argv[1], "donald") == 0);
      ASSERT (strcmp (argv[2], "-p") == 0);
      ASSERT (strcmp (argv[3], "billy") == 0);
      ASSERT (strcmp (argv[4], "duck") == 0);
      ASSERT (strcmp (argv[5], "-a") == 0);
      ASSERT (strcmp (argv[6], "bar") == 0);
      ASSERT (argv[7] == NULL);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 1);
    }

  /* Check that POSIXLY_CORRECT doesn't change optional arguments.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-p";
      argv[argc++] = "billy";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "p::", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 0);
      ASSERT (b_seen == 0);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 2);
    }

  /* Check that leading - still sees options after non-options.  */
  for (start = 0; start <= 1; start++)
    {
      const char *p_value = NULL;
      const char *q_value = NULL;
      int non_options_count = 0;
      const char *non_options[10];
      int unrecognized = 0;
      int argc = 0;
      const char *argv[10];
      a_seen = 0;
      b_seen = 0;

      argv[argc++] = "program";
      argv[argc++] = "-a";
      argv[argc++] = "billy";
      argv[argc++] = "-b";
      argv[argc] = NULL;
      optind = start;
      getopt_long_loop (argc, argv, "-ab", long_options_required,
                        &p_value, &q_value,
                        &non_options_count, non_options, &unrecognized);
      ASSERT (a_seen == 1);
      ASSERT (b_seen == 1);
      ASSERT (p_value == NULL);
      ASSERT (q_value == NULL);
      ASSERT (non_options_count == 1);
      ASSERT (strcmp (non_options[0], "billy") == 0);
      ASSERT (unrecognized == 0);
      ASSERT (optind == 4);
    }
}

/* Reduce casting, so we can use string literals elsewhere.
   getopt_long_only takes an array of char*, but luckily does not
   modify those elements, so we can pass const char*.  */
static int
do_getopt_long_only (int argc, const char **argv, const char *shortopts,
                     const struct option *longopts, int *longind)
{
  return getopt_long_only (argc, (char **) argv, shortopts, longopts, longind);
}

static void
test_getopt_long_only (void)
{
  /* Test disambiguation of options.  */
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-x";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "ab", long_options_required,
                             &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-x";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx", long_options_required,
                             &option_index);
    ASSERT (c == 'x');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--x";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx", long_options_required,
                             &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-b";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    b_seen = 0;
    c = do_getopt_long_only (argc, argv, "abx", long_options_required,
                             &option_index);
    ASSERT (c == 'b');
    ASSERT (b_seen == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "--b";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    b_seen = 0;
    c = do_getopt_long_only (argc, argv, "abx", long_options_required,
                             &option_index);
    ASSERT (c == 0);
    ASSERT (b_seen == 1);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xt";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "ab", long_options_required,
                             &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xt";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx", long_options_required,
                             &option_index);
    ASSERT (c == '?');
    ASSERT (optopt == 0);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xtra";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "ab", long_options_required,
                             &option_index);
    ASSERT (c == 1001);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xtreme";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx:", long_options_required,
                             &option_index);
    ASSERT (c == 1002);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xtremel";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "ab", long_options_required,
                             &option_index);
    /* glibc getopt_long_only is intentionally different from
       getopt_long when handling a prefix that is common to two
       spellings, when both spellings have the same option directives.
       BSD getopt_long_only treats both cases the same.  */
    ASSERT (c == 1003 || c == '?');
    ASSERT (optind == 2);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xtremel";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx::", long_options_required,
                             &option_index);
    /* glibc getopt_long_only is intentionally different from
       getopt_long when handling a prefix that is common to two
       spellings, when both spellings have the same option directives.
       BSD getopt_long_only treats both cases the same.  */
    ASSERT (c == 1003 || c == '?');
    ASSERT (optind == 2);
    ASSERT (optarg == NULL);
  }
  {
    int argc = 0;
    const char *argv[10];
    int option_index;
    int c;

    argv[argc++] = "program";
    argv[argc++] = "-xtras";
    argv[argc] = NULL;
    optind = 1;
    opterr = 0;
    c = do_getopt_long_only (argc, argv, "abx::", long_options_required,
                             &option_index);
    ASSERT (c == 'x');
    ASSERT (strcmp (optarg, "tras") == 0);
  }
}
