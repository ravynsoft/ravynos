/*
 * zle_main.c - main routines for line editor
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

#include "zle.mdh"
#include "zle_main.pro"

#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#if defined(HAVE_POLL) && !defined(POLLIN) && !defined(POLLNORM)
# undef HAVE_POLL
#endif

/* The input line assembled so far */

/**/
mod_export ZLE_STRING_T zleline;

/* Cursor position and line length in zle */

/**/
mod_export int zlecs, zlell;

/* != 0 if in a shell function called from completion, such that read -[cl]  *
 * will work (i.e., the line is metafied, and the above word arrays are OK). */

/**/
mod_export int incompctlfunc;

/* != 0 if completion module is loaded */

/**/
mod_export int hascompmod;

/* Increment for each nested recursive-edit */

/**/
mod_export int zle_recursive;

/* ZLRF_* flags passed to zleread() */

/**/
int zlereadflags;

/* ZLCON_* flags passed to zleread() */

/**/
int zlecontext;

/* != 0 if we're done editing */

/**/
int done;

/* location of mark */

/**/
int mark;

/*
 * Status ($?) saved before function entry.  This is the
 * status we need to use in prompts.
 */

/**/
static int pre_zle_status;

/*
 * Last character pressed.
 *
 * Depending how far we are with processing, the lastcharacter may
 * be a single byte read (lastchar_wide_valid is 0, lastchar_wide is not
 * valid) or a full wide character.  This is needed because we can't be
 * sure whether the user is typing old \M-style commands or multibyte
 * input.
 *
 * Calling getfullchar or getrestchar is guaranteed to ensure we have
 * a valid wide character (although this may be WEOF).  In many states
 * we know this and don't need to test lastchar_wide_valid.
 */

/**/
mod_export int
lastchar;
#ifdef MULTIBYTE_SUPPORT
/**/
mod_export ZLE_INT_T lastchar_wide;
/**/
mod_export int
lastchar_wide_valid;
#endif

/* the bindings for the previous and for this key */

/**/
mod_export Thingy lbindk, bindk;

/* insert mode/overwrite mode flag */

/**/
int insmode;

/**/
mod_export int eofchar;

static int eofsent;
/*
 * Key timeout in hundredths of a second:  we use time_t so
 * that we only have the limits on one integer type to worry about.
 */
static time_t keytimeout;

#if defined(HAVE_SELECT) || defined(HAVE_POLL)
/* Terminal baud rate */

static int baud;
static long costmult;
#endif

/* flags associated with last command */

/**/
mod_export int lastcmd;

/**/
mod_export Widget compwidget;

/* the status line, a null-terminated metafied string */

/**/
mod_export char *statusline;

/* The current history line and cursor position for the top line *
 * on the buffer stack.                                          */

/**/
int stackhist, stackcs;

/* position in undo stack from when the current vi change started */

/**/
zlong vistartchange;

/* current modifier status */

/**/
mod_export struct modifier zmod;

/* Current command prefix status.  This is normally 0.  Prefixes set *
 * this to 1.  Each time round the main loop, this is checked: if it *
 * is 0, the modifier status is reset; if it is 1, the modifier      *
 * status is left unchanged, and this flag is reset to 0.  The       *
 * effect is that several prefix commands can be executed, and have  *
 * cumulative effect, but any other command execution will clear the *
 * modifiers.                                                        */

/**/
int prefixflag;

/* Number of characters waiting to be read by the ungetbytes mechanism */
/**/
int kungetct;

/**/
mod_export char *zlenoargs[1] = { NULL };

static char **raw_lp, **raw_rp;

/*
 * File descriptors we are watching as well as the terminal fd. 
 * These are all for reading; we don't watch for writes or exceptions.
 */
/**/
int nwatch;		/* Number of fd's we are watching */
/*
 * Array of nwatch structures.
 */
/**/
Watch_fd watch_fds;

/* set up terminal */

/**/
mod_export void
zsetterm(void)
{
    struct ttyinfo ti;

    if (fetchttyinfo) {
	/*
	 * User requested terminal to be returned to normal use,
	 * so remember the terminal settings if not frozen.
	 */
	if (!ttyfrozen)
	    gettyinfo(&shttyinfo);
	fetchttyinfo = 0;
    }

/* sanitize the tty */
#ifdef HAS_TIO
    shttyinfo.tio.c_lflag |= ICANON | ECHO;
# ifdef FLUSHO
    shttyinfo.tio.c_lflag &= ~FLUSHO;
# endif
#else				/* not HAS_TIO */
    shttyinfo.sgttyb.sg_flags = (shttyinfo.sgttyb.sg_flags & ~CBREAK) | ECHO;
    shttyinfo.lmodes &= ~LFLUSHO;
#endif

    attachtty(mypgrp);
    ti = shttyinfo;
#ifdef HAS_TIO
    if (unset(FLOWCONTROL))
	ti.tio.c_iflag &= ~IXON;
    ti.tio.c_lflag &= ~(ICANON | ECHO
# ifdef FLUSHO
			| FLUSHO
# endif
	);
# ifdef TAB3
    ti.tio.c_oflag &= ~TAB3;
# else
#  ifdef OXTABS
    ti.tio.c_oflag &= ~OXTABS;
#  else
#   ifdef XTABS
    ti.tio.c_oflag &= ~XTABS;
#   endif
#  endif
# endif
#ifdef ONLCR
    ti.tio.c_oflag |= ONLCR;
#endif
    ti.tio.c_cc[VQUIT] =
# ifdef VDISCARD
	ti.tio.c_cc[VDISCARD] =
# endif
# ifdef VSUSP
	ti.tio.c_cc[VSUSP] =
# endif
# ifdef VDSUSP
	ti.tio.c_cc[VDSUSP] =
# endif
# ifdef VSWTCH
	ti.tio.c_cc[VSWTCH] =
# endif
# ifdef VLNEXT
	ti.tio.c_cc[VLNEXT] =
# endif
	VDISABLEVAL;
# if defined(VSTART) && defined(VSTOP)
    if (unset(FLOWCONTROL))
	ti.tio.c_cc[VSTART] = ti.tio.c_cc[VSTOP] = VDISABLEVAL;
# endif
    eofchar = ti.tio.c_cc[VEOF];
    ti.tio.c_cc[VMIN] = 1;
    ti.tio.c_cc[VTIME] = 0;
    ti.tio.c_iflag |= (INLCR | ICRNL);
 /* this line exchanges \n and \r; it's changed back in getbyte
	so that the net effect is no change at all inside the shell.
	This double swap is to allow typeahead in common cases, eg.

	% bindkey -s '^J' 'echo foo^M'
	% sleep 10
	echo foo<return>  <--- typed before sleep returns

	The shell sees \n instead of \r, since it was changed by the kernel
	while zsh wasn't looking. Then in getbyte() \n is changed back to \r,
	and it sees "echo foo<accept line>", as expected. Without the double
	swap the shell would see "echo foo\n", which is translated to
	"echo fooecho foo<accept line>" because of the binding.
	Note that if you type <line-feed> during the sleep the shell just sees
	\n, which is translated to \r in getbyte(), and you just get another
	prompt. For type-ahead to work in ALL cases you have to use
	stty inlcr.

	Unfortunately it's IMPOSSIBLE to have a general solution if both
	<return> and <line-feed> are mapped to the same character. The shell
	could check if there is input and read it before setting it's own
	terminal modes but if we get a \n we don't know whether to keep it or
	change to \r :-(
	*/

#else				/* not HAS_TIO */
    ti.sgttyb.sg_flags = (ti.sgttyb.sg_flags | CBREAK) & ~ECHO & ~XTABS;
    ti.lmodes &= ~LFLUSHO;
    eofchar = ti.tchars.t_eofc;
    ti.tchars.t_quitc =
	ti.ltchars.t_suspc =
	ti.ltchars.t_flushc =
	ti.ltchars.t_dsuspc = ti.ltchars.t_lnextc = -1;
#endif

#if defined(TIOCOUTQ) && defined(HAVE_SELECT)
    if (baud) {			/**/
	int n = 0;

	while ((ioctl(SHTTY, TIOCOUTQ, (char *)&n) >= 0) && n) {
	    struct timeval tv;

	    tv.tv_sec = n / baud;
	    tv.tv_usec = ((n % baud) * 1000000) / baud;
	    select(0, NULL, NULL, NULL, &tv);
	}
    }
#endif

    settyinfo(&ti);
}

static char *kungetbuf;
static int kungetsz;

