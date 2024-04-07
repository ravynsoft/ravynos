:
# hints/posix-bc.sh
#
# BS2000 (Posix Subsystem) hints by Thomas Dorner <Thomas.Dorner@start.de>
#
# Thanks to the authors of the os390.sh for the very first draft.
#
# You can modify almost any parameter set here using Configure with
# the appropriate -D option.

# remove this line if dynamic libraries are working for you:
 bs2000_ignoredl='y'

# To get ANSI C, we need to use c89
# You can override this with Configure -Dcc=gcc
# (if you ever get a gcc ported to BS2000 ;-).
case "$cc" in
'') cc='c89' ;;
esac

# C-Flags:
# -DPOSIX_BC
# -DUSE_PURE_BISON
# -D_XOPEN_SOURCE_EXTENDED alters system headers.
# Prepend your favorites with Configure -Dccflags=your_favorites
ccflags="$ccflags -Kc_names_unlimited,enum_long,llm_case_lower,llm_keep,no_integer_overflow -DPOSIX_BC -DUSE_PURE_BISON -DYYMAXDEPTH=65000 -DYYINITDEPTH=1000 -D_XOPEN_SOURCE_EXTENDED"

# Now, what kind of BS2000 system are we running on?
echo
if [ -n "`bs2cmd SHOW-SYSTEM-INFO | egrep 'HSI-ATT.*TYPE.*SR'`" ]; then
    echo "You are running a BS2000 machine with Sunrise CPUs."
    echo "Let's hope you have the matching RISC compiler as well."
    ccflags="-K risc_4000 $ccflags"
    bs2000_ldflags='-K risc_4000'
else
    echo "Seems like a standard 390 BS2000 machine to me."
    bs2000_ldflags=''
fi
echo
if [ -z "$bs2000_ignoredl" -a -e /usr/lib/libdl.a ]; then
    echo "Wow, your BS2000 is State Of The Art and seems to support dynamic libraries."
    echo "I just can't resist giving them a try."
    bs2000_lddlflags='-Bsymbolic -Bdynamic'
    # dynamic linkage of system libraries gave us runtime linker
    # errors, so we use static linkage while generating our DLLs :-(
#    bs2000_lddlflags='-Bstatic'
    bs2000_so='none'
    bs2000_usedl='define'
    bs2000_dlext='so'
    case $bs2000_ldflags in
	*risc_4000*)
	    bs2000_ld="perl_genso"
	    echo "
Now you must buy everything they sell you, mustn't you?
Didn't somebody tell you that RISC machines and dynamic library support gives
you helluva lot of configuration problems at the moment?
Sigh.  Now you'll expect me to fix it for you, eh?
OK, OK, I'll give you a wrapper.
Just copy $bs2000_ld anywhere into your path before you try to install
additional modules!"

cat > $bs2000_ld <<EOF
#! /bin/sh
#
# Perl's wrapper for genso by Thomas.Dorner@start.de

 GENSO=/usr/bin/genso
 options=""
 params=""
while [[ \$# -gt 0 ]]; do
    case \$1 in
	-K)
	    shift
	    ;;
	-K*)
	    ;;
	*.a)
	    lib=\${1##*/lib}
	    options="\$options -L\${1%/lib*.a} -l\${lib%.a}"
	    ;;
	*.o)
	    params="\$params \$1"
	    ;;
	*)
	    options="\$options \$1"
    esac
    shift
done
echo \$GENSO \$options \$params
exec \$GENSO \$options \$params
EOF

	    chmod +x $bs2000_ld
	    if [[ -w /usr/local/bin && ! -f /usr/local/bin/$bs2000_ld ]]; then
		cp -p $bs2000_ld /usr/local/bin/$bs2000_ld
		echo "(Actually I just did that as well, have a look into /usr/local/bin.)"
	    fi
	    ;;
	*)
	    bs2000_ld='genso'
    esac
else
    if [ -e /usr/lib/libdl.a ]; then
	echo "Your BS2000 supports dynamic libraries, but you (or we ;-) decided to leave them alone."
    else
	echo "Your BS2000 does'n support dynamic libraries so we're just staying static."
    fi
    bs2000_ld='c89'
    bs2000_lddlflags=''
    bs2000_so='none'
    bs2000_usedl='n'
    bs2000_dlext='none'
fi

case "$ld" in
'') ld=$bs2000_ld ;;
esac

# ccdlflags have yet to be determined.
#case "$ccdlflags" in
#'') ccdlflags='-c' ;;
#esac

# cccdlflags have yet to be determined.
#case "$cccdlflags" in
#'') cccdlflags='' ;;
#esac

case "$ldflags" in
'') ldflags=$bs2000_ldflags ;;
esac

case "$lddlflags" in
'') lddlflags=$bs2000_lddlflags ;;
esac

# Turning on optimization breaks perl (CORE-DUMP):
# You can override this with Configure -Doptimize='-O' or somesuch.
case "$optimize" in
'') optimize='none' ;;
esac

# BS2000 doesn't use dynamic memory on its own (yet):
case "$so" in
'') so=$bs2000_so ;;
esac

case "$usemymalloc" in
'') usemymalloc='n' ;;
esac

# On BS2000/Posix, libc.a does not really hold anything at all,
# so running nm on it is pretty useless.
# You can override this with Configure -Dusenm.
case "$usenm" in
'') usenm='false' ;;
esac

#  Configure -Dusedl -Ddlext=.so -Ddlsrc=dl_dllload.xs.
case "$usedl" in
'') usedl=$bs2000_usedl ;;
esac
case "$dlext" in
'') dlext=$bs2000_dlext ;;
esac
#case "$dlsrc" in
#'') dlsrc='none' ;;
#esac
#case "$ldlibpthname" in
#'') ldlibpthname=LIBPATH ;;
#esac
