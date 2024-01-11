/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-launch.h  dbus-launch utility
 *
 * Copyright (C) 2006 Thiago Macieira <thiago@kde.org>
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
#include "dbus-launch.h"

#ifdef DBUS_BUILD_X11
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

Display *xdisplay = NULL;
static Atom selection_atom;
static Atom address_atom;
static Atom pid_atom;

static int
x_io_error_handler (Display *local_xdisplay)
{
  verbose ("X IO error\n");
  kill_bus_and_exit (0);
  return 0;
}

static void
remove_prefix (char *s,
               const char *prefix)
{
  int plen;

  plen = strlen (prefix);

  if (strncmp (s, prefix, plen) == 0)
    {
      memmove (s, s + plen, strlen (s) - plen + 1);
    }
}

static const char*
get_homedir (void)
{
  const char *home;
  
  home = getenv ("HOME");
  if (home == NULL)
    {
      /* try from the user database */
      struct passwd *user = getpwuid (getuid());
      if (user != NULL)
        home = user->pw_dir;
    }

  if (home == NULL)
    {
      fprintf (stderr, "Can't get user home directory\n");
      exit (1);
    }

  return home;
}

#define DBUS_DIR ".dbus"
#define DBUS_SESSION_BUS_DIR "session-bus"

static char *
get_session_file (void)
{
  static const char prefix[] = "/" DBUS_DIR "/" DBUS_SESSION_BUS_DIR "/";
  const char *machine;
  const char *home;
  char *display;
  char *result;
  char *p;

  machine = get_machine_uuid ();
  if (machine == NULL)
    return NULL;

  display = xstrdup (getenv ("DISPLAY"));
  if (display == NULL)
    {
      verbose ("X11 integration disabled because X11 is not running\n");
      return NULL;
    }

  /* remove the screen part of the display name */
  p = strrchr (display, ':');
  if (p != NULL)
    {
      for ( ; *p; ++p)
        {
          if (*p == '.')
            {
              *p = '\0';
              break;
            }
        }
    }

  /* Note that we leave the hostname in the display most of the
   * time. The idea is that we want to be per-(machine,display,user)
   * triplet to be extra-sure we get a bus we can connect to. Ideally
   * we'd recognize when the hostname matches the machine we're on in
   * all cases; we do try to drop localhost and localhost.localdomain
   * as a special common case so that alternate spellings of DISPLAY
   * don't result in extra bus instances.
   *
   * We also kill the ":" if there's nothing in front of it. This
   * avoids an ugly double underscore in the filename.
   */
  remove_prefix (display, "localhost.localdomain:");
  remove_prefix (display, "localhost:");
  remove_prefix (display, ":");

  /* replace the : in the display with _ if the : is still there.
   * use _ instead of - since it can't be in hostnames.
   *
   * similarly, on recent versions of macOS, X11 is provided by the XQuartz
   * package which uses a path for the hostname, such as
   *   /private/tmp/com.apple.launchd.mBSMLJ3yQU/org.macosforge.xquartz
   * we therefore also replace any / with _
   */
  for (p = display; *p; ++p)
    {
      if (*p == ':' || *p == '/')
        *p = '_';
    }
  
  home = get_homedir ();
  
  result = malloc (strlen (home) + strlen (prefix) + strlen (machine) +
                   strlen (display) + 2);
  if (result == NULL)
    {
      /* out of memory */
      free (display);
      return NULL;
    }

  strcpy (result, home);
  strcat (result, prefix);
  strcat (result, machine);
  strcat (result, "-");
  strcat (result, display);
  free (display);

  verbose ("session file: %s\n", result);
  return result;
}

