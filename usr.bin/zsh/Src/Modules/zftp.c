/*
 * zftp.c - builtin FTP client
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1998 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

/*
 * TODO:
 *   should be eight-bit clean, but isn't.
 *   tracking of logical rather than physical directories, like nochaselinks
 *     (usually PWD returns physical directory).
 *   can signal handling be improved?
 *   maybe we should block CTRL-c on some more operations,
 *     otherwise you can get the connection closed prematurely.
 *   some way of turning off progress reports when backgrounded
 *     would be nice, but the shell doesn't make it easy to find that out.
 *   proxy/gateway connections if i knew what to do
 *   options to specify e.g. a non-standard port
 */

/* needed in prototypes for statics */
struct hostent;
struct in_addr;
struct sockaddr_in;
struct sockaddr_in6;
struct zftp_session;
typedef struct zftp_session *Zftp_session;

#include "tcp.h"
#include "zftp.mdh"
#include "zftp.pro"

/* it's a TELNET based protocol, but don't think I like doing this */
#include <arpa/telnet.h>

/*
 * We use poll() in preference to select because some subset of manuals says
 * that's the thing to do, plus it's a bit less fiddly.  I don't actually
 * have access to a system with poll but not select, however, though
 * both bits of the code have been tested on a machine with both.
 */
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#if defined(HAVE_POLL) && !defined(POLLIN) && !defined(POLLNORM)
# undef HAVE_POLL
#endif


#ifdef USE_LOCAL_H_ERRNO
int h_errno;
#endif

union zftp_sockaddr {
    struct sockaddr a;
    struct sockaddr_in in;
#ifdef SUPPORT_IPV6
    struct sockaddr_in6 in6;
#endif
};

#ifdef USE_LOCAL_H_ERRNO
int h_errno;
#endif

/*
 * For FTP block mode
 *
 * The server on our AIX machine here happily accepts block mode, takes the
 * first connection, then at the second complains that it's got nowhere
 * to send data.  The same problem happens with ncftp, it's not just
 * me.  And a lot of servers don't even support block mode. So I'm not sure
 * how widespread the supposed ability to leave open the data fd between
 * transfers.  Therefore, I've closed all connections after the transfer.
 * But then what's the point in block mode?  I only implemented it because
 * it says in RFC959 that you need it to be able to restart transfers
 * later in the file.  However, it turns out that's not true for
 * most servers --- but our AIX machine happily accepts the REST
 * command and then dumps the whole file onto you.  Sigh.
 *
 * Note on block sizes:
 * Not quite sure how to optimize this:  in principle
 * we should handle blocks up to 65535 bytes, which
 * is pretty big, and should presumably send blocks
 * which are smaller to be on the safe side.
 * Currently we send 32768 and use that also as
 * the maximum to receive.  No-one's complained yet.  Of course,
 * no-one's *used* it yet apart from me, but even so.
 */

struct zfheader {
    char flags;
    unsigned char bytes[2];
};

enum {
    ZFHD_MARK = 16,		/* restart marker */
    ZFHD_ERRS = 32,		/* suspected errors in block */
    ZFHD_EOFB = 64,		/* block is end of record */
    ZFHD_EORB = 128		/* block is end of file */
};

typedef int (*readwrite_t)(int, char *, off_t, int);

struct zftpcmd {
    const char *nam;
    int (*fun) _((char *, char **, int));
    int min, max, flags;
};

enum {
    ZFTP_CONN  = 0x0001,	/* must be connected */
    ZFTP_LOGI  = 0x0002,	/* must be logged in */
    ZFTP_TBIN  = 0x0004,	/* set transfer type image */
    ZFTP_TASC  = 0x0008,	/* set transfer type ASCII */
    ZFTP_NLST  = 0x0010,	/* use NLST rather than LIST */
    ZFTP_DELE  = 0x0020,	/* a delete rather than a make */
    ZFTP_SITE  = 0x0040,	/* a site rather than a quote */
    ZFTP_APPE  = 0x0080,	/* append rather than overwrite */
    ZFTP_HERE  = 0x0100,	/* here rather than over there */
    ZFTP_CDUP  = 0x0200,	/* CDUP rather than CWD */
    ZFTP_REST  = 0x0400,	/* restart: set point in remote file */
    ZFTP_RECV  = 0x0800,	/* receive rather than send */
    ZFTP_TEST  = 0x1000,	/* test command, don't test */
    ZFTP_SESS  = 0x2000		/* session command, don't need status */
};

typedef struct zftpcmd *Zftpcmd;

static struct zftpcmd zftpcmdtab[] = {
    { "open", zftp_open, 0, 4, 0 },
    { "params", zftp_params, 0, 4, 0 },
    { "login", zftp_login, 0, 3, ZFTP_CONN },
    { "user", zftp_login, 0, 3, ZFTP_CONN },
    { "test", zftp_test, 0, 0, ZFTP_TEST },
    { "cd", zftp_cd, 1, 1, ZFTP_CONN|ZFTP_LOGI },
    { "cdup", zftp_cd, 0, 0, ZFTP_CONN|ZFTP_LOGI|ZFTP_CDUP },
    { "dir", zftp_dir, 0, -1, ZFTP_CONN|ZFTP_LOGI },
    { "ls", zftp_dir, 0, -1, ZFTP_CONN|ZFTP_LOGI|ZFTP_NLST },
    { "type", zftp_type, 0, 1, ZFTP_CONN|ZFTP_LOGI },
    { "ascii", zftp_type, 0, 0, ZFTP_CONN|ZFTP_LOGI|ZFTP_TASC },
    { "binary", zftp_type, 0, 0, ZFTP_CONN|ZFTP_LOGI|ZFTP_TBIN },
    { "mode", zftp_mode, 0, 1, ZFTP_CONN|ZFTP_LOGI },
    { "local", zftp_local, 0, -1, ZFTP_HERE },
    { "remote", zftp_local, 1, -1, ZFTP_CONN|ZFTP_LOGI },
    { "get", zftp_getput, 1, -1, ZFTP_CONN|ZFTP_LOGI|ZFTP_RECV },
    { "getat", zftp_getput, 2, 2, ZFTP_CONN|ZFTP_LOGI|ZFTP_RECV|ZFTP_REST },
    { "put", zftp_getput, 1, -1, ZFTP_CONN|ZFTP_LOGI },
    { "putat", zftp_getput, 2, 2, ZFTP_CONN|ZFTP_LOGI|ZFTP_REST },
    { "append", zftp_getput, 1, -1, ZFTP_CONN|ZFTP_LOGI|ZFTP_APPE },
    { "appendat", zftp_getput, 2, 2, ZFTP_CONN|ZFTP_LOGI|ZFTP_APPE|ZFTP_REST },
    { "delete", zftp_delete, 1, -1, ZFTP_CONN|ZFTP_LOGI },
    { "mkdir", zftp_mkdir, 1, 1, ZFTP_CONN|ZFTP_LOGI },
    { "rmdir", zftp_mkdir, 1, 1, ZFTP_CONN|ZFTP_LOGI|ZFTP_DELE },
    { "rename", zftp_rename, 2, 2, ZFTP_CONN|ZFTP_LOGI },
    { "quote", zftp_quote, 1, -1, ZFTP_CONN },
    { "site", zftp_quote, 1, -1, ZFTP_CONN|ZFTP_SITE },
    { "close", zftp_close, 0, 0, ZFTP_CONN },
    { "quit", zftp_close, 0, 0, ZFTP_CONN },
    { "session", zftp_session, 0, 1, ZFTP_SESS },
    { "rmsession", zftp_rmsession, 0, 1, ZFTP_SESS },
    { 0, 0, 0, 0, 0 }
};

static struct builtin bintab[] = {
    BUILTIN("zftp", 0, bin_zftp, 1, -1, 0, NULL, NULL),
};

/*
 * these are the non-special params to unset when a connection
 * closes.  any special params are handled, well, specially.
 * currently there aren't any, which is the way I like it.
 */
static char *zfparams[] = {
    "ZFTP_HOST", "ZFTP_PORT", "ZFTP_IP", "ZFTP_SYSTEM", "ZFTP_USER",
    "ZFTP_ACCOUNT", "ZFTP_PWD", "ZFTP_TYPE", "ZFTP_MODE", NULL
};

/* flags for zfsetparam */

enum {
    ZFPM_READONLY = 0x01,	/* make parameter readonly */
    ZFPM_IFUNSET  = 0x02,	/* only set if not already set */
    ZFPM_INTEGER  = 0x04	/* passed pointer to off_t */
};

/* Number of connections actually open */
static int zfnopen;

/*
 * zcfinish = 0 keep going
 *            1 line finished, alles klar
 *            2 EOF
 */
static int zcfinish;
/* zfclosing is set if zftp_close() is active */
static int zfclosing;

/*
 * Stuff about last message:  last line of message and status code.
 * The reply is also stored in $ZFTP_REPLY; we keep these separate
 * for convenience.
 */
static char *lastmsg, lastcodestr[4];
static int lastcode;

/* remote system has size, mdtm commands */
enum {
    ZFCP_UNKN = 0,		/* dunno if it works on this server */
    ZFCP_YUPP = 1,		/* it does */
    ZFCP_NOPE = 2		/* it doesn't */
};

/*
 * We keep an fd open for communication between the main shell
 * and forked off bits and pieces.  This allows us to know
 * if something happened in a subshell:  mode changed, type changed,
 * connection was closed.  If something too substantial happened
 * in a subshell --- connection opened, ZFTP_USER and ZFTP_PWD changed
 * --- we don't try to track it because it's too complicated.
 */
enum {
    ZFST_ASCI = 0x0000,		/* type for next transfer is ASCII */
    ZFST_IMAG = 0x0001,		/* type for next transfer is image */

    ZFST_TMSK = 0x0001,		/* mask for type flags */
    ZFST_TBIT = 0x0001,		/* number of bits in type flags */

    ZFST_CASC = 0x0000,		/* current type is ASCII - default */
    ZFST_CIMA = 0x0002,		/* current type is image */

    ZFST_STRE = 0x0000,		/* stream mode - default */
    ZFST_BLOC = 0x0004,		/* block mode */

    ZFST_MMSK = 0x0004,		/* mask for mode flags */

    ZFST_LOGI = 0x0008,		/* user logged in */
    ZFST_SYST = 0x0010,		/* done system type check */
    ZFST_NOPS = 0x0020,		/* server doesn't understand PASV */
    ZFST_NOSZ = 0x0040,		/* server doesn't send `(XXXX bytes)' reply */
    ZFST_TRSZ = 0x0080,		/* tried getting 'size' from reply */
    ZFST_CLOS = 0x0100		/* connection closed */
};
#define ZFST_TYPE(x) (x & ZFST_TMSK)
/*
 * shift current type flags to match type flags: should be by
 * the number of bits in the type flags
 */
#define ZFST_CTYP(x) ((x >> ZFST_TBIT) & ZFST_TMSK)
#define ZFST_MODE(x) (x & ZFST_MMSK)

/* fd containing status for all sessions and array for internal use */
static int zfstatfd = -1, *zfstatusp;

/* Preferences, read in from the `zftp_prefs' array variable */
enum {
    ZFPF_SNDP = 0x01,		/* Use send port mode */
    ZFPF_PASV = 0x02,		/* Try using passive mode */
    ZFPF_DUMB = 0x04		/* Don't do clever things with variables */
};

/* The flags as stored internally. */
static int zfprefs;

/*
 * Data node for linked list of sessions.
 *
 * Memory management notes:
 *   name is permanently allocated and remains for the life of the node.
 *   userparams is set directly by zftp_params and also freed with the node.
 *   params and its data are allocated when we need
 *     to save an existing session, and are freed when we switch back
 *     to that session.
 *   The node itself is deleted when we remove it from the list.
 */
struct zftp_session {
    char *name;			/* name of session */
    char **params;		/* parameters ordered as in zfparams */
    char **userparams;		/* user parameters set by zftp_params */
    FILE *cin;			/* control input file */
    Tcp_session control;	/* the control connection */
    int dfd;			/* data connection */
    int has_size;		/* understands SIZE? */
    int has_mdtm;		/* understands MDTM? */
};

/* List of active sessions */
static LinkList zfsessions;

/* Current session */
static Zftp_session zfsess;

/* Number of current session, corresponding to position in list */
static int zfsessno;

/* Total number of sessions */
static int zfsesscnt;

