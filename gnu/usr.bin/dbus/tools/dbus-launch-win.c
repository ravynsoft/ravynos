/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-launch-win.c  dbus-launch utility
 *
 * Copyright (C) 2007 Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>
#ifndef UNICODE
#define UNICODE 1
#endif
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Save string functions.  Instead of figuring out the exact _MSC_VER
   that work, override for everybody.  */

#define errno_t int
#define wcscat_s my_wcscat_s
#define wcscpy_s my_wcscpy_s

static errno_t
wcscat_s (wchar_t *dest, size_t size, const wchar_t *src)
{
  assert (sizeof (wchar_t) * (wcslen (dest) + wcslen (src) + 1) <= size);
  wcscat (dest, src);
  return 0;
}


static errno_t
wcscpy_s (wchar_t *dest, size_t size, const wchar_t *src)
{
  assert (sizeof (wchar_t) * (wcslen (src) + 1) <= size);
  wcscpy (dest, src);
  return 0;
}

/* TODO (tl): This Windows version of dbus-launch is curretly rather
 * pointless as it doesn't take the same command-line options as the
 * UNIX dbus-launch does. A main point of the dbus-launch command is
 * to pass it for instance a --config-file option to make the started
 * dbus-daemon use that config file.
 * 
 * This version also doesn't print out any information, which is a
 * main point of the UNIX one. It should at least support the
 * --sh-syntax option, and maybe also a --cmd-syntax to print out the
 * variable settings in cmd.exe syntax?
 * 
 * NOTE (rh): The main task of dbus-launch is (from the man page) to start 
 * a session bus and this is archieved by the current implemention. 
 * 
 * Additional on windows the session bus starting in not integrated 
 * into the logon process, so there is no need for any --syntax option. 
 * In fact (at least for kde on windows) the session bus is autostarted 
 * with the first application requesting a session bus. 
 *
 */

#define AUTO_ACTIVATE_CONSOLE_WHEN_VERBOSE_MODE 1

#define DIM(x) (sizeof(x) / sizeof(x[0]))
#define WCSTRINGIFY_(x) L ## x
#define WCSTRINGIFY(x) WCSTRINGIFY_(x)

int
main (int argc, char **argv)
{
  wchar_t dbusDaemonPath[MAX_PATH * 2 + 1];
  wchar_t command[MAX_PATH * 2 + 1];
  wchar_t *p;
  const wchar_t *daemon_name;
  int result;

#ifdef DBUS_WINCE
  char *s = NULL;
#else
  char *s = getenv("DBUS_VERBOSE");
#endif
  int verbose = s && *s != '\0' ? 1 : 0;

  PROCESS_INFORMATION pi;
  STARTUPINFOW si;
  BOOL inherit = TRUE;
  DWORD flags = 0;

  GetModuleFileNameW (NULL, dbusDaemonPath, DIM (dbusDaemonPath));
  
  daemon_name = WCSTRINGIFY(DBUS_DAEMON_NAME) L".exe";
  
  if ((p = wcsrchr (dbusDaemonPath, L'\\'))) 
    {
      p[1] = L'\0';
      wcscat_s (dbusDaemonPath, sizeof (dbusDaemonPath), daemon_name);
    }
  else 
    {
      if (verbose)
          fprintf (stderr, "error: could not extract path from current "
                   "applications module filename\n");
      return 1;
    } 

#ifdef DBUS_WINCE
   /* Windows CE has a different interpretation of cmdline: Start with argv[1].  */
   wcscpy_s (command, sizeof (command), L"--session");
   if (verbose)
     fprintf (stderr, "%ls %ls\n", dbusDaemonPath, command);
#else
   command[0] = L'\0';
   /* Windows cmdline starts with path, which can contain spaces.  */
   wcscpy_s (command, sizeof (command), L"\"");
   wcscat_s (command, sizeof (command), dbusDaemonPath);
   wcscat_s (command, sizeof (command), L"\" --session");
   if (verbose)
     fprintf (stderr, "%ls\n", command);
#endif
  
  memset (&si, 0, sizeof (si));
  memset (&pi, 0, sizeof (pi));
  si.cb = sizeof (si);
  
  if (verbose)
    flags |= CREATE_NEW_CONSOLE;

#ifdef DBUS_WINCE
  inherit = FALSE;
#else
  flags |= NORMAL_PRIORITY_CLASS;
  if (!verbose)
    flags |= DETACHED_PROCESS;
#endif

  result = CreateProcessW (dbusDaemonPath, command, 0, 0,
                           inherit, flags, 0, 0, &si, &pi);

  if (result == 0) 
    {
      if (verbose)
        fprintf (stderr, "Could not start " DBUS_DAEMON_NAME ". error=%u\n",
                 (unsigned)GetLastError ());
      return 4;
    }
   
  CloseHandle (pi.hProcess);
  CloseHandle (pi.hThread);

  return 0;
}
