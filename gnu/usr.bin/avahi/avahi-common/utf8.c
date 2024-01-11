/* This file is based on the GLIB utf8 validation functions. The
 * original license text follows. */

/* gutf8.c - Operations on UTF-8 strings.
 *
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "utf8.h"

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     (((Char) & 0xFFFFF800) != 0xD800) &&     \
     ((Char) < 0xFDD0 || (Char) > 0xFDEF) &&  \
     ((Char) & 0xFFFE) != 0xFFFE)


#define CONTINUATION_CHAR                           \
 do {                                     \
  if ((*(const unsigned char *)p & 0xc0) != 0x80) /* 10xxxxxx */ \
    goto error;                                     \
  val <<= 6;                                        \
  val |= (*(const unsigned char *)p) & 0x3f;                     \
 } while(0)


const char *
avahi_utf8_valid (const char *str)

{
  unsigned val = 0;
  unsigned min = 0;
  const char *p;

  for (p = str; *p; p++)
    {
      if (*(const unsigned char *)p < 128)
	/* done */;
      else
	{
	  if ((*(const unsigned char *)p & 0xe0) == 0xc0) /* 110xxxxx */
	    {
	      if ( ((*(const unsigned char *)p & 0x1e) == 0))
		goto error;
	      p++;
	      if ( ((*(const unsigned char *)p & 0xc0) != 0x80)) /* 10xxxxxx */
		goto error;
	    }
	  else
	    {
	      if ((*(const unsigned char *)p & 0xf0) == 0xe0) /* 1110xxxx */
		{
		  min = (1 << 11);
		  val = *(const unsigned char *)p & 0x0f;
		  goto TWO_REMAINING;
		}
	      else if ((*(const unsigned char *)p & 0xf8) == 0xf0) /* 11110xxx */
		{
		  min = (1 << 16);
		  val = *(const unsigned char *)p & 0x07;
		}
	      else
		goto error;

	      p++;
	      CONTINUATION_CHAR;
	    TWO_REMAINING:
	      p++;
	      CONTINUATION_CHAR;
	      p++;
	      CONTINUATION_CHAR;

	      if ( (val < min))
		goto error;

	      if ( (!UNICODE_VALID(val)))
		goto error;
	    }

	  continue;

	error:
	  return NULL;
	}
    }

  return str;
}
