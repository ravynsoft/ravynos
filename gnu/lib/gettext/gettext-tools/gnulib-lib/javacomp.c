/* Compile a Java program.
   Copyright (C) 2001-2003, 2006-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#include <config.h>
#include <alloca.h>

/* Specification.  */
#include "javacomp.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "javaversion.h"
#include "execute.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "classpath.h"
#include "xsetenv.h"
#include "sh-quote.h"
#include "binary-io.h"
#include "safe-read.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "concat-filename.h"
#include "fwriteerror.h"
#include "clean-temp.h"
#include "error.h"
#include "xvasprintf.h"
#include "verify.h"
#include "c-strstr.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Survey of Java compilers.

   A = does it work without CLASSPATH being set
   C = option to set CLASSPATH, other than setting it in the environment
   O = option for optimizing
   g = option for debugging
   T = test for presence

   Program  from        A  C               O  g  T

   $JAVAC   unknown     N  n/a            -O -g  true
   javac    JDK 1.1.8   Y  -classpath P   -O -g  javac 2>/dev/null; test $? = 1
   javac    JDK 1.3.0   Y  -classpath P   -O -g  javac 2>/dev/null; test $? -le 2

   All compilers support the option "-d DIRECTORY" for the base directory
   of the classes to be written.

   The CLASSPATH is a colon separated list of pathnames. (On Windows: a
   semicolon separated list of pathnames.)

   We try the Java compilers in the following order:
     1. getenv ("JAVAC"), because the user must be able to override our
        preferences,
     2. "javac", because it is a standard compiler.

   We unset the JAVA_HOME environment variable, because a wrong setting of
   this variable can confuse the JDK's javac.
 */

/* Return the default target_version.  */
static const char *
default_target_version (void)
{
  /* Use a cache.  Assumes that the PATH environment variable doesn't change
     during the lifetime of the program.  */
  static const char *java_version_cache;
  if (java_version_cache == NULL)
    {
      /* Determine the version from the found JVM.  */
      java_version_cache = javaexec_version ();
      if (java_version_cache == NULL)
        java_version_cache = "1.6";
      else if (java_version_cache[0] == '1'
               && java_version_cache[1] == '.'
               && java_version_cache[2] >= '1' && java_version_cache[2] <= '5'
               && java_version_cache[3] == '\0')
        {
          error (0, 0, _("The java program is too old. Cannot compile Java code for this old version any more."));
          java_version_cache = "1.6";
        }
      else if ((java_version_cache[0] == '1'
                && java_version_cache[1] == '.'
                && java_version_cache[2] >= '6' && java_version_cache[2] <= '8'
                && java_version_cache[3] == '\0')
               || (java_version_cache[0] == '9'
                   && java_version_cache[1] == '\0')
               || ((java_version_cache[0] >= '1'
                    && java_version_cache[0] <= '9')
                   && (java_version_cache[1] >= '0'
                       && java_version_cache[1] <= '9')
                   && java_version_cache[2] == '\0'))
        /* Here we could choose any target_version between source_version and
           the java_version_cache.  (If it is too small, it will be incremented
           below until it works.)  Since we documented in javacomp.h that it is
           determined from the JVM, we do that.  */
        ;
      else
        java_version_cache = "1.6";
    }
  return java_version_cache;
}

/* ======================= Source version dependent ======================= */

/* Convert a source version to an index.  */
#define SOURCE_VERSION_BOUND 94 /* exclusive upper bound */
static unsigned int
source_version_index (const char *source_version)
{
  if (source_version[0] == '1' && source_version[1] == '.')
    {
      if ((source_version[2] >= '6' && source_version[2] <= '8')
          && source_version[3] == '\0')
        return source_version[2] - '6';
    }
  else if (source_version[0] == '9' && source_version[1] == '\0')
    return 3;
  else if ((source_version[0] >= '1' && source_version[0] <= '9')
           && (source_version[1] >= '0' && source_version[1] <= '9')
           && source_version[2] == '\0')
    return (source_version[0] - '1') * 10 + source_version[1] - '0' + 4;
  error (EXIT_FAILURE, 0, _("invalid source_version argument to compile_java_class"));
  return 0;
}

/* ======================= Target version dependent ======================= */

