/*
 * Copyright (c) 1992-1997 Sam Leffler
 * Copyright (c) 1992-1997 Silicon Graphics, Inc.
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

#include "libport.h"
#include "tif_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define CopyField(tag, v)                                                      \
    if (TIFFGetField(in, tag, &v))                                             \
    TIFFSetField(out, tag, v)
#define CopyField2(tag, v1, v2)                                                \
    if (TIFFGetField(in, tag, &v1, &v2))                                       \
    TIFFSetField(out, tag, v1, v2)
#define CopyField3(tag, v1, v2, v3)                                            \
    if (TIFFGetField(in, tag, &v1, &v2, &v3))                                  \
    TIFFSetField(out, tag, v1, v2, v3)

#define PATH_LENGTH 8192

#define DEFAULT_MAX_MALLOC (256 * 1024 * 1024)

/* malloc size limit (in bytes)
 * disabled when set to 0 */
static tmsize_t maxMalloc = DEFAULT_MAX_MALLOC;

static const char TIFF_SUFFIX[] = ".tif";

static char fname[PATH_LENGTH];

static int tiffcp(TIFF *, TIFF *);
static void newfilename(void);
static int cpStrips(TIFF *, TIFF *);
static int cpTiles(TIFF *, TIFF *);

static void usage(int);

/**
 * This custom malloc function enforce a maximum allocation size
 */
static void *limitMalloc(tmsize_t s)
{
    /* tmsize_t is signed and _TIFFmalloc() converts s to size_t. Therefore
     * check for negative s. */
    if (maxMalloc && ((s > maxMalloc) || (s < 0)))
    {
        fprintf(stderr,
                "MemoryLimitError: allocation of %" TIFF_SSIZE_FORMAT
                " bytes is forbidden. Limit is %" TIFF_SSIZE_FORMAT ".\n",
                s, maxMalloc);
        fprintf(stderr, "                  use -M option to change limit.\n");
        return NULL;
    }
    return _TIFFmalloc(s);
}

static void *limitRealloc(void *buf, tmsize_t s)
{
    if (maxMalloc && ((s > maxMalloc) || (s < 0)))
    {
        fprintf(stderr,
                "MemoryLimitError: re-allocation of %" TIFF_SSIZE_FORMAT
                " bytes is forbidden. Limit is %" TIFF_SSIZE_FORMAT ".\n",
                s, maxMalloc);
        fprintf(stderr, "                  use -M option to change limit.\n");
        if (buf != NULL)
            _TIFFfree(buf);
        return NULL;
    }
    return _TIFFrealloc(buf, s);
}

int main(int argc, char *argv[])
{
    TIFF *in, *out;
#if !HAVE_DECL_OPTARG
    extern char *optarg;
    extern int optind;
#endif
    int c;

    while ((c = getopt(argc, argv, "M:")) != -1)
    {
        switch (c)
        {
            case 'M':
                maxMalloc = (tmsize_t)strtoul(optarg, NULL, 0) << 20;
                if ((maxMalloc == 0) && (optarg[0] != '0'))
                {
                    fprintf(stderr,
                            "tiffsplit: Error: Option -M was not followed by a "
                            "number but <%s>\n",
                            optarg);
                    usage(EXIT_FAILURE);
                }
                break;
            case '?':
                usage(EXIT_SUCCESS);
                break;
            default:
                break;
        }
    }

    c = argc - optind;
    if (c < 1 || c > 2)
        usage(EXIT_FAILURE);
    if (c > 1)
    {
        strncpy(fname, argv[optind + 1], sizeof(fname));
        fname[sizeof(fname) - 1] = '\0';
    }

    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    if (opts == NULL)
    {
        return EXIT_FAILURE;
    }
    TIFFOpenOptionsSetMaxSingleMemAlloc(opts, maxMalloc);
    in = TIFFOpenExt(argv[optind], "r", opts);
    if (in == NULL)
    {
        fprintf(stderr, "tiffsplit: Error: Could not open %s \n", argv[optind]);
        TIFFOpenOptionsFree(opts);
        usage(EXIT_FAILURE);
    }

    do
    {
        size_t path_len;
        char *path = NULL;

        newfilename();

        path_len = strlen(fname) + sizeof(TIFF_SUFFIX);
        path = (char *)limitMalloc(path_len);
        if (!path)
        {
            fprintf(stderr,
                    "tiffsplit: Error: Can't allocate %" TIFF_SSIZE_FORMAT
                    " bytes for path-variable.\n",
                    path_len);
            TIFFClose(in);
            TIFFOpenOptionsFree(opts);
            return (EXIT_FAILURE);
        }
        strncpy(path, fname, path_len);
        path[path_len - 1] = '\0';
        strncat(path, TIFF_SUFFIX, path_len - strlen(path) - 1);
        out = TIFFOpenExt(path, TIFFIsBigEndian(in) ? "wb" : "wl", opts);

        if (out == NULL)
        {
            TIFFClose(in);
            fprintf(stderr,
                    "tiffsplit: Error: Could not open output file %s \n", path);
            _TIFFfree(path);
            TIFFOpenOptionsFree(opts);
            return (EXIT_FAILURE);
        }
        _TIFFfree(path);
        if (!tiffcp(in, out))
        {
            TIFFClose(in);
            TIFFClose(out);
            TIFFOpenOptionsFree(opts);
            fprintf(stderr, "tiffsplit: Error: Could not copy data from input "
                            "to output.\n");
            return (EXIT_FAILURE);
        }
        TIFFClose(out);
    } while (TIFFReadDirectory(in));

    TIFFOpenOptionsFree(opts);
    (void)TIFFClose(in);

    return (EXIT_SUCCESS);
}

