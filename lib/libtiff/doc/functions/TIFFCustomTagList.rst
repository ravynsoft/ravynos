TIFFCustomTagList
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFGetTagListCount(TIFF* tif)

.. c:function:: uint32_t TIFFGetTagListEntry(TIFF* tif, int tag_index)

Description
-----------

:c:func:`TIFFGetTagListCount` returns the number of entries in the
custom tag list.

:c:func:`TIFFGetTagListEntry` returns the tag number of the (n.th - 1)
entry within the custom tag list.
If the :c:var:`tag_index` is larger or equal to the number of entries
in the tag list 0xFFFFFFFF `(=(uint32_t(-1))` is returned.

Note
----

The known tags to ``libtiff`` are define as 'named' tags and a lot of them
as *custom* tags. The custom tag definitions are handled in an internal
custom tag list. This list can also be extended by adding tag definitions
to that list, so that ``libtiff`` is aware of those tags.
See :ref:`Define_Application_Tags`

Diagnostics
-----------

none

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen`  (3tiff),
:doc:`TIFFError` (3tiff)
