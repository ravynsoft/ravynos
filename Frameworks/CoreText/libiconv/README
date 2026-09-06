            GNU LIBICONV - character set conversion library

This library provides an iconv() implementation, for use on systems which
don't have one, or whose implementation cannot convert from/to Unicode.

It provides support for the encodings:

    European languages
        ASCII, ISO-8859-{1,2,3,4,5,7,9,10,13,14,15,16},
        KOI8-R, KOI8-U, KOI8-RU,
        CP{1250,1251,1252,1253,1254,1257}, CP{850,866,1131},
        Mac{Roman,CentralEurope,Iceland,Croatian,Romania},
        Mac{Cyrillic,Ukraine,Greek,Turkish},
        Macintosh
    Semitic languages
        ISO-8859-{6,8}, CP{1255,1256}, CP862, Mac{Hebrew,Arabic}
    Japanese
        EUC-JP, SHIFT_JIS, CP932, ISO-2022-JP, ISO-2022-JP-2, ISO-2022-JP-1,
        ISO-2022-JP-MS
    Chinese
        EUC-CN, HZ, GBK, CP936, GB18030, GB18030:2022, EUC-TW, BIG5, CP950,
        BIG5-HKSCS, BIG5-HKSCS:2004, BIG5-HKSCS:2001, BIG5-HKSCS:1999,
        ISO-2022-CN, ISO-2022-CN-EXT
    Korean
        EUC-KR, CP949, ISO-2022-KR, JOHAB
    Armenian
        ARMSCII-8
    Georgian
        Georgian-Academy, Georgian-PS
    Tajik
        KOI8-T
    Kazakh
        PT154, RK1048
    Thai
        ISO-8859-11, TIS-620, CP874, MacThai
    Laotian
        MuleLao-1, CP1133
    Vietnamese
        VISCII, TCVN, CP1258
    Platform specifics
        HP-ROMAN8, NEXTSTEP
    Full Unicode
        UTF-8
        UCS-2, UCS-2BE, UCS-2LE
        UCS-4, UCS-4BE, UCS-4LE
        UTF-16, UTF-16BE, UTF-16LE
        UTF-32, UTF-32BE, UTF-32LE
        UTF-7
        C99, JAVA
    Full Unicode, in terms of 'uint16_t' or 'uint32_t'
        (with machine dependent endianness and alignment)
        UCS-2-INTERNAL, UCS-4-INTERNAL
    Locale dependent, in terms of 'char' or 'wchar_t'
        (with machine dependent endianness and alignment, and with OS and
        locale dependent semantics)
        char, wchar_t
        The empty encoding name "" is equivalent to "char": it denotes the
        locale dependent character encoding.

When configured with the option --enable-extra-encodings, it also provides
support for a few extra encodings:

    European languages
        CP{437,737,775,852,853,855,857,858,860,861,863,865,869,1125}
    Semitic languages
        CP864
    Japanese
        EUC-JISX0213, Shift_JISX0213, ISO-2022-JP-3
    Chinese
        BIG5-2003 (experimental)
    Turkmen
        TDS565
    Platform specifics
        ATARIST, RISCOS-LATIN1
    EBCDIC compatible (not ASCII compatible, very rarely used)
        European languages
            IBM-{037,273,277,278,280,282,284,285,297,423,500,870,871,875,880},
            IBM-{905,924,1025,1026,1047,1112,1122,1123,1140,1141,1142,1143},
            IBM-{1144,1145,1146,1147,1148,1149,1153,1154,1155,1156,1157,1158},
            IBM-{1165,1166,4971}
        Semitic languages
            IBM-{424,425,12712,16804}
        Persian
            IBM-1097
        Thai
            IBM-{838,1160}
        Laotian
            IBM-1132
        Vietnamese
            IBM-{1130,1164}
        Indic languages
            IBM-1137

It can convert from any of these encodings to any other, through Unicode
conversion.

It has also some limited support for transliteration, i.e. when a character
cannot be represented in the target character set, it can be approximated
through one or several similarly looking characters. Transliteration is
activated when "//TRANSLIT" is appended to the target encoding name.

libiconv is for you if your application needs to support multiple character
encodings, but that support lacks from your system.


Installation
------------

As usual for GNU packages:

    $ ./configure --prefix=[[PREFIX]]     where [[PREFIX]] is e.g. $HOME/local
    $ make
    $ make install

After installing GNU libiconv for the first time, it is recommended to
recompile and reinstall GNU gettext, so that it can take advantage of
libiconv.

On systems other than GNU/Linux, the iconv program will be internationalized
only if GNU gettext has been built and installed before GNU libiconv. This
means that the first time GNU libiconv is installed, we have a circular
dependency between the GNU libiconv and GNU gettext packages, which can be
resolved by building and installing either
  - first libiconv, then gettext, then libiconv again,
or (on systems supporting shared libraries, excluding AIX)
  - first gettext, then libiconv, then gettext again.
Recall that before building a package for the second time, you need to erase
the traces of the first build by running "make distclean".

This library installs:
  - a library 'libiconv.so',
  - a header file '<iconv.h>'.

To use it, simply #include <iconv.h> and use the functions.

To use it in a package that uses GNU autoconf and GNU automake:
  - Use gnulib-tool to import the Gnulib module 'iconv'. It consists
    of a couple of *.m4 files (iconv.m4 and its dependencies) and a
    file 'build-aux/config.rpath'.
  - Add to the link command line of libraries and executables that use
    the functions the placeholder @LIBICONV@ (or, if using libtool for
    the link, @LTLIBICONV@). In Makefile.am files, the right place for
    these additions are the *_LDADD variables.


Copyright
---------

The libiconv and libcharset _libraries_ and their header files are under LGPL,
see file COPYING.LIB.

The iconv _program_ and the documentation are under GPL, see file COPYING.


Download
--------

    https://ftp.gnu.org/gnu/libiconv/libiconv-1.19.tar.gz

Homepage
--------

    https://www.gnu.org/software/libiconv/

Bug reports
-----------

Report bugs
  - in the bug tracker at <https://savannah.gnu.org/projects/libiconv>
  - or by email to <bug-gnu-libiconv@gnu.org>.

Join the GNU project
--------------------

See file JOIN-GNU.


Bruno Haible <bruno@clisp.org>
