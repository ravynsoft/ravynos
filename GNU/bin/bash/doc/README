This directory contains the bash documentation.

FAQ		- a set of frequently-asked questions about Bash with answers
INTRO		- a short introduction to bash
article.ms	- an article I wrote about bash for The Linux Journal
bash.1		- the bash man page
builtins.1	- a man page that documents the builtins, extracted from bash.1
bashref.texi	- the `bash reference manual'
bashref.info	- the `bash reference manual' processed by `makeinfo'
readline.3	- the readline man page

The `.ps' files are postscript versions of the above.  The `.html'
files are HTML versions of the man page and reference manual.  The
`.0' files are formatted manual pages.  The `.txt' versions are
ascii -- the output of `groff -Tascii'.

The rest of this file explains how to use the `builtins.1' man page.

For each command in the list of builtins create a file in man/man1 called:

${command}.1

eg.
for.1
type.1
alias.1
etc.

All these files are identical as follows:

jaws@jaws(264)$ cat alias.1
.so man1/builtins.1
jaws@jaws(265)$ 

Make sure you adjust the .so line in builtins.1 to reflect where you
put it.
