/*
 * TIFF Library
 *
 * The purpose of this test suite is to test the correctness of
 * TIFFWriteDirectory() when appending multiple directories to an open file.
 *
 * Currently, there is an optimization where the TIFF data structure stores the
 * offset of the last written directory in order to avoid having to traverse the
 * entire directory list each time a new one is added. The offset is not stored
 * in the file itself, only in the in-memory data structure. This means we still
 * go through the entire list the first time a directory is appended to a
 * newly-opened file, and the shortcut is taken for subsequent directory writes.
 *
 * In order to test the correctness of the optimization, the
 * `test_lastdir_offset` function writes 10 directories to two different files.
 * For the first file we use the optimization, by simply calling
 * TIFFWriteDirectory() repeatedly on an open TIFF handle. For the second file,
 * we avoid the optimization by closing the file after each call to
 * TIFFWriteDirectory(), which means the next directory write will traverse the
 * entire list.
 *
 * Finally, the two files are compared to check that the number of directories
 * written is the same, and that their offsets match. The test is then repeated
 * using BigTIFF files.
 *
 * Furthermore, arbitrary directory loading using TIFFSetDirectory() is checked,
 * especially after the update with "relative" movement possibility. Also
 * TIFFUnlinkDirectory() is tested.
 *
 */

#include "tif_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define SPP 3            /* Samples per pixel */
#define N_DIRECTORIES 10 /* Number of directories to write */
const uint16_t width = 1;
const uint16_t length = 1;
const uint16_t bps = 8;
const uint16_t photometric = PHOTOMETRIC_RGB;
const uint16_t rows_per_strip = 1;
const uint16_t planarconfig = PLANARCONFIG_CONTIG;

