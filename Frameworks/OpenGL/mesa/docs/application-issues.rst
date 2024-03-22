Application Issues
==================

This page documents known issues with some OpenGL applications.

Topogun
-------

`Topogun <https://www.topogun.com/>`__ for Linux (version 2, at least)
creates a GLX visual without requesting a depth buffer. This causes bad
rendering if the OpenGL driver happens to choose a visual without a
depth buffer.

Mesa 9.1.2 and later (will) support a DRI configuration option to work
around this issue. Using the
`driconf <https://dri.freedesktop.org/wiki/DriConf>`__ tool, set the
"Create all visuals with a depth buffer" option before running Topogun.
Then, all GLX visuals will be created with a depth buffer.

Old OpenGL games
----------------

Some old OpenGL games (approx. ten years or older) may crash during
start-up because of an extension string buffer-overflow problem.

The problem is a modern OpenGL driver will return a very long string for
the ``glGetString(GL_EXTENSIONS)`` query and if the application naively
copies the string into a fixed-size buffer it can overflow the buffer
and crash the application.

The work-around is to set the ``MESA_EXTENSION_MAX_YEAR`` environment
variable to the approximate release year of the game. This will cause
the ``glGetString(GL_EXTENSIONS)`` query to only report extensions older
than the given year.

For example, if the game was released in 2001, do

.. code-block:: console

   export MESA_EXTENSION_MAX_YEAR=2001

before running the game.

Viewperf
--------

See the :doc:`Viewperf issues <viewperf>` page for a detailed list of
Viewperf issues.
