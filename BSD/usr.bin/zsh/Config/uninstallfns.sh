#!/bin/sh

fndir=$DESTDIR$fndir
scriptdir=$DESTDIR$scriptdir

allfuncs="`grep ' functions=' ${dir_top}/config.modules |
  sed -e '/^#/d' -e '/ link=no/d' -e 's/^.* functions=//'`"

allfuncs="`cd ${sdir_top}; echo ${allfuncs}`"

case $fndir in
  *$VERSION*)
     # Version specific function directory, safe to remove completely.
     rm -rf $fndir
     ;;
  *) # The following will only apply with a custom install directory
     # with no version information.  This is rather undesirable.
     # But let's try and do the best we can.
     # We now have a list of files, but we need to use `test -f' to check
     # (1) the glob got expanded (2) we are not looking at directories.
     for file in $allfuncs; do
       case $file in
       Scripts/*)
	 ;;
       *)
         if test -f $sdir_top/$file; then
	   if test x$FUNCTIONS_SUBDIRS != x -a x$FUNCTIONS_SUBDIRS != xno; then
	     file=`echo $file | sed -e 's%%^(Functions|Completion)/%'`
	     rm -f $fndir/$file
	   else
	     bfile="`echo $file | sed -e 's%^.*/%%'`"
	     rm -f "$fndir/$bfile"
	   fi
         fi
	 ;;
       esac
     done
     ;;
esac

case $scriptdir in
  *$VERSION*)
     # $scriptdir might be the parent of fndir.
     rm -rf $scriptdir
     ;;
  *) for file in $allfuncs; do
	case $file in
	Scripts/*)
	  if test -f $sdir_top/$file; then
	    bfile="`echo $file | sed -e 's%^.*/%%'`"
	    rm -f "$scriptdir/$bfile"
	  fi
	  ;;
	esac
     done
     ;;
esac

exit 0
