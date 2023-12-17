/* Determine the parent process of a given process.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
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
#include "get_ppid_of.h"

#include <stdio.h>
#include <string.h>

#if defined __linux__ || defined __ANDROID__ || (defined __FreeBSD_kernel__ && !defined __FreeBSD__) || defined __GNU__ || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __minix || defined __sun /* Linux, GNU/kFreeBSD, GNU/Hurd, FreeBSD, NetBSD, Minix, Solaris */
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
# include <unistd.h>
#endif

pid_t
get_ppid_of (pid_t pid)
{
#if defined __linux__ || defined __ANDROID__ || (defined __FreeBSD_kernel__ && !defined __FreeBSD__) || defined __GNU__ /* Linux, GNU/kFreeBSD, GNU/Hurd */
/* GNU/kFreeBSD mounts /proc as linprocfs, which looks like a Linux /proc
   file system.  */

  /* Read the contents of /proc/<pid>/status into memory.  */
  char filename[6 + 10 + 7 + 1];
  int fd;

  sprintf (filename, "/proc/%u/status", (unsigned int) pid);
  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (fd >= 0)
    {
      char buf[4096 + 1];
      ssize_t nread = read (fd, buf, sizeof (buf) - 1);
      close (fd);
      if (nread >= 0)
        {
          char *bufend = buf + nread;
          char *p;

          /* NUL-terminate the buffer.  */
          *bufend = '\0';

          /* Search for a line that starts with "PPid:".  */
          for (p = buf;;)
            {
              if (bufend - p >= 5 && memcmp (p, "PPid:", 5) == 0)
                {
                  unsigned int ppid = 0;
                  if (sscanf (p + 5, "%u", &ppid) > 0)
                    return ppid;
                }
              p = strchr (p, '\n');
              if (p != NULL)
                p++;
              else
                break;
            }
        }
    }

#endif

#if defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ /* FreeBSD, NetBSD */

  /* Read the contents of /proc/<pid>/status into memory.  */
  char filename[6 + 10 + 7 + 1];
  int fd;

  sprintf (filename, "/proc/%u/status", (unsigned int) pid);
  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (fd >= 0)
    {
      char buf[4096 + 1];
      ssize_t nread = read (fd, buf, sizeof (buf) - 1);
      close (fd);
      if (nread >= 0)
        {
          char *p;

          /* NUL-terminate the buffer.  */
          buf[nread] = '\0';

          /* Search for the third space-separated field.  */
          p = strchr (buf, ' ');
          if (p != NULL)
            {
              p = strchr (p + 1, ' ');
              if (p != NULL)
                {
                  unsigned int ppid = 0;
                  if (sscanf (p + 1, "%u", &ppid) > 0)
                    return ppid;
                }
            }
        }
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

          /* Search for the 16th space-separated field.  */
          p = strchr (buf, ' ');
          for (count = 1; p != NULL && count < 15; count++)
            p = strchr (p + 1, ' ');
          if (p != NULL)
            {
              unsigned int ppid = 0;
              if (sscanf (p + 1, "%u", &ppid) > 0)
                return ppid;
            }
        }
    }

#endif

#if defined __sun                                           /* Solaris */

  /* Read the contents of /proc/<pid>/psinfo into memory.
     Alternatively, we could read the contents of /proc/<pid>/status into
     memory.  But it contains a lot of information that we don't need.  */
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
      #else
      # define PSINFO_SIZE 336
      #endif
      union { char all[PSINFO_SIZE]; unsigned int header[11]; } buf;
      ssize_t nread = read (fd, buf.all, sizeof (buf.all));
      close (fd);
      if (nread >= (ssize_t) sizeof (buf.header))
        return buf.header[3];
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
    return info.p_ppid;

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
        return info.pbsi_ppid;
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
        return info.pbi_ppid;
    }
#  endif

# endif
#endif

#if defined _AIX                                            /* AIX */

  /* Reference: https://www.ibm.com/support/knowledgecenter/en/ssw_aix_61/com.ibm.aix.basetrf1/getprocs.htm
  */
  struct procentry64 procs;
  if (getprocs64 (&procs, sizeof procs, NULL, 0, &pid, 1) > 0)
    return procs.pi_ppid;

#endif

#if defined __hpux                                          /* HP-UX */

  struct pst_status status;
  if (pstat_getproc (&status, sizeof status, 0, pid) > 0)
    return status.pst_ppid;
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
        return *(unsigned long long *)(status64 + 24);
# endif
    }

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
        return buf.pr_ppid;
    }

#endif

#if defined __CYGWIN__                                      /* Cygwin */

  struct external_pinfo *info =
    (struct external_pinfo *) cygwin_internal (CW_GETPINFO, pid);
  if (info != NULL)
    return info->ppid;

#endif

#if defined __BEOS__ || defined __HAIKU__                   /* BeOS, Haiku */

  if (pid == getpid ())
    return getppid ();

#endif

  return 0;
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
  pid_t parent = get_ppid_of (pid);
  printf ("PID=%lu PPID=%lu\n", (unsigned long) pid, (unsigned long) parent);
  return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -ggdb -DTEST -Wall -I.. get_ppid_of.c"
 * End:
 */

#endif