static void
ensure_session_directory (void)
{
  const char *home;
  char *dir;
  
  home = get_homedir ();

  /* be sure we have space for / and nul */
  dir = malloc (strlen (home) + strlen (DBUS_DIR) + strlen (DBUS_SESSION_BUS_DIR) + 3);
  if (dir == NULL)
    {
      fprintf (stderr, "no memory\n");
      exit (1);
    }
  
  strcpy (dir, home);
  strcat (dir, "/");
  strcat (dir, DBUS_DIR);

  if (mkdir (dir, 0700) < 0)
    {
      /* only print a warning here, writing the session file itself will fail later */
      if (errno != EEXIST)
        fprintf (stderr, "Unable to create %s\n", dir);
    }

  strcat (dir, "/");
  strcat (dir, DBUS_SESSION_BUS_DIR);

  if (mkdir (dir, 0700) < 0)
    {
      /* only print a warning here, writing the session file itself will fail later */
      if (errno != EEXIST)
        fprintf (stderr, "Unable to create %s\n", dir);
    }
  
  free (dir);
}

static Display *
open_x11 (void)
{
  if (xdisplay != NULL)
    return xdisplay;

  xdisplay = XOpenDisplay (NULL);
  if (xdisplay != NULL)
    {
      verbose ("Connected to X11 display '%s'\n", DisplayString (xdisplay));
      XSetIOErrorHandler (x_io_error_handler);
    }
  return xdisplay;
}

static int
init_x_atoms (Display *display)
{
  static const char selection_prefix[] = "_DBUS_SESSION_BUS_SELECTION_";
  static const char address_prefix[] = "_DBUS_SESSION_BUS_ADDRESS";
  static const char pid_prefix[] = "_DBUS_SESSION_BUS_PID";
  static int init = FALSE;
  char *atom_name;
  const char *machine;
  char *user_name;
  struct passwd *user;

  if (init)
    return TRUE;

  machine = get_machine_uuid ();
  if (machine == NULL)
    return FALSE;

  user = getpwuid (getuid ());
  if (user == NULL)
    {
      verbose ("Could not determine user information; aborting X11 integration.\n");
      return FALSE;
    }
  user_name = xstrdup(user->pw_name);

  atom_name = malloc (strlen (machine) + strlen (user_name) + 2 +
                      MAX (strlen (selection_prefix),
                           MAX (strlen (address_prefix),
                                strlen (pid_prefix))));
  if (atom_name == NULL)
    {
      verbose ("Could not create X11 atoms; aborting X11 integration.\n");
      free (user_name);
      return FALSE;
    }

  /* create the selection atom */
  strcpy (atom_name, selection_prefix);
  strcat (atom_name, user_name);
  strcat (atom_name, "_");
  strcat (atom_name, machine);
  selection_atom = XInternAtom (display, atom_name, FALSE);

  /* create the address property atom */
  strcpy (atom_name, address_prefix);
  address_atom = XInternAtom (display, atom_name, FALSE);

  /* create the PID property atom */
  strcpy (atom_name, pid_prefix);
  pid_atom = XInternAtom (display, atom_name, FALSE);

  free (atom_name);
  free (user_name);
  init = TRUE;
  return TRUE;
}

/*
 * Gets the daemon address from the X11 display.
 * Returns FALSE if there was an error. Returning
 * TRUE does not mean the address exists.
 */
int
x11_get_address (char **paddress, pid_t *pid, long *wid)
{
  int result;
  Atom type;
  Window owner;
  int format;
  unsigned long items;
  unsigned long after;
  char *data;

  *paddress = NULL;

  /* locate the selection owner */
  owner = XGetSelectionOwner (xdisplay, selection_atom);
  if (owner == None)
    return TRUE;                /* no owner */
  if (wid != NULL)
    *wid = (long) owner;

  /* get the bus address */
  result = XGetWindowProperty (xdisplay, owner, address_atom, 0, 1024, False,
                              XA_STRING, &type, &format, &items, &after,
                              (unsigned char **) &data);
  if (result != Success || type == None || after != 0 || data == NULL || format != 8)
    return FALSE;               /* error */

  *paddress = xstrdup (data);
  XFree (data);

  /* get the PID */
  if (pid != NULL)
    {
      *pid = 0;
      result = XGetWindowProperty (xdisplay, owner, pid_atom, 0, sizeof pid, False,
                                   XA_CARDINAL, &type, &format, &items, &after,
                                   (unsigned char **) &data);
      if (result == Success && type != None && after == 0 && data != NULL && format == 32)
        *pid = (pid_t) *(long*) data;
      XFree (data);
    }

  return TRUE;                  /* success */
}

