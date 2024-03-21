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
 * Test IFD loop detection
 */

#include "tif_config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

/* Compare 'requested_dir_number' with number written in PageName tag
 * into the IFD to identify that IFD.  */
int is_requested_directory(TIFF *tif, int requested_dir_number,
                           const char *filename)
{
    char *ptr = NULL;
    char *auxStr = NULL;

    if (!TIFFGetField(tif, TIFFTAG_PAGENAME, &ptr))
    {
        fprintf(stderr, "Can't get TIFFTAG_PAGENAME tag.\n");
        return 0;
    }

    /* Check for reading errors */
    if (ptr != NULL)
        auxStr = strchr(ptr, ' ');

    if (ptr == NULL || auxStr == NULL || strncmp(auxStr, " th.", 4))
    {
        ptr = ptr == NULL ? "(null)" : ptr;
        fprintf(stderr,
                "Error reading IFD directory number from PageName tag: %s\n",
                ptr);
        return 0;
    }

    /* Retrieve IFD identification number from ASCII string */
    const int nthIFD = atoi(ptr);
    if (nthIFD == requested_dir_number)
    {
        return 1;
    }
    fprintf(stderr, "Expected directory %d from %s was not loaded but: %s\n",
            requested_dir_number, filename, ptr);

    return 0;
}

/* Test loop detection within chained SubIFDs.
 * test_ifd_loop_subifd.tif contains seven main-IFDs (0 to 6) and within IFD 1
 * there are three SubIFDs (0 to 2). Main IFD 4 loops back to main IFD 2.
 * SubIFD 2 loops back to SubIFD 1.
 * Within each IFD the tag PageName is filled with a string, indicating the
 * IFD. The main IFDs are numbered 0 to 6 and the SubIFDs 200 to 202. */
int test_subifd_loop(void)
{
    const char *filename = SOURCE_DIR "/images/test_ifd_loop_subifd.tif";
    TIFF *tif;
    int i, n;
    int ret = 0;
#define NUMBER_OF_SUBIFDs 3
    toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs] = {
        0UL}; /* array for SubIFD tag */
    void *ptr;
    uint16_t number_of_sub_IFDs = 0;

    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't open  %s\n", filename);
        return 1;
    }

    /* Try to read six further main directories. Fifth read shall fail. */
    for (i = 0; i < 6; i++)
    {
        if (!TIFFReadDirectory(tif))
            break;
    }
    if (i != 4)
    {
        fprintf(stderr, "(30) Expected fifth TIFFReadDirectory() to fail\n");
        ret = 1;
    }
    if (!is_requested_directory(tif, 4, filename))
    {
        fprintf(stderr, "(31) Expected fifth main IFD to be loaded\n");
        ret = 1;
    }

    /* Switch to IFD 1 and get SubIFDs.
     * Then read through SubIFDs and detect SubIFD loop.
     * Finally go back to main-IFD and check if right IFD is loaded.
     */
    if (!TIFFSetDirectory(tif, 1))
        ret = 1;

    /* Check if there are SubIFD subfiles */
    if (TIFFGetField(tif, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr) &&
        (number_of_sub_IFDs == 3))
    {
        /* Copy SubIFD array from pointer */
        memcpy(sub_IFDs_offsets, ptr,
               number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));

        for (i = 0; i < number_of_sub_IFDs; i++)
        {
            /* Read SubIFD directory directly via offset.
             * SubIFDs PageName string contains numbers 200 to 202. */
            if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[i]))
                ret = 1;
            if (!is_requested_directory(tif, 200 + i, filename))
            {
                fprintf(stderr, "(32) Expected SubIFD %d to be loaded.\n", i);
                ret = 1;
            }
            /* Now test SubIFD loop detection.
             * The (i+n).th read in the SubIFD chain shall fail. */
            for (n = 0; n < number_of_sub_IFDs; n++)
            {
                if (!TIFFReadDirectory(tif))
                    break;
            }
            if ((i + n) != 2)
            {
                fprintf(
                    stderr,
                    "(33) Expected third SubIFD-TIFFReadDirectory() to fail\n");
                ret = 1;
            }
        }
        /* Go back to main-IFD chain and re-read that main-IFD directory */
        if (!TIFFSetDirectory(tif, 3))
            ret = 1;
        if (!is_requested_directory(tif, 3, filename))
        {
            fprintf(stderr, "(34) Expected fourth main IFD to be loaded\n");
            ret = 1;
        }
    }
    else
    {
        fprintf(stderr, "(35) No or wrong expected SubIFDs within main IFD\n");
        ret = 1;
    }

    TIFFClose(tif);
    return ret;
} /*-- test_subifd_loop() --*/

