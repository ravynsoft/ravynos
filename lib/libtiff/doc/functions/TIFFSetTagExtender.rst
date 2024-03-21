TIFFSetTagExtender
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: TIFFExtendProc TIFFSetTagExtender(TIFFExtendProc extender)

Description
-----------

:c:func:`TIFFSetTagExtender` is used to register the merge function
for user defined tags as an extender callback with ``libtiff``.
A brief description can be found at :ref:`define_application_tags`.

.. note::

    :c:func:`TIFFSetTagExtender` cannot be used to extend custom directories
    (for example EXIF, GPS or other custom directories).
    Only the TIFF directory can be extended with additional tags.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`/addingtags` (3tiff),
:doc:`TIFFMergeFieldInfo` (3tiff)