/*
 * Note on ungetbyte and ungetbytes for the confused (pws):
 * these are low level and deal with bytes before they
 * have been converted into (possibly wide) characters.
 * Hence the names.
 */

/**/
void
ungetbyte(int ch)
{
    if (kungetct == kungetsz)
	kungetbuf = realloc(kungetbuf, kungetsz *= 2);
    kungetbuf[kungetct++] = ch;
}

/**/
void
ungetbytes(char *s, int len)
{
    s += len;
    while (len--)
	ungetbyte(*--s);
}

/**/
void
ungetbytes_unmeta(char *s, int len)
{
    s += len;
    while (len--) {
	if (len && s[-2] == Meta) {
	    ungetbyte(*--s ^ 32);
	    len--;
	    s--;
	} else
	    ungetbyte(*--s);
    }
}

#if defined(pyr) && defined(HAVE_SELECT)
static int
breakread(int fd, char *buf, int n)
{
    fd_set f;

    FD_ZERO(&f);
    FD_SET(fd, &f);

    return (select(fd + 1, (SELECT_ARG_2_T) & f, NULL, NULL, NULL) == -1 ?
	    EOF : read(fd, buf, n));
}

# define read    breakread
#endif

/*
 * Possible forms of timeout.
 */
enum ztmouttp {
    /* No timeout in use. */
    ZTM_NONE,
    /*
     * Key timeout in use (do_keytmout flag set).  If this goes off
     * we return without anything being read.
     */
    ZTM_KEY,
    /*
     * Function timeout in use (from timedfns list).
     * If this goes off we call any functions which have reached
     * the time and then continue processing.
     */
    ZTM_FUNC,
    /*
     * Timeout hit the maximum allowed; if it fires we
     * need to recalculate.  As we may use poll() for the timeout,
     * which takes an int value in milliseconds, we might need this
     * for times long in the future.  (We make no attempt to extend
     * the range of time beyond that of time_t, however; that seems
     * like a losing battle.)
     *
     * For key timeouts we just limit the value to
     * ZMAXTIMEOUT; that's already absurdly large.
     *
     * The following is the maximum signed range over 1024 (2^10), which
     * is a little more convenient than 1000, but done differently
     * to avoid problems with unsigned integers.  We assume 8-bit bytes;
     * there's no general way to fix up if that's wrong.
     */
    ZTM_MAX
#define	ZMAXTIMEOUT	((time_t)1 << (sizeof(time_t)*8-11))
};

struct ztmout {
    /* Type of timeout setting, see enum above */
    enum ztmouttp tp;
    /*
     * Value for timeout in 100ths of a second if type is not ZTM_NONE.
     */
    time_t exp100ths;
};

/*
 * See if we need a timeout either for a key press or for a
 * timed function.
 *
 * do_keytmout is passed down from getbyte() here.  If it is positive,
 * we use the keytimeout value, which is in 100ths of a second (directly
 * set from the parameter).  If it is negative, we use -(do_keytmout+1)
 * (i.e. the one's complement, to allow a zero value to be set).  This
 * is only used when calling into zle from outside to specify an
 * explicit timeout.  This is also in 100ths of a second.
 */

static void
calc_timeout(struct ztmout *tmoutp, long do_keytmout, int full)
{
    if (do_keytmout && (keytimeout > 0 || do_keytmout < 0)) {
	if (do_keytmout < 0)
	    tmoutp->exp100ths = (time_t)-do_keytmout;
	else if (keytimeout > ZMAXTIMEOUT * 100 /* 24 days for a keypress???? */)
	    tmoutp->exp100ths = ZMAXTIMEOUT * 100;
	else
	    tmoutp->exp100ths = keytimeout;
	tmoutp->tp = ZTM_KEY;
    } else
	tmoutp->tp = ZTM_NONE;

    if (full && timedfns) {
	for (;;) {
	    LinkNode tfnode = firstnode(timedfns);
	    Timedfn tfdat;
	    time_t diff, exp100ths;

	    if (!tfnode)
		break;

	    tfdat = (Timedfn)getdata(tfnode);
	    diff = tfdat->when - time(NULL);
	    if (diff <= 0) {
		/* Already due; call it and rescan. */
		tfdat->func();
		continue;
	    }

	    if (diff > ZMAXTIMEOUT) {
		tmoutp->exp100ths = ZMAXTIMEOUT * 100;
		tmoutp->tp = ZTM_MAX;
	    } else if (diff > 0) {
		exp100ths = diff * 100;
		if (tmoutp->tp != ZTM_KEY ||
		    exp100ths < tmoutp->exp100ths) {
		    tmoutp->exp100ths = exp100ths;
		    tmoutp->tp = ZTM_FUNC;
		}
	    }
	    break;
	}
	/* In case we called a function which messed up the display... */
	if (resetneeded)
	    zrefresh();
    }
}

/* see calc_timeout for use of do_keytmout */

