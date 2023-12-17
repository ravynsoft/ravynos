/* Execute a Java program.
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
#include "javaexec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execute.h"
#include "classpath.h"
#include "xsetenv.h"
#include "sh-quote.h"
#include "concat-filename.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "error.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Survey of Java virtual machines.

   A = does it work without CLASSPATH being set
   B = does it work with CLASSPATH being set to empty
   C = option to set CLASSPATH, other than setting it in the environment
   T = test for presence

   Program    from         A B  C              T

   $JAVA      unknown      N Y  n/a            true
   java       JDK 1.1.8    Y Y  -classpath P   java -version 2>/dev/null
   jre        JDK 1.1.8    N Y  -classpath P   jre 2>/dev/null; test $? = 1
   java       JDK 1.3.0    Y Y  -classpath P   java -version 2>/dev/null

   The CLASSPATH is a colon separated list of pathnames. (On Windows: a
   semicolon separated list of pathnames.)

   We try the Java virtual machines in the following order:
     1. getenv ("JAVA"), because the user must be able to override our
        preferences,
     2. "java", because it is a standard JVM,
     3. "jre", comes last because it requires a CLASSPATH environment variable.

   We unset the JAVA_HOME environment variable, because a wrong setting of
   this variable can confuse the JDK's javac.
 */

bool
execute_java_class (const char *class_name,
                    const char * const *classpaths,
                    unsigned int classpaths_count,
                    bool use_minimal_classpath,
                    const char *exe_dir,
                    const char * const *args,
                    bool verbose, bool quiet,
                    execute_fn *executer, void *private_data)
{
  bool err = false;
  unsigned int nargs;
  char *old_JAVA_HOME;

  /* Count args.  */
  {
    const char * const *arg;

    for (nargs = 0, arg = args; *arg != NULL; nargs++, arg++)
     ;
  }

  /* First, try a class compiled to a native code executable.  */
  if (exe_dir != NULL)
    {
      char *exe_pathname = xconcatenated_filename (exe_dir, class_name, EXEEXT);
      char *old_classpath;
      const char **argv =
        (const char **) xmalloca ((1 + nargs + 1) * sizeof (const char *));
      unsigned int i;

      /* Set CLASSPATH.  */
      old_classpath =
        set_classpath (classpaths, classpaths_count, use_minimal_classpath,
                       verbose);

      argv[0] = exe_pathname;
      for (i = 0; i <= nargs; i++)
        argv[1 + i] = args[i];

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          printf ("%s\n", command);
          free (command);
        }

      err = executer (class_name, exe_pathname, argv, private_data);

      /* Reset CLASSPATH.  */
      reset_classpath (old_classpath);

      freea (argv);

      goto done1;
    }

  {
    const char *java = getenv ("JAVA");
    if (java != NULL && java[0] != '\0')
      {
        /* Because $JAVA may consist of a command and options, we use the
           shell.  Because $JAVA has been set by the user, we leave all
           all environment variables in place, including JAVA_HOME, and
           we don't erase the user's CLASSPATH.  */
        char *old_classpath;
        unsigned int command_length;
        char *command;
        const char *argv[4];
        const char * const *arg;
        char *p;

        /* Set CLASSPATH.  */
        old_classpath =
          set_classpath (classpaths, classpaths_count, false,
                         verbose);

        command_length = strlen (java);
        command_length += 1 + shell_quote_length (class_name);
        for (arg = args; *arg != NULL; arg++)
          command_length += 1 + shell_quote_length (*arg);
        command_length += 1;

        command = (char *) xmalloca (command_length);
        p = command;
        /* Don't shell_quote $JAVA, because it may consist of a command
           and options.  */
        memcpy (p, java, strlen (java));
        p += strlen (java);
        *p++ = ' ';
        p = shell_quote_copy (p, class_name);
        for (arg = args; *arg != NULL; arg++)
          {
            *p++ = ' ';
            p = shell_quote_copy (p, *arg);
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
        err = executer (java, BOURNE_SHELL, argv, private_data);

        freea (command);

        /* Reset CLASSPATH.  */
        reset_classpath (old_classpath);

        goto done1;
      }
  }

  /* Unset the JAVA_HOME environment variable.  */
  old_JAVA_HOME = getenv ("JAVA_HOME");
  if (old_JAVA_HOME != NULL)
    {
      old_JAVA_HOME = xstrdup (old_JAVA_HOME);
      unsetenv ("JAVA_HOME");
    }

  {
    static bool java_tested;
    static bool java_present;

    if (!java_tested)
      {
        /* Test for presence of java: "java -version 2> /dev/null"  */
        const char *argv[3];
        int exitstatus;

        argv[0] = "java";
        argv[1] = "-version";
        argv[2] = NULL;
        exitstatus = execute ("java", "java", argv, NULL,
                              false, false, true, true,
                              true, false, NULL);
        java_present = (exitstatus == 0);
        java_tested = true;
      }

    if (java_present)
      {
        char *old_classpath;
        const char **argv =
          (const char **) xmalloca ((2 + nargs + 1) * sizeof (const char *));
        unsigned int i;

        /* Set CLASSPATH.  We don't use the "-classpath ..." option because
           in JDK 1.1.x its argument should also contain the JDK's classes.zip,
           but we don't know its location.  (In JDK 1.3.0 it would work.)  */
        old_classpath =
          set_classpath (classpaths, classpaths_count, use_minimal_classpath,
                         verbose);

        argv[0] = "java";
        argv[1] = class_name;
        for (i = 0; i <= nargs; i++)
          argv[2 + i] = args[i];

        if (verbose)
          {
            char *command = shell_quote_argv (argv);
            printf ("%s\n", command);
            free (command);
          }

        err = executer ("java", "java", argv, private_data);

        /* Reset CLASSPATH.  */
        reset_classpath (old_classpath);

        freea (argv);

        goto done2;
      }
  }

  {
    static bool jre_tested;
    static bool jre_present;

    if (!jre_tested)
      {
        /* Test for presence of jre: "jre 2> /dev/null ; test $? = 1"  */
        const char *argv[2];
        int exitstatus;

        argv[0] = "jre";
        argv[1] = NULL;
        exitstatus = execute ("jre", "jre", argv, NULL,
                              false, false, true, true,
                              true, false, NULL);
        jre_present = (exitstatus == 0 || exitstatus == 1);
        jre_tested = true;
      }

    if (jre_present)
      {
        char *old_classpath;
        const char **argv =
          (const char **) xmalloca ((2 + nargs + 1) * sizeof (const char *));
        unsigned int i;

        /* Set CLASSPATH.  We don't use the "-classpath ..." option because
           in JDK 1.1.x its argument should also contain the JDK's classes.zip,
           but we don't know its location.  */
        old_classpath =
          set_classpath (classpaths, classpaths_count, use_minimal_classpath,
                         verbose);

        argv[0] = "jre";
        argv[1] = class_name;
        for (i = 0; i <= nargs; i++)
          argv[2 + i] = args[i];

        if (verbose)
          {
            char *command = shell_quote_argv (argv);
            printf ("%s\n", command);
            free (command);
          }

        err = executer ("jre", "jre", argv, private_data);

        /* Reset CLASSPATH.  */
        reset_classpath (old_classpath);

        freea (argv);

        goto done2;
      }
  }

  if (!quiet)
    error (0, 0, _("Java virtual machine not found, try setting $JAVA"));
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
