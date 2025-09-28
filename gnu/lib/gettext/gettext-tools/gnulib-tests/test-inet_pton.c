/* Test of inet_pton function.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2009.  */

#include <config.h>

#include <arpa/inet.h>

#include "signature.h"
SIGNATURE_CHECK (inet_pton, int, (int, const char *, void *));

#include <netinet/in.h>
#include <sys/socket.h>

#include "macros.h"

int
main (void)
{
#if defined AF_INET /* HAVE_IPV4 */
  {
    /* This machine was for a long time known as
       ma2s2.mathematik.uni-karlsruhe.de.  */
    const char printable[] = "129.13.115.2";
    struct in_addr internal;
    int ret;

    ret = inet_pton (AF_INET, printable, &internal);
    ASSERT (ret == 1);
    /* Verify that internal is filled in network byte order.  */
    ASSERT (((unsigned char *) &internal)[0] == 0x81);
    ASSERT (((unsigned char *) &internal)[1] == 0x0D);
    ASSERT (((unsigned char *) &internal)[2] == 0x73);
    ASSERT (((unsigned char *) &internal)[3] == 0x02);
# ifdef WORDS_BIGENDIAN
    ASSERT (internal.s_addr == 0x810D7302);
# else
    ASSERT (internal.s_addr == 0x02730D81);
# endif
  }
#endif

  return 0;
}