static int
raw_getbyte(long do_keytmout, char *cptr, int full)
{
    int ret;
    struct ztmout tmout;
#if defined(HAS_TIO) && \
  (defined(sun) || (!defined(HAVE_POLL) && !defined(HAVE_SELECT)))
    struct ttyinfo ti;
#endif
#ifndef HAVE_POLL
# ifdef HAVE_SELECT
    fd_set foofd, errfd;
    FD_ZERO(&errfd);
# endif
#endif

    calc_timeout(&tmout, do_keytmout, full);

    /*
     * Handle timeouts and watched fd's.  If a watched fd or a function
     * timeout triggers we restart any key timeout.  This is likely to
     * be harmless: the combination is extremely rare and a function
     * is likely to occupy the user for a little while anyway.  We used
     * to make timeouts take precedence, but we can't now that the
     * timeouts may be external, so we may have both a permanent watched
     * fd and a long-term timeout.
     */
    if ((nwatch || tmout.tp != ZTM_NONE)) {
#if defined(HAVE_SELECT) || defined(HAVE_POLL)
	int i, errtry = 0, selret;
# ifdef HAVE_POLL
	int nfds;
	struct pollfd *fds;
# endif
# if defined(HAS_TIO) && defined(sun)
	/*
	 * Yes, I know this is complicated.  Yes, I know we
	 * already have three bits of code to poll the terminal
	 * down below.  No, I don't want to do this either.
	 * However, it turns out on certain OSes, specifically
	 * Solaris, that you can't poll typeahead for love nor
	 * money without actually trying to read it.  But
	 * if we are trying to select (and we need to if we
	 * are watching other fd's) we won't pick that up.
	 * So we just try and read it without blocking in
	 * the time-honoured (i.e. absurdly baroque) termios
	 * fashion.
	 */
	gettyinfo(&ti);
	ti.tio.c_cc[VMIN] = 0;
	settyinfo(&ti);
	winch_unblock();
	ret = read(SHTTY, cptr, 1);
	winch_block();
	ti.tio.c_cc[VMIN] = 1;
	settyinfo(&ti);
	if (ret > 0)
	    return 1;
# endif
# ifdef HAVE_POLL
	nfds = 1 + nwatch;
	/* First pollfd is SHTTY, following are the nwatch fds */
	fds = zalloc(sizeof(struct pollfd) * nfds);
	fds[0].fd = SHTTY;
	/*
	 * POLLIN, POLLIN, POLLIN,
	 * Keep those fd's POLLIN...
	 */
	fds[0].events = POLLIN;
	for (i = 0; i < nwatch; i++) {
	    fds[i+1].fd = watch_fds[i].fd;
	    fds[i+1].events = POLLIN;
	}
# endif
	for (;;) {
# ifdef HAVE_POLL
	    int poll_timeout;

	    if (tmout.tp != ZTM_NONE)
		poll_timeout = tmout.exp100ths * 10;
	    else
		poll_timeout = -1;

	    winch_unblock();
	    selret = poll(fds, errtry ? 1 : nfds, poll_timeout);
	    winch_block();
# else
	    int fdmax = SHTTY;
	    struct timeval *tvptr;
	    struct timeval expire_tv;

	    FD_ZERO(&foofd);
	    FD_SET(SHTTY, &foofd);
	    if (!errtry) {
		for (i = 0; i < nwatch; i++) {
		    int fd = watch_fds[i].fd;
		    if (FD_ISSET(fd, &errfd))
			continue;
		    FD_SET(fd, &foofd);
		    if (fd > fdmax)
			fdmax = fd;
		}
	    }
	    FD_ZERO(&errfd);

	    if (tmout.tp != ZTM_NONE) {
		expire_tv.tv_sec = tmout.exp100ths / 100;
		expire_tv.tv_usec = (tmout.exp100ths % 100) * 10000L;
		tvptr = &expire_tv;
	    }
	    else
		tvptr = NULL;

	    winch_unblock();
	    selret = select(fdmax+1, (SELECT_ARG_2_T) & foofd,
			    NULL, NULL, tvptr);
	    winch_block();
# endif
	    /*
	     * Make sure a user interrupt gets passed on straight away.
	     */
	    if (selret < 0 && (errflag || retflag || breaks || exit_pending))
		break;
	    /*
	     * Try to avoid errors on our special fd's from
	     * messing up reads from the terminal.  Try first
	     * with all fds, then try unsetting the special ones.
	     */
	    if (selret < 0 && !errtry) {
		/* Continue after irrelevant interrupt */
		if (errno != EINTR) {
		    /* Don't trust special FDs */
		    errtry = 1;
		}
		continue;
	    }
	    if (selret == 0) {
		zlong save_lastval;

		/*
		 * Nothing ready and no error, so we timed out.
		 */
		switch (tmout.tp) {
		case ZTM_NONE:
		    /* keeps compiler happy if not debugging */
#ifdef DEBUG
		    dputs("BUG: timeout fired with no timeout set.");
#endif
		    /* treat as if a key timeout triggered */
		    /*FALLTHROUGH*/
		case ZTM_KEY:
		    /* Special value -2 signals nothing ready */
		    selret = -2;
		    break;

		case ZTM_FUNC:
		    save_lastval = lastval;
		    while (firstnode(timedfns)) {
			Timedfn tfdat = (Timedfn)getdata(firstnode(timedfns));
			/*
			 * It's possible a previous function took
			 * a long time to run (though it can't
			 * call zle recursively), so recalculate
			 * the time on each iteration.
			 */
			time_t now = time(NULL);
			if (tfdat->when > now)
			    break;
			tfdat->func();
		    }
		    lastval = save_lastval;
		    /* Function may have messed up the display */
		    if (resetneeded)
			zrefresh();
		    /* We need to recalculate the timeout */
		    /*FALLTHROUGH*/
		case ZTM_MAX:
		    /*
		     * Reached the limit of our range, but not the
		     * actual timeout; recalculate the timeout.
		     * We're cheating with the key timeout here:
		     * if one clashed with a function timeout we
		     * reconsider the key timeout from scratch.
		     * The effect of this is microscopic.
		     */
		    calc_timeout(&tmout, do_keytmout, full);
		    break;
		}
		/*
		 * If we handled the timeout successfully,
		 * carry on.
		 */
		if (selret == 0)
		    continue;
	    }
	    /* If error or unhandled timeout, give up. */
	    if (selret < 0)
		break;
	    /*
	     * If there's user input handle it straight away.
	     * This improves the user's ability to handle exceptional
	     * conditions like runaway output.
	     */
	    if (
# ifdef HAVE_POLL
		 (fds[0].revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL))
# else
		 FD_ISSET(SHTTY, &foofd)
# endif
		 )
		break;
	    if (nwatch && !errtry) {
		/*
		 * Copy the details of the watch fds in case the
		 * user decides to delete one from inside the
		 * handler function.
		 */
		int lnwatch = nwatch;
		Watch_fd lwatch_fds = zalloc(lnwatch*sizeof(struct watch_fd));
		memcpy(lwatch_fds, watch_fds, lnwatch*sizeof(struct watch_fd));
		for (i = 0; i < lnwatch; i++)
		    lwatch_fds[i].func = ztrdup(lwatch_fds[i].func);
		for (i = 0; i < lnwatch; i++) {
		    Watch_fd lwatch_fd = lwatch_fds + i;
		    if (
# ifdef HAVE_POLL
			(fds[i+1].revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL))
# else
			FD_ISSET(lwatch_fd->fd, &foofd) ||
			FD_ISSET(lwatch_fd->fd, &errfd)
# endif
			) {
			/* Handle the fd. */
			char *fdbuf;
			{
			    char buf[BDIGBUFSIZE];
			    convbase(buf, lwatch_fd->fd, 10);
			    fdbuf = ztrdup(buf);
			}

			if (lwatch_fd->widget) {
			    zlecallhook(lwatch_fd->func, fdbuf);
			    zsfree(fdbuf);
			} else {
			    LinkList funcargs = znewlinklist();
			    zaddlinknode(funcargs, ztrdup(lwatch_fd->func));
			    zaddlinknode(funcargs, fdbuf);
# ifdef HAVE_POLL
#  ifdef POLLERR
			    if (fds[i+1].revents & POLLERR)
				zaddlinknode(funcargs, ztrdup("err"));
#  endif
#  ifdef POLLHUP
			    if (fds[i+1].revents & POLLHUP)
				zaddlinknode(funcargs, ztrdup("hup"));
#  endif
#  ifdef POLLNVAL
			    if (fds[i+1].revents & POLLNVAL)
				zaddlinknode(funcargs, ztrdup("nval"));
#  endif
# else
			    if (FD_ISSET(lwatch_fd->fd, &errfd))
				zaddlinknode(funcargs, ztrdup("err"));
# endif
			    callhookfunc(lwatch_fd->func, funcargs, 0, NULL);
			    freelinklist(funcargs, freestr);
			}
			if (errflag) {
			    /* No sensible way of handling errors here */
			    errflag &= ~ERRFLAG_ERROR;
			    /*
			     * Paranoia: don't run the hooks again this
			     * time.
			     */
			    errtry = 1;
			}
		    }
		}
		/* Function may have invalidated the display. */
		if (resetneeded)
		    zrefresh();
		for (i = 0; i < lnwatch; i++)
		    zsfree(lwatch_fds[i].func);
		zfree(lwatch_fds, lnwatch*sizeof(struct watch_fd));

# ifdef HAVE_POLL
		/* Function may have added or removed handlers */
		nfds = 1 + nwatch;
		if (nfds > 1) {
		    fds = zrealloc(fds, sizeof(struct pollfd) * nfds);
		    for (i = 0; i < nwatch; i++) {
			/*
			 * This is imperfect because it assumes fds[] and
			 * watch_fds[] remain in sync, which may be false
			 * if handlers are shuffled.  However, it should
			 * be harmless (e.g., produce one extra pass of
			 * the loop) in the event they fall out of sync.
			 */
			if (fds[i+1].fd == watch_fds[i].fd &&
			    (fds[i+1].revents & (POLLERR|POLLHUP|POLLNVAL))) {
			    fds[i+1].events = 0;	/* Don't poll this */
			} else {
			    fds[i+1].fd = watch_fds[i].fd;
			    fds[i+1].events = POLLIN;
			}
			fds[i+1].revents = 0;
		    }
		}
# endif
	    }
	    /* If looping, need to recalculate timeout */
	    calc_timeout(&tmout, do_keytmout, full);
	}
# ifdef HAVE_POLL
	zfree(fds, sizeof(struct pollfd) * nfds);
# endif
	if (selret < 0)
	    return selret;
#else
# ifdef HAS_TIO
	ti = shttyinfo;
	ti.tio.c_lflag &= ~ICANON;
	ti.tio.c_cc[VMIN] = 0;
	ti.tio.c_cc[VTIME] = tmout.exp100ths / 10;
#  ifdef HAVE_TERMIOS_H
	tcsetattr(SHTTY, TCSANOW, &ti.tio);
#  else
	ioctl(SHTTY, TCSETA, &ti.tio);
#  endif
	winch_unblock();
	ret = read(SHTTY, cptr, 1);
	winch_block();
#  ifdef HAVE_TERMIOS_H
	tcsetattr(SHTTY, TCSANOW, &shttyinfo.tio);
#  else
	ioctl(SHTTY, TCSETA, &shttyinfo.tio);
#  endif
	return (ret <= 0) ? ret : *cptr;
# endif
#endif
    }

    winch_unblock();
    ret = read(SHTTY, cptr, 1);
    winch_block();

    return ret;
}

/* see calc_timeout for use of do_keytmout */

