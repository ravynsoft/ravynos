Debugging Tips
==============

Normally Mesa (and OpenGL) records but does not notify the user of
errors. It is up to the application to call ``glGetError`` to check for
errors. Mesa supports an environment variable, ``MESA_DEBUG``, to help
with debugging. If ``MESA_DEBUG`` is defined, a message will be printed
to stdout whenever an error occurs.

More extensive error checking is done in DEBUG builds
(``--buildtype debug`` for Meson).

In your debugger you can set a breakpoint in ``_mesa_error()`` to trap
Mesa errors.

There is a display list printing/debugging facility. See the end of
``src/mesa/main/dlist.c`` for details.
