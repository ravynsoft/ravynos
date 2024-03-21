TIFFClientInfo
==============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: void *TIFFGetClientInfo(TIFF* tif, const char *name)

.. c:function:: void TIFFSetClientInfo(TIFF* tif, void *data, const char *name)

Description
-----------

.. TODO: Check explanation of clientinfo linked list intention and usage.

The *clientinfo* linked list provides a method to hand over user defined
data from one routine to another via the internal ``tif`` storage of the
``libtiff`` library.

:c:func:`TIFFGetClientInfo` returns a pointer to the data of the named entry
in the clientinfo-list. If the *name* is not found ``NULL`` is returned.

:c:func:`TIFFSetClientInfo` adds or replaces an entry in the clientinfo-list
with the *name* and the pointer to the *data* provided by the caller.

Diagnostics
-----------

None.

See also
--------

:doc:`libtiff` (3tiff),
