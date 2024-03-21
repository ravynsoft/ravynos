#!/bin/sh

fndir=$DESTDIR$fndir
scriptdir=$DESTDIR$scriptdir

/bin/sh $sdir_top/mkinstalldirs $fndir || exit 1;

allfuncs="`grep ' functions=.' ${dir_top}/config.modules |
  sed -e '/^#/d' -e '/ link=no/d' -e 's/^.* functions=//'`"

allfuncs="`cd $sdir_top; echo ${allfuncs}`"

test -d installfnsdir || mkdir installfnsdir

# We now have a list of files, but we need to use `test -f' to check
# (1) the glob got expanded (2) we are not looking at directories.
for file in $allfuncs; do
  if test -f $sdir_top/$file; then
    case "$file" in
      */CVS/*) continue;;
    esac
    if test x$FUNCTIONS_SUBDIRS != x && test x$FUNCTIONS_SUBDIRS != xno; then
      case "$file" in
      Completion/*/*)
        subdir="`echo $file | sed -e 's%/[^/]*/[^/]*$%%'`"
        instdir="$fndir/$subdir"
        ;;
      Completion/*)
        instdir="$fndir/Completion"
        ;;
      Scripts/*)
        instdir="$scriptdir"
	;;
      *)
        subdir="`echo $file | sed -e 's%/[^/]*$%%' -e 's%^Functions/%%'`"
        instdir="$fndir/$subdir"
        ;;
      esac
    else
      case "$file" in
      Scripts/*)
        instdir="$scriptdir"
	;;
      *)
        instdir="$fndir"
        ;;
      esac
    fi
    basename=`basename $file`
    ok=0
    if test -d $instdir || /bin/sh $sdir_top/mkinstalldirs $instdir; then
      if sed "s|@runhelpdir@|$runhelpdir|" <$sdir_top/$file \
        >installfnsdir/$basename; then
	if $INSTALL_DATA installfnsdir/$basename $instdir; then
	  ok=1
	fi
      fi
    fi
    case $ok in
      0)
      rm -rf installfnsdir
      exit 1
      ;;
    esac
    read line < $sdir_top/$file
    case "$line" in
      '#!'*)
      chmod +x $instdir/`echo $file | sed -e 's%^.*/%%'`
      ;;
    esac
  fi
done

rm -rf installfnsdir
