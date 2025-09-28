microperl is supposed to be a really minimal perl, even more
minimal than miniperl.  No Configure is needed to build microperl,
on the other hand this means that interfaces between Perl and your
operating system are left very -- minimal.

All this is experimental.  If you don't know what to do with microperl
you probably shouldn't.  Please don't report bugs in microperl; fix the
bugs.  (Bugs reports about microperl without fixes/patches are equivalent
to wishlist requests - they won't be discarded, but they likely won't get
worked on either, unless they chance to coincide with someone's personal itch)

We assume ANSI C89 plus the following:
- <stddef.h>, <stdlib.h>
- rename()
- opendir(), readdir(), closedir() (via dirent.h)
- memchr(), memcmp(), memcpy(), memset() (via string.h)
- (a safe) putenv() (via stdlib.h)
- strtoul() (via stdlib.h)
(grep for 'define' in uconfig.sh.)
Also, Perl times() is defined to always return zeroes.

If you are still reading this and you are itching to try out microperl:

	make -f Makefile.micro

The defaults assume a little endian LP32 platform - ie long and pointers are
32 bits, so sizeof(long) and sizeof(void *) are 4
If your platform is little endian LP64 - ie long and pointers are 64 bits,
sizeof(long) and sizeof(void *) are 8, then you first need to run

	make -f Makefile.micro regen_uconfig64

to generate a suitable uconfig.h

If you make changes to uconfig.sh, run

	make -f Makefile.micro regen_uconfig

to regenerate uconfig.h.  (or regen_uconfig64 if you're editing uconfig64.sh)


If neither of the above default configs work on your platform, you might want
to try

	make -f Makefile.micro patch_uconfig

*before* the "make -f Makefile.micro".  This tries to minimally patch
the uconfig.sh using your *current* Perl so that your microperl has
the correct basic types and sizes and byteorder.