/*
 * Bits and pieces for dealing with SIGALRM (and SIGPIPE, but that's
 * easier).  The complication is that SIGALRM may already be handled
 * by the user setting TMOUT and possibly setting their own trap --- in
 * fact, it's always handled by the shell when it's interactive.  It's
 * too difficult to use zsh's own signal handler --- either it would
 * need rewriting to use a C function as a trap, or we would need a
 * hack to make it callback via a hidden builtin from a function --- so
 * just install our own, and use settrap() to restore the behaviour
 * afterwards if necessary.  However, the more that could be done by
 * the main shell code, the better I would like it.
 *
 * Since we don't want to go through the palaver of changing between
 * the main zsh signal handler and ours every time we start or stop the
 * alarm, we keep the flag zfalarmed set to 1 while zftp is rigged to
 * handle alarms.  This is tested at the end of bin_zftp(), which is
 * the entry point for all functions, and that restores the original
 * handler for SIGALRM.  To turn off the alarm temporarily in the zftp
 * code we then just call alarm(0).
 *
 * If we could rely on having select() or some replacement, we would
 * only need the alarm during zftp_open().
 */

/* flags for alarm set, alarm gone off */
static int zfalarmed, zfdrrrring;
/* remember old alarm status */
static time_t oaltime;
static unsigned int oalremain;

/*
 * Where to jump to when the alarm goes off.  This is much
 * easier than fiddling with error flags at every turn.
 * Since we don't expect too many alarm's, the simple setjmp()
 * mechanism should be good enough.
 *
 * gcc -O gives apparently spurious `may be clobbered by longjmp' warnings.
 */
static jmp_buf zfalrmbuf;

/* The signal handler itself */

/**/
static void
zfhandler(int sig)
{
    if (sig == SIGALRM) {
	zfdrrrring = 1;
#ifdef ETIMEDOUT		/* just in case */
	errno = ETIMEDOUT;
#else
	errno = EIO;
#endif
	longjmp(zfalrmbuf, 1);
    }
    DPUTS(1, "zfhandler caught incorrect signal");
}

/* Set up for an alarm call */

/**/
static void
zfalarm(int tmout)
{
    zfdrrrring = 0;
    /*
     * We need to do this even if tmout is zero, since there may
     * be a non-zero timeout set in the main shell which we don't
     * want to go off.  This could be argued the other way, since
     * if we don't get that it's probably harmless.  But this looks safer.
     */
    if (zfalarmed) {
	alarm(tmout);
	return;
    }
    signal(SIGALRM, zfhandler);
    oalremain = alarm(tmout);
    if (oalremain)
	oaltime = time(NULL);
    /*
     * We'll leave sigtrapped, sigfuncs and TRAPXXX as they are; since the
     * shell's handler doesn't get the signal, they don't matter.
     */
    zfalarmed = 1;
}

/* Set up for a broken pipe */

/**/
static void
zfpipe(void)
{
    /* Just ignore SIGPIPE and rely on getting EPIPE from the write. */
    signal(SIGPIPE, SIG_IGN);
}

/* Unset the alarm, see above */

/**/
static void
zfunalarm(void)
{
    if (oalremain) {
	/*
	 * The alarm was previously set, so set it back, adjusting
	 * for the time used.  Mostly the alarm was set to zero
	 * beforehand, so it would probably be best to reinstall
	 * the proper signal handler before resetting the alarm.
	 *
	 * I love the way alarm() uses unsigned int while time_t
	 * is probably something completely different.
	 */
	unsigned int tdiff = time(NULL) - oaltime;
	alarm(oalremain < tdiff ? 1 : oalremain - tdiff);
    } else
	alarm(0);
    if (sigtrapped[SIGALRM] || interact) {
	if (siglists[SIGALRM] || !sigtrapped[SIGALRM] ||
	    (sigtrapped[SIGALRM] & ZSIG_FUNC))
	    install_handler(SIGALRM);
	else
	    signal_ignore(SIGALRM);
    } else
	signal_default(SIGALRM);
    zfalarmed = 0;
}

/* Restore SIGPIPE handling to its usual status */

/**/
static void
zfunpipe(void)
{
    if (sigtrapped[SIGPIPE]) {
	if (siglists[SIGPIPE] || (sigtrapped[SIGPIPE] & ZSIG_FUNC))
	    install_handler(SIGPIPE);
	else
	    signal_ignore(SIGPIPE);
    } else
	signal_default(SIGPIPE);
}

/*
 * Same as movefd(), but don't mark the fd in the zsh tables,
 * because we only want it closed by zftp.  However, we still
 * need to shift the fd's out of the way of the user-visible 0-9.
 */

/**/
static int
zfmovefd(int fd)
{
    if (fd != -1 && fd < 10) {
#ifdef F_DUPFD
	int fe = fcntl(fd, F_DUPFD, 10);
#else
	int fe = zfmovefd(dup(fd));
#endif
	close(fd);
	fd = fe;
    }
    return fd;
}

/*
 * set a non-special parameter.
 * if ZFPM_IFUNSET, don't set if it already exists.
 * if ZFPM_READONLY, make it readonly, but only when creating it.
 * if ZFPM_INTEGER, val pointer is to off_t (NB not int), don't free.
 */
/**/
static void
zfsetparam(char *name, void *val, int flags)
{
    Param pm = NULL;
    int type = (flags & ZFPM_INTEGER) ? PM_INTEGER : PM_SCALAR;

    if (!(pm = (Param) paramtab->getnode(paramtab, name))
	|| (pm->node.flags & PM_UNSET)) {
	/*
	 * just make it readonly when creating, in case user
	 * *really* knows what they're doing
	 */
	if ((pm = createparam(name, type)) && (flags & ZFPM_READONLY))
	    pm->node.flags |= PM_READONLY;
    } else if (flags & ZFPM_IFUNSET) {
	pm = NULL;
    }
    if (!pm || PM_TYPE(pm->node.flags) != type) {
	/* parameters are funny, you just never know */
	if (type == PM_SCALAR)
	    zsfree((char *)val);
	return;
    }
    if (type == PM_INTEGER)
	pm->gsu.i->setfn(pm, *(off_t *)val);
    else
	pm->gsu.s->setfn(pm, (char *)val);
}

/*
 * Unset a ZFTP parameter when the connection is closed.
 * We only do this with connection-specific parameters.
 */

/**/
static void
zfunsetparam(char *name)
{
    Param pm;

    if ((pm = (Param) paramtab->getnode(paramtab, name))) {
	pm->node.flags &= ~PM_READONLY;
	unsetparam_pm(pm, 0, 1);
    }
}

/*
 * Join command and arguments to make a proper TELNET command line.
 * New line is in permanent storage.
 */

/**/
static char *
zfargstring(char *cmd, char **args)
{
    int clen = strlen(cmd) + 3;
    char *line, **aptr;

    for (aptr = args; *aptr; aptr++)
	clen += strlen(*aptr) + 1;
    line = zalloc(clen);
    strcpy(line, cmd);
    for (aptr = args; *aptr; aptr++) {
	strcat(line, " ");
	strcat(line, *aptr);
    }
    strcat(line, "\r\n");

    return line;
}

/*
 * get a line on the control connection according to TELNET rules
 * Return status is first digit of FTP reply code
 */

/**/
static int
zfgetline(char *ln, int lnsize, int tmout)
{
    int ch, added = 0;
    /* current line point */
    char *pcur = ln, cmdbuf[3];

    zcfinish = 0;
    /* leave room for null byte */
    lnsize--;
    /* in case we return unexpectedly before getting anything */
    ln[0] = '\0';

    if (setjmp(zfalrmbuf)) {
	alarm(0);
	zwarnnam("zftp", "timeout getting response");
	return 6;
    }
    zfalarm(tmout);

    /*
     * We need to be more careful about errors here; we
     * should do the stuff with errflag and so forth.
     * We should probably holdintr() here, since if we don't
     * get the message, the connection is going to be messed up.
     * But then we get `frustrated user syndrome'.
     */
    for (;;) {
	ch = fgetc(zfsess->cin);

	switch(ch) {
	case EOF:
	    if (ferror(zfsess->cin) && errno == EINTR) {
		clearerr(zfsess->cin);
		continue;
	    }
	    zcfinish = 2;
	    break;

	case '\r':
	    /* always precedes something else */
	    ch = fgetc(zfsess->cin);
	    if (ch == EOF) {
		zcfinish = 2;
		break;
	    }
	    if (ch == '\n') {
		zcfinish = 1;
		break;
	    }
	    if (ch == '\0') {
		ch = '\r';
		break;
	    }
	    /* not supposed to get here */
	    ch = '\r';
	    break;

	case '\n':
	    /* not supposed to get here */
	    zcfinish = 1;
	    break;

	case IAC:
	    /*
	     * oh great, now it's sending TELNET commands.  try
	     * to persuade it not to.
	     */
	    ch = fgetc(zfsess->cin);
	    switch (ch) {
	    case WILL:
	    case WONT:
		ch = fgetc(zfsess->cin);
		/* whatever it wants to do, stop it. */
		cmdbuf[0] = (char)IAC;
		cmdbuf[1] = (char)DONT;
		cmdbuf[2] = ch;
		write_loop(zfsess->control->fd, cmdbuf, 3);
		continue;

	    case DO:
	    case DONT:
		ch = fgetc(zfsess->cin);
		/* well, tough, we're not going to. */
		cmdbuf[0] = (char)IAC;
		cmdbuf[1] = (char)WONT;
		cmdbuf[2] = ch;
		write_loop(zfsess->control->fd, cmdbuf, 3);
		continue;

	    case EOF:
		/* strange machine. */
		zcfinish = 2;
		break;

	    default:
		break;
	    }
	    break;
	}
	
	if (zcfinish)
	    break;
	if (added < lnsize) {
	    *pcur++ = ch;
	    added++;
	}
	/* junk it if we don't have room, but go on reading */
    }

    alarm(0);

    *pcur = '\0';
    /* if zcfinish == 2, at EOF, return that, else 0 */
    return (zcfinish & 2);
}

/*
 * Get a whole message from the server.  A dash after
 * the first line code means keep reading until we get
 * a line with the same code followed by a space.
 *
 * Note that this returns an FTP status code, the first
 * digit of the reply.  There is also a pseudocode, 6, which
 * means `there's no point trying anything, just yet'.
 * We return it either if the connection is closed, or if
 * we got a 530 (user not logged in), in which case whatever
 * you're trying to do isn't going to work.
 */

/**/
static int 
zfgetmsg(void)
{
    char line[256], *ptr, *verbose;
    int stopit, printing = 0, tmout;

    if (!zfsess->control)
	return 6;
    zsfree(lastmsg);
    lastmsg = NULL;

    tmout = getiparam("ZFTP_TMOUT");

    zfgetline(line, 256, tmout);
    ptr = line;
    if (zfdrrrring || !idigit(*ptr) || !idigit(ptr[1]) || !idigit(ptr[2])) {
	/* timeout, or not talking FTP.  not really interested. */
	zcfinish = 2;
	if (!zfclosing)
	    zfclose(0);
	lastmsg = ztrdup("");
	strcpy(lastcodestr, "000");
	zfsetparam("ZFTP_REPLY", ztrdup(lastmsg), ZFPM_READONLY);
	return 6;
    }
    strncpy(lastcodestr, ptr, 3);
    ptr += 3;
    lastcodestr[3] = '\0';
    lastcode = atoi(lastcodestr);
    zfsetparam("ZFTP_CODE", ztrdup(lastcodestr), ZFPM_READONLY);
    stopit = (*ptr++ != '-');

    queue_signals();
    if (!(verbose = getsparam_u("ZFTP_VERBOSE")))
	verbose = "";
    if (strchr(verbose, lastcodestr[0])) {
	/* print the whole thing verbatim */
	printing = 1;
	fputs(line, stderr);
    }  else if (strchr(verbose, '0') && !stopit) {
	/* print multiline parts with the code stripped */
	printing = 2;
	fputs(ptr, stderr);
    }
    unqueue_signals();
    if (printing)
	fputc('\n', stderr);

    while (zcfinish != 2 && !stopit) {
	zfgetline(line, 256, tmout);
	ptr = line;
	if (zfdrrrring) {
	    line[0] = '\0';
	    break;
	}

	if (!strncmp(lastcodestr, line, 3)) {
	    if (line[3] == ' ') {
		stopit = 1;
		ptr += 4;
	    } else if (line[3] == '-')
		ptr += 4;
	} else if (!strncmp("    ", line, 4))
	    ptr += 4;

	if (printing == 2) {
	    if (!stopit) {
		fputs(ptr, stderr);
		fputc('\n', stderr);
	    }
	} else if (printing) {
	    fputs(line, stderr);
	    fputc('\n', stderr);
	}
    }

    if (printing)
	fflush(stderr);

    /* The internal message is just the text. */
    lastmsg = ztrdup(ptr);
    /*
     * The parameter is the whole thing, including the code.
     */
    zfsetparam("ZFTP_REPLY", ztrdup(line), ZFPM_READONLY);
    /*
     * close the connection here if zcfinish == 2, i.e. EOF,
     * or if we get a 421 (service not available, closing connection),
     * but don't do it if it's expected (zfclosing set).
     */
    if ((zcfinish == 2 || lastcode == 421) && !zfclosing) {
	zcfinish = 2;		/* don't need to tell server */
	zfclose(0);
	/* unexpected, so tell user */
	zwarnnam("zftp", "remote server has closed connection");
	return 6;
    }
    if (lastcode == 530) {
	/* user not logged in */
	return 6;
    }
    /*
     * May as well handle this here, though it's pretty uncommon.
     * A 120 is something like "service ready in nnn minutes".
     * It means we just hang around waiting for another reply.
     */
    if (lastcode == 120) {
	zwarnnam("zftp", "delay expected, waiting: %s", lastmsg);
	return zfgetmsg();
    }

    /* first digit of code determines success, failure, not in the mood... */
    return lastcodestr[0] - '0';
}


