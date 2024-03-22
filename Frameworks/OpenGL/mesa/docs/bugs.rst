Report a Bug
============

To file a Mesa bug, go to `GitLab on
freedesktop.org <https://gitlab.freedesktop.org/mesa/mesa/-/issues>`__.

Please follow these bug reporting guidelines:

-  Check if a new version of Mesa is available which might have fixed
   the problem.
-  Check if your bug is already reported in the database.
-  Monitor your bug report for requests for additional information, etc.
-  Attach the output of running glxinfo or wglinfo. This will tell us
   the Mesa version, which device driver you're using, etc.
-  If you're reporting a crash, try to use your debugger (gdb) to get a
   stack trace. Also, recompile Mesa in debug mode to get more detailed
   information.
-  Describe in detail how to reproduce the bug, especially with games
   and applications that the Mesa developers might not be familiar with.
-  Provide an `apitrace <https://github.com/apitrace/apitrace>`__ or
   simple GLUT-based test program if possible.

The easier a bug is to reproduce, the sooner it will be fixed. Please do
everything you can to facilitate quickly fixing bugs. If your bug report
is vague or your test program doesn't compile easily, the problem may
not be fixed very quickly.
