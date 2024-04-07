#
# Hints for the Cray XT4 Catamount/Qk system:
# cross-compilation host is a SuSE x86_64-linux,
# execution at the target with the 'yod' utility,
# linux.sh will run this hints file when necessary.
#
# cc.sh: compiles the code with the cross-compiler, patches main/exit/_exit
# (and traps signals) to be wrappers that echo the exit code.
#
# run.sh: runs the executable with yod and collects the exit status,
# and exits with that status.
#
# You probably should do the compilation in non-Lustre filesystem
# because Lustre does not support all the POSIX system calls, which may
# cause weird errors during the Perl build:
# 1182003549.604836:3-24:(super.c:1516:llu_iop_fcntl()): unsupported fcntl cmd 2
#
# As of 2007-Sep (pre-5.10) miniperl, libperl.a, and perl can be successfully
# built; no extensions are built.  It would be hard since Perl cannot run
# anything external (pipes, system(), backticks or fork/exec, or globbing)
# (which breaks MakeMaker, and confuses ext/util/make_ext).
#
# To build:
#
#   sh Configure -des
#   make perl
#
# "make install" won't work since it assumes file globbing (see above).
# You can try the following manual way:
#
# mkdir -p /opt/perl-catamount
# mkdir -p /opt/perl-catamount/include
# mkdir -p /opt/perl-catamount/lib
# mkdir -p /opt/perl-catamount/lib/perl5/5.38.2
# mkdir -p /opt/perl-catamount/bin
# cp *.h /opt/perl-catamount/include
# cp libperl.a /opt/perl-catamount/lib
# cp -pr lib/* /opt/perl-catamount/lib/perl5/5.38.2
# cp miniperl perl run.sh cc.sh /opt/perl-catamount/lib
#
# With the headers and the libperl.a you can embed Perl to your Catamount
# application, see pod/perlembed.pod.  You can do for example:
#
# cc -I/opt/perl-catamount/include -L/opt/perl-catamount/lib -o embed embed.c
# yod -sz 1 ./embed -le 'print sqrt(2)'
#
# You might want to have the run.sh execution wrapper around (it gets created
# in the Perl build directory) if you want to run the miniperl or perl in
# the XT4.  It collects the exit status (note that yod is run with "-sz 1",
# so only one instance is run), and possible crash status (bare yod does
# not collect the exit status).  For example:
#
#   sh /opt/perl-catamount/bin/run.sh /opt/perl-catamount/bin/perl -le 'print 42'
# 
# or if you are still in the build directory:
#
#   sh run.sh ./perl -le 'print 2*3*7'
#
# The cc.sh is a wrapper for the Catamount cc used when building Perl
# (and before that, when running Configure), it arranges for the main()
# exit(), _exit() to be wrapped so that the exit/crash status can be
# collected (by run.sh).
# 

case "$prefix" in
'') prefix=/opt/perl-catamount ;;
esac
cat >&4 <<__EOF1__
***
*** You seem to be compiling in Linux for the Catamount/Qk environment.
*** I'm therefore not going to install perl as /usr/bin/perl.
*** Perl will be installed under $prefix.
***
__EOF1__

archname='x86_64-catamount'
archobjs='catalib.o'
d_mmap='undef'
d_setlocale='undef' # There is setlocale() but no locales.
hintfile='catamount'
i_arpainet='undef'
i_db='undef'
i_netdb='undef'
i_niin='undef'
incpth=' '
installusrbinperl='undef'
libswanted="m crypt c"
libpth=' '
locincpth=' '
nonxs_ext=' '
osname='catamount'
procselfexe='undef'
static_ext=' '
usedl='undef'
useithreads='undef'
uselargefiles='define'
usenm='undef'
usethreads='undef'
use64bitall='define'

BUILD=$PWD

case "`yod -Version 2>&1`" in
Red*) ;; # E.g. "Red Storm Protocol Release 2.1.0"
*) echo >&4 "Could not find 'yod', aborting."
   exit 1 ;;
