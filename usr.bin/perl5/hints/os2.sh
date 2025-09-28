#! /bin/sh
# hints/os2.sh
# This file reflects the tireless work of
# Ilya Zakharevich <ilya@math.ohio-state.edu>
#
# Trimmed and comments added by 
#     Andy Dougherty  <doughera@lafayette.edu>
#     Exactly what is required beyond a standard OS/2 installation?
#     (see in README.os2)

# Note that symbol extraction code gives wrong answers (sometimes?) on
# gethostent and setsid.

# Optimization (GNU make 3.74 cannot be loaded :-():
emxload -m 30 sh.exe ls.exe tr.exe id.exe sed.exe # make.exe 
emxload -m 30 grep.exe egrep.exe fgrep.exe cat.exe rm.exe mv.exe cp.exe
emxload -m 30 uniq.exe basename.exe sort.exe awk.exe echo.exe

path_sep=\;

if test -f $sh.exe; then sh=$sh.exe; fi

startsh="#!$sh"
cc='gcc'

# Make denser object files and DLL
case "X$optimize" in
  X)
	optimize="-O2 -fomit-frame-pointer -malign-loops=2 -malign-jumps=2 -malign-functions=2 -s"
	ld_dll_optimize="-s"
	;;
esac

# Get some standard things (indented to avoid putting in config.sh):
 oifs="$IFS"
 IFS=" ;"
 set $MANPATH
 tryman="$@"
 set $LIBRARY_PATH
 libemx="$@"
 set $C_INCLUDE_PATH
 usrinc="$@"
 IFS="$oifs"
 tryman="`./UU/loc . /man $tryman`"
 tryman="`echo $tryman | tr '\\\' '/'`"
 
 # indented to avoid having it *two* times at start
 libemx="`./UU/loc os2.a /emx/lib $libemx`"

usrinc="`./UU/loc stdlib.h /emx/include $usrinc`"
usrinc="`dirname $usrinc | tr '\\\' '/'`"
libemx="`dirname $libemx | tr '\\\' '/'`"

if test -d $tryman/man1; then
  sysman="$tryman/man1"
else
  sysman="`./UU/loc . /man/man1 c:/man/man1 c:/usr/man/man1 d:/man/man1 d:/usr/man/man1 e:/man/man1 e:/usr/man/man1 f:/man/man1 f:/usr/man/man1 g:/man/man1 g:/usr/man/man1 /usr/man/man1`"
fi

emxpath="`dirname $libemx`"
if test ! -d "$emxpath"; then 
  emxpath="`./UU/loc . /emx c:/emx d:/emx e:/emx f:/emx g:/emx h:/emx /emx`"
fi

if test ! -d "$libemx"; then 
  libemx="$emxpath/lib"
fi
if test ! -d "$libemx"; then 
  if test -d "$LIBRARY_PATH"; then
    libemx="$LIBRARY_PATH"
  else
    libemx="`./UU/loc . X c:/emx/lib d:/emx/lib e:/emx/lib f:/emx/lib g:/emx/lib h:/emx/lib /emx/lib`"
  fi
fi

if test ! -d "$usrinc"; then 
  if test -d "$emxpath/include"; then 
    usrinc="$emxpath/include"
  else
    if test -d "$C_INCLUDE_PATH"; then
      usrinc="$C_INCLUDE_PATH"
    else
      usrinc="`./UU/loc . X c:/emx/include d:/emx/include e:/emx/include f:/emx/include g:/emx/include h:/emx/include /emx/include`"
    fi
  fi
fi

rsx="`./UU/loc rsx.exe undef $pth`"

if test "$libemx" = "X"; then echo "Cannot find C library!" >&2; fi

# Acute backslashitis:
libpth="`echo \"$LIBRARY_PATH\" | tr ';\\\' ' /'`"
libpth="$libpth $libemx/mt $libemx"

set `cmd /c emxrev -f emxlibcm`
emxcrtrev=$5
# indented to not put it into config.sh
  _defemxcrtrev=-D_EMX_CRT_REV_=$emxcrtrev

so='dll'

# Additional definitions:

firstmakefile='GNUmakefile'
exe_ext='.exe'

# We provide it
i_dlfcn='define'

# The default one uses exponential notation between 0.0001 and 0.1
d_Gconvert='gcvt_os2((x),(n),(b))'

