/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <Block.h>
#include <sys/param.h>

#include "fsck_messages.h"
#include "fsck_keys.h"
#include "fsck_msgnums.h"

extern fsck_message_t fsck_messages_common[];

// The following structures are used internally, only
struct messages {
	int low;
	int high;
	fsck_message_t *msgs;
	struct messages *next, *prev;
};

#define cfFromFD	0x01

/*
 * The internal verson of fsck_ctx_t -- this describes the output type,
 * where it goes, etc.  It's an opaque type so that it can change size
 * in the future without affecting any clients of the code.
 */

struct context {
	FILE	*fp;	// output file structure
	int	flags;	// various flags, mostly private
	int verb;	// the verbosity of the program -- controls what is output
	enum fsck_output_type style;
	enum fsck_default_answer_type resp;	// none, no, or yes
	int	num;	// number of messages in the array
	fsck_message_t	**msgs;
	void (*writer)(fsck_ctx_t, const char*);	// write strings to stdout
	void (*logger)(fsck_ctx_t, const char *);	// write strings to log file
	char guiControl;
	char xmlControl;
	char writeToLog;   // When 1, the string should be written to log file, otherwise to standard out.
	fsckBlock_t preMessage;
	fsckBlock_t postMessage;
};

/*
 * printv(fsck_ctxt_t, const char *, va_list)
 * Take the format and ap list, and turn them into a string.
 * Then call the writer to print it out (or do whatever
 * the writer wants with it, if it's an app-supplised function).
 * 
 */
static void
printv(fsck_ctx_t c, const char *fmt, va_list ap)
{
	struct context *ctx = (struct context *)c;
	char buf[BUFSIZ + 1];
	size_t length;
	va_list ap2;

	if (c == NULL)
		return;
	__va_copy(ap2, ap);	// Just in case we need it
	length = vsnprintf(buf, BUFSIZ, fmt, ap);
	if (length > BUFSIZ) {
		// We need to allocate space for it
		size_t l2 = length + 1;
		char *bufp = malloc(l2);
		if (bufp == NULL) {
			strcpy(buf, "* * * cannot allocate memory * * *\n");
			bufp = buf;
		} else {
			length = vsnprintf(bufp, length, fmt, ap2);
			if (length >= l2) {	// This should not happen!
				strcpy(buf, " * * * cannot allocate memory * * *\n");
				free(bufp);
				bufp = buf;
			} else {
				if (ctx->writer) (ctx->writer)(ctx, bufp);
				free(bufp);
				bufp = NULL;
			}
		}
		if (bufp == NULL)
			return;
	}
	
	// If the current state of printing is logging to file, 
	// call the logger that writes strings only in traditional
	// output forms.  Otherwise, print the strings in the 
	// format option provided by the caller. 
	if (ctx->writeToLog == 1) {
		if (ctx->logger) (ctx->logger)(ctx, buf);
	} else {
		if (ctx->writer) (ctx->writer)(ctx, buf);
	}
	return;
}

/*
 * printargs(fsck_ctx_t, const char *, ...)
 * An argument-list verison of printv.  It simply wraps up
 * the argument list in a va_list, and then calls printv.
 */
static void
printargs(fsck_ctx_t c, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printv(c, fmt, ap);
}

/*
 * stdprint(fsck_ctx_t, const char *)
 * Default writer.  Just prints to the set FILE*, or stdout
 * if it's not set.
 */

static void
stdprint(fsck_ctx_t c, const char *str)
{
	struct context *ctx = (struct context*)c;
	if (c) {
		fputs(str, ctx->fp ? ctx->fp : stdout);
		fflush(ctx->fp ? ctx->fp : stdout);
	}

}
/*
 * typestring(int type)
 * Return a string value corresponding to the type.  This is used
 * to present it during XML output, as one of the appropriate
 * tags.
 */
static const char *
typestring(int type) 
{
	switch (type) {
		case fsckMsgVerify:
			return kfsckVerify;
		case fsckMsgInfo:
			return kfsckInformation;
		case fsckMsgRepair:
			return kfsckRepair;
		case fsckMsgSuccess:
			return kfsckSuccess;
		case fsckMsgError:
			return kfsckError;
		case fsckMsgFail:
			return kfsckFail;
		case fsckMsgDamageInfo:
			return kfsckDamageinfo;
		case fsckMsgProgress:
			return kfsckProgress;
		case fsckMsgNotice:
			return kfsckInformation;
		default:
			return kfsckUnknown;
	}
}