int main()
{
    int ret = 0;
    {
        TIFF *tif =
            TIFFOpen(SOURCE_DIR "/images/test_ifd_loop_to_self.tif", "r");
        assert(tif);
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(1) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        TIFFClose(tif);
    }
    {
        TIFF *tif =
            TIFFOpen(SOURCE_DIR "/images/test_ifd_loop_to_self.tif", "r");
        assert(tif);
        int n = TIFFNumberOfDirectories(tif);
        if (n != 1)
        {
            fprintf(
                stderr,
                "(2) Expected TIFFNumberOfDirectories() to return 1. Got %d\n",
                n);
            ret = 1;
        }
        TIFFClose(tif);
    }
    {
        TIFF *tif =
            TIFFOpen(SOURCE_DIR "/images/test_ifd_loop_to_first.tif", "r");
        assert(tif);
        if (TIFFReadDirectory(tif) != 1)
        {
            fprintf(stderr, "(3) Expected TIFFReadDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(4) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 1) != 1)
        {
            fprintf(stderr, "(5) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(6) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 0) != 1)
        {
            fprintf(stderr, "(7) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif) != 1)
        {
            fprintf(stderr, "(8) Expected TIFFReadDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(9) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        TIFFClose(tif);
    }
    {
        TIFF *tif =
            TIFFOpen(SOURCE_DIR "/images/test_ifd_loop_to_first.tif", "r");
        assert(tif);
        int n = TIFFNumberOfDirectories(tif);
        if (n != 2)
        {
            fprintf(
                stderr,
                "(10) Expected TIFFNumberOfDirectories() to return 2. Got %d\n",
                n);
            ret = 1;
        }
        TIFFClose(tif);
    }
    {
        TIFF *tif = TIFFOpen(SOURCE_DIR "/images/test_two_ifds.tif", "r");
        assert(tif);
        if (TIFFReadDirectory(tif) != 1)
        {
            fprintf(stderr, "(11) Expected TIFFReadDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(12) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 1) != 1)
        {
            fprintf(stderr, "(13) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(14) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 0) != 1)
        {
            fprintf(stderr, "(15) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif) != 1)
        {
            fprintf(stderr, "(16) Expected TIFFReadDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFReadDirectory(tif))
        {
            fprintf(stderr, "(17) Expected TIFFReadDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 0) != 1)
        {
            fprintf(stderr, "(18) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 1) != 1)
        {
            fprintf(stderr, "(19) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 2))
        {
            fprintf(stderr, "(20) Expected TIFFSetDirectory() to fail\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 1) != 1)
        {
            fprintf(stderr, "(21) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        if (TIFFSetDirectory(tif, 0) != 1)
        {
            fprintf(stderr, "(22) Expected TIFFSetDirectory() to succeed\n");
            ret = 1;
        }
        TIFFClose(tif);
    }
    {
        TIFF *tif = TIFFOpen(SOURCE_DIR "/images/test_two_ifds.tif", "r");
        assert(tif);
        int n = TIFFNumberOfDirectories(tif);
        if (n != 2)
        {
            fprintf(
                stderr,
                "(23) Expected TIFFNumberOfDirectories() to return 2. Got %d\n",
                n);
            ret = 1;
        }
        TIFFClose(tif);
    }
    ret += test_subifd_loop();
    return ret;
}
