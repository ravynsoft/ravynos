/* Determine the program name of a given process.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "get_progname_of.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined __linux__ || defined __ANDROID__ || (defined __FreeBSD_kernel__ && !defined __FreeBSD__) || defined __GNU__ || defined __NetBSD__ || defined __FreeBSD__ /* Linux, GNU/kFreeBSD, GNU/Hurd, NetBSD, FreeBSD */
# include <unistd.h>
# if defined __ANDROID__
#  include <fcntl.h>
# endif
#endif

#if defined __minix || defined __sun                        /* Minix, Solaris */
# include <fcntl.h>
# include <unistd.h>
#endif

#if defined __OpenBSD__                                     /* OpenBSD */
# include <sys/sysctl.h> /* sysctl, struct kinfo_proc */
#endif

#if defined __APPLE__ && defined __MACH__                   /* Mac OS X */
/* Get MAC_OS_X_VERSION_MIN_REQUIRED, MAC_OS_X_VERSION_MAX_ALLOWED.
   The version at runtime satisfies
   MAC_OS_X_VERSION_MIN_REQUIRED <= version <= MAC_OS_X_VERSION_MAX_ALLOWED.  */
# include <AvailabilityMacros.h>
# if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
#  include <libproc.h>
#  if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
/* Mac OS X versions < 10.5 don't have this function.  Therefore declare it as
   weak, in order to avoid a runtime error when the binaries are run on these
   older versions.  */
extern int proc_pidinfo (int, int, uint64_t, void *, int) WEAK_IMPORT_ATTRIBUTE;
#  endif
# endif
#endif

#if defined _AIX                                            /* AIX */
# include <procinfo.h>
#endif

#if defined __hpux                                          /* HP-UX */
# include <unistd.h>
# include <sys/param.h>
# include <sys/pstat.h>
#endif

#if defined __sgi                                           /* IRIX */
# include <unistd.h>
# include <fcntl.h>
# include <sys/procfs.h>
#endif

#if defined __CYGWIN__                                      /* Cygwin */
# define WIN32_LEAN_AND_MEAN
# include <windows.h> /* needed to get 'struct external_pinfo' defined */
# include <sys/cygwin.h>
#endif

#if defined __BEOS__ || defined __HAIKU__                   /* BeOS, Haiku */
# include <OS.h>
#endif

