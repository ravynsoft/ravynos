/*    vmsish.h
 *
 *    VMS-specific C header file for perl5.
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007 by Charles Bailey and others.
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 *    Please see Changes*.* or the Perl Repository Browser for revision history.
 */

#ifndef __vmsish_h_included
#define __vmsish_h_included

#include <descrip.h> /* for dirent struct definitions */
#include <libdef.h>  /* status codes for various places */
#include <rmsdef.h>  /* at which errno and vaxc$errno are */
#include <ssdef.h>   /* explicitly set in the perl source code */
#include <stsdef.h>  /* bitmasks for exit status testing */

/* Suppress compiler warnings from DECC for VMS-specific extensions:
 * ADDRCONSTEXT,NEEDCONSTEXT: initialization of data with non-constant values
 *                            (e.g. pointer fields of descriptors)
 */
#ifdef __DECC
#  pragma message disable (ADDRCONSTEXT,NEEDCONSTEXT)
#endif
#ifdef __DECCXX
#  pragma message informational (INTSIGNCHANGE,CASTQUALTYP,ASSCOMMEA,NOCTOBUTCONREFM,MISSINGRETURN)
#endif

/* DEC's C compilers and gcc use incompatible definitions of _to(upp|low)er() */
#ifdef _toupper
#  undef _toupper
#endif
#define _toupper(c) (((c) < 'a' || (c) > 'z') ? (c) : (c) & ~040)
#ifdef _tolower
#  undef _tolower
#endif
#define _tolower(c) (((c) < 'A' || (c) > 'Z') ? (c) : (c) | 040)

/* Assorted things to look like Unix */
#include <processes.h> /* for vfork() */
#include <unixio.h>
#include <unixlib.h>
#include <file.h>  /* it's not <sys/file.h>, so don't use I_SYS_FILE */
#include <unistd.h>

#ifdef NO_PERL_TYPEDEFS /* a2p; we don't want Perl's special routines */
#  define DONT_MASK_RTL_CALLS
#endif

#include <namdef.h>

/* Set the maximum filespec size here as it is larger for EFS file
 * specifications.
 */
#ifndef VMS_MAXRSS
#ifdef NAML$C_MAXRSS
#define VMS_MAXRSS (NAML$C_MAXRSS+1)
#ifndef VMS_LONGNAME_SUPPORT
#define VMS_LONGNAME_SUPPORT 1
#endif /* VMS_LONGNAME_SUPPORT */
#endif /* NAML$C_MAXRSS */
#endif /* VMS_MAXRSS */

#ifndef VMS_MAXRSS
#define VMS_MAXRSS (NAM$C_MAXRSS + 1)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN (VMS_MAXRSS - 1)
#endif


/* Note that we do, in fact, have this */
#define HAS_GETENV_SV
#define HAS_GETENV_LEN


#ifndef PERL_FOR_X2P

#ifndef DONT_MASK_RTL_CALLS
#  ifdef getenv
#    undef getenv
#  endif
  /* getenv used for regular logical names */
#  define getenv(v) Perl_my_getenv(aTHX_ v,TRUE)
#endif
#ifdef getenv_len
#  undef getenv_len
#endif
#define getenv_len(v,l) Perl_my_getenv_len(aTHX_ v,l,TRUE)

/* DECC introduces this routine in the RTL as of VMS 7.0; for now,
 * we'll use ours, since it gives us the full VMS exit status. */
#define waitpid my_waitpid

/* Our own contribution to PerlShr's global symbols . . . */

#if !defined(MULTIPLICITY)
#define opendir			Perl_opendir
#define rename			Perl_rename
#define seekdir			Perl_seekdir
#define readdir			Perl_readdir
#define readdir_r		Perl_readdir_r
#else
#define opendir(a)		Perl_opendir(aTHX_ a)
#define rename(a,b)		Perl_rename(aTHX_ a,b)
#define seekdir(a,b)		Perl_seekdir(aTHX_ a,b)
#define readdir(a)		Perl_readdir(aTHX_ a)
#define readdir_r(a,b,c)	Perl_readdir_r(aTHX_ a,b,c)
#endif
#define closedir(a)		Perl_closedir(a)
#define telldir(a)		Perl_telldir(a)
#define vmsreaddirversions(a,b)	Perl_vmsreaddirversions(a,b)