cat > UU/uselongdouble.cbu <<'EOCBU'
# This script UU/uselongdouble.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use long doubles.
# If we will use them, let Configure choose us a Gconvert.
case "$uselongdouble:$d_longdbl:$d_sqrtl:$d_modfl" in
"$define:$define:$define:$define") d_Gconvert='' ;;
esac
EOCBU

# -Zomf build has a problem with _exit() *flushing*, so the test
# gets confused:
fflushNULL="define"

aout_d_shrplib='undef'
aout_useshrplib='false'
aout_obj_ext='.o'
aout_lib_ext='.a'
aout_ar='ar'
aout_plibext='.a'
aout_lddlflags="-Zdll $ld_dll_optimize"

# -D__ST_MT_ERRNO__ allows a quick relink with -Zmtd to check problems
# which may be due to linking with -Zmtd DLLs

# Cannot have 32000K stack: get SYS0170  ?!
if [ $emxcrtrev -ge 50 ]; then 
    aout_ldflags='-Zexe -Zsmall-conv -Zstack 16000 -D__ST_MT_ERRNO__'
else
    aout_ldflags='-Zexe -Zstack 16000 -D__ST_MT_ERRNO__'
fi

# To get into config.sh:
aout_ldflags="$aout_ldflags"

aout_d_fork='define'
aout_ccflags="-DDOSISH -DPERL_IS_AOUT -DOS2=2 -DEMBED -I. $_defemxcrtrev -D__ST_MT_ERRNO__"
aout_cppflags="-DDOSISH -DPERL_IS_AOUT -DOS2=2 -DEMBED -I. $_defemxcrtrev -D__ST_MT_ERRNO__"
aout_use_clib='c'
aout_usedl='undef'
aout_archobjs="os2.o dl_os2.o"
# Not listed in dynamic_ext, but needed for AOUT static_ext nevertheless
aout_extra_static_ext="OS2::DLL"

# variable which have different values for aout compile
used_aout='d_shrplib useshrplib plibext lib_ext obj_ext ar plibext d_fork lddlflags ldflags ccflags use_clib usedl archobjs cppflags'

if [ "$emxaout" != "" ]; then
    d_shrplib="$aout_d_shrplib"
    useshrplib="$aout_useshrplib"
    obj_ext="$aout_obj_ext"
    lib_ext="$aout_lib_ext"
    ar="$aout_ar"
    plibext="$aout_plibext"
    if [ $emxcrtrev -lt 50 ]; then 
	d_fork="$aout_d_fork"
    fi
    lddlflags="$aout_lddlflags"
    ldflags="$aout_ldflags"
    ccflags="$aout_ccflags"
    cppflags="$aout_cppflags"
    use_clib="$aout_use_clib"
    usedl="$aout_usedl"
else
    d_shrplib='define'
    useshrplib='true'
    obj_ext='.obj'
    lib_ext='.lib'
    ar='emxomfar'
    plibext='.lib'
    if [ $emxcrtrev -ge 50 ]; then 
	d_fork='define'
    else
	d_fork='undef'
    fi
    lddlflags="-Zdll -Zomf -Zmt -Zcrtdll -Zlinker /e:2"
    # Recursive regmatch may eat 2.5M of stack alone.
    ldflags='-Zexe -Zomf -Zmt -Zcrtdll -Zstack 32000 -Zlinker /e:2'
    if [ $emxcrtrev -ge 50 ]; then 
	ccflags="-Zomf -Zmt -DDOSISH -DOS2=2 -DEMBED -I. $_defemxcrtrev"
    else
	ccflags="-Zomf -Zmt -DDOSISH -DOS2=2 -DEMBED -I. -DEMX_BAD_SBRK $_defemxcrtrev"
    fi
    use_clib='c_import'
    usedl='define'
fi

# indented to miss config.sh
  _ar="$ar"

# To get into config.sh (should start at the beginning of line)
# or you can put it into config.over.
plibext="$plibext"
# plibext is not needed anymore.  Just directly set $libperl.
libperl="libperl${plibext}"

#libc="/emx/lib/st/c_import$lib_ext"
libc="$libemx/mt/$use_clib$lib_ext"

if test -r "$libemx/c_alias$lib_ext"; then 
    libnames="$libemx/c_alias$lib_ext"
fi
# otherwise puts -lc ???

# [Maybe we should just remove c from $libswanted ?]

