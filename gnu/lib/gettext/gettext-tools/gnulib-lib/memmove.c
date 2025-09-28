/* memmove.c -- copy memory.
   This file is in the public domain.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include <config.h>

#include <stddef.h>

/* Copy LENGTH bytes from SOURCE to DEST.  Does not null-terminate.  */

void *
memmove (void *dest0, void const *source0, size_t length)
{
  char *dest = dest0;
  char const *source = source0;
  if (source < dest)
    /* Moving from low mem to hi mem; start at end.  */
    for (source += length, dest += length; length; --length)
      *--dest = *--source;
  else if (source != dest)
    {
      /* Moving from hi mem to low mem; start at beginning.  */
      for (; length; --length)
        *dest++ = *source++;
    }
  return dest0;
}