/* Convert a target version to an index.  */
#define TARGET_VERSION_BOUND 94 /* exclusive upper bound */
static unsigned int
target_version_index (const char *target_version)
{
  if (target_version[0] == '1' && target_version[1] == '.'
      && (target_version[2] >= '6' && target_version[2] <= '8')
      && target_version[3] == '\0')
    return target_version[2] - '6';
  else if (target_version[0] == '9' && target_version[1] == '\0')
    return 3;
  else if ((target_version[0] >= '1' && target_version[0] <= '9')
           && (target_version[1] >= '0' && target_version[1] <= '9')
           && target_version[2] == '\0')
    return (target_version[0] - '1') * 10 + target_version[1] - '0' + 4;
  error (EXIT_FAILURE, 0, _("invalid target_version argument to compile_java_class"));
  return 0;
}

/* ======================== Compilation subroutines ======================== */

/* Try to compile a set of Java sources with $JAVAC.
   Return a failure indicator (true upon error).  */
static bool
compile_using_envjavac (const char *javac,
                        const char * const *java_sources,
                        unsigned int java_sources_count,
                        const char *directory,
                        bool optimize, bool debug,
                        bool verbose, bool null_stderr)
{
  /* Because $JAVAC may consist of a command and options, we use the
     shell.  Because $JAVAC has been set by the user, we leave all
     environment variables in place, including JAVA_HOME, and we don't
     erase the user's CLASSPATH.  */
  bool err;
  unsigned int command_length;
  char *command;
  const char *argv[4];
  int exitstatus;
  unsigned int i;
  char *p;

  command_length = strlen (javac);
  if (optimize)
    command_length += 3;
  if (debug)
    command_length += 3;
  if (directory != NULL)
    command_length += 4 + shell_quote_length (directory);
  for (i = 0; i < java_sources_count; i++)
    command_length += 1 + shell_quote_length (java_sources[i]);
  command_length += 1;

  command = (char *) xmalloca (command_length);
  p = command;
  /* Don't shell_quote $JAVAC, because it may consist of a command
     and options.  */
  memcpy (p, javac, strlen (javac));
  p += strlen (javac);
  if (optimize)
    {
      memcpy (p, " -O", 3);
      p += 3;
    }
  if (debug)
    {
      memcpy (p, " -g", 3);
      p += 3;
    }
  if (directory != NULL)
    {
      memcpy (p, " -d ", 4);
      p += 4;
      p = shell_quote_copy (p, directory);
    }
  for (i = 0; i < java_sources_count; i++)
    {
      *p++ = ' ';
      p = shell_quote_copy (p, java_sources[i]);
    }
  *p++ = '\0';
  /* Ensure command_length was correctly calculated.  */
  if (p - command > command_length)
    abort ();

  if (verbose)
    printf ("%s\n", command);

  argv[0] = BOURNE_SHELL;
  argv[1] = "-c";
  argv[2] = command;
  argv[3] = NULL;
  exitstatus = execute (javac, BOURNE_SHELL, argv, NULL,
                        false, false, false, null_stderr,
                        true, true, NULL);
  err = (exitstatus != 0);

  freea (command);

  return err;
}

/* Try to compile a set of Java sources with javac.
   Return a failure indicator (true upon error).  */
static bool
compile_using_javac (const char * const *java_sources,
                     unsigned int java_sources_count,
                     const char *nowarn_option,
                     bool source_option, const char *source_version,
                     bool target_option, const char *target_version,
                     const char *directory,
                     bool optimize, bool debug,
                     bool verbose, bool null_stderr)
{
  bool err;
  unsigned int argc;
  const char **argv;
  const char **argp;
  int exitstatus;
  unsigned int i;

  argc =
    1 + (nowarn_option != NULL ? 1 : 0) + (source_option ? 2 : 0)
    + (target_option ? 2 : 0) + (optimize ? 1 : 0) + (debug ? 1 : 0)
    + (directory != NULL ? 2 : 0) + java_sources_count;
  argv = (const char **) xmalloca ((argc + 1) * sizeof (const char *));

  argp = argv;
  *argp++ = "javac";
  if (nowarn_option != NULL)
    *argp++ = nowarn_option;
  if (source_option)
    {
      *argp++ = "-source";
      *argp++ = source_version;
    }
  if (target_option)
    {
      *argp++ = "-target";
      *argp++ = target_version;
    }
  if (optimize)
    *argp++ = "-O";
  if (debug)
    *argp++ = "-g";
  if (directory != NULL)
    {
      *argp++ = "-d";
      *argp++ = directory;
    }
  for (i = 0; i < java_sources_count; i++)
    *argp++ = java_sources[i];
  *argp = NULL;
  /* Ensure argv length was correctly calculated.  */
  if (argp - argv != argc)
    abort ();

  if (verbose)
    {
      char *command = shell_quote_argv (argv);
      printf ("%s\n", command);
      free (command);
    }

  exitstatus = execute ("javac", "javac", argv, NULL,
                        false, false, false,
                        null_stderr, true, true, NULL);
  err = (exitstatus != 0);

  freea (argv);

  return err;
}

