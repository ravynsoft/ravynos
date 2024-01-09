/*
 * input.c - read and store lines of input
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


/*
 * This file deals with input buffering, supplying characters to the
 * history expansion code a character at a time.  Input is stored on a
 * stack, which allows insertion of strings into the input, possibly with
 * flags marking the end of alias expansion, with minimal copying of
 * strings.  The same stack is used to record the fact that the input
 * is a history or alias expansion and to store the alias while it is in use.
 * 
 * Input is taken either from zle, if appropriate, or read directly from
 * the input file, or may be supplied by some other part of the shell (such
 * as `eval' or $(...) substitution).  In the last case, it should be
 * supplied by pushing a new level onto the stack, via inpush(input_string,
 * flag, alias); if the current input really needs to be altered, use
 * inputsetline(input_string, flag).  `Flag' can include or's of INP_FREE
 * (if the input string is to be freed when used), INP_CONT (if the input
 * is to continue onto what's already in the input queue), INP_ALIAS
 * (push supplied alias onto stack) or INP_HIST (ditto, but used to
 * mark history expansion).  `alias' is ignored unless INP_ALIAS or
 * INP_HIST is supplied.  INP_ALIAS is always set if INP_HIST is.
 * 
 * Note that the input string is itself used as the input buffer: it is not
 * copied, nor is it every written back to, so using a constant string
 * should work.  Consequently, when passing areas of memory from the heap
 * it is necessary that that heap last as long as the operation of reading
 * the string.  After the string is read, the stack should be popped with
 * inpop(), which effectively flushes any unread input as well as restoring
 * the previous input state.
 *
 * The internal flags INP_ALCONT and INP_HISTCONT show that the stack
 * element was pushed by an alias or history expansion; they should not
 * be needed elsewhere.
 *
 * The global variable inalmore is set to indicate aliases should
 * continue to be expanded because the last alias expansion ended
 * in a space.  It is only reset after a complete word was read
 * without expanding a new alias, in exalias().
 *
 * PWS 1996/12/10
 */

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include "zsh.mdh"
#include "input.pro"

/* the shell input fd */

/**/
int SHIN;

/* != 0 means we are reading input from a string */
 
/**/
int strin;
 
/* total # of characters waiting to be read. */

/**/
mod_export int inbufct;

/* the flags controlling the input routines in input.c: see INP_* in zsh.h */

/**/
int inbufflags;

static char *inbuf;		/* Current input buffer */
static char *inbufptr;		/* Pointer into input buffer */
static char *inbufpush;		/* Character at which to re-push alias */
static int inbufleft;		/* Characters left in current input
				   stack element */


 /* Input must be stacked since the input queue is used by
  * various different parts of the shell.
  */

struct instacks {
    char *buf, *bufptr;
    Alias alias;
    int bufleft, bufct, flags;
};
static struct instacks *instack, *instacktop;
/*
 * Input stack size.  We need to push the stack for aliases, history
 * expansion, and reading from internal strings: only if these operations
 * are nested do we need more than one extra level.  Thus we shouldn't need
 * too much space as a rule.  Initially, INSTACK_INITIAL is allocated; if
 * more is required, an extra INSTACK_EXPAND is added each time.
 */
#define INSTACK_INITIAL	4
#define INSTACK_EXPAND	4

static int instacksz = INSTACK_INITIAL;

/* Size of buffer for non-interactive command input */

#define SHINBUFSIZE 8192

/* Input buffer for non-interactive command input */
static char *shinbuffer;

/* Pointer into shinbuffer */
static char *shinbufptr;

/* End of contents read into shinbuffer */
static char *shinbufendptr;

/* Entry on SHIN buffer save stack */
struct shinsaveentry {
    /* Next entry on stack */
    struct shinsaveentry *next;
    /* Saved shinbuffer */
    char *buffer;
    /* Saved shinbufptr */
    char *ptr;
    /* Saved shinbufendptr */
    char *endptr;
};