#define cando_by_name(a,b,c)		Perl_cando_by_name(aTHX_ a,b,c)
#define do_rmdir(a)			Perl_do_rmdir(aTHX_ a)
#define fileify_dirspec(a,b)		Perl_fileify_dirspec(aTHX_ a,b)
#define fileify_dirspec(a,b)		Perl_fileify_dirspec(aTHX_ a,b)
#define fileify_dirspec_ts(a,b)		Perl_fileify_dirspec_ts(aTHX_ a,b)
#define fileify_dirspec_ts(a,b)		Perl_fileify_dirspec_ts(aTHX_ a,b)
#define fileify_dirspec_utf8(a,b,c)	Perl_fileify_dirspec(aTHX_ a,b,utf8)
#define fileify_dirspec_utf8_ts(a,b,c)	Perl_fileify_dirspec_ts(aTHX_ a,b,utf8)
#define flex_fstat(a,b)			Perl_flex_fstat(aTHX_ a,b)
#define flex_lstat(a,b)			Perl_flex_lstat(aTHX_ a,b)
#define flex_stat(a,b)			Perl_flex_stat(aTHX_ a,b)
#define init_os_extras			Perl_init_os_extras
#define kill_file(a)			Perl_kill_file(aTHX_ a)
#define my_chdir(a)			Perl_my_chdir(aTHX_ a)
#define my_chmod(a,b)			Perl_my_chmod(aTHX_ a,b)
#define my_crypt(a,b)			Perl_my_crypt(aTHX_ a,b)
#define my_endpwent()			Perl_my_endpwent(aTHX)
#define my_fclose(a)			Perl_my_fclose(a)
#define my_fdopen(a,b)			Perl_my_fdopen(a,b)
#define my_flush(a)			Perl_my_flush(aTHX_ a)
#define my_fwrite(a,b,c,d)		Perl_my_fwrite(a,b,c,d)
#define my_fgetname(a,b)		Perl_my_fgetname(a,b)
#define my_gconvert(a,b,c,d)		Perl_my_gconvert(a,b,c,d)
#define my_getenv(a,b)			Perl_my_getenv(aTHX_ a,b)
#define my_getenv_len(a,b,c)		Perl_my_getenv_len(aTHX_ a,b,c)
#define my_getpwent()			Perl_my_getpwent(aTHX)
#define my_getpwnam(a)			Perl_my_getpwnam(aTHX_ a)
#define my_getpwuid(a)			Perl_my_getpwuid(aTHX_ a)
#define my_gmtime(a)			Perl_my_gmtime(aTHX_ a)
#define my_localtime(a)			Perl_my_localtime(aTHX_ a)
#define my_mkdir(a,b)			Perl_my_mkdir(aTHX_ a,b)
#ifdef HAS_SYMLINK
#  define my_symlink(a,b)		Perl_my_symlink(aTHX_ a,b)
#endif
#define my_time(a)			Perl_my_time(aTHX_ a)
#define my_tmpfile			Perl_my_tmpfile
#define my_trnlnm(a,b,c)		Perl_my_trnlnm(aTHX_ a,b,c)
#define my_utime(a,b)			Perl_my_utime(aTHX_ a,b)
#define my_vfork			Perl_my_vfork
#define my_waitpid(a,b,c)		Perl_my_waitpid(aTHX_ a,b,c)
#define pathify_dirspec(a,b)		Perl_pathify_dirspec(aTHX a,b)
#define pathify_dirspec_ts(a,b)		Perl_pathify_dirspec_ts(aTHX a,b)
#define pathify_dirspec_utf8(a,b,c)	Perl_pathify_dirspec_utf8(aTHX a,b,c)
#define pathify_dirspec_utf8_ts(a,b,c)	Perl_pathify_dirspec_utf8_ts(aTHX a,b,c)
#define prime_env_iter			Perl_prime_env_iter
#define rmscopy(a,b,c)			Perl_rmscopy(aTHX_ a,b,c)
#define rmsexpand(a,b,c,d)		Perl_rmsexpand_utf8(aTHX_ a,b,c,d,NULL,NULL)
#define rmsexpand_ts(a,b,c,d)		Perl_rmsexpand_utf8_ts(aTHX_ a,b,c,d,NULL,NULL)
#define rmsexpand_utf8(a,b,c,d,e,f)	Perl_rmsexpand_utf8(aTHX_ a,b,c,d,e,f)
#define rmsexpand_utf8_ts(a,b,c,d,e,f)	Perl_rmsexpand_utf8_ts(aTHX_ a,b,c,d,e,f)
#define tounixpath(a,b)			Perl_tounixpath_utf8(aTHX_ a,b,NULL)
#define tounixpath_ts(a,b)		Perl_tounixpath_utf8_ts(aTHX_ a,b,NULL)
#define tounixpath_utf8(a,b,c)		Perl_tounixpath_utf8(aTHX_ a,b,c)
#define tounixpath_utf8_ts(a,b,c)	Perl_tounixpath_utf8_ts(aTHX_ a,b,c)
#define tounixspec(a,b)			Perl_tounixspec_utf8(aTHX_ a,b,NULL)
#define tounixspec_ts(a,b)		Perl_tounixspec_utf8_ts(aTHX_ a,b,NULL)
#define tounixspec_utf8(a,b,c)		Perl_tounixspec_utf8(aTHX_ a,b,c)
#define tounixspec_utf8_ts(a,b,c)	Perl_tounixspec_utf8_ts(aTHX_ a,b,c)
#define tovmspath(a,b)			Perl_tovmspath_utf8(aTHX_ a,b,NULL)
#define tovmspath_ts(a,b)		Perl_tovmspath_utf8_ts(aTHX_ a,b,NULL)
#define tovmspath_utf8(a,b,c)		Perl_tovmspath_utf8(aTHX_ a,b,c)
#define tovmspath_utf8_ts(a,b,c)	Perl_tovmspath_utf8_ts(aTHX_ a,b,c)
#define tovmsspec(a,b)			Perl_tovmsspec_utf8(aTHX_ a,b,NULL)
#define tovmsspec_ts(a,b)		Perl_tovmsspec_utf8_ts(aTHX_ a,b)
#define tovmsspec_utf8(a,b,c)		Perl_tovmsspec_utf8(aTHX_ a,b,c)
#define tovmsspec_utf8_ts(a,b,c)	Perl_tovmsspec_utf8_ts(aTHX_ a,b,c)
#define trim_unixpath(a,b,c)		Perl_trim_unixpath(aTHX_ a,b,c)
#define vms_do_aexec(a,b,c)		Perl_vms_do_aexec(aTHX_ a,b,c)
#define vms_do_exec(a)			Perl_vms_do_exec(aTHX_ a)
#define vms_case_tolerant(a)		Perl_vms_case_tolerant(a)
#define vms_image_init(a,b)		Perl_vms_image_init(a,b)
#define vms_realname(a,b,c)		Perl_vms_realname(aTHX_ a,b,c)
#define vms_realpath(a,b,c)		Perl_vms_realpath(aTHX_ a,b,c)
#define vmssetenv(a,b,c)		Perl_vmssetenv(aTHX_ a,b,c)
#define vmstrnenv(a,b,c,d,e)		Perl_vmstrnenv(a,b,c,d,e)
#define vmssetuserlnm(a,b)		Perl_vmssetuserlnm(a,b)

