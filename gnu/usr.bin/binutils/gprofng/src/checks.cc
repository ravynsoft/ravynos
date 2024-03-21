/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <ctype.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/param.h>

#include "gp-defs.h"
#include "Elf.h"
#include "collctrl.h"
#include "i18n.h"
#include "util.h"
#include "collect.h"

void
collect::check_target (int argc, char **argv)
{
  char *next;
  char *last = 0;
  char *a;
  char *ccret;
  char **lasts = &last;
  int tindex = targ_index;
  int ret;
  char *basename;
  is_64 = false;

  /* now check the executable */
  nargs = argc - targ_index;
  Exec_status rv = check_executable (argv[targ_index]);
  switch (rv)
    {
    case EXEC_OK:
      njargs = cc->get_java_arg_cnt ();
      arglist = (char **) calloc (nargs + 5 + njargs, sizeof (char *));
      jargs = cc->get_java_args ();

      // store the first argument -- target name
      ret = 0;
      arglist[ret++] = argv[tindex++];
      if (cc->get_java_mode () == 1)
	{
	  // add any user-specified -J (Java) arguments
	  int length = (int) strlen (argv[targ_index]);
	  int is_java = 0;
	  if ((length >= 6) && strcmp (&argv[targ_index][length - 5], NTXT ("/java")) == 0)
	    is_java = 1;
	  else if ((length == 4) && strcmp (&argv[targ_index][0], NTXT ("java")) == 0)
	    is_java = 1;
	  if (njargs != 0 && is_java)
	    {
	      next = strtok_r (jargs, NTXT (" \t"), lasts);
	      arglist[ret++] = next;
	      for (;;)
		{
		  next = strtok_r (NULL, NTXT (" \t"), lasts);
		  if (next == NULL)
		    break;
		  arglist[ret++] = next;
		}
	    }
	}

      // copy the rest of the arguments
      for (int i = 1; i < nargs; i++)
	arglist[ret++] = argv[tindex++];
      nargs = ret;
      break;
    case EXEC_IS_JAR:
      // Preface the user-supplied argument list with
      //	the path to the java, the collector invocation,
      //	any -J java arguments provided, and "-jar".
      ccret = cc->set_java_mode (NTXT ("on"));
      if (ccret != NULL)
	{
	  writeStr (2, ccret);
	  exit (1);
	}
      njargs = cc->get_java_arg_cnt ();
      arglist = (char **) calloc (nargs + 5 + njargs, sizeof (char *));
      jargs = cc->get_java_args ();

      a = find_java ();
      if (a == NULL)
	exit (1); // message was written
      ret = 0;
      arglist[ret++] = a;
      // add any user-specified Java arguments
      if (njargs != 0)
	{
	  next = strtok_r (jargs, NTXT (" \t"), lasts);
	  arglist[ret++] = next;
	  for (;;)
	    {
	      next = strtok_r (NULL, NTXT (" \t"), lasts);
	      if (next == NULL)
		break;
	      arglist[ret++] = next;
	    }
	}
      arglist[ret++] = NTXT ("-jar");
      for (int i = 0; i < nargs; i++)
	arglist[ret++] = argv[tindex++];
      nargs = ret;
      break;
    case EXEC_IS_CLASSCLASS:
      // remove the .class from the name
      ret = (int) strlen (argv[targ_index]);
      argv[targ_index][ret - 6] = 0;

      // now fall through to the EXEC_IS_CLASS case
    case EXEC_IS_CLASS:
      // Preface the user-supplied argument list with
      //	the path to the java, the collector invocation,
      //	and any -J java arguments provided.
      ccret = cc->set_java_mode (NTXT ("on"));
      if (ccret != NULL)
	{
	  writeStr (2, ccret);
	  exit (1);
	}
      jargs = cc->get_java_args ();
      njargs = cc->get_java_arg_cnt ();
      arglist = (char **) calloc (nargs + 4 + njargs, sizeof (char *));

      a = find_java ();
      if (a == NULL)
	exit (1); // message was written
      ret = 0;
      arglist[ret++] = a;
      // add any user-specified Java arguments
      if (njargs != 0)
	{
	  next = strtok_r (jargs, NTXT (" \t"), lasts);
	  arglist[ret++] = next;
	  for (;;)
	    {
	      next = strtok_r (NULL, NTXT (" \t"), lasts);
	      if (next == NULL)
		break;
	      arglist[ret++] = next;
	    }
	}

      // copy the remaining arguments to the new list
      for (int i = 0; i < nargs; i++)
	arglist[ret++] = argv[tindex++];
      nargs = ret;
      break;
    case EXEC_ELF_NOSHARE:
    case EXEC_OPEN_FAIL:
    case EXEC_ELF_LIB:
    case EXEC_ELF_HEADER:
    case EXEC_ELF_ARCH:
    case EXEC_ISDIR:
    case EXEC_NOT_EXEC:
    case EXEC_NOT_FOUND:
    default:
      /* something wrong; write a message */
      char *errstr = status_str (rv, argv[targ_index]);
      if (errstr)
	{
	  dbe_write (2, "%s", errstr);
	  free (errstr);
	}
      exit (1);
    }
  cc->set_target (arglist[0]);

  /* check the experiment */
  char *ccwarn;
  ccret = cc->check_expt (&ccwarn);
  if (ccwarn != NULL)
    {
      writeStr (2, ccwarn);
      free (ccwarn);
    }
  if (ccret != NULL)
    {
      writeStr (2, ccret);
      exit (1);
    }
  /* check if java, to see if -j flag was given */
  if ((basename = strrchr (arglist[0], '/')) == NULL)
    basename = arglist[0];
  else
    basename++;
  if (strcmp (basename, NTXT ("java")) == 0)
    {
      /* the target's name is java; was java flag set? */
      if ((jseen_global == 0) && (cc->get_java_mode () == 0))
	{
	  char *cret = cc->set_java_mode (NTXT ("on"));
	  if (cret != NULL)
	    {
	      writeStr (2, cret);
	      exit (1);
	    }
	}
    }
}

