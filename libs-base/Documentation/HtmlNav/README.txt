
The files aside from "docs-web.html" here get installed into
'$GNUSTEP_SYSTEM_ROOT/Library/Documentation'.  They provide a simple means for
the user to navigate through the various GNUstep documentation that gets
installed there.  It includes links for the GUI documentation, which may not
necessarily be installed, but it points out that the links might not work.
This approach seemed better than maintaining two separate index files for GUI
and Base.

If you add documentation, you should also add links to it in this HTML file.

The file "docs-web.html" is meant to be installed on the gnustep.org web site.
The entire contents of a freshly built
$GNUSTEP_SYSTEM_ROOT/Library/Documentation should be put into a subdirectory
of that site's main directory, then the file "docs-web.html" copied there.
(Its name can be changed if needed, and the other html, css, and jpg files
can be removed.)
