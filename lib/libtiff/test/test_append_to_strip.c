/*
 * Copyright (c) 2022, Even Rouault <even.rouault at spatialys.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library
 *
 * Test TIFFAppendToStrip() through TIFFWriteRawStrip()
 * Scenario of https://gitlab.com/libtiff/libtiff/-/issues/489
 */

#include "tif_config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "tiffio.h"

int main()
{
    int ret;
    TIFF *tif = TIFFOpen("test_append_to_strip.tif", "w");
    if (tif == NULL)
    {
        fprintf(stderr, "Cannot create test_append_to_strip.tif");
        return 1;
    }
    ret = TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    assert(ret);
    ret = TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 1);
    assert(ret);
    ret = TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 9);
    assert(ret);
    ret = TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 5);
    assert(ret);
    ret = TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    assert(ret);
    ret = TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    assert(ret);
    int row;
    for (row = 0; row < 9; ++row)
    {
        char c = (char)row;
        int strip = (row < 5) ? 0 : 1;
        ret = TIFFWriteRawStrip(tif, strip, &c, 1);
        assert(ret == 1);
    }
    (void)ret;
    TIFFClose(tif);
    tif = TIFFOpen("test_append_to_strip.tif", "r");
    if (tif == NULL)
    {
        fprintf(stderr, "Cannot create test_append_to_strip.tif");
        return 1;
    }
    toff_t *panByteCounts = NULL;
    TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &panByteCounts);
    assert(panByteCounts);
    int exitCode = 0;
    if (panByteCounts[0] != 5)
    {
        exitCode = 1;
        fprintf(stderr, "panByteCounts[0] = %d. Expected 5.\n",
                (int)panByteCounts[0]);
    }
    if (panByteCounts[1] != 4)
    {
        exitCode = 1;
        fprintf(stderr, "panByteCounts[1] = %d. Expected 4.\n",
                (int)panByteCounts[1]);
    }
    for (row = 0; row < 9; ++row)
    {
        char c;
        TIFFReadScanline(tif, &c, row, 0);
        if (c != row)
        {
            fprintf(
                stderr,
                "TIFFReadScanline() for scanline %d returned %d. %d expected\n",
                row, c, row);
            exitCode = 1;
        }
    }
    TIFFClose(tif);
    unlink("test_append_to_strip.tif");
    return exitCode;
}