collect::Exec_status
collect::check_executable (char *target_name)
{
  char target_path[MAXPATHLEN];
  struct stat64 statbuf;
  if (target_name == NULL) // not set, but assume caller knows what it's doing
    return EXEC_OK;
  if (getenv ("GPROFNG_SKIP_VALIDATION")) // don't check target
    return EXEC_OK;

  // see if target exists and is not a directory
  if ((dbe_stat (target_name, &statbuf) == 0) && ((statbuf.st_mode & S_IFMT) != S_IFDIR))
    {
      // target is found, check for access as executable
      if (access (target_name, X_OK) != 0)
	{
	  // not an executable, check for jar or class file
	  int i = (int) strlen (target_name);
	  if ((i >= 5) && strcmp (&target_name[i - 4], NTXT (".jar")) == 0)
	    {
	      // could be a jar file
	      // XXXX -- need better check for real jar file
	      cc->set_java_mode ("on");
	      return EXEC_IS_JAR;
	    }
	  if ((i >= 7) && strcmp (&target_name[i - 6], NTXT (".class")) == 0)
	    {
	      // could be a class file
	      // XXXX -- need better check for real class file
	      cc->set_java_mode (NTXT ("on"));
	      return EXEC_IS_CLASSCLASS;
	    }
	  // not a jar or class file, return not an executable
	  return EXEC_NOT_EXEC;
	}
      else  // found, and it is executable. set the path to it
	snprintf (target_path, sizeof (target_path), NTXT ("%s"), target_name);
    }
  else
    {
      // not found, look on path
      char *exe_name = get_realpath (target_name);
      if (access (exe_name, X_OK) == 0)
	{
	  // target can't be located
	  // one last attempt: append .class to name, and see if we can find it
	  snprintf (target_path, sizeof (target_path), NTXT ("%s.class"), target_name);
	  if (dbe_stat (target_path, &statbuf) == 0)
	    {
	      // the file exists
	      if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
		{
		  // this is a directory; that won't do.
		  return EXEC_ISDIR;
		}
	      // say it's a class file
	      cc->set_java_mode (NTXT ("on"));
	      return EXEC_IS_CLASS;
	    }
	  return EXEC_NOT_FOUND;
	}
      snprintf (target_path, sizeof (target_path), NTXT ("%s"), exe_name);
      delete exe_name;
    }

  // target_path is now the purported executable
  // check for ELF library out of date
  if (Elf::elf_version (EV_CURRENT) == EV_NONE)
    return EXEC_ELF_LIB;
  Elf *elf = Elf::elf_begin (target_path);
  if (elf == NULL)
    return EXEC_OK;
  // do not by pass checking architectural match
  collect::Exec_status exec_stat = check_executable_arch (elf);
  if (exec_stat != EXEC_OK)
    {
      delete elf;
      return exec_stat;
    }
  delete elf;
  return EXEC_OK;
}