# Test would pick up wrong rand, so we hardwire the value for random()
libs='-lsocket -lm -lbsd'
randbits=31
archobjs="os2$obj_ext dl_os2$obj_ext"

# Run files without extension with sh:
EXECSHELL=sh

cccdlflags='-Zdll'
dlsrc='dl_dlopen.xs'
ld='gcc'

#cppflags='-DDOSISH -DOS2=2 -DEMBED -I.'

# for speedup: (some patches to ungetc are also needed):
# Note that without this guy tests 8 and 10 of io/tell.t fail, with it 11 fails

stdstdunder=`echo "#include <stdio.h>" | cpp | egrep -c "char +\* +_ptr"`
d_stdstdio='define'
d_stdiobase='define'
d_stdio_ptr_lval='define'
d_stdio_cnt_lval='define'

if test "$stdstdunder" = 0; then
  stdio_ptr='((fp)->ptr)'
  stdio_cnt='((fp)->rcount)'
  stdio_base='((fp)->buffer)'
  stdio_bufsiz='((fp)->rcount + (fp)->ptr - (fp)->buffer)'
  ccflags="$ccflags -DMYTTYNAME"
  myttyname='define'
else
  stdio_ptr='((fp)->_ptr)'
  stdio_cnt='((fp)->_rcount)'
  stdio_base='((fp)->_buffer)'
  stdio_bufsiz='((fp)->_rcount + (fp)->_ptr - (fp)->_buffer)'
fi

# to put into config.sh
myttyname="$myttyname"

# To have manpages installed
nroff='nroff.cmd'
# above will be overwritten otherwise, indented to avoid config.sh
  _nroff='nroff.cmd'

# should be handled automatically by Configure now.
ln='cp'
# Will be rewritten otherwise, indented to not put in config.sh
  _ln='cp'
lns='cp'

nm_opt='-p'

####### We define these functions ourselves

d_strtoll='define'
d_strtoull='define'
d_getprior='define'
d_setprior='define'
d_usleep='define'
d_usleepproto='define'

# The next two are commented. pdksh handles #!, extproc gives no path part.
# sharpbang='extproc '
# shsharp='false'

# Commented:
#startsh='extproc ksh\\n#! sh'

# Find patch:
gnupatch='patch'
if (gnupatch -v || gnupatch --version)   2>&1 >/dev/null; then
    gnupatch=gnupatch
else
    if (gpatch -v || gpatch --version)   2>&1 >/dev/null; then
	gnupatch=gpatch
    else
	# They may have a special PATH during configuring
	if (patch -v || patch --version) 2>&1 >/dev/null; then
	    gnupatch="`./UU/loc patch.exe undef $pth`"
	fi
    fi
fi

for f in less.exe less.sh less.ksh less.cmd more.exe more.sh more.ksh more.cmd ; do
  if test -z "$pager"; then
    pager="`./UU/loc $f '' $pth`"
  fi
done
if test -z "$pager"; then
  pager='cmd /c more'
fi

# Apply patches if needed
case "$0$running_c_cmd" in
  *[/\\]Configure|*[/\\]Configure.|Configure|Configure.) # Skip Configure.cmd
    if test "Xyes" = "X$configure_cmd_loop"; then
	cat <<EOC >&2
!!!
!!! PANIC: Loop of self-invocations detected, aborting!
!!!
EOC
	exit 20
    fi
    configure_cmd_loop=yes
    export configure_cmd_loop

    configure_needs_patch=''
    if test -s ./os2/diff.configure; then
	if ! grep "^#OS2-PATCH-APPLIED" ./Configure > /dev/null; then
	    configure_needs_patch=yes	    
	fi
    fi
    if test -n "$configure_needs_patch"; then
	# Not patched!
	# Restore the initial command line arguments
	if test -f ./Configure.cmd ; then
	    cat <<EOC >&2
