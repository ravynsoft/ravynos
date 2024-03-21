/*
 * TIFF Library
 *
 * The purpose of this test suite is to test the correctness of
 * writing and reading transfer functions with TIFFTAG_TRANSFERFUNCTION.
 *
 * Depending on the SamplesPerPixel (and ExtraSamples) values,
 * exactly one (1) or exactly three (3) transfer functions shall be written
 * to file and are expected for reading from file.
 * An exception is if all three transfer functions are equal.
 * Then only one transfer function is written to file and
 * at reading that transfer function is returned in three
 * pointers to TIFFGetField().
 * If only one array is returned, as for images with SamplesPerPixel=1,
 * the other two are set to NULL.
 *
 * Each transfer function length is (1 << BitsPerSample) times uint16_t values.
 *
 */
/* clang-format off */

#include "tif_config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for unlink() on linux */
#endif

/* Define basic TIFF file settings */
uint16_t spp = 3;
const uint16_t width = 4;
const uint16_t length = 6;
const uint16_t bps = 8;
const uint16_t photometric = PHOTOMETRIC_RGB;
const uint16_t rows_per_strip = 100; /* up to 2**32-1 */
const uint16_t planarconfig = PLANARCONFIG_CONTIG;
const char *filename = "test_transferfunction_write_read.tif";

/* Data and pointer to three transfer functions. */
uint16_t *pTransferFunctionData;
uint32_t nSamplesPerTransferFunction;
/* Pointers into pTransferFunctionData for writing */
const uint16_t *ptf0;
const uint16_t *ptf1;
const uint16_t *ptf2;
/* Pointers for reading */
const uint16_t *ptfR0;
const uint16_t *ptfR1;
const uint16_t *ptfR2;

#define GOTOFAILURE(x)                                                                                                 \
    {                                                                                                                  \
        retval = x;                                                                                                    \
        goto failure;                                                                                                  \
    }

/*-- setup_transferfunctions() --
 * Initializes the transfer function array pTransferFunctionData
 * with three transfer functions.
 * Pointer ptf0, ptf1, ptf2 are initialized to the first array element of
 * each transfer function.
 */
int setup_transfer_functions(void)
{
    if (pTransferFunctionData)
        _TIFFfree(pTransferFunctionData);

    /* Setup array with some more values to shift start of the three arrays. */
    nSamplesPerTransferFunction = ((uint32_t)1 << bps);
    pTransferFunctionData = _TIFFmalloc(3 * (tmsize_t)nSamplesPerTransferFunction * sizeof(uint16_t));
    if (!pTransferFunctionData)
        return 1;

    for (uint32_t i = 0; i < 3 * nSamplesPerTransferFunction; i++)
        pTransferFunctionData[i] = (uint16_t)(i + 1);

    ptf0 = &pTransferFunctionData[0];
    ptf1 = &pTransferFunctionData[nSamplesPerTransferFunction];
    ptf2 = &pTransferFunctionData[2 * nSamplesPerTransferFunction];

    return 0;
} /*-- setup_transferfunctions() --*/

/*--read_check_transferfunctions --
 * The file is opened and the transfer functions are read.
 * Then the read transfer functions are compared against the arrays provided
 * with ptfx0, ptfx1, ptfx2.
 * Depending on blnClose, the opened file is closed.
 * TIFF **ptif returns the TIFF pointer.
 * The function returns a bit-field with one bit set for each successfully read transfer function.
 */
int read_check_transferfunctions(TIFF **ptif, const char *filename, int blnClose, const uint16_t *ptfx0,
                                 const uint16_t *ptfx1, const uint16_t *ptfx2)
{
    /* Test reading of transfer functions */
    int retval = 0;
    TIFF *tif = TIFFOpen(filename, "r");
    *ptif = tif;
    if (!tif)
    {
        fprintf(stderr, "read_check_transferfunctions(): Can't open %s for reading.\n", filename);
        GOTOFAILURE(-1);
    }

    if (!TIFFGetField(tif, TIFFTAG_TRANSFERFUNCTION, &ptfR0, &ptfR1, &ptfR2))
    {
        fprintf(stderr, "read_check_transferfunctions(): TIFFGetField for TIFFTAG_TRANSFERFUNCTION failed.\n");
        GOTOFAILURE(-1);
    }
    else
    {
        if (ptfR0 != NULL && !_TIFFmemcmp(ptfx0, ptfR0, nSamplesPerTransferFunction * sizeof(uint16_t)))
            retval += 1;
        if (ptfR1 != NULL && !_TIFFmemcmp(ptfx1, ptfR1, nSamplesPerTransferFunction * sizeof(uint16_t)))
            retval += 2;
        if (ptfR2 != NULL && !_TIFFmemcmp(ptfx2, ptfR2, nSamplesPerTransferFunction * sizeof(uint16_t)))
            retval += 4;
    }

failure:
    if (blnClose)
    {
        TIFFClose(tif);
        tif = NULL;
    }
    *ptif = tif;

    return retval;
} /*-- read_check_transferfunctions() --*/

