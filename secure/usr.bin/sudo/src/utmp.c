/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2018 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#if defined(HAVE_UTMPS_H)
# include <utmps.h>
#elif defined(HAVE_UTMPX_H)
# include <utmpx.h>
#else
# include <utmp.h>
#endif /* HAVE_UTMPS_H */
#ifdef HAVE_GETTTYENT
# include <ttyent.h>
#endif
#include <fcntl.h>
#include <signal.h>

#include <sudo.h>
#include <sudo_exec.h>

/*
 * GCC 8 warns about strncpy() where the size field is the size of the buffer.
 * However, strings in utmp may not be NUL terminated so this usage is correct.
 */
#if __GNUC_PREREQ__(8, 0)
# pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

/*
 * Simplify handling of different utmp types.
 */
#if defined(HAVE_GETUTSID)
# define sudo_getutline(u)	GETUTSLINE(u)
# define sudo_pututline(u)	PUTUTSLINE(u)
# define sudo_setutent()	SETUTSENT()
# define sudo_endutent()	ENDUTSENT()
#elif defined(HAVE_GETUTXID)
# define sudo_getutline(u)	getutxline(u)
# define sudo_pututline(u)	pututxline(u)
# define sudo_setutent()	setutxent()
# define sudo_endutent()	endutxent()
#elif defined(HAVE_GETUTID)
# define sudo_getutline(u)	getutline(u)
# define sudo_pututline(u)	pututline(u)
# define sudo_setutent()	setutent()
# define sudo_endutent()	endutent()
#endif

#if defined(HAVE_GETUTSID)
typedef struct utmps sudo_utmp_t;
#elif defined(HAVE_GETUTXID)
typedef struct utmpx sudo_utmp_t;
#else
typedef struct utmp sudo_utmp_t;
/* Older systems have ut_name, not ut_user */
# if !defined(HAVE_STRUCT_UTMP_UT_USER) && !defined(ut_user)
#  define ut_user ut_name
# endif
#endif

/* HP-UX has __e_termination and __e_exit, others may lack the __ */
#if defined(HAVE_STRUCT_UTMP_UT_EXIT_E_TERMINATION)
# undef  __e_termination
# define __e_termination	e_termination
# undef  __e_exit
# define __e_exit		e_exit
#endif

#if defined(HAVE_STRUCT_UTMP_UT_ID)
/*
 * Create ut_id from the new ut_line and the old ut_id.
 */
static void
utmp_setid(sudo_utmp_t *old, sudo_utmp_t *new)
{
    const char *line = new->ut_line;
    size_t idlen;
    debug_decl(utmp_setid, SUDO_DEBUG_UTMP);

    /* Skip over "tty" in the id if old entry did too. */
    if (old != NULL) {
	/* cppcheck-suppress uninitdata */
	if (strncmp(line, "tty", 3) == 0) {
	    idlen = MIN(sizeof(old->ut_id), 3);
	    if (strncmp(old->ut_id, "tty", idlen) != 0)
		line += 3;
	}
    }
    
    /* Store as much as will fit, skipping parts of the beginning as needed. */
    /* cppcheck-suppress uninitdata */
    idlen = strlen(line);
    if (idlen > sizeof(new->ut_id)) {
	line += idlen - sizeof(new->ut_id);
	idlen = sizeof(new->ut_id);
    }
    strncpy(new->ut_id, line, idlen);

    debug_return;
}
#endif /* HAVE_STRUCT_UTMP_UT_ID */

/*
 * Store time in utmp structure.
 */
static void
utmp_settime(sudo_utmp_t *ut)
{
    struct timeval tv;
    debug_decl(utmp_settime, SUDO_DEBUG_UTMP);

    if (gettimeofday(&tv, NULL) == 0) {
#if defined(HAVE_STRUCT_UTMP_UT_TV)
	ut->ut_tv.tv_sec = tv.tv_sec;
	ut->ut_tv.tv_usec = tv.tv_usec;
#else
	ut->ut_time = tv.tv_sec;
#endif
    }

    debug_return;
}

/*
 * Fill in a utmp entry, using an old entry as a template if there is one.
 */