/*
 * Send a command and get the reply.
 * The command is expected to have the \r\n already tacked on.
 * Returns the status code for the reply.
 */

/**/
static int
zfsendcmd(char *cmd)
{
    /*
     * We use the fd directly; there's no point even using
     * stdio with line buffering, since we always send the
     * complete line in one string anyway.
     */
    int ret, tmout;

    if (!zfsess->control)
	return 6;
    tmout = getiparam("ZFTP_TMOUT");
    if (setjmp(zfalrmbuf)) {
	alarm(0);
	zwarnnam("zftp", "timeout sending message");
	return 6;
    }
    zfalarm(tmout);
    ret = write(zfsess->control->fd, cmd, strlen(cmd));
    alarm(0);

    if (ret <= 0) {
	zwarnnam("zftp send", "failure sending control message: %e", errno);
	return 6;
    }

    return zfgetmsg();
}


/* Set up a data connection, return 1 for failure, 0 for success */

/**/
static int
zfopendata(char *name, union tcp_sockaddr *zdsockp, int *is_passivep)
{
    if (!(zfprefs & (ZFPF_SNDP|ZFPF_PASV))) {
	zwarnnam(name, "Must set preference S or P to transfer data");
	return 1;
    }
    zfsess->dfd = socket(zfsess->control->peer.a.sa_family, SOCK_STREAM, 0);
    if (zfsess->dfd < 0) {
	zwarnnam(name, "can't get data socket: %e", errno);
	return 1;
    }

    if (!(zfstatusp[zfsessno] & ZFST_NOPS) && (zfprefs & ZFPF_PASV)) {
	char *psv_cmd;
	int err, salen;

#ifdef SUPPORT_IPV6
	if(zfsess->control->peer.a.sa_family == AF_INET6)
	    psv_cmd = "EPSV\r\n";
	else
#endif /* SUPPORT_IPV6 */
	    psv_cmd = "PASV\r\n";
	if (zfsendcmd(psv_cmd) == 6)
	    return 1;
	else if (lastcode >= 500 && lastcode <= 504) {
	    /*
	     * Fall back to send port mode.  That will
	     * test the preferences for whether that's OK.
	     */
	    zfstatusp[zfsessno] |= ZFST_NOPS;
	    zfclosedata();
	    return zfopendata(name, zdsockp, is_passivep);
	}
	zdsockp->a.sa_family = zfsess->control->peer.a.sa_family;
#ifdef SUPPORT_IPV6
	if(zfsess->control->peer.a.sa_family == AF_INET6) {
	    /* see RFC 2428 for explanation */
	    char const *ptr, *end;
	    char delim, portbuf[6], *pbp;
	    unsigned long portnum;
	    ptr = strchr(lastmsg, '(');
	    if(!ptr) {
	    bad_epsv:
		zwarnnam(name, "bad response to EPSV: %s", lastmsg);
		zfclosedata();
		return 1;
	    }
	    delim = ptr[1];
	    if(delim < 33 || delim > 126 || ptr[2] != delim || ptr[3] != delim)
		goto bad_epsv;
	    ptr += 4;
	    end = strchr(ptr, delim);
	    if(!end || end[1] != ')')
		goto bad_epsv;
	    while(ptr != end && *ptr == '0')
		ptr++;
	    if(ptr == end || (end-ptr) > 5 || !idigit(*ptr))
		goto bad_epsv;
	    memcpy(portbuf, ptr, (end-ptr));
	    portbuf[end-ptr] = 0;
	    portnum = strtoul(portbuf, &pbp, 10);
	    if(*pbp || portnum > 65535UL)
		goto bad_epsv;
	    *zdsockp = zfsess->control->peer;
	    zdsockp->in6.sin6_port = htons((unsigned)portnum);
	    salen = sizeof(struct sockaddr_in6);
	} else
#endif /* SUPPORT_IPV6 */
	{
	    char *ptr;
	    int i, nums[6];
	    unsigned char iaddr[4], iport[2];

	    /*
	     * OK, now we need to know what port we're looking at,
	     * which is cunningly concealed in the reply.
	     * lastmsg already has the reply code expunged.
	     */
	    for (ptr = lastmsg; *ptr; ptr++)
		if (idigit(*ptr))
		    break;
	    if (sscanf(ptr, "%d,%d,%d,%d,%d,%d",
		       nums, nums+1, nums+2, nums+3, nums+4, nums+5) != 6) {
		zwarnnam(name, "bad response to PASV: %s", lastmsg);
		zfclosedata();
		return 1;
	    }
	    for (i = 0; i < 4; i++)
		iaddr[i] = STOUC(nums[i]);
	    iport[0] = STOUC(nums[4]);
	    iport[1] = STOUC(nums[5]);

	    memcpy(&zdsockp->in.sin_addr, iaddr, sizeof(iaddr));
	    memcpy(&zdsockp->in.sin_port, iport, sizeof(iport));
	    salen = sizeof(struct sockaddr_in);
	}

	/* we should timeout this connect */
	do {
	    err = connect(zfsess->dfd, (struct sockaddr *)zdsockp, salen);
	} while (err && errno == EINTR && !errflag);

	if (err) {
	    zwarnnam(name, "connect failed: %e", errno);
	    zfclosedata();
	    return 1;
	}

	*is_passivep = 1;
    } else {
#ifdef SUPPORT_IPV6
	char portcmd[8+INET6_ADDRSTRLEN+9];
#else
	char portcmd[40];
#endif
	ZSOCKLEN_T len;
	int ret;

	if (!(zfprefs & ZFPF_SNDP)) {
	    zwarnnam(name, "only sendport mode available for data");
	    return 1;
	}

	*zdsockp = zfsess->control->sock;
#ifdef SUPPORT_IPV6
	if(zdsockp->a.sa_family == AF_INET6) {
	    zdsockp->in6.sin6_port = 0;	/* to be set by bind() */
	    len = sizeof(struct sockaddr_in6);
	} else
#endif /* SUPPORT_IPV6 */
	{
	    zdsockp->in.sin_port = 0;	/* to be set by bind() */
	    len = sizeof(struct sockaddr_in);
	}
	/* need to do timeout stuff and probably handle EINTR here */
	if (bind(zfsess->dfd, (struct sockaddr *)zdsockp, len) < 0)
	    ret = 1;
	else if (getsockname(zfsess->dfd, (struct sockaddr *)zdsockp,
			     &len) < 0)
	    ret = 2;
	else if (listen(zfsess->dfd, 1) < 0)
	    ret = 3;
	else
	    ret = 0;

	if (ret) {
	    zwarnnam(name, "failure on data socket: %s: %e",
		     ret == 3 ? "listen" : ret == 2 ? "getsockname" : "bind",
		     errno);
	    zfclosedata();
	    return 1;
	}

#ifdef SUPPORT_IPV6
	if(zdsockp->a.sa_family == AF_INET6) {
	    /* see RFC 2428 for explanation */
	    strcpy(portcmd, "EPRT |2|");
	    zsh_inet_ntop(AF_INET6, &zdsockp->in6.sin6_addr,
			  portcmd+8, INET6_ADDRSTRLEN);
	    sprintf(strchr(portcmd, 0), "|%u|\r\n",
		    (unsigned)ntohs(zdsockp->in6.sin6_port));
	} else
#endif /* SUPPORT_IPV6 */
	{
	    unsigned char *addr = (unsigned char *) &zdsockp->in.sin_addr;
	    unsigned char *port = (unsigned char *) &zdsockp->in.sin_port;
	    sprintf(portcmd, "PORT %d,%d,%d,%d,%d,%d\r\n",
		    addr[0],addr[1],addr[2],addr[3],port[0],port[1]);
	}
	if (zfsendcmd(portcmd) >= 5) {
	    zwarnnam(name, "port command failed");
	    zfclosedata();
	    return 1;
	}
	*is_passivep = 0;
    }

    return 0;
}

/* Close the data connection. */

/**/
static void
zfclosedata(void)
{
    if (zfsess->dfd == -1)
	return;
    close(zfsess->dfd);
    zfsess->dfd = -1;
}

/*
 * Set up a data connection and use cmd to initiate a transfer.
 * The actual data fd will be zfsess->dfd; the calling routine
 * must handle the data itself.
 * rest is a REST command to specify starting somewhere other
 * then the start of the remote file.
 * getsize is non-zero if we want to try to find the number
 * of bytes in the reply to a RETR command.
 *
 * Return 0 on success, 1 on failure.
 */

/**/
static int
zfgetdata(char *name, char *rest, char *cmd, int getsize)
{
    ZSOCKLEN_T len;
    int newfd, is_passive;
    union tcp_sockaddr zdsock;

    if (zfopendata(name, &zdsock, &is_passive))
	return 1;

    /*
     * Set position in remote file for get/put.
     * According to RFC959, the restart command needs something
     * called a marker which has previously been put into the data.
     * Luckily for the real world, UNIX machines just interpret this
     * as an offset into the byte stream.
     *
     * This has to be sent immediately before the data transfer, i.e.
     * after all mucking around with types and sizes and so on.
     */
    if (rest && zfsendcmd(rest) > 3) {
	zfclosedata();
	return 1;
    }

    if (zfsendcmd(cmd) > 2) {
	zfclosedata();
	return 1;
    }
    if (getsize || (!(zfstatusp[zfsessno] & ZFST_TRSZ) &&
		    !strncmp(cmd, "RETR", 4))) {
	/*
	 * See if we got something like:
	 *   Opening data connection for nortypix.gif (1234567 bytes).
	 * On the first RETR, always see if this works,  Then we
	 * can avoid sending a special SIZE command beforehand.
	 */
	char *ptr = strstr(lastmsg, "bytes");
	zfstatusp[zfsessno] |= ZFST_NOSZ|ZFST_TRSZ;
	if (ptr) {
	    while (ptr > lastmsg && !idigit(*ptr))
		ptr--;
	    while (ptr > lastmsg && idigit(ptr[-1]))
		ptr--;
	    if (idigit(*ptr)) {
		zfstatusp[zfsessno] &= ~ZFST_NOSZ;
		if (getsize) {
		    off_t sz = zstrtol(ptr, NULL, 10);
		    zfsetparam("ZFTP_SIZE", &sz, ZFPM_READONLY|ZFPM_INTEGER);
		}
	    }
	}
    }

    if (!is_passive) {
	/*
	 * the current zfsess->dfd is the socket we opened, but we need
	 * to let the server set up a different fd for reading/writing.
	 * then we can close the fd we were listening for a connection on.
	 * don't expect me to understand this, i'm only the programmer.
	 */

	/* accept the connection */
	len = sizeof(zdsock);
	newfd = zfmovefd(accept(zfsess->dfd, (struct sockaddr *)&zdsock, 
				&len));
	if (newfd < 0)
	    zwarnnam(name, "unable to accept data: %e", errno);
	zfclosedata();
	if (newfd < 0)
	    return 1;
	zfsess->dfd = newfd;	/* this is now the actual data fd */
    } else {
	/*
	 * We avoided dup'ing zfsess->dfd up to this point, to try to keep
	 * things simple, so we now need to move it out of the way
	 * of the user-visible fd's.
	 */
	zfsess->dfd = zfmovefd(zfsess->dfd);
    }


    /* more options, just to look professional */
#ifdef SO_LINGER
    /*
     * Since data can take arbitrary amounts of time to arrive,
     * the socket can be made to hang around until it doesn't think
     * anything is arriving.
     *
     * In BSD 4.3, you could only linger for infinity.  Don't
     * know if this has changed.
     */
    {
	struct linger li;

	li.l_onoff = 1;
	li.l_linger = 120;
	setsockopt(zfsess->dfd, SOL_SOCKET, SO_LINGER,
		   (char *)&li, sizeof(li));
    }
#endif
#if defined(IP_TOS) && defined(IPTOS_THROUGHPUT)
    /* try to get high throughput, snigger */
    {
	int arg = IPTOS_THROUGHPUT;
	setsockopt(zfsess->dfd, IPPROTO_IP, IP_TOS, (char *)&arg, sizeof(arg));
    }
#endif
#if defined(F_SETFD) && defined(FD_CLOEXEC)
    /* If the shell execs a program, we don't want this fd left open. */
    fcntl(zfsess->dfd, F_SETFD, FD_CLOEXEC);
#endif

    return 0;
}