/* SHIN buffer save stack */
static struct shinsaveentry *shinsavestack;

/* Reset the input buffer for SHIN, discarding any pending input */

/**/
void
shinbufreset(void)
{
    shinbufendptr = shinbufptr = shinbuffer;
}

/* Allocate a new shinbuffer
 *
 * Only called at shell initialisation and when saving on the stack.
 */

/**/
void
shinbufalloc(void)
{
    shinbuffer = zalloc(SHINBUFSIZE);
    shinbufreset();
}

/* Save entry on SHIN buffer save stack */

/**/
void
shinbufsave(void)
{
    struct shinsaveentry *entry =
	(struct shinsaveentry *)zalloc(sizeof(struct shinsaveentry));

    entry->next = shinsavestack;
    entry->buffer = shinbuffer;
    entry->ptr = shinbufptr;
    entry->endptr = shinbufendptr;

    shinsavestack = entry;

    shinbufalloc();
}

/* Restore entry from SHIN buffer save stack */

/**/
void
shinbufrestore(void)
{
    struct shinsaveentry *entry = shinsavestack;

    zfree(shinbuffer, SHINBUFSIZE);

    shinbuffer = entry->buffer;
    shinbufptr = entry->ptr;
    shinbufendptr = entry->endptr;

    shinsavestack = entry->next;
    zfree(entry, sizeof(struct shinsaveentry));
}

/* Get a character from SHIN, -1 if none available */

/**/
static int
shingetchar(void)
{
    int nread, rsize = isset(SHINSTDIN) ? 1 : SHINBUFSIZE;

    if (shinbufptr < shinbufendptr)
	return STOUC(*shinbufptr++);

    shinbufreset();
#ifdef USE_LSEEK
    if (rsize == 1 && lseek(SHIN, 0, SEEK_CUR) != (off_t)-1)
	rsize = SHINBUFSIZE;
    if (rsize > 1) {
	do {
	    errno = 0;
	    nread = read(SHIN, shinbuffer, rsize);
	} while (nread < 0 && errno == EINTR);
	if (nread <= 0)
	    return -1;
	if (isset(SHINSTDIN) &&
	    (shinbufendptr = memchr(shinbuffer, '\n', nread))) {
	    shinbufendptr++;
	    rsize = (shinbufendptr - shinbuffer);
	    if (nread > rsize &&
		lseek(SHIN, -(nread - rsize), SEEK_CUR) < 0)
		zerr("lseek(%d, %d): %e", SHIN, -(nread - rsize), errno);
	} else
	    shinbufendptr = shinbuffer + nread;
	return STOUC(*shinbufptr++);
    }
#endif
    for (;;) {
       errno = 0;
       nread = read(SHIN, shinbufendptr, 1);
       if (nread > 0) {
           /* Use line buffering (POSIX requirement) */
           if (*shinbufendptr++ == '\n')
               break;
           if (shinbufendptr == shinbuffer + SHINBUFSIZE)
               break;
       } else if (nread == 0 || errno != EINTR)
           break;
    }
    if (shinbufendptr == shinbuffer)
        return -1;
    return STOUC(*shinbufptr++);
}

/* Read a line from SHIN.  Convert tokens and   *
 * null characters to Meta c^32 character pairs. */

/**/
mod_export char *
shingetline(void)
{
    char *line = NULL;
    int ll = 0;
    int c;
    char buf[BUFSIZ];
    char *p;
    int q = queue_signal_level();

    p = buf;
    winch_unblock();
    dont_queue_signals();
    for (;;) {
	c = shingetchar();
	if (c < 0 || c == '\n') {
	    winch_block();
	    restore_queue_signals(q);
	    if (c == '\n')
		*p++ = '\n';
	    if (p > buf) {
		*p++ = '\0';
		line = zrealloc(line, ll + (p - buf));
		memcpy(line + ll, buf, p - buf);
	    }
	    return line;
	}
	if (imeta(c)) {
	    *p++ = Meta;
	    *p++ = c ^ 32;
	} else
	    *p++ = c;
	if (p >= buf + BUFSIZ - 1) {
	    winch_block();
	    queue_signals();
	    line = zrealloc(line, ll + (p - buf) + 1);
	    memcpy(line + ll, buf, p - buf);
	    ll += p - buf;
	    line[ll] = '\0';
	    p = buf;
	    winch_unblock();
	    dont_queue_signals();
	}
    }
}

