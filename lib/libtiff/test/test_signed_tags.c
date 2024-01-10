/*
 * Copyright (c) 2022, Su Laus  @Su_Laus
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

#include <stdio.h>

#include "tif_config.h" /* necessary for linux compiler to get HAVE_UNISTD_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for unlink() on linux */
#endif

#include <tiffio.h>

#define FAULT_RETURN 1
#define OK_RETURN 0
#define GOTOFAILURE goto failure;

#define N(a) (sizeof(a) / sizeof(a[0]))

enum
{
    SINT8 = 65100,
    SINT16,
    SINT32,
    SINT64,
    C0_SINT8,
    C0_SINT16,
    C0_SINT32,
    C0_SINT64,
    C16_SINT8,
    C16_SINT16,
    C16_SINT32,
    C16_SINT64,
    C32_SINT8,
    C32_SINT16,
    C32_SINT32,
    C32_SINT64,
};

static const TIFFFieldInfo tiff_field_info[] = {
    {SINT8, 1, 1, TIFF_SBYTE, FIELD_CUSTOM, 0, 0, "SINT8"},
    {SINT16, 1, 1, TIFF_SSHORT, FIELD_CUSTOM, 0, 0, "SINT16"},
    {SINT32, 1, 1, TIFF_SLONG, FIELD_CUSTOM, 0, 0, "SINT32"},
    {SINT64, 1, 1, TIFF_SLONG8, FIELD_CUSTOM, 0, 0, "SINT64"},
    {C0_SINT8, 6, 6, TIFF_SBYTE, FIELD_CUSTOM, 0, 0, "C0_SINT8"},
    {C0_SINT16, 6, 6, TIFF_SSHORT, FIELD_CUSTOM, 0, 0, "C0_SINT16"},
    {C0_SINT32, 6, 6, TIFF_SLONG, FIELD_CUSTOM, 0, 0, "C0_SINT32"},
    {C0_SINT64, 6, 6, TIFF_SLONG8, FIELD_CUSTOM, 0, 0, "C0_SINT64"},
    {C16_SINT8, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SBYTE, FIELD_CUSTOM, 0, 1,
     "C16_SINT8"},
    {C16_SINT16, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SSHORT, FIELD_CUSTOM, 0, 1,
     "C16_SINT16"},
    {C16_SINT32, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SLONG, FIELD_CUSTOM, 0, 1,
     "C16_SINT32"},
    {C16_SINT64, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SLONG8, FIELD_CUSTOM, 0, 1,
     "C16_SINT64"},
    {C32_SINT8, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SBYTE, FIELD_CUSTOM, 0, 1,
     "C32_SINT8"},
    {C32_SINT16, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SSHORT, FIELD_CUSTOM, 0,
     1, "C32_SINT16"},
    {C32_SINT32, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SLONG, FIELD_CUSTOM, 0, 1,
     "C32_SINT32"},
    {C32_SINT64, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SLONG8, FIELD_CUSTOM, 0,
     1, "C32_SINT64"},
};

static TIFFExtendProc parent = NULL;

static void extender(TIFF *tif)
{
    TIFFMergeFieldInfo(tif, tiff_field_info,
                       sizeof(tiff_field_info) / sizeof(tiff_field_info[0]));
    if (parent)
    {
        (*parent)(tif);
    }
}

/*-- Global test fields --*/
int8_t s8[] = {-8, -9, -10, -11, INT8_MAX, INT8_MIN};
int16_t s16[] = {-16, -17, -18, -19, INT16_MAX, INT16_MIN};
int32_t s32[] = {-32, -33, -34, -35, INT32_MAX, INT32_MIN};
int64_t s64[] = {-64, -65, -66, -67, INT64_MAX, INT64_MIN};

const uint32_t idxSingle = 0;