/**/
mod_export int
getbyte(long do_keytmout, int *timeout, int full)
{
    char cc;
    unsigned int ret;
    int die = 0, r, icnt = 0;
    int old_errno = errno, obreaks = breaks;

    if (timeout)
	*timeout = 0;

#ifdef MULTIBYTE_SUPPORT
    /*
     * Reading a single byte always invalidates the status
     * of lastchar_wide.  We may fix this up in getrestchar
     * if this is the last byte of a wide character.
     */
    lastchar_wide_valid = 0;
#endif

    if (kungetct)
	ret = STOUC(kungetbuf[--kungetct]);
    else {
	for (;;) {
	    int q = queue_signal_level();
	    dont_queue_signals();
	    r = raw_getbyte(do_keytmout, &cc, full);
	    restore_queue_signals(q);
	    if (r == -2) {
		/* timeout */
		if (timeout)
		    *timeout = 1;
		return lastchar = EOF;
	    }
	    if (r == 1)
		break;
	    if (r == 0) {
		/* The test for IGNOREEOF was added to make zsh ignore ^Ds
		   that were typed while commands are running.  Unfortunately
		   this caused trouble under at least one system (SunOS 4.1).
		   Here shells that lost their xterm (e.g. if it was killed
		   with -9) didn't fail to read from the terminal but instead
		   happily continued to read EOFs, so that the above read
		   returned with 0, and, with IGNOREEOF set, this caused
		   an infinite loop.  The simple way around this was to add
		   the counter (icnt) so that this happens 20 times and than
		   the shell gives up (yes, this is a bit dirty...). */
		if ((zlereadflags & ZLRF_IGNOREEOF) && icnt++ < 20)
		    continue;
		stopmsg = 1;
		zexit(1, ZEXIT_NORMAL);
		/* If called from an exit hook, zexit() returns, so: */
		break;
	    }
	    icnt = 0;
	    if (errno == EINTR) {
		die = 0;
		if (!errflag && !retflag && !breaks && !exit_pending)
		    continue;
		errflag &= ~ERRFLAG_ERROR;
		breaks = obreaks;
		errno = old_errno;
		return lastchar = EOF;
	    } else if (errno == EWOULDBLOCK) {
		fcntl(0, F_SETFL, 0);
	    } else if (errno == EIO && !die) {
		ret = opts[MONITOR];
		opts[MONITOR] = 1;
		attachtty(mypgrp);
		zrefresh();	/* kludge! */
		opts[MONITOR] = ret;
		die = 1;
	    } else if (errno != 0) {
		zerr("error on TTY read: %e", errno);
		stopmsg = 1;
		zexit(1, ZEXIT_NORMAL);
		/* If called from an exit hook, zexit() returns, so: */
		break;
	    }
	}
	if (cc == '\r')		/* undo the exchange of \n and \r determined by */
	    cc = '\n';		/* zsetterm() */
	else if (cc == '\n')
	    cc = '\r';

	ret = STOUC(cc);
    }
    /*
     * curvichg.buf is raw bytes, not wide characters, so is dealt
     * with here.
     */
    if (vichgflag) {
	if (curvichg.bufptr == curvichg.bufsz)
	    curvichg.buf = realloc(curvichg.buf, curvichg.bufsz *= 2);
	curvichg.buf[curvichg.bufptr++] = ret;
    }
    errno = old_errno;
    return lastchar = ret;
}


/*
 * Get a full character rather than just a single byte.
 */

/**/
mod_export ZLE_INT_T
getfullchar(int do_keytmout)
{
    int inchar = getbyte((long)do_keytmout, NULL, 1);

#ifdef MULTIBYTE_SUPPORT
    return getrestchar(inchar, NULL, NULL);
#else
    return inchar;
#endif
}


/**/
#ifdef MULTIBYTE_SUPPORT
/*
 * Get the remainder of a character if we support multibyte
 * input strings.  It may not require any more input, but
 * we haven't yet checked.  The character previously returned
 * by getbyte() is passed down as inchar.
 */

/**/
mod_export ZLE_INT_T
getrestchar(int inchar, char *outstr, int *outcount)
{
    char c = inchar;
    wchar_t outchar;
    int timeout;
    static mbstate_t mbs;

    /*
     * We are guaranteed to set a valid wide last character,
     * although it may be WEOF (which is technically not
     * a wide character at all...)
     */
    lastchar_wide_valid = 1;

    if (outcount)
	*outcount = 0;
    if (inchar == EOF) {
	/* End of input, so reset the shift state. */
	memset(&mbs, 0, sizeof mbs);
	return lastchar_wide = WEOF;
    }

    /*
     * Return may be zero if we have a NULL; handle this like
     * any other character.
     */
    while (1) {
	size_t cnt = mbrtowc(&outchar, &c, 1, &mbs);
	if (cnt == MB_INVALID) {
	    /*
	     * Invalid input.  Hmm, what's the right thing to do here?
	     */
	    memset(&mbs, 0, sizeof mbs);
	    return lastchar_wide = WEOF;
	}
	if (cnt != MB_INCOMPLETE)
	    break;

	/*
	 * Always apply KEYTIMEOUT to the remains of the input
	 * character.  The parts of a multibyte character should
	 * arrive together.  If we don't do this the input can
	 * get stuck if an invalid byte sequence arrives.
	 */
	inchar = getbyte(1L, &timeout, 1);
	/* getbyte deliberately resets lastchar_wide_valid */
	lastchar_wide_valid = 1;
	if (inchar == EOF) {
	    memset(&mbs, 0, sizeof mbs);
	    if (timeout)
	    {
		/*
		 * This case means that we got a valid initial byte
		 * (since we tested for EOF above), but the followup
		 * timed out.  This probably indicates a duff character.
		 * Return a '?'.
		 */
		lastchar = '?';
		return lastchar_wide = L'?';
	    }
	    else
		return lastchar_wide = WEOF;
	}
	c = inchar;
	if (outstr) {
	    *outstr++ = c;
	    (*outcount)++;
	}
    }
    return lastchar_wide = (ZLE_INT_T)outchar;
}
/**/
#endif

/**/
mod_export void
redrawhook(void)
{
    Thingy initthingy;
    if ((initthingy = rthingy_nocreate("zle-line-pre-redraw"))) {
	/* Duplicating most of zlecallhook() to save additional state */
	int saverrflag = errflag, savretflag = retflag;
	int lastcmd_prev = lastcmd;
	int old_incompfunc = incompfunc;
	int old_viinrepeat = viinrepeat;
	char *args[2];
	Thingy lbindk_save = lbindk, bindk_save = bindk;

	refthingy(lbindk_save);
	refthingy(bindk_save);
	args[0] = initthingy->nam;
	args[1] = NULL;

	/* The generic redraw hook cannot be a completion function, so
	 * temporarily reset state for special variable handling etc.
	 */
	incompfunc = 0;
	execzlefunc(initthingy, args, 1, 0);
	incompfunc = old_incompfunc;
	viinrepeat = old_viinrepeat;

	/* Restore errflag and retflag as zlecallhook() does */
	errflag = saverrflag | (errflag & ERRFLAG_INT);
	retflag = savretflag;

	unrefthingy(initthingy);
	unrefthingy(lbindk);
	unrefthingy(bindk);
	lbindk = lbindk_save;
	bindk = bindk_save;

	/* we can't set ZLE_NOTCOMMAND since it's not a legit widget, so
	 * restore lastcmd manually so that we don't mess up the global state
	 */
	lastcmd = lastcmd_prev;
    }
}

