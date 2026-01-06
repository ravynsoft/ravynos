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

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include	<pthread.h> /* In lieu of Solaris <thread.h> */
#define thr_keycreate pthread_key_create /* In lieu of Solaris <thread.h> */
#define thr_getspecific(key, pval) (*pval = pthread_getspecific( key )) /* In lieu of Solaris <thread.h> */
#define thr_setspecific pthread_setspecific /* In lieu of Solaris <thread.h> */

/* Solaris thr_main() is used to detect the possibility that libelf is being used in a
   threaded application and is subject to re-entrancy. There is no Posix cognate.
   For safety, always indicate threading is live. */
#define thr_main() 0 /* In lieu of Solaris <thread.h> */
int	__libc_threaded = 1; /* In lieu of Solaris <thread.h> */

#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"libelf.h"
#include	"msg.h"
#undef		_elf_seterr

#define EINF_NULLERROR 0
#define EBUG_THRDKEY 0

#define MSG_FMT_ERR -1
#define MSG_INTL(x) "libelf internal error"
#define MSG_ORIG(x) (x == MSG_FMT_ERR ? "%s %s" : NULL)

#include	"decl.h"

#define	ELFERRSHIFT	16
#define	SYSERRMASK	0xffff


/*
 * _elf_err has two values encoded in it, both the _elf_err # and
 * the system errno value (if relevant).  These values are encoded
 * in the upper & lower 16 bits of the 4 byte integer.
 */
static int		_elf_err = 0;


/*
 * This code is here to enable the building of a native version
 * of libelf.so when the build machine has not yet been upgraded
 * to a version of libc that provides thr_keycreate_once().
 * It should be deleted when solaris_nevada ships.
 * The code is not MT-safe in a relaxed memory model.
 */

static thread_key_t	errkey = 0;
static thread_key_t	bufkey = 0;

static int
thr_keycreate_once(thread_key_t *keyp, void (*destructor)(void *))
{
	static mutex_t key_lock = DEFAULTMUTEX;
	thread_key_t key;
	int error;

	if (*keyp == 0) {
		mutex_lock(&key_lock);
		if (*keyp == 0) {
			error = thr_keycreate(&key, destructor);
			if (error) {
				mutex_unlock(&key_lock);
				return (error);
			}
			*keyp = key;
		}
		mutex_unlock(&key_lock);
	}

	return (0);
}

void
_elf_seterr(Msg lib_err, int sys_err)
{
	/*LINTED*/
	intptr_t encerr = ((int)lib_err << ELFERRSHIFT) |
	    (sys_err & SYSERRMASK);

#ifndef	__lock_lint
	if (thr_main()) {
		_elf_err = (int)encerr;
		return;
	}
#endif
	(void) thr_keycreate_once(&errkey, 0);
	(void) thr_setspecific(errkey, (void *)encerr);
}

static int
_elf_geterr() {
#ifndef	__lock_lint
	if (thr_main())
		return (_elf_err);
#endif
	return ((uintptr_t)pthread_getspecific(errkey));
}

const char *
elf_errmsg(int err)
{
	char			*errno_str;
	char			*elferr_str;
	char			*buffer = 0;
	int			syserr;
	int			elferr;
	static char		intbuf[MAXELFERR];

	if (err == 0) {
		if ((err = _elf_geterr()) == 0)
			return (0);
	} else if (err == -1) {
		if ((err = _elf_geterr()) == 0)
			/*LINTED*/ /* MSG_INTL(EINF_NULLERROR) */
			err = (int)EINF_NULLERROR << ELFERRSHIFT;
	}

	if (thr_main())
		buffer = intbuf;
	else {
		/*
		 * If this is a threaded APP then we store the
		 * errmsg buffer in Thread Specific Storage.
		 *
		 * Each thread has its own private buffer.
		 */
		if (thr_keycreate_once(&bufkey, free) != 0)
					return (MSG_INTL(EBUG_THRDKEY));
		buffer = pthread_getspecific(bufkey);

		if (!buffer) {
			if ((buffer = malloc(MAXELFERR)) == 0)
				return (MSG_INTL(EMEM_ERRMSG));
			if (thr_setspecific(bufkey, buffer) != 0) {
				free(buffer);
				return (MSG_INTL(EBUG_THRDSET));
			}
		}
	}

	elferr = (int)((uint_t)err >> ELFERRSHIFT);
	syserr = err & SYSERRMASK;
	/*LINTED*/
	elferr_str = (char *)MSG_INTL(elferr);
	if (syserr && (errno_str = strerror(syserr)))
		(void) snprintf(buffer, MAXELFERR,
		    MSG_ORIG(MSG_FMT_ERR), elferr_str, errno_str);
	else {
		(void) strncpy(buffer, elferr_str, MAXELFERR - 1);
		buffer[MAXELFERR - 1] = '\0';
	}

	return (buffer);
}

int
elf_errno()
{
	int	rc = _elf_geterr();

	_elf_seterr(0, 0);
	return (rc);
}