collect::Exec_status
collect::check_executable_arch (Elf *elf)
{
  Elf_Internal_Ehdr *ehdrp = elf->elf_getehdr ();
  if (ehdrp == NULL)
    return EXEC_ELF_HEADER;
  unsigned short machine = ehdrp->e_machine;

  switch (machine)
    {
#if ARCH(SPARC)
    case EM_SPARC:
    case EM_SPARC32PLUS:
      break;
    case EM_SPARCV9:
      is_64 = true;
      break;
#elif ARCH(Intel)
    case EM_X86_64:
      {
	is_64 = true;
	// now figure out if the platform can run it
	struct utsname unbuf;
	int r = uname (&unbuf);
	if (r == 0 && strstr (unbuf.machine, "_64") == NULL)
	  // machine can not run 64 bits, but this code is 64-bit
	  return EXEC_ELF_ARCH;
      }
      break;
    case EM_386:
      break;
#elif ARCH(Aarch64)
    case EM_AARCH64:
      is_64 = true;
      break;
#endif
    default:
      return EXEC_ELF_ARCH;
    }

  // now check if target was built with shared libraries
  int dynamic = 0;
  for (unsigned cnt = 0; cnt < ehdrp->e_phnum; cnt++)
    {
      Elf_Internal_Phdr *phdrp = elf->get_phdr (cnt);
      if (phdrp && phdrp->p_type == PT_DYNAMIC)
	{
	  dynamic = 1;
	  break;
	}
    }
  if (dynamic == 0)
    {
      // target is not a dynamic executable or shared object;
      // can't record data
      return EXEC_ELF_NOSHARE;
    }
  return EXEC_OK;
}

char *
collect::status_str (Exec_status rv, char *target_name)
{
  switch (rv)
    {
    case EXEC_OK:
    case EXEC_IS_JAR:
    case EXEC_IS_CLASS:
    case EXEC_IS_CLASSCLASS:
      // supported flavors -- no error message
      return NULL;
    case EXEC_ELF_NOSHARE:
      return dbe_sprintf (GTXT ("Target executable `%s' must be built with shared libraries\n"), target_name);
    case EXEC_OPEN_FAIL:
      return dbe_sprintf (GTXT ("Can't open target executable `%s'\n"), target_name);
    case EXEC_ELF_LIB:
      return strdup (GTXT ("Internal error: Not a working version of ELF library\n"));
    case EXEC_ELF_HEADER:
      return dbe_sprintf (GTXT ("Target `%s' is not a valid ELF executable\n"), target_name);
    case EXEC_ELF_ARCH:
      return dbe_sprintf (GTXT ("Target architecture of executable `%s' is not supported on this machine\n"), target_name);
    case EXEC_ISDIR:
      return dbe_sprintf (GTXT ("Target `%s' is a directory, not an executable\n"), target_name);
    case EXEC_NOT_EXEC:
      return dbe_sprintf (GTXT ("Target `%s' is not executable\n"), target_name);
    case EXEC_NOT_FOUND:
      return dbe_sprintf (GTXT ("Target `%s' not found\n"), target_name);
    }
  return NULL;
}

