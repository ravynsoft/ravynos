# Intro
gettext-tiny provides lightweight replacements for tools typically used
from the `GNU gettext` suite, which is incredibly bloated and takes
a lot of time to build (in the order of an hour on slow devices).
the most notable component is `msgfmt` which is used to create binary
translation files in the `.mo` format out of textual input files in
`.po` format. this is the most important tool for building software from
source, because it is used from the build processes of many software packages.

our `msgfmt` implementation was initially a fake implementation that would
just output the english input string as the translation, which is sufficient
to get software to work, but it has since grown into a complete implementation.
unlike the GNU implementation, it can even expand input strings containing
so-called `sysdep` strings into a constant translation table.
`sysdep` strings were glued as an after-thought onto the GNU implementation to
deal with system-specific format strings, and in the GNU implementation those
are created at runtime, which requires mapping the entire `.mo` file into
read/write memory locations, thereby wasting precious RAM for read-only data
that could otherwise be shared among processes.

other parts of gettext-tiny such as `xgettext` and `msgmerge` are still stubs,
but they are sufficient to build many packages.

since `musl` libc, our preferred target, didn't provide a `libintl` in the past,
(and it's also part of `GNU gettext`) we also ship a no-op libintl providing
a header and a library.

it comes in two flavours:

1) nop: gettext functions just return the input string as translation
2) musl: a compat library providing a few compatibility symbols meant to be used
   together with the libintl built-in into recent musl versions.
   the compatibility symbols help to get past configure scripts that insist on
   using only the GNU gettext suite.
additionally, it can be entirely disabled.


# Compilation/Installation

```
make LIBINTL=FLAVOR
make LIBINTL=FLAVOR DESTDIR=pkgdir prefix=/ install
```

where FLAVOR can be one of NONE, MUSL, NOOP (as detailed above).
you can override any variables used in the Makefile (such as `CFLAGS`) by
appending them to the `make` invocation, or by saving them into a file called
`config.mak` before running `make`.