/* Delete if at all possible, changing protections if necessary. */
#define unlink(a) kill_file(a)

/* 
 * Intercept calls to fork, so we know whether subsequent calls to
 * exec should be handled in VMSish or Unixish style.
 */
#define fork my_vfork
#ifndef DONT_MASK_RTL_CALLS     /* #defined in vms.c so we see real vfork */
#  ifdef vfork
#    undef vfork
#  endif
#  define vfork my_vfork
#endif

/*
 * Toss in a shim to tmpfile which creates a plain temp file if the
 * RMS tmp mechanism won't work (e.g. if someone is relying on ACLs
 * from a specific directory to permit creation of files).
 */
#ifndef DONT_MASK_RTL_CALLS
#  define tmpfile Perl_my_tmpfile
#endif
#endif


/* BIG_TIME:
 *	This symbol is defined if Time_t is an unsigned type on this system.
 */
#define BIG_TIME

/* ACME_MESS:
 *	This symbol, if defined, indicates that error messages should be 
 *	should be generated in a format that allows the use of the Acme
 *	GUI/editor's autofind feature.
 */
#undef ACME_MESS	/**/

/* ALTERNATE_SHEBANG:
 *	This symbol, if defined, contains a "magic" string which may be used
 *	as the first line of a Perl program designed to be executed directly
 *	by name, instead of the standard Unix #!.  If ALTERNATE_SHEBANG
 *	begins with a character other then #, then Perl will only treat
 *	it as a command line if if finds the string "perl" in the first
 *	word; otherwise it's treated as the first line of code in the script.
 *	(IOW, Perl won't hand off to another interpreter via an alternate
 *	shebang sequence that might be legal Perl code.)
 */
#define ALTERNATE_SHEBANG "$"

/* Macros to set errno.  */
#define set_errno(v)      (errno = (v))
#define set_vaxc_errno(v) (vaxc$errno = (v))

/* Support for 'vmsish' behaviors enabled with C<use vmsish> pragma */

#define COMPLEX_STATUS	1	/* We track both "POSIX" and VMS values */

#define HINT_M_VMSISH_STATUS	0x40000000 /* system, $? return VMS status */
#define HINT_M_VMSISH_TIME	0x80000000 /* times are local, not UTC */

#ifdef MULTIPLICITY
#  define TEST_VMSISH(h)	(my_perl && PL_curcop && (PL_curcop->cop_hints & (h)))
#else
#  define TEST_VMSISH(h)	(PL_curcop && (PL_curcop->cop_hints & (h)))
#endif
#define VMSISH_STATUS	TEST_VMSISH(HINT_M_VMSISH_STATUS)
#define VMSISH_TIME	TEST_VMSISH(HINT_M_VMSISH_TIME)

