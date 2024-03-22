Help Wanted
===========

We can always use more help with the Mesa project. Here are some
specific ideas and areas where help would be appreciated:

#. **Driver patching and testing.** Patches are often posted to the
   `mesa-dev mailing
   list <https://lists.freedesktop.org/mailman/listinfo/mesa-dev>`__,
   but aren't immediately checked into Git because not enough people are
   testing them. Just applying patches, testing and reporting back is
   helpful.
#. **Driver debugging.** There are plenty of open bugs in the `bug
   database <https://gitlab.freedesktop.org/mesa/mesa/-/issues>`__.
#. **Remove aliasing warnings.** Enable GCC's
   ``-Wstrict-aliasing=2 -fstrict-aliasing`` arguments, and track down
   aliasing issues in the code.
#. **Contribute more tests to**
   `Piglit <https://piglit.freedesktop.org/>`__.

You can find some further To-do lists here:

**Common To-Do lists:**

-  `features.txt <https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/docs/features.txt>`__
   - Status of OpenGL 3.x / 4.x features in Mesa.

**Legacy Driver specific To-Do lists:**

-  `r600g <https://dri.freedesktop.org/wiki/R600ToDo>`__ - Driver
   for ATI/AMD R600 - Northern Island.
-  `r300g <https://dri.freedesktop.org/wiki/R300ToDo>`__ - Driver
   for ATI R300 - R500.

If you want to do something new in Mesa, first join the Mesa developer's
mailing list. Then post a message to propose what you want to do, just
to make sure there's no issues.

Anyone is welcome to contribute code to the Mesa project. By doing so,
it's assumed that you agree to the code's licensing terms.

Finally:

#. Try to write high-quality code that follows the existing style.
#. Use uniform indentation, write comments, use meaningful identifiers,
   etc.
#. Test your code thoroughly. Include test programs if appropriate.
