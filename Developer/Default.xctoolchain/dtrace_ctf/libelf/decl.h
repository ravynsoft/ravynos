/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/


/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_DECL_H
#define	_DECL_H

#define SEG_CTF "__CTF"
#define SECT_CTF "__ctf"

#include <pthread.h> /* In lieu of Solaris <thread.h> */
#define USYNC_THREAD IGNORED /* Take PTHREAD_PROCESS_PRIVATE default in pthread_*_init() */
#define DEFAULTMUTEX PTHREAD_MUTEX_INITIALIZER

typedef pthread_mutex_t mutex_t;
typedef pthread_rwlock_t rwlock_t;
typedef pthread_key_t thread_key_t;

#define RW_LOCK_HELD(x) 1 /* Only used in "assert" */
#define MUTEX_HELD(x) 1 /* Only used in "assert" */

#define rwlock_init(x,y,z) pthread_rwlock_init(x,((const pthread_rwlockattr_t *)NULL))
#define rw_rdlock(x) pthread_rwlock_rdlock(x)
#define rw_wrlock(x) pthread_rwlock_wrlock(x)
#define rw_unlock(x) pthread_rwlock_unlock(x)

#define mutex_init(x,y,z) pthread_mutex_init(x,((const pthread_mutexattr_t *)NULL))
#define mutex_lock(x) pthread_mutex_lock(x)
#define mutex_unlock(x) pthread_mutex_unlock(x)

#define NOTE(x) /* NOTHING */ /* In lieu of Solaris #include <note.h> */

#include "libelf.h"
#include "machelf.h" /* In lieu of Solaris <sys/machelf.h> */
#include "gelf.h"
#include "msg.h"

#include <unistd.h>
#define PAGESIZE getpagesize()

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct Dnode	Dnode;
typedef struct Snode32	Snode32;
typedef struct Snode64	Snode64;


/*
 * Data alignment
 *	An elf file is defined to have its structures aligned on
 *	appropriate boundaries.  The following type lets the
 *	library test whether the file's alignment meets its own
 *	constraints in memory.  This assumes every machine uses
 *	an alignment that is no greater than an object's size.
 *	The pointer isn't relevant for the file, but the code uses
 *	it to get memory alignment.  ANSI C void * holds any pointer,
 *	making it appropriate here.
 */

typedef union
{
	Elf32_Word	w;
	Elf32_Addr	a;
	Elf32_Off	o;
} Elf32;

typedef union {
	Elf64_Xword	x;
	Elf64_Word	w;
	Elf64_Addr	a;
	Elf64_Off	o;
	Elf_Void	*p;
} Elf64;


/*
 * Memory allocation
 *	Structures are obtained several ways: file mapping,
 *	malloc(), from the user.  A status bit in the structures
 *	tells whether an object was obtained with malloc() and
 *	therefore should be released with free().  The bits
 *	named ...ALLOC indicate this.
 */


/*
 * Data descriptor
 *	db_data must be first in the Dnode structure, because
 *	&db_data must == &Dnode.
 *
 *	db_buf is a pointer to an allocated buffer.  The same value
 *	goes into db_data.d_buf originally, but the user can touch
 *	it.  If the data buffer is not to be freed, db_buf is null.
 *
 *	When "reading" an input file's buffer, the data are left
 *	alone until needed.  When they've been converted to internal
 *	form, the READY flag is set.
 *
 *	db_raw points to a parallel raw buffer.  Raw buffers
 *	have null db_raw.
 */

struct	Dnode
{
	Elf_Data	db_data;
	Elf_Scn		*db_scn;	/* section parent */
	Dnode		*db_next;
	Dnode		*db_raw;	/* raw data */
	off_t		db_off;		/* orig file offset, 0 o/w */
	size_t		db_fsz;		/* orig file size, 0 o/w */
	size_t		db_shsz;	/* orig shdr size, 0 o/w */
	size_t		db_osz;		/* output size for update */
	Elf_Void	*db_buf;	/* allocated data buffer */
	unsigned	db_uflags;	/* user flags: ELF_F_... */
	unsigned	db_myflags;	/* internal flags: DBF_... */
	Elf64_Off	db_xoff;	/* extended offset for 32-bit Elf64 */
};

#define	DBF_ALLOC	0x1	/* applies to Dnode itself */
#define	DBF_READY	0x2	/* buffer ready */