/* ====================== Usability test subroutines ====================== */

/* Executes a program.
   Returns the first line of its output, as a freshly allocated string, or
   NULL.  */
static char *
execute_and_read_line (const char *progname,
                       const char *prog_path, const char * const *prog_argv)
{
  pid_t child;
  int fd[1];
  FILE *fp;
  char *line;
  size_t linesize;
  size_t linelen;
  int exitstatus;

  /* Open a pipe to the program.  */
  child = create_pipe_in (progname, prog_path, prog_argv, NULL,
                          DEV_NULL, false, true, false, fd);

  if (child == -1)
    return NULL;

  /* Retrieve its result.  */
  fp = fdopen (fd[0], "r");
  if (fp == NULL)
    {
      error (0, errno, _("fdopen() failed"));
      return NULL;
    }

  line = NULL; linesize = 0;
  linelen = getline (&line, &linesize, fp);
  if (linelen == (size_t)(-1))
    {
      error (0, 0, _("%s subprocess I/O error"), progname);
      return NULL;
    }
  if (linelen > 0 && line[linelen - 1] == '\n')
    line[linelen - 1] = '\0';

  /* Read until EOF (otherwise the child process may get a SIGPIPE signal).  */
  while (getc (fp) != EOF)
    ;

  fclose (fp);

  /* Remove zombie process from process list, and retrieve exit status.  */
  exitstatus =
    wait_subprocess (child, progname, true, false, true, false, NULL);
  if (exitstatus != 0)
    {
      free (line);
      return NULL;
    }

  return line;
}

/* Executes a program, assumed to be a Java compiler with '-version' option.
   Returns the version number.  */
static unsigned int
get_compiler_version (const char *progname,
                      const char *prog_path, const char * const *prog_argv)
{
  char *line = execute_and_read_line (progname, prog_path, prog_argv);
  if (line == NULL)
    return 0;

  /* Search the first digit in line.  */
  char *version_start = line;
  for (version_start = line; ; version_start++)
    {
      if (*version_start == '\0')
        {
          /* No digits found.  */
          free (line);
          return 0;
        }
      if (*version_start >= '0' && *version_start <= '9')
        break;
    }

  /* Search the end of the version string.  */
  char *version_end = version_start;
  while ((*version_end >= '0' && *version_end <= '9') || *version_end == '.')
    version_end++;
  *version_end = '\0';

  /* Map 1.6.0_85 to 6, 1.8.0_151 to 8.  Map 9.0.4 to 9, 10.0.2 to 10, etc.  */
  if (version_start[0] == '1' && version_start[1] == '.')
    version_start += 2;
  version_end = strchr (version_start, '.');
  if (version_end != NULL)
    *version_end = '\0';

  /* Convert number to 'unsigned int'.  */
  unsigned int result;
  switch (strlen (version_start))
    {
    case 1:
      result = version_start[0] - '0';
      break;

    case 2:
      result = (version_start[0] - '0') * 10 + (version_start[1] - '0');
      break;

    default:
      result = 0;
    }

  free (line);
  return result;
}

/* Write a given contents to a temporary file.
   FILE_NAME is the name of a file inside TMPDIR that is known not to exist
   yet.
   Return a failure indicator (true upon error).  */
