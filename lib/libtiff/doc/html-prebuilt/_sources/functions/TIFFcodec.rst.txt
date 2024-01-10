TIFFcodec
=========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: const TIFFCodec* TIFFFindCODEC(uint16_t scheme)

.. c:function:: TIFFCodec* TIFFRegisterCODEC(uint16_t scheme, const char *method, TIFFInitMethod init)

.. c:function:: void TIFFUnRegisterCODEC(TIFFCodec * codec)

.. c:function:: int TIFFIsCODECConfigured(uint16_t scheme)

.. c:function:: TIFFCodec* TIFFGetConfiguredCODECs(uint16_t scheme)

.. c:function:: int TIFFSetCompressionScheme(TIFF* tif, int scheme)


Description
-----------

:program:`libtiff` supports a variety of compression schemes implemented
by software *codecs*. Each codec adheres to a modular interface that
provides for the decoding and encoding of image data; as well as some
other methods for initialization, setup, cleanup, and the control of
default strip and tile sizes.  Codecs are identified by the associated
value of the TIFF ``Compression`` tag; e.g. 5 for LZW compression.

.. ToDo: Describe functionality of next functions

:c:func:`TIFFFindCODEC`  ??????

:c:func:`TIFFUnRegisterCODEC` ?????

The :c:func:`TIFFRegisterCODEC` routine can be used to augment or
override the set of codecs available to an application.  If the
specified *scheme* already has a registered codec then it is
*overridden* and any images with data encoded with this compression
scheme will be decoded using the supplied codec.

:c:func:`TIFFIsCODECConfigured` returns 1 if the codec is configured
and working. Otherwise 0 will be returned.

:c:func:`TIFFGetConfiguredCODECs` gets list of configured codecs,
both built-in and registered by user. Function returns array of
:c:type:`TIFFCodec` records (the last record should be NULL) or NULL
if function failed. Caller is responsible to free this structure.

:c:func:`TIFFSetCompressionScheme`  ????


Diagnostics
-----------

``No space to register compression scheme %s``:

  :c:func:`TIFFRegisterCODEC` was unable to allocate memory for the
  data structures needed to register a codec.

``Cannot remove compression scheme %s; not registered``:

  :c:func:`TIFFUnRegisterCODEC` did not locate the specified codec in
  the table of registered compression schemes.

See also
--------

:doc:`libtiff` (3tiff),
