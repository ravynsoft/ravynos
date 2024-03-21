/* Provide relocatable programs.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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


#define _GL_USE_STDLIB_ALLOC 1
#include <config.h>

/* Specification.  */
#include "progname.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/* Get declaration of _NSGetExecutablePath on Mac OS X 10.2 or newer.  */
#if HAVE_MACH_O_DYLD_H
# include <mach-o/dyld.h>
#endif

#if defined _WIN32 && !defined __CYGWIN__
# define WINDOWS_NATIVE
#endif

#ifdef WINDOWS_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#ifdef __EMX__
# define INCL_DOS
# include <os2.h>
#endif

#include "relocatable.h"

#ifdef NO_XMALLOC
# include "areadlink.h"
# define xreadlink areadlink
#else
# include "xreadlink.h"
#endif

#ifdef NO_XMALLOC
# define xmalloc malloc
# define xstrdup strdup
#else
# include "xalloc.h"
#endif

#ifndef O_EXEC
# define O_EXEC O_RDONLY /* This is often close enough in older systems.  */
#endif

#if defined IN_RELOCWRAPPER && (!defined O_CLOEXEC || GNULIB_defined_O_CLOEXEC)
# undef O_CLOEXEC
# define O_CLOEXEC 0
#endif

/* Declare canonicalize_file_name.
   The <stdlib.h> included above may be the system's one, not the gnulib
   one.  */
extern char * canonicalize_file_name (const char *name);

#if defined WINDOWS_NATIVE
/* Don't assume that UNICODE is not defined.  */
# undef GetModuleFileName
# define GetModuleFileName GetModuleFileNameA
#endif

/* Pathname support.
   ISSLASH(C)                tests whether C is a directory separator character.
   IS_FILE_NAME_WITH_DIR(P)  tests whether P contains a directory specification.
 */
#if (defined _WIN32 && !defined __CYGWIN__) || defined __EMX__ || defined __DJGPP__
  /* Native Windows, OS/2, DOS */
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
# define HAS_DEVICE(P) \
    ((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) \
     && (P)[1] == ':')
# define IS_FILE_NAME_WITH_DIR(P) \
    (strchr (P, '/') != NULL || strchr (P, '\\') != NULL || HAS_DEVICE (P))
# define FILE_SYSTEM_PREFIX_LEN(P) (HAS_DEVICE (P) ? 2 : 0)
#else
  /* Unix */
# define ISSLASH(C) ((C) == '/')
# define IS_FILE_NAME_WITH_DIR(P) (strchr (P, '/') != NULL)
# define FILE_SYSTEM_PREFIX_LEN(P) 0
#endif

/* Use the system functions, not the gnulib overrides in this file.  */
#undef sprintf

#undef set_program_name


#if ENABLE_RELOCATABLE

#ifdef __sun

/* Helper function, from gnulib module 'safe-read'.  */
static size_t
safe_read (int fd, void *buf, size_t count)
{
  for (;;)
    {
      ssize_t result = read (fd, buf, count);

      if (0 <= result || errno != EINTR)
        return result;
    }
}

/* Helper function, from gnulib module 'full-read'.  */
static size_t
full_read (int fd, void *buf, size_t count)
{
  size_t total = 0;
  char *ptr = (char *) buf;

  while (count > 0)
    {
      size_t n = safe_read (fd, ptr, count);
      if (n == (size_t) -1)
        break;
      if (n == 0)
        {
          errno = 0;
          break;
        }
      total += n;
      ptr += n;
      count -= n;
    }

  return total;
}

#endif

#if defined __linux__ || defined __CYGWIN__
/* File descriptor of the executable.
   (Only used to verify that we find the correct executable.)  */
static int executable_fd = -1;
#endif

/* Define this function only when it's needed.  */
#if !(defined WINDOWS_NATIVE || defined __EMX__)

/* Tests whether a given filename may belong to the executable.  */
static bool
maybe_executable (const char *filename)
{
  /* The native Windows API lacks the access() function.  */
# if !defined WINDOWS_NATIVE
  if (access (filename, X_OK) < 0)
    return false;
# endif

# if defined __linux__ || defined __CYGWIN__
  if (executable_fd >= 0)
    {
      /* If we already have an executable_fd, check that filename points to
         the same inode.  */
      struct stat statexe;
      struct stat statfile;

      if (fstat (executable_fd, &statexe) >= 0)
        return (stat (filename, &statfile) >= 0
                && statfile.st_dev
                && statfile.st_dev == statexe.st_dev
                && statfile.st_ino == statexe.st_ino);
    }
# endif

  /* Check that the filename does not point to a directory.  */
  {
    struct stat statfile;

    return (stat (filename, &statfile) >= 0
            && ! S_ISDIR (statfile.st_mode));
  }
}

#endif

/* Determine the full pathname of the current executable, freshly allocated.
   Return NULL if unknown.
   Guaranteed to work on Linux and native Windows.  Likely to work on the
   other Unixes (maybe except BeOS), under most conditions.  */
