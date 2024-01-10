Multi Page / Multi Image TIFF
=============================

There may be more than one :ref:`Image File Directory (IFD) <ImageFileDirectory>`
in a TIFF file. Each IFD defines a :ref:`subfile <SubFile>`.

One potential use of *subfiles* is to describe related images,
such as the pages of a facsimile transmission or reduced-resolution images
of the first full-resolution image.
Such files are also named "*multi-page* TIFF" or "*multi-image* TIFF".

There are two mechanisms for storing multiple images in a TIFF file:

1.  A **main-IFD chain**, where the images are stored in linked IFDs (directories).
    This mechanism is widely used.
2.  A **SubIFD chain**, where additional images are stored within the SubIFD tag
    of a main-IFD. Such child images provide extra information for the parent image
    - such as a subsampled version of the parent image. 
    SubIFD chains are rarely supported.
    For SubIFD refer also to
    `Adobe PageMaker® 6.0 TIFF Technical Notes <https://www.awaresystems.be/imaging/tiff/specification/TIFFPM6.pdf>`_

.. _SubIFDAccess:

Sub IFD chains
--------------

In the case of several SubIFDs of a main image, there are two possibilities
that are not even mutually exclusive.

a. The ``SubIFD`` tag contains an array with all offsets of the SubIFDs.
   This is called SubIFD tree and DNG recommends the use of SubIFD trees,
   as described in the TIFF-EP specification. SubIFD chains are not supported.
b. The SubIFDs are concatenated with their ``NextIFD`` parameters
   to a SubIFD chain.

LibTiff does support SubIFD chains partially. When the first
``SubIFD`` tag is activated and read with :c:func:`TIFFSetSubDirectory()`,
the following can be parsed with :c:func:`TIFFReadDirectory()`.
The *tif_curdir* is just incremented from its current value
and thus gets arbitrary values when parsing through SubIFD chains.
When the SubIFDs are not chained, each offset
within the SubIFD array has to be activated and read individually.
:c:func:`TIFFSetDirectory()` only works with main-IFD chains because
allways starts with the first main-IFD and thus is able to reset
the SubIFD reading chain to the main-IFD chain.

Writing Multi Page TIFF
-----------------------

The following example code shows how to write multi-page TIFF
as main-IFD chain and as SubIFD chain.
``libtiff`` writes SubIFDs as an array of IFDs that are not chained
additionally, as Adobe PageMaker® 6.0 TIFF Technical Notes suggests.

.. highlight:: c

::

  #include <tiffio.h>

  int main(int argc, const char **argv)
  {
      TIFF *tiff;

      /* Define the number of pages/images (main-IFDs) you are going to write */
      int number_of_images = 3;

  /* Define the number of sub - IFDs you are going to write */
  #define NUMBER_OF_SUBIFDs 2
      int number_of_sub_IFDs = NUMBER_OF_SUBIFDs;
      toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs] = {
          0UL}; /* array for SubIFD tag */
      int blnWriteSubIFD = 0;

      if (!(tiff = TIFFOpen("multiPageTiff2.tif", "w")))
          return 1;

      for (int i = 0; i < number_of_images; i++)
      {
          char pixel[1] = {128};

          TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, 1);
          TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, 1);
          TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
          TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
          TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

          /* For the last but one multi-page image, add a SubIFD e.g. for a
          * thumbnail */
          if (number_of_images - 2 == i)
              blnWriteSubIFD = 1;

          if (blnWriteSubIFD)
          {
             /* Now here is the trick: the next n directories written
              * will be sub-IFDs of the main-IFD (where n is number_of_sub_IFDs
              * specified when you set the TIFFTAG_SUBIFD field.
              * The SubIFD offset array sub_IFDs_offsets is filled automatically
              * with the proper offset values by the following number_of_sub_IFDs
              * TIFFWriteDirectory() calls and updated in the related main-IFD
              * with the last call.
              */
              if (!TIFFSetField(tiff, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                                sub_IFDs_offsets))
                  return 1;
          }

          /* Write dummy pixel to image */
          if (TIFFWriteScanline(tiff, pixel, 0, 0) < 0)
              return 1;
          /* Write image / directory to file */
          if (!TIFFWriteDirectory(tiff))
              return 1;

          if (blnWriteSubIFD)
          {
             /* A SubIFD tag has been written for that main-IFD and this
              * triggers that pervious TIFFWriteDirectory() to switch to the
              * SubIFD-chain for the next number_of_sub_IFDs writings.
              * Thus, only the thumbnail images need to be
              * set up and written to file using TIFFWriteDirectory().
              * The last of this TIFFWriteDirectory() calls will setup
              * the next fresh main-IFD.
              */
              for (int i = 0; i < number_of_sub_IFDs; i++)
              {
                  TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, 1);
                  TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, 1);
                  TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
                  TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
                  TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
                  /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
                  * good idea to indicate thumbnails */
                  if (!TIFFSetField(tiff, TIFFTAG_SUBFILETYPE,
                                    FILETYPE_REDUCEDIMAGE))
                      return 1;

                  /* Write dummy pixel to thumbnail image */
                  pixel[0] = 64;
                  if (TIFFWriteScanline(tiff, pixel, 0, 0) < 0)
                      return 1;
                  /* Writes now in the SubIFD chain */
                  if (!TIFFWriteDirectory(tiff))
                      return 1;

                  blnWriteSubIFD = 0;
              }
          }
      }
      TIFFClose(tiff);
      return 0;
    }

Reading Multi Page TIFF
-----------------------

For a reading example see code of `tools/tiffinfo.c` or below:

.. highlight:: c

::

    /* Reading of multi-page and SubIFD images (subfiles) */
    if (!(tiff = TIFFOpen(filename, "r")))
        return 1;

    tdir_t currentDirNumber = TIFFCurrentDirectory(tiff);

    /* The first directory is already read through TIFFOpen() */
    int blnRead = 0;
    do
    {
        /*Check if there are SubIFD subfiles */
        void *ptr;
        if (TIFFGetField(tiff, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr))
        {
            /* Copy SubIFD array from pointer */
            memcpy(sub_IFDs_offsets, ptr,
                   number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));

            for (int i = 0; i < number_of_sub_IFDs; i++)
            {
                /* Read first SubIFD directory */
                if (!TIFFSetSubDirectory(tiff, sub_IFDs_offsets[i]))
                    return 1;
                /* Check if there is a SubIFD chain behind the first one from
                 * the array, as specified by Adobe */
                while (TIFFReadDirectory(tiff))
                    /* analyse subfile */
                    ;
            }
            /* Go back to main-IFD chain and re-read that main-IFD directory */
            if (!TIFFSetDirectory(tiff, currentDirNumber))
                return 1;
        }
        /* Read next main-IFD directory (subfile) */
        blnRead = TIFFReadDirectory(tiff);
        currentDirNumber = TIFFCurrentDirectory(tiff);
    } while (blnRead);
    TIFFClose(tiff);




See also
--------

:doc:`terms`,
:doc:`/functions/TIFFSetDirectory` (3tiff),
:doc:`/functions/TIFFWriteDirectory` (3tiff),
`Adobe PageMaker® 6.0 TIFF Technical Notes <https://www.awaresystems.be/imaging/tiff/specification/TIFFPM6.pdf>`_,
`Example from StackOverflow <https://stackoverflow.com/questions/11959617/in-a-tiff-create-a-sub-ifd-with-thumbnail-libtiff>`_