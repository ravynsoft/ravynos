/*
 * prototypes.h - prototypes header file
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#ifndef HAVE_STDLIB_H
char *malloc _((size_t));
char *realloc _((void *, size_t));
char *calloc _((size_t, size_t));
#endif

#if !(defined(USES_TERMCAP_H) || defined(USES_TERM_H))
/*
 * These prototypes are only used where we don't have the
 * headers.  In some cases they need tweaking.
 * TBD: we'd much prefer to get hold of the header where
 * these are defined.
 */
#ifdef _AIX
#define TC_CONST const
#else
#define TC_CONST
#endif
extern int tgetent _((char *bp, TC_CONST char *name));
extern int tgetnum _((char *id));
extern int tgetflag _((char *id));
extern char *tgetstr _((char *id, char **area));
extern int tputs _((TC_CONST char *cp, int affcnt, int (*outc) (int)));
#undef TC_CONST
#endif

/*
 * Some systems that do have termcap headers nonetheless don't
 * declare tgoto, so we detect if that is missing separately.
 */
#ifdef TGOTO_PROTO_MISSING
char *tgoto(const char *cap, int col, int row);
#endif

/* MISSING PROTOTYPES FOR VARIOUS OPERATING SYSTEMS */

#if defined(__hpux) && defined(_HPUX_SOURCE) && !defined(_XPG4_EXTENDED)
# define SELECT_ARG_2_T int *
#else
# define SELECT_ARG_2_T fd_set *
#endif

#ifdef __osf__
char *mktemp _((char *));
#endif

#if defined(__osf__) && defined(__alpha) && defined(__GNUC__)
/* Digital cc does not need these prototypes, gcc does need them */
# ifndef HAVE_IOCTL_PROTO
int ioctl _((int d, unsigned long request, void *argp));
# endif
# ifndef HAVE_MKNOD_PROTO
int mknod _((const char *pathname, int mode, dev_t device));
# endif
int nice _((int increment));
int select _((int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout));
#endif

#if defined(DGUX) && defined(__STDC__)
/* Just plain missing. */
extern int getrlimit _((int resource, struct rlimit *rlp));
extern int setrlimit _((int resource, const struct rlimit *rlp));
extern int getrusage _((int who, struct rusage *rusage));
extern int gettimeofday _((struct timeval *tv, struct timezone *tz));
extern int wait3 _((union wait *wait_status, int options, struct rusage *rusage));
extern int getdomainname _((char *name, int maxlength));
extern int select _((int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout));
#endif /* DGUX and __STDC__ */

#ifdef __NeXT__
extern pid_t getppid(void);
#endif

#if defined(__sun__) && !defined(__SVR4)  /* SunOS */
extern char *strerror _((int errnum));
#endif

/**************************************************/
/*** prototypes for functions built in compat.c ***/
#ifndef HAVE_STRSTR
extern char *strstr _((const char *s, const char *t));
#endif

#ifndef HAVE_GETHOSTNAME
extern int gethostname _((char *name, size_t namelen));
#endif

#ifndef HAVE_GETTIMEOFDAY
extern int gettimeofday _((struct timeval *tv, struct timezone *tz));
#endif

#ifndef HAVE_DIFFTIME
extern double difftime _((time_t t2, time_t t1));
#endif

#ifndef HAVE_STRERROR
extern char *strerror _((int errnum));
#endif

/*** end of prototypes for functions in compat.c ***/
/***************************************************/

#ifndef HAVE_MEMMOVE
extern void bcopy _((const void *, void *, size_t));
#endif
