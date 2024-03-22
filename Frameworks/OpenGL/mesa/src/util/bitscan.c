/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#include "bitscan.h"

#ifdef HAVE___BUILTIN_FFS
#elif defined(_MSC_VER) && (_M_IX86 || _M_ARM || _M_AMD64 || _M_IA64)
#else
int
ffs(int i)
{
   int bit = 0;
   if (!i)
      return bit;
   if (!(i & 0xffff)) {
      bit += 16;
      i >>= 16;
   }
   if (!(i & 0xff)) {
      bit += 8;
      i >>= 8;
   }
   if (!(i & 0xf)) {
      bit += 4;
      i >>= 4;
   }
   if (!(i & 0x3)) {
      bit += 2;
      i >>= 2;
   }
   if (!(i & 0x1))
      bit += 1;
   return bit + 1;
}
#endif

#ifdef HAVE___BUILTIN_FFSLL
#elif defined(_MSC_VER) && (_M_AMD64 || _M_ARM64 || _M_IA64)
#else
int
ffsll(long long int val)
{
   int bit;

   bit = ffs((unsigned) (val & 0xffffffff));
   if (bit != 0)
      return bit;

   bit = ffs((unsigned) (val >> 32));
   if (bit != 0)
      return 32 + bit;

   return 0;
}
#endif