/*
 * Find out about a local or remote file and pass back the information.
 *
 * We could jigger this to use ls like ncftp does as a backup.
 * But if the server is non-standard enough not to have SIZE and MDTM,
 * there's a very good chance ls -l isn't going to do great things.
 *
 * if fd is >= 0, it is used for an fstat when remote is zero:
 * this is because on a put we are taking input from fd 0.
 */

/**/
static int
zfstats(char *fnam, int remote, off_t *retsize, char **retmdtm, int fd)
{
    off_t sz = -1;
    char *mt = NULL;
    int ret;

    if (retsize)
	*retsize = -1;
    if (retmdtm)
	*retmdtm = NULL;
    if (remote) {
	char *cmd;
	if ((zfsess->has_size == ZFCP_NOPE && retsize) ||
	    (zfsess->has_mdtm == ZFCP_NOPE && retmdtm))
	    return 2;

	/*
	 * File is coming from over there.
	 * Make sure we get the type right.
	 */
	zfsettype(ZFST_TYPE(zfstatusp[zfsessno]));
	if (retsize) {
	    cmd = tricat("SIZE ", fnam, "\r\n");
	    ret = zfsendcmd(cmd);
	    zsfree(cmd);
	    if (ret == 6)
		return 1;
	    else if (lastcode < 300) {
		sz = zstrtol(lastmsg, 0, 10);
		zfsess->has_size = ZFCP_YUPP;
	    } else if (lastcode >= 500 && lastcode <= 504) {
		zfsess->has_size = ZFCP_NOPE;
		return 2;
	    } else if (lastcode == 550)
		return 1;
	    /* if we got a 550 from SIZE, the file doesn't exist */
	}

	if (retmdtm) {
	    cmd = tricat("MDTM ", fnam, "\r\n");
	    ret = zfsendcmd(cmd);
	    zsfree(cmd);
	    if (ret == 6)
		return 1;
	    else if (lastcode < 300) {
		mt = ztrdup(lastmsg);
		zfsess->has_mdtm = ZFCP_YUPP;
	    } else if (lastcode >= 500 && lastcode <= 504) {
		zfsess->has_mdtm = ZFCP_NOPE;
		return 2;
	    } else if (lastcode == 550)
		return 1;
	}
    } else {
	/* File is over here */
	struct stat statbuf;
	struct tm *tm;
	char tmbuf[20];

	if ((fd == -1 ? stat(fnam, &statbuf) : fstat(fd, &statbuf)) < 0)
	    return 1;
	/* make sure it's off_t, since this has to be a pointer */
	sz = statbuf.st_size;

	if (retmdtm) {
	    /* use gmtime() rather than localtime() for consistency */
	    tm = gmtime(&statbuf.st_mtime);
	    /* FTP format for date is YYYYMMDDHHMMSS */
	    ztrftime(tmbuf, sizeof(tmbuf), "%Y%m%d%H%M%S", tm, 0L);
	    mt = ztrdup(tmbuf);
	}
    }
    if (retsize)
	*retsize = sz;
    if (retmdtm)
	*retmdtm = mt;
    return 0;
}

/* Set parameters to say what's coming */

/**/
static void
zfstarttrans(char *nam, int recv, off_t sz)
{
    off_t cnt = 0;
    /*
     * sz = -1 signifies error getting size.  don't set ZFTP_SIZE if sz is
     * zero, either: it probably came from an fstat() on a pipe, so it
     * means we don't know and shouldn't tell the user porkies.
     */
    if (sz > 0)
	zfsetparam("ZFTP_SIZE", &sz, ZFPM_READONLY|ZFPM_INTEGER);
    zfsetparam("ZFTP_FILE", ztrdup(nam), ZFPM_READONLY);
    zfsetparam("ZFTP_TRANSFER", ztrdup(recv ? "G" : "P"), ZFPM_READONLY);
    zfsetparam("ZFTP_COUNT", &cnt, ZFPM_READONLY|ZFPM_INTEGER);
}

/* Tidy up afterwards */

/**/
static void
zfendtrans(void)
{
    zfunsetparam("ZFTP_SIZE");
    zfunsetparam("ZFTP_FILE");
    zfunsetparam("ZFTP_TRANSFER");
    zfunsetparam("ZFTP_COUNT");
}

/* Read with timeout if recv is set. */

/**/
static int
zfread(int fd, char *bf, off_t sz, int tmout)
{
    int ret;

    if (!tmout)
	return read(fd, bf, sz);

    if (setjmp(zfalrmbuf)) {
	alarm(0);
	zwarnnam("zftp", "timeout on network read");
	return -1;
    }
    zfalarm(tmout);

    ret = read(fd, bf, sz);

    /* we don't bother turning off the whole alarm mechanism here */
    alarm(0);
    return ret;
}

/* Write with timeout if recv is not set. */

/**/
static int
zfwrite(int fd, char *bf, off_t sz, int tmout)
{
    int ret;

    if (!tmout)
	return write(fd, bf, sz);

    if (setjmp(zfalrmbuf)) {
	alarm(0);
	zwarnnam("zftp", "timeout on network write");
	return -1;
    }
    zfalarm(tmout);

    ret = write(fd, bf, sz);

    /* we don't bother turning off the whole alarm mechanism here */
    alarm(0);
    return ret;
}

static int zfread_eof;

/* Version of zfread when we need to read in block mode. */

/**/
static int
zfread_block(int fd, char *bf, off_t sz, int tmout)
{
    int n;
    struct zfheader hdr;
    off_t blksz, cnt;
    char *bfptr;
    do {
	/* we need the header */
	do {
	    n = zfread(fd, (char *)&hdr, sizeof(hdr), tmout);
	} while (n < 0 && errno == EINTR);
	if (n != 3 && !zfdrrrring) {
	    zwarnnam("zftp", "failure reading FTP block header");
	    return n;
	}
	/* size is stored in network byte order */
	if (hdr.flags & ZFHD_EOFB)
	    zfread_eof = 1;
	blksz = (hdr.bytes[0] << 8) | hdr.bytes[1];
	if (blksz > sz) {
	    /*
	     * See comments in file headers
	     */
	    zwarnnam("zftp", "block too large to handle");
	    errno = EIO;
	    return -1;
	}
	bfptr = bf;
	cnt = blksz;
	while (cnt) {
	    n = zfread(fd, bfptr, cnt, tmout);
	    if (n > 0) {
		bfptr += n;
		cnt -= n;
	    } else if (n < 0 && (errflag || zfdrrrring || errno != EINTR))
		return n;
	    else
		break;
	}
	if (cnt) {
	    zwarnnam("zftp", "short data block");
	    errno = EIO;
	    return -1;
	}
    } while ((hdr.flags & ZFHD_MARK) && !zfread_eof);
    return (hdr.flags & ZFHD_MARK) ? 0 : blksz;
}

/* Version of zfwrite when we need to write in block mode. */

/**/
static int
zfwrite_block(int fd, char *bf, off_t sz, int tmout)
{
    int n;
    struct zfheader hdr;
    off_t cnt;
    char *bfptr;
    /* we need the header */
    do {
	hdr.bytes[0] = (sz & 0xff00) >> 8;
	hdr.bytes[1] = sz & 0xff;
	hdr.flags = sz ? 0 : ZFHD_EOFB;
	n = zfwrite(fd, (char *)&hdr, sizeof(hdr), tmout);
    } while (n < 0 && errno == EINTR);
    if (n != 3 && !zfdrrrring) {
	zwarnnam("zftp", "failure writing FTP block header");
	return n;
    }
    bfptr = bf;
    cnt = sz;
    while (cnt) {
	n = zfwrite(fd, bfptr, cnt, tmout);
	if (n > 0) {
	    bfptr += n;
	    cnt -= n;
	} else if (n < 0 && (errflag || zfdrrrring || errno != EINTR))
	    return n;
    }

    return sz;
}

/*
 * Move stuff from fdin to fdout, tidying up the data connection
 * when finished.  The data connection could be either input or output:
 * recv is 1 for receiving a file, 0 for sending.
 *
 * progress is 1 to use a progress meter.
 * startat says how far in we're starting with a REST command.
 *
 * Since we're doing some buffering here anyway, we don't bother
 * with a stdio layer.
 */

