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
 * Test open options
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
#include "tiffiop.h" // for struct TIFF

#define ERROR_STRING_SIZE 1024

/* Test TIFFLIB_AT_LEAST() macro */
#if !TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION, TIFFLIB_MINOR_VERSION,            \
                      TIFFLIB_MICRO_VERSION)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if !TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION, TIFFLIB_MINOR_VERSION, 0)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if !TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION, 0, 0)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if !TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION - 1, 0, 0)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION + 1, 0, 0)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION, TIFFLIB_MINOR_VERSION + 1, 0)
#error "TIFFLIB_AT_LEAST broken"
#endif
#if TIFFLIB_AT_LEAST(TIFFLIB_MAJOR_VERSION, TIFFLIB_MINOR_VERSION,             \
                     TIFFLIB_MICRO_VERSION + 1)
#error "TIFFLIB_AT_LEAST broken"
#endif

typedef struct MyErrorHandlerUserDataStruct
{
    char *buffer;
    size_t buffer_size;
    TIFF *tif_got_from_callback;
    char module[64];
} MyErrorHandlerUserDataStruct;

static int myErrorHandler(TIFF *tiff, void *user_data, const char *module,
                          const char *fmt, va_list ap)
{
    MyErrorHandlerUserDataStruct *errorhandler_user_data =
        (MyErrorHandlerUserDataStruct *)user_data;
    vsnprintf(errorhandler_user_data->buffer,
              errorhandler_user_data->buffer_size, fmt, ap);
    errorhandler_user_data->tif_got_from_callback = tiff;
    snprintf(errorhandler_user_data->module,
             sizeof(errorhandler_user_data->module), "%s", module);
    return 1;
}

static int test_error_handler()
{
    int ret = 0;
    char error_buffer[ERROR_STRING_SIZE] = {0};
    char warn_buffer[ERROR_STRING_SIZE] = {0};
    MyErrorHandlerUserDataStruct errorhandler_user_data = {
        .buffer = error_buffer, .buffer_size = ERROR_STRING_SIZE};
    MyErrorHandlerUserDataStruct warnhandler_user_data = {
        .buffer = warn_buffer, .buffer_size = ERROR_STRING_SIZE};

    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    assert(opts);
    TIFFOpenOptionsSetErrorHandlerExtR(opts, myErrorHandler,
                                       &errorhandler_user_data);
    TIFFOpenOptionsSetWarningHandlerExtR(opts, myErrorHandler,
                                         &warnhandler_user_data);
    TIFF *tif = TIFFOpenExt("test_error_handler.tif", "w", opts);
    TIFFOpenOptionsFree(opts);

    if (tif == NULL)
    {
        fprintf(stderr, "Cannot create test_error_handler.tif");
        exit(1);
    }

    // Simulate an error emitted by libtiff
    TIFFErrorExtR(tif, "my_error_module", "%s", "some error message");
    if (strcmp(error_buffer, "some error message") != 0)
    {
        fprintf(stderr, "Did not get expected error message\n");
        ret = 1;
    }
    if (strcmp(errorhandler_user_data.module, "my_error_module") != 0)
    {
        fprintf(stderr, "Did not get expected error module\n");
        ret = 1;
    }
    if (errorhandler_user_data.tif_got_from_callback != tif)
    {
        fprintf(stderr,
                "errorhandler_user_data.tif_got_from_callback != tif\n");
        ret = 1;
    }

    // Simulate a warning emitted by libtiff
    TIFFWarningExtR(tif, "my_warning_module", "%s", "some warning message");
    if (strcmp(warn_buffer, "some warning message") != 0)
    {
        fprintf(stderr, "Did not get expected warning message\n");
        ret = 1;
    }
    if (strcmp(warnhandler_user_data.module, "my_warning_module") != 0)
    {
        fprintf(stderr, "Did not get expected warning module\n");
        ret = 1;
    }
    if (warnhandler_user_data.tif_got_from_callback != tif)
    {
        fprintf(stderr, "warnhandler_user_data.tif_got_from_callback != tif\n");
        ret = 1;
    }

    TIFFClose(tif);
    unlink("test_error_handler.tif");
    return ret;
}

static int test_TIFFOpenOptionsSetMaxSingleMemAlloc(
    tmsize_t limit, int expected_to_fail_in_open,
    int expected_to_fail_in_write_directory, bool is_big_tiff)
{
    int ret = 0;
    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    assert(opts);
    TIFFOpenOptionsSetMaxSingleMemAlloc(opts, limit);
    TIFF *tif =
        TIFFOpenExt("test_error_handler.tif", is_big_tiff ? "w8" : "w", opts);
    TIFFOpenOptionsFree(opts);
    if (expected_to_fail_in_open)
    {
        if (tif != NULL)
        {
            fprintf(
                stderr,
                "Expected TIFFOpenExt() to fail due to memory limitation\n");
            ret = 1;
            TIFFClose(tif);
        }
    }
    else
    {
        if (tif == NULL)
        {
            fprintf(stderr, "Expected TIFFOpenExt() to succeed\n");
            ret = 1;
        }
        else
        {
#define VALUE_SAMPLESPERPIXEL 10000
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, VALUE_SAMPLESPERPIXEL);
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
            if (TIFFWriteDirectory(tif) == 0)
            {
                if (!expected_to_fail_in_write_directory)
                {
                    fprintf(stderr,
                            "Expected TIFFWriteDirectory() to succeed\n");
                    ret = 1;
                }
            }
            else
            {
                if (expected_to_fail_in_write_directory)
                {
                    fprintf(stderr, "Expected TIFFWriteDirectory() to fail\n");
                    ret = 1;
                }
            }
            TIFFClose(tif);
        }
    }
    unlink("test_error_handler.tif");
    return ret;
}

int main()
{
    int ret = 0;
    ret += test_error_handler();
    fprintf(stderr, "---- test_TIFFOpenOptionsSetMaxSingleMemAlloc "
                    "with non-BigTIFF ---- \n");
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(1, TRUE, -1, FALSE);
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(
        sizeof(TIFF) + strlen("test_error_handler.tif") + 1, FALSE, TRUE,
        FALSE);
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(
        VALUE_SAMPLESPERPIXEL * sizeof(short), FALSE, FALSE, FALSE);

    fprintf(stderr, "\n---- test_TIFFOpenOptionsSetMaxSingleMemAlloc "
                    "with BigTIFF ---- \n");
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(1, TRUE, -1, TRUE);
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(
        sizeof(TIFF) + strlen("test_error_handler.tif") + 1, FALSE, TRUE, TRUE);
    ret += test_TIFFOpenOptionsSetMaxSingleMemAlloc(
        VALUE_SAMPLESPERPIXEL * sizeof(short), FALSE, FALSE, TRUE);
    return ret;
}
