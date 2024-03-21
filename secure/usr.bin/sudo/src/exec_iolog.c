/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>

int io_fds[6] = { -1, -1, -1, -1, -1, -1 };

static struct io_buffer_list iobufs = SLIST_HEAD_INITIALIZER(&iobufs);

static sigset_t ttyblock;

/*
 * Remove and free any events associated with the specified
 * file descriptor present in the I/O buffers list.
 */
void
ev_free_by_fd(struct sudo_event_base *evbase, int fd)
{
    struct io_buffer *iob;
    debug_decl(ev_free_by_fd, SUDO_DEBUG_EXEC);

    /* Deschedule any users of the fd and free up the events. */
    SLIST_FOREACH(iob, &iobufs, entries) {
	if (iob->revent != NULL) {
	    if (sudo_ev_get_fd(iob->revent) == fd) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: deleting and freeing revent %p with fd %d",
		    __func__, iob->revent, fd);
		sudo_ev_free(iob->revent);
		iob->revent = NULL;
	    }
	}
	if (iob->wevent != NULL) {
	    if (sudo_ev_get_fd(iob->wevent) == fd) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: deleting and freeing wevent %p with fd %d",
		    __func__, iob->wevent, fd);
		sudo_ev_free(iob->wevent);
		iob->wevent = NULL;
	    }
	}
    }
    debug_return;
}

/*
 * Only close the fd if it is not /dev/tty or std{in,out,err}.
 * Return value is the same as close(2).
 */