/**/
void
zlecore(void)
{
    Keymap km;
#if !defined(HAVE_POLL) && defined(HAVE_SELECT)
    struct timeval tv;
    fd_set foofd;

    FD_ZERO(&foofd);
#endif

    pushheap();

    /*
     * A widget function may decide to exit the shell.
     * We never exit directly from functions, to allow
     * the shell to tidy up, so we have to test for
     * that explicitly.
     */
    while (!done && !errflag && !exit_pending) {
	UNMETACHECK();

	statusline = NULL;
	vilinerange = 0;
	reselectkeymap();
	selectlocalmap(invicmdmode() && region_active && (km = openkeymap("visual"))
	    ? km : NULL);
	bindk = getkeycmd();
	selectlocalmap(NULL);
	if (bindk) {
	    if (!zlell && isfirstln && !(zlereadflags & ZLRF_IGNOREEOF) &&
		lastchar == eofchar) {
		/*
		 * Slight hack: this relies on getkeycmd returning
		 * a value for the EOF character.  However,
		 * undefined-key is fine.  That's necessary because
		 * otherwise we can't distinguish this case from
		 * a ^C.
		 */
		eofsent = 1;
		break;
	    }
	    if (execzlefunc(bindk, zlenoargs, 0, 0)) {
		handlefeep(zlenoargs);
		if (eofsent)
		    break;
	    }
	    handleprefixes();
	    /* for vi mode, make sure the cursor isn't somewhere illegal */
	    if (invicmdmode() && zlecs > findbol() &&
		(zlecs == zlell || zleline[zlecs] == ZWC('\n')))
		DECCS();
	    handleundo();
	} else {
	    errflag |= ERRFLAG_ERROR;
	    break;
	}

	redrawhook();
#ifdef HAVE_POLL
	if (baud && !(lastcmd & ZLE_MENUCMP)) {
	    struct pollfd pfd;
	    int to = cost * costmult / 1000; /* milliseconds */

	    if (to > 500)
		to = 500;
	    pfd.fd = SHTTY;
	    pfd.events = POLLIN;
	    if (!kungetct && poll(&pfd, 1, to) <= 0)
		zrefresh();
	} else
#else
# ifdef HAVE_SELECT
	if (baud && !(lastcmd & ZLE_MENUCMP)) {
	    FD_SET(SHTTY, &foofd);
	    tv.tv_sec = 0;
	    if ((tv.tv_usec = cost * costmult) > 500000)
		tv.tv_usec = 500000;
	    if (!kungetct && select(SHTTY+1, (SELECT_ARG_2_T) & foofd,
				    NULL, NULL, &tv) <= 0)
		zrefresh();
	} else
# endif
#endif
	    if (!kungetct)
		zrefresh();

	freeheap();

    }

    popheap();
}

/* Read a line.  It is returned metafied.
 *
 * Parameters:
 * - lp: left prompt, e.g., $PS1
 * - rp: right prompt, e.g., $RPS1
 * - flags: ZLRF_* flags (I think), see zlereadflags
 * - context: ZLCON_* flags (I think), see zlecontext
 * - init: "zle-line-init"
 * - finish: "zle-line-finish"
 */

/**/
char *
zleread(char **lp, char **rp, int flags, int context, char *init, char *finish)
{
    char *s, **bracket;
    int old_errno = errno;
    int tmout = getiparam("TMOUT");

#if defined(HAVE_POLL) || defined(HAVE_SELECT)
    /* may not be set, but that's OK since getiparam() returns 0 == off */
    baud = getiparam("BAUD");
    costmult = (baud) ? 3840000L / baud : 0;
#endif

    /* ZLE doesn't currently work recursively.  This is needed in case a *
     * select loop is used in a function called from ZLE.  vared handles *
     * this differently itself.                                          */
    if(zleactive) {
	char *pptbuf;
	int pptlen;

	pptbuf = unmetafy(promptexpand(lp ? *lp : NULL, 0, NULL, NULL,
				       &pmpt_attr),
			  &pptlen);
	write_loop(2, pptbuf, pptlen);
	free(pptbuf);
	return shingetline();
    }
    /*
     * The current status is what we need if we are going
     * to display a prompt.  We'll remember it here for
     * use further in.
     */
    pre_zle_status = lastval;

    keytimeout = (time_t)getiparam("KEYTIMEOUT");
    if (!shout) {
	if (SHTTY != -1)
	    init_shout();

	if (!shout)
	    return NULL;
	/* We could be smarter and default to a system read. */

	/* If we just got a new shout, make sure the terminal is set up. */
	if (termflags & TERM_UNKNOWN)
	    init_term();
    }

    fflush(shout);
    fflush(stderr);
    intr();
    insmode = unset(OVERSTRIKE);
    eofsent = 0;
    resetneeded = 0;
    fetchttyinfo = 0;
    trashedzle = 0;
    raw_lp = lp;
    lpromptbuf = promptexpand(lp ? *lp : NULL, 1, NULL, NULL, &pmpt_attr);
    raw_rp = rp;
    rpmpt_attr = pmpt_attr;
    rpromptbuf = promptexpand(rp ? *rp : NULL, 1, NULL, NULL, &rpmpt_attr);
    free_prepostdisplay();

    zlereadflags = flags;
    zlecontext = context;
    histline = curhist;
    vistartchange = -1;
    zleline = (ZLE_STRING_T)zalloc(((linesz = 256) + 2) * ZLE_CHAR_SIZE);
    *zleline = ZWC('\0');
    virangeflag = lastcmd = done = zlecs = zlell = mark = yankb = yanke = 0;
    vichgflag = 0;
    viinrepeat = 0;
    viinsbegin = 0;
    statusline = NULL;
    selectkeymap("main", 1);
    initundo();
    fixsuffix();
    if ((s = getlinknode(bufstack))) {
	setline(s, ZSL_TOEND);
	zsfree(s);
	if (stackcs != -1) {
	    zlecs = stackcs;
	    stackcs = -1;
	    if (zlecs > zlell)
		zlecs = zlell;
	    CCLEFT();
	}
	if (stackhist != -1) {
	    histline = stackhist;
	    stackhist = -1;
	}
	handleundo();
    }
    /*
     * If main is linked to the viins keymap, we need to register
     * explicitly that we're now in vi insert mode as there's
     * no user operation to indicate this.
     */
    if (openkeymap("main") == openkeymap("viins"))
	viinsert_init();
    selectlocalmap(NULL);
    if (isset(PROMPTCR))
	putc('\r', shout);
    if (tmout)
	alarm(tmout);

    /*
     * On some windowing systems we may enter this function before the
     * terminal is fully opened and sized, resulting in an infinite
     * series of SIGWINCH when the handler prints the prompt before we
     * have done so here.  Therefore, hold any such signal until the
     * first full refresh has completed.  The important bit is that the
     * handler must not see zleactive = 1 until ZLE really is active.
     * See the end of adjustwinsize() in Src/utils.c
     */
    queue_signals();

    zleactive = 1;
    resetneeded = 1;
    /*
     * Start of the main zle read.
     * Fully reset error conditions, including user interrupt.
     */
    errflag = retflag = 0;
    lastcol = -1;
    initmodifier(&zmod);
    prefixflag = 0;
    region_active = 0;

    zrefresh();

    unqueue_signals();	/* Should now be safe to acknowledge SIGWINCH */

    zlecallhook(init, NULL);

    if (zleline && *zleline)
	redrawhook();

    if ((bracket = getaparam("zle_bracketed_paste")) && arrlen(bracket) == 2)
	fputs(*bracket, shout);

    zrefresh();

    zlecore();

    if (errflag)
	setsparam((zlecontext == ZLCON_VARED) ?
		  "ZLE_VARED_ABORTED" :
		  "ZLE_LINE_ABORTED", zlegetline(NULL, NULL));

    if ((bracket = getaparam("zle_bracketed_paste")) && arrlen(bracket) == 2)
	fputs(bracket[1], shout);

    if (done && !exit_pending && !errflag)
	zlecallhook(finish, NULL);

    statusline = NULL;
    invalidatelist();
    trashzle();
    free(lpromptbuf);
    free(rpromptbuf);
    zleactive = zlereadflags = lastlistlen = zlecontext = 0;
    alarm(0);

    freeundo();
    if (eofsent || errflag || exit_pending) {
	s = NULL;
    } else {
	zleline[zlell++] = ZWC('\n');
	s = zlegetline(NULL, NULL);
    }
    free(zleline);
    zleline = NULL;
    forget_edits();
    errno = old_errno;
    /* highlight no longer valid */
    set_region_highlight(NULL, NULL);
    return s;
}

/**/
static int
execimmortal(Thingy func, char **args)
{
    Thingy immortal = rthingy_nocreate(dyncat(".", func->nam));
    if (immortal)
	return execzlefunc(immortal, args, 0, 0);
    return 1;
}

/*
 * Execute a widget.  The third argument indicates that the global
 * variable bindk should be set temporarily so that WIDGET etc.
 * reflect the command being executed.
 */