/* 
 * verbosity_string(int type) 
 * Return a string value corresponding to the verbosity.  This is 
 * used to present it during XML output, as one of the appropriate 
 * tags.
 */
static const char *
verbosity_string(int level) 
{
	switch(level) {
		case fsckLevel0:
			return kfsckLevel0;
		case fsckLevel1:
		default:
			return kfsckLevel1;
	}
}

/*
 * convertfmt(const char *in)
 * This is an ugly little function whose job is to convert
 * from a normal printf-style string (e.g., "How now %s cow?")
 * into something that can be used with Cocoa formatting.  This
 * means replacing each "%<formatter>" with "%<number>$@"; the
 * reason we do this is so that the internationalized strings can
 * move parameters around as desired (e.g., in language A, the third
 * parameter may need to be first).  The caller needs to free the
 * return value.
 */
static char *
convertfmt(const char *in) 
{
	char *retval = NULL;
	int numargs = 0;
	char *cp;
	enum { fNone, fPercent } fs;

	for (cp = (char*)in; cp; cp = strchr(cp, '%')) {
		numargs++;
		cp++;
	}

	retval = calloc(1, strlen(in) + numargs * 5 + 1);
	if (retval == NULL)
		return NULL;

	fs = fNone;
	numargs = 0;
	for (cp = retval; *in; in++) {
		if (fs == fNone) {
			*cp++ = *in;
			if (*in == '%') {
				if (in[1] == '%') {
					*cp++ = '%';
					in++;
				} else {
					fs = fPercent;
					cp += sprintf(cp, "%d$@", ++numargs);
				}
			}
		} else if (fs == fPercent) {
			switch (*in) {
				case 'd': case 'i': case 'o': case 'u': case 'x': case 'l':
				case 'X': case 'D': case 'O': case 'U': case 'e':
				case 'E': case 'f': case 'F': case 'g': case 'G':
				case 'a': case 'A': case 'c': case 'C': case 's':
				case 'S': case 'p': case 'n':
					fs = fNone;
					break;
			}
		}
	}
	*cp = 0;
	return retval;
}

/*
 * fsckCreate()
 * Allocates space for an fsck_ctx_t context.  It also sets up
 * the standard message blocks (defined earlier in this file).
 * It will return NULL in the case of any error.
 */
fsck_ctx_t
fsckCreate(void) 
{
	struct context *rv = NULL;

	rv = calloc(1, sizeof(*rv));
	if (rv == NULL) {
		return NULL;
	}
	if (fsckAddMessages(rv, fsck_messages_common) == -1) {
		fsckDestroy(rv);
		return NULL;
	}
	fsckSetWriter(rv, &stdprint);

	return (fsck_ctx_t)rv;
}

/*
 * fsckSetBlock()
 * Sets the block to be called for the specific phase -- currently, only
 * before or after a message is to be printed/logged.  The block is copied
 * for later use.
 */
void
fsckSetBlock(fsck_ctx_t c, fsck_block_phase_t phase, fsckBlock_t bp)
{
	struct context *ctx = c;
	if (c != NULL) {
		switch (phase) {
		case fsckPhaseBeforeMessage:
			if (ctx->preMessage) {
				Block_release(ctx->preMessage);
				ctx->preMessage = NULL;
			}
			if (bp)
				ctx->preMessage = (fsckBlock_t)Block_copy(bp);
			break;
		case fsckPhaseAfterMessage:
			if (ctx->postMessage) {
				Block_release(ctx->postMessage);
				ctx->postMessage = NULL;
			}
			if (bp)
				ctx->postMessage = (fsckBlock_t)Block_copy(bp);
			break;
		case fsckPhaseNone:
			/* Just here for compiler warnings */
			break;
		}
		
	}
	return;
}

/*
 * fsckGetBlock()
 * Return the pointer to the block for the specified phase.  The block pointer
 * is not copied.
 */
fsckBlock_t
fsckGetBlock(fsck_ctx_t c, fsck_block_phase_t phase)
{
	struct context *ctx = c;
	fsckBlock_t retval = NULL;
	if (c != NULL) {
		switch (phase) {
		case fsckPhaseBeforeMessage:
			retval = ctx->preMessage;
			break;
		case fsckPhaseAfterMessage:
			retval = ctx->postMessage;
			break;
		case fsckPhaseNone:
			break;
		}
	}
	return retval;
}