static void newfilename(void)
{
    static int first = 1;
    static long lastTurn;
    static long fnum;
    static short defname;
    static char *fpnt;

    if (first)
    {
        if (fname[0])
        {
            fpnt = fname + strlen(fname);
            defname = 0;
        }
        else
        {
            fname[0] = 'x';
            fpnt = fname + 1;
            defname = 1;
        }
        first = 0;
    }
#define MAXFILES 17576
    if (fnum == MAXFILES)
    {
        if (!defname || fname[0] == 'z')
        {
            fprintf(stderr, "tiffsplit: too many files.\n");
            exit(EXIT_FAILURE);
        }
        fname[0]++;
        fnum = 0;
    }
    if (fnum % 676 == 0)
    {
        if (fnum != 0)
        {
            /*
             * advance to next letter every 676 pages
             * condition for 'z'++ will be covered above
             */
            fpnt[0]++;
        }
        else
        {
            /*
             * set to 'a' if we are on the very first file
             */
            fpnt[0] = 'a';
        }
        /*
         * set the value of the last turning point
         */
        lastTurn = fnum;
    }
    /*
     * start from 0 every 676 times (provided by lastTurn)
     * this keeps us within a-z boundaries
     */
    fpnt[1] = (char)((fnum - lastTurn) / 26) + 'a';
    /*
     * cycle last letter every file, from a-z, then repeat
     */
    fpnt[2] = (char)(fnum % 26) + 'a';
    fnum++;
}

