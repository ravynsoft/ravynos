TIFFWarning
===========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>
    #include <stdarg.h>

.. c:function:: void TIFFWarning(const char* module, const char* fmt, ...)

.. c:function:: void TIFFWarningExt(thandle_t fd, const char* module, const char* fmt, ...)

.. c:function:: void TIFFWarningExtR(TIFF *tif, const char* module, const char* fmt, ...)

.. c:type:: void (*TIFFWarningHandler)(const char * module, const char* fmt, va_list ap)

.. c:type:: void (*TIFFWarningHandlerExt)(thandle_t fd, const char* module, const char* fmt, va_list ap)

.. c:type:: int (*TIFFWarningHandlerExtR)(TIFF* tif, void* user_data, const char* module, const char* fmt, va_list ap)

.. c:function:: TIFFWarningHandler TIFFSetWarningHandler(TIFFWarningHandler handler)

.. c:function:: TIFFWarningHandlerExt TIFFSetWarningHandlerExt(TIFFWarningHandlerExt handler)

Description
-----------

:c:func:`TIFFWarning` invokes the library-wide warning handler function
to (normally) write a warning message to the ``stderr``.
The *fmt* parameter is a :c:func:`printf` format string, and any number
arguments can be supplied. The *module* parameter is interpreted as a
string that, if non-zero, should be printed before the message; it
typically is used to identify the software module in which a warning is
detected.

Applications that desire to capture control in the event of a warning
should use :c:func:`TIFFSetWarningHandler` to override the default
warning handler. A :c:macro:`NULL` (0) warning handler function may be
installed to suppress warning messages.

Two more application-specific warning handler callbacks are available,
each with different call parameters and passing parameters to the handler.
Each handler is also linked with a warning message function, i.e.
:c:func:`TIFFWarningExt` and :c:func:`TIFFWarningExtR` if the application
intends to call the handler with those extended parameters.

:c:func:`TIFFWarningExt` provides a file handle as parameter.
Within ``libtiff`` :c:func:`TIFFWarningExt` is called passing ``tif->tif_clientdata``
as *fd*, which represents the TIFF file handle (file descriptor).
The application-specific and library-wide handler for :c:func:`TIFFWarningExt`
is setup with :c:func:`TIFFSetWarningHandlerExt`.

:c:func:`TIFFWarningExtR` (introduced with libtiff 4.5) is called with its
TIFF handle and thus provides access to a per-TIFF handle (re-entrant)
warning handler. That means for different TIFF handles, different warning
handlers can be setup. This application-specific handler
can be setup when a TIFF file is opened with one of the following functions:
:c:func:`TIFFOpenExt`, :c:func:`TIFFOpenWExt`, :c:func:`TIFFFdOpenExt`
or :c:func:`TIFFClientOpenExt`.
Furthermore, a **custom defined data structure** *user_data* for the
warning handler can be given along.

Note
----

Both functions :c:func:`TIFFWarning` and :c:func:`TIFFWarningExt`
each attempt to call both handler functions if they are defined.
First :c:func:`TIFFWarningHandler` is called and then :c:func:`TIFFWarningHandlerExt`.
However, :c:func:`TIFFWarning` passes a "0" as a file handle to
:c:func:`TIFFWarningHandlerExt`.

:c:func:`TIFFWarningExtR` tries first to call the per-TIFF handle defined
warning handler. If :c:func:`TIFFWarningHandlerExtR` is not defined or
returns 0, :c:func:`TIFFWarningHandler` and then :c:func:`TIFFWarningHandlerExt`
are called. From libtiff 4.5 onwards :c:func:`TIFFWarningExtR` is used
within the ``libtiff`` library.

Return values
-------------

:c:func:`TIFFSetWarningHandler` and :c:func:`TIFFSetWarningHandlerExt`
returns a reference to the previous warning handler function.

:c:func:`TIFFWarningHandlerExtR` returns an integer as "stop" to control the call
of furhter warning handler functions within :c:func:`TIFFWarningExtR`:

  - 0: both functions :c:func:`TIFFWarningHandler` and :c:func:`TIFFWarningHandlerExt` are called.
  - non-zero: no further warning message function is called.

See also
--------

:doc:`TIFFError` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`libtiff` (3tiff),
printf (3)
