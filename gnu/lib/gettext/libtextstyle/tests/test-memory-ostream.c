/* Test for the memory-ostream API.  */

#include <config.h>

#include "memory-ostream.h"

#include <stdlib.h>
#include <string.h>

int
main ()
{
  memory_ostream_t stream = memory_ostream_create ();

  ostream_write_str (stream, "foo");
  ostream_printf (stream, "%d%d", 73, 55);
  ostream_write_str (stream, "\n");

  {
    const void *buf;
    size_t buflen;
    memory_ostream_contents (stream, &buf, &buflen);

    if (!(buflen == 8))
      exit (2);
    if (!(memcmp (buf, "foo7355\n", 8) == 0))
      exit (3);

    ostream_free (stream);
  }

  return 0;
}
