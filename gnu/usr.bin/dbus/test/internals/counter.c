/*
 * Copyright (c) 2023 Ralf Habacker
 * SPDX-License-Identifier: MIT
 */

#include <config.h>

#include "dbus/dbus-resources.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-test-tap.h"
#include "test/test-utils.h"

#include <pthread.h>

static dbus_bool_t verbose = FALSE;
static DBusCounter *counter = NULL;

static void* unix_fd_thread1 (void *arg _DBUS_GNUC_UNUSED)
{
  long i;
  for (i=0; i < 10000; i++)
    {
      long j;
      _dbus_counter_adjust_unix_fd (counter, 1);
      j = _dbus_counter_get_unix_fd_value (counter);
      if (verbose)
        _dbus_test_diag ("write %ld\n", j);
    }
  return NULL;
}

static void* unix_fd_thread2 (void *arg _DBUS_GNUC_UNUSED)
{
  long j = 0;
  do
    {
      j = _dbus_counter_get_unix_fd_value (counter);
      if (verbose)
        _dbus_test_diag ("read %ld\n", j);
    } while (j < 10000);
  return NULL;
}

static void* size_value_thread1 (void *arg _DBUS_GNUC_UNUSED)
{
  long i;
  for (i=0; i < 10000; i++)
    {
      long j;
      _dbus_counter_adjust_size (counter, 1);
      j = _dbus_counter_get_size_value (counter);
      if (verbose)
        _dbus_test_diag ("write %ld\n", j);
    }
  return NULL;
}

static void* size_value_thread2 (void *arg _DBUS_GNUC_UNUSED)
{
  long j = 0;
  do
    {
      j = _dbus_counter_get_size_value (counter);
      if (verbose)
        _dbus_test_diag("read %ld\n", j);
    } while (j < 10000);
  return NULL;
}

static dbus_bool_t
_dbus_counter_unix_fd_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  pthread_t tid[2];
  dbus_bool_t ret = TRUE;
  counter = _dbus_counter_new ();

  pthread_create (&(tid[0]), NULL, &unix_fd_thread1, NULL);
  pthread_create (&(tid[1]), NULL, &unix_fd_thread2, NULL);

  pthread_join (tid[0], NULL);
  pthread_join (tid[1], NULL);

  _dbus_counter_unref (counter);

  return ret;
}

static dbus_bool_t
_dbus_counter_size_value_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  pthread_t tid[2];
  dbus_bool_t ret = TRUE;
  counter = _dbus_counter_new ();

  pthread_create (&(tid[0]), NULL, &size_value_thread1, NULL);
  pthread_create (&(tid[1]), NULL, &size_value_thread2, NULL);

  pthread_join (tid[0], NULL);
  pthread_join (tid[1], NULL);

  _dbus_counter_unref (counter);

  return ret;
}

static const DBusTestCase test[] =
{
  { "unix_fd", _dbus_counter_unix_fd_test },
  { "size", _dbus_counter_size_value_test },
};


int
main (int    argc,
      char **argv)
{
  return _dbus_test_main (argc, argv, sizeof(test) / sizeof (DBusTestCase), test,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
