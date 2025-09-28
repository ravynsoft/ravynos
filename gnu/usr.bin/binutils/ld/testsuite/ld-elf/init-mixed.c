#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_INITFINI_ARRAY
static int count;

static void
init1005 ()
{
  if (count != 0)
    abort ();
  count = 1005;
}
void (*const init_array1005[]) ()
  __attribute__ ((section (".init_array.01005"), aligned (sizeof (void *))))
  = { init1005 };
static void
fini1005 ()
{
  if (count != 1005)
    abort ();
}
void (*const fini_array1005[]) ()
  __attribute__ ((section (".fini_array.01005"), aligned (sizeof (void *))))
  = { fini1005 };

static void
ctor1007a ()
{
  if (count != 1005)
    abort ();
  count = 1006;
}
static void
ctor1007b ()
{
  if (count != 1006)
    abort ();
  count = 1007;
}
void (*const ctors1007[]) ()
  __attribute__ ((section (".ctors.64528"), aligned (sizeof (void *))))
  = { ctor1007b, ctor1007a };
static void
dtor1007a ()
{
  if (count != 1006)
    abort ();
  count = 1005;
}
static void
dtor1007b ()
{
  if (count != 1007)
    abort ();
  count = 1006;
}
void (*const dtors1007[]) ()
  __attribute__ ((section (".dtors.64528"), aligned (sizeof (void *))))
  = { dtor1007b, dtor1007a };

static void
init65530 ()
{
  if (count != 1007)
    abort ();
  count = 65530;
}
void (*const init_array65530[]) ()
  __attribute__ ((section (".init_array.65530"), aligned (sizeof (void *))))
  = { init65530 };
static void
fini65530 ()
{
  if (count != 65530)
    abort ();
  count = 1007;
}
void (*const fini_array65530[]) ()
  __attribute__ ((section (".fini_array.65530"), aligned (sizeof (void *))))
  = { fini65530 };

static void
ctor65535a ()
{
  if (count != 65530)
    abort ();
  count = 65535;
}
static void
ctor65535b ()
{
  if (count != 65535)
    abort ();
  count = 65536;
}
void (*const ctors65535[]) ()
  __attribute__ ((section (".ctors"), aligned (sizeof (void *))))
  = { ctor65535b, ctor65535a };
static void
dtor65535b ()
{
  if (count != 65536)
    abort ();
  count = 65535;
}
static void
dtor65535a ()
{
  if (count != 65535)
    abort ();
  count = 65530;
}
void (*const dtors65535[]) ()
  __attribute__ ((section (".dtors"), aligned (sizeof (void *))))
  = { dtor65535b, dtor65535a };
#endif

int
main ()
{
  printf ("OK\n");
  return 0;
}