/*
 * Section descriptor
 *	These are sometimes allocated in a block.  If the SF_ALLOC
 *	bit is set in the flags, the Scn address may be passed to free.
 *	The caller must first follow the s_next list to the next freeable
 *	node, because free can clobber the s_next value in the block.
 */

struct	Elf_Scn
{
	mutex_t		s_mutex;
	Elf_Scn		*s_next;	/* next section */
	Elf		*s_elf; 	/* parent file */
	Dnode		*s_hdnode;	/* head Dnode */
	Dnode		*s_tlnode;	/* tail Dnode */
	Elf_Void	*s_shdr;	/* Elf32 or Elf64 scn header */
	size_t		s_index;	/* section index */
	int		s_err;		/* for delaying data error */
	unsigned	s_shflags;	/* user shdr flags */
	unsigned	s_uflags;	/* user flags */
	unsigned	s_myflags;	/* SF_... */
	Dnode		s_dnode;	/* every scn needs one */
};

NOTE(MUTEX_PROTECTS_DATA(Elf_Scn::s_mutex, Elf_Scn Dnode Elf_Data))
NOTE(SCHEME_PROTECTS_DATA("Scn lock held", Elf_Data))
NOTE(SCHEME_PROTECTS_DATA("Scn lock held", Elf32_Shdr Elf32_Sym))
NOTE(READ_ONLY_DATA(Elf_Scn::s_elf))
NOTE(READ_ONLY_DATA(Dnode::db_scn))


/*
 * Designates whether or not we are in a threaded_app.
 */
extern int *_elf_libc_threaded;
#define	elf_threaded	(_elf_libc_threaded && *_elf_libc_threaded)

#ifdef	__lock_lint
#define	SCNLOCK(x)	(void) mutex_lock(&((Elf_Scn *)x)->s_mutex);
#else
#define	SCNLOCK(x) \
	if (elf_threaded) \
		(void) mutex_lock(&((Elf_Scn *)x)->s_mutex);
#endif

#ifdef	__lock_lint
#define	SCNUNLOCK(x)	(void) mutex_unlock(&((Elf_Scn *)x)->s_mutex);
#else
#define	SCNUNLOCK(x) \
	if (elf_threaded) \
		(void) mutex_unlock(&((Elf_Scn *)x)->s_mutex);
#endif

#ifdef	__lock_lint
#define	UPGRADELOCKS(e, s)\
		(void) mutex_unlock(&((Elf_Scn *)s)->s_mutex); \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock); \
		(void) rw_wrlock(&((Elf *)e)->ed_rwlock);
#else
#define	UPGRADELOCKS(e, s)\
	if (elf_threaded) { \
		(void) mutex_unlock(&((Elf_Scn *)s)->s_mutex); \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock); \
		(void) rw_wrlock(&((Elf *)e)->ed_rwlock); \
	}
#endif

#ifdef	__lock_lint
#define	DOWNGRADELOCKS(e, s)\
		(void) rw_unlock(&((Elf *)e)->ed_rwlock); \
		(void) rw_rdlock(&((Elf *)e)->ed_rwlock); \
		(void) mutex_lock(&((Elf_Scn *)s)->s_mutex);
#else
#define	DOWNGRADELOCKS(e, s)\
	if (elf_threaded) { \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock); \
		(void) rw_rdlock(&((Elf *)e)->ed_rwlock); \
		(void) mutex_lock(&((Elf_Scn *)s)->s_mutex); \
	}
#endif

#ifdef	__lock_lint
#define	READLOCKS(e, s) \
		(void) rw_rdlock(&((Elf *)e)->ed_rwlock); \
		(void) mutex_lock(&((Elf_Scn *)s)->s_mutex);
#else
#define	READLOCKS(e, s) \
	if (elf_threaded) { \
		(void) rw_rdlock(&((Elf *)e)->ed_rwlock); \
		(void) mutex_lock(&((Elf_Scn *)s)->s_mutex); \
	}
#endif

#ifdef	__lock_lint
#define	READUNLOCKS(e, s) \
		(void) mutex_unlock(&((Elf_Scn *)s)->s_mutex); \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock);
#else
#define	READUNLOCKS(e, s) \
	if (elf_threaded) { \
		(void) mutex_unlock(&((Elf_Scn *)s)->s_mutex); \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock); \
	}
#endif