/* VMS-specific data storage */

#define HAVE_INTERP_INTERN
struct interp_intern {
    int    hushed;
    int	   posix_exit;
    double inv_rand_max;
};
#define VMSISH_HUSHED     (PL_sys_intern.hushed)
#define MY_INV_RAND_MAX   (PL_sys_intern.inv_rand_max)
#define MY_POSIX_EXIT	(PL_sys_intern.posix_exit)

/* Flags for vmstrnenv() */
#define PERL__TRNENV_SECURE 0x01
#define PERL__TRNENV_JOIN_SEARCHLIST 0x02

/* Handy way to vet calls to VMS system services and RTL routines. */
#define _ckvmssts(call) STMT_START { unsigned long int __ckvms_sts; \
  if (!((__ckvms_sts=(call))&1)) { \
  set_errno(EVMSERR); set_vaxc_errno(__ckvms_sts); \
  Perl_croak(aTHX_ "Fatal VMS error (status=%d) at %s, line %d", \
  __ckvms_sts,__FILE__,__LINE__); } } STMT_END

/* Same thing, but don't call back to Perl's croak(); useful for errors
 * occurring during startup, before Perl's state is initialized */
#define _ckvmssts_noperl(call) STMT_START { unsigned long int __ckvms_sts; \
  if (!((__ckvms_sts=(call))&1)) { \
  set_errno(EVMSERR); set_vaxc_errno(__ckvms_sts); \
  (void)fprintf(stderr,"Fatal VMS error (status=%d) at %s, line %d", \
  __ckvms_sts,__FILE__,__LINE__); (void)lib$signal(__ckvms_sts); } } STMT_END

#ifdef VMS_DO_SOCKETS
#define PERL_SOCK_SYSREAD_IS_RECV
#define PERL_SOCK_SYSWRITE_IS_SEND
#endif

#define BIT_BUCKET "/dev/null"
#define PERL_SYS_INIT_BODY(c,v)	MALLOC_CHECK_TAINT2(*c,*v) vms_image_init((c),(v)); PERLIO_INIT; MALLOC_INIT
/* Use standard PERL_SYS_TERM_BODY */

#define dXSUB_SYS dNOOP
#define HAS_KILL
#define HAS_WAIT

#ifndef PERL_CORE
#  define PERL_FS_VER_FMT	"%d_%d_%d"
#endif
#define PERL_FS_VERSION		STRINGIFY(PERL_REVISION) "_" \
                                STRINGIFY(PERL_VERSION) "_" \
                                STRINGIFY(PERL_SUBVERSION)
/* Temporary; we need to add support for this to Configure.Com */
#ifdef PERL_INC_VERSION_LIST
#  undef PERL_INC_VERSION_LIST
#endif

/* VMS:
 *	This symbol, if defined, indicates that the program is running under
 *	VMS.  It's a symbol automagically defined by all VMS C compilers I've seen.
 * Just in case, however . . . */
/* Note that code really should be using __VMS to comply with ANSI */
#ifndef VMS
#define VMS		/**/
#endif

/* HAS_IOCTL:
 *	This symbol, if defined, indicates that the ioctl() routine is
 *	available to set I/O characteristics
 */
#define HAS_IOCTL               /**/
 
/* HAS_UTIME:
 *	This symbol, if defined, indicates that the routine utime() is
 *	available to update the access and modification times of files.
 */
#define HAS_UTIME		/**/

/* HAS_GROUP
 *	This symbol, if defined, indicates that the getgrnam() and
 *	getgrgid() routines are available to get group entries.
 *	The getgrent() has a separate definition, HAS_GETGRENT.
 */
#define HAS_GROUP		/**/

/* HAS_PASSWD
 *	This symbol, if defined, indicates that the getpwnam() and
 *	getpwuid() routines are available to get password entries.
 *	The getpwent() has a separate definition, HAS_GETPWENT.
 */
#define HAS_PASSWD		/**/

#define HAS_KILL
#define HAS_WAIT
  
/* USEMYBINMODE
 *	This symbol, if defined, indicates that the program should
 *	use the routine my_binmode(FILE *fp, char iotype, int mode) to insure
 *	that a file is in "binary" mode -- that is, that no translation
 *	of bytes occurs on read or write operations.
 */
#undef USEMYBINMODE

/* Stat_t:
 *	This symbol holds the type used to declare buffers for information
 *	returned by stat().  It's usually just struct stat.  It may be necessary
 *	to include <sys/stat.h> and <sys/types.h> to get any typedef'ed
 *	information.
 */
