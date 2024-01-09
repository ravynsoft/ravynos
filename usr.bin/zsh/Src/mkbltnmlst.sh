#! /bin/sh
#
# mkbltnmlst.sh: generate boot code for linked-in modules
#
# Written by Andrew Main
#

srcdir=${srcdir-`echo $0|sed 's%/[^/][^/]*$%%'`}
test "x$srcdir" = "x$0" && srcdir=.
test "x$srcdir" = "x"   && srcdir=.
CFMOD=${CFMOD-$srcdir/../config.modules}

bin_mods="`grep ' link=static' $CFMOD | sed -e '/^#/d' \
-e 's/ .*/ /' -e 's/^name=/ /'`"

x_mods="`grep ' load=yes' $CFMOD | sed -e '/^#/d' -e '/ link=no/d' \
-e 's/ .*/ /' -e 's/^name=/ /'`"

trap "rm -f $1; exit 1" 1 2 15

exec > $1

for x_mod in $x_mods; do
    modfile="`grep '^name='$x_mod' ' $CFMOD | sed -e 's/^.* modfile=//' \
      -e 's/ .*//'`"
    if test "x$modfile" = x; then
	echo >&2 "WARNING: no name for \`$x_mod' in $CFMOD (ignored)"
	continue
    fi
    case "$bin_mods" in
    *" $x_mod "*)
        echo "/* linked-in known module \`$x_mod' */"
	linked=yes
	;;
    *)
        echo "#ifdef DYNAMIC"
        echo "/* non-linked-in known module \`$x_mod' */"
	linked=no
    esac
    unset moddeps autofeatures autofeatures_emu
    . $srcdir/../$modfile
    if test "x$autofeatures" != x; then
        if test "x$autofeatures_emu" != x; then
            echo "  {"
	    echo "    char *zsh_features[] = { "
	    for feature in $autofeatures; do
		echo "      \"$feature\","
	    done
	    echo "      NULL"
	    echo "    }; "
	    echo "    char *emu_features[] = { "
	    for feature in $autofeatures_emu; do
		echo "      \"$feature\","
	    done
	    echo "      NULL"
	    echo "    }; "
	    echo "    autofeatures(\"zsh\", \"$x_mod\","
	    echo "       EMULATION(EMULATE_ZSH) ? zsh_features : emu_features,"
	    echo "       0, 1);"
	    echo "  }"
        else
	    echo "  if (EMULATION(EMULATE_ZSH)) {"
	    echo "    char *features[] = { "
	    for feature in $autofeatures; do
		echo "      \"$feature\","
	    done
	    echo "      NULL"
	    echo "    }; "
	    echo "    autofeatures(\"zsh\", \"$x_mod\", features, 0, 1);"
	    echo "  }"
	fi
    fi
    for dep in $moddeps; do
	echo "  add_dep(\"$x_mod\", \"$dep\");"
    done
    test "x$linked" = xno && echo "#endif"
done

echo
done_mods=" "
for bin_mod in $bin_mods; do
    q_bin_mod=`echo $bin_mod | sed 's,Q,Qq,g;s,_,Qu,g;s,/,Qs,g'`
    modfile="`grep '^name='$bin_mod' ' $CFMOD | sed -e 's/^.* modfile=//' \
      -e 's/ .*//'`"
    echo "/* linked-in module \`$bin_mod' */"
    unset moddeps
    . $srcdir/../$modfile
    for dep in $moddeps; do
	# This assumes there are no circular dependencies in the builtin
	# modules.  Better ordering of config.modules would be necessary
	# to enforce stricter dependency checking.
	case $bin_mods in
	    *" $dep "*)
		echo "    /* depends on \`$dep' */" ;;
	    *)	echo >&2 "ERROR: linked-in module \`$bin_mod' depends on \`$dep'"
		rm -f $1
		exit 1 ;;
	esac
    done
    echo "    {"
    echo "        extern int setup_${q_bin_mod} _((Module));"
    echo "        extern int boot_${q_bin_mod} _((Module));"
    echo "        extern int features_${q_bin_mod} _((Module,char***));"
    echo "        extern int enables_${q_bin_mod} _((Module,int**));"
    echo "        extern int cleanup_${q_bin_mod} _((Module));"
    echo "        extern int finish_${q_bin_mod} _((Module));"
    echo
    echo "        register_module(\"$bin_mod\","
    echo "                        setup_${q_bin_mod},"
    echo "                        features_${q_bin_mod},"
    echo "                        enables_${q_bin_mod},"
    echo "                        boot_${q_bin_mod},"
    echo "                        cleanup_${q_bin_mod}, finish_${q_bin_mod});"
    echo "    }"
    done_mods="$done_mods$bin_mod "
done
