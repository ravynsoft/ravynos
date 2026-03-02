/*
 * Copyright (C)2021-2023 D. R. Commander.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the libjpeg-turbo Project nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <turbojpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>


#define NUMTESTS  7


struct test {
  enum TJPF pf;
  int psv, pt;
};


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  tjhandle handle = NULL;
  unsigned char *srcBuf = NULL, *dstBuf = NULL;
  int width = 0, height = 0, fd = -1, i, ti;
  char filename[FILENAME_MAX] = { 0 };
  struct test tests[NUMTESTS] = {
    { TJPF_RGB, 1, 0 },
    { TJPF_BGR, 2, 2 },
    { TJPF_RGBX, 3, 4 },
    { TJPF_BGRA, 4, 7 },
    { TJPF_XRGB, 5, 5 },
    { TJPF_GRAY, 6, 3 },
    { TJPF_CMYK, 7, 0 }
  };
#if defined(__has_feature) && __has_feature(memory_sanitizer)
  char env[18] = "JSIMD_FORCENONE=1";

  /* The libjpeg-turbo SIMD extensions produce false positives with
     MemorySanitizer. */
  putenv(env);
#endif

  snprintf(filename, FILENAME_MAX, "/tmp/libjpeg-turbo_compress_fuzz.XXXXXX");
  if ((fd = mkstemp(filename)) < 0 || write(fd, data, size) < 0)
    goto bailout;

  if ((handle = tj3Init(TJINIT_COMPRESS)) == NULL)
    goto bailout;

  for (ti = 0; ti < NUMTESTS; ti++) {
    int sum = 0, pf = tests[ti].pf;
    size_t dstSize = 0, maxBufSize;

    /* Test non-default compression options on specific iterations. */
    tj3Set(handle, TJPARAM_BOTTOMUP, ti == 0);
    tj3Set(handle, TJPARAM_NOREALLOC, ti != 2);
    tj3Set(handle, TJPARAM_RESTARTROWS, ti == 0 || ti == 6 ? 1 : 0);

    tj3Set(handle, TJPARAM_MAXPIXELS, 1048576);
    /* tj3LoadImage8() will refuse to load images larger than 1 Megapixel, so
       we don't need to check the width and height here. */
    if ((srcBuf = tj3LoadImage8(handle, filename, &width, 1, &height,
                                &pf)) == NULL)
      continue;

    maxBufSize = tj3JPEGBufSize(width, height, TJSAMP_444);
    if (tj3Get(handle, TJPARAM_NOREALLOC)) {
      if ((dstBuf = (unsigned char *)malloc(maxBufSize)) == NULL)
        goto bailout;
    } else
      dstBuf = NULL;

    tj3Set(handle, TJPARAM_LOSSLESS, 1);
    tj3Set(handle, TJPARAM_LOSSLESSPSV, tests[ti].psv);
    tj3Set(handle, TJPARAM_LOSSLESSPT, tests[ti].pt);
    if (tj3Compress8(handle, srcBuf, width, 0, height, pf, &dstBuf,
                     &dstSize) == 0) {
      /* Touch all of the output pixels in order to catch uninitialized reads
         when using MemorySanitizer. */
      for (i = 0; i < dstSize; i++)
        sum += dstBuf[i];
    }

    free(dstBuf);
    dstBuf = NULL;
    tj3Free(srcBuf);
    srcBuf = NULL;

    /* Prevent the code above from being optimized out.  This test should never
       be true, but the compiler doesn't know that. */
    if (sum > 255 * maxBufSize)
      goto bailout;
  }

bailout:
  free(dstBuf);
  tj3Free(srcBuf);
  if (fd >= 0) {
    close(fd);
    if (strlen(filename) > 0) unlink(filename);
  }
  tj3Destroy(handle);
  return 0;
}