#define	SF_ALLOC	0x1	/* applies to Scn */
#define	SF_READY	0x2	/* has section been cooked */


struct	Snode32
{
	Elf_Scn		sb_scn;		/* must be first */
	Elf32_Shdr	sb_shdr;
};

struct	Snode64
{
	Elf_Scn		sb_scn;		/* must be first */
	Elf64_Shdr	sb_shdr;
};


/*
 *	A file's status controls how the library can use file data.
 *	This is important to keep "raw" operations and "cooked"
 *	operations from interfering with each other.
 *
 *	A file's status is "fresh" until something touches it.
 *	If the first thing is a raw operation, we freeze the data
 *	and force all cooking operations to make a copy.  If the
 *	first operation cooks, raw operations use the file system.
 */

typedef enum
{
	ES_FRESH = 0,	/* unchanged */
	ES_COOKED,	/* translated */
	ES_FROZEN	/* raw, can't be translated */
} Status;


/*
 * Elf descriptor
 *	The major handle between user code and the library.
 *
 *	Descriptors can have parents: archive members reference
 *	the archive itself.  Relevant "offsets:"
 *
 *	ed_baseoff	The file offset, relative to zero, to the first
 *			byte in the file.  For all files, this gives
 *			the lseek(fd, ed_baseoff, 0) value.
 *
 *	ed_memoff	The offset from the beginning of the nesting file
 *			to the bytes of a member.  For an archive member,
 *			this is the offset from the beginning of the
 *			archive to the member bytes (not the hdr).  If an
 *			archive member slides, memoff changes.
 *
 *	Keeping these absolute and relative offsets allows nesting of
 *	files, including archives within archives, etc.  The only current
 *	nesting file is archive, but others might be supported.
 *
 *	ed_image	This is a pointer to the base memory image holding
 *			the file.  Library code assumes the image is aligned
 *			to a boundary appropriate for any object.  This must
 *			be true, because we get an image only from malloc
 *			or mmap, both of which guarantee alignment.
 */

struct Elf
{
	rwlock_t	ed_rwlock;
	Elf		*ed_parent;	/* archive parent */
	int		ed_activ;	/* activation count */
	int		ed_fd;		/* file descriptor */
	Status		ed_status;	/* file's memory status */
	off_t		ed_baseoff;	/* base file offset, zero based */
	char		*ed_image;	/* pointer to file image */
	size_t		ed_imagesz;	/* # bytes in ed_image */
	char		*ed_wrimage;	/* pointer to output image */
	size_t		ed_wrimagesz;	/* # bytes in ed_wrimagesz */
	char		*ed_ident;	/* file start, getident() bytes */
	size_t		ed_identsz;	/* # bytes for getident() */
	char		*ed_raw;	/* raw file ptr */
	size_t		ed_fsz;		/* file size */
	unsigned	*ed_vm;		/* virtual memory map */
	size_t		ed_vmsz;	/* # regions in vm */
	unsigned	ed_encode;	/* data encoding */
	unsigned	ed_version;	/* file version */
	int		ed_class;	/* file class */
	Elf_Kind	ed_kind;	/* file type */
	Elf_Void	*ed_ehdr;	/* Elf{32,64}_Ehdr elf header */
	Elf_Void	*ed_phdr;	/* Elf{32,64}_Phdr phdr table */
	size_t		ed_phdrsz;	/* sizeof phdr table */
	Elf_Void	*ed_shdr;	/* Elf{32,64}_Shdr shdr table */
	Elf_Scn		*ed_hdscn;	/* head scn */
	Elf_Scn		*ed_tlscn;	/* tail scn */
	size_t		ed_scntabsz;	/* number sects. alloc. in table */
	unsigned	ed_myflags;	/* EDF_... */
};

NOTE(RWLOCK_PROTECTS_DATA(Elf::ed_rwlock, Elf))
NOTE(RWLOCK_COVERS_LOCKS(Elf::ed_rwlock, Elf_Scn::s_mutex))

#ifdef	__lock_lint
#define	ELFRLOCK(e)	(void) rw_rdlock(&((Elf *)e)->ed_rwlock);
#else
#define	ELFRLOCK(e) \
	if (elf_threaded) \
		(void) rw_rdlock(&((Elf *)e)->ed_rwlock);
#endif