int write_data_to_current_directory(TIFF *tif, int i)
{
    unsigned char buf[SPP] = {0, 127, 255};
    char auxString[128];

    if (!tif)
    {
        fprintf(stderr, "Invalid TIFF handle.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width))
    {
        fprintf(stderr, "Can't set ImageWidth tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length))
    {
        fprintf(stderr, "Can't set ImageLength tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps))
    {
        fprintf(stderr, "Can't set BitsPerSample tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, SPP))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarconfig))
    {
        fprintf(stderr, "Can't set PlanarConfiguration tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric))
    {
        fprintf(stderr, "Can't set PhotometricInterpretation tag.\n");
        return 1;
    }
    /* Write IFD identification number to ASCII string of PageName tag. */
    sprintf(auxString, "%d th. IFD", i);
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME, auxString))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag.\n");
        return 1;
    }

    /* Write dummy pixel data. */
    if (TIFFWriteScanline(tif, buf, 0, 0) == -1)
    {
        fprintf(stderr, "Can't write image data.\n");
        return 1;
    }

    return 0;
}

int write_directory_to_closed_file(const char *filename, bool is_big_tiff,
                                   int i)
{
    TIFF *tif;
    tif = TIFFOpen(filename, is_big_tiff ? "a8" : "a");
    if (!tif)
    {
        fprintf(stderr, "Can't create/open %s\n", filename);
        return 1;
    }

    if (write_data_to_current_directory(tif, i))
    {
        fprintf(stderr, "Can't write data to directory %d of %s.\n", i,
                filename);
        TIFFClose(tif);
        return 1;
    }

    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "TIFFWriteDirectory() failed for directory %d of %s.\n",
                i, filename);
        TIFFClose(tif);
        return 1;
    }

    TIFFClose(tif);
    return 0;
}

int count_directories(const char *filename, int *count)
{
    TIFF *tif;
    *count = 0;

    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    do
    {
        (*count)++;
    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}

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

enum DirWalkMode
{
    DirWalkMode_ReadDirectory,
    DirWalkMode_SetDirectory,
    DirWalkMode_SetDirectory_Reverse,
};
/* Gets a list of the directory offsets in a file. Assumes the file has at least
 * N_DIRECTORIES directories.
 * There are three methods to test walking through the IFD chain.
 * This tests the optimization of faster SetDirectory(). */
int get_dir_offsets(const char *filename, uint64_t *offsets,
                    enum DirWalkMode dirWalkMode)
{
    TIFF *tif;
    int i;

    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    for (i = 0; i < N_DIRECTORIES; i++)
    {
        tdir_t dirn = (dirWalkMode == DirWalkMode_SetDirectory_Reverse)
                          ? (N_DIRECTORIES - i - 1)
                          : i;

        if (dirWalkMode != DirWalkMode_ReadDirectory &&
            !TIFFSetDirectory(tif, dirn) && dirn < (N_DIRECTORIES - 1))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            TIFFClose(tif);
            return 3;
        }

        offsets[dirn] = TIFFCurrentDirOffset(tif);

        /* Check, if dirn is requested directory */
        if (!is_requested_directory(tif, dirn, filename))
        {
            TIFFClose(tif);
            return 4;
        }

        if (dirWalkMode == DirWalkMode_ReadDirectory &&
            !TIFFReadDirectory(tif) && i < (N_DIRECTORIES - 1))
        {
            fprintf(stderr, "Can't read %d.th directory from %s\n", i,
                    filename);
            TIFFClose(tif);
            return 2;
        }
    }

    TIFFClose(tif);
    return 0;
}

/* Checks that TIFFSetDirectory() work well after update for relative seeking
 * to following directories.
 *
 * There are several issues especially when SubIFDs and custom directories are
 * involved. There are no real directory number for those and TIFFSetDirectory()
 * cannot be used. However, TIFFSetDirectory() is needed to switch back to the
 * main-IFD chain. Furthermore, IFD-loop handling needs to be supported in any
 * cases.
 * Also the case where directly after TIFFWriteDirectory() that directory
 * is re-read using TIFFSetDirectory() is tested.
 */
int test_arbitrary_directrory_loading(bool is_big_tiff)
{
    const char *filename = "test_arbitrary_directrory_loading.tif";
    TIFF *tif;
    uint64_t offsets_base[N_DIRECTORIES];
    int expected_original_dirnumber;

    /* Create a file and write N_DIRECTORIES (10) directories to it */
    tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    TIFFSetDirectory(tif, 0);
    for (int i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
        if (i >= 2 && i <= 4)
        {
            if (i == 3)
            {
                /* Invalidate directory - TIFFSetSubDirectory() will fail */
                if (TIFFSetSubDirectory(tif, 0))
                {
                    fprintf(stderr,
                            "Unexpected return at invalidate directory %d "
                            "within %s.\n",
                            i, filename);
                    goto failure;
                }
            }
            /* Test jump back to just written directory. */
            if (!TIFFSetDirectory(tif, i))
            {
                fprintf(stderr,
                        "Can't set directory %d within %s  "
                        "right after TIFFWriteDirectory().\n",
                        i, filename);
                goto failure;
            }
            if (i == 4)
            {
                /* Invalidate directory - TIFFSetSubDirectory() will fail */
                if (TIFFSetSubDirectory(tif, 0))
                {
                    fprintf(stderr,
                            "Unexpected return at invalidate directory %d "
                            "within %s.\n",
                            i, filename);
                    goto failure;
                }
            }
            /*Check if correct directory is loaded */
            if (!is_requested_directory(tif, i, filename))
            {
                goto failure;
            }
            /* Reset to next directory to go on with writing. */
            TIFFCreateDirectory(tif);
        }
    }
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /* Get directory offsets */
    if (get_dir_offsets(filename, offsets_base, DirWalkMode_ReadDirectory))
    {
        fprintf(stderr, "Error reading directory offsets from %s.\n", filename);
        goto failure;
    }

    /* Set last directory and then one after, which should fail. */
    if (!TIFFSetDirectory(tif, N_DIRECTORIES - 1))
    {
        fprintf(stderr, "Can't set last directory %d within %s\n",
                N_DIRECTORIES - 1, filename);
        goto failure;
    }
    if (TIFFSetDirectory(tif, N_DIRECTORIES + 1))
    {
        fprintf(stderr,
                "End of IFD chain not detected. Set non existing directory %d "
                "within %s\n",
                N_DIRECTORIES + 1, filename);
        goto failure;
    }

    /* Test very fast  TIFFSetDirectory() using IFD loop directory list.
     * First populate IFD loop directory list and then go through directories in
     * reverse order. Within between read after end of IFDs using
     * TIFFReadDirectory() where IFD loop directory list should be kept. */
    for (int i = 0; i < N_DIRECTORIES; i++)
    {
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
    }
    TIFFReadDirectory(tif);
    for (int i = N_DIRECTORIES - 1; i >= 0; i--)
    {
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
        if (!is_requested_directory(tif, i, filename))
        {
            goto failure;
        }
    }

    /* Test not existing directory number */
    if (TIFFSetDirectory(tif, N_DIRECTORIES))
    {
        fprintf(stderr,
                "No expected fail for accessing not existant directory number "
                "%d in file %s\n",
                N_DIRECTORIES, filename);
        goto failure;
    }

    /* Close and Reopen prepared testfile */
    TIFFClose(tif);
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /* Step through directories just using TIFFSetSubDirectory() */
    for (int i = N_DIRECTORIES - 1; i >= 0; i--)
    {
        if (!TIFFSetSubDirectory(tif, offsets_base[i]))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
        if (!is_requested_directory(tif, i, filename))
        {
            goto failure;
        }
    }

    /* More specialized test cases for relative seeking within TIFFSetDirectory.
     * However, with using IFD loop directory list, most of this test cases will
     * never be reached! */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set directory %d within %s\n", 2, filename);
        goto failure;
    }
    uint64_t off2 = TIFFCurrentDirOffset(tif);
    /* Note that dirnum = 2 is deleted here since TIFFUnlinkDirectory()
     * starts with 1 instead of 0. */
    if (!TIFFUnlinkDirectory(tif, 3))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 3, filename);
        goto failure;
    }
    /* Move to the unlinked IFD. This sets tif_curdir=0 because unlinked IFD
     * offset is not in the IFD loop list. This indicates a SubIFD chain. */
    if (!TIFFSetSubDirectory(tif, off2))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                off2, off2, filename);
        goto failure;
    }
    /*Check if correct directory is loaded */
    expected_original_dirnumber = 2;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    /* This should move back to the main-IFD chain and load the third
     * directory which has IFD number 4, due to the deleted third IFD. */
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 3, filename);
        goto failure;
    }
    expected_original_dirnumber = 4;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    /* Test backwards jump. */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 2, filename);
        goto failure;
    }
    expected_original_dirnumber = 3;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Second UnlinkDirectory -> two IFDs are missing in the main-IFD chain
     * then, orignal dirnum 2 and 3 */
    if (!TIFFUnlinkDirectory(tif, 3))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 3, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr,
                "Can't set new directory %d after second "
                "TIFFUnlinkDirectory(3) within %s\n",
                2, filename);
        goto failure;
    }
    /* which should now be the previous dir-3. */
    expected_original_dirnumber = 4;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Check, if third original directory could be loaded and the following,
     * still chained one. This is like for a SubIFD. */
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    if (!TIFFReadDirectory(tif))
    {
        fprintf(stderr,
                "Can't read directory after directory at offset 0x%" PRIx64
                " (%" PRIu64 ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    /* Check if correct directory is loaded, which was unlinked the second
     * time.
     */
    expected_original_dirnumber = 3;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Load unlinked directory like a SubIFD and then go back to the
     * main-IFD chain using TIFFSetDirectory(). Also check two loads of the
     * same directory. */
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 3, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    /*Check if correct directory is loaded. Because two original IFDs are
     * unlinked / missing, the original dirnumber is now 5. */
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Another sequence for testing. */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                2, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    expected_original_dirnumber = 2;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    /*Check if correct directory is loaded. Because two original IFDs are
     * unlinked / missing, the original dirnumber is now 5. */
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Third UnlinkDirectory -> three IFDs are missing in the main-IFD chain
     * then, orignal dirnum 0, 2 and 3
     * Furthermore, this test checks that TIFFUnlinkDirectory() can unlink
     * the first directory dirnum = 0 and a following TIFFSetDirectory(0)
     * does not load the unlinked directory. */
    if (!TIFFUnlinkDirectory(tif, 1))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 0, filename);
        goto failure;
    }
    /* Now three directories are missing (0,2,3) and thus directory 0 is
     * original directory 1 and directory 2 is original directory 5. */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr,
                "Can't set new directory %d after third "
                "TIFFUnlinkDirectory(1) within %s\n",
                0, filename);
        goto failure;
    }
    expected_original_dirnumber = 1;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr,
                "Can't set new directory %d after third "
                "TIFFUnlinkDirectory(1) within %s\n",
                2, filename);
        goto failure;
    }
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* TIFFUnlinkDirectory(0) is not allowed, because dirnum starts for
     * this function with 1 instead of 0.
     * An error return is expected here. */
    if (TIFFUnlinkDirectory(tif, 0))
    {
        fprintf(stderr, "TIFFUnlinkDirectory(0) did not return an error.\n");
        goto failure;
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    unlink(filename);
    return 1;
}

