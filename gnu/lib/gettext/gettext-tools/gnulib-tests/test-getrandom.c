/* Test of getting random bytes.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible.  */

#include <config.h>

#include <sys/random.h>

#include "signature.h"
SIGNATURE_CHECK (getrandom, ssize_t, (void *, size_t, unsigned int));

#include <errno.h>
#include <string.h>

#include "macros.h"

int
main (void)
{
  char buf1[8];
  char buf2[8];
  char large_buf[100000];
  ssize_t ret;

  /* Check that different calls produce different results (with a high
     probability).  */
  ret = getrandom (buf1, sizeof (buf1), 0);
  if (ret < 0)
    ASSERT (errno == ENOSYS);
  else
    {
      ret = getrandom (buf2, sizeof (buf2), 0);
      if (ret < 0)
        ASSERT (errno == ENOSYS);
      else
        {
          /* It is very unlikely that two calls to getrandom produce the
             same results.  */
          ASSERT (memcmp (buf1, buf2, sizeof (buf1)) != 0);
        }
    }

  /* Likewise with the "truly random" number generator.  */
  ret = getrandom (buf1, sizeof (buf1), GRND_RANDOM);
  if (ret < 0)
    ASSERT (errno == ENOSYS);
  else
    {
      ret = getrandom (buf2, sizeof (buf2), GRND_RANDOM);
      if (ret < 0)
        ASSERT (errno == ENOSYS);
      else
        {
          /* It is very unlikely that two calls to getrandom produce the
             same results.  */
          ASSERT (memcmp (buf1, buf2, sizeof (buf1)) != 0);
        }
    }

  /* Check that GRND_NONBLOCK works.  */
  ret = getrandom (large_buf, sizeof (large_buf), GRND_RANDOM | GRND_NONBLOCK);
  ASSERT (ret <= (ssize_t) sizeof (large_buf));
  /* It is very unlikely that so many truly random bytes were ready.  */
  if (ret < 0)
    ASSERT (errno == ENOSYS || errno == EAGAIN
            || errno == EINVAL /* Solaris */);
  else
    ASSERT (ret > 0);

  if (getrandom (buf1, 1, 0) < 1)
    if (getrandom (buf1, 1, GRND_RANDOM) < 1)
      {
        fputs ("Skipping test: getrandom is ineffective\n", stderr);
        return 77;
      }

  return 0;
}