esac
run=$BUILD/run.sh
cat > $run <<'__EOF2__'
#!/bin/sh
#
# $run
#
yod -sz 1 "$@" 2> .yod$$e > .yod$$o
status=`awk '/^cata: exe .* pid [0-9][0-9]* (main|exit|_exit) [0-9][0-9]*$/ {print $NF}' .yod$$o|tail -1`
grep -v "sz is 1" .yod$$e
grep -v "^cata: exe .* pid [0-9][0-9]* " .yod$$o
grep "^cata: exe .* signal " .yod$$o
rm -f .yod$$o .yod$$e
exit $status
__EOF2__
chmod 755 $run
case "`cc -V 2>&1`" in
*catamount*) ;; # E.g. "/opt/xt-pe/1.5.41/bin/snos64/cc: INFO: catamount target is being used"
*) echo "Could not find 'cc' for catamount, aborting."
   exit 1 ;;
esac

cc=$BUILD/cc.sh
cat > $cc <<__EOF3a__
#!/bin/sh
#
# $0
#
# This is essentially a frontend driver for the Catamount cc.
# We arrange for
# (1) the main(), exit(), _exit() being wrapped (cpp-defined)
#     catamain(), cataexit(), and _cataexit()
# (2) the actual main() etc. are in cata.c, and cata*.o are
#     linked in when needed
# (3) signals being caught
# All this mostly for being able to catch the exit status (or crash cause).
#
argv=''
srco=''
srct=''
exe=''
defs='-Dmain=catamain -Dexit=cataexit -D_exit=_cataexit'
argv=''
BUILD=$BUILD
__EOF3a__
cat >> $cc <<'__EOF3b__'
case "$1" in
--cata_o) ;;
*) if test ! -f catalib.o
   then
     if test ! -f catalib.c
     then
       if test -f ../catalib.c # If compiling in UU during Configure.
       then
         cp ../catalib.c catalib.c
         cp ../catamain.c catamain.c
         cp ../cata.h cata.h
       fi
     fi
     $0 --cata_o -c catalib.c || exit 1
     $0 --cata_o -c catamain.c || exit 1
   fi
   ;;
esac
while test $# -ne 0
do
  i=$1
  shift
  case "$i" in
  --cata_o) ;;
  *.c)
    argv="$argv $defs"
    defs=""
    if test ! -f $i
    then
      echo "$0: $i: No such file or directory"
      exit 1
    fi
    j=$i$$.c
    rm -f $j
    if grep -q -s '#include "cata.h"' $i
    then
      :
    else
      cat >>$j<<__EOF4__
#include "cata.h"
# 1 "$i"
__EOF4__
    fi
    cat $i >>$j
    if grep -q -s 'int main()' $i
    then
      argv="$argv -Dmain0"
    else
      if grep -q -s 'int main([^,]*,[^,]*)' $i
      then
        argv="$argv -Dmain2"
      else
        if grep -q -s 'int main([^,]*,[^,]*,[^,]*)' $i
        then
          argv="$argv -Dmain3"
        fi
      fi
    fi
    argv="$argv $j"
    srct="$j"
    srco="$i"
    ;;
  *.o)
    if test ! -f "$i"
    then
      c=$(echo $i|sed 's/\.o$/.c/')
      $0 -c $c || exit 1
    fi
    argv="$argv $i"
    ;;
  -o)
    exe="$1"
    argv="$argv -o $exe -Dargv0=$exe"
    shift
    ;;
  *)
    argv="$argv $i"
    ;;
  esac
done
case "$exe" in
'') ;;
*) case "$argv" in
   *catalib.o*|*" perlmain.o "*) ;;
   *) argv="$argv catalib.o" ;;
   esac
   case "$argv" in
   *catamain.o*) ;;
   *) argv="$argv catamain.o" ;;
   esac
   ;;
esac
cc -I$BUILD $argv 2> .cc$$e > .cc$$o
status=$?
egrep -v 'catamount target|'$$'\.c:$' .cc$$e 1>&2
case "`grep "is not implemented" .cc$$e`" in
*"will always fail"*) status=1 ;;
esac
cat .cc$$o
rm -f .cc$$o
case "$status" in
0) rm -f .cc$$e $srct
   ;;