/**/
static int
zfsenddata(char *name, int recv, int progress, off_t startat)
{
#define ZF_BUFSIZE 32768
#define ZF_ASCSIZE (ZF_BUFSIZE/2)
    /* ret = 2 signals the local read/write failed, so send abort */
    int n, ret = 0, gotack = 0, fdin, fdout, fromasc = 0, toasc = 0;
    int rtmout = 0, wtmout = 0;
    char lsbuf[ZF_BUFSIZE], *ascbuf = NULL, *optr;
    off_t sofar = 0, last_sofar = 0;
    readwrite_t read_ptr = zfread, write_ptr = zfwrite;
    Shfunc shfunc;

    if (progress && (shfunc = getshfunc("zftp_progress"))) {
	/*
	 * progress to set up:  ZFTP_COUNT is zero.
	 * We do this here in case we needed to wait for a RETR
	 * command to tell us how many bytes are coming.
	 */
	int osc = sfcontext;

	sfcontext = SFC_HOOK;
	doshfunc(shfunc, NULL, 1);
	sfcontext = osc;
	/* Now add in the bit of the file we've got/sent already */
	sofar = last_sofar = startat;
    }
    if (recv) {
	fdin = zfsess->dfd;
	fdout = 1;
	rtmout = getiparam("ZFTP_TMOUT");
	if (ZFST_CTYP(zfstatusp[zfsessno]) == ZFST_ASCI)
	    fromasc = 1;
	if (ZFST_MODE(zfstatusp[zfsessno]) == ZFST_BLOC)
	    read_ptr = zfread_block;
    } else {
	fdin = 0;
	fdout = zfsess->dfd;
	wtmout = getiparam("ZFTP_TMOUT");
	if (ZFST_CTYP(zfstatusp[zfsessno]) == ZFST_ASCI)
	    toasc = 1;
	if (ZFST_MODE(zfstatusp[zfsessno]) == ZFST_BLOC)
	    write_ptr = zfwrite_block;
    }

    if (toasc)
	ascbuf = zalloc(ZF_ASCSIZE);
    zfpipe();
    zfread_eof = 0;
    while (!ret && !zfread_eof) {
	n = (toasc) ? read_ptr(fdin, ascbuf, ZF_ASCSIZE, rtmout)
	    : read_ptr(fdin, lsbuf, ZF_BUFSIZE, rtmout);
	if (n > 0) {
	    char *iptr;
	    if (toasc) {
		/* \n -> \r\n it shouldn't happen to a dog. */
		char *iptr = ascbuf, *optr = lsbuf;
		int cnt = n;
		while (cnt--) {
		    if (*iptr == '\n') {
			*optr++ = '\r';
			n++;
		    }
		    *optr++ = *iptr++;
		}
	    }
	    if (fromasc && (iptr = memchr(lsbuf, '\r', n))) {
		/* \r\n -> \n */
		char *optr = iptr;
		int cnt = n - (iptr - lsbuf);
		while (cnt--) {
		    if (*iptr != '\r' || iptr[1] != '\n') {
			*optr++ = *iptr;
		    } else
			n--;
		    iptr++;
		}
	    }
	    optr = lsbuf;

	    sofar += n;

	    for (;;) {
		/*
		 * in principle, write can be interrupted after
		 * safely writing some bytes, and will return the
		 * number already written, which may not be the
		 * complete buffer.  so make this robust.  they call me
		 * `robustness stephenson'.  in my dreams.
		 */
		int newn = write_ptr(fdout, optr, n, wtmout);
		if (newn == n)
		    break;
		if (newn < 0) {
		    /*
		     * The somewhat contorted test here (for write)
		     * and below (for read) means:
		     * real error if
		     *  - errno is set and it's not just an interrupt, or
		     *  - errflag is set, probably due to CTRL-c, or
		     *  - zfdrrrring is set, due to the alarm going off.
		     * print an error message if
		     *  - not a timeout, since that was reported, and
		     *    either
		     *    - a non-interactive shell, where we don't
		     *      know what happened otherwise
		     *    - or both of
		     *      - not errflag, i.e. CTRL-c or what have you,
		     *        since the user probably knows about that, and
		     *      - not a SIGPIPE, since usually people are
		     *        silent about those when going to pagers
		     *        (if you quit less or more in the middle
		     *        and see an error message you think `I
		     *        shouldn't have done that').
		     *
		     * If we didn't print an error message here,
		     * and were going to do an abort (ret == 2)
		     * because the error happened on the local end
		     * of the connection, set ret to 3 and don't print
		     * the 'aborting...' either.
		     *
		     * There must be a better way of doing this.
		     */
		    if (errno != EINTR || errflag || zfdrrrring) {
			if (!zfdrrrring &&
			    (!interact || (!errflag && errno != EPIPE))) {
			    ret = recv ? 2 : 1;
			    zwarnnam(name, "write failed: %e", errno);
			} else
			    ret = recv ? 3 : 1;
			break;
		    }
		    continue;
		}
		optr += newn;
		n -= newn;
	    }
	} else if (n < 0) {
	    if (errno != EINTR || errflag || zfdrrrring) {
		if (!zfdrrrring &&
		    (!interact || (!errflag && errno != EPIPE))) {
		    ret = recv ? 1 : 2;
		    zwarnnam(name, "read failed: %e", errno);
		} else
		    ret = recv ? 1 : 3;
		break;
	    }
	} else
	    break;
	if (!ret && sofar != last_sofar && progress &&
	    (shfunc = getshfunc("zftp_progress"))) {
	    int osc = sfcontext;

	    zfsetparam("ZFTP_COUNT", &sofar, ZFPM_READONLY|ZFPM_INTEGER);
	    sfcontext = SFC_HOOK;
	    doshfunc(shfunc, NULL, 1);
	    sfcontext = osc;
	    last_sofar = sofar;
	}
    }
    zfunpipe();
    /*
     * At this point any timeout was on the data connection,
     * so we don't need to force the control connection to close.
     */
    zfdrrrring = 0;
    if (!errflag && !ret && !recv &&
	ZFST_MODE(zfstatusp[zfsessno]) == ZFST_BLOC) {
	/* send an end-of-file marker block */
	ret = (zfwrite_block(fdout, lsbuf, 0, wtmout) < 0);
    }
    if (errflag || ret > 1) {
	/*
	 * some error occurred, maybe a keyboard interrupt, or
	 * a local file/pipe handling problem.
	 * send an abort.
	 *
	 * safest to block all signals here?  can get frustrating if
	 * we're waiting for an abort.  don't I know.  let's start
	 * off just by blocking SIGINT's.
	 *
	 * maybe the timeout for the abort should be shorter than
	 * for normal commands.  and what about aborting after
	 * we had a timeout on the data connection, is that
	 * really a good idea?
	 */
	/* RFC 959 says this is what to send */
	unsigned char msg[4] = { IAC, IP, IAC, SYNCH };

	if (ret == 2)
	    zwarnnam(name, "aborting data transfer...");

	holdintr();

	/* the following is black magic, as far as I'm concerned. */
	/* what are we going to do if it fails?  not a lot, actually. */
	send(zfsess->control->fd, (char *)msg, 3, 0);
	send(zfsess->control->fd, (char *)msg+3, 1, MSG_OOB);

	zfsendcmd("ABOR\r\n");
	if (lastcode == 226) {
	    /*
	     * 226 is supposed to mean the transfer got sent OK after
	     * all, and the abort got ignored, at least that's what
	     * rfc959 seems to be saying.  but in fact what can happen
	     * is the transfer finishes (at least as far as the
	     * server's concerned) and it's response is waiting, then
	     * the abort gets sent, and we need to mop up a response to
	     * that.  so actually in most cases we get two replies
	     * anyway.  we could test if we had select() on all hosts.
	     */
	    /* gotack = 1; */
	    /*
	     * we'd better leave errflag, since we don't know
	     * where it came from.  maybe the user wants to abort
	     * a whole script or function.
	     */
	} else
	    ret = 1;

	noholdintr();
    }
	
    if (toasc)
	zfree(ascbuf, ZF_ASCSIZE);
    zfclosedata();
    if (!gotack && zfgetmsg() > 2)
	ret = 1;
    return ret != 0;
}

/* Open a new control connection, i.e. start a new FTP session */

/**/
static int
zftp_open(char *name, char **args, int flags)
{
    struct protoent *zprotop;
    struct servent *zservp;
    struct hostent *zhostp;
    char **addrp, *fname, *tmpptr, *portnam = "ftp";
    char *hostnam, *hostsuffix;
    int err, tmout, port = -1;
    ZSOCKLEN_T  len;
    int herrno, af, hlen;

    if (!*args) {
	if (zfsess->userparams)
	    args = zfsess->userparams;
	else {
	    zwarnnam(name, "no host specified");
	    return 1;
	}
    }

    /*
     * Close the existing connection if any.
     * Probably this is the safest thing to do.  It's possible
     * a `QUIT' will hang, though.
     */
    if (zfsess->control)
	zfclose(0);

    hostnam = dupstring(args[0]);
    /*
     * Check for IPv6 address in square brackets (RFC2732).
     * We are more lenient and allow any form for the host here.
     */
    if (hostnam[0] == '[') {
	hostnam++;
	hostsuffix = strchr(hostnam, ']');
	if (!hostsuffix || (hostsuffix[1] && hostsuffix[1] != ':')) {
	    zwarnnam(name, "Invalid host format: %s", hostnam);
	    return 1;
	}
	*hostsuffix++ = '\0';
    }
    else
	hostsuffix = hostnam;

    if ((tmpptr = strchr(hostsuffix, ':'))) {
	char *endptr;

	*tmpptr++ = '\0';
	port = (int)zstrtol(tmpptr, &endptr, 10);
	/*
	 * If the port is not numeric, look it up by name below.
	 */
	if (*endptr) {
	    portnam = tmpptr;
	    port = -1;
	}
#if defined(HAVE_NTOHS) && defined(HAVE_HTONS)
	else {
	    port = (int)htons((unsigned short)port);
	}
#endif
    }

    /* this is going to give 0.  why bother? */
    zprotop = getprotobyname("tcp");
    if (!zprotop) {
	zwarnnam(name, "Can't find protocol TCP (is your network functional)?");
	return 1;
    }
    if (port < 0)
	zservp = getservbyname(portnam, "tcp");
    else
	zservp = getservbyport(port, "tcp");

    if (!zprotop || !zservp) {
	zwarnnam(name, "Can't find port for service `%s'", portnam);
	return 1;
    }

    /* don't try talking to server yet */
    zcfinish = 2;

    /*
     * This sets an alarm for the whole process, getting the host name
     * as well as connecting.  Arguably you could time them out separately. 
     */
    tmout = getiparam("ZFTP_TMOUT");
    if (setjmp(zfalrmbuf)) {
	char *hname;
	alarm(0);
	queue_signals();
	if ((hname = getsparam_u("ZFTP_HOST")) && *hname) 
	    zwarnnam(name, "timeout connecting to %s", hname);
	else
	    zwarnnam(name, "timeout on host name lookup");
	unqueue_signals();
	zfclose(0);
	return 1;
    }
    zfalarm(tmout);

#ifdef SUPPORT_IPV6
    for(af=AF_INET6; 1; af = AF_INET)
# define SUCCEEDED() break
# define FAILED() if(af == AF_INET) { } else continue
#else
	af = AF_INET;
# define SUCCEEDED() do { } while(0)
# define FAILED() do { } while(0)
#endif
    {
	off_t tcp_port;

	zhostp = zsh_getipnodebyname(hostnam, af, 0, &herrno);
	if (!zhostp || errflag) {
	    /* should use herror() here if available, but maybe
	     * needs configure test. on AIX it's present but not
	     * in headers.
	     * 
	     * on the other hand, herror() is obsolete
	     */
	    FAILED();
	    zwarnnam(name, "host not found: %s", hostnam);
	    alarm(0);
	    return 1;
	}
	zfsetparam("ZFTP_HOST", ztrdup(zhostp->h_name), ZFPM_READONLY);
	/* careful with pointer types */
#if defined(HAVE_NTOHS) && defined(HAVE_HTONS)
	tcp_port = (off_t)ntohs((unsigned short)zservp->s_port);
#else
	tcp_port = (off_t)zservp->s_port;
#endif
	zfsetparam("ZFTP_PORT", &tcp_port, ZFPM_READONLY|ZFPM_INTEGER);

#ifdef SUPPORT_IPV6
	if(af == AF_INET6) {
	    hlen = 16;
	} else
#endif /* SUPPORT_IPV6 */
	{
	    hlen = 4;
	}

	zfsess->control = tcp_socket(af, SOCK_STREAM, 0, ZTCP_ZFTP);

	if (!(zfsess->control) || (zfsess->control->fd < 0)) {
	    if (zfsess->control) {
		tcp_close(zfsess->control);
		zfsess->control = NULL;
	    }
	    freehostent(zhostp);
	    zfunsetparam("ZFTP_HOST");
	    zfunsetparam("ZFTP_PORT");
	    FAILED();
	    zwarnnam(name, "socket failed: %e", errno);
	    alarm(0);
	    return 1;
	}
	/* counts as `open' so long as it's not negative */
	zfnopen++;

	/*
	 * now connect the socket.  manual pages all say things like `this is
	 * all explained oh-so-wonderfully in some other manual page'.  not.
	 */

	err = 1;

	/* try all possible IP's */
	for (addrp = zhostp->h_addr_list; err && *addrp; addrp++) {
	    if(hlen != zhostp->h_length)
		zwarnnam(name, "address length mismatch");
	    do {
		err = tcp_connect(zfsess->control, *addrp, zhostp, zservp->s_port);
	    } while (err && errno == EINTR && !errflag);
	    /* you can check whether it's worth retrying here */
	}

	if (err) {
	    freehostent(zhostp);
	    zfclose(0);
	    FAILED();
	    zwarnnam(name, "connect failed: %e", errno);
	    alarm(0);
	    return 1;
	}

	SUCCEEDED();
    }
    alarm(0);
    {
#ifdef SUPPORT_IPV6
	char pbuf[INET6_ADDRSTRLEN];
#else
	char pbuf[INET_ADDRSTRLEN];
#endif
	addrp--;
	zsh_inet_ntop(af, *addrp, pbuf, sizeof(pbuf));
	zfsetparam("ZFTP_IP", ztrdup(pbuf), ZFPM_READONLY);
    }
    freehostent(zhostp);
    /* now we can talk to the control connection */
    zcfinish = 0;

    /*
     * Move the fd out of the user-visible range.  We need to do
     * this after the connect() on some systems.
     */
    zfsess->control->fd = zfmovefd(zfsess->control->fd);

#if defined(F_SETFD) && defined(FD_CLOEXEC)
    /* If the shell execs a program, we don't want this fd left open. */
    fcntl(zfsess->control->fd, F_SETFD, FD_CLOEXEC);
#endif

    len = sizeof(zfsess->control->sock);
    if (getsockname(zfsess->control->fd, (struct sockaddr *)&zfsess->control->sock, &len) < 0) {
	zwarnnam(name, "getsockname failed: %e", errno);
	zfclose(0);
	return 1;
    }
    /* nice to get some options right, ignore if they don't work */
#ifdef SO_OOBINLINE
    /*
     * this says we want messages in line.  maybe sophisticated people
     * do clever things with SIGURG.
     */
    len = 1;
    setsockopt(zfsess->control->fd, SOL_SOCKET, SO_OOBINLINE,
	       (char *)&len, sizeof(len));
#endif
#if defined(IP_TOS) && defined(IPTOS_LOWDELAY)
    /* for control connection we want low delay.  please don't laugh. */
    len = IPTOS_LOWDELAY;
    setsockopt(zfsess->control->fd, IPPROTO_IP, IP_TOS, (char *)&len, sizeof(len));
#endif

    /*
     * We use stdio with line buffering for convenience on input.
     * On output, we can just dump a complete message to the fd via write().
     */
    zfsess->cin = fdopen(zfsess->control->fd, "r");

    if (!zfsess->cin) {
	zwarnnam(name, "file handling error");
	zfclose(0);
	return 1;
    }

#ifdef _IONBF
    setvbuf(zfsess->cin, NULL, _IONBF, 0);
#else
    setlinebuf(zfsess->cin);
#endif

    /*
     * now see what the remote server has to say about that.
     */
    if (zfgetmsg() >= 4) {
	zfclose(0);
	return 1;
    }

    zfsess->has_size = zfsess->has_mdtm = ZFCP_UNKN;
    zfsess->dfd = -1;
    /* initial status: open, ASCII data, stream mode 'n' stuff */
    zfstatusp[zfsessno] = 0;

    /*
     * Open file for saving the current status.
     * We keep this open at the end of the session because
     * it is used to store the status for all sessions.
     * However, it is closed whenever there are no connections open.
     */
    if (zfstatfd == -1) {
	zfstatfd = gettempfile(NULL, 1, &fname);
	DPUTS(zfstatfd == -1, "zfstatfd not created");
#if defined(F_SETFD) && defined(FD_CLOEXEC)
	/* If the shell execs a program, we don't want this fd left open. */
	fcntl(zfstatfd, F_SETFD, FD_CLOEXEC);
#endif
	unlink(fname);
    }

    if (zfsess->control->fd == -1) {
	/* final paranoid check */
	tcp_close(zfsess->control);
	zfsess->control = NULL;
	zfnopen--;
    } else {
	zfsetparam("ZFTP_MODE", ztrdup("S"), ZFPM_READONLY);
	/* if remaining arguments, use them to log in. */
	if (*++args)
	    return zftp_login(name, args, flags);
    }
    /* if something wayward happened, connection was already closed */
    return !zfsess->control;
}

