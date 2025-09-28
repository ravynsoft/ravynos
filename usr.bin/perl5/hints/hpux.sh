#!/usr/bin/sh

### SYSTEM ARCHITECTURE

# Determine the architecture type of this system.
# Keep leading tab below -- Configure Black Magic -- RAM, 03/02/97
	xxOsRevMajor=`uname -r | sed -e 's/^[^0-9]*//' | cut -d. -f1`;
	xxOsRevMinor=`uname -r | sed -e 's/^[^0-9]*//' | cut -d. -f2`;
	xxOsRev=`expr 100 \* $xxOsRevMajor + $xxOsRevMinor`
if [ "$xxOsRevMajor" -ge 10 ]; then
    # This system is running >= 10.x

    # Tested on 10.01 PA1.x and 10.20 PA[12].x.
    # Idea: Scan /usr/include/sys/unistd.h for matches with
    # "#define CPU_* `getconf # CPU_VERSION`" to determine CPU type.
    # Note the text following "CPU_" is used, *NOT* the comment.
    #
    # ASSUMPTIONS: Numbers will continue to be defined in hex -- and in
    # /usr/include/sys/unistd.h -- and the CPU_* #defines will be kept
    # up to date with new CPU/OS releases.
    xxcpu=`getconf CPU_VERSION`; # Get the number.
    xxcpu=`printf '0x%x' $xxcpu`; # convert to hex
    archname=`sed -n -e "s/^#[[:space:]]*define[[:space:]]*CPU_//p" /usr/include/sys/unistd.h |
	sed -n -e "s/[[:space:]]*$xxcpu[[:space:]].*//p" |
	sed -e s/_RISC/-RISC/ -e s/HP_// -e s/_/./ -e "s/[[:space:]]*//g"`;
else
    # This system is running <= 9.x
    # Tested on 9.0[57] PA and [78].0 MC680[23]0.  Idea: After removing
    # MC6888[12] from context string, use first CPU identifier.
    #
    # ASSUMPTION: Only CPU identifiers contain no lowercase letters.
    archname=`getcontext | tr ' ' '\012' | grep -v '[a-z]' | grep -v MC688 |
	sed -e 's/HP-//' -e 1q`;
    selecttype='int *'
    fi

# For some strange reason, the u32align test from Configure hangs in
# HP-UX 10.20 since the December 2001 patches.  So hint it to avoid
# the test.
if [ "$xxOsRevMajor" -le 10 ]; then
    d_u32align=$define
    fi

echo "Archname is $archname"

# Fix XSlib (CPAN) confusion when re-using a prefix but changing from ILP32
# to LP64 builds.  They're NOT binary compatible, so quit claiming they are.
archname64=LP64


### HP-UX OS specific behaviour

# -ldbm is obsolete and should not be used
# -lBSD contains BSD-style duplicates of SVR4 routines that cause confusion
# -lPW is obsolete and should not be used
# The libraries crypt, malloc, ndir, and net are empty.
set `echo "X $libswanted " | sed -e 's/ ld / /' -e 's/ dbm / /' -e 's/ BSD / /' -e 's/ PW / /'`
shift
libswanted="$*"

cc=${cc:-cc}
ar=/usr/bin/ar	# Yes, truly override.  We do not want the GNU ar.
full_ar=$ar	# I repeat, no GNU ar.  arrr.

set `echo "X $ccflags " | sed -e 's/ -A[ea] / /' -e 's/ -D_HPUX_SOURCE / /'`
shift
	cc_cppflags="$* -D_HPUX_SOURCE"
cppflags="-Aa -D__STDC_EXT__ $cc_cppflags"

case "$prefix" in
    "") prefix='/opt/perl5' ;;
    esac

    gnu_as=no
    gnu_ld=no