static bool
write_temp_file (struct temp_dir *tmpdir, const char *file_name,
                 const char *contents)
{
  FILE *fp;

  register_temp_file (tmpdir, file_name);
  fp = fopen_temp (file_name, "we", false);
  if (fp == NULL)
    {
      error (0, errno, _("failed to create \"%s\""), file_name);
      unregister_temp_file (tmpdir, file_name);
      return true;
    }
  fputs (contents, fp);
  if (fwriteerror_temp (fp))
    {
      error (0, errno, _("error while writing \"%s\" file"), file_name);
      return true;
    }
  return false;
}

/* Return the class file version number of a class file on disk.  */
static int
get_classfile_version (const char *compiled_file_name)
{
  unsigned char header[8];
  int fd;

  /* Open the class file.  */
  fd = open (compiled_file_name, O_RDONLY | O_BINARY | O_CLOEXEC, 0);
  if (fd >= 0)
    {
      /* Read its first 8 bytes.  */
      if (safe_read (fd, header, 8) == 8)
        {
          /* Verify the class file signature.  */
          if (header[0] == 0xCA && header[1] == 0xFE
              && header[2] == 0xBA && header[3] == 0xBE)
            {
              close (fd);
              return header[7];
            }
        }
      close (fd);
    }

  /* Could not get the class file version.  Return a very large one.  */
  return INT_MAX;
}

/* Test whether $JAVAC can be used, and whether it needs a -source and/or
   -target option, as well as an option to inhibit warnings.
   Return a failure indicator (true upon error).  */