/**/
int
execzlefunc(Thingy func, char **args, int set_bindk, int set_lbindk)
{
    int r = 0, ret = 0, remetafy = 0;
    int nestedvichg = vichgflag;
    int isrepeat = (viinrepeat == 3);
    Widget w;
    Thingy save_bindk = bindk;
    Thingy save_lbindk = lbindk;

    if (set_bindk)
	bindk = func;
    if (zlemetaline) {
	unmetafy_line();
	remetafy = 1;
    }
    if (set_lbindk)
	refthingy(save_lbindk);
    if (isrepeat)
	viinrepeat = 2;

    if (func->flags & DISABLED) {
	/* this thingy is not the name of a widget */
	char *nm = nicedup(func->nam, 0);
	char *msg = tricat("No such widget `", nm, "'");

	zsfree(nm);
	showmsg(msg);
	zsfree(msg);
	ret = execimmortal(func, args);
    } else if((w = func->widget)->flags & (WIDGET_INT|WIDGET_NCOMP)) {
	int wflags = w->flags;

	/*
	 * The rule is that "zle -N" widgets suppress EOF warnings.  When
	 * a "zle -N" widget invokes "zle another-widget" we pass through
	 * this code again, but with actual arguments rather than with the
	 * zlenoargs placeholder.
	 */
	if (keybuf[0] == eofchar && !keybuf[1] && args == zlenoargs &&
	    !zlell && isfirstln && (zlereadflags & ZLRF_IGNOREEOF)) {
	    showmsg((!islogin) ? "zsh: use 'exit' to exit." :
		    "zsh: use 'logout' to logout.");
	    use_exit_printed = 1;
	    eofsent = 1;
	    ret = 1;
	} else {
	    int inuse = wflags & WIDGET_INUSE;
	    w->flags |= WIDGET_INUSE;
	    if(!(wflags & ZLE_KEEPSUFFIX))
		removesuffix();
	    if(!(wflags & ZLE_MENUCMP)) {
		fixsuffix();
		invalidatelist();
	    }
	    if (wflags & ZLE_LINEMOVE)
		vilinerange = 1;
	    if(!(wflags & ZLE_LASTCOL))
		lastcol = -1;
	    if (wflags & WIDGET_NCOMP) {
		int atcurhist = histline == curhist;
		compwidget = w;
		ret = completecall(args);
		if (atcurhist)
		    histline = curhist;
	    } else if (!w->u.fn) {
		handlefeep(zlenoargs);
	    } else {
		queue_signals();
		ret = w->u.fn(args);
		unqueue_signals();
	    }
	    if (!inuse) {
		if (w->flags & WIDGET_FREE)
		    freewidget(w);
		else
		    w->flags &= ~WIDGET_INUSE;
	    }
	    if (!(wflags & ZLE_NOTCOMMAND))
		lastcmd = wflags;
	}
	r = 1;
    } else {
	Shfunc shf = (Shfunc) shfunctab->getnode(shfunctab, w->u.fnnam);

	if (!shf) {
	    /* the shell function doesn't exist */
	    char *nm = nicedup(w->u.fnnam, 0);
	    char *msg = tricat("No such shell function `", nm, "'");

	    zsfree(nm);
	    showmsg(msg);
	    zsfree(msg);
	    ret = execimmortal(func, args);
	} else {
	    int osc = sfcontext, osi = movefd(0);
	    int oxt = isset(XTRACE);
	    LinkList largs = NULL;
	    int inuse = w->flags & WIDGET_INUSE;
	    w->flags |= WIDGET_INUSE;

	    if (osi > 0) {
		/*
		 * Many commands don't like having a closed stdin, open on
		 * /dev/null instead
		 */
		open("/dev/null", O_RDWR | O_NOCTTY); /* ignore failure */
	    }
	    if (*args) {
		largs = newlinklist();
		addlinknode(largs, dupstring(w->u.fnnam));
		while (*args)
		    addlinknode(largs, dupstring(*args++));
	    }
	    startparamscope();
	    makezleparams(0);
	    sfcontext = SFC_WIDGET;
	    opts[XTRACE] = 0;
	    ret = doshfunc(shf, largs, 1);
	    opts[XTRACE] = oxt;
	    sfcontext = osc;
	    endparamscope();
	    if (errflag == ERRFLAG_ERROR) {
		int saverr = errflag;
		errflag &= ~ERRFLAG_ERROR;
		if ((ret = execimmortal(func, args)) != 0)
		    errflag |= saverr;
	    }
	    lastcmd = w->flags & ~(WIDGET_INUSE|WIDGET_FREE);
	    if (inuse) {
		w->flags &= WIDGET_INUSE|WIDGET_FREE;
	    } else {
		if (w->flags & WIDGET_FREE)
		    freewidget(w);
		else
		    w->flags = 0;
	    }
	    r = 1;
	    redup(osi, 0);
	}
    }
    if (set_lbindk) {
	unrefthingy(lbindk);
	lbindk = save_lbindk;
    } else if (r) {
	unrefthingy(lbindk);
	refthingy(func);
	lbindk = func;
    }
    if (set_bindk)
	bindk = save_bindk;
    /*
     * Goodness knows where the user's left us; make sure
     * it's not on a combining character that won't be displayed
     * directly.
     */
    CCRIGHT();
    if (remetafy)
	metafy_line();

    /* if this widget constituted the vi change, end it */
    if (vichgflag == 2 && !nestedvichg) {
	if (invicmdmode()) {
	    if (ret) {
		free(curvichg.buf);
	    } else {
		if (lastvichg.buf)
		    free(lastvichg.buf);
		lastvichg = curvichg;
	    }
	    vichgflag = 0;
	    curvichg.buf = NULL;
	} else
	    vichgflag = 1; /* vi change continues while in insert mode */
    }
    if (isrepeat)
        viinrepeat = !invicmdmode();

    return ret;
}

/* initialise command modifiers */

/**/
static void
initmodifier(struct modifier *mp)
{
    mp->flags = 0;
    mp->mult = 1;
    mp->tmult = 1;
    mp->vibuf = 0;
    mp->base = 10;
}

/* Reset command modifiers, unless the command just executed was a prefix. *
 * Also set zmult, if the multiplier has been amended.                     */

/**/
static void
handleprefixes(void)
{
    if (prefixflag) {
	prefixflag = 0;
	if(zmod.flags & MOD_TMULT) {
	    zmod.flags |= MOD_MULT;
	    zmod.mult = zmod.tmult;
	}
    } else
	initmodifier(&zmod);
}

/**/
static int
savekeymap(char *cmdname, char *oldname, char *newname, Keymap *savemapptr)
{
    Keymap km = openkeymap(newname);

    if (km) {
	*savemapptr = openkeymap(oldname);
	/* I love special cases */
	if (*savemapptr == km)
	    *savemapptr = NULL;
	else {
	    /* make sure this doesn't get deleted. */
	    if (*savemapptr)
		refkeymap(*savemapptr);
	    linkkeymap(km, oldname, 0);
	}
	return 0;
    } else {
	zwarnnam(cmdname, "no such keymap: %s", newname);
	return 1;
    }
}

/**/
static void
restorekeymap(char *cmdname, char *oldname, char *newname, Keymap savemap)
{
    if (savemap) {
	linkkeymap(savemap, oldname, 0);
	/* we incremented the reference count above */
	unrefkeymap(savemap);
    } else if (newname) {
	/* urr... can this happen? */
	zwarnnam(cmdname,
		 "keymap %s was not defined, not restored", oldname);
    }
}

/* this exports the argument we are currently vared'iting if != NULL */

/**/
mod_export char *varedarg;

/* vared: edit (literally) a parameter value */