case `$cc -v 2>&1`"" in
    *gcc*)  ccisgcc="$define"
	    ccflags="$cc_cppflags"
	    if [ "X$gccversion" = "X" ]; then
		# Done too late in Configure if hinted
		gccversion=`$cc -dumpversion`
		fi
	    case "$gccversion" in
		[012]*) # HP-UX and gcc-2.* break UINT32_MAX :-(
		    ccflags="$ccflags -DUINT32_MAX_BROKEN"
		    ;;
		[34]*) # GCC (both 32bit and 64bit) will define __STDC_EXT__
                       # by default when using GCC 3.0 and newer versions of
                       # the compiler.
		   cppflags="$cc_cppflags"
		   ;;
		esac
	    case "`getconf KERNEL_BITS 2>/dev/null`" in
		*64*)
		    echo "main(){}">try.c
		    case "$gccversion" in
			[34]*)
			    case "$archname" in
                               PA-RISC*)
                                   case "$ccflags" in
                                       *-mpa-risc*) ;;
                                       *) ccflags="$ccflags -mpa-risc-2-0" ;;
                                       esac
                                   ;;
				esac
			    ;;
			*)  # gcc with gas will not accept +DA2.0
			    case "`$cc -c -Wa,+DA2.0 try.c 2>&1`" in
				*"+DA2.0"*)		# gas
				    gnu_as=yes
				    ;;
				*)			# HPas
				    ccflags="$ccflags -Wa,+DA2.0"
				    ;;
				esac
			    ;;
			esac
		    # gcc with gld will not accept +vnocompatwarnings
		    case "`$cc -o try -Wl,+vnocompatwarnings try.c 2>&1`" in
			*"+vnocompat"*)		# gld
			    gnu_ld=yes
			    ;;
			*)			# HPld
			   case "$gccversion" in
			       [12]*)
				   # Why not 3 as well here?
				   # Since not relevant to IA64, not changed.
				   ldflags="$ldflags -Wl,+vnocompatwarnings"
				   ccflags="$ccflags -Wl,+vnocompatwarnings"
				   ;;
			       esac
			    ;;
			esac
		    rm -f try.c
		    ;;
		esac
	    ;;
    *)      ccisgcc=''
	    # What cannot be use in combination with ccache links :(
	    cc_found=""
	    for p in `echo $PATH | tr : ' ''` ; do
		x="$p/cc"
		if [ -f $x ] && [ -x $x ]; then
		    if [ -h $x ]; then
			l=`ls -l $x | sed 's,.*-> ,,'`
			case $l in
			    /*) x=$l		;;
			    *)  x="$p/$l"	;;
			    esac
			fi
		    x=`echo $x | sed 's,/\./,/,g'`
		    case $x in
			*ccache*) ;;
			*) [ -z "$cc_found" ] && cc_found=$x ;;
			esac
		    fi
		done
	    [ -z "$cc_found" ] && cc_found=`which cc`
	    what $cc_found >&4
	    ccversion=`what $cc_found | awk '/Compiler/{print $2}/Itanium/{print $6,$7}/for Integrity/{print $6,$7}'`
	    case "$ccflags" in
               "-Ae "*) ;;
		*)  ccflags="-Ae -Wp,-H150000 $cc_cppflags"
		    # +vnocompatwarnings not known in 10.10 and older
		    if [ $xxOsRev -ge 1020 ]; then
			ccflags="$ccflags -Wl,+vnocompatwarnings"
			fi
		    ;;
               esac
	    # Needed because cpp does only support -Aa (not -Ae)
	    cpplast='-'
	    cppminus='-'
	    cppstdin='cc -E -Aa -D__STDC_EXT__'
	    cpprun=$cppstdin
#	    case "$d_casti32" in
#		"") d_casti32='undef' ;;
#		esac
	    ;;
    esac

# When HP-UX runs a script with "#!", it sets argv[0] to the script name.
toke_cflags='ccflags="$ccflags -DARG_ZERO_IS_SCRIPT"'

### 64 BITNESS

# Some gcc versions do native 64 bit long (e.g. 2.9-hppa-000310 and gcc-3.0)
# We have to force 64bitness to go search the right libraries
    gcc_64native=no
case "$ccisgcc" in
    $define|true|[Yy])
	echo '#include <stdio.h>\nint main(){long l;printf("%d\\n",sizeof(l));}'>try.c
	$cc -o try $ccflags $ldflags try.c
	if [ "`try`" = "8" ]; then
	    case "$use64bitall" in
		$define|true|[Yy]) ;;
		*)  cat <<EOM >&4

*** This version of gcc uses 64 bit longs. -Duse64bitall is
*** implicitly set to enable continuation
EOM
		esac
	    use64bitall=$define
	    gcc_64native=yes
	    fi
	;;
    esac

case "$use64bitall" in
    $define|true|[yY]*) use64bitint="$define" ;;
    esac

case "$usemorebits" in
    $define|true|[yY]*) use64bitint="$define"; uselongdouble="$define" ;;
    esac

# There is a weird pre-C99 long double (a struct of four uin32_t)
# in HP-UX 10.20 but beyond strtold() there's no support for them
# for example in <math.h>.
case "$uselongdouble" in
    $define|true|[yY]*)
	if [ "$xxOsRevMajor" -lt 11 ]; then
	    cat <<EOM >&4

*** uselongdouble (or usemorebits) is not supported on HP-UX $xxOsRevMajor.
*** You need at least HP-UX 11.0.
*** Cannot continue, aborting.
EOM
	    exit 1
	fi
	;;
    esac

# Configure long double scan will detect the HP-UX 10.20 "long double"
# (a struct of four uin32_t) and think it is IEEE quad.  Make it not so.
if [ "$xxOsRevMajor" -lt 11 ]; then
    d_longdbl="$undef"
    longdblsize=8 # Make it double.
fi

case "$archname" in
    IA64*)
	# While here, override so=sl auto-detection
	so='so'
	;;
    esac

case "$use64bitall" in
    $define|true|[Yy])

	if [ "$xxOsRevMajor" -lt 11 ]; then
	    cat <<EOM >&4

*** 64-bit compilation is not supported on HP-UX $xxOsRevMajor.
*** You need at least HP-UX 11.0.
*** Cannot continue, aborting.
EOM
	    exit 1
	    fi

	if [ $xxOsRev -eq 1100 ]; then
	    # HP-UX 11.00 uses only 48 bits internally in 64bit mode, not 64
	    # force min/max to 2**47-1
	    sGMTIME_max=140737488355327
	    sGMTIME_min=-62167219200
	    sLOCALTIME_max=140737488355327
	    sLOCALTIME_min=-62167219200
	    fi

	# Set libc and the library paths
	case "$archname" in
	    PA-RISC*)
		loclibpth="$loclibpth /lib/pa20_64"
		libc='/lib/pa20_64/libc.sl' ;;
	    IA64*)
		loclibpth="$loclibpth /usr/lib/hpux64"
		libc='/usr/lib/hpux64/libc.so' ;;
	    esac
	if [ ! -f "$libc" ]; then
	    cat <<EOM >&4

*** You do not seem to have the 64-bit libc.
*** I cannot find the file $libc.
*** Cannot continue, aborting.
EOM
	    exit 1
	    fi

	case "$ccisgcc" in
	    $define|true|[Yy])
		# The fixed socket.h header file is wrong for gcc-4.x
		# on PA-RISC2.0W, so Sock_type_t is size_t which is
		# unsigned long which is 64bit which is too long
		case "$gccversion" in
		    4*) case "$archname" in
			    PA-RISC*) socksizetype=int ;;
			    esac
			;;
		    esac

		# For the moment, don't care that it ain't supported (yet)
		# by gcc (up to and including 2.95.3), cause it'll crash
		# anyway. Expect auto-detection of 64-bit enabled gcc on
		# HP-UX soon, including a user-friendly exit
		case $gcc_64native in
		    no) case "$gccversion" in
			    [1234]*)
				ccflags="$ccflags -mlp64"
				case "$archname" in
				    PA-RISC*)
					ldflags="$ldflags -Wl,+DD64"
					;;
				    IA64*)
					ldflags="$ldflags -mlp64"
					;;
				    esac
				;;
			    esac
			;;
		    esac
		;;
	    *)
		case "$use64bitall" in
		    $define|true|[yY]*)
			ccflags="$ccflags +DD64"
			ldflags="$ldflags +DD64"
			;;
		    esac
		;;
	    esac

	# Reset the library checker to make sure libraries
	# are the right type
	# (NOTE: on IA64, this doesn't work with .a files.)
	libscheck='case "`/usr/bin/file $xxx`" in
		       *ELF-64*|*LP64*|*PA-RISC2.0*) ;;
		       *) xxx=/no/64-bit$xxx ;;
		       esac'

	;;

    *)	# Not in 64-bit mode

	case "$archname" in
	    PA-RISC*)
		libc='/lib/libc.sl' ;;
	    IA64*)
		loclibpth="$loclibpth /usr/lib/hpux32"
		libc='/usr/lib/hpux32/libc.so' ;;
	    esac
	;;
    esac

# By setting the deferred flag below, this means that if you run perl
# on a system that does not have the required shared library that you
# linked it with, it will die when you try to access a symbol in the
# (missing) shared library.  If you would rather know at perl startup
# time that you are missing an important shared library, switch the
# comments so that immediate, rather than deferred loading is
# performed.  Even with immediate loading, you can postpone errors for
# undefined (or multiply defined) routines until actual access by
# adding the "nonfatal" option.
# ccdlflags="-Wl,-E -Wl,-B,immediate $ccdlflags"
# ccdlflags="-Wl,-E -Wl,-B,immediate,-B,nonfatal $ccdlflags"
if [ "$gnu_ld" = "yes" ]; then
    ccdlflags="-Wl,-E $ccdlflags"
else
    ccdlflags="-Wl,-E -Wl,-B,deferred $ccdlflags"
    fi


### COMPILER SPECIFICS

## Local restrictions (point to README.hpux to lift these)

## Optimization limits
cat >try.c <<EOF
#include <stdio.h>
#include <sys/resource.h>

int main ()
{
    struct rlimit rl;
    int i = getrlimit (RLIMIT_DATA, &rl);
    printf ("%d\n", (int)(rl.rlim_cur / (1024 * 1024)));
    } /* main */
