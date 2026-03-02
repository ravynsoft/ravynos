/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2015, 2017-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/* TCSASOFT is a BSD extension that ignores control flags and speed. */
#ifndef TCSASOFT
# define TCSASOFT	0
#endif

/* Non-standard termios input flags */
#ifndef IUCLC
# define IUCLC		0
#endif
#ifndef IMAXBEL
# define IMAXBEL	0
#endif
#ifndef IUTF8
# define IUTF8	0
#endif

/* Non-standard termios output flags */
#ifndef OLCUC
# define OLCUC	0
#endif
#ifndef ONLCR
# define ONLCR	0
#endif
#ifndef OCRNL
# define OCRNL	0
#endif
#ifndef ONOCR
# define ONOCR	0
#endif
#ifndef ONLRET
# define ONLRET	0
#endif

/* Non-standard termios local flags */
#ifndef XCASE
# define XCASE		0
#endif
#ifndef IEXTEN
# define IEXTEN		0
#endif
#ifndef ECHOCTL
# define ECHOCTL	0
#endif
#ifndef ECHOKE
# define ECHOKE		0
#endif

/* Termios flags to copy between terminals. */
#define INPUT_FLAGS (IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF|IMAXBEL|IUTF8)
#define OUTPUT_FLAGS (OPOST|OLCUC|ONLCR|OCRNL|ONOCR|ONLRET)
#define CONTROL_FLAGS (CS7|CS8|PARENB|PARODD)
#define LOCAL_FLAGS (ISIG|ICANON|XCASE|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH|TOSTOP|IEXTEN|ECHOCTL|ECHOKE)

static struct termios orig_term;
static struct termios cur_term;
static bool changed;

/* tgetpass() needs to know the erase and kill chars for cbreak mode. */
sudo_dso_public int sudo_term_eof;
sudo_dso_public int sudo_term_erase;
sudo_dso_public int sudo_term_kill;

static volatile sig_atomic_t got_sigttou;

/*
 * SIGTTOU signal handler for tcsetattr_nobg() that just sets a flag.
 */
static void
sigttou(int signo)
{
    got_sigttou = 1;
}

/*
 * Like tcsetattr() but restarts on EINTR _except_ for SIGTTOU.
 * Returns 0 on success or -1 on failure, setting errno.
 * Sets got_sigttou on failure if interrupted by SIGTTOU.
 */
static int
tcsetattr_nobg(int fd, int flags, struct termios *tp)
{
    struct sigaction sa, osa;
    int rc;
    debug_decl(tcsetattr_nobg, SUDO_DEBUG_UTIL);

    /*
     * If we receive SIGTTOU from tcsetattr() it means we are
     * not in the foreground process group.
     * This should be less racy than using tcgetpgrp().
     */
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigttou;
    got_sigttou = 0;
    sigaction(SIGTTOU, &sa, &osa);
    do {
	rc = tcsetattr(fd, flags, tp);
    } while (rc == -1 && errno == EINTR && !got_sigttou);
    sigaction(SIGTTOU, &osa, NULL);

    debug_return_int(rc);
}

/*
 * Restore saved terminal settings if we are in the foreground process group.
 * Returns true on success or false on failure.
 */
bool
sudo_term_restore_v1(int fd, bool flush)
{
    const int flags = flush ? (TCSASOFT|TCSAFLUSH) : (TCSASOFT|TCSADRAIN);
    struct termios term = { 0 };
    bool ret = false;
    debug_decl(sudo_term_restore, SUDO_DEBUG_UTIL);

    if (!changed)
	debug_return_bool(true);

    sudo_lock_file(fd, SUDO_LOCK);

    /* Avoid changing term settings if changed out from under us. */
    if (tcgetattr(fd, &term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr(%d)", __func__, fd);
	goto unlock;
    }
    if ((term.c_iflag & INPUT_FLAGS) != (cur_term.c_iflag & INPUT_FLAGS)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: not restoring terminal, "
	    "c_iflag changed; 0x%x, expected 0x%x", __func__,
	    (unsigned int)term.c_iflag, (unsigned int)cur_term.c_iflag);
	/* Not an error. */
	ret = true;
	goto unlock;
    }
    if ((term.c_oflag & OUTPUT_FLAGS) != (cur_term.c_oflag & OUTPUT_FLAGS)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: not restoring terminal, "
	    "c_oflag changed; 0x%x, expected 0x%x", __func__,
	    (unsigned int)term.c_oflag, (unsigned int)cur_term.c_oflag);
	/* Not an error. */
	ret = true;
	goto unlock;
    }
#if !TCSASOFT
    /* Only systems without TCSASOFT make changes to c_cflag. */
    if ((term.c_cflag & CONTROL_FLAGS) != (cur_term.c_cflag & CONTROL_FLAGS)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: not restoring terminal, "
	    "c_cflag changed; 0x%x, expected 0x%x", __func__,
	    (unsigned int)term.c_cflag, (unsigned int)cur_term.c_cflag);
	/* Not an error. */
	ret = true;
	goto unlock;
    }