/* Get the next character from the input.
 * Will call inputline() to get a new line where necessary.
 */

/**/
int
ingetc(void)
{
    int lastc = ' ';

    if (lexstop)
	return ' ';
    for (;;) {
	if (inbufleft) {
	    inbufleft--;
	    inbufct--;
	    if (itok(lastc = STOUC(*inbufptr++)))
		continue;
	    if (((inbufflags & INP_LINENO) || !strin) && lastc == '\n')
		lineno++;
	    break;
	}

	/*
	 * See if we have reached the end of input
	 * (due to an error, or to reading from a single string).
	 * Check the remaining characters left, since if there aren't
	 * any we don't want to pop the stack---it'll mark any aliases
	 * as not in use before we've finished processing.
	 */
	if (!inbufct && (strin || errflag)) {
	    lexstop = 1;
	    break;
	}
	/* If the next element down the input stack is a continuation of
	 * this, use it.
	 */
	if (inbufflags & INP_CONT) {
	    inpoptop();
	    continue;
	}
	/* As a last resort, get some more input */
	if (inputline())
	    break;
    }
    if (!lexstop)
	zshlex_raw_add(lastc);
    return lastc;
}

/* Read a line from the current command stream and store it as input */

/**/
static int
inputline(void)
{
    char *ingetcline, **ingetcpmptl = NULL, **ingetcpmptr = NULL;
    int context = ZLCON_LINE_START;

    /* If reading code interactively, work out the prompts. */
    if (interact && isset(SHINSTDIN)) {
	if (!isfirstln) {
	    ingetcpmptl = &prompt2;
	    if (rprompt2)
		ingetcpmptr = &rprompt2;
	    context = ZLCON_LINE_CONT;
	}
	else {
	    ingetcpmptl = &prompt;
	    if (rprompt)
		ingetcpmptr = &rprompt;
	}
    }
    if (!(interact && isset(SHINSTDIN) && SHTTY != -1 && isset(USEZLE))) {
	/*
	 * If not using zle, read the line straight from the input file.
	 * Possibly we don't get the whole line at once:  in that case,
	 * we get another chunk with the next call to inputline().
	 */

	if (interact && isset(SHINSTDIN)) {
	    /*
	     * We may still be interactive (e.g. running under emacs),
	     * so output a prompt if necessary.  We don't know enough
	     * about the input device to be able to handle an rprompt,
	     * though.
	     */
	    char *pptbuf;
	    int pptlen;
	    pptbuf = unmetafy(promptexpand(ingetcpmptl ? *ingetcpmptl : NULL,
					   0, NULL, NULL, NULL), &pptlen);
	    write_loop(2, pptbuf, pptlen);
	    free(pptbuf);
	}
	ingetcline = shingetline();
    } else {
	/*
	 * Since we may have to read multiple lines before getting
	 * a complete piece of input, we tell zle not to restore the
	 * original tty settings after reading each chunk.  Instead,
	 * this is done when the history mechanism for the current input
	 * terminates, which is not until we have the whole input.
	 * This is supposed to minimise problems on systems that clobber
	 * typeahead when the terminal settings are altered.
	 *                     pws 1998/03/12
	 */
	int flags = ZLRF_HISTORY|ZLRF_NOSETTY;
	if (isset(IGNOREEOF))
	    flags |= ZLRF_IGNOREEOF;
	ingetcline = zleentry(ZLE_CMD_READ, ingetcpmptl, ingetcpmptr,
			      flags, context);
	histdone |= HISTFLAG_SETTY;
    }
    if (!ingetcline) {
	return lexstop = 1;
    }
    if (errflag) {
	free(ingetcline);
	errflag |= ERRFLAG_ERROR;
	return lexstop = 1;
    }
    if (isset(VERBOSE)) {
	/* Output the whole line read so far. */
	zputs(ingetcline, stderr);
	fflush(stderr);
    }
    if (keyboardhackchar && *ingetcline &&
	ingetcline[strlen(ingetcline) - 1] == '\n' &&
	interact && isset(SHINSTDIN) &&
	SHTTY != -1 && ingetcline[1])
    {
	char *stripptr = ingetcline + strlen(ingetcline) - 2;
	if (*stripptr == keyboardhackchar) {
	    /* Junk an unwanted character at the end of the line.
	       (key too close to return key) */
	    int ct = 1;  /* force odd */
	    char *ptr;

	    if (keyboardhackchar == '\'' || keyboardhackchar == '"' ||
		keyboardhackchar == '`') {
		/*
		 * for the chars above, also require an odd count before
		 * junking
		 */
		for (ct = 0, ptr = ingetcline; *ptr; ptr++)
		    if (*ptr == keyboardhackchar)
			ct++;
	    }
	    if (ct & 1) {
		stripptr[0] = '\n';
		stripptr[1] = '\0';
	    }
	}
    }
    isfirstch = 1;
    if ((inbufflags & INP_APPEND) && inbuf) {
	/*
	 * We need new input but need to be able to back up
	 * over the old input, so append this line.
	 * Pushing the line onto the stack doesn't have the right
	 * effect.
	 *
	 * This is quite a simple and inefficient fix, but currently
	 * we only need it when backing up over a multi-line $((...
	 * that turned out to be a command substitution rather than
	 * a math substitution, which is a very special case.
	 * So it's not worth rewriting.
	 */
	char *oinbuf = inbuf;
	int newlen = strlen(ingetcline);
	int oldlen = (int)(inbufptr - inbuf) + inbufleft;
	if (inbufflags & INP_FREE) {
	    inbuf = realloc(inbuf, oldlen + newlen + 1);
	} else {
	    inbuf = zalloc(oldlen + newlen + 1);
	    memcpy(inbuf, oinbuf, oldlen);
	}
	inbufptr += inbuf - oinbuf;
	strcpy(inbuf + oldlen, ingetcline);
	free(ingetcline);
	inbufleft += newlen;
	inbufct += newlen;
	inbufflags |= INP_FREE;
    } else {
	/* Put this into the input channel. */
	inputsetline(ingetcline, INP_FREE);
    }

    return 0;
}