/*
 * fsckSetWriter(context, void (*)(fsck_ctx_t, const char *)
 * Call a function for each message to be printed.
 * This defaults to stdprint (see above).
 */
int
fsckSetWriter(fsck_ctx_t c, void (*fp)(fsck_ctx_t, const char*))
{
	struct context *ctx = c;
	if (c != NULL) {
		ctx->writer = fp;
		return 0;
	} else {
		return -1;
	}
}

/* Initialize the logger function that will write strings to log file */
int
fsckSetLogger(fsck_ctx_t c, void (*fp)(fsck_ctx_t, const char*))
{
	struct context *ctx = c;
	if (c != NULL) {
		ctx->logger = fp;
		return 0;
	} else {
		return -1;
	}
}

/*
 * fsckSetOutput(context, FILE*)
 * Set the FILE* to be used for output.  Returns
 * 0 on success, and -1 if it has already been set.
 */
int
fsckSetOutput(fsck_ctx_t c, FILE *fp) 
{
	struct context *ctx = c;

	if (c != NULL) {
		ctx->fp = fp;
		return 0;
	} else
		return -1;
}

/*
 * fsckSetFile(context, fd)
 * Use a file descriptor, instead of a FILE*, for output.
 * Because of how stdio works, you should not use 1 or 2
 * for this -- use fsckSetOutput() with stdout/stderr instead.
 * If you do use this, then fsckDestroy() will close the FILE*
 * it creates here.
 * It returns -1 on error, and 0 on success.
 */
int
fsckSetFile(fsck_ctx_t c, int f) 
{
	struct context *ctx = c;

	if (c != NULL) {
		FILE *out = fdopen(f, "w");

		if (out != NULL) {
			ctx->fp = out;
			ctx->flags |= cfFromFD;
			return 0;
		}
	}
	return -1;
}

/*
 * fsckSetVerbosity(context, level)
 * Sets the verbosity level associated with this context.
 * This is used to determine which messages are output -- only
 * messages with a level equal to, or less than, the context's
 * verbosity level are output.
 */
int
fsckSetVerbosity(fsck_ctx_t c, int v) 
{
	struct context *ctx = c;

	if (c != NULL) {
		ctx->verb = v;
		return 0;
	}
	return -1;
}

/*
 * fsckGetVerbosity(context)
 * Return the verbosity level previously set, or -1 on error.
 */
int
fsckGetVerbosity(fsck_ctx_t c) 
{
	struct context *ctx = c;

	return ctx ? ctx->verb : -1;
}

/*
 * fsckSetOutputStyle(context, output_type)
 * Set the output style to one of the defined style:
 * Traditional (normal terminal-output); GUI (the parenthesized
 * method used previously by DM/DU); and XML (the new plist
 * format that is the raison d'etre for this code).  It does not
 * (yet) check if the input value is sane.
 */
int
fsckSetOutputStyle(fsck_ctx_t c, enum fsck_output_type s) 
{
	struct context *ctx = c;

	if (c != NULL) {
		ctx->style = s;
		return 0;
	}
	return -1;
}

/*
 * fsckGetStyle(context)
 * Return the output style set for this context, or
 * fsckOUtputUndefined.
 */
enum fsck_output_type
fsckGetOutputStyle(fsck_ctx_t c) 
{
	struct context *ctx = c;

	return ctx ? ctx->style : fsckOutputUndefined;
}

/*
 * fsckSetDefaultResponse(context, default_answer_tye)
 * The purpose of this function is to allow fsck to run without
 * interaction, and have a default answer (yes or no) for any
 * question that might be presented.  See fsckAskPrompt()
 */
int
fsckSetDefaultResponse(fsck_ctx_t c, enum fsck_default_answer_type r) 
{
	struct context *ctx = c;

	if (ctx) {
		ctx->resp = r;
		return 0;
	}
	return -1;
}

/*
 * fsckAskPrompt(context, prompt, ...)
 * Ask a question of the user, preceded by the given
 * printf-format prompt.  E.g., "CONTINUE? "); the
 * question mark should be included if you want it
 * displayed.  If a default answer has been set, then
 * it will be used; otherwise, it will try to get an
 * answer from the user.  Return values are 1 for "yes",
 * 0 for "no"; -1 for an invalid default; and -2 for error.
 */
