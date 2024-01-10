/*
 * Copyright (c) 2023, LibTIFF Contributors
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
 *===========  Purpose ========================================================
 * TIFF Library Testing:
 * - Write- and read- test of most tags defined within tif_dirinfo.c
 * - Write- and read- test of custom directories for EXIF and GPS.
 *
 * Attention:
 * This test program uses private functions from static LibTIFF library.
 * Therefore, it does not work when compiled for shared library.
 */

#define FOR_AUTO_TESTING
#ifdef FOR_AUTO_TESTING
/*  Only for automake and CMake infrastructure the test should:
    a.) delete any written testfiles when test passed (otherwise autotest
   will fail)
   b.) goto failure, if any failure is detected, which is not
   necessary when test is initiated manually for debugging
*/
#define GOTOFAILURE goto failure;
#else
#define GOTOFAILURE
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4101)
#endif

#include "tif_config.h" //necessary for linux compiler

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "tiffio.h"
#include "tiffiop.h"

static const char filename[] = "test_write_read_tags.tif";
static const char filenameBigTiff[] = "test_write_read_tags_Big.tif";

/* Settings for basic TIFF image to be written */
#define SPP 3 /* Samples per pixel */
const uint16_t width = 1;
const uint16_t length = 1;
const uint16_t bps = 8;
const uint16_t photometric = PHOTOMETRIC_RGB;
const uint16_t rows_per_strip = 1;
const uint16_t planarconfig = PLANARCONFIG_CONTIG;

/* -- Test data for writing -- */
#define STRSIZE 1000
#define N_SIZE 400
#define VARIABLE_ARRAY_SIZE 6

char auxCharArrayW[N_SIZE];
short auxShortArrayW[N_SIZE];
int32_t auxInt32ArrayW[N_SIZE];
float auxFloatArrayW[N_SIZE];
double auxDoubleArrayW[N_SIZE];
char auxTextArrayW[N_SIZE][STRSIZE];

/*-- Additional variables --*/
char exifVersion[4] = {
    '0', '2', '3', '1'}; /* EXIF 2.31 version is 4 characters of a string! */
char gpsVersion[4] = {2, 2, 0, 1}; /* GPS Version is 4 numbers! */

/* Inkname string is separated but not closed at the very end with a NULL.
 * Otherwise NumberOfInks would be counted to four by countInkNamesString()
 * at TIFFSetField() for the inkname string.
 */
#define NINKS 3
char inkNamesW[] = {"Ink1\0Ink2\0Ink3"};

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

/* -- List of tags, which must not be written with arbitrary values because they
 *    are written explicitely by the test program (like the basic TIFF tags),
 *    are automatically written by LibTIFF (e.g.TIFFTAG_NUMBEROFINKS),
 *    have some special characteristics, or write procedure is not supported
 * yet.
 *  - For TileOffsets and TileBytecounts the same variable is used as for
 *    StripOffsets and StripBytecounts, respectively.
 */
uint32_t listTagsNotToWrite[] = {TIFFTAG_OSUBFILETYPE,
                                 TIFFTAG_STRIPOFFSETS,
                                 TIFFTAG_COMPRESSION,
                                 TIFFTAG_STRIPBYTECOUNTS,
                                 TIFFTAG_IMAGEWIDTH,
                                 TIFFTAG_IMAGELENGTH,
                                 TIFFTAG_BITSPERSAMPLE,
                                 TIFFTAG_SAMPLESPERPIXEL,
                                 TIFFTAG_ROWSPERSTRIP,
                                 TIFFTAG_PLANARCONFIG,
                                 TIFFTAG_PHOTOMETRIC,
                                 TIFFTAG_FREEOFFSETS,
                                 TIFFTAG_FREEBYTECOUNTS,
                                 TIFFTAG_GRAYRESPONSEUNIT,
                                 TIFFTAG_GRAYRESPONSECURVE,
                                 TIFFTAG_TRANSFERFUNCTION,
                                 TIFFTAG_COLORMAP,
                                 TIFFTAG_COLORRESPONSEUNIT,
                                 TIFFTAG_TILEWIDTH,
                                 TIFFTAG_TILELENGTH,
                                 TIFFTAG_TILEOFFSETS,
                                 TIFFTAG_TILEBYTECOUNTS,
                                 TIFFTAG_SUBIFD,
                                 TIFFTAG_NUMBEROFINKS,
                                 TIFFTAG_GPSIFD,
                                 TIFFTAG_EXIFIFD,
                                 TIFFTAG_DATATYPE,
                                 TIFFTAG_MATTEING};

/* Some tag definitions do not follow the rules for
 * readcount/writecount/set_field_type/passcount-flag.
 * Most of them are handled by LibTIFF in special cases anyway, so the tag
 * definition is irrelevant.
 */
uint32_t listTagsNotFollowPasscountRules[] = {
    TIFFTAG_STRIPOFFSETS,      TIFFTAG_STRIPBYTECOUNTS,
    TIFFTAG_MINSAMPLEVALUE,    TIFFTAG_MAXSAMPLEVALUE,
    TIFFTAG_FREEOFFSETS,       TIFFTAG_FREEBYTECOUNTS,
    TIFFTAG_GRAYRESPONSECURVE, TIFFTAG_TRANSFERFUNCTION,
    TIFFTAG_COLORMAP,          TIFFTAG_SAMPLEFORMAT,
    TIFFTAG_SMINSAMPLEVALUE,   TIFFTAG_SMAXSAMPLEVALUE,
    TIFFTAG_DATATYPE,          TIFFTAG_TILEOFFSETS,
    TIFFTAG_TILEBYTECOUNTS,    TIFFTAG_PERSAMPLE};

/* Function definition */
int check_tag_definitions(void);
int write_test_tiff(TIFF *tif, const char *filenameRead);
int write_all_tags(TIFF *tif, const TIFFFieldArray *tFieldArray,
                   uint32_t *listTagsNotToWrite, uint32_t nTagsInList,
                   uint32_t *iCnt);
int tagIsInList(uint32_t tTag, uint32_t *list, uint32_t nTagsInList);
int testPasscountFlag(const char *szMsg, const TIFFFieldArray *tFieldArray,
                      uint32_t *listTagsNotFollowPasscountRules,
                      uint32_t nTagsInList);
int read_all_tags(TIFF *tif, const TIFFFieldArray *tFieldArray,
                  uint32_t *listTagsNotToWrite, uint32_t nTagsNotToWrite,
                  uint32_t *iCnt);

/* ==== main() ========================================================
 * Main program checks for correct tags definition.
 * Then, writing/reading is tested for ClassicTIFF and BigTIFF.
 * Testprograms shall return 0 for success.
 */
int main()
{
    int ret, ret1, ret2;

    fprintf(stderr, "==== Test automatically if all tags are "
                    "written/read correctly. ====\n");
    /* --- Test with Classic-TIFF ---*/
    /* Delete file, if exists and open it freshly. */
    ret = unlink(filename);
    if (ret != 0 && errno != ENOENT)
    {
        fprintf(stderr, "Can't delete test TIFF file %s.\n", filename);
    }
    TIFF *tif = TIFFOpen(filename, "w+");
    if (!tif)
    {
        fprintf(stderr, "Can't create test TIFF file %s.\n", filename);
        return 1;
    }

    /* Firstly, check for correct definition of tags. */
    fprintf(stderr, "-------- Check tag definition  -------------------\n");
    ret1 = check_tag_definitions();

    if (ret1 > 0)
        return (ret1);

    /* Write a simple image with all tags and check if correctly written
     * by reading all tags. */
    fprintf(stderr, "-------- Test with ClassicTIFF started  ----------\n");
    ret1 = write_test_tiff(tif, filename);

    if (ret1 > 0)
        return (ret1);

    /*--- Test with BIG-TIFF ---*/
    ret = unlink(filenameBigTiff);
    if (ret != 0 && errno != ENOENT)
    {
        fprintf(stderr, "Can't delete test TIFF file %s.\n", filenameBigTiff);
    }
    tif = TIFFOpen(filenameBigTiff, "w8");
    if (!tif)
    {
        fprintf(stderr, "Can't create test TIFF file %s.\n", filenameBigTiff);
        return 1;
    }
    fprintf(stderr, "\n\n-------- Test with BigTIFF started  ----------\n");
    ret2 = write_test_tiff(tif, filenameBigTiff);

    if (ret2 > 0)
        return (ret2 + 10);
    else
        return (ret2);
} /* main() */