/* VMS:
 * We need this typedef to point to the new type even if DONT_MASK_RTL_CALLS
 * is in effect, since Perl's thread.h embeds one of these structs in its
 * thread data struct, and our struct mystat is a different size from the
 * regular struct stat (cf. note above about having to pad struct to work
 * around bug in compiler.)
 * It's OK to pass one of these to the RTL's stat(), though, since the
 * fields it fills are the same in each struct.
 */
#define Stat_t struct mystat

/* USE_STAT_RDEV:
*	This symbol is defined if this system has a stat structure declaring
*	st_rdev
*	VMS: Field exists in POSIXish version of struct stat(), but is not used.
*
*  No definition of what value an operating system or file system should
*  put in the st_rdev field has been found by me so far.  Examination of
*  LINUX source code indicates that the value is both very platform and
*  file system specific, with many filesystems just putting 1 or 0 in it.
*  J. Malmberg.
*/
#undef USE_STAT_RDEV		/**/

/*
 * fwrite1() should be a routine with the same calling sequence as fwrite(),
 * but which outputs all of the bytes requested as a single stream (unlike
 * fwrite() itself, which on some systems outputs several distinct records
 * if the number_of_items parameter is >1).
 */
#define fwrite1 my_fwrite


#ifndef DONT_MASK_RTL_CALLS
#  define fwrite my_fwrite     /* for PerlSIO_fwrite */
#  define fdopen my_fdopen
#  define fclose my_fclose
#  define fgetname(a, b) my_fgetname(a, b)
#ifdef HAS_SYMLINK
#  define symlink my_symlink
#endif
#endif


/* By default, flush data all the way to disk, not just to RMS buffers */
#define Fflush(fp) my_flush(fp)

/* Use our own rmdir() */
#ifndef DONT_MASK_RTL_CALLS
#define rmdir(name) do_rmdir(name)
#endif

/* Assorted fiddling with sigs . . . */
# include <signal.h>
#define ABORT() abort()

#ifdef I_UTIME
#include <utime.h>
#else
/* Used with our my_utime() routine in vms.c */
struct utimbuf {
  time_t actime;
  time_t modtime;
};
#endif
#ifndef DONT_MASK_RTL_CALLS
#define utime my_utime
#endif

/* tbuffer_t was replaced with struct tms in v7.0.  We no longer support
 * systems prior to v7.0, but there could be old XS code out there that
 * references tbuffer_t, so provide a compatibility macro.
 */

#define tbuffer_t struct tms

/* Substitute our own routines for gmtime(), localtime(), and time(),
 * which allow us to implement the vmsish 'time' pragma, and work
 * around absence of system-level UTC support on old versions of VMS.
 */
#define gmtime(t) my_gmtime(t)
#define localtime(t) my_localtime(t)
#define time(t) my_time(t)

/*
 * The C RTL's sigaction fails to check for invalid signal numbers so we 
 * help it out a bit.
 */
#ifndef DONT_MASK_RTL_CALLS
#    define sigaction(a,b,c) Perl_my_sigaction(aTHX_ a,b,c)
#endif
#ifdef KILL_BY_SIGPRC
#  define kill  Perl_my_kill
#endif
# define killpg  Perl_my_killpg


/* VMS doesn't use a real sys_nerr, but we need this when scanning for error
 * messages in text strings . . .
 */

#define sys_nerr EVMSERR  /* EVMSERR is as high as we can go. */

/* Look up new %ENV values on the fly */
#define DYNAMIC_ENV_FETCH 1
  /* Special getenv function for retrieving %ENV elements. */
#define ENVgetenv(v) my_getenv(v,FALSE)
#define ENVgetenv_len(v,l) my_getenv_len(v,l,FALSE)


/* Ditto for sys$hash_password() . . . */
#define crypt(a,b)  Perl_my_crypt(aTHX_ a,b)

/* Tweak arg to mkdir & chdir first, so we can tolerate trailing /. */
#define Mkdir(dir,mode) Perl_my_mkdir(aTHX_ (dir),(mode))
#define Chdir(dir) my_chdir((dir))
#ifndef DONT_MASK_RTL_CALLS
#define chmod(file_spec, mode) my_chmod((file_spec), (mode))
#endif

/* Use our own stat() clones, which handle Unix-style directory names */
#define Stat(name,bufptr) flex_stat(name,bufptr)
#define Fstat(fd,bufptr) Perl_flex_fstat(aTHX_ fd,bufptr)
#ifndef DONT_MASK_RTL_CALLS
#define lstat(name, bufptr) flex_lstat(name, bufptr)
#endif

/* Setup for the dirent routines:
 * opendir(), closedir(), readdir(), seekdir(), telldir(), and
 * vmsreaddirversions(), and preprocessor stuff on which these depend:
 *    Written by Rich $alz, <rsalz@bbn.com> in August, 1990.
 *
 */