EOF
$cc -o try $ccflags $ldflags try.c
	maxdsiz=`try`
rm -f try try.c core
if [ $maxdsiz -le 64 ]; then
    # 64 Mb is probably not enough to optimize toke.c
    # and regexp.c with -O2
    cat <<EOM >&4
Your kernel limits the data section of your programs to $maxdsiz Mb,
which is (sadly) not enough to fully optimize some parts of the
perl binary. I'll try to use a lower optimization level for
those parts. If you are a sysadmin, and you *do* want full
optimization, raise the 'maxdsiz' kernel configuration parameter
to at least 0x08000000 (128 Mb) and rebuild your kernel.
EOM
regexec_cflags=''
doop_cflags=''
op_cflags=''
opmini_cflags=''
perlmain_cflags=''
pp_pack_cflags=''
    fi

case "$ccisgcc" in
    $define|true|[Yy])

	case "$optimize" in
	    "")           optimize="-g -O" ;;
	    *O[3456789]*) optimize=`echo "$optimize" | sed -e 's/O[3-9]/O2/'` ;;
	    esac
	#ld="$cc"
	ld=/usr/bin/ld
	cccdlflags='-fPIC'
	#lddlflags='-shared'
	lddlflags='-b'
	case "$optimize" in
	    *-g*-O*|*-O*-g*)
		# gcc without gas will not accept -g
		echo "main(){}">try.c
		case "`$cc $optimize -c try.c 2>&1`" in
		    *"-g option disabled"*)
			set `echo "X $optimize " | sed -e 's/ -g / /'`
			shift
			optimize="$*"
			;;
		    esac
		;;
	    esac
	if [ $maxdsiz -le 64 ]; then
	    case "$optimize" in
		*O2*)
		    opt=`echo "$optimize" | sed -e 's/O2/O1/'`
		    toke_cflags="$toke_cflags;optimize=\"$opt\""
		    regexec_cflags="optimize=\"$opt\""
		    ;;
		esac
	    fi
	;;

    *)
	case "$optimize" in
	    "")           optimize="+O2 +Onolimit" ;;
	    *O[3456789]*) optimize=`echo "$optimize" | sed -e 's/O[3-9]/O2/'` ;;
	    esac
	case "$optimize" in
	    *-O*|\
	    *O2*)   opt=`echo "$optimize" | sed -e 's/-O/+O2/' -e 's/O2/O1/' -e 's/ *+Onolimit//'`
		    ;;
	    *)      opt="$optimize"
		    ;;
	    esac
	case "$archname" in
	    PA-RISC2.0)
		case "$ccversion" in
		    B.11.11.*)
			# opmini.c and op.c with +O2 makes the compiler die
			# of internal error, for perlmain.c only +O0 (no opt)
                        # works. Disable +Ox for pp_pack, as the optimizer
                        # causes this unit to fail (not a limit issue)
			case "$optimize" in
			*O[12]*)
			    opt=`echo "$optimize" | sed -e 's/O2/O1/' -e 's/ *+Onolimit//'`
			    opmini_cflags="optimize=\"$opt\""
			    op_cflags="optimize=\"$opt\""
			    perlmain_cflags="optimize=\"\""
			    pp_pack_cflags="optimize=\"\""
			    ;;
			esac
		    esac
		;;
	    IA64*)
		case "$ccversion" in
		    B3910B*A.06.0[12345])
			# > cc --version
			# cc: HP aC++/ANSI C B3910B A.06.05 [Jul 25 2005]
			# Has optimizing problems with -O2 and up for both
			# maint (5.8.8+) and blead (5.9.3+)
			# -O1/+O1 passed all tests (m)'05 [ 10 Jan 2005 ]
			optimize="$opt"			;;
			B3910B*A.06.15)
			# > cc --version
			# cc: HP C/aC++ B3910B A.06.15 [May 16 2007]
			# Has optimizing problems with +O2 for blead (5.17.4),
			# see https://github.com/Perl/perl5/issues/11748.
			#
			# +O2 +Onolimit +Onoprocelim  +Ostore_ordering \
			# +Onolibcalls=strcmp
			# passes all tests (with/without -DDEBUGGING) [Nov 17 2011]
			case "$optimize" in
			    *O2*) optimize="$optimize +Onoprocelim +Ostore_ordering +Onolibcalls=strcmp" ;;
			    esac
			;;
		    *)  doop_cflags="optimize=\"$opt\""
			op_cflags="optimize=\"$opt\""
			#opt=`echo "$optimize" | sed -e 's/O1/O0/'`
			globals_cflags="optimize=\"$opt\""	;;
		    esac
		;;
	    esac
	if [ $maxdsiz -le 64 ]; then
	    toke_cflags="$toke_cflags;optimize=\"$opt\""
	    regexec_cflags="optimize=\"$opt\""
	    fi
	ld=/usr/bin/ld
	cccdlflags='+Z'
	lddlflags='-b +vnocompatwarnings'
	;;
    esac