/**/
static int
bin_vared(char *name, char **args, Options ops, UNUSED(int func))
{
    char *s, *t, *ova = varedarg;
    struct value vbuf;
    Value v;
    Param pm = 0;
    int ifl;
    int type = PM_SCALAR, obreaks = breaks, haso = 0, oSHTTY = 0;
    int warn_flags;
    char *p1, *p2, *main_keymapname, *vicmd_keymapname, *init, *finish;
    Keymap main_keymapsave = NULL, vicmd_keymapsave = NULL;
    FILE *oshout = NULL;

    if ((interact && unset(USEZLE)) || !strcmp(term, "emacs")) {
	zwarnnam(name, "ZLE not enabled");
	return 1;
    }
    if (zleactive) {
	zwarnnam(name, "ZLE cannot be used recursively (yet)");
	return 1;
    }

    warn_flags = OPT_ISSET(ops, 'g') ? 0 : ASSPM_WARN;
    if (OPT_ISSET(ops,'A'))
    {
	if (OPT_ISSET(ops, 'a'))
	{
	    zwarnnam(name, "specify only one of -a and -A");
	    return 1;
	}
	type = PM_HASHED;
    }
    else if (OPT_ISSET(ops,'a'))
	type = PM_ARRAY;
    p1 = OPT_ARG_SAFE(ops,'p');
    p2 = OPT_ARG_SAFE(ops,'r');
    main_keymapname = OPT_ARG_SAFE(ops,'M');
    vicmd_keymapname = OPT_ARG_SAFE(ops,'m');
    init = OPT_ARG_SAFE(ops,'i');
    finish = OPT_ARG_SAFE(ops,'f');

    if (type != PM_SCALAR && !OPT_ISSET(ops,'c')) {
	zwarnnam(name, "-%s ignored", type == PM_ARRAY ? "a" : "A");
    }

    /* handle non-existent parameter */
    s = args[0];
    queue_signals();
    v = fetchvalue(&vbuf, &s, (!OPT_ISSET(ops,'c') || type == PM_SCALAR),
		   SCANPM_WANTKEYS|SCANPM_WANTVALS|SCANPM_MATCHMANY);
    if (!v && !OPT_ISSET(ops,'c')) {
	unqueue_signals();
	zwarnnam(name, "no such variable: %s", args[0]);
	return 1;
    } else if (v) {
	if (*s) {
	    unqueue_signals();
	    zwarnnam(name, "not an identifier: `%s'", args[0]);
	    return 1;
	}
	if (v->isarr) {
	    /* Array: check for separators and quote them. */
	    char **arr = getarrvalue(v), **aptr, **tmparr, **tptr;
	    tptr = tmparr = (char **)zhalloc(sizeof(char *)*(arrlen(arr)+1));
	    for (aptr = arr; *aptr; aptr++) {
		int sepcount = 0, clen;
		convchar_t c;
		/*
		 * See if this word contains a separator character
		 * or backslash
		 */
		MB_METACHARINIT();
		for (t = *aptr; *t; ) {
		    if (*t == '\\') {
			t++;
			sepcount++;
		    } else {
			t += MB_METACHARLENCONV(t, &c);
			if (WC_ZISTYPE(c, ISEP))
			    sepcount++;
		    }
		}
		if (sepcount) {
		    /* Yes, so allocate enough space to quote it. */
		    char *newstr, *nptr;
		    newstr = zhalloc(strlen(*aptr)+sepcount+1);
		    /* Go through string quoting separators */
		    MB_METACHARINIT();
		    for (t = *aptr, nptr = newstr; *t; ) {
			if (*t == '\\') {
			    *nptr++ = '\\';
			    *nptr++ = *t++;
			} else {
			    clen = MB_METACHARLENCONV(t, &c);
			    if (WC_ZISTYPE(c, ISEP))
				*nptr++ = '\\';
			    while (clen--)
				*nptr++ = *t++;
			}
		    }
		    *nptr = '\0';
		    /* Stick this into the array of words to join up */
		    *tptr++ = newstr;
		} else
		    *tptr++ = *aptr; /* No, keep original array element */
	    }
	    *tptr = NULL;
	    s = sepjoin(tmparr, NULL, 0);
	} else {
	    s = ztrdup(getstrvalue(v));
	}
	unqueue_signals();
    } else if (*s) {
	unqueue_signals();
	zwarnnam(name, "invalid parameter name: %s", args[0]);
	return 1;
    } else {
	unqueue_signals();
	s = ztrdup(s);
    }

    if (SHTTY == -1 || OPT_ISSET(ops,'t')) {
	/* need to open /dev/tty specially */
	oSHTTY = SHTTY;
	if ((SHTTY = open(OPT_ISSET(ops,'t') ? OPT_ARG(ops,'t') : "/dev/tty",
			  O_RDWR|O_NOCTTY)) == -1) {
	    zwarnnam(name, "can't access terminal");
	    zsfree(s);
	    return 1;
	}
	if (!isatty(SHTTY)) {
	    zwarnnam(name, "%s: not a terminal", OPT_ARG(ops,'t'));
	    close(SHTTY);
	    SHTTY = oSHTTY;
	    zsfree(s);
	    return 1;
	}
	oshout = shout;
	init_shout();

	haso = 1;
    }

    /* edit the parameter value */
    zpushnode(bufstack, s);

    if (main_keymapname &&
	savekeymap(name, "main", main_keymapname, &main_keymapsave))
	main_keymapname = NULL;
    if (vicmd_keymapname &&
	savekeymap(name, "vicmd", vicmd_keymapname, &vicmd_keymapsave))
	vicmd_keymapname = NULL;

    varedarg = *args;
    ifl = isfirstln;
    if (OPT_ISSET(ops,'h'))
	hbegin(2);
    isfirstln = OPT_ISSET(ops,'e');

    t = zleread(&p1, &p2, OPT_ISSET(ops,'h') ? ZLRF_HISTORY : 0, ZLCON_VARED,
		init ? init : "zle-line-init",
		finish ? finish : "zle-line-finish");
    if (OPT_ISSET(ops,'h'))
	hend(NULL);
    isfirstln = ifl;
    varedarg = ova;

    restorekeymap(name, "main", main_keymapname, main_keymapsave);
    restorekeymap(name, "vicmd", vicmd_keymapname, vicmd_keymapsave);

    if (haso) {
	fclose(shout);	/* close(SHTTY) */
	shout = oshout;
	SHTTY = oSHTTY;
    }
    if (!t || errflag) {
	/* error in editing */
	errflag &= ~ERRFLAG_ERROR;
	breaks = obreaks;
	if (t)
	    zsfree(t);
	return 1;
    }
    /* strip off trailing newline, if any */
    if (t[strlen(t) - 1] == '\n')
	t[strlen(t) - 1] = '\0';
    /* final assignment of parameter value */
    if (OPT_ISSET(ops,'c')) {
	unsetparam(args[0]);
	createparam(args[0], type);
    }
    queue_signals();
    pm = (Param) paramtab->getnode(paramtab, args[0]);
    if (pm && (PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED))) {
	char **a;

	/*
	 * Use spacesplit with fourth argument 1: identify quoted separators,
	 * and unquote.  This duplicates the string, so we still need to free.
	 */
	a = spacesplit(t, 1, 0, 1);
	zsfree(t);
	if (PM_TYPE(pm->node.flags) == PM_ARRAY)
	    assignaparam(args[0], a, warn_flags);
	else
	    sethparam(args[0], a);
    } else
	assignsparam(args[0], t, warn_flags);
    unqueue_signals();
    return 0;
}

/**/
int
describekeybriefly(UNUSED(char **args))
{
    char *seq, *str, *msg, *is;
    Thingy func;
    Keymap km;

    if (statusline)
	return 1;
    clearlist = 1;
    statusline = "Describe key briefly: _";
    zrefresh();
    if (invicmdmode() && region_active && (km = openkeymap("visual")))
        selectlocalmap(km);
    seq = getkeymapcmd(curkeymap, &func, &str);
    selectlocalmap(NULL);
    statusline = NULL;
    if(!*seq)
	return 1;
    msg = bindztrdup(seq);
    msg = appstr(msg, " is ");
    if (!func)
	is = bindztrdup(str);
    else
	is = nicedup(func->nam, 0);
    msg = appstr(msg, is);
    zsfree(is);
    showmsg(msg);
    zsfree(msg);
    return 0;
}

#define MAXFOUND 4

struct findfunc {
    Thingy func;
    int found;
    char *msg;
};

/**/
static void
scanfindfunc(char *seq, Thingy func, UNUSED(char *str), void *magic)
{
    struct findfunc *ff = magic;

    if(func != ff->func)
	return;
    if (!ff->found++)
	ff->msg = appstr(ff->msg, " is on");
    if(ff->found <= MAXFOUND) {
	char *b = bindztrdup(seq);

	ff->msg = appstr(ff->msg, " ");
	ff->msg = appstr(ff->msg, b);
	zsfree(b);
    }
}

/**/
int
whereis(UNUSED(char **args))
{
    struct findfunc ff;

    if (!(ff.func = executenamedcommand("Where is: ")))
	return 1;
    ff.found = 0;
    ff.msg = nicedup(ff.func->nam, 0);
    scankeymap(curkeymap, 1, scanfindfunc, &ff);
    if (!ff.found)
	ff.msg = appstr(ff.msg, " is not bound to any key");
    else if(ff.found > MAXFOUND)
	ff.msg = appstr(ff.msg, " et al");
    showmsg(ff.msg);
    zsfree(ff.msg);
    return 0;
}

/**/
int
recursiveedit(UNUSED(char **args))
{
    int locerror;
    int q = queue_signal_level();

    ++zle_recursive;

    /* zlecore() expects to be entered with signal queue disabled */
    dont_queue_signals();

    redrawhook();
    zrefresh();
    zlecore();

    restore_queue_signals(q);

    --zle_recursive;

    locerror = errflag ? 1 : 0;
    errflag = done = eofsent = 0;

    return locerror;
}