esac
objt=`echo $srct|sed -e 's/\.c$/.o/'`
objo=`echo $srco|sed -e 's/\.c$/.o/'`
if test -n "$objt" -a -f "$objt"
then
  mv -f $objt $objo
fi
exit $status
__EOF3b__
chmod 755 $cc

cat >cata.h<<__EOF6__
#ifndef CATA_H
#define CATA_H
void cataexit(int status);
void _cataexit(int status);
void catasigsetup();
void catasighandle(int signum);
#ifdef main0
int catamain();
#else
#ifdef main2
int main(int argc, char **argv);
#else
int main(int argc, char **argv, char **env);
#endif
#endif
#endif
#ifdef argv0
#define ARGV0 STRINGIFY(argv0)
#else
#define ARGV0 argv0
#endif
__EOF6__

cat >catalib.c<<__EOF7__
#include <stdio.h>
#include <signal.h>
#undef printf
#undef main
#undef exit
#undef _exit
#include "cata.h"
char* argv0;
void cataexit(int status) {
  printf("cata: exe %s pid %d exit %d\n", ARGV0, getpid(), status);
  exit(status);
}
void _cataexit(int status) {
  printf("cata: exe %s pid %d _exit %d\n", ARGV0, getpid(), status);
  _exit(status);
}
void catasighandle(int signum) {
  int core = 0;
  printf("cata: exe %s pid %d signal %d\n", ARGV0, getpid(), signum);
  switch (signum) {
  case SIGQUIT:
  case SIGILL:
  case SIGTRAP:
  case SIGABRT:
  case SIGBUS:
  case SIGSEGV:
  case SIGXCPU:
  case SIGXFSZ:
    core = 0200;
    break;
  default:
    break;
  }
  cataexit(core << 8 | signum);
}
void catasigsetup() {
  signal(SIGHUP, catasighandle);
  signal(SIGINT, catasighandle);
  signal(SIGQUIT, catasighandle);
  signal(SIGILL, catasighandle);
  signal(SIGTRAP, catasighandle);
  signal(SIGABRT, catasighandle);
  signal(SIGIOT, catasighandle);
  /* KILL */
  signal(SIGBUS, catasighandle);
  signal(SIGFPE, catasighandle);
  signal(SIGUSR1, catasighandle);
  signal(SIGUSR2, catasighandle);
  signal(SIGSEGV, catasighandle);
  signal(SIGPIPE, catasighandle);
  signal(SIGALRM, catasighandle);
  signal(SIGTERM, catasighandle);
  signal(SIGSTKFLT, catasighandle);
  signal(SIGCHLD, catasighandle);
  signal(SIGCONT, catasighandle);
  /* STOP */
  signal(SIGTSTP, catasighandle);
  signal(SIGTTIN, catasighandle);
  signal(SIGTTOU, catasighandle);
  signal(SIGURG, catasighandle);
  signal(SIGXCPU, catasighandle);
  signal(SIGXFSZ, catasighandle);
  signal(SIGVTALRM, catasighandle);
  signal(SIGPROF, catasighandle);
  signal(SIGWINCH, catasighandle);
  signal(SIGIO, catasighandle);
  signal(SIGPWR, catasighandle);
  signal(SIGSYS, catasighandle);
}
void boot_DynaLoader (void* cv) { }
__EOF7__
cat >catamain.c<<__EOF8__
#include <stdio.h>
#undef printf
#undef main
#undef exit
#undef _exit
#include "cata.h"
extern char* argv0;
int main(int argc, char *argv[], char *envv[]) {
  int status;
#ifndef argv0
  argv0 = argv[0];
#endif
  catasigsetup();
  status =
#ifdef main0
    catamain();
#else
#ifdef main2
    catamain(argc, argv);
#else
    catamain(argc, argv, envv);
#endif
#endif
  printf("cata: exe %s pid %d main %d\n", ARGV0, getpid(), status);
  return status;
}
__EOF8__

echo "Faking DynaLoader"
touch DynaLoader.o # Oh, the agony.

# That's it.