## LARGEFILES
if [ $xxOsRev -lt 1020 ]; then
    uselargefiles="$undef"
    fi

#case "$uselargefiles-$ccisgcc" in
#    "$define-$define"|'-define')
#	cat <<EOM >&4
#
#*** I'm ignoring large files for this build because
#*** I don't know how to do use large files in HP-UX using gcc.
#
#EOM
#	uselargefiles="$undef"
#	;;
#    esac

# Once we have the compiler flags defined, Configure will
# execute the following call-back script. See hints/README.hints
# for details.
cat > UU/cc.cbu <<'EOCBU'
# This script UU/cc.cbu will get 'called-back' by Configure after it
# has prompted the user for the C compiler to use.

# Compile and run the a test case to see if a certain gcc bug is
# present. If so, lower the optimization level when compiling
# pp_pack.c.  This works around a bug in unpack.

if test -z "$ccisgcc" -a -z "$gccversion"; then
    : no tests needed for HPc
else
    echo " "
    echo "Testing for a certain gcc bug is fixed in your compiler..."

    # Try compiling the test case.
    if $cc -o t001 -O $ccflags $ldflags -lm ../hints/t001.c; then
       gccbug=`$run ./t001`
       case "$gccbug" in
           *fails*)
               cat >&4 <<EOF