/* ==== check_tag_definitions() =======================================
 * Check for correct definition of tags within tags FieldArray wrt passcount
 * flag and set_field_type.
 */
int check_tag_definitions(void)
{
    const TIFFFieldArray *tFieldArray;
    int ret = 0;

    tFieldArray = _TIFFGetFields();
    ret = testPasscountFlag("---- TIFF tags:", tFieldArray,
                            listTagsNotFollowPasscountRules,
                            NUM_ELEMENTS(listTagsNotFollowPasscountRules));

    tFieldArray = _TIFFGetGpsFields();
    ret += testPasscountFlag("---- GPS tags:", tFieldArray, NULL, 0);

    tFieldArray = _TIFFGetExifFields();
    ret += testPasscountFlag("---- EXIF tags:", tFieldArray, NULL, 0);

    return ret;
} /*-- check_tag_definitions() --*/

/* ==== write_test_tiff() =============================================
 * Performs write and read test of all tags defined within LibTiff
 * to the opened file passed through "tif".
 * All TIFF, GPS and EXIF tags are written with arbitrary values.
 * Finally, the file is closed, re-opened and all tags are read
 * and compared to their written value.
 */

int write_test_tiff(TIFF *tif, const char *filenameRead)
{
    unsigned char buf[SPP] = {0, 127, 255};
    uint16_t auxUint16 = 0;
    uint32_t auxUint32 = 0;
    int i;

    /*-- Fill test data arrays for writing and later comparison when written
     * tags are checked. */
    for (i = 0; i < N_SIZE; i++)
    {
        sprintf(auxTextArrayW[i],
                "N%d-String-%d_tttttttttttttttttttttttttttttx", i, i);
    }
    for (i = 0; i < N_SIZE; i++)
    {
        auxCharArrayW[i] = (char)(i + 1);
    }
    for (i = 0; i < N_SIZE; i++)
    {
        auxShortArrayW[i] = (short)(i + 1) * 7;
    }
    for (i = 0; i < N_SIZE; i++)
    {
        auxInt32ArrayW[i] = (i + 1) * 133;
    }
    for (i = 0; i < N_SIZE; i++)
    {
        auxFloatArrayW[i] = (float)((i + 1) * 133) / 3.3f;
    }
    for (i = 0; i < N_SIZE; i++)
    {
        auxDoubleArrayW[i] = (double)((i + 1) * 3689) / 4.5697;
    }

    /*-- Setup standard tags of a simple tiff file --*/
    fprintf(stderr, "----Write TIFF tags ...\n");
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width))
    {
        fprintf(stderr, "Can't set ImageWidth tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length))
    {
        fprintf(stderr, "Can't set ImageLength tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps))
    {
        fprintf(stderr, "Can't set BitsPerSample tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, SPP))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarconfig))
    {
        fprintf(stderr, "Can't set PlanarConfiguration tag.\n");
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric))
    {
        fprintf(stderr, "Can't set PhotometricInterpretation tag.\n");
        goto failure;
    }

    /*============ Write GPS and EXIF Dummy tags =====================*/
    /*-- Set dummy EXIF/GPS tag in original tif-structure in order to reserve
     *   space for final dir_offset value,
     *   which is properly written at the end.
     */
    uint64_t dir_offset = 0; /* Zero, in case no Custom-IFD is written */

    if (!TIFFSetField(tif, TIFFTAG_GPSIFD, dir_offset))
    {
        fprintf(stderr, "Can't write TIFFTAG_GPSIFD\n");
    }

    if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, dir_offset))
    {
        fprintf(stderr, "Can't write TIFFTAG_EXIFIFD\n");
    }

    /*============ Write mostly all other TIFF tags ==================*/
    /* Get array, where tag fields are defined. */
    const TIFFFieldArray *tFieldArray = _TIFFGetFields();

    /*-- write_all_tags() writes all tags automatically with the defined
     *   precision according to its set_field_type definition. --*/
    uint32_t iDataCnt = 0;
    if (write_all_tags(tif, tFieldArray, listTagsNotToWrite,
                       NUM_ELEMENTS(listTagsNotToWrite), &iDataCnt))
    {
        fprintf(stderr, "Error during TIFF tag writing.\n");
        goto failure;
    }

#ifndef WRITEPIXELLAST
    /*-- Write dummy pixel data. --*/
    if (TIFFWriteScanline(tif, buf, 0, 0) < 0)
    {
        fprintf(stderr, "Can't write image data.\n");
        goto failure;
    }
#endif

    /*================== Save TIFF Directory =====================*/
    /*-- Save current TIFF-directory to file before directory is changed.
     *   Otherwise it will be lost!
     *   The tif-structure is overwritten/ freshly initialized by any
     *   "CreateDirectory".
     *-----  What is needed here ??? ----
     * In custom_dir.c only 	TIFFFreeDirectory( tif );   is used to set
     * fields of another Sub-Directory. It releases storage associated
     * with a directory, especially custom-fields.
     *-- Using only  TIFFFreeDirectory() here leads to an error!!
     *-- Using here TIFFCheckpointDirectory() leads to an additional Main-IFD
     *   because it does not cleanup the tif-structure.
     *-- TIFFWriteDirectory() does cleanup the tif-structure and tries to
     *   write the IFD to the same location.
     */
    /*retCode = TIFFCheckpointDirectory(tif);*/
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr,
                "Can't write TIFF directory after TIFF tags and image data.\n");
        goto failure;
    }

    /*======================= GPS Directory =============================*/
    /*====== Create GPS Directory and write all GPS fields ==============*/
    /* - Create the GPS directory.
     * - Get GPS field array, where tag fields are defined.
     * - Then write all tags automatically according to their set_field_type
     *   definition.
     */
    fprintf(stderr, "----Write GPS tags ...\n");
    if (TIFFCreateGPSDirectory(tif) != 0)
    {
        fprintf(stderr, "TIFFCreateGPSDirectory() failed.\n");
        goto failure;
    }

    tFieldArray = _TIFFGetGpsFields();
    if (write_all_tags(tif, tFieldArray, NULL, 0, &iDataCnt))
    {
        fprintf(stderr, "Error during GPS tag writing.\n");
        goto failure;
    }

    /*-- Re-write a valid GPS version ID --*/
    if (!TIFFSetField(tif, GPSTAG_VERSIONID, gpsVersion))
    {
        fprintf(stderr, "Can't write GPSTAG_VERSIONID\n");
        goto failure;
    }

    /*-- GPS - write custom directory GPS into file and
     *   get back the offset of GPS directory.
     */
    uint64_t dir_offset_GPS = 0;
    if (!TIFFWriteCustomDirectory(tif, &dir_offset_GPS))
    {
        fprintf(stderr, "TIFFWriteCustomDirectory() with GPS failed.\n");
        goto failure;
    }

    /*-- Go back to the first (main) directory, and set correct value of the
     *   GPS IFD pointer. (main directory is reloaded from file!)
     */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "TIFFSetDirectory(0) after writing GPS failed.\n");
        goto failure;
    }
    /*-- Write GPS tag reference / offset into GPSIFD tag in main directory --*/
    if (!TIFFSetField(tif, TIFFTAG_GPSIFD, dir_offset_GPS))
    {
        fprintf(stderr, "Can't write TIFFTAG_GPSIFD.\n");
        goto failure;
    }

    /*-- Save current TIFF-directory to file before directory is changed.
     * Otherwise it will be lost! The tif-structure is overwritten/ freshly
     * initialized by any "CreateDirectory".
     * TIFFWriteDirectory() does cleanup tif-structure.
     */
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write TIFF directory after GPS tags.\n");
        goto failure;
    }

    /*======================= EXIF Directory ============================*/
    /*====== Create EXIF Directory and write all EXIF fields ============*/
    /* - Create the EXIF directory.
     * - Get EXIF field array, where tag fields are defined.
     * - Then write all tags automatically according to their set_field_type
     *   definition.
     */
    fprintf(stderr, "----Write EXIF tags ...\n");
    if (TIFFCreateEXIFDirectory(tif) != 0)
    {
        fprintf(stderr, "TIFFCreateEXIFDirectory() failed.\n");
        goto failure;
    }

    tFieldArray = _TIFFGetExifFields();
    if (write_all_tags(tif, tFieldArray, listTagsNotToWrite,
                       NUM_ELEMENTS(listTagsNotToWrite), &iDataCnt))
    {
        fprintf(stderr, "Error during GPS tag writing.\n");
        goto failure;
    }

    /*--- Set valid EXIF version, which is a 4 byte string --*/
    if (!TIFFSetField(tif, EXIFTAG_EXIFVERSION, exifVersion))
    {
        fprintf(stderr, "Can't write EXIFTAG_EXIFVERSION\n");
        goto failure;
    }

    /*-- EXIF - write custom directory EXIF into file and
     *   get back the offset of EXIF directory.
     */
    uint64_t dir_offset_EXIF = 0;
    if (!TIFFWriteCustomDirectory(tif, &dir_offset_EXIF))
    {
        fprintf(stderr, "TIFFWriteCustomDirectory() with EXIF failed.\n");
        goto failure;
    }

    /*-- Go back to the first (main) directory, and set correct value of the
     *   EXIF IFD pointer. (main directory is reloaded from file!)
     */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "TIFFSetDirectory(0) after writing EXIF failed.\n");
        goto failure;
    }
    /*-- Write EXIF tag reference / offset into EXIFIFD tag in main directory
     * --*/
    if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, dir_offset_EXIF))
    {
        fprintf(stderr, "Can't write TIFFTAG_EXIFIFD.\n");
        goto failure;
    }