/* Tests SubIFD writing and reading
 *
 *
 */
int test_SubIFD_directrory_handling(bool is_big_tiff)
{
    const char *filename = "test_SubIFD_directrory_handling.tif";

/* Define the number of sub-IFDs you are going to write */
#define NUMBER_OF_SUBIFDs 3
    uint16_t number_of_sub_IFDs = NUMBER_OF_SUBIFDs;
    toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs] = {
        0UL}; /* array for SubIFD tag */
    int blnWriteSubIFD = 0;
    int i;
    int iIFD = 0, iSubIFD = 0;
    TIFF *tif;
    int expected_original_dirnumber;

    /* Create a file and write N_DIRECTORIES (10) directories to it */
    tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(
                tif, blnWriteSubIFD ? 200 + iSubIFD++ : iIFD++))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (blnWriteSubIFD)
        {
            /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
             * good idea to indicate thumbnails */
            if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE))
                goto failure;
        }

        /* For the second multi-page image, trigger TIFFWriteDirectory() to
         * switch for the next number_of_sub_IFDs calls to add those as SubIFDs
         * e.g. for thumbnails */
        if (1 == i)
        {
            blnWriteSubIFD = 1;
            /* Now here is the trick: the next n directories written
             * will be sub-IFDs of the main-IFD (where n is number_of_sub_IFDs
             * specified when you set the TIFFTAG_SUBIFD field.
             * The SubIFD offset array sub_IFDs_offsets is filled automatically
             * with the proper offset values by the following number_of_sub_IFDs
             * TIFFWriteDirectory() calls and are updated in the related
             * main-IFD with the last call.
             */
            if (!TIFFSetField(tif, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                              sub_IFDs_offsets))
                goto failure;
        }

        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }

        if (iSubIFD >= number_of_sub_IFDs)
            blnWriteSubIFD = 0;
    }
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        goto failure;
    }

    tdir_t numberOfMainIFDs = TIFFNumberOfDirectories(tif);
    if (numberOfMainIFDs != N_DIRECTORIES - number_of_sub_IFDs)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename, N_DIRECTORIES - number_of_sub_IFDs, numberOfMainIFDs);
        goto failure;
    }

    tdir_t currentDirNumber = TIFFCurrentDirectory(tif);

    /* The first directory is already read through TIFFOpen() */
    int blnRead = 0;
    expected_original_dirnumber = 1;
    do
    {
        /* Check if there are SubIFD subfiles */
        void *ptr;
        if (TIFFGetField(tif, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr))
        {
            /* Copy SubIFD array from pointer */
            memcpy(sub_IFDs_offsets, ptr,
                   number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));

            for (i = 0; i < number_of_sub_IFDs; i++)
            {
                /* Read first SubIFD directory */
                if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[i]))
                    goto failure;
                if (!is_requested_directory(tif, 200 + i, filename))
                {
                    goto failure;
                }
                /* Check if there is a SubIFD chain behind the first one from
                 * the array, as specified by Adobe */
                int n = 0;
                while (TIFFReadDirectory(tif))
                {
                    /* analyse subfile */
                    if (!is_requested_directory(tif, 201 + i + n++, filename))
                        goto failure;
                }
            }
            /* Go back to main-IFD chain and re-read that main-IFD directory */
            if (!TIFFSetDirectory(tif, currentDirNumber))
                goto failure;
        }
        /* Read next main-IFD directory (subfile) */
        blnRead = TIFFReadDirectory(tif);
        currentDirNumber = TIFFCurrentDirectory(tif);
        if (blnRead && !is_requested_directory(
                           tif, expected_original_dirnumber++, filename))
            goto failure;
    } while (blnRead);

    /*--- Now test arbitrary directory loading with SubIFDs ---*/
    if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[1]))
        goto failure;
    if (!is_requested_directory(tif, 201, filename))
    {
        goto failure;
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
        tif = NULL;
    }
    unlink(filename);
    return 1;
} /*--- test_SubIFD_directrory_handling() ---*/

