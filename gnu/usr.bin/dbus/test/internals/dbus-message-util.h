/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2002-2009 Red Hat, Inc.
 * Copyright 2002-2003 CodeFactory AB
 * Copyright 2007-2018 Collabora Ltd.
 * Copyright 2009 Scott James Remnant / Canonical Ltd.
 * Copyright 2009 William Lachance
 * Copyright 2010 Christian Dywan / Lanedo
 * Copyright 2013 Chengwei Yang / Intel
 * Copyright 2013 Vasiliy Balyasnyy / Samsung
 * Copyright 2014 Ralf Habacker
 * Copyright 2017 Endless Mobile, Inc.
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

#ifndef TEST_INTERNALS_DBUS_MESSAGE_UTIL_H
#define TEST_INTERNALS_DBUS_MESSAGE_UTIL_H

#include <dbus/dbus-types.h>

dbus_bool_t _dbus_message_test (const char *test_data_dir);

#endif