/* Flags for the _dirdesc structure */
#define PERL_VMSDIR_M_VERSIONS		0x02 /* Want VMS versions */
#define PERL_VMSDIR_M_UNIXSPECS		0x04 /* Want UNIX specifications */


    /* Data structure returned by READDIR(). */
struct dirent {
    char	d_name[256];		/* File name		*/
    int		d_namlen;		/* Length of d_name */
    int		vms_verscount;		/* Number of versions	*/
    int		vms_versions[20];	/* Version numbers	*/
};

    /* Handle returned by opendir(), used by the other routines.  You
     * are not supposed to care what's inside this structure. */
typedef struct _dirdesc {
    long			context;
    int				flags;
    unsigned long int           count;
    char			*pattern;
    struct dirent		entry;
    struct dsc$descriptor_s	pat;
    void			*mutex;
} DIR;


#define rewinddir(dirp)		seekdir((dirp), 0)

/* used for our emulation of getpw* */
struct passwd {
        char    *pw_name;    /* Username */
        char    *pw_passwd;
        Uid_t   pw_uid;      /* UIC member number */
        Gid_t   pw_gid;      /* UIC group  number */
        char    *pw_comment; /* Default device/directory (Unix-style) */
        char    *pw_gecos;   /* Owner */
        char    *pw_dir;     /* Default device/directory (VMS-style) */
        char    *pw_shell;   /* Default CLI name (eg. DCL) */
};
#define pw_unixdir pw_comment  /* Default device/directory (Unix-style) */
#define getpwnam my_getpwnam
#define getpwuid my_getpwuid
#define getpwent my_getpwent
#define endpwent my_endpwent
#define setpwent my_endpwent

/* Our own stat_t substitute, since we play with st_dev and st_ino -
 * we want atomic types so Unix-bound code which compares these fields
 * for two files will work most of the time under VMS.
 * N.B. 1. The st_ino hack assumes that sizeof(unsigned short[3]) ==
 * sizeof(unsigned) + sizeof(unsigned short).  We can't use a union type
 * to map the unsigned int we want and the unsigned short[3] the CRTL
 * returns into the same member, since gcc has different ideas than DECC
 * and VAXC about sizing union types.
 * N.B. 2. The routine cando() in vms.c assumes that &stat.st_ino is the
 * address of a FID.
 */
/* First, grab the system types, so we don't clobber them later */
#include <stat.h>
/* Since we've got to match the size of the CRTL's stat_t, we need
 * to mimic DECC's alignment settings.
 *
 * The simplest thing is to just put a wrapper around the stat structure
 * supplied by the CRTL and use #defines to redirect references to the
 * members to the real names.
 */

#if defined(__DECC) || defined(__DECCXX)
#  pragma __member_alignment __save
#  pragma member_alignment
#endif

typedef unsigned mydev_t;
#if !defined(_USE_STD_STAT) && !defined(_LARGEFILE)
typedef unsigned myino_t;
#else
typedef __ino64_t myino_t;
#endif

struct mystat
{
    struct stat crtl_stat;
    myino_t st_ino;
#if !defined(_USE_STD_STAT) && !defined(_LARGEFILE)
    unsigned rvn; /* FID (num,seq,rvn) + pad */
#endif
    mydev_t st_dev;
    char st_devnam[256]; /* Cache the (short) VMS name */
};

#define st_mode crtl_stat.st_mode
#define st_nlink crtl_stat.st_nlink
#define st_uid crtl_stat.st_uid
#define st_gid crtl_stat.st_gid
#define st_rdev crtl_stat.st_rdev
#define st_size crtl_stat.st_size
#define st_atime crtl_stat.st_atime
#define st_mtime crtl_stat.st_mtime
#define st_ctime crtl_stat.st_ctime
#define st_fab_rfm crtl_stat.st_fab_rfm
#define st_fab_rat crtl_stat.st_fab_rat
#define st_fab_fsz crtl_stat.st_fab_fsz
#define st_fab_mrs crtl_stat.st_fab_mrs

#if defined(_USE_STD_STAT) || defined(_LARGEFILE)
#define VMS_INO_T_COMPARE(__a, __b) (__a != __b)
#define VMS_INO_T_COPY(__a, __b) __a = __b
#else
#define VMS_INO_T_COMPARE(__a, __b) memcmp(&__a, &__b, 6)
#define VMS_INO_T_COPY(__a, __b) memcpy(&__a, &__b, 6)
#endif

#if defined(__DECC) || defined(__DECCXX)
#  pragma __member_alignment __restore
#endif