#endif
    if ((term.c_lflag & LOCAL_FLAGS) != (cur_term.c_lflag & LOCAL_FLAGS)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: not restoring terminal, "
	    "c_lflag changed; 0x%x, expected 0x%x", __func__,
	    (unsigned int)term.c_lflag, (unsigned int)cur_term.c_lflag);
	/* Not an error. */
	ret = true;
	goto unlock;
    }
    if (memcmp(term.c_cc, cur_term.c_cc, sizeof(term.c_cc)) != 0) {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: not restoring terminal, c_cc[] changed", __func__);
	/* Not an error. */
	ret = true;
	goto unlock;
    }

    if (tcsetattr_nobg(fd, flags, &orig_term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcsetattr(%d)", __func__, fd);
	goto unlock;
    }
    cur_term = orig_term;
    changed = false;
    ret = true;

unlock:
    sudo_lock_file(fd, SUDO_UNLOCK);

    debug_return_bool(ret);
}

/*
 * Disable terminal echo.
 * Returns true on success or false on failure.
 */
bool
sudo_term_noecho_v1(int fd)
{
    struct termios term = { 0 };
    bool ret = false;
    debug_decl(sudo_term_noecho, SUDO_DEBUG_UTIL);

    sudo_lock_file(fd, SUDO_LOCK);
    if (!changed && tcgetattr(fd, &orig_term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr(%d)", __func__, fd);
	goto unlock;
    }

    term = orig_term;
    CLR(term.c_lflag, ECHO|ECHONL);
#ifdef VSTATUS
    term.c_cc[VSTATUS] = _POSIX_VDISABLE;
#endif
    if (tcsetattr_nobg(fd, TCSASOFT|TCSADRAIN, &term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcsetattr(%d)", __func__, fd);
	goto unlock;
    }
    cur_term = term;
    changed = true;
    ret = true;

unlock:
    sudo_lock_file(fd, SUDO_UNLOCK);
    debug_return_bool(ret);
}

/*
 * Returns true if term is in raw mode, else false.
 */
static bool
sudo_term_is_raw_int(struct termios *term)
{
    debug_decl(sudo_term_is_raw_int, SUDO_DEBUG_UTIL);

    if (term->c_cc[VMIN] != 1 || term->c_cc[VTIME] != 0)
	debug_return_bool(false);

    if (ISSET(term->c_oflag, OPOST))
	debug_return_bool(false);

    if (ISSET(term->c_oflag, ECHO|ECHONL|ICANON))
	debug_return_bool(false);

    debug_return_bool(true);
}

/*
 * Returns true if fd refers to a tty in raw mode, else false.
 */
bool
sudo_term_is_raw_v1(int fd)
{
    struct termios term = { 0 };
    debug_decl(sudo_term_is_raw, SUDO_DEBUG_UTIL);

    if (!sudo_isatty(fd, NULL))
	debug_return_bool(false);

    sudo_lock_file(fd, SUDO_LOCK);
    if (tcgetattr(fd, &term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr(%d)", __func__, fd);
	sudo_lock_file(fd, SUDO_UNLOCK);
	debug_return_bool(false);
    }
    sudo_lock_file(fd, SUDO_UNLOCK);

    debug_return_bool(sudo_term_is_raw_int(&term));
}

/*
 * Set terminal to raw mode as modified by flags.
 * Returns true on success or false on failure.
 */
bool
sudo_term_raw_v1(int fd, unsigned int flags)
{
    struct termios term = { 0 };
    bool ret = false;
    tcflag_t oflag;
    debug_decl(sudo_term_raw, SUDO_DEBUG_UTIL);

    sudo_lock_file(fd, SUDO_LOCK);
    if (!changed && tcgetattr(fd, &orig_term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr(%d)", __func__, fd);
	goto unlock;
    }

    if (sudo_term_is_raw_int(&term)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: fd %d already in raw mode",
	    __func__, fd);
	ret = true;
	goto unlock;
    }

    /*
     * Set terminal to raw mode but optionally enable terminal signals
     * and/or preserve output flags.
     */
    term = orig_term;
    oflag = term.c_oflag;
    cfmakeraw(&term);
    if (ISSET(flags, SUDO_TERM_ISIG))
	SET(term.c_lflag, ISIG);
    if (ISSET(flags, SUDO_TERM_OFLAG))
	term.c_oflag = oflag;
    if (tcsetattr_nobg(fd, TCSASOFT|TCSADRAIN, &term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcsetattr(%d)", __func__, fd);
	goto unlock;
    }
    cur_term = term;
    changed = true;
    ret = true;

unlock:
    sudo_lock_file(fd, SUDO_UNLOCK);
    debug_return_bool(ret);
}

/*
 * Set terminal to cbreak mode.
 * Returns true on success or false on failure.
 */
bool
sudo_term_cbreak_v1(int fd)
{
    struct termios term = { 0 };
    bool ret = false;
    debug_decl(sudo_term_cbreak, SUDO_DEBUG_UTIL);

    sudo_lock_file(fd, SUDO_LOCK);
    if (!changed && tcgetattr(fd, &orig_term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr(%d)", __func__, fd);
	goto unlock;
    }

    /* Set terminal to half-cooked mode */
    term = orig_term;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    /* cppcheck-suppress redundantAssignment */
    CLR(term.c_lflag, ECHO | ECHONL | ICANON | IEXTEN);
    /* cppcheck-suppress redundantAssignment */
    SET(term.c_lflag, ISIG);
#ifdef VSTATUS
    term.c_cc[VSTATUS] = _POSIX_VDISABLE;
#endif
    if (tcsetattr_nobg(fd, TCSASOFT|TCSADRAIN, &term) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcsetattr(%d)", __func__, fd);
	goto unlock;
    }
    sudo_term_eof = term.c_cc[VEOF];
    sudo_term_erase = term.c_cc[VERASE];
    sudo_term_kill = term.c_cc[VKILL];
    cur_term = term;
    changed = true;
    ret = true;

unlock:
    sudo_lock_file(fd, SUDO_UNLOCK);
    debug_return_bool(ret);
}

/*
 * Copy terminal settings from one descriptor to another.
 * We cannot simply copy the struct termios as src and dst may be
 * different terminal types (pseudo-tty vs. console or glass tty).
 * Returns true on success or false on failure.
 */
bool
sudo_term_copy_v1(int src, int dst)
{
    struct termios tt_src, tt_dst;
    struct winsize wsize;
    speed_t speed;
    unsigned int i;
    bool ret = false;
    debug_decl(sudo_term_copy, SUDO_DEBUG_UTIL);

    sudo_lock_file(src, SUDO_LOCK);
    sudo_lock_file(dst, SUDO_LOCK);
    if (tcgetattr(src, &tt_src) == -1 || tcgetattr(dst, &tt_dst) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcgetattr", __func__);
	goto unlock;
    }

    /* Clear select input, output, control and local flags. */
    CLR(tt_dst.c_iflag, INPUT_FLAGS);
    CLR(tt_dst.c_oflag, OUTPUT_FLAGS);
    CLR(tt_dst.c_cflag, CONTROL_FLAGS);
    CLR(tt_dst.c_lflag, LOCAL_FLAGS);

    /* Copy select input, output, control and local flags. */
    SET(tt_dst.c_iflag, (tt_src.c_iflag & INPUT_FLAGS));
    SET(tt_dst.c_oflag, (tt_src.c_oflag & OUTPUT_FLAGS));
    SET(tt_dst.c_cflag, (tt_src.c_cflag & CONTROL_FLAGS));
    SET(tt_dst.c_lflag, (tt_src.c_lflag & LOCAL_FLAGS));

    /* Copy special chars from src verbatim. */
    for (i = 0; i < NCCS; i++)
	tt_dst.c_cc[i] = tt_src.c_cc[i];

    /* Copy speed from src (zero output speed closes the connection). */
    if ((speed = cfgetospeed(&tt_src)) == B0)
	speed = B38400;
    cfsetospeed(&tt_dst, speed);
    speed = cfgetispeed(&tt_src);
    cfsetispeed(&tt_dst, speed);

    if (tcsetattr_nobg(dst, TCSASOFT|TCSAFLUSH, &tt_dst) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: tcsetattr(%d)", __func__, dst);
	goto unlock;
    }
    ret = true;

    if (ioctl(src, TIOCGWINSZ, &wsize) == 0)
	(void)ioctl(dst, IOCTL_REQ_CAST TIOCSWINSZ, &wsize);

unlock:
    sudo_lock_file(dst, SUDO_UNLOCK);
    sudo_lock_file(src, SUDO_UNLOCK);
    debug_return_bool(ret);
}

/*
 * Like isatty(3) but stats the fd and stores the result in sb.
 * Only calls isatty(3) if fd is a character special device.
 * Returns true if a tty, else returns false and sets errno.
 */
bool
sudo_isatty_v1(int fd, struct stat *sbp)
{
    bool ret = false;
    struct stat sb;
    debug_decl(sudo_isatty, SUDO_DEBUG_EXEC);

    if (sbp == NULL)
	sbp = &sb;

    if (fstat(fd, sbp) == 0) {
        if (!S_ISCHR(sbp->st_mode)) {
            errno = ENOTTY;
        } else {
            ret = isatty(fd) == 1;
        }
    } else if (sbp != &sb) {
        /* Always initialize sbp. */
        memset(sbp, 0, sizeof(*sbp));
    }
    debug_return_bool(ret);
}
