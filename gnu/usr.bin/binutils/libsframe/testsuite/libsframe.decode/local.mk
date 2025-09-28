check_PROGRAMS =
if HAVE_COMPAT_DEJAGNU
  check_PROGRAMS += %D%/be-flipping %D%/frecnt-1 %D%/frecnt-2
endif

%C%_be_flipping_SOURCES = %D%/be-flipping.c
%C%_be_flipping_LDADD = ${top_builddir}/libsframe.la
%C%_be_flipping_CPPFLAGS = -I${top_srcdir}/../include -Wall

%C%_frecnt_1_SOURCES = %D%/frecnt-1.c
%C%_frecnt_1_LDADD = ${top_builddir}/libsframe.la
%C%_frecnt_1_CPPFLAGS = -I${top_srcdir}/../include -Wall

%C%_frecnt_2_SOURCES = %D%/frecnt-2.c
%C%_frecnt_2_LDADD = ${top_builddir}/libsframe.la 
%C%_frecnt_2_CPPFLAGS = -I${top_srcdir}/../include -Wall
