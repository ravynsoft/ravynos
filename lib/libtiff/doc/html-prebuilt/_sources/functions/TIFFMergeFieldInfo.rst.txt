TIFFMergeFieldInfo
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFMergeFieldInfo(TIFF* tif, const TIFFFieldInfo info[], uint32_t n)

Description
-----------

:c:func:`TIFFMergeFieldInfo` is used to add application-defined TIFF tags
to the list of known ``libtiff`` tags.
A brief description is given at :ref:`Define_Application_Tags`
and a description of the :c:struct:`TIFFFieldInfo` array elements can be
found at :ref:`TIFFFFieldInfo definition <TIFFFieldInfo_Definition>`.

Diagnostics
-----------

None.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`/addingtags`,
:doc:`TIFFSetTagExtender`