/*-- write_basic_IFD_data() --
 * Opens a file for writing and initializes a basic TIFF image with the
 * globally specified parameters.
 *
 * wrtTransferFunction: 0 = don't write TIFFTAG_TRANSFERFUNCTION tag
 *                      1 = write TIFFTAG_TRANSFERFUNCTION tag before and
 *                      2 = after ExtraSamples tag.
 * blnExtraSamples=true writes ExtraSamples tag.
 * Only if blnCloseFile is true, the file is closed.
 * TIFF **ptif returns the pointer to the opened TIFF file.
 */
int write_basic_IFD_data(TIFF **ptif, const char *filename, int wrtTransferFunction, int nExtraSamples,
                         int blnCloseFile)
{
    unsigned char buf[3] = {0, 127, 255};
    int retval = 0;
    uint8_t *bufLine = NULL;
    TIFF *tif = TIFFOpen(filename, "w");
    *ptif = tif;
    if (!tif)
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't create %s\n", filename);
        GOTOFAILURE(1);
    }

    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set ImageWidth tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set ImageLength tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set BitsPerSample tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set SamplesPerPixel tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set SamplesPerPixel tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarconfig))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set PlanarConfiguration tag.\n");
        GOTOFAILURE(1);
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric))
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't set PhotometricInterpretation tag.\n");
        GOTOFAILURE(1);
    }

    /* Set transfer function before ExtraSamples*/
    if (wrtTransferFunction == 1)
    {

        if (!TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, ptf1, ptf2))
        {
            fprintf(stderr, "write_basic_IFD_data(): TIFFSetField for TIFFTAG_TRANSFERFUNCTION failed.\n");
            GOTOFAILURE(1);
        }
    }

    /* Set ExtraSamples thus SamplesPerPixel for transfer functions is reduced by one. */
    uint16_t extraSamples[4] = {EXTRASAMPLE_UNSPECIFIED, EXTRASAMPLE_UNSPECIFIED, EXTRASAMPLE_UNSPECIFIED,
                                EXTRASAMPLE_UNSPECIFIED};
    if (nExtraSamples > 0 && nExtraSamples < 4)
    {
        if (!TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, nExtraSamples, extraSamples))
        {
            fprintf(stderr, "write_basic_IFD_data(): Warning: Could not set TIFFTAG_EXTRASAMPLES tag.\n");
        }
    }

    /* Set transfer function after ExtraSamples*/
    if (wrtTransferFunction == 2)
    {

        if (!TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, ptf1, ptf2))
        {
            fprintf(stderr, "write_basic_IFD_data(): TIFFSetField for TIFFTAG_TRANSFERFUNCTION failed.\n");
            GOTOFAILURE(1);
        }
    }

    /* Setup buffer for image line */
    size_t bufLen = (size_t)width * spp * (bps + 7) / 8;
    bufLine = _TIFFmalloc(bufLen);
    if (!bufLine)
    {
        fprintf(stderr, "write_basic_IFD_data(): Can't allocate bufLine buffer.\n");
        GOTOFAILURE(1);
    }
    if (bufLen > sizeof(buf))
        _TIFFmemcpy(bufLine, buf, sizeof(buf));

    /* Write dummy pixel data. */
    for (int i = 0; i < length; i++)
    {
        if (TIFFWriteScanline(tif, bufLine, i, 0) == -1)
        {
            fprintf(stderr, "write_basic_IFD_data(): Can't write image data.\n");
            GOTOFAILURE(1);
        }
        size_t idx = (size_t)i * spp < bufLen ? (size_t)i * spp : bufLen - 2;
        bufLine[idx] = (uint8_t)(i * 80);
    }

failure:
    if (bufLine) 
        _TIFFfree(bufLine);
    if (blnCloseFile)
    {
        TIFFClose(tif);
        tif = NULL;
    }
    *ptif = tif;

    return retval;
} /*-- write_basic_IFD_data() --*/

