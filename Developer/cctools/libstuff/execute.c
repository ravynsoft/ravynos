/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef RLD
#include <libc.h> /* first to get rid of pre-comp warning */
#include <mach/mach.h> /* first to get rid of pre-comp warning */
#include "stdio.h"
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <signal.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <errno.h>
#ifdef __linux__
#define _BSD_SOURCE
#include <limits.h>
#include <stdlib.h>
#endif
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/execute.h"
#include "mach-o/dyld.h"

/*
 * execute() does an execvp using the argv passed to it.  If the parameter
 * verbose is non-zero the command is printed to stderr.  A non-zero return
 * value indicates success zero indicates failure.
 */
__private_extern__
int
execute(
char **argv,
int verbose)
{
    char *name, **p;
    int forkpid, waitpid, termsig;
#ifndef __OPENSTEP__
    int waitstatus;
#else
    union wait waitstatus;
#endif

    name = argv[0];

	if(verbose){
	    fprintf(stderr, "+ %s ", name);
	    p = &(argv[1]);
	    while(*p != (char *)0)
		    fprintf(stderr, "%s ", *p++);
	    fprintf(stderr, "\n");
	}

	forkpid = fork();
	if(forkpid == -1)
	    system_fatal("can't fork a new process to execute: %s", name);

	if(forkpid == 0){
	    if(execvp(name, argv) == -1)
		system_fatal("can't find or exec: %s", name);
	    return(1); /* can't get here, removes a warning from the compiler */
	}
	else{
	    do{
	        waitpid = wait(&waitstatus);
	    } while (waitpid == -1 && errno == EINTR);
	    if(waitpid == -1)
		system_fatal("wait on forked process %d failed", forkpid);
#ifndef __OPENSTEP__
	    termsig = WTERMSIG(waitstatus);
#else
	    termsig = waitstatus.w_termsig;
#endif
	    if(termsig != 0 && termsig != SIGINT && termsig != SIGPIPE)
		fatal("fatal error in %s", name);
	    return(
#ifndef __OPENSTEP__
		WEXITSTATUS(waitstatus) == 0 &&
#else
		waitstatus.w_retcode == 0 &&
#endif
		termsig == 0);
	}
}

/*
 * runlist is used by the routine execute_list() to execute a program and it 
 * contains the command line arguments.  Strings are added to it by
 * add_execute_list().  The routine reset_execute_list() resets it for new use.
 */
static struct {
    int size;
    int next;
    char **strings;
} runlist;

/*
 * This routine is passed a string to be added to the list of strings for 
 * command line arguments.
 */
__private_extern__
void
add_execute_list(
char *str)
{
	if(runlist.strings == (char **)0){
	    runlist.next = 0;
	    runlist.size = 128;
	    runlist.strings = allocate(runlist.size * sizeof(char **));
	}
	if(runlist.next + 1 >= runlist.size){
	    runlist.strings = reallocate(runlist.strings,
				(runlist.size * 2) * sizeof(char **));
	    runlist.size *= 2;
	}
	runlist.strings[runlist.next++] = str;
	runlist.strings[runlist.next] = (char *)0;
}

/*
 * This routine is passed a string to be added to the list of strings for 
 * command line arguments and is then prefixed with the path of the executable.
 */
__private_extern__
void
add_execute_list_with_prefix(
char *str)
{
	add_execute_list(cmd_with_prefix(str));
}

/*
 * This routine is passed a string of a command name and a string is returned
 * prefixed with the path of the executable and that command name.
 */
__private_extern__
char *
cmd_with_prefix(
char *str)
{
	int i;
	char *p;
	char *prefix, buf[MAXPATHLEN], resolved_name[PATH_MAX];
	uint32_t bufsize;

	/*
	 * Construct the prefix to the program running.
	 */
	bufsize = MAXPATHLEN;
	p = buf;
	i = _NSGetExecutablePath(p, &bufsize);
	if(i == -1){
	    p = allocate(bufsize);
	    _NSGetExecutablePath(p, &bufsize);
	}
	/* cctools-port start */
#if 0 /* old code */
	prefix = realpath(p, resolved_name);
	p = rindex(prefix, '/');
	if(p != NULL)
	    p[1] = '\0';

	return(makestr(prefix, str, NULL));
#endif
	if (*p){
		prefix = realpath(p, resolved_name);
		if (prefix){
			p = rindex(prefix, '/');
			if(p != NULL)
				p[1] = '\0';
		} else{
			goto invalid;
		}
	} else{
		invalid:;
		prefix = "";
	}
	/* here we add the target alias to the command string */
	return(makestr(prefix, PROGRAM_PREFIX, str, NULL));
	/* cctools-port end */
}

/*
 * This routine reset the list of strings of command line arguments so that
 * an new command line argument list can be built.
 */
__private_extern__
void
reset_execute_list(void)
{
	runlist.next = 0;
}

/*
 * This routine calls execute() to run the command built up in the runlist
 * strings.
 */
__private_extern__
int
execute_list(
int verbose)
{
	return(execute(runlist.strings, verbose));
}
#endif /* !defined(RLD) */