int
fsckAskPrompt(fsck_ctx_t c, const char *prompt, ...) 
{
	struct context *ctx = c;
	int rv = -2;
	va_list ap;

	if (ctx == NULL)
		return -1;

	va_start(ap, prompt);

	if (ctx->style == fsckOutputTraditional && ctx->fp) {
		int count = 0;
doit:
		printv(ctx, prompt, ap);
		switch (ctx->resp) {
			default:	
				rv = -1;
				break;
			case fsckDefaultNo:
				rv = 0;
				break;
			case fsckDefaultYes:
				rv = 1;
				break;
		}
		if (rv == -1) {
			char *resp = NULL;
			size_t len;

			count++;
			resp = fgetln(stdin, &len);
			if (resp == NULL || len == 0) {
				if (count > 10) {
					// Only ask so many times...
					rv = 0;
					printargs(ctx, "%s", "\n");
					goto done;
				} else {
					goto doit;
				}
			}
			switch (resp[0]) {
				case 'y':
				case 'Y':
					rv = 1;
					break;
				case 'n':
				case 'N':
					rv = 0;
					break;
				default:
					goto doit;
			}
		} else {
			printargs(ctx, "%s", rv == 0 ? "NO\n" : "YES\n");
		}
	} else {
		switch (ctx->resp) {
			default:
				rv = -1;
				break;
			case fsckDefaultNo:
				rv = 0;
				break;
			case fsckDefaultYes:
				rv = 1;
				break;
		}
	}
done:
	return rv;
}

/*
 * fsckDestroy(context)
 * Finish up with a context, and release any resources
 * it had.
 */
void
fsckDestroy(fsck_ctx_t c) 
{
	struct context *ctx = c;

	if (c == NULL)
		return;

	if (ctx->msgs)
		free(ctx->msgs);

	if (ctx->flags & cfFromFD) {
		fclose(ctx->fp);
	}
	if (ctx->preMessage) {
		Block_release(ctx->preMessage);
	}
	if (ctx->postMessage) {
		Block_release(ctx->postMessage);
	}

	free(ctx);
	return;
}

/*
 * msgCompar(void*, void*)
 * Used by fsckAddMessages() for qsort().  All it does is
 * compare the message number for two fsck_messages.
 */
static int
msgCompar(const void *p1, const void *p2) 
{
	fsck_message_t *const *k1 = p1, *const *k2 = p2;

	return ((*k1)->msgnum - (*k2)->msgnum);
}

/*
 * fsckAddMessages(context, message*)
 * Add a block of messages to this context.  We do not assume,
 * or require, that they are in sorted order.  This is probably
 * not the best it could be, becasue first we look through the
 * block once, counting how many messages there are; then we
 * allocate extra space for the existing block, and copy in the
 * messages to it.  This means 2 passes through, which isn't ideal
 * (however, it should be called very infrequently).  After that,
 * we sort the new block, sorting based on the message number.
 * In the event of failure, it'll return -1.
 * XXX We make no attempt to ensure that there are not duplicate
 * message numbers!
 */
int
fsckAddMessages(fsck_ctx_t c, fsck_message_t *m) 
{
	struct context *ctx = c;
	fsck_message_t *ptr, **new;
	int cnt, i;

	if (ctx == NULL || m == NULL || m->msg == NULL)
		return 0;

	for (cnt = 0, ptr = m; ptr->msg; ptr++, cnt++)
		;

	new = realloc(ctx->msgs, sizeof(fsck_message_t*) * (ctx->num + cnt));
	if (new == NULL)
		return -1;
	ctx->msgs = new;

	for (i = 0; i < cnt; i++) {
		ctx->msgs[i + ctx->num] = &m[i];
	}
	ctx->num += cnt;

	qsort(ctx->msgs, ctx->num, sizeof(fsck_message_t*), msgCompar);

	return 0;
}

/*
 * bCompar(void *, void *)
 * An fsck_message_t* comparision function for
 * bsearch().  The first parameter is a pointer to
 * the message number we're searching for; the second
 * parameter is a pointer to an fsck_message_t.
 * bsearch() needs to know whether that message is less than,
 * equal to, or greater than the desired one.
 */
static int
bCompar(const void *kp, const void *ap) 
{
	const int *ip = kp;
	fsck_message_t * const *mp = ap;

	return (*ip - (*mp)->msgnum);
}

/*
 * findmessage(context, msgnum)
 * Find the desired message number in the context.  It uses
 * bsearch() and... does very little itself.  (An earlier version
 * did a lot more.)
 */