#ifndef DONT_MASK_RTL_CALLS  /* defined for vms.c so we can see RTL calls */
#  ifdef stat
#    undef stat
#  endif
#  define stat mystat
#  define dev_t mydev_t
#  define ino_t myino_t
#endif
/* Cons up a 'delete' bit for testing access */
#define S_IDUSR (S_IWUSR | S_IXUSR)
#define S_IDGRP (S_IWGRP | S_IXGRP)
#define S_IDOTH (S_IWOTH | S_IXOTH)


#ifndef PERL_FOR_X2P
/* Prototypes for functions unique to vms.c.  Don't include replacements
 * for routines in the mainline source files excluded by #ifndef VMS;
 * their prototypes are already in proto.h.
 */

#ifdef __cplusplus
extern "C" {
#endif

void	prime_env_iter (void);
void	init_os_extras (void);
int	Perl_vms_status_to_unix(int vms_status, int child_flag);
int	Perl_unix_status_to_vms(int unix_status);
int	Perl_vmstrnenv (const char *, char *, unsigned long int, struct dsc$descriptor_s **, unsigned long int);
char *	Perl_vms_realpath (pTHX_ const char *, char *, int *);
char *	Perl_my_getenv (pTHX_ const char *, bool);
int	Perl_my_trnlnm (pTHX_ const char *, char *, unsigned long int);
char *	Perl_tounixspec (pTHX_ const char *, char *);
char *	Perl_tounixspec_ts (pTHX_ const char *, char *);
char *	Perl_tounixspec_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_tounixspec_utf8_ts (pTHX_ const char *, char *, int *);
char *	Perl_tovmsspec (pTHX_ const char *, char *);
char *	Perl_tovmsspec_ts (pTHX_ const char *, char *);
char *	Perl_tovmsspec_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_tovmsspec_utf8_ts (pTHX_ const char *, char *, int *);
char *	Perl_tounixpath (pTHX_ const char *, char *);
char *	Perl_tounixpath_ts (pTHX_ const char *, char *);
char *	Perl_tounixpath_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_tounixpath_utf8_ts (pTHX_ const char *, char *, int *);
char *	Perl_tovmspath (pTHX_ const char *, char *);
char *	Perl_tovmspath_ts (pTHX_ const char *, char *);
char *	Perl_tovmspath_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_tovmspath_utf8_ts (pTHX_ const char *, char *, int *);
int	Perl_do_rmdir (pTHX_ const char *);
char *	Perl_fileify_dirspec (pTHX_ const char *, char *);
char *	Perl_fileify_dirspec_ts (pTHX_ const char *, char *);
char *	Perl_fileify_dirspec_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_fileify_dirspec_utf8_ts (pTHX_ const char *, char *, int *);
char *	Perl_pathify_dirspec (pTHX_ const char *, char *);
char *	Perl_pathify_dirspec_ts (pTHX_ const char *, char *);
char *	Perl_pathify_dirspec_utf8 (pTHX_ const char *, char *, int *);
char *	Perl_pathify_dirspec_utf8_ts (pTHX_ const char *, char *, int *);
char *	Perl_rmsexpand (pTHX_ const char *, char *, const char *, unsigned);
char *	Perl_rmsexpand_ts (pTHX_ const char *, char *, const char *, unsigned);
char *	Perl_rmsexpand_utf8 (pTHX_ const char *, char *, const char *, unsigned, int *, int *);
char *	Perl_rmsexpand_utf8_ts (pTHX_ const char *, char *, const char *, unsigned, int *, int *);
int	Perl_trim_unixpath (pTHX_ char *, const char*, int);
DIR * Perl_opendir (pTHX_ const char *);
int	Perl_rename (pTHX_ const char *, const char *);
int	Perl_rmscopy (pTHX_ const char *, const char *, int);
int	Perl_my_mkdir (pTHX_ const char *, Mode_t);
bool	Perl_vms_do_aexec (pTHX_ SV *, SV **, SV **);
int	Perl_vms_case_tolerant(void);
char *	Perl_my_getenv_len (pTHX_ const char *, unsigned long *, bool);
int	Perl_vmssetenv (pTHX_ const char *, const char *, struct dsc$descriptor_s **);
void	Perl_vmssetuserlnm(const char *name, const char *eqv);
char *	Perl_my_crypt (pTHX_ const char *, const char *);
Pid_t	Perl_my_waitpid (pTHX_ Pid_t, int *, int);
char *	my_gconvert (double, int, int, char *);
int	Perl_kill_file (pTHX_ const char *);
int	Perl_my_chdir (pTHX_ const char *);
int	Perl_my_chmod(pTHX_ const char *, mode_t);
FILE *	Perl_my_tmpfile (void);
int	Perl_my_sigaction (pTHX_ int, const struct sigaction*, struct sigaction*);
#ifdef KILL_BY_SIGPRC
unsigned int	Perl_sig_to_vmscondition (int);
int	Perl_my_kill (int, int);
int	Perl_my_killpg (int, int);
void	Perl_csighandler_init (void);
#endif
int	Perl_my_utime (pTHX_ const char *, const struct utimbuf *);
void	Perl_vms_image_init (int *, char ***);
struct dirent *	Perl_readdir (pTHX_ DIR *);
int	Perl_readdir_r(pTHX_ DIR *, struct dirent *, struct dirent **);
long	Perl_telldir (DIR *);
void	Perl_seekdir (pTHX_ DIR *, long);
void	Perl_closedir (DIR *);
void	vmsreaddirversions (DIR *, int);
struct tm *	Perl_my_gmtime (pTHX_ const time_t *);
struct tm *	Perl_my_localtime (pTHX_ const time_t *);
time_t	Perl_my_time (pTHX_ time_t *);
I32	Perl_cando_by_name (pTHX_ I32, bool, const char *);
int	Perl_flex_fstat (pTHX_ int, Stat_t *);
int	Perl_flex_lstat (pTHX_ const char *, Stat_t *);
int	Perl_flex_stat (pTHX_ const char *, Stat_t *);
int	my_vfork (void);
bool	Perl_vms_do_exec (pTHX_ const char *);
FILE *  my_fdopen (int, const char *);
int     my_fclose (FILE *);
int     my_fwrite (const void *, size_t, size_t, FILE *);
char *  Perl_my_fgetname (FILE *fp, char *buf);
#ifdef HAS_SYMLINK
int     Perl_my_symlink(pTHX_ const char *path1, const char *path2);
#endif
int	Perl_my_flush (pTHX_ FILE *);
struct passwd *	Perl_my_getpwnam (pTHX_ const char *name);
struct passwd *	Perl_my_getpwuid (pTHX_ Uid_t uid);
void	Perl_my_endpwent (pTHX);

/*
 * The following prototypes are in math.h but for some reason they
 * are ifdefed out for C++.  So we have to repeat them here in order
 * to build the POSIX extension.
 */

#ifdef __DECCXX

double exp2(double __x);
double fdim(double __x, double __y);
double fma(double __x, double __y, double __z);
double fmax(double __x, double __y);
double fmin(double __x, double __y);
double nexttoward(double __x, long double __y);
double remainder(double __x, double __y);
double remquo(double __x, double __y, int *__quo);
double tgamma(double __x);
float exp2f(float __x);
float fdimf(float __x, float __y);
float fmaf(float __x, float __y, float __z);
float fmaxf(float __x, float __y);
float fminf(float __x, float __y);
float nexttowardf(float __x, long double __y);
float remainderf(float __x, float __y);
float remquof(float __x, float __y, int *__quo);
float tgammaf(float __x);
long double exp2l(long double __x);
long double fdiml(long double __x, long double __y);
long double fmal(long double __x, long double __y, long double __z);
long double fmaxl(long double __x, long double __y);
long double fminl(long double __x, long double __y);
long double nexttowardl(long double __x, long double __y);
long double remainderl(long double __x, long double __y);
long double remquol(long double __x, long double __y, int *__quo);
long double tgammal(long double __x);
int ilogb(double __x);
int ilogbf(float __x);
int ilogbl(long double __x);
long int lrint(double __x);
long int lrintf(float __x);
long int lrintl(long double __x);
long int lround(double __x);
long int lroundf(float __x);
long int lroundl(long double __x);

#endif


#ifdef __cplusplus
}
#endif

#endif

#ifndef VMS_DO_SOCKETS
/* This relies on tricks in perl.h to pick up that these manifest constants
 * are undefined and set up conversion routines.  It will then redefine
 * these manifest constants, so the actual values will match config.h
 */
#undef HAS_HTONS
#undef HAS_NTOHS
#undef HAS_HTONL
#undef HAS_NTOHL
#endif

/* The C RTL manual says to undef the macro for DEC C 5.2 and lower. */
#if defined(fileno) && defined(__DECC_VER) && __DECC_VER < 50300000
#  undef fileno 
#endif 

#define NO_ENVIRON_ARRAY

/* RMSEXPAND options */
#define PERL_RMSEXPAND_M_VMS		0x02 /* Force output to VMS format */
#define PERL_RMSEXPAND_M_LONG		0x04 /* Expand to long name format */
#define PERL_RMSEXPAND_M_VMS_IN		0x08 /* Assume input is VMS already */
#define PERL_RMSEXPAND_M_SYMLINK	0x20 /* Use symbolic link, not target */

/* With long doubles, NaN == NaN, which it shouldn't. */
#ifdef USE_LONG_DOUBLE
#  define NAN_COMPARE_BROKEN 1
#endif
#endif  /* __vmsish_h_included */
