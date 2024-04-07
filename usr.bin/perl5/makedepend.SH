#! /bin/sh
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

echo "Extracting makedepend (with variable substitutions)"
rm -f makedepend
$spitshell >makedepend <<!GROK!THIS!
$startsh
# makedepend.SH
#
MAKE=$make
trnl='$trnl'
!GROK!THIS!
$spitshell >>makedepend <<'!NO!SUBS!'

if test -d .depending; then
	echo "$0: Already running, exiting."
	exit 0
fi

mkdir .depending

# This script should be called with
#     sh ./makedepend MAKE=$(MAKE)
case "$1" in
	MAKE=*) eval $1; shift ;;
esac

export PATH || (echo "OOPS, this isn't sh.  Desperation time.  I will feed myself to sh."; sh \$0; kill \$\$)

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

# Avoid localized gcc messages
case "$ccname" in
    gcc) LC_ALL=C ; export LC_ALL ;;
esac

# We need .. when we are in the x2p directory if we are using the
# cppstdin wrapper script.
# Put .. and . first so that we pick up the present cppstdin, not
# an older one lying about in /usr/local/bin.
PATH=".$path_sep..$path_sep$PATH"
export PATH

case "$osname" in
amigaos) cat=/bin/cat ;; # must be absolute
esac

$cat /dev/null >.deptmp
$rm -f *.c.c c/*.c.c
if test -f Makefile; then
    rm -f $firstmakefile
    cp Makefile $firstmakefile
    # On QNX, 'cp' preserves timestamp, so $firstmakefile appears
    # to be out of date.  I don't know if OS/2 has touch, so do this:
    case "$osname" in
    os2) ;;
    *) $touch $firstmakefile ;;
    esac
fi
mf=$firstmakefile
if test -f $mf; then
    defrule=`<$mf sed -n		\
	-e '/^\.c\$(OBJ_EXT):.*;/{'	\
	-e    's/\$\*\.c//'		\
	-e    's/^[^;]*;[	 ]*//p'	\
	-e    q				\
	-e '}'				\
	-e '/^\.c\$(OBJ_EXT): *$/{'	\
	-e    N				\
	-e    's/\$\*\.c//'		\
	-e    's/^.*\n[	 ]*//p'		\
	-e    q				\
	-e '}'`
fi
case "$defrule" in
'') defrule='$(CC) -c $(CFLAGS)' ;;
esac

: Create files in UU directory to avoid problems with long filenames
: on systems with 14 character filename limits so file.c.c and file.c
: might be identical
$test -d UU || mkdir UU

$MAKE clist || ($echo "Searching for .c files..."; \
	$echo *.c | $tr ' ' $trnl | $egrep -v '\*' >.clist)

clist=`$cat .clist | $sed -e 's,$,.depends,'`

# Now, create a Makefile from .clist and run that in parallel
# Makefiles creating more Makefiles
# A Makefile exists to beget more Makefiles. Perl is
# just a vehicle.
rm -f $clist
$MAKE $clist
$cat $clist >.deptmp
rm -f $clist

$sed <$mf >$mf.new -e '1,/^# AUTOMATICALLY/!d'

if $test -s .deptmp; then
    $echo "Updating $mf..."
    $echo "# If this runs make out of memory, delete /usr/include lines." \
	>> $mf.new
    if [ "$osname" = vos ]; then
        $sed 's|\.incl\.c|.h|' .deptmp >.deptmp.vos
        mv -f .deptmp.vos .deptmp
    fi
    $sed -e 's|^\(.*\$(OBJ_EXT):\) *\(.*/.*\.c\) *$|\1 \2; '"$defrule \2|" \
       -e 'h; s/mini\(perlmain\)/\1/p; g' \
       .deptmp >>$mf.new
else
    $echo "Should not get here"
    exit 1
    $MAKE hlist || ($echo "Searching for .h files..."; \
	$echo *.h | $tr ' ' $trnl | $egrep -v '\*' >.hlist)
    $echo "You don't seem to have a proper C preprocessor.  Using grep instead."
    $egrep '^#include ' `cat .clist` `cat .hlist`  >.deptmp
    $echo "Updating $mf..."
    <.clist $sed -n							\
	-e '/\//{'							\
	-e   's|^\(.*\)/\(.*\)\.c|\2\$(OBJ_EXT): \1/\2.c; '"$defrule \1/\2.c|p"	\
	-e   d								\
	-e '}'								\
	-e 's|^\(.*\)\.c|\1\$(OBJ_EXT): \1.c|p' >> $mf.new
    <.hlist $sed -n 's|\(.*/\)\(.*\)|s= \2= \1\2=|p' >.hsed
    <.deptmp $sed -n 's|c:#include "\(.*\)".*$|o: \1|p' | \
       $sed 's|^[^;]*/||' | \
       $sed -f .hsed >> $mf.new
    <.deptmp $sed -n 's|h:#include "\(.*\)".*$|h: \1|p' | \
       $sed -f .hsed >> $mf.new
fi
$rm -f $mf.old
$cp $mf $mf.old
$rm -f $mf
$cp $mf.new $mf
$rm $mf.new
$echo "# WARNING: Put nothing here or make depend will gobble it up!" >> $mf
$rm -rf .deptmp UU .clist .hlist .hsed .cout .cerr
rmdir .depending

!NO!SUBS!
$eunicefix makedepend
chmod +x makedepend