This C compiler ($gccversion) is known to have optimizer
problems when compiling pp_pack.c.

Disabling optimization for pp_pack.c.
EOF
               case "$pp_pack_cflags" in
                   '') pp_pack_cflags='optimize='
                       echo "pp_pack_cflags='optimize=\"\"'" >> config.sh ;;
                   *)  echo "You specified pp_pack_cflags yourself, so we'll go with your value." >&4 ;;
                   esac
               ;;
           *)  echo "Your compiler is ok." >&4
               ;;
           esac
    else
       echo " "
       echo "*** WHOA THERE!!! ***" >&4
       echo "    Your C compiler \"$cc\" doesn't seem to be working!" >&4
       case "$knowitall" in
           '') echo "    You'd better start hunting for one and let me know about it." >&4
               exit 1
               ;;
           esac
       fi

    rm -f t001$_o t001$_exe
    fi
EOCBU

cat >config.arch <<'EOCBU'
# This script UU/config.arch will get 'called-back' by Configure after
# all other configurations are done just before config.h is generated
case "$archname:$optimize" in
  PA*:*-g*[-+]O*|PA*:*[-+]O*-g*)
    case "$ccflags" in
      *DD64*) ;;
      *) case "$ccversion" in
	  # Only on PA-RISC. B3910B (aCC) is not faulty
	  # B.11.* and A.10.* are
	  [AB].1*)
	      # cc: error 1414: Can't handle preprocessed file foo.i if -g and -O specified.
	      echo "HP-UX C-ANSI-C on PA-RISC does not accept both -g and -O on preprocessed files" >&4
	      echo "when compiling in 32bit mode. The optimizer will be disabled." >&4
	      optimize=`echo "$optimize" | sed -e 's/[-+]O[0-9]*//' -e 's/+Onolimit//' -e 's/^ *//'`
	      ;;
	  esac
      esac
  esac
