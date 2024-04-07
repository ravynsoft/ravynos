# irix_5.sh
# Tue Jan  9 16:04:38 EST 1996
#  Add note about socket patch.
#
# Tue Jan  2 14:52:36 EST 1996
# Apparently, there's a stdio bug that can lead to memory
# corruption using perl's malloc, but not SGI's malloc.
usemymalloc='n'

ld=ld
i_time='define'
i_inttypes='undef'

case "$cc" in
*gcc*) ccflags="$ccflags -D_BSD_TYPES" ;;
*)
   # The warnings turned off are:
   # 608: Undefined the ANSI standard library defined macro stderr (nostdio.h)
   # 658: bit-field 'th_off' type required to be int, unsigned int, or signed int. <netinet/tcp.h>
   # 734: enum declaration must contain enum literals <sys/vnode.h>
   # 799: 'long long' is not standard ANSI.
   ccflags="$ccflags -D_POSIX_SOURCE -ansiposix -D_BSD_TYPES -Olimit 4300 -woff 608,658,734,799"
# Without this the cc thinks that a struct timeval * is not equivalent to
# a struct timeval *.  Yeah, you read that right.
pp_sys_cflags='ccflags="$ccflags -DPERL_IRIX5_SELECT_TIMEVAL_VOID_CAST"'
   ;;
esac

lddlflags="-shared"
# For some reason we don't want -lsocket -lnsl or -ldl.  Can anyone
# contribute an explanation?
set `echo X "$libswanted "|sed -e 's/ socket / /' -e 's/ nsl / /' -e 's/ dl / /'`
shift
libswanted="$*"

# IRIX 5.x does not have -woff for ld.
# Don't groan about unused libraries.
# case "$ldflags" in
#     *-Wl,-woff,84*) ;;
#     *) ldflags="$ldflags -Wl,-woff,84" ;;
# esac

# Date: Fri, 22 Dec 1995 11:49:17 -0800
# From: Matthew Black <black@csulb.edu>
# Subject: sockets broken under IRIX 5.3? YES...how to fix
# Anyone attempting to use perl4 or perl5 with SGI IRIX 5.3 may discover
# that sockets are essentially broken.  The syslog interface for perl also
# fails because it uses the broken socket interface.  This problem was
# reported to SGI as bug #255347 and it can be fixed by installing 
# patchSG0000596.  The patch can be downloaded from Advantage OnLine (SGI's
# WWW server) or from the Support Advantage 9/95 Patch CDROM.  Thanks to Tom 
# Christiansen and others who provided assistance.

case "$usethreads" in
$define|true|[yY]*)
        cat >&4 <<EOM
IRIX `uname -r` does not support POSIX threads.
You should upgrade to at least IRIX 6.2 with pthread patches.
EOM
	exit 1
	;;
esac

case " $use64bits $use64bitint $use64bitall " in
*" $define "*|*" true "*|*" [yY] "*)
	cat >&4 <<EOM
IRIX `uname -r` does not support 64-bit types.
You should upgrade to at least IRIX 6.2.
Cannot continue, aborting.
EOM
	exit 1
esac