/* Checks that rewriting a directory does not break the directory linked
 * list
 *
 * This could happen because TIFFRewriteDirectory relies on the traversal of
 * the directory linked list in order to move the rewritten directory to the
 * end of the file. This means the `lastdir_offset` optimization should be
 * skipped, otherwise the linked list will be broken at the point where it
 * connected to the rewritten directory, resulting in the loss of the
 * directories that come after it.
 * Rewriting the first directory requires an additional test, because it is
 * treated differently from the directories that have a predecessor in the list.
 */
int test_rewrite_lastdir_offset(bool is_big_tiff)
{
    const char *filename = "test_directory_rewrite.tif";
    int i, count;
    TIFF *tif;

    /* Create a file and write N_DIRECTORIES (10) directories to it */
    tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
    }

    /* Without closing the file, go to the fifth directory
     * and check, if dirn is requested directory. */
    TIFFSetDirectory(tif, 4);
    if (!is_requested_directory(tif, 4, filename))
    {
        TIFFClose(tif);
        return 4;
    }

    /* Rewrite the fifth directory by calling TIFFRewriteDirectory
     * and check, if the offset of IFD loaded by TIFFSetDirectory() is
     * different. Then, the IFD loop directory list is correctly maintained for
     * speed up of TIFFSetDirectory() with directly getting the offset that
     * list.
     */
    uint64_t off1 = TIFFCurrentDirOffset(tif);
    if (write_data_to_current_directory(tif, 4))
    {
        fprintf(stderr, "Can't write data to fifth directory in %s\n",
                filename);
        goto failure;
    }
    if (!TIFFRewriteDirectory(tif))
    {
        fprintf(stderr, "Can't rewrite fifth directory to %s\n", filename);
        goto failure;
    }
    i = 4;
    if (!TIFFSetDirectory(tif, i))
    {
        fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
        goto failure;
    }
    uint64_t off2 = TIFFCurrentDirOffset(tif);
    if (!is_requested_directory(tif, i, filename))
    {
        goto failure;
    }
    if (off1 == off2)
    {
        fprintf(stderr,
                "Rewritten directory %d was not correctly accessed by "
                "TIFFSetDirectory() in file %s\n",
                i, filename);
        goto failure;
    }

    /* Now, perform the test for the first directory */
    TIFFSetDirectory(tif, 0);
    if (!is_requested_directory(tif, 0, filename))
    {
        TIFFClose(tif);
        return 5;
    }
    off1 = TIFFCurrentDirOffset(tif);
    if (write_data_to_current_directory(tif, 0))
    {
        fprintf(stderr, "Can't write data to first directory in %s\n",
                filename);
        goto failure;
    }
    if (!TIFFRewriteDirectory(tif))
    {
        fprintf(stderr, "Can't rewrite first directory to %s\n", filename);
        goto failure;
    }
    i = 0;
    if (!TIFFSetDirectory(tif, i))
    {
        fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
        goto failure;
    }
    off2 = TIFFCurrentDirOffset(tif);
    if (!is_requested_directory(tif, i, filename))
    {
        goto failure;
    }
    if (off1 == off2)
    {
        fprintf(stderr,
                "Rewritten directory %d was not correctly accessed by "
                "TIFFSetDirectory() in file %s\n",
                i, filename);
        goto failure;
    }

    TIFFClose(tif);
    tif = NULL;

    /* Check that the file has the expected number of directories*/
    if (count_directories(filename, &count))
    {
        fprintf(stderr, "Error counting directories in file %s.\n", filename);
        goto failure;
    }
    if (count != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename, N_DIRECTORIES, count);
        goto failure;
    }

    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    unlink(filename);
    return 1;
}