static char *
find_executable (const char *argv0)
{
#if defined WINDOWS_NATIVE
  /* Native Windows only.
     On Cygwin, it is better to use the Cygwin provided /proc interface, than
     to use native Windows API and cygwin_conv_to_posix_path, because it
     supports longer file names
     (see <https://cygwin.com/ml/cygwin/2011-01/msg00410.html>).  */
  char location[MAX_PATH];
  int length = GetModuleFileName (NULL, location, sizeof (location));
  if (length < 0)
    return NULL;
  if (!IS_FILE_NAME_WITH_DIR (location))
    /* Shouldn't happen.  */
    return NULL;
  return xstrdup (location);
#elif defined __EMX__
  PPIB ppib;
  char location[CCHMAXPATH];

  /* See http://cyberkinetica.homeunix.net/os2tk45/cp1/619_L2H_DosGetInfoBlocksSynt.html
     for specification of DosGetInfoBlocks().  */
  if (DosGetInfoBlocks (NULL, &ppib))
    return NULL;

  /* See http://cyberkinetica.homeunix.net/os2tk45/cp1/1247_L2H_DosQueryModuleNameSy.html
     for specification of DosQueryModuleName().  */
  if (DosQueryModuleName (ppib->pib_hmte, sizeof (location), location))
    return NULL;

  _fnslashify (location);

  return xstrdup (location);
#else /* Unix */
# if defined __linux__
  /* The executable is accessible as /proc/<pid>/exe.  In newer Linux
     versions, also as /proc/self/exe.  Linux >= 2.1 provides a symlink
     to the true pathname; older Linux versions give only device and ino,
     enclosed in brackets, which we cannot use here.  */
  {
    char *link;

    link = xreadlink ("/proc/self/exe");
    if (link != NULL && link[0] != '[')
      return link;
    if (executable_fd < 0)
      executable_fd = open ("/proc/self/exe", O_EXEC | O_CLOEXEC, 0);

    {
      char buf[6+10+5];
      sprintf (buf, "/proc/%d/exe", getpid ());
      link = xreadlink (buf);
      if (link != NULL && link[0] != '[')
        return link;
      if (executable_fd < 0)
        executable_fd = open (buf, O_EXEC | O_CLOEXEC, 0);
    }
  }
# endif
# if defined __ANDROID__ || defined __FreeBSD_kernel__
  /* On Android and GNU/kFreeBSD, the executable is accessible as
     /proc/<pid>/exe and /proc/self/exe.  */
  {
    char *link;

    link = xreadlink ("/proc/self/exe");
    if (link != NULL)
      return link;
  }
# endif
# if defined __FreeBSD__ || defined __DragonFly__
  /* In FreeBSD >= 5.0, the executable is accessible as /proc/<pid>/file and
     /proc/curproc/file.  */
  {
    char *link;

    link = xreadlink ("/proc/curproc/file");
    if (link != NULL)
      {
        if (strcmp (link, "unknown") != 0)
          return link;
        free (link);
      }
  }
# endif
# if defined __NetBSD__
  /* In NetBSD >= 4.0, the executable is accessible as /proc/<pid>/exe and
     /proc/curproc/exe.  */
  {
    char *link;

    link = xreadlink ("/proc/curproc/exe");
    if (link != NULL)
      return link;
  }
# endif
# if defined __sun
  /* On Solaris >= 11.4, /proc/<pid>/execname and /proc/self/execname contains
     the name of the executable, either as an absolute file name or relative to
     the current directory.  */
  {
    char namebuf[4096];
    int fd = open ("/proc/self/execname", O_RDONLY | O_CLOEXEC, 0);
    if (fd >= 0)
      {
        size_t len = full_read (fd, namebuf, sizeof (namebuf));
        close (fd);
        if (len > 0 && len < sizeof (namebuf))
          {
            namebuf[len] = '\0';
            return canonicalize_file_name (namebuf);
          }
      }
  }
# endif
# if defined __CYGWIN__
  /* The executable is accessible as /proc/<pid>/exe, at least in
     Cygwin >= 1.5.  */
  {
    char *link;

    link = xreadlink ("/proc/self/exe");
    if (link != NULL)
      return link;
    if (executable_fd < 0)
      executable_fd = open ("/proc/self/exe", O_EXEC | O_CLOEXEC, 0);
  }
# endif
# if HAVE_MACH_O_DYLD_H && HAVE__NSGETEXECUTABLEPATH
  /* On Mac OS X 10.2 or newer, the function
       int _NSGetExecutablePath (char *buf, uint32_t *bufsize);
     can be used to retrieve the executable's full path.  */
  char location[4096];
  unsigned int length = sizeof (location);
  if (_NSGetExecutablePath (location, &length) == 0
      && location[0] == '/')
    return canonicalize_file_name (location);
# endif
  /* Guess the executable's full path.  We assume the executable has been
     called via execlp() or execvp() with properly set up argv[0].  The
     login(1) convention to add a '-' prefix to argv[0] is not supported.  */
  {
    bool has_slash = false;
    {
      const char *p;
      for (p = argv0; *p; p++)
        if (*p == '/')
          {
            has_slash = true;
            break;
          }
    }
    if (!has_slash)
      {
        /* exec searches paths without slashes in the directory list given
           by $PATH.  */
        const char *path = getenv ("PATH");

        if (path != NULL)
          {
            const char *p;
            const char *p_next;

            for (p = path; *p; p = p_next)
              {
                const char *q;
                size_t p_len;
                char *concat_name;

                for (q = p; *q; q++)
                  if (*q == ':')
                    break;
                p_len = q - p;
                p_next = (*q == '\0' ? q : q + 1);

                /* We have a path item at p, of length p_len.
                   Now concatenate the path item and argv0.  */
                concat_name = (char *) xmalloc (p_len + strlen (argv0) + 2);
# ifdef NO_XMALLOC
                if (concat_name == NULL)
                  return NULL;
# endif
                if (p_len == 0)
                  /* An empty PATH element designates the current directory.  */
                  strcpy (concat_name, argv0);
                else
                  {
                    memcpy (concat_name, p, p_len);
                    concat_name[p_len] = '/';
                    strcpy (concat_name + p_len + 1, argv0);
                  }
                if (maybe_executable (concat_name))
                  return canonicalize_file_name (concat_name);
                free (concat_name);
              }
          }
        /* Not found in the PATH, assume the current directory.  */
      }
    /* exec treats paths containing slashes as relative to the current
       directory.  */
    if (maybe_executable (argv0))
      return canonicalize_file_name (argv0);
  }
  /* No way to find the executable.  */
  return NULL;
#endif
}

