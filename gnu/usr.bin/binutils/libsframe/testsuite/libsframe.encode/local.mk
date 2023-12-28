if HAVE_COMPAT_DEJAGNU
  check_PROGRAMS += %D%/encode-1
endif

%C%_encode_1_SOURCES = %D%/encode-1.c
%C%_encode_1_LDADD = ${top_builddir}/libsframe.la
%C%_encode_1_CPPFLAGS = -I${top_srcdir}/../include -Wall
