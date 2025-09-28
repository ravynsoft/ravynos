/* inet_pton.c -- convert IPv4 and IPv6 addresses from text to binary form

   Copyright (C) 2006, 2008-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/*
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <config.h>

/* Specification.  */
#include <arpa/inet.h>

#if HAVE_DECL_INET_PTON

# undef inet_pton

int
rpl_inet_pton (int af, const char *restrict src, void *restrict dst)
{
  return inet_pton (af, src, dst);
}

#else

# include <c-ctype.h>
# include <string.h>
# include <errno.h>

# define NS_INADDRSZ 4
# define NS_IN6ADDRSZ 16
# define NS_INT16SZ 2

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4 (const char *src, unsigned char *dst);
# if HAVE_IPV6
static int inet_pton6 (const char *src, unsigned char *dst);
# endif

/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid ('dst' is untouched in this case)
 *      -1 if some other error occurred ('dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
int
inet_pton (int af, const char *restrict src, void *restrict dst)
{
  switch (af)
    {
    case AF_INET:
      return (inet_pton4 (src, dst));

# if HAVE_IPV6
    case AF_INET6:
      return (inet_pton6 (src, dst));
# endif

    default:
      errno = EAFNOSUPPORT;
      return (-1);
    }
  /* NOTREACHED */
}

/* int
 * inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal, octal (with the
 *      exception of 0) and shorthand.
 * return:
 *      1 if 'src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch 'dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton4 (const char *restrict src, unsigned char *restrict dst)
{
  int saw_digit, octets, ch;
  unsigned char tmp[NS_INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
    {

      if (ch >= '0' && ch <= '9')
        {
          unsigned new = *tp * 10 + (ch - '0');

          if (saw_digit && *tp == 0)
            return (0);
          if (new > 255)
            return (0);
          *tp = new;
          if (!saw_digit)
            {
              if (++octets > 4)
                return (0);
              saw_digit = 1;
            }
        }
      else if (ch == '.' && saw_digit)
        {
          if (octets == 4)
            return (0);
          *++tp = 0;
          saw_digit = 0;
        }
      else
        return (0);
    }
  if (octets < 4)
    return (0);
  memcpy (dst, tmp, NS_INADDRSZ);
  return (1);
}

# if HAVE_IPV6

/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if 'src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch 'dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton6 (const char *restrict src, unsigned char *restrict dst)
{
  static const char xdigits[] = "0123456789abcdef";
  unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
  const char *curtok;
  int ch, saw_xdigit;
  unsigned val;

  tp = memset (tmp, '\0', NS_IN6ADDRSZ);
  endp = tp + NS_IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if (*src == ':')
    if (*++src != ':')
      return (0);
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while ((ch = c_tolower (*src++)) != '\0')
    {
      const char *pch;

      pch = strchr (xdigits, ch);
      if (pch != NULL)
        {
          val <<= 4;
          val |= (pch - xdigits);
          if (val > 0xffff)
            return (0);
          saw_xdigit = 1;
          continue;
        }
      if (ch == ':')
        {
          curtok = src;
          if (!saw_xdigit)
            {
              if (colonp)
                return (0);
              colonp = tp;
              continue;
            }
          else if (*src == '\0')
            {
              return (0);
            }
          if (tp + NS_INT16SZ > endp)
            return (0);
          *tp++ = (u_char) (val >> 8) & 0xff;
          *tp++ = (u_char) val & 0xff;
          saw_xdigit = 0;
          val = 0;
          continue;
        }
      if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
          inet_pton4 (curtok, tp) > 0)
        {
          tp += NS_INADDRSZ;
          saw_xdigit = 0;
          break;                /* '\0' was seen by inet_pton4(). */
        }
      return (0);
    }
  if (saw_xdigit)
    {
      if (tp + NS_INT16SZ > endp)
        return (0);
      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) val & 0xff;
    }
  if (colonp != NULL)
    {
      /*
       * Since some memmove()'s erroneously fail to handle
       * overlapping regions, we'll do the shift by hand.
       */
      const int n = tp - colonp;
      int i;

      if (tp == endp)
        return (0);
      for (i = 1; i <= n; i++)
        {
          endp[-i] = colonp[n - i];
          colonp[n - i] = 0;
        }
      tp = endp;
    }
  if (tp != endp)
    return (0);
  memcpy (dst, tmp, NS_IN6ADDRSZ);
  return (1);
}

# endif

#endif