#ifdef WRITEPIXELLAST
    /*-- Write dummy pixel data at the end of all directories. --*/
    if (TIFFWriteScanline(tif, buf, 0, 0) < 0)
    {
        fprintf(stderr, "Can't write image data after all direcories.\n");
        goto failure;
    }
#endif

    /*================= Close TIFF file  ====================*/
    /* Always TIFFWriteDirectory() before using/creating another directory.
     * Not necessary before TIFFClose(), however, TIFFClose() uses
     * TIFFReWriteDirectory(), which forces directory to be written at another
     * location.
     * Therefore, better use TIFFWriteDirectory() just before TIFFCllose().
     */
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write TIFF directory after EXIF tags.\n");
        goto failure;
    }
    TIFFClose(tif);

    fprintf(stderr, "-------- Continue Test  ---------- reading ...\n");

    /*===========  READING  =============  READING ==========================*/
    /* Ok, now test whether we can read written values correctly. */
    tif = TIFFOpen(filenameRead, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't open test TIFF file %s for reading.\n",
                filenameRead);
        goto failure;
    }

    /*-- Read some special parameters out of the main directory --*/
    if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &auxUint32))
    {
        fprintf(stderr, "Can't read %s.\n", "TIFFTAG_IMAGEWIDTH");
        goto failure;
    }
    if (auxUint32 != width)
    {
        fprintf(stderr,
                "Read value of IMAGEWIDTH %" PRIu32
                " differs from set value %" PRIu16 ".\n",
                auxUint32, width);
        goto failure;
    }
    if (!TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &auxUint32))
    {
        fprintf(stderr, "Can't read %s.\n", "TIFFTAG_IMAGELENGTH");
        goto failure;
    }
    if (auxUint32 != width)
    {
        fprintf(stderr,
                "Read value of TIFFTAG_IMAGELENGTH %" PRIu32
                " differs from set value %" PRIu16 ".\n",
                auxUint32, length);
        goto failure;
    }

    /* TIFFTAG_NUMBEROFINKS is a special tag, which is automatically set when
     * InkNames string is written. */
    if (!TIFFGetField(tif, TIFFTAG_NUMBEROFINKS, &auxUint16))
    {
        fprintf(stderr, "Can't read %s\n", "TIFFTAG_NUMBEROFINKS");
        goto failure;
    }
    else
    {
        if (auxUint16 != NINKS)
        {
            fprintf(stderr,
                    "Read value of TIFFTAG_NUMBEROFINKS %" PRIu32
                    " differs from set value %" PRIu16 ".\n",
                    auxUint16, NINKS);
            goto failure;
        }
    }

    /*================== Reading all TIFF tags =======================*/
    fprintf(stderr, "----Read TIFF tags ...\n");
    tFieldArray = _TIFFGetFields();
    iDataCnt = 0;
    if (read_all_tags(tif, tFieldArray, listTagsNotToWrite,
                      NUM_ELEMENTS(listTagsNotToWrite), &iDataCnt))
    {
        fprintf(stderr, "Error during TIFF tag reading.\n");
        goto failure;
    }

    /*================== Reading GPS tags ============================*/
    /*-- First get offset to GPS-directory and set it active with
     *   TIFFReadGPSDirectory().
     *   (this will destroy previously main directory fields in memory!)
     */
    uint64_t read_dir_offset = 0;
    fprintf(stderr, "----Read GPS tags ...\n");
    if (!TIFFGetField(tif, TIFFTAG_GPSIFD, &read_dir_offset))
    {
        fprintf(stderr, "Can't read %s\n", "TIFFTAG_GPSIFD");
        goto failure;
    }
    if (!TIFFReadGPSDirectory(tif, read_dir_offset))
    {
        fprintf(stderr, "Can't read %s\n", "TIFFReadGPSDirectory()");
        goto failure;
    }

    /*-- Now read and compare tags from GPS-directory --*/
    tFieldArray = _TIFFGetGpsFields();
    if (read_all_tags(tif, tFieldArray, NULL, 0, &iDataCnt))
    {
        fprintf(stderr, "Error during GPS tag reading.\n");
        goto failure;
    }

    /*================== Reading EXIF tags ===========================*/
    /*-- Go back to the first (main) directory using TIFFSetDirectory(),
     * and then get value of the EXIFIFD directory offset.
     * Then read EXIF directory tags from file into tif-structure.
     * Finally, read and compare tags to the written values.
     */
    TIFFSetDirectory(tif, 0);
    fprintf(stderr, "----Read EXIF tags ...\n");
    if (!TIFFGetField(tif, TIFFTAG_EXIFIFD, &read_dir_offset))
    {
        fprintf(stderr, "Can't read %s\n", "TIFFTAG_EXIFIFD");
        goto failure;
    }

    if (!TIFFReadEXIFDirectory(tif, read_dir_offset))
    {
        fprintf(stderr, "Can't read %s\n", "TIFFReadEXIFDirectory()");
        goto failure;
    }

    tFieldArray = _TIFFGetExifFields();
    if (read_all_tags(tif, tFieldArray, NULL, 0, &iDataCnt))
    {
        fprintf(stderr, "Error during EXIF tag reading.\n");
        goto failure;
    }

    TIFFClose(tif);

    /* All tests passed; delete file and exit with success status. */
#ifdef FOR_AUTO_TESTING
    unlink(filenameRead);
#endif
    fprintf(stderr, "-------- Test finished OK ----------\n");
    return 0;

failure:
    /*
     * Something went wrong; close file and return unsuccessful status.
     * Do not remove the file for further manual investigation.
     */
    TIFFClose(tif);
    fprintf(stderr, "-------- Test finished with FAILURE --------\n");
    return 1;
} /*-- write_test_tiff() --*/

/* ==== testPasscountFlag() ===========================================
 * Checks the tag FieldArray settings for the passcount flag.
 * Some tags, which are handled directly, do not follow the rules.
 *
 */