EOCBU

cat >UU/uselargefiles.cbu <<'EOCBU'
# This script UU/uselargefiles.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use large files.

case "$archname:$use64bitall:$use64bitint" in
    *-LP64*:undef:define)
	archname=`echo "$archname" | sed 's/-LP64/-64int/'`
	echo "Archname changed to $archname"
	;;
    esac

case "$uselargefiles" in
    ""|$define|true|[yY]*)
	# there are largefile flags available via getconf(1)
	# but we cheat for now.  (Keep that in the left margin.)
ccflags_uselargefiles="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"

	case " $ccflags " in
	*" $ccflags_uselargefiles "*) ;;
	*) ccflags="$ccflags $ccflags_uselargefiles" ;;
	esac

	if test -z "$ccisgcc" -a -z "$gccversion"; then
	    # The strict ANSI mode (-Aa) doesn't like large files.
	    ccflags=`echo " $ccflags "|sed 's@ -Aa @ @g'`
	    case "$ccflags" in
		*-Ae*) ;;
		*)     ccflags="$ccflags -Ae" ;;
		esac
	    fi
	;;
    esac
EOCBU

# THREADING

# This script UU/usethreads.cbu will get 'called-back' by Configure
# after it has prompted the user for whether to use threads.
cat >UU/usethreads.cbu <<'EOCBU'
case "$usethreads" in
    $define|true|[yY]*)
	if [ "$xxOsRevMajor" -lt 10 ]; then
	    cat <<EOM >&4

HP-UX $xxOsRevMajor cannot support POSIX threads.
Consider upgrading to at least HP-UX 11.
Cannot continue, aborting.
EOM
	    exit 1
	    fi

	if [ "$xxOsRevMajor" -eq 10 ]; then
	    # Under 10.X, a threaded perl can be built
	    if [ -f /usr/include/pthread.h ]; then
		if [ -f /usr/lib/libcma.sl ]; then
		    # DCE (from Core OS CD) is installed

		   # Check if it is pristine, or patched
		   cmavsn=`what /usr/lib/libcma.sl 2>&1 | grep 1996`
		   if [ ! -z "$cmavsn" ]; then
		       cat <<EOM >&4

***************************************************************************

Perl will support threading through /usr/lib/libcma.sl from
the HP DCE package, but the version found is too old to be
reliable.

If you are not depending on this specific version of the library,
consider to upgrade using patch PHSS_23672 (read README.hpux)

***************************************************************************

(sleeping for 10 seconds...)
EOM
		       sleep 10
		       fi

		    # It needs # libcma and OLD_PTHREADS_API. Also
		    # <pthread.h> needs to be #included before any
		    # other includes (in perl.h)

		    # HP-UX 10.X uses the old pthreads API
		    d_oldpthreads="$define"

		    # include libcma before all the others
		    libswanted="cma $libswanted"

		    # tell perl.h to include <pthread.h> before other
		    # include files
		    ccflags="$ccflags -DPTHREAD_H_FIRST"
# First column on purpose:
# this is not a standard Configure variable
# but we need to get this noticed.
pthread_h_first="$define"

		    # HP-UX 10.X seems to have no easy
		    # way of detecting these *time_r protos.
		    d_gmtime_r_proto='define'
		    gmtime_r_proto='REENTRANT_PROTO_I_TS'
		    d_localtime_r_proto='define'
		    localtime_r_proto='REENTRANT_PROTO_I_TS'

		    # Avoid the poisonous conflicting (and irrelevant)
		    # prototypes of setkey ().
		    i_crypt="$undef"

		    # CMA redefines select to cma_select, and cma_select
		    # expects int * instead of fd_set * (just like 9.X)
		    selecttype='int *'

		elif [ -f /usr/lib/libpthread.sl ]; then
		    # PTH package is installed
		    libswanted="pthread $libswanted"
		else
		    libswanted="no_threads_available"
		    fi
	    else
		libswanted="no_threads_available"
		fi

	    if [ $libswanted = "no_threads_available" ]; then
		cat <<EOM >&4