static bool
is_envjavac_usable (const char *javac,
                    const char *source_version, const char *target_version,
                    bool *usablep,
                    char nowarn_option_out[17],
                    char source_option_out[30], char target_option_out[30])
{
  /* The cache depends on the source_version and target_version.  */
  struct result_t
  {
    /*bool*/ unsigned int tested : 1;
    /*bool*/ unsigned int usable : 1;
    /*bool*/ unsigned int nowarn_option : 1;
    unsigned int source_option : 7;
    unsigned int target_option : 7;
  };
  static struct result_t result_cache[SOURCE_VERSION_BOUND][TARGET_VERSION_BOUND];
  struct result_t *resultp;

  resultp = &result_cache[source_version_index (source_version)]
                         [target_version_index (target_version)];
  if (!resultp->tested)
    {
      /* Canonicalize source_version and target_version, for easier
         arithmetic.  */
      int try_source_version = 6 + source_version_index (source_version);
      int try_target_version = 6 + target_version_index (target_version);
      /* Sanity check.  */
      if (try_source_version <= try_target_version)
        {
          /* Try $JAVAC.  */
          struct temp_dir *tmpdir;
          char *conftest_file_name;
          char *compiled_file_name;
          const char *java_sources[1];
          const char *nowarn_option;
          struct stat statbuf;

          tmpdir = create_temp_dir ("java", NULL, false);
          if (tmpdir == NULL)
            return true;

          conftest_file_name =
            xconcatenated_filename (tmpdir->dir_name, "conftest.java", NULL);
          if (write_temp_file (tmpdir, conftest_file_name, "class conftest {}"))
            {
              free (conftest_file_name);
              cleanup_temp_dir (tmpdir);
              return true;
            }

          compiled_file_name =
            xconcatenated_filename (tmpdir->dir_name, "conftest.class", NULL);
          register_temp_file (tmpdir, compiled_file_name);

          /* See the discussion in javacomp.m4.  */
          nowarn_option = " -Xlint:-options";
          char *javac_nowarn = xasprintf ("%s%s", javac, nowarn_option);
          assume (javac_nowarn != NULL);

          java_sources[0] = conftest_file_name;
          if ((!compile_using_envjavac (javac_nowarn,
                                        java_sources, 1, tmpdir->dir_name,
                                        false, false, false, true)
               && stat (compiled_file_name, &statbuf) >= 0)
              || (nowarn_option = "",
                  unlink (compiled_file_name),
                  (!compile_using_envjavac (javac,
                                            java_sources, 1, tmpdir->dir_name,
                                            false, false, false, true)
                   && stat (compiled_file_name, &statbuf) >= 0)))
            {
              /* $JAVAC compiled conftest.java successfully.  */
              int compiler_cfversion =
                get_classfile_version (compiled_file_name);
              int compiler_target_version = compiler_cfversion - 44;

              /* It is hard to determine the compiler_source_version.  This
                 would require a list of code snippets that can be compiled only
                 with a specific '-source' option and up, and this list would
                 need to grow every 6 months.
                 Also, $JAVAC may already include a '-source' option.
                 Therefore, pass a '-source' option always.  */
              char source_option[30];
              sprintf (source_option, " -source %s%d",
                       try_source_version <= 8 ? "1." : "",
                       try_source_version);

              /* And pass a '-target' option as well, if needed.
                 (All supported javac versions support both, see the table in
                 javacomp.m4.)  */
              char target_option[30];
              if (try_target_version == compiler_target_version)
                target_option[0] = '\0';
              else
                sprintf (target_option, " -target %s%d",
                         try_target_version <= 8 ? "1." : "",
                         try_target_version);

              char *javac_source_target =
                xasprintf ("%s%s%s%s", javac, nowarn_option,
                           source_option, target_option);
              assume (javac_source_target != NULL);

              unlink (compiled_file_name);

              java_sources[0] = conftest_file_name;
              if (!compile_using_envjavac (javac_source_target,
                                           java_sources, 1, tmpdir->dir_name,
                                           false, false, false, true)
                  && stat (compiled_file_name, &statbuf) >= 0)
                {
                  /* The compiler directly supports the desired source_version
                     and target_version.  Perfect.  */
                  free (javac_source_target);

                  resultp->nowarn_option = (nowarn_option[0] != '\0');
                  resultp->source_option = try_source_version;
                  resultp->target_option =
                    (try_target_version == compiler_target_version ? 0 :
                     try_target_version);
                  resultp->usable = true;
                }
              else
                {
                  /* If the desired source_version or target_version were too
                     large for the compiler, there's nothing else we can do.  */
                  unsigned int compiler_version;

                  free (javac_source_target);

                  {
                    size_t command_length;
                    char *command;
                    const char *argv[4];

                    command_length = strlen (javac) + 9 + 1;

                    command = (char *) xmalloca (command_length);
                    {
                      char *p = command;
                      p = stpcpy (p, javac);
                      p = stpcpy (p, " -version");
                      *p++ = '\0';
                      /* Ensure command_length was correctly calculated.  */
                      if (p - command > command_length)
                        abort ();
                    }

                    argv[0] = BOURNE_SHELL;
                    argv[1] = "-c";
                    argv[2] = command;
                    argv[3] = NULL;
                    compiler_version =
                      get_compiler_version (javac, BOURNE_SHELL, argv);

                    freea (command);
                  }

                  if (try_source_version <= compiler_version
                      && try_target_version <= compiler_version)
                    {
                      /* Increase try_source_version and compiler_version until
                         the compiler accepts these values.  This is necessary
                         to make e.g. try_source_version = 6 work with Java 12
                         or newer, or try_source_version = 7 work with Java 20
                         or newer.  */
                      for (;;)
                        {
                          /* Invariant:
                             try_source_version <= try_target_version.  */
                          if (try_source_version == try_target_version)
                            try_target_version++;
                          try_source_version++;
                          if (try_source_version > compiler_version)
                            break;

                          sprintf (source_option, " -source %s%d",
                                   try_source_version <= 8 ? "1." : "",
                                   try_source_version);

                          if (try_target_version == compiler_target_version)
                            target_option[0] = '\0';
                          else
                            sprintf (target_option, " -target %s%d",
                                     try_target_version <= 8 ? "1." : "",
                                     try_target_version);

                          javac_source_target =
                            xasprintf ("%s%s%s%s", javac, nowarn_option,
                                       source_option, target_option);
                          assume (javac_source_target != NULL);

                          unlink (compiled_file_name);

                          java_sources[0] = conftest_file_name;
                          if (!compile_using_envjavac (javac_source_target,
                                                       java_sources, 1,
                                                       tmpdir->dir_name,
                                                       false, false,
                                                       false, true)
                              && stat (compiled_file_name, &statbuf) >= 0)
                            {
                              /* The compiler supports the try_source_version
                                 and try_target_version.  It's better than
                                 nothing.  */
                              free (javac_source_target);

                              resultp->nowarn_option = (nowarn_option[0] != '\0');
                              resultp->source_option = try_source_version;
                              resultp->target_option =
                                (try_target_version == compiler_target_version ? 0 :
                                 try_target_version);
                              resultp->usable = true;
                              break;
                            }

                          free (javac_source_target);
                        }
                    }
                }
            }

          cleanup_temp_dir (tmpdir);

          free (javac_nowarn);
          free (compiled_file_name);
          free (conftest_file_name);
        }

      resultp->tested = true;
    }

  *usablep = resultp->usable;
  if (resultp->nowarn_option)
    strcpy (nowarn_option_out, " -Xlint:-options");
  else
    nowarn_option_out[0] = '\0';
  sprintf (source_option_out, " -source %s%d",
           resultp->source_option <= 8 ? "1." : "",
           resultp->source_option);
  if (resultp->target_option == 0)
    target_option_out[0] = '\0';
  else
    sprintf (target_option_out, " -target %s%d",
             resultp->target_option <= 8 ? "1." : "",
             resultp->target_option);
  return false;
}