static fsck_message_t *
findmessage(struct context *ctx, int msgnum) 
{
	fsck_message_t **rv;

	if (ctx == NULL)
		return NULL;

	rv = bsearch(&msgnum, ctx->msgs, ctx->num, sizeof(rv), bCompar);

	if (rv)
		return *rv;
	else
		return NULL;
}

/*
 * fsckPrintToString(message, va_list)
 * fsckPrintString(context, message, va_list)
 * These two functions are used to print out a traditional message on the
 * console.  Note that it outputs "** " for the messages
 * it does print out (Verify, Repair, Success, and Fail);
 * other messages are not printed out.
 *
 * fsckPrintToString() is also used for message logging.
 *
 */
static char *
fsckPrintToString(fsck_message_t *m, va_list ap)
{
	char *retval = NULL;
	char *tmpstr = NULL;
	char *astr = "";	// String at beginning
	char *pstr = "";	// String at end

	/* No progress messages required in traditional output */
	if (m->type == fsckMsgProgress) {
		return NULL;
	}
	switch (m->type) {
		case fsckMsgVerify:
		case fsckMsgRepair:
		case fsckMsgSuccess:
		case fsckMsgFail:
			astr = "** ";
			break;

		case fsckMsgError:
		case fsckMsgDamageInfo: 
		case fsckMsgInfo:
			astr = "   ";
			break;
		case fsckMsgNotice:
			pstr = astr = " *****";
			break;
	}
	vasprintf(&tmpstr, m->msg, ap);
	if (tmpstr) {
		asprintf(&retval, "%s%s%s\n", astr, tmpstr, pstr);
		free(tmpstr);
	}
	return retval;
}

static int
fsckPrintString(struct context *ctx, fsck_message_t *m, va_list ap) 
{
	// Traditional fsck doesn't print this out
	if (m->type != fsckMsgProgress)
	{
		char *str = fsckPrintToString(m, ap);
		if (str) {
			printargs(ctx, "%s", str);
			free(str);
		}
	}
	return 0;
}

static char* escapeCharactersXML(char *input_string)
{
    if (input_string == NULL) {
        return NULL;
    }

    char *output_string = NULL;

    // In the worst case, each character of the input_string could be a special character,
    // Each special character can add at most 5 extra characters to the string.
    char temp[MAXPATHLEN*6 + 1];
    memset(temp, 0, MAXPATHLEN*6 + 1);

    char *input_ptr = input_string;
    char *temp_ptr = &temp[0];

    while(*input_ptr != '\0') {
        char c = *input_ptr;

        switch (c) {
            case '&': {
                *temp_ptr = '&';
                ++temp_ptr;
                *temp_ptr = 'a';
                ++temp_ptr;
                *temp_ptr = 'm';
                ++temp_ptr;
                *temp_ptr = 'p';
                ++temp_ptr;
                *temp_ptr = ';';
                ++temp_ptr;
                break;
            }

            case '<': {
                *temp_ptr = '&';
                ++temp_ptr;
                *temp_ptr = 'l';
                ++temp_ptr;
                *temp_ptr = 't';
                ++temp_ptr;
                *temp_ptr = ';';
                ++temp_ptr;
                break;
            }

            case '>': {
                *temp_ptr = '&';
                ++temp_ptr;
                *temp_ptr = 'g';
                ++temp_ptr;
                *temp_ptr = 't';
                ++temp_ptr;
                *temp_ptr = ';';
                ++temp_ptr;
                break;
            }

            case '"': {
                *temp_ptr = '&';
                ++temp_ptr;
                *temp_ptr = 'q';
                ++temp_ptr;
                *temp_ptr = 'u';
                ++temp_ptr;
                *temp_ptr = 'o';
                ++temp_ptr;
                *temp_ptr = 't';
                ++temp_ptr;
                *temp_ptr = ';';
                ++temp_ptr;
                break;
            }

            case '\'': {
                *temp_ptr = '&';
                ++temp_ptr;
                *temp_ptr = 'a';
                ++temp_ptr;
                *temp_ptr = 'p';
                ++temp_ptr;
                *temp_ptr = 'o';
                ++temp_ptr;
                *temp_ptr = 's';
                ++temp_ptr;
                *temp_ptr = ';';
                ++temp_ptr;
                break;
            }
            default: {
                *temp_ptr = c;
                ++temp_ptr;
            }
        }
        ++input_ptr;
    }

    output_string = strdup((const char *)(&temp));
    return output_string;
}

