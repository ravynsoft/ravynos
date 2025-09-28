/* Determine whether the current process is running under QEMU.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2021.  */

#ifdef __linux__
# include <fcntl.h>
# include <string.h>
# include <unistd.h>
#endif

/* This function determines whether the current process is running under QEMU
   (user-mode).

   It does so by looking at parts of the environment that QEMU does not emulate
   100% perfectly well.

   For comparison, the techniques given in the paper
     Thomas Raffetseder, Christopher Kruegel, Engin Kirda
     "Detecting System Emulators"
     2007
     https://publik.tuwien.ac.at/files/pub-inf_5317.pdf
   apply to both the QEMU system mode and QEMU user mode.  */

static bool
is_running_under_qemu_user (void)
{
#ifdef __linux__
  char buf[4096 + 1];
  int fd;

# if defined __m68k__
  fd = open ("/proc/hardware", O_RDONLY);
  if (fd >= 0)
    {
      int n = read (fd, buf, sizeof (buf) - 1);
      close (fd);
      if (n > 0)
        {
          buf[n] = '\0';
          if (strstr (buf, "qemu") != NULL)
            return true;
        }
    }
# endif

  fd = open ("/proc/cpuinfo", O_RDONLY);
  if (fd >= 0)
    {
      int n = read (fd, buf, sizeof (buf) - 1);
      close (fd);
      if (n > 0)
        {
          buf[n] = '\0';
# if defined __hppa__
          if (strstr (buf, "QEMU") != NULL)
            return true;
# endif
# if !(defined __i386__ || defined __x86_64__)
          if (strstr (buf, "AuthenticAMD") != NULL
              || strstr (buf, "GenuineIntel") != NULL)
            return true;
# endif
# if !(defined __arm__ || defined __aarch64__)
          if (strstr (buf, "ARM") != NULL
              || strcasestr (buf, "aarch64") != NULL)
            return true;
# endif
# if !defined __sparc__
          if (strcasestr (buf, "SPARC") != NULL)
            return true;
# endif
# if !defined __powerpc__
          if (strstr (buf, "POWER") != NULL)
            return true;
# endif
        }
    }

  /* If you need more heuristics, look at system calls that are not perfectly
     well emulated in qemu/linux-user/syscall.c.  */
#endif

  return false;
}
