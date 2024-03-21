#! /bin/sh

if test $# -lt 4; then
    echo >&2 "Usage: $0 <dist-tree-name> <top-source-dir> <top-build-dir> <type> <make-args>"
    exit 2
fi

case "$1" in
    /*) disttree=$1 ;;
    *) disttree=`pwd`/$1 ;;
esac

case "$2" in
    /*) sdir_top=$2 ;;
    *) sdir_top=`pwd`/$2 ;;
esac

case "$3" in
    /*) dir_top=$3 ;;
    *) dir_top=`pwd`/$3 ;;
esac

type=$4
shift 4

rm -rf $disttree

sed_separate='
    :1
    $!{
	N
	b1
    }
    s/\n/ /g
    s/^/deplist=;globlist=! /
    s/$/ !/
    s/  */ /g
    s/ \([^?*[!][^?*[!]*\) / !deplist="$deplist \1"! /g
    s/! !/;/g
    s/! \([^!]*\) !/;globlist="$globlist \1";/g
    s/!/;/g
    s/;;*/;/g
'

filelist=filelist$$
trap 'rm -f $filelist; rm -rf $disttree; exit 1' 1 2 15
(
    cd $sdir_top
    find . -name .git -prune -o -name '?*.*' -prune -o -name .distfiles -print
) > $filelist
( while read dfn; do
    subdir=`echo $dfn | sed 's,/\.distfiles$,,'`
    echo >&2 "Processing directory $subdir..."
    eval "DISTFILES_$type= DISTFILES_NOT="
    . $sdir_top/$dfn
    eval "distfiles=\$DISTFILES_$type"
    if [ $type = SRC ]; then
	# All files in git appear in the source bundle, unless
	# explicitly excluded with DISTFILES_NOT.
	distfiles="$distfiles
        `cd $sdir_top/$subdir; git ls-files | grep -v /`"
    fi
    if test -n "$distfiles"; then
	cmds=`echo "$distfiles" | sed -e "$sed_separate"`
	eval "$cmds"
	if test -n "$deplist" && test -f $dir_top/$subdir/Makefile; then
	    ( trap '' 1 2 15; cd $dir_top/$subdir && "$@" $deplist ) || exit 1
	fi
	$sdir_top/mkinstalldirs $disttree/$subdir || exit 1
	for f in $deplist `test -z "$globlist" || ( cd $dir_top/$subdir && eval "echo $globlist")`; do
	    for fnot in $DISTFILES_NOT; do
		if [ $fnot = $f ]; then
		    continue 2
		fi
	    done
	    if test -f $dir_top/$subdir/$f; then
#		ln $dir_top/$subdir/$f $disttree/$subdir/$f || \
		    cp -p $dir_top/$subdir/$f $disttree/$subdir/$f || exit 1
	    elif test -f $sdir_top/$subdir/$f; then
#		ln $sdir_top/$subdir/$f $disttree/$subdir/$f || \
		    cp -p $sdir_top/$subdir/$f $disttree/$subdir/$f || exit 1
	    else
		echo >&2 "$0: can't find file $subdir/$f"
		exit 1
	    fi
	done
    fi
done ) < $filelist

status=$?
rm -f $filelist
trap '' 1 2 15
if test $status -ne 0; then
    rm -rf $disttree
    exit $status
fi

exec chmod -R a+rX,u+w,g-s,go-w $disttree
