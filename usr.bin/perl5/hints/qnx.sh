#----------------------------------------------------------------
# QNX hints
#
# Most of the hints in this file are for QNX4, which needed
# more help. The QNX6 hints are located toward the bottom.
#
# perl-5.7.3 passes all tests under QNX4.24G
#  Watcom 10.6 with Beta/970211.wcc.update.tar.F
#  socket3r.lib Nov21 1996.
# perl-5.7.3 fails 2 known tests under QNX6.1.0
#
# perl-5.10.0-tobe compiles with Watcom C 10.6
#                and QNX 4.25 patch G w/TCPSDK installed
#  Some tests still fail, mostly to do with dynamic/static
#  or unsupported features in QNX.
# 
## As with many unix ports, this one depends on a few "standard"
# unix utilities which are not necessarily standard for QNX4.
#
# /bin/sh  This is used heavily by Configure and then by
#          perl itself. QNX4's version is fine, but Configure
#          will choke on the 16-bit version, so if you are
#          running QNX 4.22, link /bin/sh to /bin32/ksh
# ar       This is the standard unix library builder.
#          We use wlib. With Watcom 10.6, when wlib is
#          linked as "ar", it behaves like ar and all is
#          fine. Under 9.5, a cover is required. One is
#          included in ../qnx
# nm       This is used (optionally) by configure to list
#          the contents of libraries. I will generate
#          a cover function on the fly in the UU directory.
# cpp      Configure and perl need a way to invoke a C
#          preprocessor. I have created a simple cover
#          for cc which does the right thing. Without this,
#          Configure will create its own wrapper which works,
#          but it doesn't handle some of the command line arguments
#          that perl will throw at it.
# make     You really need GNU make to compile this. GNU make
#          ships by default with QNX 4.23, but you can get it
#          from quics for earlier versions.
#----------------------------------------------------------------
# Outstanding Issues for QNX4:
#   There is no support for dynamically linked libraries in
#   QNX4.
#
#   If you wish to compile with the Socket extension, you need
#   to have the TCP/IP toolkit, and you need to make sure that
#   -lsocket locates the correct copy of socket3r.lib. Beware
#   that the Watcom compiler ships with a stub version of
#   socket3r.lib which has very little functionality. Also
#   beware the order in which wlink searches directories for
#   libraries. You may have /usr/lib/socket3r.lib pointing to
#   the correct library, but wlink may pick up
#   /usr/watcom/10.6/usr/lib/socket3r.lib instead. Make sure
#   they both point to the correct library, that is,
#   /usr/tcptk/current/usr/lib/socket3r.lib.
# 
#   dist/Cwd/Cwd.t will complain if `pwd` and cwd don't give
#   the same results. cwd calls `fullpath -t`, so if you
#   cd `fullpath -t` before running the test, it will
#   pass.
#
#   lib/File/Find/taint.t will complain if '.' is in your
#   PATH. The PATH test is triggered because cwd calls
#   `fullpath -t`.
#
#   ext/IO/lib/IO/t/io_sock.t: Subtest 14 is skipped due to
#   the fact that the functionality to read back the non-blocking
#   status of a socket is not implemented in QNX's TCP/IP. This
#   has been reported to QNX and it may work with later versions
#   of TCP/IP.
#
# Older issues:
#   lib/posix.t test failed on test 17 because acos(1) != 0.
#      Resolved in 970211 Beta
#   lib/io_udp.t test hangs because of a bug in getsockname().
#      Fixed in latest BETA socket3r.lib
#----------------------------------------------------------------
# Outstanding Issues for QNX6:
#  The following tests are still failing as of 5.7.3:
#
#   op/sprintf.........................FAILED at test 91
#   lib/Benchmark......................FAILED at test 26
#
# This is due to a bug in the C library's printf routine.
# printf("'%e'", 0. ) produces '0.000000e+0', but ANSI requires
# '0.000000e+00'. QNX has acknowledged the bug and it should be
# fixed in 6.2.0.
#
#----------------------------------------------------------------
# These hints were submitted by:
#   Norton T. Allen
#   Harvard University Atmospheric Research Project
#   allen@huarp.harvard.edu
#
# If you have suggestions or changes, please let me know.
#----------------------------------------------------------------

echo ""
echo "Some tests may fail. Please read the hints/qnx.sh file."
echo ""

#----------------------------------------------------------------
# At present, all QNX4 systems are equivalent architectures,
# so it is reasonable to call archname=x86-qnx rather than
# making an unnecessary distinction between AT-qnx and PCI-qnx,
# for example. I will use uname's architecture for Neutrino.
#----------------------------------------------------------------
set X `$run uname -a`
shift
[ "$1" != "QNX" ] && echo "uname doesn't look like QNX!"
case $4 in
  42[2-9]) archname='x86-qnx';;
  *) osname='nto'
	 osvers=$3
     archname="$5-nto";;
esac