/*
 * Put a string in the input queue:
 * inbuf is only freeable if the flags include INP_FREE.
 */

/**/
static void
inputsetline(char *str, int flags)
{
    queue_signals();

    if ((inbufflags & INP_FREE) && inbuf) {
	free(inbuf);
    }
    inbuf = inbufptr = str;
    inbufleft = strlen(inbuf);

    /*
     * inbufct must reflect the total number of characters left,
     * as it used by other parts of the shell, so we need to take account
     * of whether the input stack continues, and whether there
     * is an extra space to add on at the end.
     */
    if (flags & INP_CONT)
	inbufct += inbufleft;
    else
	inbufct = inbufleft;
    inbufflags = flags;

    unqueue_signals();
}

/*
 * Backup one character of the input.
 * The last character can always be backed up, provided we didn't just
 * expand an alias or a history reference.
 * In fact, the character is ignored and the previous character is used.
 * (If that's wrong, the bug is in the calling code.  Use the #ifdef DEBUG
 * code to check.) 
 */

/**/
void
inungetc(int c)
{
    if (!lexstop) {
	if (inbufptr != inbuf) {
#ifdef DEBUG
	    /* Just for debugging: enable only if foul play suspected. */
	    if (inbufptr[-1] != (char) c)
		fprintf(stderr, "Warning: backing up wrong character.\n");
#endif
	    /* Just decrement the pointer:  if it's not the same
	     * character being pushed back, we're in trouble anyway.
	     */
	    inbufptr--;
	    inbufct++;
	    inbufleft++;
	    if (((inbufflags & INP_LINENO) || !strin) && c == '\n')
		lineno--;
	}
        else if (!(inbufflags & INP_CONT)) {
#ifdef DEBUG
	    /* Just for debugging */
	    fprintf(stderr, "Attempt to inungetc() at start of input.\n");
#endif
	    zerr("Garbled input at %c (binary file as commands?)", c);
	    return;
	}
	else {
	    /*
	     * The character is being backed up from a previous input stack
	     * layer.  However, there was an expansion in the middle, so we
	     * can't back up where we want to.  Instead, we just push it
	     * onto the input stack as an extra character.
	     */
	    char *cback = (char *)zshcalloc(2);
	    cback[0] = (char) c;
	    inpush(cback, INP_FREE|INP_CONT, NULL);
	}
	/* If we are back at the start of a segment,
	 * we may need to restore an alias popped from the stack.
	 * Note this may be a dummy (history expansion) entry.
	 */
	if (inbufptr == inbufpush &&
	    (inbufflags & (INP_ALCONT|INP_HISTCONT))) {
	    /*
	     * Go back up the stack over all entries which were alias
	     * expansions and were pushed with nothing remaining to read.
	     */
	    do {
		if (instacktop->alias)
		    instacktop->alias->inuse = 1;
		instacktop++;
	    } while ((instacktop->flags & (INP_ALCONT|INP_HISTCONT))
		     && !instacktop->bufleft);
	    if (inbufflags & INP_HISTCONT)
		inbufflags = INP_CONT|INP_ALIAS|INP_HIST;
	    else
		inbufflags = INP_CONT|INP_ALIAS;
	    inbufleft = 0;
	    inbuf = inbufptr = "";
	}
	zshlex_raw_back();
    }
}