char *
get_progname_of (pid_t pid)
{
#if defined __linux__ || defined __ANDROID__ || (defined __FreeBSD_kernel__ && !defined __FreeBSD__) || defined __GNU__ || defined __NetBSD__ /* Linux, GNU/kFreeBSD, GNU/Hurd, NetBSD */
/* GNU/kFreeBSD mounts /proc as linprocfs, which looks like a Linux /proc
   file system.  */

  /* Read the symlink /proc/<pid>/exe.  */
  {
    char filename[6 + 10 + 4 + 1];
    char linkbuf[1024 + 1];
    ssize_t linklen;

    sprintf (filename, "/proc/%u/exe", (unsigned int) pid);
    linklen = readlink (filename, linkbuf, sizeof (linkbuf) - 1);
    if (linklen > 0)
      {
        char *slash;

        /* NUL-terminate the link.  */
        linkbuf[linklen] = '\0';
        /* Find the portion after the last slash.  */
        slash = strrchr (linkbuf, '/');
        return strdup (slash != NULL ? slash + 1 : linkbuf);
      }
  }

# if defined __ANDROID__
  /* But it may fail with "Permission denied".  As a fallback,
     read the contents of /proc/<pid>/cmdline into memory.  */
  {
    char filename[6 + 10 + 8 + 1];
    int fd;

    sprintf (filename, "/proc/%u/cmdline", (unsigned int) pid);
    fd = open (filename, O_RDONLY | O_CLOEXEC);
    if (fd >= 0)
      {
        char buf[4096 + 1];
        ssize_t nread = read (fd, buf, sizeof (buf) - 1);
        close (fd);
        if (nread >= 0)
          {
            char *slash;

            /* NUL-terminate the buffer (just in case it does not have the
               expected format).  */
            buf[nread] = '\0';
            /* The program name and each argument is followed by a NUL byte.  */
            /* Find the portion after the last slash.  */
            slash = strrchr (buf, '/');
            return strdup (slash != NULL ? slash + 1 : buf);
          }
      }
  }
# endif

#endif

#if defined __FreeBSD__                                     /* FreeBSD */

  /* Read the symlink /proc/<pid>/file.  */
  char filename[6 + 10 + 5 + 1];
  char linkbuf[1024 + 1];
  ssize_t linklen;

  sprintf (filename, "/proc/%u/file", (unsigned int) pid);
  linklen = readlink (filename, linkbuf, sizeof (linkbuf) - 1);
  if (linklen > 0)
    {
      char *slash;

      /* NUL-terminate the link.  */
      linkbuf[linklen] = '\0';
      /* Find the portion after the last slash.  */
      slash = strrchr (linkbuf, '/');
      return strdup (slash != NULL ? slash + 1 : linkbuf);
    }

#endif

#if defined __minix                                         /* Minix */

  /* Read the contents of /proc/<pid>/psinfo into memory.  */
  char filename[6 + 10 + 7 + 1];
  int fd;

  sprintf (filename, "/proc/%u/psinfo", (unsigned int) pid);
  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (fd >= 0)
    {
      char buf[4096 + 1];
      ssize_t nread = read (fd, buf, sizeof (buf) - 1);
      close (fd);
      if (nread >= 0)
        {
          char *p;
          int count;

          /* NUL-terminate the buffer.  */
          buf[nread] = '\0';

          /* Search for the 4th space-separated field.  */
          p = strchr (buf, ' ');
          for (count = 1; p != NULL && count < 3; count++)
            p = strchr (p + 1, ' ');
          if (p != NULL)
            {
              char *start = p + 1;
              char *end = strchr (p + 1, ' ');
              if (end != NULL)
                {
                  *end = '\0';
                  return strdup (start);
                }
            }
        }
    }

#endif

#if defined __sun                                           /* Solaris */

  /* Read the symlink /proc/<pid>/path/a.out.
     When it succeeds, it doesn't truncate.  */
  {
    char filename[6 + 10 + 11 + 1];
    char linkbuf[1024 + 1];
    ssize_t linklen;

    sprintf (filename, "/proc/%u/path/a.out", (unsigned int) pid);
    linklen = readlink (filename, linkbuf, sizeof (linkbuf) - 1);
    if (linklen > 0)
      {
        char *slash;

        /* NUL-terminate the link.  */
        linkbuf[linklen] = '\0';
        /* Find the portion after the last slash.  */
        slash = strrchr (linkbuf, '/');
        return strdup (slash != NULL ? slash + 1 : linkbuf);
      }
  }

  /* But it may fail with "Permission denied".  As a fallback,
     read the contents of /proc/<pid>/psinfo into memory.
     Alternatively, we could read the contents of /proc/<pid>/status into
     memory.  But it contains a lot of information that we don't need.  */
  {
    char filename[6 + 10 + 7 + 1];
    int fd;

    sprintf (filename, "/proc/%u/psinfo", (unsigned int) pid);
    fd = open (filename, O_RDONLY | O_CLOEXEC);
    if (fd >= 0)
      {
        /* The contents is a 'struct psinfo'.  But since 'struct psinfo'
           has a different size in a 32-bit and a 64-bit environment, we
           avoid it.  Nevertheless, the size of this contents depends on
           whether the process that reads it is 32-bit or 64-bit!  */
        #if defined __LP64__
        # define PSINFO_SIZE 416
        # define PSINFO_FNAME_OFFSET 136
        #else
        # define PSINFO_SIZE 336
        # define PSINFO_FNAME_OFFSET 88
        #endif
        char buf[PSINFO_SIZE];
        ssize_t nread = read (fd, buf, sizeof (buf));
        close (fd);
        if (nread >= PSINFO_FNAME_OFFSET + 16)
          {
            /* Make sure it's NUL-terminated.  */
            buf[PSINFO_FNAME_OFFSET + 16] = '\0';
            return strdup (&buf[PSINFO_FNAME_OFFSET]);
          }
      }
  }

#endif

#if defined __OpenBSD__                                     /* OpenBSD */

  /* Documentation: https://man.openbsd.org/sysctl.2  */
  int info_path[] =
    { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid, sizeof (struct kinfo_proc), 1 };
  struct kinfo_proc info;
  size_t len;

  len = sizeof (info);
  if (sysctl (info_path, 6, &info, &len, NULL, 0) >= 0 && len == sizeof (info))
    return strdup (info.p_comm);

#endif

#if defined __APPLE__ && defined __MACH__                   /* Mac OS X */
# if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050

  /* Mac OS X >= 10.7 has PROC_PIDT_SHORTBSDINFO.  */
#  if defined PROC_PIDT_SHORTBSDINFO
#   if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
  if (proc_pidinfo != NULL) /* at runtime Mac OS X >= 10.5 ? */
#   endif
    {
      struct proc_bsdshortinfo info;

      if (proc_pidinfo (pid, PROC_PIDT_SHORTBSDINFO, 0, &info, sizeof (info))
          == sizeof (info))
        return strdup (info.pbsi_comm);
    }
#  endif

#  if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
  /* For older versions, use PROC_PIDTBSDINFO instead.  */
  /* Note: The second part of 'struct proc_bsdinfo' differs in size between
     32-bit and 64-bit environments, and the kernel of Mac OS X 10.5 knows
     only about the 32-bit 'struct proc_bsdinfo'.  Fortunately all the info
     we need is in the first part, which is the same in 32-bit and 64-bit.  */
#   if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
  if (proc_pidinfo != NULL) /* at runtime Mac OS X >= 10.5 ? */
#   endif
    {
      struct proc_bsdinfo info;

      if (proc_pidinfo (pid, PROC_PIDTBSDINFO, 0, &info, 128) == 128)
        return strdup (info.pbi_comm);
    }
#  endif

# endif
#endif

#if defined _AIX                                            /* AIX */

  /* Reference: https://www.ibm.com/support/knowledgecenter/en/ssw_aix_61/com.ibm.aix.basetrf1/getprocs.htm
  */
  struct procentry64 procs;
  if (getprocs64 (&procs, sizeof procs, NULL, 0, &pid, 1) > 0)
    return strdup (procs.pi_comm);

#endif

#if defined __hpux                                          /* HP-UX */

  char *p;
  struct pst_status status;
  if (pstat_getproc (&status, sizeof status, 0, pid) > 0)
    {
      char *ucomm = status.pst_ucomm;
      char *cmd = status.pst_cmd;
      if (strlen (ucomm) < PST_UCOMMLEN - 1)
        p = ucomm;
      else
        {
          /* ucomm is truncated to length PST_UCOMMLEN - 1.
             Look at cmd instead.  */
          char *space = strchr (cmd, ' ');
          if (space != NULL)
            *space = '\0';
          p = strrchr (cmd, '/');
          if (p != NULL)
            p++;
          else
            p = cmd;
          if (strlen (p) > PST_UCOMMLEN - 1
              && memcmp (p, ucomm, PST_UCOMMLEN - 1) == 0)
            /* p is less truncated than ucomm.  */
            ;
          else
            p = ucomm;
        }
      p = strdup (p);
    }
  else
    {
# if !defined __LP64__
      /* Support for 32-bit programs running in 64-bit HP-UX.
         The documented way to do this is to use the same source code
         as above, but in a compilation unit where '#define _PSTAT64 1'
         is in effect.  I prefer a single compilation unit; the struct
         size and the offsets are not going to change.  */
      char status64[1216];
      if (__pstat_getproc64 (status64, sizeof status64, 0, pid) > 0)
        {
          char *ucomm = status64 + 288;
          char *cmd = status64 + 168;
          if (strlen (ucomm) < PST_UCOMMLEN - 1)
            p = ucomm;
          else
            {
              /* ucomm is truncated to length PST_UCOMMLEN - 1.
                 Look at cmd instead.  */
              char *space = strchr (cmd, ' ');
              if (space != NULL)
                *space = '\0';
              p = strrchr (cmd, '/');
              if (p != NULL)
                p++;
              else
                p = cmd;
              if (strlen (p) > PST_UCOMMLEN - 1
                  && memcmp (p, ucomm, PST_UCOMMLEN - 1) == 0)
                /* p is less truncated than ucomm.  */
                ;
              else
                p = ucomm;
            }
          p = strdup (p);
        }
      else
# endif
        p = NULL;
    }
  if (p != NULL)
    return strdup (p);

#endif

#if defined __sgi                                           /* IRIX */

  char filename[12 + 10 + 1];
  int fd;

  sprintf (filename, "/proc/pinfo/%u", pid);
  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (0 <= fd)
    {
      prpsinfo_t buf;
      int ioctl_ok = 0 <= ioctl (fd, PIOCPSINFO, &buf);
      close (fd);
      if (ioctl_ok)
        {
          char *name = buf.pr_fname;
          size_t namesize = sizeof buf.pr_fname;
          /* It may not be NUL-terminated.  */
          char *namenul = memchr (name, '\0', namesize);
          size_t namelen = namenul ? namenul - name : namesize;
          char *namecopy = malloc (namelen + 1);
          if (namecopy)
            {
              namecopy[namelen] = '\0';
              return memcpy (namecopy, name, namelen);
            }
        }
    }

#endif

#if defined __CYGWIN__                                      /* Cygwin */

  struct external_pinfo *info =
    (struct external_pinfo *) cygwin_internal (CW_GETPINFO, pid);
  if (info != NULL)
    {
      const char *name = info->progname;
      size_t namesize = sizeof (info->progname);
      /* It may not be NUL-terminated.  */
      const char *namenul = memchr (name, '\0', namesize);
      size_t namelen = namenul ? namenul - name : namesize;

      /* Find the portion after the last backslash.
         Cygwin does not have memrchr().  */
      {
        const char *backslash = memchr (name, '\\', namelen);
        if (backslash != NULL)
          {
            const char *name_end = name + namelen;
            for (;;)
              {
                const char *next_backslash =
                  memchr (backslash + 1, '\\', name_end - (backslash + 1));
                if (next_backslash == NULL)
                  break;
                backslash = next_backslash;
              }
            name = backslash + 1;
            namelen = name_end - name;
          }
      }

      {
        char *namecopy = malloc (namelen + 1);
        if (namecopy)
          {
            namecopy[namelen] = '\0';
            return memcpy (namecopy, name, namelen);
          }
      }
    }

#endif

#if defined __BEOS__ || defined __HAIKU__                   /* BeOS, Haiku */

  team_info info;
  if (_get_team_info (pid, &info, sizeof (info)) == B_OK)
    {
      const char *name = info.args;
      size_t namesize = sizeof (info.args);
      /* It may not be NUL-terminated.  */
      const char *namenul = memchr (name, '\0', namesize);
      size_t namelen = namenul ? namenul - name : namesize;

      /* Take the portion up to the first space.  */
      {
        const char *space = memchr (name, ' ', namelen);
        if (space != NULL)
          namelen = space - name;
      }

      /* Find the portion after the last slash.  */
      {
        const char *slash = memchr (name, '/', namelen);
        if (slash != NULL)
          {
            const char *name_end = name + namelen;
            for (;;)
              {
                const char *next_slash =
                  memchr (slash + 1, '/', name_end - (slash + 1));
                if (next_slash == NULL)
                  break;
                slash = next_slash;
              }
            name = slash + 1;
            namelen = name_end - name;
          }
      }

      {
        char *namecopy = malloc (namelen + 1);
        if (namecopy)
          {
            namecopy[namelen] = '\0';
            return memcpy (namecopy, name, namelen);
          }
      }
    }

#endif

  return NULL;
}

#ifdef TEST

#include <stdlib.h>
#include <unistd.h>

/* Usage: ./a.out
   or:    ./a.out PID
 */
int
main (int argc, char *argv[])
{
  char *arg = argv[1];
  pid_t pid = (arg != NULL ? atoi (arg) : getpid ());
  char *progname = get_progname_of (pid);
  printf ("PID=%lu COMMAND=%s\n",
          (unsigned long) pid, progname != NULL ? progname : "(null)");
  free (progname);
  return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -ggdb -DTEST -Wall -I.. get_progname_of.c"
 * End:
 */

#endif