!!!
!!! I see that what is running is ./Configure.
!!! ./Configure is not patched, but ./Configure.cmd exists.
!!!
!!! You are supposed to run Configure.cmd, not Configure
!!!  after an automagic patching.
!!!
!!! If you insist on running Configure, you may
!!!  patch it manually from ./os2/diff.configure.
!!!
!!! However, I went through incredible hoolahoops, and I expect I can
!!!  auto-restart Configure.cmd myself.  I will start it with arguments:
!!!
!!!    Configure.cmd $args_exp
!!!
EOC
	    rp='Do you want to auto-restart Configure.cmd?'
	    dflt='y'
	    . UU/myread
	    case "$ans" in
		[yY]) echo >&4 "Okay, continuing." ;;
		*) echo >&4 "Okay, bye."
		   exit 2
		   ;;
	    esac
	    eval "set X $args_exp";
	    shift;
	    # Restore the output
	    exec Configure.cmd "$@" 1>&2
	    exit 2
	fi
	cat <<EOC >&2
!!!
!!! You did not patch ./Configure!
!!! I can create Configure.cmd and patch it from ./os2/diff.configure with the command
!!!
!!!   $gnupatch -b -p1 --output=Configure.cmd <./os2/diff.configure 2>&1 | tee 00_auto_patch
EOC
	rp='Do you want to auto-patch Configure to Configure.cmd?'
	dflt='y'
	. UU/myread
	case "$ans" in
		[yY]) echo >&4 "Okay, continuing." ;;
		*) echo >&4 "Okay, bye."
		   exit 2
		   ;;
	esac
	($gnupatch -b -p1 --output=Configure.cmd <./os2/diff.configure 2>&1 | tee 00_auto_patch) >&2
	cat <<EOC >&2
!!!
!!! The report of patching is copied to 00_auto_patch.
!!! Now we need to restart Configure.cmd with all the options.
!!!
EOC
	echo "extproc sh" > Configure.ctm
	( cat Configure.cmd >> Configure.ctm && mv -f Configure.ctm Configure.cmd ) || (echo "!!! Failure to add extproc-line to Configure.cmd." >&2 ; exit 21)
	cat <<EOC >&2
!!! I went through incredible hoolahoops, and I expect I can
!!!  auto-restart Configure.cmd myself.  I will start it with arguments:
!!!
!!!    Configure.cmd $args_exp
!!!
EOC
	rp='Do you want to auto-restart Configure.cmd?'
	dflt='y'
	. UU/myread
	case "$ans" in
		[yY]) echo >&4 "Okay, continuing." ;;
		*) echo >&4 "Okay, bye."
		   exit 2
		   ;;
	esac
	eval "set X $args_exp";
	shift;
	exec Configure.cmd "$@" 1>&2
	exit 2
    else
	if test -s ./os2/diff.configure; then
	    echo "!!! Apparently we are running a patched Configure." >&2
	else
	    echo "!!! Apparently there is no need to patch Configure." >&2
	fi
    fi 
    ;;
  *) echo "!!! Apparently we are running a renamed Configure: '$0'." >&2
esac

# This script UU/usethreads.cbu will get 'called-back' by Configure 
# after it has prompted the user for whether to use threads.
cat > UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
$define|true|[yY]*)
	ccflags="-Zmt $ccflags"
        cppflags="-Zmt $cppflags"  # Do we really need to set this?
        aout_ccflags="-DUSE_THREADS $aout_ccflags"
        aout_cppflags="-DUSE_THREADS $aout_cppflags"
        aout_lddlflags="-Zmt $aout_lddlflags"
        aout_ldflags="-Zmt $aout_ldflags"
	;;
esac
EOCBU

if test -z "$cryptlib"; then
	cryptlib=`UU/loc crypt$lib_ext "" $libpth`
	if $test -n "$cryptlib"; then
		cryptlib=-lcrypt
	else
		cryptlib=`UU/loc ufc$lib_ext "" $libpth`
		if $test -n "$cryptlib"; then
			cryptlib=-lufc
		fi
	fi
fi
if test -n "$cryptlib"; then
	libs="$libs $cryptlib"
	# d_crypt=define
fi

# Now install the external modules. We are in the ./hints directory.

cd ./os2/OS2

cp -rfu * ../../ext/

# Install tests:

cp -uf ../*.t ../../t/lib
for xxx in * ; do
	if $test -d $xxx/t; then
		cp -uf $xxx/t/*.t ../../t/lib
	else
		if $test -d $xxx; then
			cd $xxx
			for yyy in * ; do
				if $test -d $yyy/t; then
				    cp -uf $yyy/t/*.t ../../t/lib
				fi
			done
			cd ..
		fi
	fi
done

case "$ldlibpthname" in
'') ldlibpthname=none ;;
esac

# Now go back
cd ../..
cp os2/*.t t/lib