/*
 * Read a parameter string, with a prompt if reading from stdin.
 * The returned string is on the heap.
 * If noecho, turn off ECHO mode while reading.
 */

/**/
static char *
zfgetinfo(char *prompt, int noecho)
{
    int resettty = 0;
    /* 256 characters should be enough, hardly worth allocating
     * a password string byte by byte
     */
    char instr[256], *strret;
    int len;

    /*
     * Only print the prompt if getting info from a tty.  Of
     * course, we don't know if stderr has been redirected, but
     * that seems a minor point.
     */
    if (isatty(0)) {
	if (noecho) {
	    /* hmmm... all this great big shell and we have to read
	     * something with no echo by ourselves.
	     * bin_read() is far to complicated for our needs.
	     * we could use zread(), but that relies on static
	     * variables, so someone doesn't want that to happen.
	     *
	     * this is modified from setcbreak() in utils.c,
	     * except I don't see any point in using cbreak mode
	     */
	    struct ttyinfo ti;

	    ti = shttyinfo;
#ifdef HAS_TIO
	    ti.tio.c_lflag &= ~ECHO;
#else
	    ti.sgttyb.sg_flags &= ~ECHO;
#endif
	    settyinfo(&ti);
	    resettty = 1;
	}
	fflush(stdin);
	fputs(prompt, stderr);
	fflush(stderr);
    }

    if (fgets(instr, 256, stdin) == NULL)
	instr[len = 0] = '\0';
    else if (instr[len = strlen(instr)-1] == '\n')
	instr[len] = '\0';

    strret = dupstring(instr);

    if (resettty) {
	/* '\n' didn't get echoed */
	fputc('\n', stdout);
	fflush(stdout);
	settyinfo(&shttyinfo);
    }

    return strret;
}

/*
 * set params for an open with no arguments.
 * this allows easy re-opens.
 */

/**/
static int
zftp_params(UNUSED(char *name), char **args, UNUSED(int flags))
{
    char *prompts[] = { "Host: ", "User: ", "Password: ", "Account: " };
    char **aptr, **newarr;
    int i, j, len;

    if (!*args) {
	if (zfsess->userparams) {
	    for (aptr = zfsess->userparams, i = 0; *aptr; aptr++, i++) {
		if (i == 2) {
		    len = strlen(*aptr);
		    for (j = 0; j < len; j++)
			fputc('*', stdout);
		    fputc('\n', stdout);
		} else
		    fprintf(stdout, "%s\n", *aptr);
	    }
	    return 0;
	} else
	    return 1;
    }
    if (!strcmp(*args, "-")) {
	if (zfsess->userparams)
	    freearray(zfsess->userparams);
	zfsess->userparams = 0;
	return 0;
    }
    len = arrlen(args);
    newarr = (char **)zshcalloc((len+1)*sizeof(char *));
    for (aptr = args, i = 0; *aptr && !errflag; aptr++, i++) {
	char *str;
	if (**aptr == '?')
	    str = zfgetinfo((*aptr)[1] ? (*aptr+1) : prompts[i], i == 2);
	else
	    str = (**aptr == '\\') ? *aptr+1 : *aptr;
	newarr[i] = ztrdup(str);
    }
    if (errflag) {
	/* maybe user CTRL-c'd in the middle somewhere */
	for (aptr = newarr; *aptr; aptr++)
	    zsfree(*aptr);
	zfree(newarr, len+1);
	return 1;
    }
    if (zfsess->userparams)
	freearray(zfsess->userparams);
    zfsess->userparams = newarr;
    return 0;
}

/* login a user:  often called as part of the open sequence */

/**/
static int
zftp_login(char *name, char **args, UNUSED(int flags))
{
    char *ucmd, *passwd = NULL, *acct = NULL;
    char *user, tbuf[2] = "X";
    int stopit;

    if ((zfstatusp[zfsessno] & ZFST_LOGI) && zfsendcmd("REIN\r\n") >= 4)
	return 1;

    zfstatusp[zfsessno] &= ~ZFST_LOGI;
    if (*args) {
	user = *args++;
    } else {
	user = zfgetinfo("User: ", 0);
    }

    ucmd = tricat("USER ", user, "\r\n");
    stopit = 0;

    if (zfsendcmd(ucmd) == 6)
	stopit = 2;

    while (!stopit && !errflag) {
	switch (lastcode) {
	case 230: /* user logged in */
	case 202: /* command not implemented, don't care */
	    stopit = 1;
	    break;

	case 331: /* need password */
	    if (*args)
		passwd = *args++;
	    else
		passwd = zfgetinfo("Password: ", 1);
	    zsfree(ucmd);
	    ucmd = tricat("PASS ", passwd, "\r\n");
	    if (zfsendcmd(ucmd) == 6)
		stopit = 2;
	    break;

	case 332: /* need account */
	case 532:
	    if (*args)
		acct = *args++;
	    else
		acct = zfgetinfo("Account: ", 0);
	    zsfree(ucmd);
	    ucmd = tricat("ACCT ", acct, "\r\n");
	    if (zfsendcmd(ucmd) == 6)
		stopit = 2;
	    break;

	case 421: /* service not available, so closed anyway */
	case 501: /* syntax error */
	case 503: /* bad commands */
	case 530: /* not logged in */
	case 550: /* random can't-do-that */
	default:  /* whatever, should flag this as bad karma */
	    /* need more diagnostics here */
	    stopit = 2;
	    break;
	}
    }

    zsfree(ucmd);
    if (!zfsess->control)
	return 1;
    if (stopit == 2 || (lastcode != 230 && lastcode != 202)) {
	zwarnnam(name, "login failed");
	return 1;
    }

    if (*args) {
	int cnt;
	for (cnt = 0; *args; args++)
	    cnt++;
	zwarnnam(name, "warning: %d command arguments not used\n", cnt);
    }
    zfstatusp[zfsessno] |= ZFST_LOGI;
    zfsetparam("ZFTP_USER", ztrdup(user), ZFPM_READONLY);
    if (acct)
	zfsetparam("ZFTP_ACCOUNT", ztrdup(acct), ZFPM_READONLY);

    /*
     * Now find out what system we're connected to. Some systems
     * won't let us do this until we're logged in; it's fairly safe
     * to delay it here for all systems.
     */
    if (!(zfprefs & ZFPF_DUMB) && !(zfstatusp[zfsessno] & ZFST_SYST)) {
	if (zfsendcmd("SYST\r\n") == 2) {
	    char *ptr = lastmsg, *eptr, *systype;
	    for (eptr = ptr; *eptr; eptr++)
		;
	    systype = ztrduppfx(ptr, eptr-ptr);
	    if (!strncmp(systype, "UNIX Type: L8", 13)) {
		/*
		 * Use binary for transfers.  This simple test saves much
		 * hassle for all concerned, particularly me.
		 *
		 * We could set this based just on the UNIX part,
		 * but I don't really know the consequences of that.
		 */
		zfstatusp[zfsessno] |= ZFST_IMAG;
	    }
	    zfsetparam("ZFTP_SYSTEM", systype, ZFPM_READONLY);
	}
	zfstatusp[zfsessno] |= ZFST_SYST;
    }
    tbuf[0] = (ZFST_TYPE(zfstatusp[zfsessno]) == ZFST_ASCI) ? 'A' : 'I';
    zfsetparam("ZFTP_TYPE", ztrdup(tbuf), ZFPM_READONLY);

    /*
     * Get the directory.  This is possibly an unnecessary overhead, of
     * course, but when you're being driven by shell functions there's
     * just no way of telling.
     */
    return zfgetcwd();
}

/*
 * See if the server wants to tell us something.  On a timeout, we usually
 * have a `421 Timeout' or something such waiting for us, so we read
 * it here.  As well as being called explicitly by the user
 * (precmd is a very good place for this, it's cheap since it has
 * no network overhead), we call it in the bin_zftp front end if we
 * have a connection and weren't going to call it anyway.
 *
 * Poll-free and select-free systems are few and far between these days,
 * but I'm willing to consider suggestions.
 */

/**/
static int
zftp_test(UNUSED(char *name), UNUSED(char **args), UNUSED(int flags))
{
#if defined(HAVE_POLL) || defined(HAVE_SELECT)
    int ret;
# ifdef HAVE_POLL
    struct pollfd pfd;
# else
    fd_set f;
    struct timeval tv;
# endif /* HAVE_POLL */

    if (!zfsess->control)
	return 1;

# ifdef HAVE_POLL
#  ifndef POLLIN
    /* safety first, though I think POLLIN is more common */
#   define POLLIN POLLNORM
#  endif /* HAVE_POLL */
    pfd.fd = zfsess->control->fd;
    pfd.events = POLLIN;
    if ((ret = poll(&pfd, 1, 0)) < 0 && errno != EINTR && errno != EAGAIN)
	zfclose(0);
    else if (ret > 0 && pfd.revents) {
	/* handles 421 (maybe a bit noisily?) */
	zfgetmsg();
    }
# else
    FD_ZERO(&f);
    FD_SET(zfsess->control->fd, &f);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if ((ret = select(zfsess->control->fd +1, (SELECT_ARG_2_T) &f,
		      NULL, NULL, &tv)) < 0
	&& errno != EINTR)
	zfclose(0);
    else if (ret > 0) {
	/* handles 421 */
	zfgetmsg();
    }
# endif /* HAVE_POLL */
    /* if we have no zfsess->control, then we've just been dumped out. */
    return zfsess->control ? 0 : 2;
#else
    zfwarnnam(name, "not supported on this system.");
    return 3;
#endif /* defined(HAVE_POLL) || defined(HAVE_SELECT) */
}


/* do ls or dir on the remote directory */

/**/
static int
zftp_dir(char *name, char **args, int flags)
{
    /* maybe should be cleverer about handling arguments */
    char *cmd;
    int ret;

    /*
     * RFC959 says this must be ASCII or EBCDIC, not image format.
     * I rather suspect on a UNIX server we get away handsomely
     * with doing everything, including this, as image.
     */
    zfsettype(ZFST_ASCI);

    cmd = zfargstring((flags & ZFTP_NLST) ? "NLST" : "LIST", args);
    ret = zfgetdata(name, NULL, cmd, 0);
    zsfree(cmd);
    if (ret)
	return 1;

    fflush(stdout);		/* since we're now using fd 1 */
    return zfsenddata(name, 1, 0, 0);
}

/* change the remote directory */

/**/
static int
zftp_cd(UNUSED(char *name), char **args, int flags)
{
    /* change directory --- enhance to allow 'zftp cdup' */
    int ret;

    if ((flags & ZFTP_CDUP) || !strcmp(*args, "..") ||
	!strcmp(*args, "../")) {
	ret = zfsendcmd("CDUP\r\n");
    } else {
	char *cmd = tricat("CWD ", *args, "\r\n");
	ret = zfsendcmd(cmd);
	zsfree(cmd);
    }
    if (ret > 2)
	return 1;
    /* sometimes the directory may be in the response. usually not. */
    if (zfgetcwd())
	return 1;

    return 0;
}