In HP-UX 10.X for POSIX threads you need both of the files
/usr/include/pthread.h and either /usr/lib/libcma.sl or /usr/lib/libpthread.sl.
Either you must upgrade to HP-UX 11 or install a posix thread library:

    DCE-CoreTools from HP-UX 10.20 Hardware Extensions 3.0 CD (B3920-13941)

or

    PTH package from e.g. http://hpux.connect.org.uk/hppd/hpux/Gnu/pth-2.0.7/

Cannot continue, aborting.
EOM
		exit 1
		fi
	else
	    # 12 may want upping the _POSIX_C_SOURCE datestamp...
	    ccflags=" -D_POSIX_C_SOURCE=199506L -D_REENTRANT $ccflags"
	    set `echo X "$libswanted "| sed -e 's/ c / pthread c /'`
	    shift
	    libswanted="$*"

	    # HP-UX 11.X seems to have no easy
	    # way of detecting these *time_r protos.
	    d_gmtime_r_proto='define'
	    gmtime_r_proto='REENTRANT_PROTO_S_TS'
	    d_localtime_r_proto='define'
	    localtime_r_proto='REENTRANT_PROTO_S_TS'
	    fi
	;;
    esac
EOCBU

# There used to be:
#  The mysterious io_xs memory corruption in 11.00 32bit seems to get
#  fixed by not using Perl's malloc.  Flip side is performance loss.
#  So we want mymalloc for all situations possible
# That set usemymalloc to 'n' for threaded builds and non-gcc 32bit
#  non-debugging builds and 'y' for all others

usemymalloc='n'
case "$useperlio" in
    $undef|false|[nN]*) usemymalloc='y' ;;
    esac

# malloc wrap works
case "$usemallocwrap" in
    '') usemallocwrap='define' ;;
    esac

# ctime_r () and asctime_r () seem to have issues for versions before
# HP-UX 11
if [ $xxOsRevMajor -lt 11 ]; then
    d_ctime_r="$undef"
    d_asctime_r="$undef"
    fi

# fpclassify () is a macro, the library call is Fpclassify
# Similarly with the others below.
d_fpclassify='define'
d_isnan='define'
d_isinf='define'
d_isfinite='define'
d_unordered='define'
# Next one(s) need the leading tab.  These are special 'hint' symbols that
# are not to be propagated to config.sh, all related to pthreads draft 4
# interfaces.
case "$d_oldpthreads" in
    ''|$undef)
	d_crypt_r_proto='undef'
	d_getgrent_r_proto='undef'
	d_getpwent_r_proto='undef'
	d_strerror_r_proto='undef'
	;;
    esac

# H.Merijn says it's not 1998 anymore: ODBM is not needed,
# and it seems to be buggy in HP-UX anyway.
i_dbm=undef

if [ "$xxOsRevMajor" -lt 11 ] || [ "$xxOsRevMajor" -eq 11 ] && [ "$xxOsRevMinor" -lt 23 ]; then
    # In HP-UXes prior to 11.23 strtold() returned a HP-UX
    # specific union called long_double, not a C99 long double.
    case "`grep 'double strtold.const' /usr/include/stdlib.h`" in
        *"long double strtold"*) ;; # strtold should be safe.
        *) echo "Looks like your strtold() is non-standard..." >&4
        d_strtold=undef ;;
    esac
fi

# In pre-11 HP-UXes there really isn't isfinite(), despite what
# Configure might think. (There is finite(), though.)
case "`grep 'isfinite' /usr/include/math.h`" in
*"isfinite"*) ;;
*) d_isfinite=undef ;;
esac

# 11.23 says it has mbrlen and mbrtowc, but compiling them fails as it can't
# find the type definition for mbstate_t which one of the parameters is.  It's
# not in the hdr the man page says it is.  Perhaps a better Configure probe is
# needed, but for now simply undefine them
d_mbrlen='undef'
d_mbrtowc='undef'
# And this one is not know on 11.11 (with HP C-ANSI-C)
if [ "$xxOsRevMajor" -lt 11 ] || [ "$xxOsRevMinor" -lt 12 ]; then
d_wcrtomb='undef'
fi