static void
utmp_fill(const char *line, const char *user, sudo_utmp_t *ut_old,
    sudo_utmp_t *ut_new)
{
    debug_decl(utmp_file, SUDO_DEBUG_UTMP);

    if (ut_old == NULL) {
	memset(ut_new, 0, sizeof(*ut_new));
    } else if (ut_old != ut_new) {
	memcpy(ut_new, ut_old, sizeof(*ut_new));
    }
    strncpy(ut_new->ut_user, user, sizeof(ut_new->ut_user));
    strncpy(ut_new->ut_line, line, sizeof(ut_new->ut_line));
#if defined(HAVE_STRUCT_UTMP_UT_ID)
    utmp_setid(ut_old, ut_new);
#endif
#if defined(HAVE_STRUCT_UTMP_UT_PID)
    ut_new->ut_pid = getpid();
#endif
    utmp_settime(ut_new);
#if defined(HAVE_STRUCT_UTMP_UT_TYPE)
    ut_new->ut_type = USER_PROCESS;
#endif
    debug_return;
}

/*
 * There are two basic utmp file types:
 *
 *  POSIX:  sequential access with new entries appended to the end.
 *	    Manipulated via {get,put}[sx]?utent()
 *
 *  Legacy: sparse file indexed by ttyslot() * sizeof(struct utmp)
 */
