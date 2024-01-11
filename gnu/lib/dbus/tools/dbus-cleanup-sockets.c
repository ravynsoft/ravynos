/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-cleanup-sockets.c  dbus-cleanup-sockets utility
 *
 * Copyright (C) 2003 Red Hat, Inc.
 * Copyright (C) 2002 Michael Meeks
 *
 * Note that this file is NOT licensed under the Academic Free License,
 * as it is based on linc-cleanup-sockets which is LGPL.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus-macros.h>

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

static void*
xmalloc (size_t bytes)
{
  void *mem;

  if (bytes == 0)
    return NULL;

  mem = malloc (bytes);

  if (mem == NULL)
    {
      fprintf (stderr, "Allocation of %d bytes failed\n",
               (int) bytes);
      exit (1);
    }

  return mem;
}

static void*
xrealloc (void *old, size_t bytes)
{
  void *mem;

  if (bytes == 0)
    {
      free (old);
      return NULL;
    }

  mem = realloc (old, bytes);

  if (mem == NULL)
    {
      fprintf (stderr, "Reallocation of %d bytes failed\n",
               (int) bytes);
      exit (1);
    }

  return mem;
}

#ifdef AF_UNIX

typedef enum
  {
    SOCKET_UNKNOWN,
    SOCKET_FAILED_TO_HANDLE,
    SOCKET_DEAD,
    SOCKET_ALIVE,
    SOCKET_UNLINKED
  } SocketStatus;

static int alive_count = 0;
static int cleaned_count = 0;
static int unhandled_count = 0;

typedef struct
{
  char *name;
  int   fd;
  SocketStatus status;
  int   n_retries;
} SocketEntry;

static SocketEntry*
socket_entry_new (const char *dir,
                  const char *fname)
{
  SocketEntry *se;
  int len;

  se = xmalloc (sizeof (SocketEntry));

  len = strlen (dir) + strlen (fname) + 2; /* 2 = nul and '/' */
  se->name = xmalloc (len);

  strcpy (se->name, dir);
  strcat (se->name, "/");
  strcat (se->name, fname);

  se->fd = -1;

  se->status = SOCKET_UNKNOWN;

  se->n_retries = 0;

  return se;
}

static void
free_socket_entry (SocketEntry *se)
{
  if (se)
    {
      free (se->name);
      if (se->fd >= 0)
        close (se->fd);
      free (se);
    }
}

static void
free_socket_entries (SocketEntry** entries,
                     int           n_entries)
{
  int i;

  if (entries)
    {
      for (i = 0; i < n_entries; ++i)
        free_socket_entry (entries[i]);
      free (entries);
    }
}

static void
read_sockets (const char    *dir,
              SocketEntry ***entries_p,
              int           *n_entries_p)
{
  DIR   *dirh;
  struct dirent *dent;
  SocketEntry **entries;
  int n_entries;
  int allocated;

  n_entries = 0;
  allocated = 2;
  entries = xmalloc (sizeof (SocketEntry*) * allocated);

  dirh = opendir (dir);
  if (dirh == NULL)
    {
      fprintf (stderr, "Failed to open directory %s: %s\n",
               dir, strerror (errno));
      exit (1);
    }

  while ((dent = readdir (dirh)))
    {
      SocketEntry *se;

      if (strncmp (dent->d_name, "dbus-", 5) != 0)
        continue;

      se = socket_entry_new (dir, dent->d_name);

      if (n_entries == allocated)
        {
          allocated *= 2;
          entries = xrealloc (entries, sizeof (SocketEntry*) * allocated);
        }

      entries[n_entries] = se;
      n_entries += 1;
    }

  closedir (dirh);

  *entries_p = entries;
  *n_entries_p = n_entries;
}

static SocketStatus
open_socket (SocketEntry *se)
{
  int ret;
  struct sockaddr_un saddr;

  if (se->n_retries > 5)
    {
      fprintf (stderr, "Warning: giving up on socket %s after several retries; unable to determine socket's status\n",
               se->name);
      return SOCKET_FAILED_TO_HANDLE;
    }

  se->n_retries += 1;

  se->fd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (se->fd < 0)
    {
      fprintf (stderr, "Warning: failed to open a socket to use for connecting: %s\n",
               strerror (errno));
      return SOCKET_UNKNOWN;
    }

  if (fcntl (se->fd, F_SETFL, O_NONBLOCK) < 0)
    {
      fprintf (stderr, "Warning: failed set socket %s nonblocking: %s\n",
               se->name, strerror (errno));
      return SOCKET_UNKNOWN;
    }


  memset (&saddr, '\0', sizeof (saddr)); /* nul-terminates the sun_path */

  saddr.sun_family = AF_UNIX;
  strncpy (saddr.sun_path, se->name, sizeof (saddr.sun_path) - 1);

  do
    {
      ret = connect (se->fd, (struct sockaddr*) &saddr, sizeof (saddr));
    }
  while (ret < 0 && errno == EINTR);

  if (ret >= 0)
    return SOCKET_ALIVE;
  else
    {
      switch (errno)
        {
        case EINPROGRESS:
        case EAGAIN:
          return SOCKET_UNKNOWN;
        case ECONNREFUSED:
          return SOCKET_DEAD;
        default:
          fprintf (stderr, "Warning: unexpected error connecting to socket %s: %s\n",
                   se->name, strerror (errno));
          return SOCKET_FAILED_TO_HANDLE;
        }
    }
}