/* Compares multi-directory files written with and without the lastdir
 * optimization */
int test_lastdir_offset(bool is_big_tiff)
{
    const char *filename_optimized = "test_directory_optimized.tif";
    const char *filename_non_optimized = "test_directory_non_optimized.tif";
    int i, count_optimized, count_non_optimized;
    uint64_t offsets_base[N_DIRECTORIES];
    uint64_t offsets_comparison[N_DIRECTORIES];
    TIFF *tif;

    /* First file: open it and add multiple directories. This uses the
     * lastdir optimization. */
    tif = TIFFOpen(filename_optimized, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename_optimized);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename_optimized);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n",
                    filename_optimized);
            goto failure;
        }
    }
    TIFFClose(tif);
    tif = NULL;

    /* Second file: close it after adding each directory. This avoids the
     * lastdir optimization. */
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_directory_to_closed_file(filename_non_optimized, is_big_tiff,
                                           i))
        {
            fprintf(stderr, "Can't write directory to %s\n",
                    filename_non_optimized);
            goto failure;
        }
    }

    /* Check that both files have the expected number of directories */
    if (count_directories(filename_optimized, &count_optimized))
    {
        fprintf(stderr, "Error counting directories in file %s.\n",
                filename_optimized);
        goto failure;
    }
    if (count_optimized != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename_optimized, N_DIRECTORIES, count_optimized);
        goto failure;
    }
    if (count_directories(filename_non_optimized, &count_non_optimized))
    {
        fprintf(stderr, "Error counting directories in file %s.\n",
                filename_non_optimized);
        goto failure;
    }
    if (count_non_optimized != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename_non_optimized, N_DIRECTORIES, count_non_optimized);
        goto failure;
    }

    /* Check that both files have the same directory offsets */
    /* In parallel of comparing offsets between optimized base file and
     * non-optimized file, test also three methods of walking through the
     * IFD chain within get_dir_offsets(). This tests the optimization of
     * faster SetDirectory(). */
    if (get_dir_offsets(filename_optimized, offsets_base,
                        DirWalkMode_ReadDirectory))
    {
        fprintf(stderr, "Error reading directory offsets from %s.\n",
                filename_optimized);
        goto failure;
    }
    for (int file_i = 0; file_i < 2; ++file_i)
    {
        const char *filename =
            (file_i == 0) ? filename_optimized : filename_non_optimized;

        for (enum DirWalkMode mode = DirWalkMode_ReadDirectory;
             mode <= DirWalkMode_SetDirectory_Reverse; ++mode)
        {
            if (get_dir_offsets(filename, offsets_comparison, mode))
            {
                fprintf(stderr,
                        "Error reading directory offsets from %s in mode %d.\n",
                        filename, mode);
                goto failure;
            }
            for (i = 0; i < N_DIRECTORIES; i++)
            {
                if (offsets_base[i] != offsets_comparison[i])
                {
                    fprintf(stderr,
                            "Unexpected directory offset for directory %i, "
                            "expected "
                            "offset %" PRIu64 " but got %" PRIu64 ".\n",
                            i, offsets_base[i], offsets_comparison[i]);
                    goto failure;
                }
            }
        }
    }

    unlink(filename_optimized);
    unlink(filename_non_optimized);

    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    unlink(filename_optimized);
    unlink(filename_non_optimized);
    return 1;
}