/*
 * fsckPrintXML(context, message, va_list)
 * Print out a message in XML (well, plist) format.
 * This involves printint out a standard header and closer
 * for each message, and calling fflush() when it's done.
 */
static int
fsckPrintXML(struct context *ctx, fsck_message_t *m, va_list ap) 
{
    char *newmsg = convertfmt(m->msg);
	/* See convertfmt() for details */
	if (newmsg == NULL) {
		return -1;
	}
	printargs(ctx, "%s", "<plist version=\"1.0\">\n");
	printargs(ctx, "%s", "\t<dict>\n");
	printargs(ctx, "\t\t<key>%s</key> <string>%s</string>\n",
		kfsckType, typestring(m->type));
	/*
	 * XXX - should be a "cleaner" way of doing this:  we only want
	 * to print out these keys if it's NOT a progress indicator.
	 */
	if (m->msgnum != fsckProgress) {
		printargs(ctx, "\t\t<key>%s</key> <integer>%s</integer>\n",
			kfsckVerbosity, verbosity_string(m->level));
		printargs(ctx, "\t\t<key>%s</key> <integer>%u</integer>\n",
			kfsckMsgNumber, m->msgnum);
		printargs(ctx, "\t\t<key>%s</key> <string>%s</string>\n",
			kfsckMsgString, newmsg);
	}
	if (m->numargs > 0) {
		int i;
		/*
 		 * Each parameter has a type.  This basically boils down to
		 * a string or an integer, but some kinds of strings are
		 * handled specially.  Specifically, paths, volume names,
		 * etc.
		 */
		printargs(ctx, "\t\t<key>%s</key>\n", kfsckParams);
		printargs(ctx, "%s", "\t\t<array>\n");
		for (i = 0; i < m->numargs; i++) {
			if (m->argtype[i] == fsckTypeInt) {
				int x = va_arg(ap, int);
				printargs(ctx, "\t\t\t<integer>%d</integer>\n", x);
			} else if (m->argtype[i] == fsckTypeLong) {
				long x = va_arg(ap, long);
				printargs(ctx, "\t\t\t<integer>%ld</integer>\n", x);
			} else if (m->argtype[i] == fsckTypeFileSize) {
				off_t x = va_arg(ap, off_t);
				printargs(ctx, "\t\t\t<integer>%llu</integer>\n", x);
			} else if (m->argtype[i] == fsckTypeString) {
				char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
				printargs(ctx, "\t\t\t<string>%s</string>\n", escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypePath) {
                char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
                printargs(ctx, "\t\t\t<dict><key>%s</key> <string>%s</string></dict>\n", kfsckParamPathKey, escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypeFile) {
                char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
                printargs(ctx, "\t\t\t<dict><key>%s</key> <string>%s</string></dict>\n", kfsckParamFileKey, escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypeDirectory) {
                char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
                printargs(ctx, "\t\t\t<dict><key>%s</key> <string>%s</string></dict>\n", kfsckParamDirectoryKey, escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypeVolume) {
                char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
                printargs(ctx, "\t\t\t<dict><key>%s</key> <string>%s</string></dict>\n", kfsckParamVolumeKey, escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypeFSType) {
                char *p = va_arg(ap, char*);
                char *escapedP = escapeCharactersXML(p);
                printargs(ctx, "\t\t\t<dict><key>%s</key> <string>%s</string></dict>\n", kfsckParamFSTypeKey, escapedP);
                free(escapedP);
			} else if (m->argtype[i] == fsckTypeProgress) {
				int x = va_arg(ap, int);
				printargs(ctx, "\t\t\t<integer>%d</integer>\n", x); 
			} else {
				/* XXX - what should default be --- string, integer, pointer? */
				void *p = va_arg(ap, void*);
				printargs(ctx, "\t\t\t<integer>%p</integer>\n", p);
			}
		}
		printargs(ctx, "%s", "\t\t</array>\n");
	}
	printargs(ctx, "%s", "\t</dict>\n");
	printargs(ctx, "%s", "</plist>\n");
	free(newmsg);
	return 0;
}

/*
 * fsckPrintGUI(context, message, va_list)
 * Print out a message for the previous interface for DM/DU;
 * this looks like:
 *	('X', "message", z)
 * where 'X' is a type ('S' for success, 'E' for error, and
 * '%' for progress), and z is an argument count.  (Okay,
 * progress counts are just "(% z)", where "z" is a number
 * between 0 and 100).  If there are any arguments, they follow
 * one per line.
 */
static int
fsckPrintGUI(struct context *ctx, fsck_message_t *m, va_list ap) 
{
	char t;
	int i;
	char *newmsg = convertfmt(m->msg);
	if (newmsg == NULL)
		return -1;

	switch (m->type) {
		case fsckMsgVerify:
		case fsckMsgInfo:
		case fsckMsgRepair:
		case fsckMsgSuccess:
		case fsckMsgNotice:
			t = 'S'; break;
		case fsckMsgError:
		case fsckMsgFail:
		case fsckMsgDamageInfo:
			t = 'E'; break;
		case fsckMsgProgress:
			t = '%'; break;
		default:
			t = '?'; break;
	}
	if (m->msgnum != fsckProgress) {
		printargs(ctx, "(%c,\"%s\",%d)\n", t, newmsg, m->numargs);
	}
	for (i = 0; i < m->numargs; i++) {
		switch (m->argtype[i]) {
			case fsckTypeInt:
				printargs(ctx, "%d\n", (int)va_arg(ap, int)); break;
			case fsckTypeLong:
				printargs(ctx, "%ld\n", (long)va_arg(ap, long)); break;
			case fsckTypeFileSize:
				printargs(ctx, "%llu\n", (off_t)va_arg(ap, off_t)); break;
			case fsckTypeProgress:
				printargs(ctx, "(%d %%)\n", (int)va_arg(ap, int)); break;
			case fsckTypeString:
			case fsckTypePath:
			case fsckTypeFile:
			case fsckTypeDirectory:
			case fsckTypeVolume:
			case fsckTypeFSType:
				printargs(ctx, "%s\n", (char*)va_arg(ap, char*)); break;
			default:
				printargs(ctx, "%p\n", (void*)va_arg(ap, void*)); break;
		}
	}
	free(newmsg);
	return 0;
}

/*
 * fsckPrintNothing(context, message, va_list)
 * Don't actually print anything.  Used for testing and debugging, nothing
 * else.
 */
static int
fsckPrintNothing(struct context *ctx, fsck_message_t *m, va_list ap) 
{
	return -1;
}

/*
 * fsckPrint(context, msgnum, ...)
 * Print out a message identified by msgnum, using the data and
 * context information in the contexxt.  This will look up the message,
 * and then print it out to the requested output stream using the style
 * that was selected.  It returns 0 on success, and -1 on failure.
 *
 * Note: WriteError() and RcdError() call fsckPrint internally, and 
 * therefore take care of generating the output correctly.
 */
int
fsckPrint(fsck_ctx_t c, int m, ...) 
{
	int (*func)(struct context *, fsck_message_t *, va_list);
	struct context *ctx = c;
	fsck_message_t *msg;
	va_list ap;
	int retval = 0;

	va_start(ap, m);

	if (c == NULL)
		return -1;

	msg = findmessage(ctx, m);
	assert(msg != NULL);
	if (msg == NULL) {
		return -1;	// Should log something
	}

	switch (ctx->style) {
		case fsckOutputTraditional:
			func = fsckPrintString;
			break;
		case fsckOutputGUI:
			func = fsckPrintGUI;
			break;
		case fsckOutputXML:
			func = fsckPrintXML;
			break;
		default:
			func = fsckPrintNothing;
			break;
	}

	if (ctx->preMessage) {
		va_list vaBlock;
		fsck_block_status_t rv;

		va_copy(vaBlock, ap);
		rv = (ctx->preMessage)(c, m, vaBlock);
		if (rv == fsckBlockAbort) {
			retval = -1;
			goto done;
		}
		if (rv == fsckBlockIgnore) {
			retval = 0;
			goto done;
		}
	}

	// Write string in traditional form to log file first
	ctx->writeToLog = 1;
	va_list logfile_ap;
	va_copy(logfile_ap, ap);
	retval = fsckPrintString(ctx, msg, logfile_ap);
	ctx->writeToLog = 0;

	if (ctx->writer) {
		// Now write string to standard output now as per caller's specifications
		retval = (*func)(ctx, msg, ap);
	} else {
		retval = 0;    // NULL fp means don't output anything
	}
	if (ctx->postMessage) {
		va_list vaBlock;
		fsck_block_status_t rv;

		va_copy(vaBlock, ap);
		rv = (ctx->postMessage)(c, m, vaBlock);
		if (rv == fsckBlockAbort) {
			retval = -1;
			goto done;
		}
		if (rv == fsckBlockIgnore) {
			retval = 0;
			goto done;
		}
	}
	
done:
	return retval;
}

/*
 * fsckMsgClass(context, msgnum)
 * Return the message class (Verify, Successs, Failure, etc.)
 * for a given message number.  If the message number is unknown,
 * it returns fsckMsgUnknown.
 */
enum fsck_msgtype
fsckMsgClass(fsck_ctx_t c, int msgNum) 
{
	struct context *ctx = c;
	fsck_message_t *m;

	if (c == NULL)
		return fsckMsgUnknown;

	m = findmessage(ctx, msgNum);
	if (m == NULL)
		return fsckMsgUnknown;

	return m->type;
}

/*
 * The following section is used to make the internationalizable
 * string file; this is a file that contains each message string,
 * followed by an '=' and then the string again.  This is then doctored
 * by the internationalization folks.  By putting it in here, this means
 * we need to compile the source file (and any others that have the messages
 * we care about) specially, and then be run as part of the build process.
 */
#ifdef FSCK_MAKESTRINGS
int
main(int ac, char **av) 
{
	fsck_message_t *msg;
	extern fsck_message_t hfs_errors[];
	extern fsck_message_t hfs_messages[];

	printf("/* Standard messages */\n");
	for (msg = fsck_messages_common;
	     msg->msg != NULL;
	     msg++) {
		char *newstr = convertfmt(msg->msg);

		if (newstr == NULL) {
		  printf("\"%s\" = \"%s\";\n", msg->msg, msg->msg);
		} else {
		  printf("\"%s\" = \"%s\";\n", newstr, newstr);
		  free(newstr);
		}
	}

	printf("\n/* HFS-specific standard messages */\n");
	for (msg = hfs_messages;
	     msg->msg != NULL;
	     msg++) {
		char *newstr = convertfmt(msg->msg);

		if (newstr == NULL) {
		  printf("\"%s\" = \"%s\";\n", msg->msg, msg->msg);
		} else {
		  printf("\"%s\" = \"%s\";\n", newstr, newstr);
		  free(newstr);
		}
	}

	printf("\n/* HFS-specific errors */\n");
	for (msg = hfs_errors;
	     msg->msg != NULL;
	     msg++) {
		char *newstr = convertfmt(msg->msg);

		if (newstr == NULL) {
		  printf("\"%s\" = \"%s\";\n", msg->msg, msg->msg);
		} else {
		  printf("\"%s\" = \"%s\";\n", newstr, newstr);
		  free(newstr);
		}
	}

	return 0;
}
#endif /* FSCK_MAKESTRINGS */

/*
 * This is used only for testing; it'll take some dumb arguments on
 * the command line, and then print out some messages.  It tests the
 * allocation, initialization, and searching.
 */
#ifdef FSCK_TEST
main(int ac, char **av) 
{
	fsck_ctx_t fctx;
	enum fsck_output_type t = fsckOutputUndefined;
	int (*func)(fsck_ctx_t, int, ...);
	int i;

	fctx = fsckCreate();

	if (ac == 2) {
		if (!strcmp(av[1], "-g")) {
			t = fsckOutputGUI;
			fsckSetStyle(fctx, t);
			fsckSetDefaultResponse(fctx, fsckDefaultYes);
		} else if (!strcmp(av[1], "-s")) {
			t = fsckOutputTraditional;
			fsckSetStyle(fctx, t);
		} else if (!strcmp(av[1], "-x")) {
			t = fsckOutputXML;
			fsckSetStyle(fctx, t);
			fsckSetDefaultResponse(fctx, fsckDefaultYes);
		}
	}

	fsckSetOutput(fctx, stdout);
	fsckPrint(fctx, fsckInformation, "fsck", "version");
	
	i = fsckAskPrompt(fctx, "Unknown file %s; remove? [y|n] ", "/tmp/foo");
	if (i == 1) {
		fprintf(stderr, "\n\nfile %s is to be removed\n\n", "/tmp/foo");
	}
	fsckPrint(fctx, fsckProgress, 10);
	fsckPrint(fctx, fsckVolumeNotRepaired);

	fsckDestroy(fctx);

	return 0;
}

#endif /* FSCK_TEST */