#if defined(HAVE_GETUTSID) || defined(HAVE_GETUTXID) || defined(HAVE_GETUTID)
bool
utmp_login(const char *from_line, const char *to_line, int ttyfd,
    const char *user)
{
    sudo_utmp_t utbuf, *ut_old = NULL;
    bool ret = false;
    debug_decl(utmp_login, SUDO_DEBUG_UTMP);

    /* Strip off /dev/ prefix from line as needed. */
    if (strncmp(to_line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	to_line += sizeof(_PATH_DEV) - 1;
    sudo_setutent();
    if (from_line != NULL) {
	if (strncmp(from_line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	    from_line += sizeof(_PATH_DEV) - 1;

	/* Lookup old line. */
	memset(&utbuf, 0, sizeof(utbuf));
	strncpy(utbuf.ut_line, from_line, sizeof(utbuf.ut_line));
	ut_old = sudo_getutline(&utbuf);
	sudo_setutent();
    }
    utmp_fill(to_line, user, ut_old, &utbuf);
    if (sudo_pututline(&utbuf) != NULL)
	ret = true;
    sudo_endutent();

    debug_return_bool(ret);
}

bool
utmp_logout(const char *line, int status)
{
    bool ret = false;
    sudo_utmp_t *ut, utbuf;
    debug_decl(utmp_logout, SUDO_DEBUG_UTMP);

    /* Strip off /dev/ prefix from line as needed. */
    if (strncmp(line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	line += sizeof(_PATH_DEV) - 1;
   
    memset(&utbuf, 0, sizeof(utbuf));
    strncpy(utbuf.ut_line, line, sizeof(utbuf.ut_line));
    if ((ut = sudo_getutline(&utbuf)) != NULL) {
	memset(ut->ut_user, 0, sizeof(ut->ut_user));
# if defined(HAVE_STRUCT_UTMP_UT_TYPE)
	ut->ut_type = DEAD_PROCESS;
# endif
# if defined(HAVE_STRUCT_UTMP_UT_EXIT)
	ut->ut_exit.__e_termination = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
	ut->ut_exit.__e_exit = WIFEXITED(status) ? WEXITSTATUS(status) : 0;
# endif
	utmp_settime(ut);
	if (sudo_pututline(ut) != NULL)
	    ret = true;
    }
    debug_return_bool(ret);
}

#else /* !HAVE_GETUTSID && !HAVE_GETUTXID && !HAVE_GETUTID */

/*
 * Find the slot for the specified line (tty name and file descriptor).
 * Returns a slot suitable for seeking into utmp on success or <= 0 on error.
 * If getttyent() is available we can use that to compute the slot.
 */
# ifdef HAVE_GETTTYENT
static int
utmp_slot(const char *line, int ttyfd)
{
    int slot = 1;
    struct ttyent *tty;
    debug_decl(utmp_slot, SUDO_DEBUG_UTMP);

    setttyent();
    while ((tty = getttyent()) != NULL) {
	if (strcmp(line, tty->ty_name) == 0)
	    break;
	slot++;
    }
    endttyent();
    debug_return_int(tty ? slot : 0);
}
# elif defined(HAVE_TTYSLOT)
static int
utmp_slot(const char *line, int ttyfd)
{
    int sfd, slot;
    debug_decl(utmp_slot, SUDO_DEBUG_UTMP);

    /*
     * Temporarily point stdin to the tty since ttyslot()
     * doesn't take an argument.
     */
    if ((sfd = dup(STDIN_FILENO)) == -1)
	sudo_fatal("%s", U_("unable to save stdin"));
    if (dup2(ttyfd, STDIN_FILENO) == -1)
	sudo_fatal("%s", U_("unable to dup2 stdin"));
    slot = ttyslot();
    if (dup2(sfd, STDIN_FILENO) == -1)
	sudo_fatal("%s", U_("unable to restore stdin"));
    close(sfd);

    debug_return_int(slot);
}
# else /* !HAVE_TTYSLOT */
static int
utmp_slot(const char *line, int ttyfd)
{
    return -1;
}
# endif /* HAVE_GETTTYENT */

bool
utmp_login(const char *from_line, const char *to_line, int ttyfd,
    const char *user)
{
    sudo_utmp_t utbuf, *ut_old = NULL;
    bool ret = false;
    int slot;
    FILE *fp;
    debug_decl(utmp_login, SUDO_DEBUG_UTMP);

    /* Strip off /dev/ prefix from line as needed. */
    if (strncmp(to_line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	to_line += sizeof(_PATH_DEV) - 1;

    /* Find slot for new entry. */
    slot = utmp_slot(to_line, ttyfd);
    if (slot <= 0)
	goto done;

    if ((fp = fopen(_PATH_UTMP, "r+")) == NULL)
	goto done;

    if (from_line != NULL) {
	if (strncmp(from_line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	    from_line += sizeof(_PATH_DEV) - 1;

	/* Lookup old line. */
	while (fread(&utbuf, sizeof(utbuf), 1, fp) == 1) {
# ifdef HAVE_STRUCT_UTMP_UT_ID
	    if (utbuf.ut_type != LOGIN_PROCESS && utbuf.ut_type != USER_PROCESS)
		continue;
# endif
	    if (utbuf.ut_user[0] &&
		!strncmp(utbuf.ut_line, from_line, sizeof(utbuf.ut_line))) {
		ut_old = &utbuf;
		break;
	    }
	}
    }
    utmp_fill(to_line, user, ut_old, &utbuf);
    if (fseeko(fp, slot * (off_t)sizeof(utbuf), SEEK_SET) == 0) {
	if (fwrite(&utbuf, sizeof(utbuf), 1, fp) == 1)
	    ret = true;
    }
    fclose(fp);

done:
    debug_return_bool(ret);
}

bool
utmp_logout(const char *line, int status)
{
    sudo_utmp_t utbuf;
    bool ret = false;
    FILE *fp;
    debug_decl(utmp_logout, SUDO_DEBUG_UTMP);

    if ((fp = fopen(_PATH_UTMP, "r+")) == NULL)
	debug_return_int(ret);

    /* Strip off /dev/ prefix from line as needed. */
    if (strncmp(line, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	line += sizeof(_PATH_DEV) - 1;
   
    while (fread(&utbuf, sizeof(utbuf), 1, fp) == 1) {
	if (!strncmp(utbuf.ut_line, line, sizeof(utbuf.ut_line))) {
	    memset(utbuf.ut_user, 0, sizeof(utbuf.ut_user));
# if defined(HAVE_STRUCT_UTMP_UT_TYPE)
	    utbuf.ut_type = DEAD_PROCESS;
# endif
	    utmp_settime(&utbuf);
	    /* Back up and overwrite record. */
	    if (fseeko(fp, (off_t)0 - (off_t)sizeof(utbuf), SEEK_CUR) == 0) {
		if (fwrite(&utbuf, sizeof(utbuf), 1, fp) == 1)
		    ret = true;
	    }
	    break;
	}
    }
    fclose(fp);

    debug_return_bool(ret);
}
#endif /* HAVE_GETUTSID || HAVE_GETUTXID || HAVE_GETUTID */