/* stuff a whole file into the input queue and print it */

/**/
int
stuff(char *fn)
{
    FILE *in;
    char *buf;
    off_t len;

    if (!(in = fopen(unmeta(fn), "r"))) {
	zerr("can't open %s", fn);
	return 1;
    }
    fseek(in, 0, SEEK_END);
    len = ftell(in);
    fseek(in, 0, SEEK_SET);
    buf = (char *)zalloc(len + 1);
    if (!(fread(buf, len, 1, in))) {
	zerr("read error on %s", fn);
	fclose(in);
	zfree(buf, len + 1);
	return 1;
    }
    fclose(in);
    buf[len] = '\0';
    fwrite(buf, len, 1, stderr);
    fflush(stderr);
    inputsetline(metafy(buf, len, META_REALLOC), INP_FREE);
    return 0;
}

/* flush input queue */

/**/
void
inerrflush(void)
{
    while (!lexstop && inbufct)
	ingetc();
}

/* Set some new input onto a new element of the input stack */

/**/
mod_export void
inpush(char *str, int flags, Alias inalias)
{
    if (!instack) {
	/* Initial stack allocation */
	instack = (struct instacks *)zalloc(instacksz*sizeof(struct instacks));
	instacktop = instack;
    }

    instacktop->buf = inbuf;
    instacktop->bufptr = inbufptr;
    instacktop->bufleft = inbufleft;
    instacktop->bufct = inbufct;
    inbufflags &= ~(INP_ALCONT|INP_HISTCONT);
    if (flags & (INP_ALIAS|INP_HIST)) {
	/*
	 * Text is expansion for history or alias, so continue
	 * back to old level when done.  Also mark stack top
	 * as alias continuation so as to back up if necessary,
	 * and mark alias as in use.
	 */
	flags |= INP_CONT|INP_ALIAS;
	if (flags & INP_HIST)
	    instacktop->flags = inbufflags | INP_HISTCONT;
	else
	    instacktop->flags = inbufflags | INP_ALCONT;
	if ((instacktop->alias = inalias))
	    inalias->inuse = 1;
    } else {
	instacktop->alias = NULL;
	/* If we are continuing an alias expansion, record the alias
	 * expansion in new set of flags (do we need this?)
	 */
	if (((instacktop->flags = inbufflags) & INP_ALIAS) &&
	    (flags & INP_CONT))
	    flags |= INP_ALIAS;
    }

    instacktop++;
    if (instacktop == instack + instacksz) {
	/* Expand the stack */
	instack = (struct instacks *)
	    realloc(instack,
		    (instacksz + INSTACK_EXPAND)*sizeof(struct instacks));
	instacktop = instack + instacksz;
	instacksz += INSTACK_EXPAND;
    }
    /*
     * We maintain the entry above the highest one with real
     * text as a flag to inungetc() that it can stop re-pushing the stack.
     */
    instacktop->flags = 0;

    inbufpush = inbuf = NULL;

    inputsetline(str, flags);
}