int testPasscountFlag(const char *szMsg, const TIFFFieldArray *tFieldArray,
                      uint32_t *listTagsNotFollowPasscountRules,
                      uint32_t nTagsInList)
{
    uint32_t t;
    int retFails = 0;

    uint32_t nTags = tFieldArray->count;
    /*-- Test ReadCount equals WriteCount. --*/
    int nFails = 0;
    fprintf(stderr, "%-16s Check if writecount equals readcount ... ", szMsg);
    for (t = 0; t < nTags; t++)
    {
        if (tagIsInList(tFieldArray->fields[t].field_tag,
                        listTagsNotFollowPasscountRules, nTagsInList))
            continue;

        if (tFieldArray->fields[t].field_writecount !=
            tFieldArray->fields[t].field_readcount)
        {
            if (nFails == 0)
                fprintf(stderr, "\n");
            fprintf(stderr,
                    "WriteCount %d for tag \t%-25s not equal ReadCount %d.\n",
                    tFieldArray->fields[t].field_writecount,
                    tFieldArray->fields[t].field_name,
                    tFieldArray->fields[t].field_readcount);
            nFails++;
        }
    } /* for - readcount == writecount */
    if (nFails == 0)
        fprintf(stderr, "--> OK\n");
    retFails += nFails;

    /*-- Test correct setting of passcount flag. --*/
    nFails = 0;
    fprintf(stderr,
            "%-16s Check if passcount flag is set according to "
            "writecount value ... ",
            szMsg);
    for (t = 0; t < nTags; t++)
    {
        if (tagIsInList(tFieldArray->fields[t].field_tag,
                        listTagsNotFollowPasscountRules, nTagsInList))
            continue;

        if (tFieldArray->fields[t].field_writecount < 0)
        {
            if (tFieldArray->fields[t].field_type != TIFF_ASCII &&
                tFieldArray->fields[t].field_passcount == 0)
            {
                if (tFieldArray->fields[t].field_readcount != TIFF_SPP)
                {
                    if (nFails == 0)
                        fprintf(stderr, "\n");
                    fprintf(stderr,
                            "Passcount flag for variable rd/wrtcount tag "
                            "\t%-25s not "
                            "correctly set. Should be "
                            "TRUE.\n",
                            tFieldArray->fields[t].field_name);
                }
                else
                {
                    if (nFails == 0)
                        fprintf(stderr, "\n");
                    fprintf(
                        stderr,
                        "Passcount flag for TIFF_SPP readcount tag \t%-25s not "
                        "correctly set. Should be "
                        "TRUE.\n",
                        tFieldArray->fields[t].field_name);
                }
                nFails++;
            }
            else if (tFieldArray->fields[t].field_type == TIFF_ASCII &&
                     tFieldArray->fields[t].set_field_type ==
                         TIFF_SETGET_ASCII &&
                     tFieldArray->fields[t].field_writecount !=
                         TIFF_VARIABLE2 &&
                     tFieldArray->fields[t].field_passcount != 0)
            {
                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(
                    stderr,
                    "Passcount flag for ASCII tag \t%-25s not correctly set. "
                    "Should be "
                    "FALSE.\n",
                    tFieldArray->fields[t].field_name);
                nFails++;
            }
            else if (tFieldArray->fields[t].field_type == TIFF_ASCII &&
                     tFieldArray->fields[t].field_writecount ==
                         TIFF_VARIABLE2 &&
                     tFieldArray->fields[t].field_passcount == 0)
            {
                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr,
                        "Passcount flag for ASCII 'TIFF_VARIABLE2' tag \t%-25s "
                        "not correctly set. "
                        "Should be TRUE.\n",
                        tFieldArray->fields[t].field_name);
                nFails++;
            }
        }
        else
        {
            /* field_writecount >= 0 */
            if (tFieldArray->fields[t].field_passcount != 0)
            {
                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr,
                        "Passcount flag for tag \t%-25s not correctly set. "
                        "Should be "
                        "FALSE.\n",
                        tFieldArray->fields[t].field_name);
                nFails++;
            }
            if (tFieldArray->fields[t].field_writecount == 0)
            {
                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr,
                        "WriteCount for tag \t%-25s shall be not zero.\n",
                        tFieldArray->fields[t].field_name);
                nFails++;
            }
        }
    } /* for() - passcount flag */
    if (nFails == 0)
        fprintf(stderr, "--> OK\n");
    retFails += nFails;

    /*-- Test correct read-/writecount and TIFF_SETGET type. --*/
    nFails = 0;
    fprintf(stderr, "%-16s Check if writecount fits to set_field_type ... ",
            szMsg);
    for (t = 0; t < nTags; t++)
    {
        if (tagIsInList(tFieldArray->fields[t].field_tag,
                        listTagsNotFollowPasscountRules, nTagsInList))
            continue;

        /* TIFF_SETGET_UNDEFINED tags FIELD_IGNORE tags are not written to file.
         * Thus definition is obsolete. */
        // if (tFieldArray->fields[t].set_field_type >= TIFF_SETGET_UNDEFINED &&
        //     tFieldArray->fields[t].field_bit == FIELD_IGNORE)
        //     continue;

        if (tFieldArray->fields[t].field_writecount == TIFF_VARIABLE2 &&
            !(tFieldArray->fields[t].set_field_type >= TIFF_SETGET_C32_ASCII &&
              tFieldArray->fields[t].set_field_type <= TIFF_SETGET_C32_IFD8))
        {
            if (nFails == 0)
                fprintf(stderr, "\n");
            fprintf(stderr,
                    "WriteCount %d for tag \t%-25s does not fit to "
                    "'set_field_type' %d. "
                    "Should be TIFF_VARIABLE2=-3\n",
                    tFieldArray->fields[t].field_writecount,
                    tFieldArray->fields[t].field_name,
                    tFieldArray->fields[t].set_field_type);
            nFails++;
        }
        else if (tFieldArray->fields[t].field_writecount == TIFF_VARIABLE &&
                 tFieldArray->fields[t].field_type != TIFF_ASCII &&
                 !(tFieldArray->fields[t].set_field_type >=
                       TIFF_SETGET_C16_ASCII &&
                   tFieldArray->fields[t].set_field_type <=
                       TIFF_SETGET_C16_IFD8))
        {
            if (tFieldArray->fields[t].field_readcount == TIFF_SPP)
            {
                /* Special case for tags defined with one value per sample,
                 * thus readcount == SamplesPerPixel.
                 */
            }
            else
            {

                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr,
                        "WriteCount %d for tag \t%-25s does not fit to "
                        "'set_field_type' %d. "
                        "Should be TIFF_VARIABLE=-1\n",
                        tFieldArray->fields[t].field_writecount,
                        tFieldArray->fields[t].field_name,
                        tFieldArray->fields[t].set_field_type);
                nFails++;
            }
        }
        else if (tFieldArray->fields[t].field_writecount > 1 &&
                 !((tFieldArray->fields[t].set_field_type >=
                        TIFF_SETGET_C0_ASCII &&
                    tFieldArray->fields[t].set_field_type <=
                        TIFF_SETGET_C0_IFD8) ||
                   tFieldArray->fields[t].set_field_type == TIFF_SETGET_ASCII))
        {
            if (!(tFieldArray->fields[t].set_field_type ==
                      TIFF_SETGET_UINT16_PAIR &&
                  tFieldArray->fields[t].field_writecount == 2))
            {

                if (nFails == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr,
                        "WriteCount %d for tag \t%-25s does not fit to "
                        "'set_field_type' %d. "
                        "Should be >1. \n",
                        tFieldArray->fields[t].field_writecount,
                        tFieldArray->fields[t].field_name,
                        tFieldArray->fields[t].set_field_type);
                nFails++;
            }
        }
        else if (tFieldArray->fields[t].field_writecount == 1 &&
                 !(tFieldArray->fields[t].set_field_type >=
                       TIFF_SETGET_UNDEFINED &&
                   tFieldArray->fields[t].set_field_type <= TIFF_SETGET_INT))
        {
            if (nFails == 0)
                fprintf(stderr, "\n");
            fprintf(stderr,
                    "WriteCount %d for tag \t%-25s does not fit to "
                    "'set_field_type' %d. "
                    "Should be 1. \n",
                    tFieldArray->fields[t].field_writecount,
                    tFieldArray->fields[t].field_name,
                    tFieldArray->fields[t].set_field_type);
            nFails++;
        }
    } /* for - read-/writecount and TIFF_SETGET type */
    if (nFails == 0)
        fprintf(stderr, "--> OK\n");
    retFails += nFails;

    fprintf(stderr, "\n");
    return retFails;
} /*-- testPassountFlag() --*/

/* ==== write_all_tags() ==============================================
 * Writes all tags within tFieldArray according to their set_field_type
 * definition except tags listed in listTagsNotToWrite.
 * iCnt is an index into predefined arrays for the values to write.
 */