int
safe_close(int fd)
{
    debug_decl(safe_close, SUDO_DEBUG_EXEC);

    /* Avoid closing /dev/tty or std{in,out,err}. */
    if (fd < 3 || fd == io_fds[SFD_USERTTY]) {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: not closing fd %d (%s)", __func__, fd, _PATH_TTY);
	errno = EINVAL;
	debug_return_int(-1);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: closing fd %d", __func__, fd);
    debug_return_int(close(fd));
}

/*
 * Allocate a new I/O buffer and associated read/write events.
 */
void
io_buf_new(int rfd, int wfd,
    bool (*action)(const char *, unsigned int, struct io_buffer *),
    void (*read_cb)(int fd, int what, void *v),
    void (*write_cb)(int fd, int what, void *v), struct exec_closure *ec)
{
    int n;
    struct io_buffer *iob;
    debug_decl(io_buf_new, SUDO_DEBUG_EXEC);

    /* Set non-blocking mode. */
    n = fcntl(rfd, F_GETFL, 0);
    if (n != -1 && !ISSET(n, O_NONBLOCK))
	(void) fcntl(rfd, F_SETFL, n | O_NONBLOCK);
    n = fcntl(wfd, F_GETFL, 0);
    if (n != -1 && !ISSET(n, O_NONBLOCK))
	(void) fcntl(wfd, F_SETFL, n | O_NONBLOCK);

    /* Allocate and add to head of list. */
    if ((iob = malloc(sizeof(*iob))) == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    iob->ec = ec;
    iob->revent = sudo_ev_alloc(rfd, SUDO_EV_READ|SUDO_EV_PERSIST,
	read_cb, iob);
    iob->wevent = sudo_ev_alloc(wfd, SUDO_EV_WRITE|SUDO_EV_PERSIST,
	write_cb, iob);
    iob->len = 0;
    iob->off = 0;
    iob->action = action;
    iob->buf[0] = '\0';
    if (iob->revent == NULL || iob->wevent == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    SLIST_INSERT_HEAD(&iobufs, iob, entries);

    debug_return;
}

/*
 * Schedule I/O events before starting the main event loop or
 * resuming from suspend.
 */
void
add_io_events(struct exec_closure *ec)
{
    struct io_buffer *iob;
    debug_decl(add_io_events, SUDO_DEBUG_EXEC);

    /*
     * Schedule all readers as long as the buffer is not full.
     * Schedule writers that contain buffered data.
     * Normally, write buffers are added on demand when data is read.
     */
    SLIST_FOREACH(iob, &iobufs, entries) {
	/* Don't read from /dev/tty if we are not in the foreground. */
	if (iob->revent != NULL &&
	    (ec->term_raw || !USERTTY_EVENT(iob->revent))) {
	    if (iob->len != sizeof(iob->buf)) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "added I/O revent %p, fd %d, events %d",
		    iob->revent, iob->revent->fd, iob->revent->events);
		if (sudo_ev_add(ec->evbase, iob->revent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	}
	if (iob->wevent != NULL) {
	    /* Enable writer if buffer is not empty. */
	    if (iob->len > iob->off) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "added I/O wevent %p, fd %d, events %d",
		    iob->wevent, iob->wevent->fd, iob->wevent->events);
		if (sudo_ev_add(ec->evbase, iob->wevent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	}
    }
    debug_return;
}

/*
 * Flush any output buffered in iobufs or readable from fds other
 * than /dev/tty.  Removes I/O events from the event base when done.
 */
void
del_io_events(bool nonblocking)
{
    struct io_buffer *iob;
    struct sudo_event_base *evbase;
    debug_decl(del_io_events, SUDO_DEBUG_EXEC);

    /* Remove iobufs from existing event base. */
    SLIST_FOREACH(iob, &iobufs, entries) {
	if (iob->revent != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"deleted I/O revent %p, fd %d, events %d",
		iob->revent, iob->revent->fd, iob->revent->events);
	    sudo_ev_del(NULL, iob->revent);
	}
	if (iob->wevent != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"deleted I/O wevent %p, fd %d, events %d",
		iob->wevent, iob->wevent->fd, iob->wevent->events);
	    sudo_ev_del(NULL, iob->wevent);
	}
    }

    /* Create temporary event base for flushing. */
    evbase = sudo_ev_base_alloc();
    if (evbase == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /* Avoid reading from /dev/tty, just flush existing data. */
    SLIST_FOREACH(iob, &iobufs, entries) {
	/* Don't read from /dev/tty while flushing. */
	if (iob->revent != NULL && !USERTTY_EVENT(iob->revent)) {
	    if (iob->len != sizeof(iob->buf)) {
		if (sudo_ev_add(evbase, iob->revent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	}
	/* Flush any write buffers with data in them. */
	if (iob->wevent != NULL) {
	    if (iob->len > iob->off) {
		if (sudo_ev_add(evbase, iob->wevent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	}
    }
    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: flushing remaining I/O buffers (nonblocking)", __func__);
    (void) sudo_ev_loop(evbase, SUDO_EVLOOP_NONBLOCK);

    /*
     * If not in non-blocking mode, make sure we flush write buffers.
     * We don't want to read from the pty or stdin since that might block
     * and the command is no longer running anyway.
     */
    if (!nonblocking) {
	/* Clear out iobufs from event base. */
	SLIST_FOREACH(iob, &iobufs, entries) {
	    if (iob->revent != NULL && !USERTTY_EVENT(iob->revent))
		sudo_ev_del(evbase, iob->revent);
	    if (iob->wevent != NULL)
		sudo_ev_del(evbase, iob->wevent);
	}

	SLIST_FOREACH(iob, &iobufs, entries) {
	    /* Flush any write buffers with data in them. */
	    if (iob->wevent != NULL) {
		if (iob->len > iob->off) {
		    if (sudo_ev_add(evbase, iob->wevent, NULL, false) == -1)
			sudo_fatal("%s", U_("unable to add event to queue"));
		}
	    }
	}
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: flushing remaining write buffers (blocking)", __func__);
	(void) sudo_ev_dispatch(evbase);
     
	/* We should now have flushed all write buffers. */
	SLIST_FOREACH(iob, &iobufs, entries) {
	    if (iob->wevent != NULL) {
		if (iob->len > iob->off) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR,
			"unflushed data: wevent %p, fd %d, events %d",
			iob->wevent, iob->wevent->fd, iob->wevent->events);
		}
	    }
	}
    }

    /* Free temporary event base, removing its events. */
    sudo_ev_base_free(evbase);

    debug_return;
}

/*
 * Free the contents of the I/O buffers queue.
 */
void
free_io_bufs(void)
{
    struct io_buffer *iob;
    debug_decl(free_io_bufs, SUDO_DEBUG_EXEC);

    while ((iob = SLIST_FIRST(&iobufs)) != NULL) {
	SLIST_REMOVE_HEAD(&iobufs, entries);
	if (iob->revent != NULL)
	    sudo_ev_free(iob->revent);
	if (iob->wevent != NULL)
	    sudo_ev_free(iob->wevent);
	free(iob);
    }

    debug_return;
}

/* Call I/O plugin tty input log method. */
bool
log_ttyin(const char *buf, unsigned int n, struct io_buffer *iob)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    bool ret = true;
    debug_decl(log_ttyin, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_ttyin) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_ttyin(buf, n, &errstr);
	    if (rc <= 0) {
		if (rc < 0) {
		    /* Error: disable plugin's I/O function. */
		    plugin->u.io->log_ttyin = NULL;
		    audit_error(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("I/O plugin error"),
			iob->ec->details->info);
		} else {
		    audit_reject(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("command rejected by I/O plugin"),
			iob->ec->details->info);
		}
		ret = false;
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_bool(ret);
}

/* Call I/O plugin stdin log method. */
bool
log_stdin(const char *buf, unsigned int n, struct io_buffer *iob)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    bool ret = true;
    debug_decl(log_stdin, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_stdin) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_stdin(buf, n, &errstr);
	    if (rc <= 0) {
		if (rc < 0) {
		    /* Error: disable plugin's I/O function. */
		    plugin->u.io->log_stdin = NULL;
		    audit_error(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("I/O plugin error"),
			iob->ec->details->info);
		} else {
		    audit_reject(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("command rejected by I/O plugin"),
			iob->ec->details->info);
		}
		ret = false;
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_bool(ret);
}

/* Call I/O plugin tty output log method. */
bool
log_ttyout(const char *buf, unsigned int n, struct io_buffer *iob)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    bool ret = true;
    debug_decl(log_ttyout, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_ttyout) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_ttyout(buf, n, &errstr);
	    if (rc <= 0) {
		if (rc < 0) {
		    /* Error: disable plugin's I/O function. */
		    plugin->u.io->log_ttyout = NULL;
		    audit_error(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("I/O plugin error"),
			iob->ec->details->info);
		} else {
		    audit_reject(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("command rejected by I/O plugin"),
			iob->ec->details->info);
		}
		ret = false;
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    if (!ret) {
	/*
	 * I/O plugin rejected the output, delete the write event
	 * (user's tty) so we do not display the rejected output.
	 */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: deleting and freeing devtty wevent %p", __func__, iob->wevent);
	sudo_ev_free(iob->wevent);
	iob->wevent = NULL;
	iob->off = iob->len = 0;
    }
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_bool(ret);
}

/* Call I/O plugin stdout log method. */
bool
log_stdout(const char *buf, unsigned int n, struct io_buffer *iob)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    bool ret = true;
    debug_decl(log_stdout, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_stdout) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_stdout(buf, n, &errstr);
	    if (rc <= 0) {
		if (rc < 0) {
		    /* Error: disable plugin's I/O function. */
		    plugin->u.io->log_stdout = NULL;
		    audit_error(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("I/O plugin error"),
			iob->ec->details->info);
		} else {
		    audit_reject(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("command rejected by I/O plugin"),
			iob->ec->details->info);
		}
		ret = false;
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    if (!ret) {
	/*
	 * I/O plugin rejected the output, delete the write event
	 * (user's stdout) so we do not display the rejected output.
	 */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: deleting and freeing stdout wevent %p", __func__, iob->wevent);
	sudo_ev_free(iob->wevent);
	iob->wevent = NULL;
	iob->off = iob->len = 0;
    }
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_bool(ret);
}

/* Call I/O plugin stderr log method. */
bool
log_stderr(const char *buf, unsigned int n, struct io_buffer *iob)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    bool ret = true;
    debug_decl(log_stderr, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_stderr) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_stderr(buf, n, &errstr);
	    if (rc <= 0) {
		if (rc < 0) {
		    /* Error: disable plugin's I/O function. */
		    plugin->u.io->log_stderr = NULL;
		    audit_error(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("I/O plugin error"),
			iob->ec->details->info);
		} else {
		    audit_reject(plugin->name, SUDO_IO_PLUGIN,
			errstr ? errstr : _("command rejected by I/O plugin"),
			iob->ec->details->info);
		}
		ret = false;
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    if (!ret) {
	/*
	 * I/O plugin rejected the output, delete the write event
	 * (user's stderr) so we do not display the rejected output.
	 */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: deleting and freeing stderr wevent %p", __func__, iob->wevent);
	sudo_ev_free(iob->wevent);
	iob->wevent = NULL;
	iob->off = iob->len = 0;
    }
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_bool(ret);
}

/* Call I/O plugin suspend log method. */
void
log_suspend(void *v, int signo)
{
    struct exec_closure *ec = v;
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    debug_decl(log_suspend, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->version < SUDO_API_MKVERSION(1, 13))
	    continue;
	if (plugin->u.io->log_suspend) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->log_suspend(signo, &errstr);
	    if (rc <= 0) {
		/* Error: disable plugin's I/O function. */
		plugin->u.io->log_suspend = NULL;
		audit_error(plugin->name, SUDO_IO_PLUGIN,
		    errstr ? errstr : _("error logging suspend"),
		    ec->details->info);
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return;
}

/* Call I/O plugin window change log method. */
void
log_winchange(struct exec_closure *ec, unsigned int rows,
    unsigned int cols)
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    sigset_t omask;
    debug_decl(log_winchange, SUDO_DEBUG_EXEC);

    sigprocmask(SIG_BLOCK, &ttyblock, &omask);
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->version < SUDO_API_MKVERSION(1, 12))
	    continue;
	if (plugin->u.io->change_winsize) {
	    int rc;

	    sudo_debug_set_active_instance(plugin->debug_instance);
	    rc = plugin->u.io->change_winsize(rows, cols, &errstr);
	    if (rc <= 0) {
		/* Error: disable plugin's I/O function. */
		plugin->u.io->change_winsize = NULL;
		audit_error(plugin->name, SUDO_IO_PLUGIN,
		    errstr ? errstr : _("error changing window size"),
		    ec->details->info);
		break;
	    }
	}
    }
    sudo_debug_set_active_instance(sudo_debug_instance);
    sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return;
}

void
init_ttyblock(void)
{
    /* So we can block tty-generated signals */
    sigemptyset(&ttyblock);
    sigaddset(&ttyblock, SIGINT);
    sigaddset(&ttyblock, SIGQUIT);
    sigaddset(&ttyblock, SIGTSTP);
    sigaddset(&ttyblock, SIGTTIN);
    sigaddset(&ttyblock, SIGTTOU);
}