/* get the remote directory */

/**/
static int
zfgetcwd(void)
{
    char *ptr, *eptr;
    int endc;
    Shfunc shfunc;

    if (zfprefs & ZFPF_DUMB)
	return 1;
    if (zfsendcmd("PWD\r\n") > 2) {
	zfunsetparam("ZFTP_PWD");
	return 1;
    }
    ptr = lastmsg;
    while (*ptr == ' ')
	ptr++;
    if (!*ptr)			/* ultra safe */
	return 1;
    if (*ptr == '"') {
	ptr++;
	endc = '"';
    } else 
	endc = ' ';
    for (eptr = ptr; *eptr && *eptr != endc; eptr++)
	;
    zfsetparam("ZFTP_PWD", ztrduppfx(ptr, eptr-ptr), ZFPM_READONLY);

    /*
     * This isn't so necessary if we're going to have a shell function
     * front end.  By putting it here, and in close when ZFTP_PWD is unset,
     * we at least cover the bases.
     */
    if ((shfunc = getshfunc("zftp_chpwd"))) {
	int osc = sfcontext;

	sfcontext = SFC_HOOK;
	doshfunc(shfunc, NULL, 1);
	sfcontext = osc;
    }
    return 0;
}

/*
 * Set the type for the next transfer, usually image (binary) or ASCII.
 */

/**/
static int
zfsettype(int type)
{
    char buf[] = "TYPE X\r\n";
    if (ZFST_TYPE(type) == ZFST_CTYP(zfstatusp[zfsessno]))
	return 0;
    buf[5] = (ZFST_TYPE(type) == ZFST_ASCI) ? 'A' : 'I';
    if (zfsendcmd(buf) > 2)
	return 1;
    zfstatusp[zfsessno] &= ~(ZFST_TMSK << ZFST_TBIT);
    /* shift the type left to set the current type bits */;
    zfstatusp[zfsessno] |= type << ZFST_TBIT;
    return 0;
}

/*
 * Print or get a new type for the transfer.
 * We don't actually set the type at this point.
 */

/**/
static int
zftp_type(char *name, char **args, int flags)
{
    char *str, nt, tbuf[2] = "A";
    if (flags & (ZFTP_TBIN|ZFTP_TASC)) {
	nt = (flags & ZFTP_TBIN) ? 'I' : 'A';
    } else if (!(str = *args)) {
	/*
	 * Since this is supposed to be a low-level basis for
	 * an FTP system, just print the single code letter.
	 */
	printf("%c\n", (ZFST_TYPE(zfstatusp[zfsessno]) == ZFST_ASCI) ?
	       'A' : 'I');
	fflush(stdout);
	return 0;
    } else {
	nt = toupper(STOUC(*str));
	/*
	 * RFC959 specifies other types, but these are the only
	 * ones we know what to do with.
	 */
	if (str[1] || (nt != 'A' && nt != 'B' && nt != 'I')) {
	    zwarnnam(name, "transfer type %s not recognised", str);
	    return 1;
	}
	
	if (nt == 'B')		/* binary = image */
	    nt = 'I';
    }

    zfstatusp[zfsessno] &= ~ZFST_TMSK;
    zfstatusp[zfsessno] |= (nt == 'I') ? ZFST_IMAG : ZFST_ASCI;
    tbuf[0] = nt;
    zfsetparam("ZFTP_TYPE", ztrdup(tbuf), ZFPM_READONLY);
    return 0;
}

/**/
static int
zftp_mode(char *name, char **args, UNUSED(int flags))
{
    char *str, cmd[] = "MODE X\r\n";
    int nt;

    if (!(str = *args)) {
	printf("%c\n", (ZFST_MODE(zfstatusp[zfsessno]) == ZFST_STRE) ?
	       'S' : 'B');
	fflush(stdout);
	return 0;
    }
    nt = str[0] = toupper(STOUC(*str));
    if (str[1] || (nt != 'S' && nt != 'B')) {
	zwarnnam(name, "transfer mode %s not recognised", str);
	return 1;
    }
    cmd[5] = (char) nt;
    if (zfsendcmd(cmd) > 2)
	return 1;
    zfstatusp[zfsessno] &= ZFST_MMSK;
    zfstatusp[zfsessno] |= (nt == 'S') ? ZFST_STRE : ZFST_BLOC;
    zfsetparam("ZFTP_MODE", ztrdup(str), ZFPM_READONLY);
    return 0;
}

/**/
static int
zftp_local(UNUSED(char *name), char **args, int flags)
{
    int more = !!args[1], ret = 0, dofd = !*args;
    while (*args || dofd) {
	off_t sz;
	char *mt;
	int newret = zfstats(*args, !(flags & ZFTP_HERE), &sz, &mt,
			     dofd ? 0 : -1);
	if (newret == 2)	/* at least one is not implemented */
	    return 2;
	else if (newret) {
	    ret = 1;
	    if (mt)
		zsfree(mt);
	    args++;
	    continue;
	}
	if (more) {
	    fputs(*args, stdout);
	    fputc(' ', stdout);
	}
#ifdef OFF_T_IS_64_BIT
	printf("%s %s\n", output64(sz), mt);
#else
	DPUTS(sizeof(sz) > sizeof(long),
		"Shell compiled with wrong off_t size");
	printf("%ld %s\n", (long)sz, mt);
#endif
	zsfree(mt);
	if (dofd)
	    break;
	args++;
    }
    fflush(stdout);

    return ret;
}

/*
 * Generic transfer for get, put and append.
 *
 * Get sends all files to stdout, i.e. this is basically cat. It's up to a
 * shell function driver to turn this into standard FTP-like commands.
 *
 * Put/append sends everything from stdin down the drai^H^H^Hata connection. 
 * Slightly weird with multiple files in that it tries to read
 * a separate complete file from stdin each time, which is
 * only even potentially useful interactively.  But the only
 * real alternative is just to allow one file at a time.
 */

/**/
static int
zftp_getput(char *name, char **args, int flags)
{
    int ret = 0, recv = (flags & ZFTP_RECV), getsize = 0, progress = 1;
    char *cmd = recv ? "RETR " : (flags & ZFTP_APPE) ? "APPE " : "STOR ";
    Shfunc shfunc;

    /*
     * At this point I'd like to set progress to 0 if we're
     * backgrounded, since it's hard for the user to find out.
     * It turns out it's hard enough for us to find out.
     * The problem is that zsh clears it's job table, so we
     * just don't know if we're some forked shell in a pipeline
     * somewhere or in the background.  This seems to me a problem.
     */

    zfsettype(ZFST_TYPE(zfstatusp[zfsessno]));

    if (recv)
	fflush(stdout);		/* since we may be using fd 1 */
    for (; *args; args++) {
	char *ln, *rest = NULL;
	off_t startat = 0;
	if (progress && (shfunc = getshfunc("zftp_progress"))) {
	    off_t sz = -1;
	    /*
	     * This calls the SIZE command to get the size for remote
	     * files.  Some servers send the size with the reply to
	     * the transfer command (i.e. RETR), in which
	     * case we note the fact and don't call this
	     * next time.  For that reason, the first call
	     * of zftp_progress is delayed until zfsenddata().
	     */
	    if ((!(zfprefs & ZFPF_DUMB) &&
		 (zfstatusp[zfsessno] & (ZFST_NOSZ|ZFST_TRSZ)) != ZFST_TRSZ)
		|| !recv) {
		/* the final 0 is a local fd to fstat if recv is zero */
		zfstats(*args, recv, &sz, NULL, 0);
		/* even if it doesn't support SIZE, it may tell us */
		if (recv && sz == -1)
		    getsize = 1;
	    } else
		getsize = 1;
	    zfstarttrans(*args, recv, sz);
	}

	if (flags & ZFTP_REST) {
	    startat = zstrtol(args[1], NULL, 10);
	    rest = tricat("REST ", args[1], "\r\n");
	}

	ln = tricat(cmd, *args, "\r\n");
	/* note zfsess->dfd doesn't exist till zfgetdata() creates it */
	if (zfgetdata(name, rest, ln, getsize))
	    ret = 2;
	else if (zfsenddata(name, recv, progress, startat))
	    ret = 1;
	zsfree(ln);
	/*
	 * The progress report isn't started till zfsenddata(), where
	 * it's the first item.  Hence we send a final progress report
	 * if and only if we called zfsenddata();
	 */
	if (progress && ret != 2 &&
	    (shfunc = getshfunc("zftp_progress"))) {
	    /* progress to finish: ZFTP_TRANSFER set to GF or PF */
	    int osc = sfcontext;

	    zfsetparam("ZFTP_TRANSFER", ztrdup(recv ? "GF" : "PF"),
		       ZFPM_READONLY);
	    sfcontext = SFC_HOOK;
	    doshfunc(shfunc, NULL, 1);
	    sfcontext = osc;
	}
	if (rest) {
	    zsfree(rest);
	    args++;
	}
	if (errflag)
	    break;
    }
    zfendtrans();
    return ret != 0;
}

/*
 * Delete a list of files on the server.  We allow a list by analogy with
 * `rm'.
 */

/**/
static int
zftp_delete(UNUSED(char *name), char **args, UNUSED(int flags))
{
    int ret = 0;
    char *cmd, **aptr;
    for (aptr = args; *aptr; aptr++) {
	cmd = tricat("DELE ", *aptr, "\r\n");
	if (zfsendcmd(cmd) > 2)
	    ret = 1;
	zsfree(cmd);
    }
    return ret;
}

/* Create or remove a directory on the server */

/**/
static int
zftp_mkdir(UNUSED(char *name), char **args, int flags)
{
    int ret;
    char *cmd = tricat((flags & ZFTP_DELE) ? "RMD " : "MKD ",
		       *args, "\r\n");
    ret = (zfsendcmd(cmd) > 2);
    zsfree(cmd);
    return ret;
}

/* Rename a file on the server */

/**/
static int
zftp_rename(UNUSED(char *name), char **args, UNUSED(int flags))
{
    int ret;
    char *cmd;

    cmd = tricat("RNFR ", args[0], "\r\n");
    ret = 1;
    if (zfsendcmd(cmd) == 3) {
	zsfree(cmd);
	cmd = tricat("RNTO ", args[1], "\r\n");
	if (zfsendcmd(cmd) == 2)
	    ret = 0;
    }
    zsfree(cmd);
    return ret;
}

/*
 * Send random commands, either with SITE or literal.
 * In the second case, the user better know what they're doing.
 */

/**/
static int
zftp_quote(UNUSED(char *name), char **args, int flags)
{
    int ret = 0;
    char *cmd;

    cmd = (flags & ZFTP_SITE) ? zfargstring("SITE", args)
	: zfargstring(args[0], args+1);
    ret = (zfsendcmd(cmd) > 2);
    zsfree(cmd);

    return ret;
}

/*
 * Close the connection, ending the session.  With leaveparams,
 * don't do anything to the external status (parameters, zftp_chpwd),
 * probably because this isn't the current session.
 */

/**/
static void
zfclose(int leaveparams)
{
    char **aptr;
    Shfunc shfunc;

    if (!zfsess->control)
	return;

    zfclosing = 1;
    if (zcfinish != 2) {
	/*
	 * haven't had EOF from server, so send a QUIT and get the response.
	 * maybe we should set a shorter timeout for this to avoid
	 * CTRL-c rage.
	 */
	zfsendcmd("QUIT\r\n");
    }
    if (zfsess->cin) {
	/*
	 * We fdopen'd the TCP control fd; since we can't fdclose it,
	 * we need to perform a full fclose, which invalidates the
	 * TCP fd.  We need to do this before closing the FILE, since
	 * it's not usable afterwards.
	 */
	if (fileno(zfsess->cin) == zfsess->control->fd)
	    zfsess->control->fd = -1;
	fclose(zfsess->cin);
	zfsess->cin = NULL;
    }
    if (zfsess->control) {
	zfnopen--;
	tcp_close(zfsess->control);
	/* We leak if the above failed */
	zfsess->control = NULL;
    }

    if (zfstatfd != -1) {
	zfstatusp[zfsessno] |= ZFST_CLOS;
	if (!zfnopen) {
	    /* Write the final status in case this is a subshell */
	    lseek(zfstatfd, zfsessno*sizeof(int), 0);
	    write_loop(zfstatfd, (char *)zfstatusp+zfsessno, sizeof(int));

	    close(zfstatfd);
	    zfstatfd = -1;
	}
    }

    if (!leaveparams) {
	/* Unset the non-special parameters */
	for (aptr = zfparams; *aptr; aptr++)
	    zfunsetparam(*aptr);

	/* Now ZFTP_PWD is unset.  It's up to zftp_chpwd to notice. */
	if ((shfunc = getshfunc("zftp_chpwd"))) {
	    int osc = sfcontext;

	    sfcontext = SFC_HOOK;
	    doshfunc(shfunc, NULL, 1);
	    sfcontext = osc;
	}
    }

    /* tidy up status variables, because mess is bad */
    zfclosing = zfdrrrring = 0;
}