int main()
{
    if (test_lastdir_offset(false))
    {
        fprintf(stderr, "Failed during non-BigTIFF WriteDirectory test.\n");
        return 1;
    }
    if (test_lastdir_offset(true))
    {
        fprintf(stderr, "Failed during BigTIFF WriteDirectory test.\n");
        return 1;
    }

    if (test_rewrite_lastdir_offset(false))
    {
        fprintf(stderr, "Failed during non-BigTIFF RewriteDirectory test.\n");
        return 1;
    }
    if (test_rewrite_lastdir_offset(true))
    {
        fprintf(stderr, "Failed during BigTIFF RewriteDirectory test.\n");
        return 1;
    }

    /* Test arbitrary directory loading */
    if (test_arbitrary_directrory_loading(false))
    {
        fprintf(stderr,
                "Failed during non-BigTIFF ArbitraryDirectoryLoading test.\n");
        return 1;
    }
    if (test_arbitrary_directrory_loading(true))
    {
        fprintf(stderr,
                "Failed during BigTIFF ArbitraryDirectoryLoading test.\n");
        return 1;
    }

    /* Test SubIFD writing and reading */
    if (test_SubIFD_directrory_handling(false))
    {
        fprintf(stderr,
                "Failed during non-BigTIFF SubIFD_directrory_handling test.\n");
        return 1;
    }
    if (test_SubIFD_directrory_handling(true))
    {
        fprintf(stderr,
                "Failed during BigTIFF SubIFD_directrory_handling test.\n");
        return 1;
    }

    return 0;
}