/*
 * Saves the address in the X11 display. Returns 0 on success.
 * If an error occurs, returns -1. If the selection already exists,
 * returns 1. (i.e. another daemon is already running)
 */
static Window
set_address_in_x11(char *address, pid_t pid)
{
  char *current_address;
  Window wid = None;
  unsigned long pid32; /* Xlib property functions want _long_ not 32-bit for format "32" */
  
  /* lock the X11 display to make sure we're doing this atomically */
  XGrabServer (xdisplay);

  if (!x11_get_address (&current_address, NULL, NULL))
    {
      /* error! */
      goto out;
    }

  if (current_address != NULL)
    {
      /* someone saved the address in the meantime */
      free (current_address);
      goto out;
    }

  /* Create our window */
  wid = XCreateWindow (xdisplay, RootWindow (xdisplay, 0), -20, -20, 10, 10,
                       0, CopyFromParent, InputOnly, CopyFromParent,
                       0, NULL);
  /* The type of a Window varies, so cast it to something reasonable */
  verbose ("Created window %lu\n", (unsigned long) wid);

  /* Save the property in the window */
  XChangeProperty (xdisplay, wid, address_atom, XA_STRING, 8, PropModeReplace,
                   (unsigned char *)address, strlen (address));
  pid32 = pid;
  XChangeProperty (xdisplay, wid, pid_atom, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&pid32, 1);

  /* Now grab the selection */
  XSetSelectionOwner (xdisplay, selection_atom, wid, CurrentTime);

 out:
  /* Ungrab the server to let other people use it too */
  XUngrabServer (xdisplay);

  /* And make sure that the ungrab gets sent to X11 */
  XFlush (xdisplay);

  return wid;
}

/*
 * Saves the session address in session file. Returns TRUE on
 * success, FALSE if an error occurs.
 */
static int
set_address_in_file (char *address, pid_t pid, Window wid)
{
  char *session_file;
  FILE *f;

  ensure_session_directory ();
  session_file = get_session_file();
  if (session_file == NULL)
    return FALSE;

  f = fopen (session_file, "w");
  free (session_file);
  if (f == NULL)
    return FALSE;               /* some kind of error */
  fprintf (f,
           "# This file allows processes on the machine with id %s using \n"
           "# display %s to find the D-Bus session bus with the below address.\n"
           "# If the DBUS_SESSION_BUS_ADDRESS environment variable is set, it will\n"
           "# be used rather than this file.\n"
           "# See \"man dbus-launch\" for more details.\n"
           "DBUS_SESSION_BUS_ADDRESS='%s'\n"
           "DBUS_SESSION_BUS_PID=%ld\n"
           "DBUS_SESSION_BUS_WINDOWID=%ld\n",
           get_machine_uuid (),
           getenv ("DISPLAY"),
           address, (long)pid, (long)wid);

  fclose (f);

  return TRUE;
}

int
x11_save_address (char *address, pid_t pid, long *wid)
{
  Window id = set_address_in_x11 (address, pid);
  if (id != None)
    {
      if (!set_address_in_file (address, pid, id))
        return FALSE;

      if (wid != NULL)
        *wid = (long) id;
      return TRUE;
    }
  return FALSE;
}

int
x11_init (void)
{
  return open_x11 () != NULL && init_x_atoms (xdisplay);
}

void
x11_handle_event (void)
{
  if (xdisplay != NULL)
    {      
      while (XPending (xdisplay))
        {
          XEvent ignored;
          XNextEvent (xdisplay, &ignored);
        }
    }
}  

#else
void dummy_dbus_launch_x11 (void);

void dummy_dbus_launch_x11 (void) { }
#endif