static bool
is_javac_present (void)
{
  static bool javac_tested;
  static bool javac_present;

  if (!javac_tested)
    {
      /* Test for presence of javac: "javac 2> /dev/null ; test $? -le 2"  */
      const char *argv[2];
      int exitstatus;

      argv[0] = "javac";
      argv[1] = NULL;
      exitstatus = execute ("javac", "javac", argv, NULL,
                            false, false, true, true,
                            true, false, NULL);
      javac_present = (exitstatus == 0 || exitstatus == 1 || exitstatus == 2);
      javac_tested = true;
    }

  return javac_present;
}

/* Test whether javac can be used and whether it needs a -source and/or
   -target option, as well as an option to inhibit warnings.
   Return a failure indicator (true upon error).  */
static bool
is_javac_usable (const char *source_version, const char *target_version,
                 bool *usablep,
                 char nowarn_option_out[17],
                 char source_option_out[20], char target_option_out[20])
{
  /* The cache depends on the source_version and target_version.  */
  struct result_t
  {
    /*bool*/ unsigned int tested : 1;
    /*bool*/ unsigned int usable : 1;
    /*bool*/ unsigned int nowarn_option : 1;
    unsigned int source_option : 7;
    unsigned int target_option : 7;
  };
  static struct result_t result_cache[SOURCE_VERSION_BOUND][TARGET_VERSION_BOUND];
  struct result_t *resultp;

  resultp = &result_cache[source_version_index (source_version)]
                         [target_version_index (target_version)];
  if (!resultp->tested)
    {
      /* Canonicalize source_version and target_version, for easier
         arithmetic.  */
      int try_source_version = 6 + source_version_index (source_version);
      int try_target_version = 6 + target_version_index (target_version);
      /* Sanity check.  */
      if (try_source_version <= try_target_version)
        {
          /* Try javac.  */
          struct temp_dir *tmpdir;
          char *conftest_file_name;
          char *compiled_file_name;
          const char *java_sources[1];
          const char *nowarn_option;
          struct stat statbuf;

          tmpdir = create_temp_dir ("java", NULL, false);
          if (tmpdir == NULL)
            return true;

          conftest_file_name =
            xconcatenated_filename (tmpdir->dir_name, "conftest.java", NULL);
          if (write_temp_file (tmpdir, conftest_file_name, "class conftest {}"))
            {
              free (conftest_file_name);
              cleanup_temp_dir (tmpdir);
              return true;
            }

          compiled_file_name =
            xconcatenated_filename (tmpdir->dir_name, "conftest.class", NULL);
          register_temp_file (tmpdir, compiled_file_name);

          /* See the discussion in javacomp.m4.  */
          nowarn_option = "-Xlint:-options";

          java_sources[0] = conftest_file_name;
          if ((!compile_using_javac (java_sources, 1,
                                     nowarn_option,
                                     false, source_version,
                                     false, target_version,
                                     tmpdir->dir_name,
                                     false, false, false, true)
               && stat (compiled_file_name, &statbuf) >= 0)
              || (nowarn_option = NULL,
                  unlink (compiled_file_name),
                  (!compile_using_javac (java_sources, 1,
                                         nowarn_option,
                                         false, source_version,
                                         false, target_version,
                                         tmpdir->dir_name,
                                         false, false, false, true)
                   && stat (compiled_file_name, &statbuf) >= 0)))
            {
              /* javac compiled conftest.java successfully.  */
              int compiler_cfversion =
                get_classfile_version (compiled_file_name);
              int compiler_target_version = compiler_cfversion - 44;

              /* It is hard to determine the compiler_source_version.  This
                 would require a list of code snippets that can be compiled only
                 with a specific '-source' option and up, and this list would
                 need to grow every 6 months.
                 Also, javac may point to a shell script that already includes a
                 '-source' option.
                 Therefore, pass a '-source' option always.  */
              char source_option[20];
              sprintf (source_option, "%s%d",
                       try_source_version <= 8 ? "1." : "",
                       try_source_version);

              /* And pass a '-target' option as well, if needed.
                 (All supported javac versions support both, see the table in
                 javacomp.m4.)  */
              char target_option[20];
              sprintf (target_option, "%s%d",
                       try_target_version <= 8 ? "1." : "",
                       try_target_version);

              unlink (compiled_file_name);

              java_sources[0] = conftest_file_name;
              if (!compile_using_javac (java_sources, 1,
                                        nowarn_option,
                                        true,
                                        source_option,
                                        try_target_version != compiler_target_version,
                                        target_option,
                                        tmpdir->dir_name,
                                        false, false, false, true)
                  && stat (compiled_file_name, &statbuf) >= 0)
                {
                  /* The compiler directly supports the desired source_version
                     and target_version.  Perfect.  */
                  resultp->nowarn_option = (nowarn_option != NULL);
                  resultp->source_option = try_source_version;
                  resultp->target_option =
                    (try_target_version == compiler_target_version ? 0 :
                     try_target_version);
                  resultp->usable = true;
                }
              else
                {
                  /* If the desired source_version or target_version were too
                     large for the compiler, there's nothing else we can do.  */
                  unsigned int compiler_version;

                  {
                    const char *argv[3];

                    argv[0] = "javac";
                    argv[1] = "-version";
                    argv[2] = NULL;
                    compiler_version =
                      get_compiler_version ("javac", argv[0], argv);
                  }

                  if (try_source_version <= compiler_version
                      && try_target_version <= compiler_version)
                    {
                      /* Increase try_source_version and compiler_version until
                         the compiler accepts these values.  This is necessary
                         to make e.g. try_source_version = 6 work with Java 12
                         or newer, or try_source_version = 7 work with Java 20
                         or newer.  */
                      for (;;)
                        {
                          /* Invariant:
                             try_source_version <= try_target_version.  */
                          if (try_source_version == try_target_version)
                            try_target_version++;
                          try_source_version++;
                          if (try_source_version > compiler_version)
                            break;

                          sprintf (source_option, "%s%d",
                                   try_source_version <= 8 ? "1." : "",
                                   try_source_version);

                          sprintf (target_option, "%s%d",
                                   try_target_version <= 8 ? "1." : "",
                                   try_target_version);

                          unlink (compiled_file_name);

                          java_sources[0] = conftest_file_name;
                          if (!compile_using_javac (java_sources, 1,
                                                    nowarn_option,
                                                    true,
                                                    source_option,
                                                    try_target_version != compiler_target_version,
                                                    target_option,
                                                    tmpdir->dir_name,
                                                    false, false, false, true)
                              && stat (compiled_file_name, &statbuf) >= 0)
                            {
                              /* The compiler supports the try_source_version
                                 and try_target_version.  It's better than
                                 nothing.  */
                              resultp->nowarn_option = (nowarn_option != NULL);
                              resultp->source_option = try_source_version;
                              resultp->target_option =
                                (try_target_version == compiler_target_version ? 0 :
                                 try_target_version);
                              resultp->usable = true;
                              break;
                            }
                        }
                    }
                }
            }

          cleanup_temp_dir (tmpdir);

          free (compiled_file_name);
          free (conftest_file_name);
        }

      resultp->tested = true;
    }

  *usablep = resultp->usable;
  if (resultp->nowarn_option)
    strcpy (nowarn_option_out, "-Xlint:-options");
  else
    nowarn_option_out[0] = '\0';
  sprintf (source_option_out, "%s%d",
           resultp->source_option <= 8 ? "1." : "",
           resultp->source_option);
  if (resultp->target_option == 0)
    target_option_out[0] = '\0';
  else
    sprintf (target_option_out, "%s%d",
             resultp->target_option <= 8 ? "1." : "",
             resultp->target_option);
  return false;
}

