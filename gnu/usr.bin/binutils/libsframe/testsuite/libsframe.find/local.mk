if HAVE_COMPAT_DEJAGNU
  check_PROGRAMS += %D%/findfre-1 %D%/findfunc-1 %D%/plt-findfre-1
endif

%C%_findfre_1_SOURCES = %D%/findfre-1.c
%C%_findfre_1_LDADD = ${top_builddir}/libsframe.la
%C%_findfre_1_CPPFLAGS = -I${top_srcdir}/../include -Wall

%C%_findfunc_1_SOURCES = %D%/findfunc-1.c
%C%_findfunc_1_LDADD = ${top_builddir}/libsframe.la
%C%_findfunc_1_CPPFLAGS = -I${top_srcdir}/../include -Wall

%C%_plt_findfre_1_SOURCES = %D%/plt-findfre-1.c
%C%_plt_findfre_1_LDADD = ${top_builddir}/libsframe.la
%C%_plt_findfre_1_CPPFLAGS = -I${top_srcdir}/../include -Wall
