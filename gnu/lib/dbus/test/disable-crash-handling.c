/*
 * Copyright 2003 Red Hat, Inc.
 * Copyright 2007-2016 Ralf Habacker
 * Copyright 2014-2018 Collabora Ltd.
 * Copyright 2016 Yiyang Fei
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
 */

/*
 * This test utility function is separated from test-utils.h because it
 * needs to be used by test-segfault, which deliberately crashes itself.
 *
 * test-segfault can't be linked to non-self-contained dbus code because
 * we want to avoid building it with the AddressSanitizer even if we are
 * using the AddressSanitizer for the rest of dbus, so that the
 * AddressSanitizer doesn't turn raise(SIGSEGV) into the equivalent of
 * _exit(1), causing the test that uses test-segfault to see an unexpected
 * exit status.
 */

#include "config.h"
#include "disable-crash-handling.h"

#ifdef DBUS_WIN

#include <stdio.h>
#include <windows.h>

#include <dbus/dbus-macros.h>

static int exception_handler (LPEXCEPTION_POINTERS p) _DBUS_GNUC_NORETURN;

static int
exception_handler (LPEXCEPTION_POINTERS p)
{
  ExitProcess (0xc0000005);
}

/**
 * Try to disable core dumps and similar special crash handling.
 */
void
_dbus_disable_crash_handling (void)
{
  /* Disable Windows popup dialog when an app crashes so that app quits
   * immediately with error code instead of waiting for user to dismiss
   * the dialog.  */
  DWORD dwMode = SetErrorMode (SEM_NOGPFAULTERRORBOX);

  SetErrorMode (dwMode | SEM_NOGPFAULTERRORBOX);
  /* Disable "just in time" debugger */
  SetUnhandledExceptionFilter ((LPTOP_LEVEL_EXCEPTION_FILTER) &exception_handler);
}

#else /* !DBUS_WIN */

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/**
 * Try to disable core dumps and similar special crash handling.
 */
void
_dbus_disable_crash_handling (void)
{
#ifdef HAVE_SETRLIMIT
  /* No core dumps please, we know we crashed. */
  struct rlimit r = { 0, };

  getrlimit (RLIMIT_CORE, &r);
  r.rlim_cur = 0;
  setrlimit (RLIMIT_CORE, &r);
#endif

#if defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
  /* Really, no core dumps please. On Linux, if core_pattern is
   * set to a pipe (for abrt/apport/corekeeper/etc.), RLIMIT_CORE of 0
   * is ignored (deliberately, so people can debug init(8) and other
   * early stuff); but Linux has PR_SET_DUMPABLE, so we can avoid core
   * dumps anyway. */
  prctl (PR_SET_DUMPABLE, 0, 0, 0, 0);
#endif
}

#endif /* !DBUS_WIN */