static int writeTestTiff(const char *szFileName, int isBigTiff)
{
    int ret;
    TIFF *tif;
    int retcode = FAULT_RETURN;

    unlink(szFileName);
    if (isBigTiff)
    {
        fprintf(stdout, "\n-- Writing signed values to BigTIFF...\n");
        tif = TIFFOpen(szFileName, "w8");
    }
    else
    {
        fprintf(stdout, "\n-- Writing signed values to ClassicTIFF...\n");
        tif = TIFFOpen(szFileName, "w");
    }
    if (!tif)
    {
        fprintf(stdout, "Can't create test TIFF file %s.\n", szFileName);
        return (FAULT_RETURN);
    }

    ret = TIFFSetField(tif, SINT8, s8[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT8: ret=%d\n", ret);
        GOTOFAILURE;
    }
    ret = TIFFSetField(tif, SINT16, s16[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT16: ret=%d\n", ret);
        GOTOFAILURE;
    }
    ret = TIFFSetField(tif, SINT32, s32[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT32: ret=%d\n", ret);
        GOTOFAILURE;
    }

    TIFFSetField(tif, C0_SINT8, &s8);
    TIFFSetField(tif, C0_SINT16, &s16);
    TIFFSetField(tif, C0_SINT32, &s32);

    TIFFSetField(tif, C16_SINT8, 6, &s8);
    TIFFSetField(tif, C16_SINT16, 6, &s16);
    TIFFSetField(tif, C16_SINT32, 6, &s32);

    TIFFSetField(tif, C16_SINT8, 6, &s8);
    TIFFSetField(tif, C16_SINT16, 6, &s16);
    TIFFSetField(tif, C16_SINT32, 6, &s32);

    TIFFSetField(tif, C32_SINT8, 6, &s8);
    TIFFSetField(tif, C32_SINT16, 6, &s16);
    TIFFSetField(tif, C32_SINT32, 6, &s32);

    if (isBigTiff)
    {
        ret = TIFFSetField(tif, SINT64, s64[0]);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C0_SINT64, &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C0_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C16_SINT64, N(s64), &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C16_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C32_SINT64, N(s64), &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C32_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 1);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    ret = (int)TIFFWriteEncodedStrip(tif, 0, "\0", 1);
    if (ret != 1)
    {
        fprintf(stdout, "Error TIFFWriteEncodedStrip: ret=%d\n", ret);
        GOTOFAILURE;
    }

    retcode = OK_RETURN;
failure:
    TIFFClose(tif);
    return (retcode);
}

static int readTestTiff(const char *szFileName, int isBigTiff)
{
    int ret;
    int i;
    int8_t s8l, *s8p;
    int16_t s16l, *s16p;
    int32_t s32l, *s32p;
    int64_t s64l, *s64p;
    uint16_t count;
    uint32_t count32;
    int retcode = FAULT_RETURN;

    fprintf(stdout, "-- Reading signed values ...\n");
    TIFF *tif = TIFFOpen(szFileName, "r");
    if (!tif)
    {
        fprintf(stdout, "Can't open test TIFF file %s.\n", szFileName);
        return (FAULT_RETURN);
    }

    ret = TIFFGetField(tif, SINT8, &s8l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT8: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s8l != s8[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT8  %d differs from set value %d\n", s8l,
                    s8[idxSingle]);
            GOTOFAILURE
        }
    }
    ret = TIFFGetField(tif, SINT16, &s16l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT16: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s16l != s16[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT16  %d differs from set value %d\n",
                    s16l, s16[idxSingle]);
            GOTOFAILURE
        }
    }
    ret = TIFFGetField(tif, SINT32, &s32l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT32: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s32l != s32[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT32  %d differs from set value %d\n",
                    s32l, s32[idxSingle]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT8, &s8p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT8: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s8);
    for (i = 0; i < count; i++)
    {
        if (s8p[i] != s8[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT8-Array %d differs from set value "
                    "%d\n",
                    i, s8p[i], s8[i]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT16, &s16p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT16: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s16);
    for (i = 0; i < count; i++)
    {
        if (s16p[i] != s16[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT16-Array %d differs from set "
                    "value %d\n",
                    i, s16p[i], s16[i]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT32, &s32p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT32: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s32);
    for (i = 0; i < count; i++)
    {
        if (s32p[i] != s32[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT32-Array %d differs from set "
                    "value %d\n",
                    i, s32p[i], s32[i]);
            GOTOFAILURE
        }
    }

    s8p = NULL;
    ret = TIFFGetField(tif, C16_SINT8, &count, &s8p);
    if (ret != 1 || s8p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT8: ret=%d; count=%d; pointer=%p\n", ret,
                count, s8p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s8p[i] != s8[i])
            {
                fprintf(
                    stdout,
                    "Read value %d of s8-Array %d differs from set value %d\n",
                    i, s8p[i], s8[i]);
                GOTOFAILURE
            }
        }
    }

    s16p = NULL;
    ret = TIFFGetField(tif, C16_SINT16, &count, &s16p);
    if (ret != 1 || s16p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT16: ret=%d; count=%d; pointer=%p\n", ret,
                count, s16p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s16p[i] != s16[i])
            {
                fprintf(stdout,
                        "Read value %d of C16_SINT16-Array %d differs from set "
                        "value %d\n",
                        i, s16p[i], s16[i]);
                GOTOFAILURE
            }
        }
    }

    s32p = NULL;
    ret = TIFFGetField(tif, C16_SINT32, &count, &s32p);
    if (ret != 1 || s32p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT32: ret=%d; count=%d; pointer=%p\n", ret,
                count, s32p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s32p[i] != s32[i])
            {
                fprintf(stdout,
                        "Read value %d of C16_SINT32-Array %d differs from set "
                        "value %d\n",
                        i, s32p[i], s32[i]);
                GOTOFAILURE
            }
        }
    }

    if (isBigTiff)
    {
        ret = TIFFGetField(tif, SINT64, &s64l);
        if (ret != 1)
        {
            fprintf(stdout, "Error reading SINT64: ret=%d\n", ret);
            GOTOFAILURE
        }
        else
        {
            if (s64l != s64[idxSingle])
            {
                fprintf(stdout,
                        "Read value of SINT64  %" PRIi64
                        " differs from set value %" PRIi64 "\n",
                        s64l, s64[idxSingle]);
                GOTOFAILURE
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C0_SINT64, &s64p);
        count = N(s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error reading C0_SINT64: ret=%d\n", ret);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C0_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C16_SINT64, &count, &s64p);
        if (ret != 1 || s64p == NULL)
        {
            fprintf(stdout,
                    "Error reading C16_SINT64: ret=%d; count=%d; pointer=%p\n",
                    ret, count, s64p);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C16_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C32_SINT64, &count32, &s64p);
        if (ret != 1 || s64p == NULL)
        {
            fprintf(stdout,
                    "Error reading C32_SINT64: ret=%d; count=%d; pointer=%p\n",
                    ret, count, s64p);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < (int)count32; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C32_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }
    } /*-- if(isBigTiff) --*/

    retcode = OK_RETURN;
failure:

    fprintf(stdout, "-- End of test. Closing TIFF file. --\n");
    TIFFClose(tif);
    return (retcode);
}

int main(void)
{
    parent = TIFFSetTagExtender(&extender);
    if (writeTestTiff("temp.tif", 0) != OK_RETURN)
        return (-1);
    if (readTestTiff("temp.tif", 0) != OK_RETURN)
        return (-1);
    unlink("temp.tif");
    if (writeTestTiff("tempBig.tif", 1) != OK_RETURN)
        return (-1);
    if (readTestTiff("tempBig.tif", 1) != OK_RETURN)
        return (-1);
    unlink("tempBig.tif");
    fprintf(stdout, "---------- Test finished OK -----------\n");
    return 0;
}
