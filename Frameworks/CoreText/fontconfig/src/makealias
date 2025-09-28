#!/bin/sh
SRCDIR=$1
shift
HEAD=$1
shift
TAIL=$1
shift
rm -f $HEAD $TAIL
echo "#if HAVE_GNUC_ATTRIBUTE" >> $TAIL
cat "$@" | grep '^Fc[^ ]* *(' | sed -e 's/ *(.*$//' |
while read name; do
	case $name in
	FcCacheDir|FcCacheSubdir)
		;;
	*)
		alias="IA__$name"
		hattr='FC_ATTRIBUTE_VISIBILITY_HIDDEN'
		echo "extern __typeof ($name) $alias $hattr;" >> $HEAD
		echo "#define $name $alias" >> $HEAD
		ifdef=`grep -l '^'$name'[ (]' "$SRCDIR"/*.c | sed -n 1p | sed -e 's/^.*\/\([^.]*\)\.c/__\1__/'`
		if [ -z "$ifdef" ] ; then
			echo "error: could not locate $name in src/*.c" 1>&2
			exit 1
		fi
		if [ "$ifdef" != "$last" ] ; then
			[ -n "$last" ] && echo "#endif /* $last */" >> $TAIL
			echo "#ifdef $ifdef" >> $TAIL
			last=$ifdef
		fi
		echo "# undef $name" >> $TAIL
		cattr='__attribute((alias("'$alias'"))) FC_ATTRIBUTE_VISIBILITY_EXPORT'
		echo "extern __typeof ($name) $name $cattr;" >> $TAIL
		;;
	esac
done
[ $? -ne 0 ] && exit 1
echo "#endif /* $ifdef */" >> $TAIL
echo "#endif /* HAVE_GNUC_ATTRIBUTE */" >> $TAIL