static int
handle_sockets (SocketEntry **entries,
                int           n_entries)
{
  int i;
  int n_unknown;

  n_unknown = 0;

  i = 0;
  while (i < n_entries)
    {
      SocketEntry *se;
      SocketStatus status;

      se = entries[i];
      ++i;

      if (se->fd >= 0)
        {
          fprintf (stderr, "Internal error, socket has fd  kept open while status = %d\n",
                   se->status);
          exit (1);
        }

      if (se->status != SOCKET_UNKNOWN)
        continue;

      status = open_socket (se);

      switch (status)
        {
        case SOCKET_DEAD:
          cleaned_count += 1;
          if (unlink (se->name) < 0)
            {
              fprintf (stderr, "Warning: Failed to delete %s: %s\n",
                       se->name, strerror (errno));

              se->status = SOCKET_FAILED_TO_HANDLE;
            }
          else
            se->status = SOCKET_UNLINKED;
          break;

        case SOCKET_ALIVE:
          alive_count += 1;
          /* FALL THRU */

        case SOCKET_FAILED_TO_HANDLE:
        case SOCKET_UNKNOWN:
          se->status = status;
          break;

        case SOCKET_UNLINKED:
        default:
          fprintf (stderr, "Bad status from open_socket(), should not happen\n");
          exit (1);
          break;
        }

      if (se->fd >= 0)
        {
          close (se->fd);
          se->fd = -1;
        }

      if (se->status == SOCKET_UNKNOWN)
        n_unknown += 1;
    }

  return n_unknown == 0;
}

static void
clean_dir (const char *dir)
{
  SocketEntry **entries;
  int n_entries;

  read_sockets (dir, &entries, &n_entries);

  /* open_socket() will fail conclusively after
   * several retries, so this loop is guaranteed
   * to terminate eventually
   */
  while (!handle_sockets (entries, n_entries))
    {
      fprintf (stderr, "Unable to determine state of some sockets, retrying in 2 seconds\n");
      sleep (2);
    }

  unhandled_count += (n_entries - alive_count - cleaned_count);

  free_socket_entries (entries, n_entries);
}

#endif /* AF_UNIX */

static void usage (int ecode) _DBUS_GNUC_NORETURN;
static void
usage (int ecode)
{
  fprintf (stderr, "dbus-cleanup-sockets [--version] [--help] <socketdir>\n");
  exit (ecode);
}

static void version (void) _DBUS_GNUC_NORETURN;
static void
version (void)
{
  printf ("D-Bus Socket Cleanup Utility %s\n"
          "Copyright (C) 2003 Red Hat, Inc.\n"
          "Copyright (C) 2002 Michael Meeks\n"
          "This is free software; see the source for copying conditions.\n"
          "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
          VERSION);
  exit (0);
}

int
main (int argc, char **argv)
{
  int i;
  int saw_doubledash;
  const char *dirname;

  saw_doubledash = FALSE;
  dirname = NULL;
  i = 1;
  while (i < argc)
    {
      const char *arg = argv[i];

      if (strcmp (arg, "--help") == 0 ||
          strcmp (arg, "-h") == 0 ||
          strcmp (arg, "-?") == 0)
        usage (0);
      else if (strcmp (arg, "--version") == 0)
        version ();
      else if (!saw_doubledash)
	{
          if (strcmp (arg, "--") == 0)
            saw_doubledash = TRUE;
          else if (*arg == '-')
            usage (1);
	}
      else
        {
          if (dirname != NULL)
            {
              fprintf (stderr, "dbus-cleanup-sockets only supports a single directory name\n");
              exit (1);
            }

          dirname = arg;
        }

      ++i;
    }

  /* Default to session socket dir, usually /tmp */
  if (dirname == NULL)
    dirname = DBUS_SESSION_SOCKET_DIR;

#ifdef AF_UNIX
  clean_dir (dirname);

  printf ("Cleaned up %d sockets in %s; %d sockets are still in use; %d in unknown state\n",
          cleaned_count, dirname, alive_count, unhandled_count);
#else
  printf ("This system does not support UNIX domain sockets, so dbus-cleanup-sockets does nothing\n");
#endif

  return 0;
}