/* Full pathname of executable, or NULL.  */
static char *executable_fullname;

static void
prepare_relocate (const char *orig_installprefix, const char *orig_installdir,
                  const char *argv0)
{
  char *curr_prefix;

  /* Determine the full pathname of the current executable.  */
  executable_fullname = find_executable (argv0);

  /* Determine the current installation prefix from it.  */
  curr_prefix = compute_curr_prefix (orig_installprefix, orig_installdir,
                                     executable_fullname);
  if (curr_prefix != NULL)
    {
      /* Now pass this prefix to all copies of the relocate.c source file.  */
      set_relocation_prefix (orig_installprefix, curr_prefix);

      free (curr_prefix);
    }
}

/* Set program_name, based on argv[0], and original installation prefix and
   directory, for relocatability.  */
void
set_program_name_and_installdir (const char *argv0,
                                 const char *orig_installprefix,
                                 const char *orig_installdir)
{
  const char *argv0_stripped = argv0;

  /* Relocatable programs are renamed to .bin by install-reloc.  Or, more
     generally, their suffix is changed from $exeext to .bin$exeext.
     Remove the ".bin" here.  */
  {
    size_t argv0_len = strlen (argv0);
    const size_t exeext_len = sizeof (EXEEXT) - sizeof ("");
    if (argv0_len > 4 + exeext_len)
      if (memcmp (argv0 + argv0_len - exeext_len - 4, ".bin", 4) == 0)
        {
          if (sizeof (EXEEXT) > sizeof (""))
            {
              /* Compare using an inlined copy of c_strncasecmp(), because
                 the filenames may have undergone a case conversion since
                 they were packaged.  In other words, EXEEXT may be ".exe"
                 on one system and ".EXE" on another.  */
              static const char exeext[] = EXEEXT;
              const char *s1 = argv0 + argv0_len - exeext_len;
              const char *s2 = exeext;
              for (; *s1 != '\0'; s1++, s2++)
                {
                  unsigned char c1 = *s1;
                  unsigned char c2 = *s2;
                  if ((c1 >= 'A' && c1 <= 'Z' ? c1 - 'A' + 'a' : c1)
                      != (c2 >= 'A' && c2 <= 'Z' ? c2 - 'A' + 'a' : c2))
                    goto done_stripping;
                }
            }
          /* Remove ".bin" before EXEEXT or its equivalent.  */
          {
            char *shorter = (char *) xmalloc (argv0_len - 4 + 1);
#ifdef NO_XMALLOC
            if (shorter != NULL)
#endif
              {
                memcpy (shorter, argv0, argv0_len - exeext_len - 4);
                if (sizeof (EXEEXT) > sizeof (""))
                  memcpy (shorter + argv0_len - exeext_len - 4,
                          argv0 + argv0_len - exeext_len - 4,
                          exeext_len);
                shorter[argv0_len - 4] = '\0';
                argv0_stripped = shorter;
              }
          }
         done_stripping: ;
      }
  }

  set_program_name (argv0_stripped);

  prepare_relocate (orig_installprefix, orig_installdir, argv0);
}

/* Return the full pathname of the current executable, based on the earlier
   call to set_program_name_and_installdir.  Return NULL if unknown.  */
char *
get_full_program_name (void)
{
  return executable_fullname;
}

#endif