/* Safe front end to zftp_close() from within the package */

/**/
static int
zftp_close(UNUSED(char *name), UNUSED(char **args), UNUSED(int flags))
{
    zfclose(0);
    return 0;
}


/*
 * Session management routines.  A session consists of various
 * internal variables describing the connection, the set of shell
 * parameters --- the same set which is unset by closing a connection ---
 * and the set of host/user parameters if set by zftp params.
 */

/*
 * Switch to a new session, creating it if necessary.
 * Sets zfsessno, zfsess and $ZFTP_SESSION; updates zfsesscnt and zfstatusp.
 */

/**/
static void
newsession(char *nm)
{
    LinkNode nptr;

    for (zfsessno = 0, nptr = firstnode(zfsessions);
	 nptr; zfsessno++, incnode(nptr)) {
	zfsess = (Zftp_session) nptr->dat;
	if (!strcmp(zfsess->name, nm))
	    break;
    }

    if (!nptr) {
	zfsess = (Zftp_session) zshcalloc(sizeof(struct zftp_session));
	zfsess->name = ztrdup(nm);
	zfsess->dfd = -1;
	zfsess->params = (char **) zshcalloc(sizeof(zfparams));
	zaddlinknode(zfsessions, zfsess);

	zfsesscnt++;
	zfstatusp = (int *)zrealloc(zfstatusp, sizeof(int)*zfsesscnt);
	zfstatusp[zfsessno] = 0;
    }

    zfsetparam("ZFTP_SESSION", ztrdup(zfsess->name), ZFPM_READONLY);
}

/* Save the existing session: this just means saving the parameters. */

static void
savesession(void)
{
    char **ps, **pd, *val;

    for (ps = zfparams, pd = zfsess->params; *ps; ps++, pd++) {
	if (*pd)
	    zsfree(*pd);
	queue_signals();
	if ((val = getsparam(*ps)))
	    *pd = ztrdup(val);
	else
	    *pd = NULL;
	unqueue_signals();
    }
    *pd = NULL;
}

/*
 * Switch to session nm, creating it if necessary.
 * Just call newsession, then set up the session-specific parameters.
 */

/**/
static void
switchsession(char *nm)
{
    char **ps, **pd;

    newsession(nm);

    for (ps = zfparams, pd = zfsess->params; *ps; ps++, pd++) {
	if (*pd) {
	    /* Use the permanently allocated string for the parameter */
	    zfsetparam(*ps, *pd, ZFPM_READONLY);
	    *pd = NULL;
	} else
	    zfunsetparam(*ps);
    }
}

/**/
static void
freesession(Zftp_session sptr)
{
    char **ps, **pd;
    zsfree(sptr->name);
    for (ps = zfparams, pd = zfsess->params; *ps; ps++, pd++)
	if (*pd)
	    zsfree(*pd);
    zfree(zfsess->params, sizeof(zfparams));
    if (sptr->userparams)
	freearray(sptr->userparams);
    zfree(sptr, sizeof(struct zftp_session));
}

/**/
static int
zftp_session(UNUSED(char *name), char **args, UNUSED(int flags))
{
    if (!*args) {
	LinkNode nptr;

	for (nptr = firstnode(zfsessions); nptr; incnode(nptr))
	    printf("%s\n", ((Zftp_session)nptr->dat)->name);
	return 0;
    }

    /*
     * Check if we are already in the required session: if so,
     * it's a no-op, not an error.
     */
    if (!strcmp(*args, zfsess->name))
	return 0;

    savesession();
    switchsession(*args);
    return 0;
}

/* Remove a session and free it */

/**/
static int
zftp_rmsession(UNUSED(char *name), char **args, UNUSED(int flags))
{
    int no;
    LinkNode nptr;
    Zftp_session sptr = NULL;
    char *newsess = NULL;

    /* Find the session in the list: either the current one, or by name */
    for (no = 0, nptr = firstnode(zfsessions); nptr; no++, incnode(nptr)) {
	sptr = (Zftp_session) nptr->dat;
	if ((!*args && sptr == zfsess) ||
	    (*args && !strcmp(sptr->name, *args)))
	    break;
    }
    if (!nptr)
	return 1;

    if (sptr == zfsess) {
	/* Freeing current session: make sure it's closed */
	zfclosedata();
	zfclose(0);

	/*
	 * Choose new session to switch to if any: first in list
	 * excluding the one just freed.
	 */
	if (zfsesscnt > 1) {
	    LinkNode newn = firstnode(zfsessions);
	    if (newn == nptr)
		incnode(newn);
	    newsess = ((Zftp_session)newn->dat)->name;
	}
    } else {
	Zftp_session oldsess = zfsess;
	zfsess = sptr;
	/*
	 * Freeing another session: don't need to switch, just
	 * tell zfclose() not to delete parameters etc.
	 */
	zfclosedata();
	zfclose(1);
	zfsess = oldsess;
    }
    remnode(zfsessions, nptr);
    freesession(sptr);

    /*
     * Fix up array of status pointers.
     */
    if (--zfsesscnt) {
	/*
	 * Some remaining, so just shift up
	 */
	int *newstatusp = (int *)zalloc(sizeof(int)*zfsesscnt);
	int *src, *dst, i;
	for (i = 0, src = zfstatusp, dst = newstatusp; i < zfsesscnt;
	     i++, src++, dst++) {
	    if (i == no)
		src++;
	    *dst = *src;
	}
	zfree(zfstatusp, sizeof(int)*(zfsesscnt+1));
	zfstatusp = newstatusp;

	/*
	 * Maybe we need to switch to one of the remaining sessions.
	 */
	if (newsess)
	    switchsession(newsess);
    } else {
	zfree(zfstatusp, sizeof(int));
	zfstatusp = NULL;

	/*
	 * We've just deleted the last session, so we need to
	 * start again from scratch.
	 */
	newsession("default");
    }

    return 0;
}

/* The builtin command frontend to the rest of the package */

/**/
static int
bin_zftp(char *name, char **args, UNUSED(Options ops), UNUSED(int func))
{
    char fullname[20] = "zftp ";
    char *cnam = *args++, *prefs, *ptr;
    Zftpcmd zptr;
    int n, ret = 0;

    for (zptr = zftpcmdtab; zptr->nam; zptr++)
	if (!strcmp(zptr->nam, cnam))
	    break;

    if (!zptr->nam) {
	zwarnnam(name, "no such subcommand: %s", cnam);
	return 1;
    }

    /* check number of arguments */
    for (n = 0; args[n]; n++)
	;
    if (n < zptr->min || (zptr->max != -1 && n > zptr->max)) {
	zwarnnam(name, "wrong no. of arguments for %s", cnam);
	return 1;
    }

    strcat(fullname, cnam);
    if (zfstatfd != -1 && !(zptr->flags & ZFTP_SESS)) {
	/* Get the status in case it was set by a forked process */
	int oldstatus = zfstatusp[zfsessno];
	lseek(zfstatfd, 0, 0);
	read_loop(zfstatfd, (char *)zfstatusp, sizeof(int)*zfsesscnt);
	if (zfsess->control && (zfstatusp[zfsessno] & ZFST_CLOS)) {
	    /* got closed in subshell without us knowing */
	    zcfinish = 2;
	    zfclose(0);
	} else {
	    /*
	     * fix up status types: unfortunately they may already
	     * have been looked at between being changed in the subshell
	     * and now, but we can't help that.
	     */
	    if (ZFST_TYPE(oldstatus) != ZFST_TYPE(zfstatusp[zfsessno]))
		zfsetparam("ZFTP_TYPE",
			   ztrdup(ZFST_TYPE(zfstatusp[zfsessno]) == ZFST_ASCI ?
				  "A" : "I"), ZFPM_READONLY);
	    if (ZFST_MODE(oldstatus) != ZFST_MODE(zfstatusp[zfsessno]))
		zfsetparam("ZFTP_MODE",
			   ztrdup(ZFST_MODE(zfstatusp[zfsessno]) == ZFST_BLOC ?
				  "B" : "S"), ZFPM_READONLY);
	}
    }
#if defined(HAVE_SELECT) || defined (HAVE_POLL)
    if (zfsess->control && !(zptr->flags & (ZFTP_TEST|ZFTP_SESS))) {
	/*
	 * Test the connection for a bad fd or incoming message, but
	 * only if the connection was last heard of open, and
	 * if we are not about to call the test command anyway.
	 * Not worth it unless we have select() or poll().
	 */
	ret = zftp_test("zftp test", NULL, 0);
    }
#endif
    if ((zptr->flags & ZFTP_CONN) && !zfsess->control) {
	if (ret != 2) {
	    /*
	     * with ret == 2, we just got dumped out in the test,
	     * so enough messages already.
	     */	       
	    zwarnnam(fullname, "not connected.");
	}
	return 1;
    }

    queue_signals();
    if ((prefs = getsparam_u("ZFTP_PREFS"))) {
	zfprefs = 0;
	for (ptr = prefs; *ptr; ptr++) {
	    switch (toupper(STOUC(*ptr))) {
	    case 'S':
		/* sendport */
		zfprefs |= ZFPF_SNDP;
		break;

	    case 'P':
		/*
		 * passive
		 * If we have already been told to use sendport mode,
		 * we're never going to use passive mode.
		 */
		if (!(zfprefs & ZFPF_SNDP))
		    zfprefs |= ZFPF_PASV;
		break;

	    case 'D':
		/* dumb */
		zfprefs |= ZFPF_DUMB;
		break;

	    default:
		zwarnnam(name, "preference %c not recognized", *ptr);
		break;
	    }
	}
    }
    unqueue_signals();

    ret = (*zptr->fun)(fullname, args, zptr->flags);

    if (zfalarmed)
	zfunalarm();
    if (zfdrrrring) {
	/* had a timeout, close the connection */
	zcfinish = 2;		/* don't try sending QUIT */
	zfclose(0);
    }
    if (zfstatfd != -1) {
	/*
	 * Set the status in case another process needs to know,
	 * but only for the active session.
	 */
	lseek(zfstatfd, zfsessno*sizeof(int), 0);
	write_loop(zfstatfd, (char *)zfstatusp+zfsessno, sizeof(int));
    }
    return ret;
}

static void
zftp_cleanup(void)
{
    /*
     * There are various parameters hanging around, but they're
     * all non-special so are entirely non-life-threatening.
     */
    LinkNode nptr;
    Zftp_session cursess = zfsess;
    for (zfsessno = 0, nptr = firstnode(zfsessions); nptr;
	 zfsessno++, incnode(nptr)) {
	zfsess = (Zftp_session)nptr->dat;
	zfclosedata();
	/*
	 * When closing the current session, do the usual unsetting,
	 * otherwise don't.
	 */
	zfclose(zfsess != cursess);
    }
    zsfree(lastmsg);
    lastmsg = NULL;
    zfunsetparam("ZFTP_SESSION");
    freelinklist(zfsessions, (FreeFunc) freesession);
    zfree(zfstatusp, sizeof(int)*zfsesscnt);
    zfstatusp = NULL;
}

static int
zftpexithook(UNUSED(Hookdef d), UNUSED(void *dummy))
{
    zftp_cleanup();
    return 0;
}

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/* The load/unload routines required by the zsh library interface */

/**/
int
setup_(UNUSED(Module m))
{
    return (require_module("zsh/net/tcp", NULL, 0) == 1);
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
boot_(UNUSED(Module m))
{
    /*
     * Set some default parameters.
     * These aren't special, so aren't associated with features.
     */
    off_t tmout_def = 60;
    zfsetparam("ZFTP_VERBOSE", ztrdup("450"), ZFPM_IFUNSET);
    zfsetparam("ZFTP_TMOUT", &tmout_def, ZFPM_IFUNSET|ZFPM_INTEGER);
    zfsetparam("ZFTP_PREFS", ztrdup("PS"), ZFPM_IFUNSET);
    /* default preferences if user deletes variable */
    zfprefs = ZFPF_SNDP|ZFPF_PASV;
    
    zfsessions = znewlinklist();
    newsession("default");

    addhookfunc("exit", zftpexithook);

    return 0;
}

/**/
int
cleanup_(Module m)
{
    deletehookfunc("exit", zftpexithook);
    zftp_cleanup();
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