char *
collect::find_java (void)
{
  char buf[MAXPATHLEN];
  char *var = NULL;
  Exec_status rv = EXEC_OK;

  // first see if the user entered a -j argument
  var = cc->get_java_path ();
  if (var != NULL)
    {
      snprintf (buf, sizeof (buf), NTXT ("%s/bin/java"), var);
      java_how = NTXT ("-j");
      rv = check_executable (buf);
    }
  // then try JDK_HOME
  if (java_how == NULL)
    {
      var = getenv (NTXT ("JDK_HOME"));
      if ((var != NULL) && (strlen (var) > 0))
	{
	  snprintf (buf, sizeof (buf), NTXT ("%s/bin/java"), var);
	  java_how = NTXT ("JDK_HOME");
	  rv = check_executable (buf);
	}
    }
  // then try JAVA_PATH
  if (java_how == NULL)
    {
      var = getenv (NTXT ("JAVA_PATH"));
      if ((var != NULL) && (strlen (var) > 0))
	{
	  snprintf (buf, sizeof (buf), NTXT ("%s/bin/java"), var);
	  java_how = NTXT ("JAVA_PATH");
	  rv = check_executable (buf);
	}
    }
  // try the user's path
  if (java_how == NULL)
    {
      snprintf (buf, sizeof (buf), NTXT ("java"));
      rv = check_executable (buf);
      if (rv == EXEC_OK)
	java_how = NTXT ("PATH");
    }
  // finally, just try /usr/java -- system default
  if (java_how == NULL)
    {
      snprintf (buf, sizeof (buf), NTXT ("/usr/java/bin/java"));
      rv = check_executable (buf);
      java_how = NTXT ("/usr/java/bin/java");
    }

  // we now have a nominal path to java, and how we chose it
  // and we have rv set to the check_executable return
  switch (rv)
    {
    case EXEC_OK:
      java_path = strdup (buf);
      if (verbose == 1)
	dbe_write (2, GTXT ("Path to `%s' (set from %s) used for Java profiling\n"),
		   java_path, java_how);
      return ( strdup (buf));
    default:
      dbe_write (2, GTXT ("Path to `%s' (set from %s) does not point to a JVM executable\n"),
		 buf, java_how);
      break;
    }
  return NULL;
}

void
collect::validate_config (int how)
{
  if (getenv (NTXT ("GPROFNG_SKIP_VALIDATION")) != NULL)
    return;
  char *cmd = dbe_sprintf (NTXT ("%s/perftools_validate"), run_dir);
  if (access (cmd, X_OK) != 0)
    {
      if (how)
	dbe_write (2, GTXT ("WARNING: Unable to validate system: `%s' could not be executed\n"), cmd);
      return;
    }
  char *quiet = how == 0 ? NTXT ("") : NTXT ("-q"); // check collection, verbosely
  char *buf;
  if (cc->get_java_default () == 0 && java_path)
    buf = dbe_sprintf (NTXT ("%s -c -j %s -H \"%s\" %s"), cmd, java_path, java_how, quiet);
  else  // not java mode -- don't check the java version
    buf = dbe_sprintf (NTXT ("%s -c %s"), cmd, quiet);
  free (cmd);

  /* now run the command */
  int ret = system (buf);
  int status = WEXITSTATUS (ret);
  if ((status & 0x1) != 0)
    dbe_write (2, GTXT ("WARNING: Data collection may fail: system is not properly configured or is unsupported.\n"));
  if ((status & 0x2) != 0)
    dbe_write (2, GTXT ("WARNING: Java data collection may fail: J2SE[tm] version is unsupported.\n"));
  free (buf);
}

void
collect::validate_java (const char *jvm, const char *jhow, int q)
{
  char *cmd = dbe_sprintf (NTXT ("%s/perftools_ckjava"), run_dir);
  if (access (cmd, X_OK) != 0)
    {
      dbe_write (2, GTXT ("WARNING: Unable to validate Java: `%s' could not be executed\n"), cmd);
      return;
    }
  char *buf = dbe_sprintf (NTXT ("%s -j %s -H \"%s\" %s"), cmd, jvm, jhow,
			   (q == 1 ? "-q" : ""));
  free (cmd);

  /* now run the command */
  int ret = system (buf);
  int status = WEXITSTATUS (ret);
  if (status != 0)
    dbe_write (2, GTXT ("WARNING: Java data collection may fail: J2SE[tm] version is unsupported.\n"));
  free (buf);
}