static int tiffcp(TIFF *in, TIFF *out)
{
    uint16_t bitspersample, samplesperpixel, compression, shortv, *shortav;
    uint32_t w, l;
    float floatv;
    char *stringv;
    uint32_t longv;

    CopyField(TIFFTAG_SUBFILETYPE, longv);
    CopyField(TIFFTAG_TILEWIDTH, w);
    CopyField(TIFFTAG_TILELENGTH, l);
    CopyField(TIFFTAG_IMAGEWIDTH, w);
    CopyField(TIFFTAG_IMAGELENGTH, l);
    CopyField(TIFFTAG_BITSPERSAMPLE, bitspersample);
    CopyField(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    CopyField(TIFFTAG_COMPRESSION, compression);
    if (compression == COMPRESSION_JPEG)
    {
        uint32_t count = 0;
        void *table = NULL;
        if (TIFFGetField(in, TIFFTAG_JPEGTABLES, &count, &table) && count > 0 &&
            table)
        {
            TIFFSetField(out, TIFFTAG_JPEGTABLES, count, table);
        }
    }
    CopyField(TIFFTAG_PHOTOMETRIC, shortv);
    CopyField(TIFFTAG_PREDICTOR, shortv);
    CopyField(TIFFTAG_THRESHHOLDING, shortv);
    CopyField(TIFFTAG_FILLORDER, shortv);
    CopyField(TIFFTAG_ORIENTATION, shortv);
    CopyField(TIFFTAG_MINSAMPLEVALUE, shortv);
    CopyField(TIFFTAG_MAXSAMPLEVALUE, shortv);
    CopyField(TIFFTAG_XRESOLUTION, floatv);
    CopyField(TIFFTAG_YRESOLUTION, floatv);
    CopyField(TIFFTAG_GROUP3OPTIONS, longv);
    CopyField(TIFFTAG_GROUP4OPTIONS, longv);
    CopyField(TIFFTAG_RESOLUTIONUNIT, shortv);
    CopyField(TIFFTAG_PLANARCONFIG, shortv);
    CopyField(TIFFTAG_ROWSPERSTRIP, longv);
    CopyField(TIFFTAG_XPOSITION, floatv);
    CopyField(TIFFTAG_YPOSITION, floatv);
    CopyField(TIFFTAG_IMAGEDEPTH, longv);
    CopyField(TIFFTAG_TILEDEPTH, longv);
    CopyField(TIFFTAG_SAMPLEFORMAT, shortv);
    CopyField2(TIFFTAG_EXTRASAMPLES, shortv, shortav);
    {
        uint16_t *red, *green, *blue;
        CopyField3(TIFFTAG_COLORMAP, red, green, blue);
    }
    {
        uint16_t shortv2;
        CopyField2(TIFFTAG_PAGENUMBER, shortv, shortv2);
    }
    CopyField(TIFFTAG_ARTIST, stringv);
    CopyField(TIFFTAG_IMAGEDESCRIPTION, stringv);
    CopyField(TIFFTAG_MAKE, stringv);
    CopyField(TIFFTAG_MODEL, stringv);
    CopyField(TIFFTAG_SOFTWARE, stringv);
    CopyField(TIFFTAG_DATETIME, stringv);
    CopyField(TIFFTAG_HOSTCOMPUTER, stringv);
    CopyField(TIFFTAG_PAGENAME, stringv);
    CopyField(TIFFTAG_DOCUMENTNAME, stringv);
    CopyField(TIFFTAG_BADFAXLINES, longv);
    CopyField(TIFFTAG_CLEANFAXDATA, longv);
    CopyField(TIFFTAG_CONSECUTIVEBADFAXLINES, longv);
    CopyField(TIFFTAG_FAXRECVPARAMS, longv);
    CopyField(TIFFTAG_FAXRECVTIME, longv);
    CopyField(TIFFTAG_FAXSUBADDRESS, stringv);
    CopyField(TIFFTAG_FAXDCS, stringv);
    if (TIFFIsTiled(in))
        return (cpTiles(in, out));
    else
        return (cpStrips(in, out));
}

static int cpStrips(TIFF *in, TIFF *out)
{
    tmsize_t bufsize = TIFFStripSize(in);
    unsigned char *buf = (unsigned char *)limitMalloc(bufsize);
    if (buf)
    {
        tstrip_t s, ns = TIFFNumberOfStrips(in);
        uint64_t *bytecounts;

        if (!TIFFGetField(in, TIFFTAG_STRIPBYTECOUNTS, &bytecounts))
        {
            fprintf(stderr, "tiffsplit: strip byte counts are missing\n");
            _TIFFfree(buf);
            return (0);
        }
        for (s = 0; s < ns; s++)
        {
            if (bytecounts[s] > (uint64_t)bufsize)
            {
                buf =
                    (unsigned char *)limitRealloc(buf, (tmsize_t)bytecounts[s]);
                if (!buf)
                {
                    fprintf(stderr,
                            "tiffsplit: Error: Can't re-allocate "
                            "%" TIFF_SSIZE_FORMAT " bytes for strip-size.\n",
                            (tmsize_t)bytecounts[s]);
                    return (0);
                }
                bufsize = (tmsize_t)bytecounts[s];
            }
            if (TIFFReadRawStrip(in, s, buf, (tmsize_t)bytecounts[s]) < 0 ||
                TIFFWriteRawStrip(out, s, buf, (tmsize_t)bytecounts[s]) < 0)
            {
                _TIFFfree(buf);
                return (0);
            }
        }
        _TIFFfree(buf);
        return (1);
    }
    else
    {
        fprintf(stderr,
                "tiffsplit: Error: Can't allocate %" TIFF_SSIZE_FORMAT
                " bytes for strip-size.\n",
                bufsize);
    }
    return (0);
}

static int cpTiles(TIFF *in, TIFF *out)
{
    tmsize_t bufsize = TIFFTileSize(in);
    unsigned char *buf = (unsigned char *)limitMalloc(bufsize);

    if (buf)
    {
        ttile_t t, nt = TIFFNumberOfTiles(in);
        uint64_t *bytecounts;

        if (!TIFFGetField(in, TIFFTAG_TILEBYTECOUNTS, &bytecounts))
        {
            fprintf(stderr, "tiffsplit: tile byte counts are missing\n");
            _TIFFfree(buf);
            return (0);
        }
        for (t = 0; t < nt; t++)
        {
            if (bytecounts[t] > (uint64_t)bufsize)
            {
                buf =
                    (unsigned char *)limitRealloc(buf, (tmsize_t)bytecounts[t]);
                if (!buf)
                {
                    fprintf(stderr,
                            "tiffsplit: Error: Can't re-allocate "
                            "%" TIFF_SSIZE_FORMAT " bytes for tile-size.\n",
                            (tmsize_t)bytecounts[t]);
                    return (0);
                }
                bufsize = (tmsize_t)bytecounts[t];
            }
            if (TIFFReadRawTile(in, t, buf, (tmsize_t)bytecounts[t]) < 0 ||
                TIFFWriteRawTile(out, t, buf, (tmsize_t)bytecounts[t]) < 0)
            {
                _TIFFfree(buf);
                return (0);
            }
        }
        _TIFFfree(buf);
        return (1);
    }
    else
    {
        fprintf(stderr,
                "tiffsplit: Error: Can't allocate %" TIFF_SSIZE_FORMAT
                " bytes for tile-size.\n",
                bufsize);
    }
    return (0);
}

static void usage(int code)
{
    FILE *out = (code == EXIT_SUCCESS) ? stdout : stderr;

    fprintf(out, "\n\n%s\n\n", TIFFGetVersion());
    fprintf(out, "Split a multi-image TIFF into single-image TIFF files\n\n");
    fprintf(out, "usage: tiffsplit [option] input.tif [prefix]\n");
    fprintf(out, "where option is:\n");
    fprintf(out, " -M size       set the memory allocation limit in MiB. 0 to "
                 "disable limit.\n");
    exit(code);
}