/* ============================= Main function ============================= */

bool
compile_java_class (const char * const *java_sources,
                    unsigned int java_sources_count,
                    const char * const *classpaths,
                    unsigned int classpaths_count,
                    const char *source_version,
                    const char *target_version,
                    const char *directory,
                    bool optimize, bool debug,
                    bool use_minimal_classpath,
                    bool verbose)
{
  bool err = false;
  char *old_JAVA_HOME;

  /* Map source_version 1.1 ... 1.5 to 1.6.  */
  if (source_version[0] == '1' && source_version[1] == '.'
      && (source_version[2] >= '1' && source_version[2] <= '5')
      && source_version[3] == '\0')
    source_version = "1.6";

  /* Map target_version 1.1 ... 1.5 to 1.6.  */
  if (target_version != NULL
      && target_version[0] == '1' && target_version[1] == '.'
      && (target_version[2] >= '1' && target_version[2] <= '5')
      && target_version[3] == '\0')
    target_version = "1.6";

  {
    const char *javac = getenv ("JAVAC");
    if (javac != NULL && javac[0] != '\0')
      {
        bool usable = false;
        char nowarn_option[17];
        char source_option[30];
        char target_option[30];

        if (target_version == NULL)
          target_version = default_target_version ();

        if (is_envjavac_usable (javac,
                                source_version, target_version,
                                &usable,
                                nowarn_option,
                                source_option, target_option))
          {
            err = true;
            goto done1;
          }

        if (usable)
          {
            char *old_classpath;
            char *javac_with_options;

            /* Set CLASSPATH.  */
            old_classpath =
              set_classpath (classpaths, classpaths_count, false, verbose);

            javac_with_options =
              xasprintf ("%s%s%s%s", javac,
                         nowarn_option, source_option, target_option);
            assume (javac_with_options != NULL);

            err = compile_using_envjavac (javac_with_options,
                                          java_sources, java_sources_count,
                                          directory, optimize, debug, verbose,
                                          false);

            free (javac_with_options);

            /* Reset CLASSPATH.  */
            reset_classpath (old_classpath);

            goto done1;
          }
      }
  }

  /* Unset the JAVA_HOME environment variable.  */
  old_JAVA_HOME = getenv ("JAVA_HOME");
  if (old_JAVA_HOME != NULL)
    {
      old_JAVA_HOME = xstrdup (old_JAVA_HOME);
      unsetenv ("JAVA_HOME");
    }

  if (is_javac_present ())
    {
      bool usable = false;
      char nowarn_option[17];
      char source_option[20];
      char target_option[20];

      if (target_version == NULL)
        target_version = default_target_version ();

      if (is_javac_usable (source_version, target_version,
                           &usable,
                           nowarn_option,
                           source_option, target_option))
        {
          err = true;
          goto done1;
        }

      if (usable)
        {
          char *old_classpath;

          /* Set CLASSPATH.  We don't use the "-classpath ..." option because
             in JDK 1.1.x its argument should also contain the JDK's
             classes.zip, but we don't know its location.  (In JDK 1.3.0 it
             would work.)  */
          old_classpath =
            set_classpath (classpaths, classpaths_count, use_minimal_classpath,
                           verbose);

          err = compile_using_javac (java_sources, java_sources_count,
                                     nowarn_option[0] != '\0' ? nowarn_option : NULL,
                                     true, source_option,
                                     target_option[0] != '\0', target_option,
                                     directory, optimize, debug, verbose,
                                     false);

          /* Reset CLASSPATH.  */
          reset_classpath (old_classpath);

          goto done2;
        }
    }

  error (0, 0, _("Java compiler not found, try setting $JAVAC"));
  err = true;

 done2:
  if (old_JAVA_HOME != NULL)
    {
      xsetenv ("JAVA_HOME", old_JAVA_HOME, 1);
      free (old_JAVA_HOME);
    }

 done1:
  return err;
}
