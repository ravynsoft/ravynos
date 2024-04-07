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

echo "Extracting makedepend_file (with variable substitutions)"
rm -f makedepend_file
$spitshell >makedepend_file <<!GROK!THIS!
$startsh
# makedepend_file.SH
#
MAKE=$make
trnl='$trnl'
!GROK!THIS!
$spitshell >>makedepend_file <<'!NO!SUBS!'

file=$1
shift
outfile=$1
shift

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

    case "$osname" in
    os2)      uwinfix="-e s,\\\\\\\\,/,g" ;;
    cygwin)   uwinfix="-e s,\\\\\\\\,/,g" ;;
    posix-bc) uwinfix="-e s/\\*POSIX(\\(.*\\))/\\1/" ;;
    vos)      uwinfix="-e s/\#/\\\#/" ;;
    *)        uwinfix="" ;;
    esac
    case "$file" in
    *.c) filebase=`basename $file .c` ;;
    *.y) filebase=`basename $file .y` ;;
    esac
    case "$file" in
    */*) finc="-I`echo $file | sed 's#/[^/]*$##'`" ;;
    *)   finc= ;;
    esac
    $echo "Finding dependencies for $filebase$_o"
    # Below, we strip out all but preprocessor directives.
    # We have to take care of situations like
    #       #if defined(FOO) BAR   /* comment line 1
    #                                 more comment lines */
    # If we just delete text starting from the '/*' to the end of line, we will
    # screw up cases like
    #      #if defined(FOO)    /* comment */ \
    #          && defined(BAR) /* comment */ \
    #          && defined(BAZ) /* comment */ \
    #               etc.
    # Also, in lines like
    #      #defined FOO(a,b)    a/**/b
    # the comment may be important and so needs to be retained.
    # This code processes the single-line comments first; it assumes there is
    # at most one straightforward comment per continued preprocessor line,
    # replacing each non-empty comment (and its surrounding white space) by a
    # single space.  (sed only has a greedy '*' quantifier, so this doesn't
    # work right if there are multiple comments per line, and strings can look
    # like comments to it; both are unlikely in a preprocessor statement.) Any
    # continuation line is joined, and the process repeated on the enlarged
    # line as long as there are continuations.  At the end, if there are any
    # comments remaining, they are either completely empty or are like the
    # first situation.  The latter are just deleted by first deleting to the
    # end of line (including preceding white space) things that start with '/*'
    # and the next char isn't a '*'; then things that start with '/**', but the
    # next char isn't a '/'.  (Subsequent lines of the comment are irrelevant
    # and get dropped.)  At the end, we unjoin very long lines to avoid
    # preprocessor limitations
    ( $echo "#line 2 \"$file\"";                               \
      $sed -n <$file                                           \
	-e "/^${filebase}_init(/q"                             \
        -e ': tstcont'                                         \
	-e '/^[ 	]*#/s|[ 	]*/\*..*\*/[ 	]*| |' \
        -e '/\\$/{'                                            \
            -e 'N'                                             \
            -e 'b tstcont'                                     \
        -e '}'                                                 \
	-e 's/\\\n//g'                                         \
	-e '/^#line/d'                                         \
	-e '/^[ 	]*#/{'                                 \
	    -e 's|[ 	]*/\*[^*].*$||'                        \
	    -e 's|[ 	]*/\*\*[^/].*$||'                      \
            -e 's/.\{255\}/&\\\n/g'                           \
	    -e p                                               \
	-e '}' | $grep -v ":" ) >UU/$file.c

    # We're not sure why this was there; the #endif is extraneous on modern z/OS
    #if [ "$osname" = os390 -a "$file" = perly.c ]; then
    #   $echo '#endif' >>UU/$file.c
    #fi

    if [ "$osname" = os390 ]; then
        $cppstdin -DPERL_CORE $finc -I. $cppflags $cppminus <UU/$file.c |
        $sed \
    	    -e '/^#.*<stdin>/d' \
	    -e '/^#.*"-"/d' \
	    -e '/^#.*git_version\.h/d' \
	    -e 's#\.[0-9][0-9]*\.c#'"$file.c#" \
	    -e 's/^[	 ]*#[	 ]*line/#/' \
	    -e '/^# *[0-9][0-9]* *[".\/]/!d' \
	    -e 's/^.*"\(.*\)".*$/'$filebase'\$(OBJ_EXT): \1/' \
	    -e 's/^# *[0-9][0-9]* \(.*\)$/'$filebase'\$(OBJ_EXT): \1/' \
	    -e 's|: \./|: |' \
	    -e 's|\.c\.c|.c|' $uwinfix | \
        $uniq | $sort | $uniq >>$outfile
    else
        $cppstdin -DPERL_CORE $finc -I. $cppflags $cppminus <UU/$file.c >$$.cout 2>$$.cerr
        $sed \
	    -e '1d' \
	    -e '/^#.*<stdin>/d' \
            -e '/^#.*<builtin>/d' \
            -e '/^#.*<built-in>/d' \
            -e '/^#.*<command line>/d' \
            -e '/^#.*<command-line>/d' \
	    -e '/^#.*"-"/d' \
	    -e '/^#.*"\/.*\/"/d' \
	    -e '/: file path prefix .* never used$/d' \
	    -e '/^#.*git_version\.h/d' \
	    -e 's#\.[0-9][0-9]*\.c#'"$file.c#" \
	    -e 's/^[	 ]*#[	 ]*line/#/' \
	    -e '/^# *[0-9][0-9]* *[".\/]/!d' \
	    -e 's/^.*"\(.*\)".*$/'$filebase'\$(OBJ_EXT): \1/' \
	    -e 's/^# *[0-9][0-9]* \(.*\)$/'$filebase'\$(OBJ_EXT): \1/' \
	    -e 's|: \./|: |' \
           -e 's|\.c\.c|.c|' $uwinfix $$.cout $$.cerr| \
        $uniq | $sort | $uniq >>$outfile
        rm $$.cout
        rm $$.cerr
    fi
    $echo "$filebase\$(OBJ_EXT): $@" >>$outfile

!NO!SUBS!
$eunicefix makedepend_file
chmod +x makedepend_file