#ifdef	__lock_lint
#define	ELFWLOCK(e)	(void) rw_wrlock(&((Elf *)e)->ed_rwlock);
#else
#define	ELFWLOCK(e) \
	if (elf_threaded) \
		(void) rw_wrlock(&((Elf *)e)->ed_rwlock);
#endif

#ifdef	__lock_lint
#define	ELFUNLOCK(e)	(void) rw_unlock(&((Elf *)e)->ed_rwlock);
#else
#define	ELFUNLOCK(e) \
	if (elf_threaded) \
		(void) rw_unlock(&((Elf *)e)->ed_rwlock);
#endif

#define	EDF_EHALLOC	0x2	/* applies to ed_ehdr */
#define	EDF_PHALLOC	0x4	/* applies to ed_phdr */
#define	EDF_SHALLOC	0x8	/* applies to ed_shdr */
#define	EDF_READ	0x40	/* file can be read */
#define	EDF_WRITE	0x80	/* file can be written */
#define	EDF_RDKERNTYPE	0x2000	/* When Mach-o is fat, choose member matching the running kernel. */


typedef enum
{
	OK_YES = 0,
	OK_NO = ~0
} Okay;

#define	_(a)		a

/*
 * Max size for an Elf error message string
 */
#define	MAXELFERR	1024

/*
 * General thread management macros
 */
#ifdef __lock_lint
#define	ELFACCESSDATA(a, b) \
	(void) mutex_lock(&_elf_globals_mutex); \
	a = b; \
	(void) mutex_unlock(&_elf_globals_mutex);
#else
#define	ELFACCESSDATA(a, b) \
	if (elf_threaded) { \
		(void) mutex_lock(&_elf_globals_mutex); \
		a = b; \
		(void) mutex_unlock(&_elf_globals_mutex); \
	} else \
		a = b;
#endif

#ifdef __lock_lint
#define	ELFRWLOCKINIT(lock) \
	(void) rwlock_init((lock), USYNC_THREAD, 0);
#else
#define	ELFRWLOCKINIT(lock) \
	if (elf_threaded) { \
		(void) rwlock_init((lock), USYNC_THREAD, 0); \
	}
#endif

#ifdef	__lock_lint
#define	ELFMUTEXINIT(lock) \
	(void) mutex_init(lock, USYNC_THREAD, 0);
#else
#define	ELFMUTEXINIT(lock) \
	if (elf_threaded) { \
		(void) mutex_init(lock, USYNC_THREAD, 0); \
	}
#endif


extern void		_elf_arinit(Elf *);
extern Okay		_elf_cook(Elf *);
extern Okay		_elf_cookscn(Elf_Scn * s);
extern Okay		_elf32_cookscn(Elf_Scn * s);
extern Okay		_elf64_cookscn(Elf_Scn * s);
extern Elf_Data		*_elf_locked_getdata(Elf_Scn *, Elf_Data *);
extern Okay		_elf_inmap(Elf *);
extern size_t		_elf32_msize(Elf_Type, unsigned);
extern size_t		_elf64_msize(Elf_Type, unsigned);
extern Elf_Type		_elf32_mtype(Elf *, Elf32_Word, unsigned);
extern Elf_Type		_elf64_mtype(Elf *, Elf64_Word, unsigned);
extern Snode32		*_elf32_snode(void);
extern Snode64		*_elf64_snode(void);
extern void		_elf_unmap(char *, size_t);
extern Okay		_elf_vm(Elf *, size_t, size_t);
extern int		_elf32_ehdr(Elf *, int);
extern int		_elf32_phdr(Elf *, int);
extern int		_elf32_shdr(Elf *, int);
extern int		_elf64_ehdr(Elf *, int);
extern int		_elf64_phdr(Elf *, int);
extern int		_elf64_shdr(Elf *, int);
extern const Elf64_Ehdr	_elf64_ehdr_init;
extern unsigned		_elf_encode;
extern void		_elf_seterr(Msg, int);
extern const Snode32	_elf32_snode_init;
extern const Snode64	_elf64_snode_init;
extern unsigned		_elf_work;
extern mutex_t		_elf_globals_mutex;

/* CSTYLED */
NOTE(MUTEX_PROTECTS_DATA(_elf_globals_mutex, \
	_elf_byte _elf32_ehdr_init _elf64_ehdr_init _elf_encode \
	_elf_snode_init _elf_work))

#ifdef	__cplusplus
}
#endif

#endif	/* _DECL_H */