/* Remove the top element of the stack */

/**/
static void
inpoptop(void)
{
    if (!lexstop) {
	inbufflags &= ~(INP_ALCONT|INP_HISTCONT);
	while (inbufptr > inbuf) {
	    inbufptr--;
	    inbufct++;
	    inbufleft++;
	    /*
	     * As elsewhere in input and history mechanisms:
	     * unwinding aliases and unwinding history have different
	     * implications as aliases are after the lexer while
	     * history is before, but they're both pushed onto
	     * the input stack.
	     */
	    if ((inbufflags & (INP_ALIAS|INP_HIST|INP_RAW_KEEP)) == INP_ALIAS)
		zshlex_raw_back();
	}
    }

    if (inbuf && (inbufflags & INP_FREE))
	free(inbuf);

    instacktop--;

    inbuf = instacktop->buf;
    inbufptr = inbufpush = instacktop->bufptr;
    inbufleft = instacktop->bufleft;
    inbufct = instacktop->bufct;
    inbufflags = instacktop->flags;

    if (!(inbufflags & (INP_ALCONT|INP_HISTCONT)))
	return;

    if (instacktop->alias) {
	char *t = instacktop->alias->text;
	/* a real alias:  mark it as unused. */
	instacktop->alias->inuse = 0;
	if (*t && t[strlen(t) - 1] == ' ') {
	    inalmore = 1;
	    histbackword();
	}
    }
}

/* Remove the top element of the stack and all its continuations. */

/**/
mod_export void
inpop(void)
{
    int remcont;

    do {
	remcont = inbufflags & INP_CONT;

	inpoptop();
    } while (remcont);
}

/*
 * Expunge any aliases from the input stack; they shouldn't appear
 * in the history and need to be flushed explicitly when we encounter
 * an error.
 */

/**/
void
inpopalias(void)
{
    while (inbufflags & INP_ALIAS)
	inpoptop();
}


/*
 * Get pointer to remaining string to read.
 */

/**/
char *
ingetptr(void)
{
    return inbufptr;
}

/*
 * Check if the current input line, including continuations, is
 * expanding an alias.  This does not detect alias expansions that
 * have been fully processed and popped from the input stack.
 * If there is an alias, the most recently expanded is returned,
 * else NULL.
 */

/**/
char *input_hasalias(void)
{
    int flags = inbufflags;
    struct instacks *instackptr = instacktop;

    for (;;)
    {
	if (!(flags & INP_CONT))
	    break;
	DPUTS(instackptr == instack, "BUG: continuation at bottom of instack");
	instackptr--;
	if (instackptr->alias)
	    return instackptr->alias->node.nam;
	flags = instackptr->flags;
    }

    return NULL;
}