/*==== main() ====*/
int main()
{
    if (setup_transfer_functions())
        return 1;

    int ret;
    TIFF *tif = NULL;

    fprintf(stderr,
            "--- Testing TIFF_TRANSFERFUNCTION tag writing and reading. ---\n    Expect 6 warning messages ...\n");

    /* Test with RGB data */
    spp = 3;
    if (write_basic_IFD_data(&tif, filename, 2, 0, true))
        return 1;
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != 7)
    {
        fprintf(stderr, "TRANSFERFUNCTION for RGB (SPP=3) could not be read correctly.\n");
        return 1;
    }

    /* Test with Grayscale data */
    spp = 1;
    if (write_basic_IFD_data(&tif, filename, 2, 0, true))
        return 1;
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != 1)
    {
        fprintf(stderr, "TRANSFERFUNCTION SPP=1 could not be read correctly.\n");
        return 1;
    }

    /* Test with RGB and ExtraSample, thus only 2 SamplesPerPixel left.
     * ExtraSample is written before transfer function.
     * --> For v4.5.1: This gives an Error when reading the transfer functions!
     */
    spp = 3;
    if (write_basic_IFD_data(&tif, filename, 2, 1, true))
        return 1;
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != 7)
    {
        fprintf(stderr, "One ExtraSample plus TRANSFERFUNCTION SPP=3 could not be read correctly.\n");
        return 1;
    }
    /* Now write two ExtraSamples, thus only one transfer function is written to file and read. */
    if (write_basic_IFD_data(&tif, filename, 2, 2, true))
        return 1;
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != 1)
    {
        fprintf(stderr, "Two ExtraSamples plus TRANSFERFUNCTION SPP=3 could not be read correctly.\n");
        return 1;
    }

    /* Test with tree times the same transfer functions.
     * Thus, only one should be written to file and when read copied three times.
     */
    spp = 3;
    if (write_basic_IFD_data(&tif, filename, 0, 0, false))
        return 1;
    ret = TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, ptf0, ptf0);
    TIFFClose(tif);
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf0, ptf0);
    if (ret != 7)
    {
        fprintf(stderr, "TRANSFERFUNCTIONs with same content could not be read correctly.\n");
        return 1;
    }

    /*---- Test the setting of the transfer functions with different wrong calls of TIFFSetField(). ----*/
    /* (ptf0, NULL, NULL) For v4.5.1: writes ptf0 to file and reads it back.
     *  Now, this should work for spp=1 but not for spp=3
     */
    spp = 1;
    if (write_basic_IFD_data(&tif, filename, 0, 0, false))
        return 1;
    ret = TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, NULL, NULL);
    TIFFClose(tif);
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != 1)
    {
        fprintf(stderr, "Unexpected at %d.\n", __LINE__);
        return 1;
    }
    spp = 3;
    /* (ptf0, NULL, NULL) For v4.5.1: writes ptf0 to file and reads it back.*/
    if (write_basic_IFD_data(&tif, filename, 0, 0, false))
        return 1;
    ret = TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, NULL, NULL);
    TIFFClose(tif);
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != -1)
    {
        fprintf(stderr, "Unexpected at %d.\n", __LINE__);
        return 1;
    }

    /* (ptf0, NULL, ptf2): For v4.5.1: Overflow error, when second ptf1 is copied in
     * TIFFWriteDirectoryTagTransferfunction().*/
    if (write_basic_IFD_data(&tif, filename, 0, 0, false))
        return 1;
    ret = TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, NULL, ptf2);
    TIFFClose(tif);
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != -1)
    {
        fprintf(stderr, "Unexpected at %d.\n", __LINE__);
        return 1;
    }

    /* (ptf0, ptf1, NULL) For v4.5.1: writes ptf0 and ptf1 to file but does not read it back, because only 2 but 3
     * required.*/
    if (write_basic_IFD_data(&tif, filename, 0, 0, false))
        return 1;
    ret = TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, ptf0, ptf1, NULL);
    TIFFClose(tif);
    ret = read_check_transferfunctions(&tif, filename, true, ptf0, ptf1, ptf2);
    if (ret != -1)
    {
        fprintf(stderr, "Unexpected at %d.\n", __LINE__);
        return 1;
    }

    /* All tests passed; delete file and exit with success status. */
    unlink(filename);
    fprintf(stderr, "-------- Test finished OK ----------\n");
    return 0;
}