if [ "$osname" = "qnx" ]; then
  #----------------------------------------------------------------
  # QNX doesn't come with a csh and the ports of tcsh I've used
  # don't work reliably:
  #----------------------------------------------------------------
  csh=''
  d_csh='undef'
  full_csh=''

  #----------------------------------------------------------------
  # setuid scripts are secure under QNX.
  #  (Basically, the same race conditions apply, but assuming
  #  the scripts are located in a secure directory, the methods
  #  for exploiting the race condition are defeated because
  #  the loader expands the script name fully before executing
  #  the interpreter.)
  #----------------------------------------------------------------
  d_suidsafe='define'

  #----------------------------------------------------------------
  # difftime is implemented as a preprocessor macro, so it doesn't show
  # up in the libraries:
  #----------------------------------------------------------------
  d_difftime='define'

  #----------------------------------------------------------------
  # strtod is in the math library, but we can't tell Configure
  # about the math library or it will confuse the linker
  #----------------------------------------------------------------
  d_strtod='define'

  lib_ext='3r.lib'
  libc='/usr/lib/clib3r.lib'

  #----------------------------------------------------------------
  # ccflags:
  # I like to turn the warnings up high, but a few common
  # constructs make a lot of noise, so I turn those warnings off.
  # A few still remain...
  #
  # unix.h is required as a general rule for unixy applications.
  #----------------------------------------------------------------
  ccflags='-mf -w4 -Wc,-wcd=202 -Wc,-wcd=203 -Wc,-wcd=302 -Wc,-fi=unix.h'

  #----------------------------------------------------------------
  # ldflags:
  # If you want debugging information, you must specify -g on the
  # link as well as the compile. If optimize != -g, you should
  # remove this.
  #----------------------------------------------------------------
  ldflags="-g -N1M"

  so='none'
  selecttype='fd_set *'

  #----------------------------------------------------------------
  # Add -lunix to list of libs. This is needed mainly so the nm
  # search will find funcs in the unix lib. Including unix.h should
  # automatically include the library without -l.
  #----------------------------------------------------------------
  libswanted="$libswanted unix"

  if [ -z "`which ar 2>/dev/null`" ]; then
	cat <<-'EOF' >&4
	  I don't see an 'ar', so I'm guessing you are running
	  Watcom 9.5 or earlier. You may want to install the ar
	  cover found in the qnx subdirectory of this distribution.
	  It might reasonably be placed in /usr/local/bin.

	EOF
  fi
  #----------------------------------------------------------------
  # Here is a nm script which fixes up wlib's output to look
  # something like nm's, at least enough so that Configure can
  # use it.
  #----------------------------------------------------------------
  if [ -z "`which nm 2>/dev/null`" ]; then
	cat <<-EOF
	  Creating a quick-and-dirty nm cover for	Configure to use:

	EOF
	cat >./UU/nm <<-'EOF'
	  #! /bin/sh
	  #__USAGE
	  #%C	<lib> [<lib> ...]
	  #	Designed to mimic Unix's nm utility to list
	  #	defined symbols in a library
	  unset WLIB
	  for i in $*; do wlib $i; done |
		awk '
		  /^  / {
			for (i = 1; i <= NF; i++) {
			  sub("_$", "", $i)
			  print "000000  T " $i
			}
		  }'
	EOF
	chmod +x ./UU/nm
  fi

  cppstdin=`which cpp 2>/dev/null`
  if [ -n "$cppstdin" ]; then
	cat <<-EOF >&4
	  I found a cpp at $cppstdin and will assume it is a good
	  thing to use. If this proves to be false, there is a
	  thin cover for cpp in the qnx subdirectory of this
	  distribution which you could move into your path.
	EOF
	cpprun="$cppstdin"
  else
	cat <<-EOF >&4
	
	  There is a cpp cover in the qnx subdirectory of this
	  distribution which works a little better than the
	  Configure default. You may wish to copy it to
	  /usr/local/bin or some other suitable location.
	EOF
  fi

  # includes a matherr() to silence noise from watcom libc
  archobjs="qnx.o"
  test -f qnx.c || cp qnx/qnx.c .

else
  # $^O eq nto

  ccflags="$ccflags -U__STRICT_ANSI__"

  # Options required to get dynamic linking to work
  lddlflags="$lddlflags -shared"
  ccdlflags="$ccdlflags -Wl,-E"

  case "$usecrosscompile" in
  define)
    # TODO this else should probably be an elif on $2 including
    # "Blackberry" or similar
    
    # Configure detects these, but they won't link for some reason:
    d_eaccess="$undef"
    d_dirfd="$undef"
    d_getspnam="$undef"
    d_setlinebuf="$undef"
    
    # Default to USE_SHELL_ALWAYS -- like with Android, Blackberry's
    # shell has several commands built-in, and this works around the
    # issue.
    d_useshellcmds='define'
    
    # By this point, targetarch will be:
    # For x86 builds, 'ntox86' or 'i486-pc-nto-qnx8.0.0-gcc'
    # For arm builds, 'ntoarmv7' 'arm-unknown-nto-qnx8.0.0eabi'
    # This will change those to plain x86 and armle-v7, respectively.
    case "`$run uname -m`" in
        *x86*) targetarch=x86;;
        *) targetarch=armle-v7;;
    esac
    
    libc="$QNX_TARGET/$targetarch/lib/libc.so"
    
    ldflags="$ldflags -L${QNX_TARGET}/$targetarch/lib -L${QNX_TARGET}/$targetarch/usr/lib -L${QNX_TARGET}/$targetarch/lib/gcc/4.6.3"
    
    ldflags="$ldflags -lc -lm -lsocket"
    lddlflags="$lddlflags -lc -lm -lsocket "    
    libpth="$libpth /proc/boot"
    targetenv="export LC_ALL=C; $targetenv"
    ;;
  *)
    # Somewhere in the build, something tries to throw a gcc
    # option to $cc if it knows it invokes gcc. Our cc doesn't
    # recognize that option, so we're better off setting cc=gcc.
    # Of course, only do this when not cross-compiling, for
    # obvious reasons.
    cc='gcc'
    ;;
  esac

  # gcc uses $QNX_TARGET/usr/include as the include directory.
  usrinc="$QNX_TARGET/usr/include"

  # If we use perl's malloc, it dies with an invalid sbrk.
  # This is probably worth tracking down someday.
  usemymalloc='false'
  
  libswanted=`echo " $libswanted "| sed 's/ malloc / /'`

  # Some routines are only in our static libc.
  # eg crypt() getlogin() getlogin_r()
  usenm=false
fi