int write_all_tags(TIFF *tif, const TIFFFieldArray *tFieldArray,
                   uint32_t *listTagsNotToWrite, uint32_t nTagsInList,
                   uint32_t *iCnt)
{

    char auxCharArray[2 * STRSIZE];

    uint32_t i = *iCnt;
    uint32_t nTags = tFieldArray->count;
    for (uint32_t t = 0; t < nTags; i++, t++)
    {
        bool deferredSetField = false;
        /* Allways reset variableArrayCount to default value here. */
        uint32_t variableArrayCount = VARIABLE_ARRAY_SIZE;

        uint32_t tTag = tFieldArray->fields[t].field_tag;
        if (tagIsInList(tTag, listTagsNotToWrite, nTagsInList))
            continue;

        TIFFDataType tType =
            tFieldArray->fields[t].field_type; /* e.g. TIFF_RATIONAL */
        (void)tType;
        short tWriteCount = tFieldArray->fields[t].field_writecount;
        TIFFSetGetFieldType tSetFieldType =
            tFieldArray->fields[t]
                .set_field_type; /* e.g. TIFF_SETGET_C0_FLOAT */
        char *tFieldName = tFieldArray->fields[t].field_name;
        void *pVoid = NULL;

        /*-- dependent on set_field_type write value --*/
        uint32_t auxUint32 = 0;
        switch (tSetFieldType)
        {
            case TIFF_SETGET_ASCII:
                /* Either the stringlength is defined as a fixed length in
                 * .field_writecount or a NULL-terminated string is used.
                 * Shorter strings than in auxTextArraxW need a
                 * NULL-termination. Therefore copy the string. */
                if (tWriteCount > 0)
                    auxUint32 = tWriteCount - 1;
                else
                    auxUint32 = (uint32_t)strlen(auxTextArrayW[i]) - 1;
                auxUint32 = (size_t)auxUint32 >= sizeof(auxCharArray)
                                ? (uint32_t)(sizeof(auxCharArray) - 1)
                                : auxUint32;
                strncpy(auxCharArray, auxTextArrayW[i], auxUint32);
                auxCharArray[auxUint32] = 0;
                if (!TIFFSetField(tif, tTag, auxCharArray))
                {
                    fprintf(stderr, "Can't write %s\n",
                            tFieldArray->fields[t].field_name);
                    goto failure;
                }
                break;
            case TIFF_SETGET_C16_ASCII:
                if (tTag == TIFFTAG_INKNAMES)
                {
                    if (!TIFFSetField(tif, tTag, sizeof(inkNamesW), inkNamesW))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
                break;
            case TIFF_SETGET_UINT8:
            case TIFF_SETGET_UINT16:
            case TIFF_SETGET_UINT32:
            case TIFF_SETGET_INT:
            case TIFF_SETGET_UNDEFINED:
                /*-- All those can be written with char, short or long
                 * parameter. Only value range should be in line. */
                if ((tSetFieldType == TIFF_SETGET_UINT8) &&
                    (auxInt32ArrayW[i] > 255))
                    auxInt32ArrayW[i] = 255;
                else if ((tSetFieldType == TIFF_SETGET_UINT16) &&
                         (auxInt32ArrayW[i] > 65535))
                    auxInt32ArrayW[i] = 65535;
                if (tTag == TIFFTAG_FILLORDER)
                    auxInt32ArrayW[i] = 1;
                if (tTag == TIFFTAG_ORIENTATION)
                    auxInt32ArrayW[i] = 7;
                if (tTag == TIFFTAG_RESOLUTIONUNIT)
                    auxInt32ArrayW[i] = 3;
                if (tTag == TIFFTAG_SAMPLEFORMAT)
                    auxInt32ArrayW[i] = SAMPLEFORMAT_UINT;
                if (!TIFFSetField(tif, tTag, auxInt32ArrayW[i]))
                {
                    fprintf(stderr, "Can't write %s\n",
                            tFieldArray->fields[t].field_name);
                    goto failure;
                }
                break;
            case TIFF_SETGET_UINT16_PAIR:
                if (auxInt32ArrayW[i] > 65535)
                    auxInt32ArrayW[i] = 65535;
                if (!TIFFSetField(tif, tTag, 7, auxInt32ArrayW[i]))
                {
                    fprintf(stderr, "Can't write %s\n",
                            tFieldArray->fields[t].field_name);
                    goto failure;
                }
                break;
            case TIFF_SETGET_UINT64:
            case TIFF_SETGET_IFD8:
                /* IFD8 is uint64_t but we write here only values for uint32
                 * size, even for BigTIFF.
                 * -- Attention: --
                 * "Variable promotion" for va_arg parameters are promoted to
                 * "int", which is different in x64 and x86 compilations.
                 * Thus cast to expected uint64_t.
                 */
                if (!TIFFSetField(tif, tTag, (uint64_t)auxInt32ArrayW[i]))
                {
                    fprintf(stderr, "Can't write %s\n",
                            tFieldArray->fields[t].field_name);
                    goto failure;
                }
                break;
            case TIFF_SETGET_SINT8:
            case TIFF_SETGET_SINT16:
            case TIFF_SETGET_SINT32:
                /*-- All those can be written with char, short or long
                 * parameter. Only value range should be in line. */
                if (!TIFFSetField(tif, tTag, -1.0 * auxInt32ArrayW[i]))
                {
                    fprintf(stderr, "Can't write %s\n",
                            tFieldArray->fields[t].field_name);
                    goto failure;
                }
                break;
            case TIFF_SETGET_FLOAT:
            case TIFF_SETGET_DOUBLE:
                if (tWriteCount == 1)
                {
                    /*-- All single values can be written with float or
                     * double parameter. Only value range should be in line.
                     */
                    if (!TIFFSetField(tif, tTag, auxDoubleArrayW[i]))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
                else if (tFieldArray->fields[t].field_readcount == TIFF_SPP)
                {
                    /*-- Some tags are written per SamplesPerPixel. For
                     * those, readcount is TIFF_SPP. */
                    if (tTag == TIFFTAG_SMINSAMPLEVALUE ||
                        tTag == TIFFTAG_SMAXSAMPLEVALUE)
                    {
                        auxDoubleArrayW[i] = 250;
                    }
                    if (!TIFFSetField(tif, tTag, auxDoubleArrayW[i]))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
                else
                {
                    fprintf(stderr,
                            "WriteCount for .set_field_type %d should be "
                            "1!  %s\n",
                            tSetFieldType, tFieldArray->fields[t].field_name);
                }
                break;
                /* _Cxx_ just defines the precense and size of the count
                 * parameter for the array:
                 * - C0=no count parameter (fixed array where,
                 *      positive readcount/writecount gives array count)
                 * - C16=short (uint16_t) count parameter size
                 * - C32=long (uint32_t) count parameter size
                 * Check, if it is a single parameter, a fixed array or a
                 * variable array.
                 */
            case TIFF_SETGET_C0_FLOAT:
            case TIFF_SETGET_C16_FLOAT:
            case TIFF_SETGET_C32_FLOAT:
                /* For arrays, distinguishing between float or double is
                 * essential, even for writing. */
                pVoid = &auxFloatArrayW[i];
                deferredSetField = true;
                break;
            case TIFF_SETGET_C0_DOUBLE:
            case TIFF_SETGET_C16_DOUBLE:
            case TIFF_SETGET_C32_DOUBLE:
                /* For arrays, distinguishing between float or double is
                 * essential, even for writing. */
                pVoid = &auxDoubleArrayW[i];
                deferredSetField = true;
                break;
            case TIFF_SETGET_C0_UINT8:
            case TIFF_SETGET_C0_SINT8:
            case TIFF_SETGET_C16_UINT8:
            case TIFF_SETGET_C16_SINT8:
            case TIFF_SETGET_C32_UINT8:
            case TIFF_SETGET_C32_SINT8:
                pVoid = &auxCharArrayW[i];
                deferredSetField = true;
                break;
            case TIFF_SETGET_C0_UINT16:
            case TIFF_SETGET_C0_SINT16:
            case TIFF_SETGET_C16_UINT16:
            case TIFF_SETGET_C16_SINT16:
            case TIFF_SETGET_C32_UINT16:
            case TIFF_SETGET_C32_SINT16:
                if (tTag == TIFFTAG_EXTRASAMPLES)
                {
                    variableArrayCount = 1;
                    auxShortArrayW[i] = 2;
                }
                pVoid = &auxShortArrayW[i];
                deferredSetField = true;
                break;
            case TIFF_SETGET_C0_UINT32:
            case TIFF_SETGET_C0_SINT32:
            case TIFF_SETGET_C16_UINT32:
            case TIFF_SETGET_C16_SINT32:
            case TIFF_SETGET_C32_UINT32:
            case TIFF_SETGET_C32_SINT32:
                pVoid = &auxInt32ArrayW[i];
                deferredSetField = true;
                break;
            case TIFF_SETGET_C16_IFD8:
                if (tTag == TIFFTAG_EXTRACAMERAPROFILES)
                {
                    uint64_t a = 18;
                    if (!TIFFSetField(tif, tTag, 1, &a))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
                else
                {
                    pVoid = &auxInt32ArrayW[i];
                    deferredSetField = true;
                }
                break;
            case TIFF_SETGET_OTHER:
                /* Transferfunction and Colormap to be inserted here. */
                fprintf(stderr,
                        "Writing of TIFF_SETGET_OTHER for tag %s not supported "
                        "yet.\n",
                        tFieldArray->fields[t].field_name);
                break;
            default:
                fprintf(stderr,
                        "SetFieldType %d not defined within writing switch for "
                        "%s.\n",
                        tSetFieldType, tFieldName);
        }; /*-- switch(tSetFieldType) --*/

        if (deferredSetField)
        {
            /* deferredSetField is used to write fixed arrays and
             * variable arrays.
             * pVoid pointer is set above to the corresponding array and
             * some other parameters like variableArrayCount are set for the
             * field, which is written now.
             */
            if (tWriteCount == 1)
            {
                fprintf(stderr,
                        "WriteCount for .set_field_type %d should be -1 or "
                        "greater than 1!  %s\n",
                        tSetFieldType, tFieldArray->fields[t].field_name);
            }
            else
            {
                /*-- Either fix or variable array --*/
                /* Now decide between fixed or variable array */
                if (tWriteCount > 1)
                {
                    /* fixed array with needed arraysize defined in
                     * .field_writecount */
                    if (!TIFFSetField(tif, tTag, pVoid))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
                else
                {
                    /*-- Special treatment of variable array --
                     * For test, use always arraysize of
                     * VARIABLE_ARRAY_SIZE, apart a few exceptions,
                     * for which variableArrayCount is set differently above.
                     */
                    if (!TIFFSetField(tif, tTag, variableArrayCount, pVoid))
                    {
                        fprintf(stderr, "Can't write %s\n",
                                tFieldArray->fields[t].field_name);
                        goto failure;
                    }
                }
            }
        }
    } /*-- for() --*/

    *iCnt = i;
    return 0;
failure:
    fprintf(stderr, "\n");
    *iCnt = i;
    return 1;
} /*--- write_all_tags() ---*/

/* ==== tagIsInList() =================================================
 * Returns TRUE if tag is in the list, otherwise FALSE.
 * When the pointer to the list is NULL also FALSE is returned.
 */
int tagIsInList(uint32_t tTag, uint32_t *list, uint32_t nTagsInList)
{
    if (list == NULL)
        return 0;

    for (uint32_t i = 0; i < nTagsInList; i++)
    {
        if (tTag == list[i])
            return 1;
    }
    return 0;
} /*--- tagIsInList() ---*/

/* ==== read_all_tags() ===============================================
 * Reads all tags within tFieldArray according to their set_field_type
 * definition except tags listed in listTagsNotToWrite.
 * iCnt is an index into predefined arrays for the values written.
 * The read values are compared to the written ones.
 */
int read_all_tags(TIFF *tif, const TIFFFieldArray *tFieldArray,
                  uint32_t *listTagsNotToWrite, uint32_t nTagsNotToWrite,
                  uint32_t *iCnt)
{

    /* -- Variables for reading within the switch() -- */
    int32_t j;
    float auxFloat = 0.0f;
    double auxDouble = 0.0;
    char auxChar = 0;
    uint32_t auxUint32 = 0;
    uint64_t auxUint64 = 0;
    short auxShort = 0;
    int32_t auxInt32 = 0;
    union
    {
        int32_t Int32;
        short Short1;
        short Short2[2];
        char Char[4];
    } unionInt32;
    void *pVoidArray;
    char *pAscii;
    double dblDiff, dblDiffLimit;

    char auxCharArray[2 * STRSIZE];
    short auxShortArray[2 * N_SIZE];
    int32_t auxInt32Array[2 * N_SIZE];
    float auxFloatArray[2 * N_SIZE];
    double auxDoubleArray[2 * N_SIZE];
#define RATIONAL_EPS                                                           \
    (1.0 / 30000.0) /* reduced difference of rational values, approx 3.3e-5 */

    uint32_t i = *iCnt;
    uint32_t nTags = tFieldArray->count;
    for (uint32_t t = 0; t < nTags; t++, i++)
    {
        uint32_t tTag = tFieldArray->fields[t].field_tag;
        TIFFDataType tType =
            tFieldArray->fields[t].field_type; /* e.g. TIFF_RATIONAL */
        short tWriteCount = tFieldArray->fields[t].field_writecount;
        TIFFSetGetFieldType tSetFieldType =
            tFieldArray->fields[t]
                .set_field_type; /* e.g. TIFF_SETGET_C0_FLOAT */
        char *tFieldName = tFieldArray->fields[t].field_name;
        if (tagIsInList(tTag, listTagsNotToWrite, nTagsNotToWrite))
            continue;

        /*-- dependent on set_field_type read value --*/
        switch (tSetFieldType)
        {
            case TIFF_SETGET_ASCII:
                /* Either the stringlength is defined as a fixed length in
                 * .field_writecount or a NULL-terminated string is used. */
                if (!TIFFGetField(tif, tTag, &pAscii))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* Save string from temporary buffer and compare with
                 * written string. */
                strncpy(auxCharArray, pAscii, sizeof(auxCharArray) - 1u);
                auxCharArray[sizeof(auxCharArray) - 1u] = '\0';
                if (tWriteCount > 0)
                    auxInt32 = tWriteCount - 1;
                else
                    auxInt32 = (int32_t)strlen(auxCharArray);
                int retCode2 =
                    strncmp(auxCharArray, auxTextArrayW[i], auxInt32);
                if (retCode2 != 0)
                {
                    fprintf(stderr,
                            "%d:Read value of %s %s differs from set value "
                            "%s\n",
                            i, tFieldName, auxCharArray, auxTextArrayW[i]);
                    GOTOFAILURE
                }
                break;
            case TIFF_SETGET_C16_ASCII:
                if (tTag == TIFFTAG_INKNAMES)
                {
                    if (!TIFFGetField(tif, tTag, &pVoidArray))
                    {
                        fprintf(stderr, "Can't read %s\n",
                                tFieldArray->fields[t].field_name);
                        GOTOFAILURE
                        break;
                    }
                    if (_TIFFmemcmp(inkNamesW, pVoidArray, sizeof(inkNamesW)) !=
                        0)
                    {
                        fprintf(stderr,
                                "InkNames read (%s) are different from "
                                "InkNamesW "
                                "written %s.\n",
                                (char *)pVoidArray, inkNamesW);
                        GOTOFAILURE
                    }
                }
                break;
                /*-- For reading, the parameter size is to be observed !! */
            case TIFF_SETGET_UINT8:
            case TIFF_SETGET_SINT8:
                if (!TIFFGetField(tif, tTag, &auxChar))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones */
                auxInt32 = auxChar;
                if (auxInt32 != (char)auxInt32ArrayW[i])
                {
                    fprintf(stderr,
                            "%d:Read value of %s %d differs from set "
                            "value %d\n",
                            i, tFieldName, auxInt32, auxInt32ArrayW[i]);
                }
                break;
            case TIFF_SETGET_UINT16:
            case TIFF_SETGET_SINT16:
                if (!TIFFGetField(tif, tTag, &auxShort))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones */
                auxInt32 = auxShort;
                if (auxInt32 != (short)auxInt32ArrayW[i])
                {
                    fprintf(stderr,
                            "%d:Read value of %s %d differs from set "
                            "value %d\n",
                            i, tFieldName, auxInt32, auxInt32ArrayW[i]);
                    GOTOFAILURE
                }
                break;
            case TIFF_SETGET_UINT16_PAIR:
                if (!TIFFGetField(tif, tTag, &auxShortArray[0],
                                  &auxShortArray[1]))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                if (auxShortArray[0] != 7 ||
                    auxShortArray[1] != auxInt32ArrayW[i])
                {
                    fprintf(stderr,
                            "%d:Read value of %s %d/%d UINT16_PAIR differs "
                            "from set "
                            "values %d/%d\n",
                            i, tFieldName, auxShortArray[0], auxShortArray[1],
                            7, auxInt32ArrayW[i]);
                    GOTOFAILURE
                }
                break;
            case TIFF_SETGET_UINT32:
            case TIFF_SETGET_SINT32:
            case TIFF_SETGET_INT:
                if (!TIFFGetField(tif, tTag, &auxUint32))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones */
                auxInt32 = auxUint32;
                if (auxInt32 != auxInt32ArrayW[i])
                {
                    uint32_t auxU;
                    if (tTag == TIFFTAG_TILEDEPTH &&
                        TIFFGetField(tif, TIFFTAG_IMAGEDEPTH, &auxU))
                    {
                        /* At directory re-reading from file,
                           tif->tif_dir.td_tiledepth =
                           tif->tif_dir.td_imagedepth is set. */
                        if (auxUint32 != auxU)
                        {
                            fprintf(stderr,
                                    "%d:Read value of %s %d differs "
                                    "from set "
                                    "value %d\n",
                                    i, tFieldName, auxUint32, auxU);
                            GOTOFAILURE
                        }
                    }
                    else
                    {
                        fprintf(stderr,
                                "%d:Read value of %s %d differs from set "
                                "value %d\n",
                                i, tFieldName, auxInt32, auxInt32ArrayW[i]);
                        GOTOFAILURE
                    }
                }
                break;
            case TIFF_SETGET_IFD8:
                if (!TIFFGetField(tif, tTag, &auxUint64))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones, we wrote only
                 * unit32 values! */
                if (auxUint64 != (uint64_t)auxInt32ArrayW[i])
                {
                    fprintf(stderr,
                            "%d:Read value of %s %" PRIu64 " differs from set"
                            "value %d\n",
                            i, tFieldName, auxUint64, auxInt32ArrayW[i]);
                    GOTOFAILURE
                }
                break;

            case TIFF_SETGET_C16_IFD8:
                if (!TIFFGetField(tif, tTag, &unionInt32, &pVoidArray))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                if (tTag == TIFFTAG_EXTRACAMERAPROFILES)
                {
                    auxUint64 = *(uint64_t *)pVoidArray;
                    if (unionInt32.Short1 != 1 && auxUint64 != 18)
                    {
                        fprintf(stderr,
                                "%d:Read value of %s %" PRIu64
                                " differs from set"
                                "value %d\n",
                                i, tFieldName, auxUint64, 18);
                        GOTOFAILURE
                    }
                }
                else
                {
                    /* ToDo: Such tags are not written yet.
                     */
                    fprintf(stderr, "TIFF_SETGET_C16_IFD8 arrays not supported "
                                    "in this testprogram.\n");
                    GOTOFAILURE
                }
                break;
            case TIFF_SETGET_FLOAT:
                if (!TIFFGetField(tif, tTag, &auxFloat))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones */
                if (tType == TIFF_RATIONAL || tType == TIFF_SRATIONAL)
                    dblDiffLimit = RATIONAL_EPS * auxDoubleArrayW[i];
                else
                    dblDiffLimit = 1e-3;
                dblDiff = auxFloat - auxDoubleArrayW[i];
                if (fabs(dblDiff) > fabs(dblDiffLimit))
                {
                    /*--: EXIFTAG_SUBJECTDISTANCE: LibTiff returns value of
                     * "-1.0" if numerator equals 4294967295 (0xFFFFFFFF) to
                     * indicate infinite distance! However, there are two
                     * other EXIF tags where numerator indicates a special
                     * value and six other cases where the denominator
                     * indicates special values, which are not treated
                     * within LibTiff!!
                     */
                    if (!(tTag == EXIFTAG_SUBJECTDISTANCE && auxFloat == -1.0))
                    {
                        fprintf(stderr,
                                "%d:Read value of %s %f differs from set value "
                                "%f\n",
                                i, tFieldName, auxFloat, auxDoubleArrayW[i]);
                        GOTOFAILURE
                    }
                }
                break;
            case TIFF_SETGET_DOUBLE:
                if (!TIFFGetField(tif, tTag, &auxDouble))
                {
                    fprintf(stderr, "Can't read %s\n",
                            tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                    break;
                }
                /* compare read values with written ones */
                if (tType == TIFF_RATIONAL || tType == TIFF_SRATIONAL)
                    dblDiffLimit = RATIONAL_EPS * auxDoubleArrayW[i];
                else
                    dblDiffLimit = 1e-6;
                dblDiff = auxDouble - auxDoubleArrayW[i];
                if (fabs(dblDiff) > fabs(dblDiffLimit))
                {
                    /*--: EXIFTAG_SUBJECTDISTANCE: LibTiff returns value of
                     * "-1.0" if numerator equals 4294967295 (0xFFFFFFFF) to
                     * indicate infinite distance!
                     */
                    if (!(tTag == EXIFTAG_SUBJECTDISTANCE && auxDouble == -1.0))
                    {
                        fprintf(stderr,
                                "%d:Read value of %s %f differs from set value "
                                "%f\n",
                                i, tFieldName, auxDouble, auxDoubleArrayW[i]);
                        GOTOFAILURE
                    }
                }
                break;
            case TIFF_SETGET_C0_FLOAT:
            case TIFF_SETGET_C0_DOUBLE:
            case TIFF_SETGET_C16_FLOAT:
            case TIFF_SETGET_C16_DOUBLE:
            case TIFF_SETGET_C32_FLOAT:
            case TIFF_SETGET_C32_DOUBLE:
            case TIFF_SETGET_C0_UINT8:
            case TIFF_SETGET_C0_SINT8:
            case TIFF_SETGET_C16_UINT8:
            case TIFF_SETGET_C16_SINT8:
            case TIFF_SETGET_C32_UINT8:
            case TIFF_SETGET_C32_SINT8:
            case TIFF_SETGET_C0_UINT16:
            case TIFF_SETGET_C0_SINT16:
            case TIFF_SETGET_C16_UINT16:
            case TIFF_SETGET_C16_SINT16:
            case TIFF_SETGET_C32_UINT16:
            case TIFF_SETGET_C32_SINT16:
            case TIFF_SETGET_C0_UINT32:
            case TIFF_SETGET_C0_SINT32:
            case TIFF_SETGET_C16_UINT32:
            case TIFF_SETGET_C16_SINT32:
            case TIFF_SETGET_C32_UINT32:
            case TIFF_SETGET_C32_SINT32:
            {
                /* _Cxx_ just defines the precense and size of the count
                 * parameter for the array:
                 * - C0=no count parameter (fixed array where,
                 *      positive readcount/writecount gives array count)
                 * - C16=short (uint16_t) count parameter size
                 * - C32=long (uint32_t) count parameter size
                 * Check, if it is a single parameter, a fixed array or a
                 * variable array.
                 */
                if (tWriteCount == 1)
                {
                    fprintf(stderr,
                            "WriteCount for .set_field_type %d should be -1, "
                            "-2, -3 or "
                            "greater than 1!  %s\n",
                            tSetFieldType, tFieldArray->fields[t].field_name);
                    GOTOFAILURE
                }
                else
                {
                    /*-- Either fix or variable array --*/
                    if (tWriteCount > 1)
                    {
                        /* fixed array with needed arraysize defined in
                         * .field_writecount */
                        if (!TIFFGetField(tif, tTag, &pVoidArray))
                        {
                            fprintf(stderr, "Can't read %s\n",
                                    tFieldArray->fields[t].field_name);
                            GOTOFAILURE
                            break;
                        }
                        /* set tWriteCount to number of read samples for
                         * next steps
                         */
                        auxInt32 = tWriteCount;
                    }
                    else
                    {
                        /*-- Special treatment of variable array. --
                         * Dependent on _Cxx_, the count parameter is
                         * short or long. Therefore use unionInt32!
                         */
                        if (!TIFFGetField(tif, tTag, &unionInt32, &pVoidArray))
                        {
                            fprintf(stderr, "Can't read %s\n",
                                    tFieldArray->fields[t].field_name);
                            GOTOFAILURE
                            break;
                        }
                        /* set tWriteCount to number of read samples for
                         * next steps
                         */
                        if (tSetFieldType == TIFF_SETGET_C32_UINT8 ||
                            tSetFieldType == TIFF_SETGET_C32_SINT8 ||
                            tSetFieldType == TIFF_SETGET_C32_UINT16 ||
                            tSetFieldType == TIFF_SETGET_C32_SINT16 ||
                            tSetFieldType == TIFF_SETGET_C32_UINT32 ||
                            tSetFieldType == TIFF_SETGET_C32_SINT32 ||
                            tSetFieldType == TIFF_SETGET_C32_FLOAT ||
                            tSetFieldType == TIFF_SETGET_C32_DOUBLE)
                        {
                            auxInt32 = unionInt32.Int32;
                        }
                        else
                        {
                            auxInt32 = unionInt32.Short1;
                        }
                    }
                    /* Save values from temporary array and compare to
                     * written values. */
                    if (tSetFieldType == TIFF_SETGET_C0_FLOAT ||
                        tSetFieldType == TIFF_SETGET_C16_FLOAT ||
                        tSetFieldType == TIFF_SETGET_C32_FLOAT)
                    {
                        memcpy(&auxFloatArray, pVoidArray,
                               (auxInt32 * sizeof(auxFloatArray[0])));
                        /* compare read values with written ones */
                        if (tType == TIFF_RATIONAL || tType == TIFF_SRATIONAL)
                            dblDiffLimit = RATIONAL_EPS * auxDoubleArrayW[i];
                        else
                            dblDiffLimit = 1e-6;
                        for (j = 0; j < auxInt32; j++)
                        {
                            dblDiff = auxFloatArray[j] - auxFloatArrayW[i + j];
                            if (fabs(dblDiff) > fabs(dblDiffLimit))
                            {
                                fprintf(stderr,
                                        "Read value %d of %s #%d %f differs "
                                        "from set value %f\n",
                                        i, tFieldName, j, auxFloatArray[j],
                                        auxFloatArrayW[i + j]);
                                GOTOFAILURE
                            }
                        }
                    }
                    else if (tSetFieldType == TIFF_SETGET_C0_DOUBLE ||
                             tSetFieldType == TIFF_SETGET_C16_DOUBLE ||
                             tSetFieldType == TIFF_SETGET_C32_DOUBLE)
                    {
                        memcpy(&auxDoubleArray, pVoidArray,
                               (auxInt32 * sizeof(auxDoubleArray[0])));
                        /* compare read values with written ones */
                        if (tType == TIFF_RATIONAL || tType == TIFF_SRATIONAL)
                            dblDiffLimit = RATIONAL_EPS * auxDoubleArrayW[i];
                        else
                            dblDiffLimit = 1e-6;
                        for (j = 0; j < auxInt32; j++)
                        {
                            dblDiff =
                                auxDoubleArray[j] - auxDoubleArrayW[i + j];
                            if (fabs(dblDiff) > fabs(dblDiffLimit))
                            {
                                fprintf(stderr,
                                        "Read value %d of %s #%d %f differs "
                                        "from set value %f\n",
                                        i, tFieldName, j, auxDoubleArray[j],
                                        auxDoubleArrayW[i + j]);
                                GOTOFAILURE
                            }
                        }
                    }
                    else if (tSetFieldType == TIFF_SETGET_C0_UINT8 ||
                             tSetFieldType == TIFF_SETGET_C0_SINT8 ||
                             tSetFieldType == TIFF_SETGET_C16_UINT8 ||
                             tSetFieldType == TIFF_SETGET_C16_SINT8 ||
                             tSetFieldType == TIFF_SETGET_C32_UINT8 ||
                             tSetFieldType == TIFF_SETGET_C32_SINT8)
                    {
                        memcpy(&auxCharArray, pVoidArray,
                               (auxInt32 * sizeof(auxCharArray[0])));
                        /* Compare and check values  */
                        char *auxCharCompare;
                        if (tTag == EXIFTAG_EXIFVERSION)
                        {
                            auxCharCompare = exifVersion;
                        }
                        else if (tTag == GPSTAG_VERSIONID)
                        {
                            auxCharCompare = gpsVersion;
                        }
                        else
                        {
                            auxCharCompare = &auxCharArrayW[i];
                        }

                        for (j = 0; j < auxInt32; j++)
                        {
                            if (auxCharArray[j] != auxCharCompare[j])
                            {
                                fprintf(stderr,
                                        "Read value %d of %s #%d %d differs "
                                        "from set value %d\n",
                                        i, tFieldName, j, auxCharArray[j],
                                        auxCharArrayW[i + j]);
                                GOTOFAILURE
                            }
                        }
                    }
                    else if (tSetFieldType == TIFF_SETGET_C0_UINT16 ||
                             tSetFieldType == TIFF_SETGET_C0_SINT16 ||
                             tSetFieldType == TIFF_SETGET_C16_UINT16 ||
                             tSetFieldType == TIFF_SETGET_C16_SINT16 ||
                             tSetFieldType == TIFF_SETGET_C32_UINT16 ||
                             tSetFieldType == TIFF_SETGET_C32_SINT16)
                    {
                        memcpy(&auxShortArray, pVoidArray,
                               (auxInt32 * sizeof(auxShortArray[0])));
                        /* Compare and check values  */
                        for (j = 0; j < auxInt32; j++)
                        {
                            if (auxShortArray[j] != auxShortArrayW[i + j])
                            {
                                fprintf(stderr,
                                        "Read value %d of %s #%d %d "
                                        "differs from "
                                        "set value %d\n",
                                        i, tFieldName, j, auxShortArray[j],
                                        auxShortArrayW[i + j]);
                                GOTOFAILURE
                            }
                        }
                    }
                    else if (tSetFieldType == TIFF_SETGET_C0_UINT32 ||
                             tSetFieldType == TIFF_SETGET_C0_SINT32 ||
                             tSetFieldType == TIFF_SETGET_C16_UINT32 ||
                             tSetFieldType == TIFF_SETGET_C16_SINT32 ||
                             tSetFieldType == TIFF_SETGET_C32_UINT32 ||
                             tSetFieldType == TIFF_SETGET_C32_SINT32)
                    {
                        memcpy(&auxInt32Array, pVoidArray,
                               (auxInt32 * sizeof(auxInt32Array[0])));
                        /* Compare and check values  */
                        for (j = 0; j < auxInt32; j++)
                        {
                            if (auxInt32Array[j] != auxInt32ArrayW[i + j])
                            {
                                fprintf(stderr,
                                        "Read value %d of %s #%d %d "
                                        "differs from "
                                        "set value %d\n",
                                        i, tFieldName, j, auxInt32Array[j],
                                        auxInt32ArrayW[i + j]);
                                GOTOFAILURE
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr,
                                "SetFieldType %d not defined within array"
                                "reading clause for %s.\n",
                                tSetFieldType, tFieldName);
                        GOTOFAILURE
                    }
                }
            } /* case for all arrays */
            break;
            case TIFF_SETGET_OTHER:
                /* ToDo: Transferfunction and Colormap to be inserted here. */
                break;
            case TIFF_SETGET_UNDEFINED:
                /* Such tags are not written to file. */
                break;
            default:
                fprintf(stderr,
                        "SetFieldType %d not defined within reading switch for "
                        "%s.\n",
                        tSetFieldType, tFieldName);
                GOTOFAILURE
        }; /*-- switch(tSetFieldType) --*/
    }      /*-- for(t to nTags) --*/

    *iCnt = i;
    return 0;
failure:
    fprintf(stderr, "\n");
    *iCnt = i;
    return 1;
} /*---  read_all_tags() ---*/
