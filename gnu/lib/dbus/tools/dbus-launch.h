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

#ifndef DBUS_LAUNCH_H
#define DBUS_LAUNCH_H

#include <sys/types.h>

#include <dbus/dbus.h>

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#undef  MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define MAX_ADDR_LEN 512

/* defined in dbus-launch.c */
void verbose (const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);
char *xstrdup (const char *str);
void kill_bus_and_exit (int exitcode) _DBUS_GNUC_NORETURN;

const char* get_machine_uuid (void);

#ifdef DBUS_BUILD_X11
/* defined in dbus-launch-x11.c */
int x11_init (void);
int x11_get_address (char **paddress, pid_t *pid, long *wid);
int x11_save_address (char *address, pid_t pid, long *wid);
void x11_handle_event (void);
#endif

#endif
