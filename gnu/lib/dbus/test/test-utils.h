/*
 * Copyright 2002-2009 Red Hat Inc.
 * Copyright 2011-2017 Collabora Ltd.
 * Copyright 2017 Endless Mobile, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus.h>

#include <dbus/dbus-mainloop.h>
#include <dbus/dbus-internals.h>
#include <dbus/dbus-types.h>

#include "dbus/dbus-test-tap.h"

typedef DBusLoop TestMainContext;

_DBUS_WARN_UNUSED_RESULT
TestMainContext *test_main_context_get            (void);
_DBUS_WARN_UNUSED_RESULT
TestMainContext *test_main_context_try_get        (void);
TestMainContext *test_main_context_ref            (TestMainContext *ctx);
void             test_main_context_unref          (TestMainContext *ctx);
void             test_main_context_iterate        (TestMainContext *ctx,
                                                   dbus_bool_t      may_block);

_DBUS_WARN_UNUSED_RESULT
dbus_bool_t test_connection_try_setup             (TestMainContext *ctx,
                                                   DBusConnection *connection);
void        test_connection_setup                 (TestMainContext *ctx,
                                                   DBusConnection *connection);
void        test_connection_shutdown              (TestMainContext *ctx,
                                                   DBusConnection *connection);

_DBUS_WARN_UNUSED_RESULT
dbus_bool_t test_server_try_setup                 (TestMainContext *ctx,
                                                   DBusServer    *server);
void        test_server_setup                     (TestMainContext *ctx,
                                                   DBusServer    *server);
void        test_server_shutdown                  (TestMainContext *ctx,
                                                   DBusServer    *server);
void        test_pending_call_store_reply         (DBusPendingCall *pc,
                                                   void *data);

typedef struct
{
  const char *name;
  dbus_bool_t (*func) (const char *test_data_dir);
} DBusTestCase;

typedef enum
{
  DBUS_TEST_FLAGS_REQUIRE_DATA = (1 << 0),
  DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS = (1 << 1),
  DBUS_TEST_FLAGS_CHECK_FD_LEAKS = (1 << 2),
  DBUS_TEST_FLAGS_NONE = 0
} DBusTestFlags;

int _dbus_test_main (int                  argc,
                     char               **argv,
                     size_t               n_tests,
                     const DBusTestCase  *tests,
                     DBusTestFlags        flags,
                     void               (*test_pre_hook) (void),
                     void               (*test_post_hook) (void));

_DBUS_WARN_UNUSED_RESULT
dbus_bool_t _dbus_test_append_different_uid (DBusString *uid);

#ifdef DBUS_UNIX
_DBUS_WARN_UNUSED_RESULT
dbus_bool_t _dbus_test_append_different_username (DBusString *username);
#endif

typedef struct DBusInitialFDs DBusInitialFDs;
DBusInitialFDs *_dbus_check_fdleaks_enter (void);
void            _dbus_check_fdleaks_leave (DBusInitialFDs *fds,
                                           const char     *context);

#endif
