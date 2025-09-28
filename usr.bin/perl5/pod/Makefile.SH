case $PERL_CONFIG_SH in
'')
	if test -f config.sh; then TOP=.;
	elif test -f ../config.sh; then TOP=..;
	elif test -f ../../config.sh; then TOP=../..;
	elif test -f ../../../config.sh; then TOP=../../..;
	elif test -f ../../../../config.sh; then TOP=../../../..;
	else
		echo "Can't find config.sh."; exit 1
	fi
	. $TOP/config.sh
	;;
esac
: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

if test -d pod; then
  cd pod || exit 1
fi
POD=`echo *.pod`
MAN=`echo $POD|sed 's/\.pod/\.man/g'`
HTML=`echo $POD|sed 's/perltoc.pod//'|sed 's/\.pod/\.html/g'`
TEX=`echo $POD|sed 's/\.pod/\.tex/g'`

echo "Extracting pod/Makefile (with variable substitutions)"
: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.

$spitshell >Makefile <<!GROK!THIS!
# pod/Makefile
# This file is derived from pod/Makefile.SH.  Any changes made here will
# be lost the next time you run Configure.

POD = $POD

MAN = $MAN

# no perltoc.html
HTML = $HTML

TEX  = $TEX  

# The following is used to include the current directory in
# the dynamic loader path you are building a shared libperl.
LDLIBPTH = $ldlibpth

!GROK!THIS!

## In the following dollars and backticks do not need the extra backslash.
$spitshell >>Makefile <<'!NO!SUBS!'

HTMLROOT = /	# Change this to fix cross-references in HTML
POD2HTML_ARGS = --htmlroot=$(HTMLROOT) --podroot=.. --podpath=pod:lib:ext:vms
POD2HTML = ../ext/Pod-Html/pod2html
POD2MAN = ../cpan/podlators/pod2man
PODCHECKER =  ../cpan/Pod-Parser/podchecker

PERL = ../miniperl
PERLILIB = $(PERL) -I../lib
REALPERL = ../perl

all: man

man:	$(POD2MAN) $(MAN)

html:	$(POD2HTML) $(HTML)

toc perltoc.pod:	buildtoc
	$(PERLILIB) buildtoc

.SUFFIXES: .pm .pod

.SUFFIXES: .man

.pm.man:     $(POD2MAN)
	$(REALPERL) -I../lib $(POD2MAN) $*.pm >$*.man

.pod.man:     $(POD2MAN)
	$(REALPERL) -I../lib $(POD2MAN) $*.pod >$*.man

.SUFFIXES: .html

.pm.html:    $(POD2HTML)
	$(PERL) -I../lib $(POD2HTML) $(POD2HTML_ARGS) --infile=$*.pm --outfile=$*.html

.pod.html:    $(POD2HTML)
	$(PERL) -I../lib $(POD2HTML) $(POD2HTML_ARGS) --infile=$*.pod --outfile=$*.html

clean:
	rm -f $(MAN)
	rm -f $(HTML)
	rm -f $(TEX)
	rm -f pod2html-*cache
	rm -f *.aux *.log *.exe

realclean:	clean

distclean:	realclean

veryclean:	distclean
	-rm -f *~ *.orig

check:	$(PODCHECKER)
	@echo "checking..."; \
	$(PERL) -I../lib $(PODCHECKER) $(POD)
!NO!SUBS!
