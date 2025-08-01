/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically @generated.
 */

#include <sys/param.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <compat/freebsd32/freebsd32_proto.h>
#include <vm/vm.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/message.h>
#include <sys/mach/mach_time.h>
#include <sys/mach/port.h>
#include <sys/mach/clock_server.h>

#define AS(name) (sizeof(struct name) / sizeof(syscallarg_t))

#ifdef COMPAT_43
#define compat(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(o, name)
#else
#define compat(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD4
#define compat4(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd4_, name)
#else
#define compat4(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD6
#define compat6(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd6_, name)
#else
#define compat6(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD7
#define compat7(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd7_, name)
#else
#define compat7(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD10
#define compat10(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd10_, name)
#else
#define compat10(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD11
#define compat11(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd11_, name)
#else
#define compat11(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD12
#define compat12(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd12_, name)
#else
#define compat12(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD13
#define compat13(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd13_, name)
#else
#define compat13(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

#ifdef COMPAT_FREEBSD14
#define compat14(n, name) .sy_narg = n, .sy_call = (sy_call_t *)__CONCAT(freebsd14_, name)
#else
#define compat14(n, name) .sy_narg = 0, .sy_call = (sy_call_t *)nosys
#endif

/* The casts are bogus but will do for now. */
struct sysent freebsd32_sysent[] = {
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 0 = syscall */
	{ .sy_narg = AS(exit_args), .sy_call = (sy_call_t *)sys_exit, .sy_auevent = AUE_EXIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 1 = exit */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_fork, .sy_auevent = AUE_FORK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 2 = fork */
	{ .sy_narg = AS(read_args), .sy_call = (sy_call_t *)sys_read, .sy_auevent = AUE_READ, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 3 = read */
	{ .sy_narg = AS(write_args), .sy_call = (sy_call_t *)sys_write, .sy_auevent = AUE_WRITE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 4 = write */
	{ .sy_narg = AS(open_args), .sy_call = (sy_call_t *)sys_open, .sy_auevent = AUE_OPEN_RWTC, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 5 = open */
	{ .sy_narg = AS(close_args), .sy_call = (sy_call_t *)sys_close, .sy_auevent = AUE_CLOSE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 6 = close */
	{ .sy_narg = AS(freebsd32_wait4_args), .sy_call = (sy_call_t *)freebsd32_wait4, .sy_auevent = AUE_WAIT4, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 7 = freebsd32_wait4 */
	{ compat(AS(ocreat_args),creat), .sy_auevent = AUE_CREAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 8 = old creat */
	{ .sy_narg = AS(link_args), .sy_call = (sy_call_t *)sys_link, .sy_auevent = AUE_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 9 = link */
	{ .sy_narg = AS(unlink_args), .sy_call = (sy_call_t *)sys_unlink, .sy_auevent = AUE_UNLINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 10 = unlink */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 11 = obsolete execv */
	{ .sy_narg = AS(chdir_args), .sy_call = (sy_call_t *)sys_chdir, .sy_auevent = AUE_CHDIR, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 12 = chdir */
	{ .sy_narg = AS(fchdir_args), .sy_call = (sy_call_t *)sys_fchdir, .sy_auevent = AUE_FCHDIR, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 13 = fchdir */
	{ compat11(AS(freebsd11_mknod_args),mknod), .sy_auevent = AUE_MKNOD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 14 = freebsd11 mknod */
	{ .sy_narg = AS(chmod_args), .sy_call = (sy_call_t *)sys_chmod, .sy_auevent = AUE_CHMOD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 15 = chmod */
	{ .sy_narg = AS(chown_args), .sy_call = (sy_call_t *)sys_chown, .sy_auevent = AUE_CHOWN, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 16 = chown */
	{ .sy_narg = AS(break_args), .sy_call = (sy_call_t *)sys_break, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 17 = break */
	{ compat4(AS(freebsd4_freebsd32_getfsstat_args),freebsd32_getfsstat), .sy_auevent = AUE_GETFSSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 18 = freebsd4 freebsd32_getfsstat */
	{ compat(AS(ofreebsd32_lseek_args),freebsd32_lseek), .sy_auevent = AUE_LSEEK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 19 = old freebsd32_lseek */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getpid, .sy_auevent = AUE_GETPID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 20 = getpid */
	{ .sy_narg = AS(mount_args), .sy_call = (sy_call_t *)sys_mount, .sy_auevent = AUE_MOUNT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 21 = mount */
	{ .sy_narg = AS(unmount_args), .sy_call = (sy_call_t *)sys_unmount, .sy_auevent = AUE_UMOUNT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 22 = unmount */
	{ .sy_narg = AS(setuid_args), .sy_call = (sy_call_t *)sys_setuid, .sy_auevent = AUE_SETUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 23 = setuid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getuid, .sy_auevent = AUE_GETUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 24 = getuid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_geteuid, .sy_auevent = AUE_GETEUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 25 = geteuid */
	{ .sy_narg = AS(freebsd32_ptrace_args), .sy_call = (sy_call_t *)freebsd32_ptrace, .sy_auevent = AUE_PTRACE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 26 = freebsd32_ptrace */
	{ .sy_narg = AS(freebsd32_recvmsg_args), .sy_call = (sy_call_t *)freebsd32_recvmsg, .sy_auevent = AUE_RECVMSG, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 27 = freebsd32_recvmsg */
	{ .sy_narg = AS(freebsd32_sendmsg_args), .sy_call = (sy_call_t *)freebsd32_sendmsg, .sy_auevent = AUE_SENDMSG, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 28 = freebsd32_sendmsg */
	{ .sy_narg = AS(recvfrom_args), .sy_call = (sy_call_t *)sys_recvfrom, .sy_auevent = AUE_RECVFROM, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 29 = recvfrom */
	{ .sy_narg = AS(accept_args), .sy_call = (sy_call_t *)sys_accept, .sy_auevent = AUE_ACCEPT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 30 = accept */
	{ .sy_narg = AS(getpeername_args), .sy_call = (sy_call_t *)sys_getpeername, .sy_auevent = AUE_GETPEERNAME, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 31 = getpeername */
	{ .sy_narg = AS(getsockname_args), .sy_call = (sy_call_t *)sys_getsockname, .sy_auevent = AUE_GETSOCKNAME, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 32 = getsockname */
	{ .sy_narg = AS(access_args), .sy_call = (sy_call_t *)sys_access, .sy_auevent = AUE_ACCESS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 33 = access */
	{ .sy_narg = AS(chflags_args), .sy_call = (sy_call_t *)sys_chflags, .sy_auevent = AUE_CHFLAGS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 34 = chflags */
	{ .sy_narg = AS(fchflags_args), .sy_call = (sy_call_t *)sys_fchflags, .sy_auevent = AUE_FCHFLAGS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 35 = fchflags */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_sync, .sy_auevent = AUE_SYNC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 36 = sync */
	{ .sy_narg = AS(kill_args), .sy_call = (sy_call_t *)sys_kill, .sy_auevent = AUE_KILL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 37 = kill */
	{ compat(AS(ofreebsd32_stat_args),freebsd32_stat), .sy_auevent = AUE_STAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 38 = old freebsd32_stat */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getppid, .sy_auevent = AUE_GETPPID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 39 = getppid */
	{ compat(AS(ofreebsd32_lstat_args),freebsd32_lstat), .sy_auevent = AUE_LSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 40 = old freebsd32_lstat */
	{ .sy_narg = AS(dup_args), .sy_call = (sy_call_t *)sys_dup, .sy_auevent = AUE_DUP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 41 = dup */
	{ compat10(0,pipe), .sy_auevent = AUE_PIPE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 42 = freebsd10 pipe */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getegid, .sy_auevent = AUE_GETEGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 43 = getegid */
	{ .sy_narg = AS(profil_args), .sy_call = (sy_call_t *)sys_profil, .sy_auevent = AUE_PROFILE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 44 = profil */
	{ .sy_narg = AS(ktrace_args), .sy_call = (sy_call_t *)sys_ktrace, .sy_auevent = AUE_KTRACE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 45 = ktrace */
	{ compat(AS(ofreebsd32_sigaction_args),freebsd32_sigaction), .sy_auevent = AUE_SIGACTION, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 46 = old freebsd32_sigaction */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getgid, .sy_auevent = AUE_GETGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 47 = getgid */
	{ compat(AS(osigprocmask_args),sigprocmask), .sy_auevent = AUE_SIGPROCMASK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 48 = old sigprocmask */
	{ .sy_narg = AS(getlogin_args), .sy_call = (sy_call_t *)sys_getlogin, .sy_auevent = AUE_GETLOGIN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 49 = getlogin */
	{ .sy_narg = AS(setlogin_args), .sy_call = (sy_call_t *)sys_setlogin, .sy_auevent = AUE_SETLOGIN, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 50 = setlogin */
	{ .sy_narg = AS(acct_args), .sy_call = (sy_call_t *)sys_acct, .sy_auevent = AUE_ACCT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 51 = acct */
	{ compat(0,sigpending), .sy_auevent = AUE_SIGPENDING, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 52 = old sigpending */
	{ .sy_narg = AS(freebsd32_sigaltstack_args), .sy_call = (sy_call_t *)freebsd32_sigaltstack, .sy_auevent = AUE_SIGALTSTACK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 53 = freebsd32_sigaltstack */
	{ .sy_narg = AS(freebsd32_ioctl_args), .sy_call = (sy_call_t *)freebsd32_ioctl, .sy_auevent = AUE_IOCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 54 = freebsd32_ioctl */
	{ .sy_narg = AS(reboot_args), .sy_call = (sy_call_t *)sys_reboot, .sy_auevent = AUE_REBOOT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 55 = reboot */
	{ .sy_narg = AS(revoke_args), .sy_call = (sy_call_t *)sys_revoke, .sy_auevent = AUE_REVOKE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 56 = revoke */
	{ .sy_narg = AS(symlink_args), .sy_call = (sy_call_t *)sys_symlink, .sy_auevent = AUE_SYMLINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 57 = symlink */
	{ .sy_narg = AS(readlink_args), .sy_call = (sy_call_t *)sys_readlink, .sy_auevent = AUE_READLINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 58 = readlink */
	{ .sy_narg = AS(freebsd32_execve_args), .sy_call = (sy_call_t *)freebsd32_execve, .sy_auevent = AUE_EXECVE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 59 = freebsd32_execve */
	{ .sy_narg = AS(umask_args), .sy_call = (sy_call_t *)sys_umask, .sy_auevent = AUE_UMASK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 60 = umask */
	{ .sy_narg = AS(chroot_args), .sy_call = (sy_call_t *)sys_chroot, .sy_auevent = AUE_CHROOT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 61 = chroot */
	{ compat(AS(ofreebsd32_fstat_args),freebsd32_fstat), .sy_auevent = AUE_FSTAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 62 = old freebsd32_fstat */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 63 = obsolete getkerninfo */
	{ compat(0,getpagesize), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 64 = old getpagesize */
	{ .sy_narg = AS(msync_args), .sy_call = (sy_call_t *)sys_msync, .sy_auevent = AUE_MSYNC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 65 = msync */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_vfork, .sy_auevent = AUE_VFORK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 66 = vfork */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 67 = obsolete vread */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 68 = obsolete vwrite */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 69 = obsolete sbrk */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 70 = obsolete sstk */
	{ compat(AS(ofreebsd32_mmap_args),freebsd32_mmap), .sy_auevent = AUE_MMAP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 71 = old freebsd32_mmap */
	{ compat11(AS(freebsd11_vadvise_args),vadvise), .sy_auevent = AUE_O_VADVISE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 72 = freebsd11 vadvise */
	{ .sy_narg = AS(munmap_args), .sy_call = (sy_call_t *)sys_munmap, .sy_auevent = AUE_MUNMAP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 73 = munmap */
	{ .sy_narg = AS(freebsd32_mprotect_args), .sy_call = (sy_call_t *)freebsd32_mprotect, .sy_auevent = AUE_MPROTECT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 74 = freebsd32_mprotect */
	{ .sy_narg = AS(madvise_args), .sy_call = (sy_call_t *)sys_madvise, .sy_auevent = AUE_MADVISE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 75 = madvise */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 76 = obsolete vhangup */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 77 = obsolete vlimit */
	{ .sy_narg = AS(mincore_args), .sy_call = (sy_call_t *)sys_mincore, .sy_auevent = AUE_MINCORE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 78 = mincore */
	{ .sy_narg = AS(getgroups_args), .sy_call = (sy_call_t *)sys_getgroups, .sy_auevent = AUE_GETGROUPS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 79 = getgroups */
	{ .sy_narg = AS(setgroups_args), .sy_call = (sy_call_t *)sys_setgroups, .sy_auevent = AUE_SETGROUPS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 80 = setgroups */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getpgrp, .sy_auevent = AUE_GETPGRP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 81 = getpgrp */
	{ .sy_narg = AS(setpgid_args), .sy_call = (sy_call_t *)sys_setpgid, .sy_auevent = AUE_SETPGRP, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 82 = setpgid */
	{ .sy_narg = AS(freebsd32_setitimer_args), .sy_call = (sy_call_t *)freebsd32_setitimer, .sy_auevent = AUE_SETITIMER, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 83 = freebsd32_setitimer */
	{ compat(0,wait), .sy_auevent = AUE_WAIT4, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 84 = old wait */
	{ .sy_narg = AS(swapon_args), .sy_call = (sy_call_t *)sys_swapon, .sy_auevent = AUE_SWAPON, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 85 = swapon */
	{ .sy_narg = AS(freebsd32_getitimer_args), .sy_call = (sy_call_t *)freebsd32_getitimer, .sy_auevent = AUE_GETITIMER, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 86 = freebsd32_getitimer */
	{ compat(AS(ogethostname_args),gethostname), .sy_auevent = AUE_SYSCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 87 = old gethostname */
	{ compat(AS(osethostname_args),sethostname), .sy_auevent = AUE_SYSCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 88 = old sethostname */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_getdtablesize, .sy_auevent = AUE_GETDTABLESIZE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 89 = getdtablesize */
	{ .sy_narg = AS(dup2_args), .sy_call = (sy_call_t *)sys_dup2, .sy_auevent = AUE_DUP2, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 90 = dup2 */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 91 = reserved for local use */
	{ .sy_narg = AS(freebsd32_fcntl_args), .sy_call = (sy_call_t *)freebsd32_fcntl, .sy_auevent = AUE_FCNTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 92 = freebsd32_fcntl */
	{ .sy_narg = AS(freebsd32_select_args), .sy_call = (sy_call_t *)freebsd32_select, .sy_auevent = AUE_SELECT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 93 = freebsd32_select */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 94 = reserved for local use */
	{ .sy_narg = AS(fsync_args), .sy_call = (sy_call_t *)sys_fsync, .sy_auevent = AUE_FSYNC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 95 = fsync */
	{ .sy_narg = AS(setpriority_args), .sy_call = (sy_call_t *)sys_setpriority, .sy_auevent = AUE_SETPRIORITY, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 96 = setpriority */
	{ .sy_narg = AS(socket_args), .sy_call = (sy_call_t *)sys_socket, .sy_auevent = AUE_SOCKET, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 97 = socket */
	{ .sy_narg = AS(connect_args), .sy_call = (sy_call_t *)sys_connect, .sy_auevent = AUE_CONNECT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 98 = connect */
	{ compat(AS(oaccept_args),accept), .sy_auevent = AUE_ACCEPT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 99 = old accept */
	{ .sy_narg = AS(getpriority_args), .sy_call = (sy_call_t *)sys_getpriority, .sy_auevent = AUE_GETPRIORITY, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 100 = getpriority */
	{ compat(AS(osend_args),send), .sy_auevent = AUE_SEND, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 101 = old send */
	{ compat(AS(orecv_args),recv), .sy_auevent = AUE_RECV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 102 = old recv */
	{ compat(AS(ofreebsd32_sigreturn_args),freebsd32_sigreturn), .sy_auevent = AUE_SIGRETURN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 103 = old freebsd32_sigreturn */
	{ .sy_narg = AS(bind_args), .sy_call = (sy_call_t *)sys_bind, .sy_auevent = AUE_BIND, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 104 = bind */
	{ .sy_narg = AS(setsockopt_args), .sy_call = (sy_call_t *)sys_setsockopt, .sy_auevent = AUE_SETSOCKOPT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 105 = setsockopt */
	{ .sy_narg = AS(listen_args), .sy_call = (sy_call_t *)sys_listen, .sy_auevent = AUE_LISTEN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 106 = listen */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 107 = obsolete vtimes */
	{ compat(AS(ofreebsd32_sigvec_args),freebsd32_sigvec), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 108 = old freebsd32_sigvec */
	{ compat(AS(osigblock_args),sigblock), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 109 = old sigblock */
	{ compat(AS(osigsetmask_args),sigsetmask), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 110 = old sigsetmask */
	{ compat(AS(osigsuspend_args),sigsuspend), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 111 = old sigsuspend */
	{ compat(AS(ofreebsd32_sigstack_args),freebsd32_sigstack), .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 112 = old freebsd32_sigstack */
	{ compat(AS(ofreebsd32_recvmsg_args),freebsd32_recvmsg), .sy_auevent = AUE_RECVMSG, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 113 = old freebsd32_recvmsg */
	{ compat(AS(ofreebsd32_sendmsg_args),freebsd32_sendmsg), .sy_auevent = AUE_SENDMSG, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 114 = old freebsd32_sendmsg */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 115 = obsolete vtrace */
	{ .sy_narg = AS(freebsd32_gettimeofday_args), .sy_call = (sy_call_t *)freebsd32_gettimeofday, .sy_auevent = AUE_GETTIMEOFDAY, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 116 = freebsd32_gettimeofday */
	{ .sy_narg = AS(freebsd32_getrusage_args), .sy_call = (sy_call_t *)freebsd32_getrusage, .sy_auevent = AUE_GETRUSAGE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 117 = freebsd32_getrusage */
	{ .sy_narg = AS(getsockopt_args), .sy_call = (sy_call_t *)sys_getsockopt, .sy_auevent = AUE_GETSOCKOPT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 118 = getsockopt */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 119 = reserved for local use */
	{ .sy_narg = AS(freebsd32_readv_args), .sy_call = (sy_call_t *)freebsd32_readv, .sy_auevent = AUE_READV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 120 = freebsd32_readv */
	{ .sy_narg = AS(freebsd32_writev_args), .sy_call = (sy_call_t *)freebsd32_writev, .sy_auevent = AUE_WRITEV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 121 = freebsd32_writev */
	{ .sy_narg = AS(freebsd32_settimeofday_args), .sy_call = (sy_call_t *)freebsd32_settimeofday, .sy_auevent = AUE_SETTIMEOFDAY, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 122 = freebsd32_settimeofday */
	{ .sy_narg = AS(fchown_args), .sy_call = (sy_call_t *)sys_fchown, .sy_auevent = AUE_FCHOWN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 123 = fchown */
	{ .sy_narg = AS(fchmod_args), .sy_call = (sy_call_t *)sys_fchmod, .sy_auevent = AUE_FCHMOD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 124 = fchmod */
	{ compat(AS(orecvfrom_args),recvfrom), .sy_auevent = AUE_RECVFROM, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 125 = old recvfrom */
	{ .sy_narg = AS(setreuid_args), .sy_call = (sy_call_t *)sys_setreuid, .sy_auevent = AUE_SETREUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 126 = setreuid */
	{ .sy_narg = AS(setregid_args), .sy_call = (sy_call_t *)sys_setregid, .sy_auevent = AUE_SETREGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 127 = setregid */
	{ .sy_narg = AS(rename_args), .sy_call = (sy_call_t *)sys_rename, .sy_auevent = AUE_RENAME, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 128 = rename */
	{ compat(AS(ofreebsd32_truncate_args),freebsd32_truncate), .sy_auevent = AUE_TRUNCATE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 129 = old freebsd32_truncate */
	{ compat(AS(ofreebsd32_ftruncate_args),freebsd32_ftruncate), .sy_auevent = AUE_FTRUNCATE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 130 = old freebsd32_ftruncate */
	{ .sy_narg = AS(flock_args), .sy_call = (sy_call_t *)sys_flock, .sy_auevent = AUE_FLOCK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 131 = flock */
	{ .sy_narg = AS(mkfifo_args), .sy_call = (sy_call_t *)sys_mkfifo, .sy_auevent = AUE_MKFIFO, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 132 = mkfifo */
	{ .sy_narg = AS(sendto_args), .sy_call = (sy_call_t *)sys_sendto, .sy_auevent = AUE_SENDTO, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 133 = sendto */
	{ .sy_narg = AS(shutdown_args), .sy_call = (sy_call_t *)sys_shutdown, .sy_auevent = AUE_SHUTDOWN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 134 = shutdown */
	{ .sy_narg = AS(socketpair_args), .sy_call = (sy_call_t *)sys_socketpair, .sy_auevent = AUE_SOCKETPAIR, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 135 = socketpair */
	{ .sy_narg = AS(mkdir_args), .sy_call = (sy_call_t *)sys_mkdir, .sy_auevent = AUE_MKDIR, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 136 = mkdir */
	{ .sy_narg = AS(rmdir_args), .sy_call = (sy_call_t *)sys_rmdir, .sy_auevent = AUE_RMDIR, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 137 = rmdir */
	{ .sy_narg = AS(freebsd32_utimes_args), .sy_call = (sy_call_t *)freebsd32_utimes, .sy_auevent = AUE_UTIMES, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 138 = freebsd32_utimes */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 139 = obsolete freebsd32_sigreturn */
	{ .sy_narg = AS(freebsd32_adjtime_args), .sy_call = (sy_call_t *)freebsd32_adjtime, .sy_auevent = AUE_ADJTIME, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 140 = freebsd32_adjtime */
	{ compat(AS(ogetpeername_args),getpeername), .sy_auevent = AUE_GETPEERNAME, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 141 = old getpeername */
	{ compat(0,gethostid), .sy_auevent = AUE_SYSCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 142 = old gethostid */
	{ compat(AS(ofreebsd32_sethostid_args),freebsd32_sethostid), .sy_auevent = AUE_SYSCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 143 = old freebsd32_sethostid */
	{ compat(AS(ogetrlimit_args),getrlimit), .sy_auevent = AUE_GETRLIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 144 = old getrlimit */
	{ compat(AS(osetrlimit_args),setrlimit), .sy_auevent = AUE_SETRLIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 145 = old setrlimit */
	{ compat(AS(okillpg_args),killpg), .sy_auevent = AUE_KILLPG, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 146 = old killpg */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_setsid, .sy_auevent = AUE_SETSID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 147 = setsid */
	{ .sy_narg = AS(quotactl_args), .sy_call = (sy_call_t *)sys_quotactl, .sy_auevent = AUE_QUOTACTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 148 = quotactl */
	{ compat(0,quota), .sy_auevent = AUE_O_QUOTA, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 149 = old quota */
	{ compat(AS(ogetsockname_args),getsockname), .sy_auevent = AUE_GETSOCKNAME, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 150 = old getsockname */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 151 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 152 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 153 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 154 = freebsd32_nlm_syscall */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 155 = nfssvc */
	{ compat(AS(ofreebsd32_getdirentries_args),freebsd32_getdirentries), .sy_auevent = AUE_GETDIRENTRIES, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 156 = old freebsd32_getdirentries */
	{ compat4(AS(freebsd4_freebsd32_statfs_args),freebsd32_statfs), .sy_auevent = AUE_STATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 157 = freebsd4 freebsd32_statfs */
	{ compat4(AS(freebsd4_freebsd32_fstatfs_args),freebsd32_fstatfs), .sy_auevent = AUE_FSTATFS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 158 = freebsd4 freebsd32_fstatfs */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 159 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 160 = lgetfh */
	{ .sy_narg = AS(getfh_args), .sy_call = (sy_call_t *)sys_getfh, .sy_auevent = AUE_NFS_GETFH, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 161 = getfh */
	{ compat4(AS(freebsd4_getdomainname_args),getdomainname), .sy_auevent = AUE_SYSCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 162 = freebsd4 getdomainname */
	{ compat4(AS(freebsd4_setdomainname_args),setdomainname), .sy_auevent = AUE_SYSCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 163 = freebsd4 setdomainname */
	{ compat4(AS(freebsd4_uname_args),uname), .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 164 = freebsd4 uname */
	{ .sy_narg = AS(freebsd32_sysarch_args), .sy_call = (sy_call_t *)freebsd32_sysarch, .sy_auevent = AUE_SYSARCH, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 165 = freebsd32_sysarch */
	{ .sy_narg = AS(rtprio_args), .sy_call = (sy_call_t *)sys_rtprio, .sy_auevent = AUE_RTPRIO, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 166 = rtprio */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 167 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 168 = reserved for local use */
	{ .sy_narg = AS(freebsd32_semsys_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 169 = freebsd32_semsys */
	{ .sy_narg = AS(freebsd32_msgsys_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 170 = freebsd32_msgsys */
	{ .sy_narg = AS(freebsd32_shmsys_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 171 = freebsd32_shmsys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 172 = reserved for local use */
	{ compat6(AS(freebsd6_freebsd32_pread_args),freebsd32_pread), .sy_auevent = AUE_PREAD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 173 = freebsd6 freebsd32_pread */
	{ compat6(AS(freebsd6_freebsd32_pwrite_args),freebsd32_pwrite), .sy_auevent = AUE_PWRITE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 174 = freebsd6 freebsd32_pwrite */
	{ .sy_narg = AS(setfib_args), .sy_call = (sy_call_t *)sys_setfib, .sy_auevent = AUE_SETFIB, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 175 = setfib */
	{ .sy_narg = AS(freebsd32_ntp_adjtime_args), .sy_call = (sy_call_t *)freebsd32_ntp_adjtime, .sy_auevent = AUE_NTP_ADJTIME, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 176 = freebsd32_ntp_adjtime */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 177 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 178 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 179 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 180 = reserved for local use */
	{ .sy_narg = AS(setgid_args), .sy_call = (sy_call_t *)sys_setgid, .sy_auevent = AUE_SETGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 181 = setgid */
	{ .sy_narg = AS(setegid_args), .sy_call = (sy_call_t *)sys_setegid, .sy_auevent = AUE_SETEGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 182 = setegid */
	{ .sy_narg = AS(seteuid_args), .sy_call = (sy_call_t *)sys_seteuid, .sy_auevent = AUE_SETEUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 183 = seteuid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 184 = obsolete lfs_bmapv */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 185 = obsolete lfs_markv */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 186 = obsolete lfs_segclean */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 187 = obsolete lfs_segwait */
	{ compat11(AS(freebsd11_freebsd32_stat_args),freebsd32_stat), .sy_auevent = AUE_STAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 188 = freebsd11 freebsd32_stat */
	{ compat11(AS(freebsd11_freebsd32_fstat_args),freebsd32_fstat), .sy_auevent = AUE_FSTAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 189 = freebsd11 freebsd32_fstat */
	{ compat11(AS(freebsd11_freebsd32_lstat_args),freebsd32_lstat), .sy_auevent = AUE_LSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 190 = freebsd11 freebsd32_lstat */
	{ .sy_narg = AS(pathconf_args), .sy_call = (sy_call_t *)sys_pathconf, .sy_auevent = AUE_PATHCONF, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 191 = pathconf */
	{ .sy_narg = AS(fpathconf_args), .sy_call = (sy_call_t *)sys_fpathconf, .sy_auevent = AUE_FPATHCONF, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 192 = fpathconf */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 193 = reserved for local use */
	{ .sy_narg = AS(getrlimit_args), .sy_call = (sy_call_t *)sys_getrlimit, .sy_auevent = AUE_GETRLIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 194 = getrlimit */
	{ .sy_narg = AS(setrlimit_args), .sy_call = (sy_call_t *)sys_setrlimit, .sy_auevent = AUE_SETRLIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 195 = setrlimit */
	{ compat11(AS(freebsd11_freebsd32_getdirentries_args),freebsd32_getdirentries), .sy_auevent = AUE_GETDIRENTRIES, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 196 = freebsd11 freebsd32_getdirentries */
	{ compat6(AS(freebsd6_freebsd32_mmap_args),freebsd32_mmap), .sy_auevent = AUE_MMAP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 197 = freebsd6 freebsd32_mmap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 198 = __syscall */
	{ compat6(AS(freebsd6_freebsd32_lseek_args),freebsd32_lseek), .sy_auevent = AUE_LSEEK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 199 = freebsd6 freebsd32_lseek */
	{ compat6(AS(freebsd6_freebsd32_truncate_args),freebsd32_truncate), .sy_auevent = AUE_TRUNCATE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 200 = freebsd6 freebsd32_truncate */
	{ compat6(AS(freebsd6_freebsd32_ftruncate_args),freebsd32_ftruncate), .sy_auevent = AUE_FTRUNCATE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 201 = freebsd6 freebsd32_ftruncate */
	{ .sy_narg = AS(freebsd32___sysctl_args), .sy_call = (sy_call_t *)freebsd32___sysctl, .sy_auevent = AUE_SYSCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 202 = freebsd32___sysctl */
	{ .sy_narg = AS(mlock_args), .sy_call = (sy_call_t *)sys_mlock, .sy_auevent = AUE_MLOCK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 203 = mlock */
	{ .sy_narg = AS(munlock_args), .sy_call = (sy_call_t *)sys_munlock, .sy_auevent = AUE_MUNLOCK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 204 = munlock */
	{ .sy_narg = AS(undelete_args), .sy_call = (sy_call_t *)sys_undelete, .sy_auevent = AUE_UNDELETE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 205 = undelete */
	{ .sy_narg = AS(freebsd32_futimes_args), .sy_call = (sy_call_t *)freebsd32_futimes, .sy_auevent = AUE_FUTIMES, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 206 = freebsd32_futimes */
	{ .sy_narg = AS(getpgid_args), .sy_call = (sy_call_t *)sys_getpgid, .sy_auevent = AUE_GETPGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 207 = getpgid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 208 = reserved for local use */
	{ .sy_narg = AS(poll_args), .sy_call = (sy_call_t *)sys_poll, .sy_auevent = AUE_POLL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 209 = poll */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 210 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 211 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 212 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 213 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 214 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 215 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 216 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 217 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 218 = lkmnosys */
	{ .sy_narg = AS(nosys_args), .sy_call = (sy_call_t *)lkmnosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 219 = lkmnosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 220 = freebsd7 freebsd32___semctl */
	{ .sy_narg = AS(semget_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 221 = semget */
	{ .sy_narg = AS(semop_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 222 = semop */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 223 = obsolete semconfig */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 224 = freebsd7 freebsd32_msgctl */
	{ .sy_narg = AS(msgget_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 225 = msgget */
	{ .sy_narg = AS(freebsd32_msgsnd_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 226 = freebsd32_msgsnd */
	{ .sy_narg = AS(freebsd32_msgrcv_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 227 = freebsd32_msgrcv */
	{ .sy_narg = AS(shmat_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 228 = shmat */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 229 = freebsd7 freebsd32_shmctl */
	{ .sy_narg = AS(shmdt_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 230 = shmdt */
	{ .sy_narg = AS(shmget_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 231 = shmget */
	{ .sy_narg = AS(freebsd32_clock_gettime_args), .sy_call = (sy_call_t *)freebsd32_clock_gettime, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 232 = freebsd32_clock_gettime */
	{ .sy_narg = AS(freebsd32_clock_settime_args), .sy_call = (sy_call_t *)freebsd32_clock_settime, .sy_auevent = AUE_CLOCK_SETTIME, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 233 = freebsd32_clock_settime */
	{ .sy_narg = AS(freebsd32_clock_getres_args), .sy_call = (sy_call_t *)freebsd32_clock_getres, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 234 = freebsd32_clock_getres */
	{ .sy_narg = AS(freebsd32_ktimer_create_args), .sy_call = (sy_call_t *)freebsd32_ktimer_create, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 235 = freebsd32_ktimer_create */
	{ .sy_narg = AS(ktimer_delete_args), .sy_call = (sy_call_t *)sys_ktimer_delete, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 236 = ktimer_delete */
	{ .sy_narg = AS(freebsd32_ktimer_settime_args), .sy_call = (sy_call_t *)freebsd32_ktimer_settime, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 237 = freebsd32_ktimer_settime */
	{ .sy_narg = AS(freebsd32_ktimer_gettime_args), .sy_call = (sy_call_t *)freebsd32_ktimer_gettime, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 238 = freebsd32_ktimer_gettime */
	{ .sy_narg = AS(ktimer_getoverrun_args), .sy_call = (sy_call_t *)sys_ktimer_getoverrun, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 239 = ktimer_getoverrun */
	{ .sy_narg = AS(freebsd32_nanosleep_args), .sy_call = (sy_call_t *)freebsd32_nanosleep, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 240 = freebsd32_nanosleep */
	{ .sy_narg = AS(ffclock_getcounter_args), .sy_call = (sy_call_t *)sys_ffclock_getcounter, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 241 = ffclock_getcounter */
	{ .sy_narg = AS(freebsd32_ffclock_setestimate_args), .sy_call = (sy_call_t *)freebsd32_ffclock_setestimate, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 242 = freebsd32_ffclock_setestimate */
	{ .sy_narg = AS(freebsd32_ffclock_getestimate_args), .sy_call = (sy_call_t *)freebsd32_ffclock_getestimate, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 243 = freebsd32_ffclock_getestimate */
	{ .sy_narg = AS(freebsd32_clock_nanosleep_args), .sy_call = (sy_call_t *)freebsd32_clock_nanosleep, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 244 = freebsd32_clock_nanosleep */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 245 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 246 = reserved for local use */
	{ .sy_narg = AS(freebsd32_clock_getcpuclockid2_args), .sy_call = (sy_call_t *)freebsd32_clock_getcpuclockid2, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 247 = freebsd32_clock_getcpuclockid2 */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 248 = freebsd32_ntp_gettime */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 249 = reserved for local use */
	{ .sy_narg = AS(minherit_args), .sy_call = (sy_call_t *)sys_minherit, .sy_auevent = AUE_MINHERIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 250 = minherit */
	{ .sy_narg = AS(rfork_args), .sy_call = (sy_call_t *)sys_rfork, .sy_auevent = AUE_RFORK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 251 = rfork */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 252 = obsolete openbsd_poll */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_issetugid, .sy_auevent = AUE_ISSETUGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 253 = issetugid */
	{ .sy_narg = AS(lchown_args), .sy_call = (sy_call_t *)sys_lchown, .sy_auevent = AUE_LCHOWN, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 254 = lchown */
	{ .sy_narg = AS(freebsd32_aio_read_args), .sy_call = (sy_call_t *)freebsd32_aio_read, .sy_auevent = AUE_AIO_READ, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 255 = freebsd32_aio_read */
	{ .sy_narg = AS(freebsd32_aio_write_args), .sy_call = (sy_call_t *)freebsd32_aio_write, .sy_auevent = AUE_AIO_WRITE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 256 = freebsd32_aio_write */
	{ .sy_narg = AS(freebsd32_lio_listio_args), .sy_call = (sy_call_t *)freebsd32_lio_listio, .sy_auevent = AUE_LIO_LISTIO, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 257 = freebsd32_lio_listio */
	{ .sy_narg = AS(__proc_info_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 258 = __proc_info */
	{ .sy_narg = AS(__iopolicysys_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 259 = __iopolicysys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 260 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 261 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 262 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 263 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 264 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 265 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 266 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 267 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 268 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 269 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 270 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 271 = reserved for local use */
	{ compat11(AS(freebsd11_getdents_args),getdents), .sy_auevent = AUE_O_GETDENTS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 272 = freebsd11 getdents */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 273 = reserved for local use */
	{ .sy_narg = AS(lchmod_args), .sy_call = (sy_call_t *)sys_lchmod, .sy_auevent = AUE_LCHMOD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 274 = lchmod */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 275 = obsolete netbsd_lchown */
	{ .sy_narg = AS(freebsd32_lutimes_args), .sy_call = (sy_call_t *)freebsd32_lutimes, .sy_auevent = AUE_LUTIMES, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 276 = freebsd32_lutimes */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 277 = obsolete netbsd_msync */
	{ compat11(AS(freebsd11_freebsd32_nstat_args),freebsd32_nstat), .sy_auevent = AUE_STAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 278 = freebsd11 freebsd32_nstat */
	{ compat11(AS(freebsd11_freebsd32_nfstat_args),freebsd32_nfstat), .sy_auevent = AUE_FSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 279 = freebsd11 freebsd32_nfstat */
	{ compat11(AS(freebsd11_freebsd32_nlstat_args),freebsd32_nlstat), .sy_auevent = AUE_LSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 280 = freebsd11 freebsd32_nlstat */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 281 = audit_session_self */
	{ .sy_narg = AS(audit_session_join_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 282 = audit_session_join */
	{ .sy_narg = AS(audit_session_port_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 283 = audit_session_port */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 284 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 285 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 286 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 287 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 288 = reserved for local use */
	{ .sy_narg = AS(freebsd32_preadv_args), .sy_call = (sy_call_t *)freebsd32_preadv, .sy_auevent = AUE_PREADV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 289 = freebsd32_preadv */
	{ .sy_narg = AS(freebsd32_pwritev_args), .sy_call = (sy_call_t *)freebsd32_pwritev, .sy_auevent = AUE_PWRITEV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 290 = freebsd32_pwritev */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 291 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 292 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 293 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 294 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 295 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 296 = reserved for local use */
	{ compat4(AS(freebsd4_freebsd32_fhstatfs_args),freebsd32_fhstatfs), .sy_auevent = AUE_FHSTATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 297 = freebsd4 freebsd32_fhstatfs */
	{ .sy_narg = AS(fhopen_args), .sy_call = (sy_call_t *)sys_fhopen, .sy_auevent = AUE_FHOPEN, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 298 = fhopen */
	{ compat11(AS(freebsd11_freebsd32_fhstat_args),freebsd32_fhstat), .sy_auevent = AUE_FHSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 299 = freebsd11 freebsd32_fhstat */
	{ .sy_narg = AS(modnext_args), .sy_call = (sy_call_t *)sys_modnext, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 300 = modnext */
	{ .sy_narg = AS(freebsd32_modstat_args), .sy_call = (sy_call_t *)freebsd32_modstat, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 301 = freebsd32_modstat */
	{ .sy_narg = AS(modfnext_args), .sy_call = (sy_call_t *)sys_modfnext, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 302 = modfnext */
	{ .sy_narg = AS(modfind_args), .sy_call = (sy_call_t *)sys_modfind, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 303 = modfind */
	{ .sy_narg = AS(kldload_args), .sy_call = (sy_call_t *)sys_kldload, .sy_auevent = AUE_MODLOAD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 304 = kldload */
	{ .sy_narg = AS(kldunload_args), .sy_call = (sy_call_t *)sys_kldunload, .sy_auevent = AUE_MODUNLOAD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 305 = kldunload */
	{ .sy_narg = AS(kldfind_args), .sy_call = (sy_call_t *)sys_kldfind, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 306 = kldfind */
	{ .sy_narg = AS(kldnext_args), .sy_call = (sy_call_t *)sys_kldnext, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 307 = kldnext */
	{ .sy_narg = AS(freebsd32_kldstat_args), .sy_call = (sy_call_t *)freebsd32_kldstat, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 308 = freebsd32_kldstat */
	{ .sy_narg = AS(kldfirstmod_args), .sy_call = (sy_call_t *)sys_kldfirstmod, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 309 = kldfirstmod */
	{ .sy_narg = AS(getsid_args), .sy_call = (sy_call_t *)sys_getsid, .sy_auevent = AUE_GETSID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 310 = getsid */
	{ .sy_narg = AS(setresuid_args), .sy_call = (sy_call_t *)sys_setresuid, .sy_auevent = AUE_SETRESUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 311 = setresuid */
	{ .sy_narg = AS(setresgid_args), .sy_call = (sy_call_t *)sys_setresgid, .sy_auevent = AUE_SETRESGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 312 = setresgid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 313 = obsolete signanosleep */
	{ .sy_narg = AS(freebsd32_aio_return_args), .sy_call = (sy_call_t *)freebsd32_aio_return, .sy_auevent = AUE_AIO_RETURN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 314 = freebsd32_aio_return */
	{ .sy_narg = AS(freebsd32_aio_suspend_args), .sy_call = (sy_call_t *)freebsd32_aio_suspend, .sy_auevent = AUE_AIO_SUSPEND, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 315 = freebsd32_aio_suspend */
	{ .sy_narg = AS(aio_cancel_args), .sy_call = (sy_call_t *)sys_aio_cancel, .sy_auevent = AUE_AIO_CANCEL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 316 = aio_cancel */
	{ .sy_narg = AS(freebsd32_aio_error_args), .sy_call = (sy_call_t *)freebsd32_aio_error, .sy_auevent = AUE_AIO_ERROR, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 317 = freebsd32_aio_error */
	{ compat6(AS(freebsd6_freebsd32_aio_read_args),freebsd32_aio_read), .sy_auevent = AUE_AIO_READ, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 318 = freebsd6 freebsd32_aio_read */
	{ compat6(AS(freebsd6_freebsd32_aio_write_args),freebsd32_aio_write), .sy_auevent = AUE_AIO_WRITE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 319 = freebsd6 freebsd32_aio_write */
	{ compat6(AS(freebsd6_freebsd32_lio_listio_args),freebsd32_lio_listio), .sy_auevent = AUE_LIO_LISTIO, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 320 = freebsd6 freebsd32_lio_listio */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_yield, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 321 = yield */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 322 = obsolete thr_sleep */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 323 = obsolete thr_wakeup */
	{ .sy_narg = AS(mlockall_args), .sy_call = (sy_call_t *)sys_mlockall, .sy_auevent = AUE_MLOCKALL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 324 = mlockall */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_munlockall, .sy_auevent = AUE_MUNLOCKALL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 325 = munlockall */
	{ .sy_narg = AS(__getcwd_args), .sy_call = (sy_call_t *)sys___getcwd, .sy_auevent = AUE_GETCWD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 326 = __getcwd */
	{ .sy_narg = AS(sched_setparam_args), .sy_call = (sy_call_t *)sys_sched_setparam, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 327 = sched_setparam */
	{ .sy_narg = AS(sched_getparam_args), .sy_call = (sy_call_t *)sys_sched_getparam, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 328 = sched_getparam */
	{ .sy_narg = AS(sched_setscheduler_args), .sy_call = (sy_call_t *)sys_sched_setscheduler, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 329 = sched_setscheduler */
	{ .sy_narg = AS(sched_getscheduler_args), .sy_call = (sy_call_t *)sys_sched_getscheduler, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 330 = sched_getscheduler */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_sched_yield, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 331 = sched_yield */
	{ .sy_narg = AS(sched_get_priority_max_args), .sy_call = (sy_call_t *)sys_sched_get_priority_max, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 332 = sched_get_priority_max */
	{ .sy_narg = AS(sched_get_priority_min_args), .sy_call = (sy_call_t *)sys_sched_get_priority_min, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 333 = sched_get_priority_min */
	{ .sy_narg = AS(freebsd32_sched_rr_get_interval_args), .sy_call = (sy_call_t *)freebsd32_sched_rr_get_interval, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 334 = freebsd32_sched_rr_get_interval */
	{ .sy_narg = AS(utrace_args), .sy_call = (sy_call_t *)sys_utrace, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 335 = utrace */
	{ compat4(AS(freebsd4_freebsd32_sendfile_args),freebsd32_sendfile), .sy_auevent = AUE_SENDFILE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 336 = freebsd4 freebsd32_sendfile */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 337 = freebsd32_kldsym */
	{ .sy_narg = AS(freebsd32_jail_args), .sy_call = (sy_call_t *)freebsd32_jail, .sy_auevent = AUE_JAIL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 338 = freebsd32_jail */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 339 = nnpfs_syscall */
	{ .sy_narg = AS(sigprocmask_args), .sy_call = (sy_call_t *)sys_sigprocmask, .sy_auevent = AUE_SIGPROCMASK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 340 = sigprocmask */
	{ .sy_narg = AS(sigsuspend_args), .sy_call = (sy_call_t *)sys_sigsuspend, .sy_auevent = AUE_SIGSUSPEND, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 341 = sigsuspend */
	{ compat4(AS(freebsd4_freebsd32_sigaction_args),freebsd32_sigaction), .sy_auevent = AUE_SIGACTION, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 342 = freebsd4 freebsd32_sigaction */
	{ .sy_narg = AS(sigpending_args), .sy_call = (sy_call_t *)sys_sigpending, .sy_auevent = AUE_SIGPENDING, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 343 = sigpending */
	{ compat4(AS(freebsd4_freebsd32_sigreturn_args),freebsd32_sigreturn), .sy_auevent = AUE_SIGRETURN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 344 = freebsd4 freebsd32_sigreturn */
	{ .sy_narg = AS(freebsd32_sigtimedwait_args), .sy_call = (sy_call_t *)freebsd32_sigtimedwait, .sy_auevent = AUE_SIGWAIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 345 = freebsd32_sigtimedwait */
	{ .sy_narg = AS(freebsd32_sigwaitinfo_args), .sy_call = (sy_call_t *)freebsd32_sigwaitinfo, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 346 = freebsd32_sigwaitinfo */
	{ .sy_narg = AS(__acl_get_file_args), .sy_call = (sy_call_t *)sys___acl_get_file, .sy_auevent = AUE_ACL_GET_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 347 = __acl_get_file */
	{ .sy_narg = AS(__acl_set_file_args), .sy_call = (sy_call_t *)sys___acl_set_file, .sy_auevent = AUE_ACL_SET_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 348 = __acl_set_file */
	{ .sy_narg = AS(__acl_get_fd_args), .sy_call = (sy_call_t *)sys___acl_get_fd, .sy_auevent = AUE_ACL_GET_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 349 = __acl_get_fd */
	{ .sy_narg = AS(__acl_set_fd_args), .sy_call = (sy_call_t *)sys___acl_set_fd, .sy_auevent = AUE_ACL_SET_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 350 = __acl_set_fd */
	{ .sy_narg = AS(__acl_delete_file_args), .sy_call = (sy_call_t *)sys___acl_delete_file, .sy_auevent = AUE_ACL_DELETE_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 351 = __acl_delete_file */
	{ .sy_narg = AS(__acl_delete_fd_args), .sy_call = (sy_call_t *)sys___acl_delete_fd, .sy_auevent = AUE_ACL_DELETE_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 352 = __acl_delete_fd */
	{ .sy_narg = AS(__acl_aclcheck_file_args), .sy_call = (sy_call_t *)sys___acl_aclcheck_file, .sy_auevent = AUE_ACL_CHECK_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 353 = __acl_aclcheck_file */
	{ .sy_narg = AS(__acl_aclcheck_fd_args), .sy_call = (sy_call_t *)sys___acl_aclcheck_fd, .sy_auevent = AUE_ACL_CHECK_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 354 = __acl_aclcheck_fd */
	{ .sy_narg = AS(extattrctl_args), .sy_call = (sy_call_t *)sys_extattrctl, .sy_auevent = AUE_EXTATTRCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 355 = extattrctl */
	{ .sy_narg = AS(extattr_set_file_args), .sy_call = (sy_call_t *)sys_extattr_set_file, .sy_auevent = AUE_EXTATTR_SET_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 356 = extattr_set_file */
	{ .sy_narg = AS(extattr_get_file_args), .sy_call = (sy_call_t *)sys_extattr_get_file, .sy_auevent = AUE_EXTATTR_GET_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 357 = extattr_get_file */
	{ .sy_narg = AS(extattr_delete_file_args), .sy_call = (sy_call_t *)sys_extattr_delete_file, .sy_auevent = AUE_EXTATTR_DELETE_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 358 = extattr_delete_file */
	{ .sy_narg = AS(freebsd32_aio_waitcomplete_args), .sy_call = (sy_call_t *)freebsd32_aio_waitcomplete, .sy_auevent = AUE_AIO_WAITCOMPLETE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 359 = freebsd32_aio_waitcomplete */
	{ .sy_narg = AS(getresuid_args), .sy_call = (sy_call_t *)sys_getresuid, .sy_auevent = AUE_GETRESUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 360 = getresuid */
	{ .sy_narg = AS(getresgid_args), .sy_call = (sy_call_t *)sys_getresgid, .sy_auevent = AUE_GETRESGID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 361 = getresgid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_kqueue, .sy_auevent = AUE_KQUEUE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 362 = kqueue */
	{ compat11(AS(freebsd11_freebsd32_kevent_args),freebsd32_kevent), .sy_auevent = AUE_KEVENT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 363 = freebsd11 freebsd32_kevent */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 364 = obsolete __cap_get_proc */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 365 = obsolete __cap_set_proc */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 366 = obsolete __cap_get_fd */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 367 = obsolete __cap_get_file */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 368 = obsolete __cap_set_fd */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 369 = obsolete __cap_set_file */
	{ .sy_narg = AS(kevent64_args), .sy_call = (sy_call_t *)sys_kevent64, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 370 = kevent64 */
	{ .sy_narg = AS(extattr_set_fd_args), .sy_call = (sy_call_t *)sys_extattr_set_fd, .sy_auevent = AUE_EXTATTR_SET_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 371 = extattr_set_fd */
	{ .sy_narg = AS(extattr_get_fd_args), .sy_call = (sy_call_t *)sys_extattr_get_fd, .sy_auevent = AUE_EXTATTR_GET_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 372 = extattr_get_fd */
	{ .sy_narg = AS(extattr_delete_fd_args), .sy_call = (sy_call_t *)sys_extattr_delete_fd, .sy_auevent = AUE_EXTATTR_DELETE_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 373 = extattr_delete_fd */
	{ .sy_narg = AS(__setugid_args), .sy_call = (sy_call_t *)sys___setugid, .sy_auevent = AUE_SETUGID, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 374 = __setugid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 375 = obsolete nfsclnt */
	{ .sy_narg = AS(eaccess_args), .sy_call = (sy_call_t *)sys_eaccess, .sy_auevent = AUE_EACCESS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 376 = eaccess */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 377 = freebsd32_afs3_syscall */
	{ .sy_narg = AS(freebsd32_nmount_args), .sy_call = (sy_call_t *)freebsd32_nmount, .sy_auevent = AUE_NMOUNT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 378 = freebsd32_nmount */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 379 = obsolete kse_exit */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 380 = obsolete kse_wakeup */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 381 = obsolete kse_create */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 382 = obsolete kse_thr_interrupt */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 383 = obsolete kse_release */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 384 = freebsd32___mac_get_proc */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 385 = freebsd32___mac_set_proc */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 386 = freebsd32___mac_get_fd */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 387 = freebsd32___mac_get_file */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 388 = freebsd32___mac_set_fd */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 389 = freebsd32___mac_set_file */
	{ .sy_narg = AS(kenv_args), .sy_call = (sy_call_t *)sys_kenv, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 390 = kenv */
	{ .sy_narg = AS(lchflags_args), .sy_call = (sy_call_t *)sys_lchflags, .sy_auevent = AUE_LCHFLAGS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 391 = lchflags */
	{ .sy_narg = AS(uuidgen_args), .sy_call = (sy_call_t *)sys_uuidgen, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 392 = uuidgen */
	{ .sy_narg = AS(freebsd32_sendfile_args), .sy_call = (sy_call_t *)freebsd32_sendfile, .sy_auevent = AUE_SENDFILE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 393 = freebsd32_sendfile */
	{ .sy_narg = AS(mac_syscall_args), .sy_call = (sy_call_t *)sys_mac_syscall, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 394 = mac_syscall */
	{ compat11(AS(freebsd11_freebsd32_getfsstat_args),freebsd32_getfsstat), .sy_auevent = AUE_GETFSSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 395 = freebsd11 freebsd32_getfsstat */
	{ compat11(AS(freebsd11_statfs_args),statfs), .sy_auevent = AUE_STATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 396 = freebsd11 statfs */
	{ compat11(AS(freebsd11_fstatfs_args),fstatfs), .sy_auevent = AUE_FSTATFS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 397 = freebsd11 fstatfs */
	{ compat11(AS(freebsd11_fhstatfs_args),fhstatfs), .sy_auevent = AUE_FHSTATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 398 = freebsd11 fhstatfs */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 399 = reserved for local use */
	{ .sy_narg = AS(ksem_close_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 400 = ksem_close */
	{ .sy_narg = AS(ksem_post_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 401 = ksem_post */
	{ .sy_narg = AS(ksem_wait_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 402 = ksem_wait */
	{ .sy_narg = AS(ksem_trywait_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 403 = ksem_trywait */
	{ .sy_narg = AS(freebsd32_ksem_init_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 404 = freebsd32_ksem_init */
	{ .sy_narg = AS(freebsd32_ksem_open_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 405 = freebsd32_ksem_open */
	{ .sy_narg = AS(ksem_unlink_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 406 = ksem_unlink */
	{ .sy_narg = AS(ksem_getvalue_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 407 = ksem_getvalue */
	{ .sy_narg = AS(ksem_destroy_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 408 = ksem_destroy */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 409 = freebsd32___mac_get_pid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 410 = freebsd32___mac_get_link */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 411 = freebsd32___mac_set_link */
	{ .sy_narg = AS(extattr_set_link_args), .sy_call = (sy_call_t *)sys_extattr_set_link, .sy_auevent = AUE_EXTATTR_SET_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 412 = extattr_set_link */
	{ .sy_narg = AS(extattr_get_link_args), .sy_call = (sy_call_t *)sys_extattr_get_link, .sy_auevent = AUE_EXTATTR_GET_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 413 = extattr_get_link */
	{ .sy_narg = AS(extattr_delete_link_args), .sy_call = (sy_call_t *)sys_extattr_delete_link, .sy_auevent = AUE_EXTATTR_DELETE_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 414 = extattr_delete_link */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 415 = freebsd32___mac_execve */
	{ .sy_narg = AS(freebsd32_sigaction_args), .sy_call = (sy_call_t *)freebsd32_sigaction, .sy_auevent = AUE_SIGACTION, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 416 = freebsd32_sigaction */
	{ .sy_narg = AS(freebsd32_sigreturn_args), .sy_call = (sy_call_t *)freebsd32_sigreturn, .sy_auevent = AUE_SIGRETURN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 417 = freebsd32_sigreturn */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 418 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 419 = reserved for local use */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 420 = reserved for local use */
	{ .sy_narg = AS(freebsd32_getcontext_args), .sy_call = (sy_call_t *)freebsd32_getcontext, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 421 = freebsd32_getcontext */
	{ .sy_narg = AS(freebsd32_setcontext_args), .sy_call = (sy_call_t *)freebsd32_setcontext, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 422 = freebsd32_setcontext */
	{ .sy_narg = AS(freebsd32_swapcontext_args), .sy_call = (sy_call_t *)freebsd32_swapcontext, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 423 = freebsd32_swapcontext */
	{ compat13(AS(freebsd13_swapoff_args),swapoff), .sy_auevent = AUE_SWAPOFF, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 424 = freebsd13 swapoff */
	{ .sy_narg = AS(__acl_get_link_args), .sy_call = (sy_call_t *)sys___acl_get_link, .sy_auevent = AUE_ACL_GET_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 425 = __acl_get_link */
	{ .sy_narg = AS(__acl_set_link_args), .sy_call = (sy_call_t *)sys___acl_set_link, .sy_auevent = AUE_ACL_SET_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 426 = __acl_set_link */
	{ .sy_narg = AS(__acl_delete_link_args), .sy_call = (sy_call_t *)sys___acl_delete_link, .sy_auevent = AUE_ACL_DELETE_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 427 = __acl_delete_link */
	{ .sy_narg = AS(__acl_aclcheck_link_args), .sy_call = (sy_call_t *)sys___acl_aclcheck_link, .sy_auevent = AUE_ACL_CHECK_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 428 = __acl_aclcheck_link */
	{ .sy_narg = AS(sigwait_args), .sy_call = (sy_call_t *)sys_sigwait, .sy_auevent = AUE_SIGWAIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 429 = sigwait */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 430 = freebsd32_thr_create */
	{ .sy_narg = AS(thr_exit_args), .sy_call = (sy_call_t *)sys_thr_exit, .sy_auevent = AUE_THR_EXIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 431 = thr_exit */
	{ .sy_narg = AS(thr_self_args), .sy_call = (sy_call_t *)sys_thr_self, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 432 = thr_self */
	{ .sy_narg = AS(thr_kill_args), .sy_call = (sy_call_t *)sys_thr_kill, .sy_auevent = AUE_THR_KILL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 433 = thr_kill */
	{ compat10(AS(freebsd10_freebsd32__umtx_lock_args),freebsd32__umtx_lock), .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 434 = freebsd10 freebsd32__umtx_lock */
	{ compat10(AS(freebsd10_freebsd32__umtx_unlock_args),freebsd32__umtx_unlock), .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 435 = freebsd10 freebsd32__umtx_unlock */
	{ .sy_narg = AS(jail_attach_args), .sy_call = (sy_call_t *)sys_jail_attach, .sy_auevent = AUE_JAIL_ATTACH, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 436 = jail_attach */
	{ .sy_narg = AS(extattr_list_fd_args), .sy_call = (sy_call_t *)sys_extattr_list_fd, .sy_auevent = AUE_EXTATTR_LIST_FD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 437 = extattr_list_fd */
	{ .sy_narg = AS(extattr_list_file_args), .sy_call = (sy_call_t *)sys_extattr_list_file, .sy_auevent = AUE_EXTATTR_LIST_FILE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 438 = extattr_list_file */
	{ .sy_narg = AS(extattr_list_link_args), .sy_call = (sy_call_t *)sys_extattr_list_link, .sy_auevent = AUE_EXTATTR_LIST_LINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 439 = extattr_list_link */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 440 = obsolete kse_switchin */
	{ .sy_narg = AS(freebsd32_ksem_timedwait_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 441 = freebsd32_ksem_timedwait */
	{ .sy_narg = AS(freebsd32_thr_suspend_args), .sy_call = (sy_call_t *)freebsd32_thr_suspend, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 442 = freebsd32_thr_suspend */
	{ .sy_narg = AS(thr_wake_args), .sy_call = (sy_call_t *)sys_thr_wake, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 443 = thr_wake */
	{ .sy_narg = AS(kldunloadf_args), .sy_call = (sy_call_t *)sys_kldunloadf, .sy_auevent = AUE_MODUNLOAD, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 444 = kldunloadf */
	{ .sy_narg = AS(audit_args), .sy_call = (sy_call_t *)sys_audit, .sy_auevent = AUE_AUDIT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 445 = audit */
	{ .sy_narg = AS(auditon_args), .sy_call = (sy_call_t *)sys_auditon, .sy_auevent = AUE_AUDITON, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 446 = auditon */
	{ .sy_narg = AS(getauid_args), .sy_call = (sy_call_t *)sys_getauid, .sy_auevent = AUE_GETAUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 447 = getauid */
	{ .sy_narg = AS(setauid_args), .sy_call = (sy_call_t *)sys_setauid, .sy_auevent = AUE_SETAUID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 448 = setauid */
	{ .sy_narg = AS(getaudit_args), .sy_call = (sy_call_t *)sys_getaudit, .sy_auevent = AUE_GETAUDIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 449 = getaudit */
	{ .sy_narg = AS(setaudit_args), .sy_call = (sy_call_t *)sys_setaudit, .sy_auevent = AUE_SETAUDIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 450 = setaudit */
	{ .sy_narg = AS(getaudit_addr_args), .sy_call = (sy_call_t *)sys_getaudit_addr, .sy_auevent = AUE_GETAUDIT_ADDR, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 451 = getaudit_addr */
	{ .sy_narg = AS(setaudit_addr_args), .sy_call = (sy_call_t *)sys_setaudit_addr, .sy_auevent = AUE_SETAUDIT_ADDR, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 452 = setaudit_addr */
	{ .sy_narg = AS(auditctl_args), .sy_call = (sy_call_t *)sys_auditctl, .sy_auevent = AUE_AUDITCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 453 = auditctl */
	{ .sy_narg = AS(freebsd32__umtx_op_args), .sy_call = (sy_call_t *)freebsd32__umtx_op, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 454 = freebsd32__umtx_op */
	{ .sy_narg = AS(freebsd32_thr_new_args), .sy_call = (sy_call_t *)freebsd32_thr_new, .sy_auevent = AUE_THR_NEW, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 455 = freebsd32_thr_new */
	{ .sy_narg = AS(freebsd32_sigqueue_args), .sy_call = (sy_call_t *)freebsd32_sigqueue, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 456 = freebsd32_sigqueue */
	{ .sy_narg = AS(freebsd32_kmq_open_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 457 = freebsd32_kmq_open */
	{ .sy_narg = AS(freebsd32_kmq_setattr_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 458 = freebsd32_kmq_setattr */
	{ .sy_narg = AS(freebsd32_kmq_timedreceive_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 459 = freebsd32_kmq_timedreceive */
	{ .sy_narg = AS(freebsd32_kmq_timedsend_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 460 = freebsd32_kmq_timedsend */
	{ .sy_narg = AS(freebsd32_kmq_notify_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 461 = freebsd32_kmq_notify */
	{ .sy_narg = AS(kmq_unlink_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 462 = kmq_unlink */
	{ .sy_narg = AS(freebsd32_abort2_args), .sy_call = (sy_call_t *)freebsd32_abort2, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 463 = freebsd32_abort2 */
	{ .sy_narg = AS(thr_set_name_args), .sy_call = (sy_call_t *)sys_thr_set_name, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 464 = thr_set_name */
	{ .sy_narg = AS(freebsd32_aio_fsync_args), .sy_call = (sy_call_t *)freebsd32_aio_fsync, .sy_auevent = AUE_AIO_FSYNC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 465 = freebsd32_aio_fsync */
	{ .sy_narg = AS(rtprio_thread_args), .sy_call = (sy_call_t *)sys_rtprio_thread, .sy_auevent = AUE_RTPRIO, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 466 = rtprio_thread */
	{ .sy_narg = AS(thr_stack_args), .sy_call = (sy_call_t *)sys_thr_stack, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 467 = thr_stack */
	{ .sy_narg = AS(thr_workq_args), .sy_call = (sy_call_t *)sys_thr_workq, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 468 = thr_workq */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 469 = __getpath_fromfd */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 470 = __getpath_fromaddr */
	{ .sy_narg = AS(sctp_peeloff_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 471 = sctp_peeloff */
	{ .sy_narg = AS(sctp_generic_sendmsg_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 472 = sctp_generic_sendmsg */
	{ .sy_narg = AS(sctp_generic_sendmsg_iov_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 473 = sctp_generic_sendmsg_iov */
	{ .sy_narg = AS(sctp_generic_recvmsg_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_ABSENT },	/* 474 = sctp_generic_recvmsg */
	{ .sy_narg = AS(freebsd32_pread_args), .sy_call = (sy_call_t *)freebsd32_pread, .sy_auevent = AUE_PREAD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 475 = freebsd32_pread */
	{ .sy_narg = AS(freebsd32_pwrite_args), .sy_call = (sy_call_t *)freebsd32_pwrite, .sy_auevent = AUE_PWRITE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 476 = freebsd32_pwrite */
	{ .sy_narg = AS(freebsd32_mmap_args), .sy_call = (sy_call_t *)freebsd32_mmap, .sy_auevent = AUE_MMAP, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 477 = freebsd32_mmap */
	{ .sy_narg = AS(freebsd32_lseek_args), .sy_call = (sy_call_t *)freebsd32_lseek, .sy_auevent = AUE_LSEEK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 478 = freebsd32_lseek */
	{ .sy_narg = AS(freebsd32_truncate_args), .sy_call = (sy_call_t *)freebsd32_truncate, .sy_auevent = AUE_TRUNCATE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 479 = freebsd32_truncate */
	{ .sy_narg = AS(freebsd32_ftruncate_args), .sy_call = (sy_call_t *)freebsd32_ftruncate, .sy_auevent = AUE_FTRUNCATE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 480 = freebsd32_ftruncate */
	{ .sy_narg = AS(thr_kill2_args), .sy_call = (sy_call_t *)sys_thr_kill2, .sy_auevent = AUE_THR_KILL2, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 481 = thr_kill2 */
	{ compat12(AS(freebsd12_shm_open_args),shm_open), .sy_auevent = AUE_SHMOPEN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 482 = freebsd12 shm_open */
	{ .sy_narg = AS(shm_unlink_args), .sy_call = (sy_call_t *)sys_shm_unlink, .sy_auevent = AUE_SHMUNLINK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 483 = shm_unlink */
	{ .sy_narg = AS(cpuset_args), .sy_call = (sy_call_t *)sys_cpuset, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 484 = cpuset */
	{ .sy_narg = AS(freebsd32_cpuset_setid_args), .sy_call = (sy_call_t *)freebsd32_cpuset_setid, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 485 = freebsd32_cpuset_setid */
	{ .sy_narg = AS(freebsd32_cpuset_getid_args), .sy_call = (sy_call_t *)freebsd32_cpuset_getid, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 486 = freebsd32_cpuset_getid */
	{ .sy_narg = AS(freebsd32_cpuset_getaffinity_args), .sy_call = (sy_call_t *)freebsd32_cpuset_getaffinity, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 487 = freebsd32_cpuset_getaffinity */
	{ .sy_narg = AS(freebsd32_cpuset_setaffinity_args), .sy_call = (sy_call_t *)freebsd32_cpuset_setaffinity, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 488 = freebsd32_cpuset_setaffinity */
	{ .sy_narg = AS(faccessat_args), .sy_call = (sy_call_t *)sys_faccessat, .sy_auevent = AUE_FACCESSAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 489 = faccessat */
	{ .sy_narg = AS(fchmodat_args), .sy_call = (sy_call_t *)sys_fchmodat, .sy_auevent = AUE_FCHMODAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 490 = fchmodat */
	{ .sy_narg = AS(fchownat_args), .sy_call = (sy_call_t *)sys_fchownat, .sy_auevent = AUE_FCHOWNAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 491 = fchownat */
	{ .sy_narg = AS(freebsd32_fexecve_args), .sy_call = (sy_call_t *)freebsd32_fexecve, .sy_auevent = AUE_FEXECVE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 492 = freebsd32_fexecve */
	{ compat11(AS(freebsd11_freebsd32_fstatat_args),freebsd32_fstatat), .sy_auevent = AUE_FSTATAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 493 = freebsd11 freebsd32_fstatat */
	{ .sy_narg = AS(freebsd32_futimesat_args), .sy_call = (sy_call_t *)freebsd32_futimesat, .sy_auevent = AUE_FUTIMESAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 494 = freebsd32_futimesat */
	{ .sy_narg = AS(linkat_args), .sy_call = (sy_call_t *)sys_linkat, .sy_auevent = AUE_LINKAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 495 = linkat */
	{ .sy_narg = AS(mkdirat_args), .sy_call = (sy_call_t *)sys_mkdirat, .sy_auevent = AUE_MKDIRAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 496 = mkdirat */
	{ .sy_narg = AS(mkfifoat_args), .sy_call = (sy_call_t *)sys_mkfifoat, .sy_auevent = AUE_MKFIFOAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 497 = mkfifoat */
	{ compat11(AS(freebsd11_mknodat_args),mknodat), .sy_auevent = AUE_MKNODAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 498 = freebsd11 mknodat */
	{ .sy_narg = AS(openat_args), .sy_call = (sy_call_t *)sys_openat, .sy_auevent = AUE_OPENAT_RWTC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 499 = openat */
	{ .sy_narg = AS(readlinkat_args), .sy_call = (sy_call_t *)sys_readlinkat, .sy_auevent = AUE_READLINKAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 500 = readlinkat */
	{ .sy_narg = AS(renameat_args), .sy_call = (sy_call_t *)sys_renameat, .sy_auevent = AUE_RENAMEAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 501 = renameat */
	{ .sy_narg = AS(symlinkat_args), .sy_call = (sy_call_t *)sys_symlinkat, .sy_auevent = AUE_SYMLINKAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 502 = symlinkat */
	{ .sy_narg = AS(unlinkat_args), .sy_call = (sy_call_t *)sys_unlinkat, .sy_auevent = AUE_UNLINKAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 503 = unlinkat */
	{ .sy_narg = AS(posix_openpt_args), .sy_call = (sy_call_t *)sys_posix_openpt, .sy_auevent = AUE_POSIX_OPENPT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 504 = posix_openpt */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 505 = obsolete kgssapi */
	{ .sy_narg = AS(freebsd32_jail_get_args), .sy_call = (sy_call_t *)freebsd32_jail_get, .sy_auevent = AUE_JAIL_GET, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 506 = freebsd32_jail_get */
	{ .sy_narg = AS(freebsd32_jail_set_args), .sy_call = (sy_call_t *)freebsd32_jail_set, .sy_auevent = AUE_JAIL_SET, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 507 = freebsd32_jail_set */
	{ .sy_narg = AS(jail_remove_args), .sy_call = (sy_call_t *)sys_jail_remove, .sy_auevent = AUE_JAIL_REMOVE, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 508 = jail_remove */
	{ compat12(AS(freebsd12_closefrom_args),closefrom), .sy_auevent = AUE_CLOSEFROM, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 509 = freebsd12 closefrom */
	{ .sy_narg = AS(freebsd32___semctl_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 510 = freebsd32___semctl */
	{ .sy_narg = AS(freebsd32_msgctl_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 511 = freebsd32_msgctl */
	{ .sy_narg = AS(freebsd32_shmctl_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 512 = freebsd32_shmctl */
	{ .sy_narg = AS(lpathconf_args), .sy_call = (sy_call_t *)sys_lpathconf, .sy_auevent = AUE_LPATHCONF, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 513 = lpathconf */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 514 = obsolete cap_new */
	{ .sy_narg = AS(__cap_rights_get_args), .sy_call = (sy_call_t *)sys___cap_rights_get, .sy_auevent = AUE_CAP_RIGHTS_GET, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 515 = __cap_rights_get */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_cap_enter, .sy_auevent = AUE_CAP_ENTER, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 516 = cap_enter */
	{ .sy_narg = AS(cap_getmode_args), .sy_call = (sy_call_t *)sys_cap_getmode, .sy_auevent = AUE_CAP_GETMODE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 517 = cap_getmode */
	{ .sy_narg = AS(pdfork_args), .sy_call = (sy_call_t *)sys_pdfork, .sy_auevent = AUE_PDFORK, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 518 = pdfork */
	{ .sy_narg = AS(pdkill_args), .sy_call = (sy_call_t *)sys_pdkill, .sy_auevent = AUE_PDKILL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 519 = pdkill */
	{ .sy_narg = AS(pdgetpid_args), .sy_call = (sy_call_t *)sys_pdgetpid, .sy_auevent = AUE_PDGETPID, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 520 = pdgetpid */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 521 = reserved for local use */
	{ .sy_narg = AS(freebsd32_pselect_args), .sy_call = (sy_call_t *)freebsd32_pselect, .sy_auevent = AUE_SELECT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 522 = freebsd32_pselect */
	{ .sy_narg = AS(getloginclass_args), .sy_call = (sy_call_t *)sys_getloginclass, .sy_auevent = AUE_GETLOGINCLASS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 523 = getloginclass */
	{ .sy_narg = AS(setloginclass_args), .sy_call = (sy_call_t *)sys_setloginclass, .sy_auevent = AUE_SETLOGINCLASS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 524 = setloginclass */
	{ .sy_narg = AS(rctl_get_racct_args), .sy_call = (sy_call_t *)sys_rctl_get_racct, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 525 = rctl_get_racct */
	{ .sy_narg = AS(rctl_get_rules_args), .sy_call = (sy_call_t *)sys_rctl_get_rules, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 526 = rctl_get_rules */
	{ .sy_narg = AS(rctl_get_limits_args), .sy_call = (sy_call_t *)sys_rctl_get_limits, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 527 = rctl_get_limits */
	{ .sy_narg = AS(rctl_add_rule_args), .sy_call = (sy_call_t *)sys_rctl_add_rule, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 528 = rctl_add_rule */
	{ .sy_narg = AS(rctl_remove_rule_args), .sy_call = (sy_call_t *)sys_rctl_remove_rule, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 529 = rctl_remove_rule */
	{ .sy_narg = AS(freebsd32_posix_fallocate_args), .sy_call = (sy_call_t *)freebsd32_posix_fallocate, .sy_auevent = AUE_POSIX_FALLOCATE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 530 = freebsd32_posix_fallocate */
	{ .sy_narg = AS(freebsd32_posix_fadvise_args), .sy_call = (sy_call_t *)freebsd32_posix_fadvise, .sy_auevent = AUE_POSIX_FADVISE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 531 = freebsd32_posix_fadvise */
	{ .sy_narg = AS(freebsd32_wait6_args), .sy_call = (sy_call_t *)freebsd32_wait6, .sy_auevent = AUE_WAIT6, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 532 = freebsd32_wait6 */
	{ .sy_narg = AS(cap_rights_limit_args), .sy_call = (sy_call_t *)sys_cap_rights_limit, .sy_auevent = AUE_CAP_RIGHTS_LIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 533 = cap_rights_limit */
	{ .sy_narg = AS(freebsd32_cap_ioctls_limit_args), .sy_call = (sy_call_t *)freebsd32_cap_ioctls_limit, .sy_auevent = AUE_CAP_IOCTLS_LIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 534 = freebsd32_cap_ioctls_limit */
	{ .sy_narg = AS(freebsd32_cap_ioctls_get_args), .sy_call = (sy_call_t *)freebsd32_cap_ioctls_get, .sy_auevent = AUE_CAP_IOCTLS_GET, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 535 = freebsd32_cap_ioctls_get */
	{ .sy_narg = AS(cap_fcntls_limit_args), .sy_call = (sy_call_t *)sys_cap_fcntls_limit, .sy_auevent = AUE_CAP_FCNTLS_LIMIT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 536 = cap_fcntls_limit */
	{ .sy_narg = AS(cap_fcntls_get_args), .sy_call = (sy_call_t *)sys_cap_fcntls_get, .sy_auevent = AUE_CAP_FCNTLS_GET, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 537 = cap_fcntls_get */
	{ .sy_narg = AS(bindat_args), .sy_call = (sy_call_t *)sys_bindat, .sy_auevent = AUE_BINDAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 538 = bindat */
	{ .sy_narg = AS(connectat_args), .sy_call = (sy_call_t *)sys_connectat, .sy_auevent = AUE_CONNECTAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 539 = connectat */
	{ .sy_narg = AS(chflagsat_args), .sy_call = (sy_call_t *)sys_chflagsat, .sy_auevent = AUE_CHFLAGSAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 540 = chflagsat */
	{ .sy_narg = AS(accept4_args), .sy_call = (sy_call_t *)sys_accept4, .sy_auevent = AUE_ACCEPT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 541 = accept4 */
	{ .sy_narg = AS(pipe2_args), .sy_call = (sy_call_t *)sys_pipe2, .sy_auevent = AUE_PIPE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 542 = pipe2 */
	{ .sy_narg = AS(freebsd32_aio_mlock_args), .sy_call = (sy_call_t *)freebsd32_aio_mlock, .sy_auevent = AUE_AIO_MLOCK, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 543 = freebsd32_aio_mlock */
	{ .sy_narg = AS(freebsd32_procctl_args), .sy_call = (sy_call_t *)freebsd32_procctl, .sy_auevent = AUE_PROCCTL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 544 = freebsd32_procctl */
	{ .sy_narg = AS(freebsd32_ppoll_args), .sy_call = (sy_call_t *)freebsd32_ppoll, .sy_auevent = AUE_POLL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 545 = freebsd32_ppoll */
	{ .sy_narg = AS(freebsd32_futimens_args), .sy_call = (sy_call_t *)freebsd32_futimens, .sy_auevent = AUE_FUTIMES, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 546 = freebsd32_futimens */
	{ .sy_narg = AS(freebsd32_utimensat_args), .sy_call = (sy_call_t *)freebsd32_utimensat, .sy_auevent = AUE_FUTIMESAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 547 = freebsd32_utimensat */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 548 = obsolete numa_getaffinity */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 549 = obsolete numa_setaffinity */
	{ .sy_narg = AS(fdatasync_args), .sy_call = (sy_call_t *)sys_fdatasync, .sy_auevent = AUE_FSYNC, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 550 = fdatasync */
	{ .sy_narg = AS(freebsd32_fstat_args), .sy_call = (sy_call_t *)freebsd32_fstat, .sy_auevent = AUE_FSTAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 551 = freebsd32_fstat */
	{ .sy_narg = AS(freebsd32_fstatat_args), .sy_call = (sy_call_t *)freebsd32_fstatat, .sy_auevent = AUE_FSTATAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 552 = freebsd32_fstatat */
	{ .sy_narg = AS(freebsd32_fhstat_args), .sy_call = (sy_call_t *)freebsd32_fhstat, .sy_auevent = AUE_FHSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 553 = freebsd32_fhstat */
	{ .sy_narg = AS(getdirentries_args), .sy_call = (sy_call_t *)sys_getdirentries, .sy_auevent = AUE_GETDIRENTRIES, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 554 = getdirentries */
	{ .sy_narg = AS(statfs_args), .sy_call = (sy_call_t *)sys_statfs, .sy_auevent = AUE_STATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 555 = statfs */
	{ .sy_narg = AS(fstatfs_args), .sy_call = (sy_call_t *)sys_fstatfs, .sy_auevent = AUE_FSTATFS, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 556 = fstatfs */
	{ .sy_narg = AS(freebsd32_getfsstat_args), .sy_call = (sy_call_t *)freebsd32_getfsstat, .sy_auevent = AUE_GETFSSTAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 557 = freebsd32_getfsstat */
	{ .sy_narg = AS(fhstatfs_args), .sy_call = (sy_call_t *)sys_fhstatfs, .sy_auevent = AUE_FHSTATFS, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 558 = fhstatfs */
	{ .sy_narg = AS(freebsd32_mknodat_args), .sy_call = (sy_call_t *)freebsd32_mknodat, .sy_auevent = AUE_MKNODAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 559 = freebsd32_mknodat */
	{ .sy_narg = AS(freebsd32_kevent_args), .sy_call = (sy_call_t *)freebsd32_kevent, .sy_auevent = AUE_KEVENT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 560 = freebsd32_kevent */
	{ .sy_narg = AS(freebsd32_cpuset_getdomain_args), .sy_call = (sy_call_t *)freebsd32_cpuset_getdomain, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 561 = freebsd32_cpuset_getdomain */
	{ .sy_narg = AS(freebsd32_cpuset_setdomain_args), .sy_call = (sy_call_t *)freebsd32_cpuset_setdomain, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 562 = freebsd32_cpuset_setdomain */
	{ .sy_narg = AS(getrandom_args), .sy_call = (sy_call_t *)sys_getrandom, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 563 = getrandom */
	{ .sy_narg = AS(getfhat_args), .sy_call = (sy_call_t *)sys_getfhat, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 564 = getfhat */
	{ .sy_narg = AS(fhlink_args), .sy_call = (sy_call_t *)sys_fhlink, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 565 = fhlink */
	{ .sy_narg = AS(fhlinkat_args), .sy_call = (sy_call_t *)sys_fhlinkat, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 566 = fhlinkat */
	{ .sy_narg = AS(fhreadlink_args), .sy_call = (sy_call_t *)sys_fhreadlink, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 567 = fhreadlink */
	{ .sy_narg = AS(funlinkat_args), .sy_call = (sy_call_t *)sys_funlinkat, .sy_auevent = AUE_UNLINKAT, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 568 = funlinkat */
	{ .sy_narg = AS(copy_file_range_args), .sy_call = (sy_call_t *)sys_copy_file_range, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 569 = copy_file_range */
	{ .sy_narg = AS(freebsd32___sysctlbyname_args), .sy_call = (sy_call_t *)freebsd32___sysctlbyname, .sy_auevent = AUE_SYSCTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 570 = freebsd32___sysctlbyname */
	{ .sy_narg = AS(shm_open2_args), .sy_call = (sy_call_t *)sys_shm_open2, .sy_auevent = AUE_SHMOPEN, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 571 = shm_open2 */
	{ .sy_narg = AS(shm_rename_args), .sy_call = (sy_call_t *)sys_shm_rename, .sy_auevent = AUE_SHMRENAME, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 572 = shm_rename */
	{ .sy_narg = AS(sigfastblock_args), .sy_call = (sy_call_t *)sys_sigfastblock, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 573 = sigfastblock */
	{ .sy_narg = AS(__realpathat_args), .sy_call = (sy_call_t *)sys___realpathat, .sy_auevent = AUE_REALPATHAT, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 574 = __realpathat */
	{ .sy_narg = AS(close_range_args), .sy_call = (sy_call_t *)sys_close_range, .sy_auevent = AUE_CLOSERANGE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 575 = close_range */
	{ .sy_narg = AS(rpctls_syscall_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 576 = rpctls_syscall */
	{ .sy_narg = AS(__specialfd_args), .sy_call = (sy_call_t *)sys___specialfd, .sy_auevent = AUE_SPECIALFD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 577 = __specialfd */
	{ .sy_narg = AS(freebsd32_aio_writev_args), .sy_call = (sy_call_t *)freebsd32_aio_writev, .sy_auevent = AUE_AIO_WRITEV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 578 = freebsd32_aio_writev */
	{ .sy_narg = AS(freebsd32_aio_readv_args), .sy_call = (sy_call_t *)freebsd32_aio_readv, .sy_auevent = AUE_AIO_READV, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 579 = freebsd32_aio_readv */
	{ .sy_narg = AS(fspacectl_args), .sy_call = (sy_call_t *)sys_fspacectl, .sy_auevent = AUE_FSPACECTL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 580 = fspacectl */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)sys_sched_getcpu, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 581 = sched_getcpu */
	{ .sy_narg = AS(swapoff_args), .sy_call = (sy_call_t *)sys_swapoff, .sy_auevent = AUE_SWAPOFF, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 582 = swapoff */
	{ .sy_narg = AS(kqueuex_args), .sy_call = (sy_call_t *)sys_kqueuex, .sy_auevent = AUE_KQUEUE, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 583 = kqueuex */
	{ .sy_narg = AS(membarrier_args), .sy_call = (sy_call_t *)sys_membarrier, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 584 = membarrier */
	{ .sy_narg = AS(timerfd_create_args), .sy_call = (sy_call_t *)sys_timerfd_create, .sy_auevent = AUE_TIMERFD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 585 = timerfd_create */
	{ .sy_narg = AS(freebsd32_timerfd_gettime_args), .sy_call = (sy_call_t *)freebsd32_timerfd_gettime, .sy_auevent = AUE_TIMERFD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 586 = freebsd32_timerfd_gettime */
	{ .sy_narg = AS(freebsd32_timerfd_settime_args), .sy_call = (sy_call_t *)freebsd32_timerfd_settime, .sy_auevent = AUE_TIMERFD, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 587 = freebsd32_timerfd_settime */
	{ .sy_narg = AS(kcmp_args), .sy_call = (sy_call_t *)sys_kcmp, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 588 = kcmp */
	{ .sy_narg = AS(getrlimitusage_args), .sy_call = (sy_call_t *)sys_getrlimitusage, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 589 = getrlimitusage */
	{ .sy_narg = AS(fchroot_args), .sy_call = (sy_call_t *)sys_fchroot, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_STATIC },	/* 590 = fchroot */
	{ .sy_narg = AS(freebsd32_setcred_args), .sy_call = (sy_call_t *)freebsd32_setcred, .sy_auevent = AUE_SETCRED, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 591 = freebsd32_setcred */
	{ .sy_narg = AS(exterrctl_args), .sy_call = (sy_call_t *)sys_exterrctl, .sy_auevent = AUE_NULL, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 592 = exterrctl */
	{ .sy_narg = AS(inotify_add_watch_at_args), .sy_call = (sy_call_t *)sys_inotify_add_watch_at, .sy_auevent = AUE_INOTIFY, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 593 = inotify_add_watch_at */
	{ .sy_narg = AS(inotify_rm_watch_args), .sy_call = (sy_call_t *)sys_inotify_rm_watch, .sy_auevent = AUE_INOTIFY, .sy_flags = SYF_CAPENABLED, .sy_thrcnt = SY_THR_STATIC },	/* 594 = inotify_rm_watch */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 595 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 596 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 597 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 598 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 599 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 600 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 601 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 602 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 603 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 604 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 605 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 606 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 607 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 608 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 609 = nosys */
	{ .sy_narg = AS(_kernelrpc_mach_vm_allocate_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 610 = _kernelrpc_mach_vm_allocate_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 611 = nosys */
	{ .sy_narg = AS(_kernelrpc_mach_vm_deallocate_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 612 = _kernelrpc_mach_vm_deallocate_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 613 = nosys */
	{ .sy_narg = AS(_kernelrpc_mach_vm_protect_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 614 = _kernelrpc_mach_vm_protect_trap */
	{ .sy_narg = AS(_kernelrpc_mach_vm_map_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 615 = _kernelrpc_mach_vm_map_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_allocate_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 616 = _kernelrpc_mach_port_allocate_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_destroy_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 617 = _kernelrpc_mach_port_destroy_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_deallocate_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 618 = _kernelrpc_mach_port_deallocate_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_mod_refs_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 619 = _kernelrpc_mach_port_mod_refs_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_move_member_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 620 = _kernelrpc_mach_port_move_member_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_insert_right_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 621 = _kernelrpc_mach_port_insert_right_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_insert_member_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 622 = _kernelrpc_mach_port_insert_member_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_extract_member_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 623 = _kernelrpc_mach_port_extract_member_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_construct_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 624 = _kernelrpc_mach_port_construct_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_destruct_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 625 = _kernelrpc_mach_port_destruct_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 626 = mach_reply_port */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 627 = thread_self_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 628 = task_self_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 629 = host_self_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 630 = nosys */
	{ .sy_narg = AS(mach_msg_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 631 = mach_msg_trap */
	{ .sy_narg = AS(mach_msg_overwrite_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 632 = mach_msg_overwrite_trap */
	{ .sy_narg = AS(semaphore_signal_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 633 = semaphore_signal_trap */
	{ .sy_narg = AS(semaphore_signal_all_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 634 = semaphore_signal_all_trap */
	{ .sy_narg = AS(semaphore_signal_thread_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 635 = semaphore_signal_thread_trap */
	{ .sy_narg = AS(semaphore_wait_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 636 = semaphore_wait_trap */
	{ .sy_narg = AS(semaphore_wait_signal_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 637 = semaphore_wait_signal_trap */
	{ .sy_narg = AS(semaphore_timedwait_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 638 = semaphore_timedwait_trap */
	{ .sy_narg = AS(semaphore_timedwait_signal_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 639 = semaphore_timedwait_signal_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 640 = nosys */
	{ .sy_narg = AS(_kernelrpc_mach_port_guard_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 641 = _kernelrpc_mach_port_guard_trap */
	{ .sy_narg = AS(_kernelrpc_mach_port_unguard_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 642 = _kernelrpc_mach_port_unguard_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 643 = obsolete  */
	{ .sy_narg = AS(task_name_for_pid_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 644 = task_name_for_pid */
	{ .sy_narg = AS(task_for_pid_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 645 = task_for_pid */
	{ .sy_narg = AS(pid_for_task_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 646 = pid_for_task */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 647 = nosys */
	{ .sy_narg = AS(macx_swapon_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 648 = macx_swapon */
	{ .sy_narg = AS(macx_swapoff_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 649 = macx_swapoff */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 650 = nosys */
	{ .sy_narg = AS(macx_triggers_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 651 = macx_triggers */
	{ .sy_narg = AS(macx_backing_store_suspend_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 652 = macx_backing_store_suspend */
	{ .sy_narg = AS(macx_backing_store_recovery_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 653 = macx_backing_store_recovery */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 654 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 655 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 656 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 657 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 658 = nosys */
	{ .sy_narg = AS(swtch_pri_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 659 = swtch_pri */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 660 = swtch */
	{ .sy_narg = AS(thread_switch_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 661 = thread_switch */
	{ .sy_narg = AS(clock_sleep_trap_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 662 = clock_sleep_trap */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 663 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 664 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 665 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 666 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 667 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 668 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 669 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 670 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 671 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 672 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 673 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 674 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 675 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 676 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 677 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 678 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 679 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 680 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 681 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 682 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 683 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 684 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 685 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 686 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 687 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 688 = nosys */
	{ .sy_narg = AS(mach_timebase_info_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 689 = mach_timebase_info */
	{ .sy_narg = AS(mach_wait_until_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 690 = mach_wait_until */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 691 = mk_timer_create */
	{ .sy_narg = AS(mk_timer_destroy_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 692 = mk_timer_destroy */
	{ .sy_narg = AS(mk_timer_arm_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 693 = mk_timer_arm */
	{ .sy_narg = AS(mk_timer_cancel_args), .sy_call = (sy_call_t *)lkmressys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 694 = mk_timer_cancel */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 695 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 696 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 697 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 698 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 699 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 700 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 701 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 702 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 703 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 704 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 705 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 706 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 707 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 708 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 709 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 710 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 711 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 712 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 713 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 714 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 715 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 716 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 717 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 718 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 719 = nosys */
	{ .sy_narg = 0, .sy_call = (sy_call_t *)nosys, .sy_auevent = AUE_NULL, .sy_flags = 0, .sy_thrcnt = SY_THR_ABSENT },	/* 720 = nosys */
};