/**/
void
reexpandprompt(void)
{
    static int reexpanding;
    static int looping;

    if (!reexpanding++) {
	/*
	 * If we're displaying a status in the prompt, it
	 * needs to be the toplevel one, not the one from
	 * any status set within the local zle function.
	 */
	int local_lastval = lastval;
	lastval = pre_zle_status;

	do {
	    /* A new SIGWINCH may arrive while in promptexpand(), causing
	     * looping to increment.  This only happens when a command
	     * substitution is used in a PROMPT_SUBST prompt, but
	     * nevertheless keep trying until we see no more changes.
	     */
	    char *new_lprompt, *new_rprompt;
	    looping = reexpanding;

	    new_lprompt = promptexpand(raw_lp ? *raw_lp : NULL, 1, NULL, NULL,
				       &pmpt_attr);
	    free(lpromptbuf);
	    lpromptbuf = new_lprompt;

	    if (looping != reexpanding)
		continue;

	    rpmpt_attr = pmpt_attr;
	    new_rprompt = promptexpand(raw_rp ? *raw_rp : NULL, 1, NULL, NULL,
				       &rpmpt_attr);
	    free(rpromptbuf);
	    rpromptbuf = new_rprompt;
	} while (looping != reexpanding);

	lastval = local_lastval;
    } else
	looping = reexpanding;
    reexpanding--;
}

/**/
int
resetprompt(UNUSED(char **args))
{
    reexpandprompt();
    return redisplay(NULL);
}

/* same bug called from outside zle */

/**/
mod_export void
zle_resetprompt(void)
{
    reexpandprompt();
    if (zleactive)
        redisplay(NULL);
}


/**/
mod_export void
trashzle(void)
{
    if (zleactive && !trashedzle) {
	/* This zrefresh() is just to get the main editor display right and *
	 * get the cursor in the right place.  For that reason, we disable  *
	 * list display (which would otherwise result in infinite           *
	 * recursion [at least, it would if zrefresh() didn't have its      *
	 * extra `inlist' check]).                                          */
	int sl = showinglist;
	showinglist = 0;
	trashedzle = 1;
	zrefresh();
	showinglist = sl;
	moveto(nlnct, 0);
	if (clearflag && tccan(TCCLEAREOD)) {
	    tcout(TCCLEAREOD);
	    clearflag = listshown = 0;
	}
	if (postedit)
	    fprintf(shout, "%s", unmeta(postedit));
	fflush(shout);
	resetneeded = 1;
	if (!(zlereadflags & ZLRF_NOSETTY))
	  settyinfo(&shttyinfo);
    }
    if (errflag)
	kungetct = 0;
}


/* Hook functions. Used to allow access to zle parameters if zle is
 * active. */

static int
zlebeforetrap(UNUSED(Hookdef dummy), UNUSED(void *dat))
{
    if (zleactive) {
	startparamscope();
	makezleparams(1);
    }
    return 0;
}

static int
zleaftertrap(UNUSED(Hookdef dummy), UNUSED(void *dat))
{
    if (zleactive)
	endparamscope();

    return 0;
}

static char *
zle_main_entry(int cmd, va_list ap)
{
    switch (cmd) {
    case ZLE_CMD_GET_LINE:
    {
	int *ll, *cs;
	ll = va_arg(ap, int *);
	cs = va_arg(ap, int *);
	return zlegetline(ll, cs);
    }

    case ZLE_CMD_READ:
    {
	char **lp, **rp;
	int flags, context;

	lp = va_arg(ap, char **);
	rp = va_arg(ap, char **);
	flags = va_arg(ap, int);
	context = va_arg(ap, int);

	return zleread(lp, rp, flags, context,
		       "zle-line-init", "zle-line-finish");
    }

    case ZLE_CMD_ADD_TO_LINE:
	zleaddtoline(va_arg(ap, int));
	break;

    case ZLE_CMD_TRASH:
	trashzle();
	break;

    case ZLE_CMD_RESET_PROMPT:
	zle_resetprompt();
	break;

    case ZLE_CMD_REFRESH:
	zrefresh();
	break;

    case ZLE_CMD_SET_KEYMAP:
	zlesetkeymap(va_arg(ap, int));
	break;

    case ZLE_CMD_GET_KEY:
    {
	long do_keytmout;
	int *timeout, *chrp;

	do_keytmout = va_arg(ap, long);
	timeout = va_arg(ap, int *);
	chrp = va_arg(ap, int *);
	*chrp = getbyte(do_keytmout, timeout, 0);
	break;
    }

    case ZLE_CMD_SET_HIST_LINE:
    {
	histline = va_arg(ap, zlong);

	break;
    }

    default:
#ifdef DEBUG
	    dputs("Bad command %d in zle_main_entry", cmd);
#endif
	    break;
    }
    return NULL;
}

static struct builtin bintab[] = {
    BUILTIN("bindkey", 0, bin_bindkey, 0, -1, 0, "evaM:ldDANmrsLRp", NULL),
    BUILTIN("vared",   0, bin_vared,   1,  1, 0, "aAcef:ghi:M:m:p:r:t:", NULL),
    BUILTIN("zle",     0, bin_zle,     0, -1, 0, "aAcCDfFgGIKlLmMNrRTUw", NULL),
};

/* The order of the entries in this table has to match the *HOOK
 * macros in zle.h */

/**/
mod_export struct hookdef zlehooks[] = {
    /* LISTMATCHESHOOK */
    HOOKDEF("list_matches", NULL, 0),
    /* COMPLETEHOOK */
    HOOKDEF("complete", NULL, 0),
    /* BEFORECOMPLETEHOOK */
    HOOKDEF("before_complete", NULL, 0),
    /* AFTERCOMPLETEHOOK */
    HOOKDEF("after_complete", NULL, 0),
    /* ACCEPTCOMPHOOK */
    HOOKDEF("accept_completion", NULL, 0),
    /* INVALIDATELISTHOOK */
    HOOKDEF("invalidate_list", NULL, 0),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    char **bpaste;

    /* Set up editor entry points */
    zle_entry_ptr = zle_main_entry;
    zle_load_state = 1;

    /* initialise the thingies */
    init_thingies();
    lbindk = NULL;

    /* miscellaneous initialisations */
    stackhist = stackcs = -1;
    kungetbuf = (char *) zalloc(kungetsz = 32);
    comprecursive = 0;
    rdstrs = NULL;

    /* initialise the keymap system */
    init_keymaps();

    varedarg = NULL;

    incompfunc = incompctlfunc = hascompmod = 0;
    hascompwidgets = 0;

    clwords = (char **) zshcalloc((clwsize = 16) * sizeof(char *));

    bpaste = zshcalloc(3*sizeof(char *));
    bpaste[0] = ztrdup("\033[?2004h");
    bpaste[1] = ztrdup("\033[?2004l");
    /* Intended to be global, no WARNCREATEGLOBAL check. */
    assignaparam("zle_bracketed_paste", bpaste, 0);

    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(Module m)
{
    addhookfunc("before_trap", (Hookfn) zlebeforetrap);
    addhookfunc("after_trap", (Hookfn) zleaftertrap);
    (void)addhookdefs(m, zlehooks, sizeof(zlehooks)/sizeof(*zlehooks));
    zle_refresh_boot();
    return 0;
}

/**/
int
cleanup_(Module m)
{
    if(zleactive) {
	zerrnam(m->node.nam,
		"can't unload the zle module while zle is active");
	return 1;
    }
    deletehookfunc("before_trap", (Hookfn) zlebeforetrap);
    deletehookfunc("after_trap", (Hookfn) zleaftertrap);
    (void)deletehookdefs(m, zlehooks, sizeof(zlehooks)/sizeof(*zlehooks));
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    int i;

    unrefthingy(lbindk);

    cleanup_keymaps();
    deletehashtable(thingytab);

    zfree(lastvichg.buf, lastvichg.bufsz);
    zfree(kungetbuf, kungetsz);
    free_isrch_spots();
    if (rdstrs)
        freelinklist(rdstrs, freestr);
    free(cutbuf.buf);
    if (kring) {
	for(i = kringsize; i--; )
	    free(kring[i].buf);
	zfree(kring, kringsize * sizeof(struct cutbuffer));
    }
    for(i = 36; i--; )
	zfree(vibuf[i].buf, vibuf[i].len);

    /* editor entry points */
    zle_entry_ptr = (ZleEntryPoint)0;
    zle_load_state = 0;

    zfree(clwords, clwsize * sizeof(char *));
    zle_refresh_finish();

    return 0;
}
