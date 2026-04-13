/*
 * Copyright (c) 1999-2000, 2002, 2004, 2007-2008 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/syslimits.h>
#include <pwd.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sys/sysctl.h>
#include <fcntl.h>

#include "fsck_hfs.h"

char *rawname __P((char *name));
char *unrawname __P((char *name));


int
reply(char *question)
{
	int persevere;
	char c;

	if (preen)
		pfatal("INTERNAL ERROR: GOT TO reply()");
	persevere = !strcmp(question, "CONTINUE");
	plog("\n");
	if (!persevere && (nflag || fswritefd < 0)) {
		plog("%s? no\n\n", question);
		return (0);
	}
	if (yflag || (persevere && nflag)) {
		plog("%s? yes\n\n", question);
		return (1);
	}
	do	{
		plog("%s? [yn] ", question);
		(void) fflush(stdout);
		c = getc(stdin);
		while (c != '\n' && getc(stdin) != '\n')
			if (feof(stdin))
				return (0);
	} while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');
	plog("\n");
	if (c == 'y' || c == 'Y')
		return (1);
	return (0);
}


void
ckfini(markclean)
	int markclean;
{
//	register struct bufarea *bp, *nbp;
//	int ofsmodified, cnt = 0;

	(void) CacheDestroy(&fscache);

	if (fswritefd < 0) {
		(void)close(fsreadfd);
		return;
	}
#if 0
	flush(fswritefd, &sblk);
	if (havesb && sblk.b_bno != SBOFF / dev_bsize &&
	    !preen && reply("UPDATE STANDARD SUPERBLOCK")) {
		sblk.b_bno = SBOFF / dev_bsize;
		sbdirty();
		flush(fswritefd, &sblk);
	}
	flush(fswritefd, &cgblk);
	free(cgblk.b_un.b_buf);
	for (bp = bufhead.b_prev; bp && bp != &bufhead; bp = nbp) {
		cnt++;
		flush(fswritefd, bp);
		nbp = bp->b_prev;
		free(bp->b_un.b_buf);
		free((char *)bp);
	}
	if (bufhead.b_size != cnt)
		errx(EEXIT, "Panic: lost %d buffers", bufhead.b_size - cnt);
	pbp = pdirbp = (struct bufarea *)0;
	if (markclean && sblock.fs_clean == 0) {
		sblock.fs_clean = 1;
		sbdirty();
		ofsmodified = fsmodified;
		flush(fswritefd, &sblk);
		fsmodified = ofsmodified;
		if (!preen)
			plog("\n***** FILE SYSTEM MARKED CLEAN *****\n");
	}
	if (debug)
		plog("cache missed %ld of %ld (%d%%)\n", diskreads,
		    totalreads, (int)(diskreads * 100 / totalreads));
#endif
	(void)close(fsreadfd);
	(void)close(fswritefd);
}


char *
blockcheck(char *origname)
{
	struct stat stslash, stblock, stchar;
	char *newname, *raw = NULL;
	int retried = 0;

	hotroot = 0;
	if (stat("/", &stslash) < 0) {
		perror("/");
		plog("Can't stat root\n");
		return (origname);
	}
	newname = origname;
retry:
    if (!strncmp(newname, "/dev/fd/", 8)) {
        detonator_run = 1;
        return (origname);
    } else {
        detonator_run = 0;
    }
    
	if (stat(newname, &stblock) < 0) {
		perror(newname);
		plog("Can't stat %s\n", newname);
		return (origname);
	}
	if ((stblock.st_mode & S_IFMT) == S_IFBLK) {
		if (stslash.st_dev == stblock.st_rdev)
			hotroot++;
		raw = rawname(newname);
		if (stat(raw, &stchar) < 0) {
			perror(raw);
			plog("Can't stat %s\n", raw);
			return (origname);
		}
		if ((stchar.st_mode & S_IFMT) == S_IFCHR) {
			return (raw);
		} else {
			plog("%s is not a character device\n", raw);
			return (origname);
		}
	} else if ((stblock.st_mode & S_IFMT) == S_IFCHR && !retried) {
		newname = unrawname(newname);
		retried++;
		goto retry;
	}
	/*
	 * Not a block or character device, just return name and
	 * let the caller decide whether to use it.
	 */
	return (origname);
}


char *
rawname(char *name)

{
	static char rawbuf[32];
	char *dp;

	if ((dp = strrchr(name, '/')) == 0)
		return (0);
	*dp = 0;
	(void)strlcpy(rawbuf, name, sizeof(rawbuf));
	*dp = '/';
	(void)strlcat(rawbuf, "/r", sizeof(rawbuf));
	(void)strlcat(rawbuf, &dp[1], sizeof(rawbuf));

	return (rawbuf);
}


char *
unrawname(char *name)
{
	char *dp;
	struct stat stb;

	if ((dp = strrchr(name, '/')) == 0)
		return (name);
	if (stat(name, &stb) < 0)
		return (name);
	if ((stb.st_mode & S_IFMT) != S_IFCHR)
		return (name);
	if (dp[1] != 'r')
		return (name);
	memmove(&dp[1], &dp[2], strlen(&dp[2]) + 1);

	return (name);
}


void
catch(sig)
	int sig;
{
	if (!upgrading)
		ckfini(0);
	exit(12);
}


//
// Logging stuff...
//
//
#include <stdarg.h>
#include <pthread.h>
#include <time.h>

#define FSCK_LOG_FILE "/var/log/fsck_hfs.log"

extern char lflag;           // indicates if we're doing a live fsck (defined in fsck_hfs.c)
extern char guiControl;      // indicates if we're outputting for the gui (defined in fsck_hfs.c)
extern char xmlControl; 	 // indicates if we're outputting XML output for GUI / Disk Utility (defined in fsck_hfs.c). 

FILE   *log_file = NULL;

/* Variables for in-memory log for strings that will be written to log file */
char   *in_mem_log = NULL;
char   *cur_in_mem_log = NULL;
size_t  in_mem_log_size = 0;

/* Variables for in-memory log for strings that will be printed on standard out */
char   *in_mem_out = NULL;
char   *cur_in_mem_out = NULL;
size_t  in_mem_out_size = 0;

int     live_fsck = 0;

#define DEFAULT_IN_MEM_SIZE  4096

static pthread_mutex_t mem_buf_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  mem_buf_cond;

static pthread_t       printing_thread;
static pthread_t       logging_thread;
static volatile int    keep_going = 1;

#undef fprintf
#undef printf

// prototype
void print_to_mem(int type, int mem_type, const char *fmt, const char *str, va_list ap);

#define  DO_VPRINT   1    // types for print_to_mem
#define  DO_STR      2

/* Types for mem_type */
#define IN_MEM_LOG   1  // in-memory log strings
#define IN_MEM_OUT   2  // in-memory stdout strings

static void *
fsck_logging_thread(void *arg)
{ 
    int  copy_amt;
    char buff[1024], *ptr;
    
    /* Handle writing to the log file */
    while(keep_going || cur_in_mem_log != in_mem_log) {

	pthread_mutex_lock(&mem_buf_lock);
	while (keep_going != 0 && cur_in_mem_log == in_mem_log) {
	    int err;

	    err = pthread_cond_wait(&mem_buf_cond, &mem_buf_lock);
	    if (err != 0) {
		fprintf(stderr, "error %d from cond wait\n", err);
		break;
	    }
	}	    

	copy_amt = (int)(cur_in_mem_log - in_mem_log);
	if (copy_amt == 0) {
	    pthread_mutex_unlock(&mem_buf_lock);
	    continue;
	}
	
	if (copy_amt >= sizeof(buff)) {
	    copy_amt = sizeof(buff) - 1;
	    memcpy(buff, in_mem_log, copy_amt);

	    memmove(in_mem_log, &in_mem_log[copy_amt], (cur_in_mem_log - in_mem_log) - copy_amt);
	    cur_in_mem_log -= copy_amt;
	} else {
	    memcpy(buff, in_mem_log, copy_amt);
	    cur_in_mem_log = in_mem_log;
	}

	buff[copy_amt] = '\0';
	
	pthread_mutex_unlock(&mem_buf_lock);

	for(ptr=buff; *ptr; ) {
	    char *start;
	    
	    start = ptr;
	    while(*ptr && *ptr != '\n') {
		ptr++;
	    }
	    if (*ptr == '\n') {
		*ptr++ = '\0';
		if (log_file) {
		    fprintf(log_file, "%s: %s\n", cdevname ? cdevname : "UNKNOWN-DEV", start);
		}
	    } else {
		if (log_file) {
		    fprintf(log_file, "%s", start);
		}
	    }
	    
	}

	fflush(stdout);
    }

    return NULL;
}

static void *
fsck_printing_thread(void *arg)
{ 
    int  copy_amt;
    char buff[1024], *ptr;
    
    /* Handle writing to the out file */
    while(keep_going || cur_in_mem_out != in_mem_out) {

	pthread_mutex_lock(&mem_buf_lock);
	while (keep_going != 0 && cur_in_mem_out == in_mem_out) {
	    int err;

	    err = pthread_cond_wait(&mem_buf_cond, &mem_buf_lock);
	    if (err != 0) {
		fprintf(stderr, "error %d from cond wait\n", err);
		break;
	    }
	}	    

	copy_amt = (int)(cur_in_mem_out - in_mem_out);
	if (copy_amt == 0) {
	    pthread_mutex_unlock(&mem_buf_lock);
	    continue;
	}
	
	if (copy_amt >= sizeof(buff)) {
	    copy_amt = sizeof(buff) - 1;
	    memcpy(buff, in_mem_out, copy_amt);

	    memmove(in_mem_out, &in_mem_out[copy_amt], (cur_in_mem_out - in_mem_out) - copy_amt);
	    cur_in_mem_out -= copy_amt;
	} else {
	    memcpy(buff, in_mem_out, copy_amt);
	    cur_in_mem_out = in_mem_out;
	}

	buff[copy_amt] = '\0';
	
	pthread_mutex_unlock(&mem_buf_lock);

	for(ptr=buff; *ptr; ) {
	    char *start;
	    
	    start = ptr;
	    while(*ptr && *ptr != '\n') {
		ptr++;
	    }
	    if (*ptr == '\n') {
		*ptr++ = '\0';
		printf("%s\n", start);
	    } else {
		printf("%s", start);
	    }
	    
	}

	fflush(stdout);
    }

    return NULL;
}
    
static FILE *
safely_open_log_file(const char *path)
{
    int fd = open(path, O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW, 0666);
    if (fd < 0)
        return NULL;

    struct stat sb;
    if (fstat(fd, &sb) || !S_ISREG(sb.st_mode)) {
        close(fd);
        errno = EPERM;
        return NULL;
    }

    return fdopen(fd, "a");
}

int was_signaled = 0;

void
shutdown_logging(void)
{
    keep_going = 0;
    time_t t;
    
    /* Log fsck_hfs check completion time */
    t = time(NULL);
    if (in_mem_log) {
	va_list empty_list = {0};
	print_to_mem(DO_STR, IN_MEM_LOG, "fsck_hfs completed at %s\n", ctime(&t), empty_list);
    } else {
	fprintf(log_file, "%s: fsck_hfs completed at %s\n", cdevname ? cdevname : "UNKNOWN-DEV", ctime(&t));
    }

    if (was_signaled) {
	// if we were signaled, we can't really call any of these
	// functions from the context of a signal handler (which
	// is how we're called if we don't have a signal handler).
	// so we have our own signal handler which sets this var
	// which tells us to just bail out.
	return;
    }

    if (log_file && !live_fsck) {
	fflush(log_file);
	fclose(log_file);
	log_file = NULL;
    } else if ((in_mem_out || in_mem_log) && live_fsck && log_file) {
	// make sure the printing and logging threads are woken up...
	pthread_mutex_lock(&mem_buf_lock);
	pthread_cond_broadcast(&mem_buf_cond);
	pthread_mutex_unlock(&mem_buf_lock);
	
	// then wait for them 
	pthread_join(printing_thread, NULL);
	pthread_join(logging_thread, NULL);

	free(in_mem_out);
	in_mem_out = cur_in_mem_out = NULL;
	in_mem_out_size = 0;

	free(in_mem_log);
	in_mem_log = cur_in_mem_log = NULL;
	in_mem_log_size = 0;

	if (log_file) {
	    fflush(log_file);
	    fclose(log_file);
	    log_file = NULL;
	}
    } else if (in_mem_log) {
	int ret;
	
	if (getuid() == 0) {
	    // just in case, flush any pending output
	    fflush(stdout);
	    fflush(stderr);
	    
	    //
	    // fork so that the child can wait around until the
	    // root volume is mounted read-write and we can add
	    // our output to the log
	    //
	    ret = fork();
	} else {
	    // if we're not root we don't need to fork
	    ret = 0;
	}
	if (ret == 0) {
	    int i;
	    char *fname = FSCK_LOG_FILE, path[PATH_MAX];

		// Disk Management waits for fsck_hfs' stdout to close rather
		// than the process death to understand if fsck_hfs has exited 
		// or not.  Since we do not use stdout any further, close all 
		// the file descriptors so that Disk Management does not wait 
		// for 60 seconds unnecessarily on read-only boot volumes.
	    	fclose(stdout);
	    	fclose(stdin);
	    	fclose(stderr);

	    // non-root will never be able to write to /var/log
	    // so point the file somewhere else.
	    if (getuid() != 0) {
	    	struct passwd *pwd;
		fname = NULL;
		// each user will get their own log as ~/Library/Logs/fsck_hfs.log
		pwd = getpwuid(getuid());
		if (pwd) {
			snprintf(path, sizeof(path), "%s/Library/Logs/fsck_hfs.log", pwd->pw_dir);
			fname = &path[0];
		} 
	    }

	    for(i=0; i < 60; i++) {
            log_file = safely_open_log_file(fname);
            if (log_file) {
                fwrite(in_mem_log, cur_in_mem_log - in_mem_log, 1, log_file);

                fflush(log_file);
                fclose(log_file);
                log_file = NULL;

                free(in_mem_log);
                in_mem_log = cur_in_mem_log = NULL;
                in_mem_log_size = 0;

                break;
            } else {
                // hmmm, failed to open the output file so wait
                // a while only if the fs is read-only and then
                // try again
                if (errno == EROFS) {
                    sleep(1);
                } else {
                    break;
                }
            }
	    }
	}
    }
}

static void
my_sighandler(int sig)
{
    was_signaled = 1;
    cleanup_fs_fd();
    exit(sig);
}


void
setup_logging(void)
{
    static int at_exit_setup = 0;
    time_t t;
    
    // if this is set, we don't have to do anything
    if (at_exit_setup) {
	return;
    }

    if (guiControl) {
	    setlinebuf(stdout);
	    setlinebuf(stderr);
    }
    
    if (detonator_run) {
        // Do not create a log file 
        return;
    }

    // our copy of this variable since we may
    // need to change it to make the right thing
    // happen for fsck on the root volume.
    live_fsck = (int)lflag;

    if (log_file == NULL) {
        log_file = safely_open_log_file(FSCK_LOG_FILE);
	if (log_file) {
	    setlinebuf(log_file);
	} else {
	    //
	    // if we can't open the output file it's either because
	    // we're being run on the root volume during early boot
	    // or we were not run as the root user and so we can't
	    // write to /var/log/fsck_hfs.log.  in either case we
	    // turn off "live_fsck" so that the right thing happens
	    // in here with respect to where output goes.
	    //
	    live_fsck = 0;
	}

	if (!live_fsck && log_file) {
	    t = time(NULL);
	    fprintf(log_file, "\n%s: fsck_hfs started at %s", cdevname ? cdevname : "UNKNOWN-DEV", ctime(&t));
	    fflush(log_file);

	} else if (live_fsck || in_mem_log == NULL || in_mem_out == NULL) {
	    //
	    // hmm, we couldn't open the log file (or it's a
	    // live fsck).  let's just squirrel away a copy 
	    // of the data in memory and then deal with it
	    // later (or print it out from a separate thread
	    // if we're doing a live fsck).
	    //
	    in_mem_log = (char *)malloc(DEFAULT_IN_MEM_SIZE);
	    in_mem_out = (char *)malloc(DEFAULT_IN_MEM_SIZE);
	    if ((in_mem_log != NULL) && (in_mem_out != NULL)) {
		in_mem_log_size = DEFAULT_IN_MEM_SIZE;
		in_mem_log[0] = '\0';
		cur_in_mem_log = in_mem_log;

		in_mem_out_size = DEFAULT_IN_MEM_SIZE;
		in_mem_out[0] = '\0';
		cur_in_mem_out = in_mem_out;

		t = time(NULL);
		va_list empty_list = {0};
		print_to_mem(DO_STR, IN_MEM_LOG, "\nfsck_hfs started at %s", ctime(&t), empty_list);

		if (live_fsck && log_file) {
		    pthread_cond_init(&mem_buf_cond, NULL);
		    
		    signal(SIGINT,  my_sighandler);
		    signal(SIGHUP,  my_sighandler);
		    signal(SIGTERM, my_sighandler);
		    signal(SIGQUIT, my_sighandler);		    
		    signal(SIGBUS,  my_sighandler);		    
		    signal(SIGSEGV, my_sighandler);		    
		    signal(SIGILL,  my_sighandler);		    
		    
		    pthread_create(&printing_thread, NULL, fsck_printing_thread, NULL);
		    pthread_create(&logging_thread, NULL, fsck_logging_thread, NULL);

		}
	    }
	}

	if (at_exit_setup == 0 && (log_file || in_mem_log || in_mem_out)) {
	    atexit(shutdown_logging);
	    at_exit_setup = 1;
	}
    }
}


void
print_to_mem(int type, int mem_type, const char *fmt, const char *str, va_list ap)
{
    int ret;
    size_t size_remaining;
    va_list ap_copy;
    char *cur_in_mem;
    char *in_mem_data;
    size_t in_mem_data_size;
    
    if (type == DO_VPRINT) {
	va_copy(ap_copy, ap);
    }
	
	/* Grab the lock only when adding output strings to the in-memory data */
	if (live_fsck && (mem_type == IN_MEM_OUT)) {
		pthread_mutex_lock(&mem_buf_lock);
	}
	
    if (mem_type == IN_MEM_LOG) {
	    cur_in_mem = cur_in_mem_log;
	    in_mem_data = in_mem_log;
	    in_mem_data_size = in_mem_log_size;
    } else {
	    cur_in_mem = cur_in_mem_out;
	    in_mem_data = in_mem_out;
	    in_mem_data_size = in_mem_out_size;
    }
	
    size_remaining = in_mem_data_size - (ptrdiff_t)(cur_in_mem - in_mem_data);
    if (type == DO_VPRINT) {
	ret = vsnprintf(cur_in_mem, size_remaining, fmt, ap);
    } else {
	ret = snprintf(cur_in_mem, size_remaining, fmt, str);
    }
    if (ret > size_remaining) {
	char *new_log;
	size_t amt;

	if (ret >= DEFAULT_IN_MEM_SIZE) {
	    amt = (ret + 4095) & (~4095);   // round up to a 4k boundary
	} else {
	    amt = DEFAULT_IN_MEM_SIZE;
	}
	    
	new_log = realloc(in_mem_data, in_mem_data_size + amt);
	if (new_log == NULL)
	    goto done;

	in_mem_data_size += amt;
	cur_in_mem = new_log + (cur_in_mem - in_mem_data);
	in_mem_data = new_log;
	size_remaining = in_mem_data_size - (ptrdiff_t)(cur_in_mem - new_log);
	if (type == DO_VPRINT) {
	    ret = vsnprintf(cur_in_mem, size_remaining, fmt, ap_copy);
	} else {
	    ret = snprintf(cur_in_mem, size_remaining, fmt, str);
	}
	if (ret <= size_remaining) {
	    cur_in_mem += ret;
	}
    } else {
	cur_in_mem += ret;
    }

done:

    if (mem_type == IN_MEM_LOG) {
	    cur_in_mem_log = cur_in_mem;
	    in_mem_log = in_mem_data;
	    in_mem_log_size = in_mem_data_size;
    } else {
	    cur_in_mem_out = cur_in_mem;
	    in_mem_out = in_mem_data;
	    in_mem_out_size = in_mem_data_size;
    }
	
	if (live_fsck && (mem_type == IN_MEM_OUT)) {
		pthread_cond_signal(&mem_buf_cond);
		pthread_mutex_unlock(&mem_buf_lock);
	}

    if (type == DO_VPRINT) {
	va_end(ap_copy);
    }
}


static int need_prefix=1;

#define LOG_PREFIX   \
	if (need_prefix) { \
            fprintf(log_file, "%s: ", cdevname); \
	    if (strchr(fmt, '\n')) { \
		need_prefix = 1; \
	    } else { \
		need_prefix = 0; \
	    } \
	} else if (strchr(fmt, '\n')) { \
	    need_prefix = 1; \
	}

/* Print output string on given stream or store it into in-memory buffer */
#define VOUT(stream, fmt, ap) \
    if (!live_fsck) { \
	vfprintf(stream, fmt, ap);		\
    } else { \
	print_to_mem(DO_VPRINT, IN_MEM_OUT, fmt, NULL, ap);	\
    }

#define FOUT(fmt, str) \
    print_to_mem(DO_STR, IN_MEM_OUT, fmt, str, NULL);	

/* Store output string written to fsck_hfs.log into file or in-memory buffer */
#define VLOG(fmt, ap) \
    va_start(ap, fmt); \
    VLOG_INTERNAL(fmt, ap);

#define VLOG_INTERNAL(fmt, ap) \
    if (log_file && !live_fsck) { \
	LOG_PREFIX \
	vfprintf(log_file, fmt, ap); \
    } else { \
	print_to_mem(DO_VPRINT, IN_MEM_LOG, fmt, NULL, ap);	\
    }

#define FLOG(fmt, str) \
    if (log_file && !live_fsck) { \
	LOG_PREFIX;				\
	fprintf(log_file, fmt, str);		\
    } else { \
	va_list empty_list = {0}; \
	print_to_mem(DO_STR, IN_MEM_LOG, fmt, str, empty_list);	\
    }


#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * An unexpected inconsistency occurred.
 * Die if preening, otherwise just print message and continue.
 */
void
#if __STDC__
pfatal(const char *fmt, ...)
#else
pfatal(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

	setup_logging();
	
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	if (!preen) {
		(void)vfprintf(stderr, fmt, ap);
		VLOG(fmt, ap);
		va_end(ap);
		return;
	}
	if (!live_fsck) 
	    (void)fprintf(stderr, "%s: ", cdevname);
	FLOG("%s: ", cdevname);
	
	if (!live_fsck) 
	    (void)vfprintf(stderr, fmt, ap);
	VLOG(fmt, ap);
	
	if (!live_fsck) 
	    (void)fprintf(stderr,
		"\n%s: UNEXPECTED INCONSISTENCY; RUN fsck_hfs MANUALLY.\n",
		cdevname);
	FLOG("\n%s: UNEXPECTED INCONSISTENCY; RUN fsck_hfs MANUALLY.\n", cdevname);

	exit(EEXIT);
}

/*
 * Pwarn just prints a message when not preening,
 * or a warning (preceded by filename) when preening.
 */
void
#if __STDC__
pwarn(const char *fmt, ...)
#else
pwarn(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

	setup_logging();

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	if (preen) {
		(void)fprintf(stderr, "%s: ", cdevname);
		FLOG("%s: ", cdevname);
	}
	if (!live_fsck) 
	    (void)vfprintf(stderr, fmt, ap);
	VLOG(fmt, ap);
	
	va_end(ap);
}

/* Write a string and parameters, if any, directly to the log file.
 * These strings will not be printed to standard out/error.
 */
void 
logstring(void *c, const char *str)
{
	llog("%s", str);
}

/* Write a string and parameters, if any, directly to standard out/error.
 * These strings will not be printed to log file.
 */
void 
outstring(void *c, const char *str) 
{
	olog("%s", str);
}

/* Write to both standard out and log file */
void
plog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vplog(fmt, ap);
    va_end(ap);
}

/* Write to only standard out */
void
olog(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	setup_logging();

	/* 
	 * For live fsck_hfs, add output strings to in-memory log, 
	 * and for non-live fsck_hfs, print output to stdout. 
	 */
	VOUT(stdout, fmt, ap);

	va_end(ap);
}

/* Write to only log file */
void 
llog(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	setup_logging();
	need_prefix = 1;
	VLOG(fmt, ap);

	va_end(ap);
}

/* Write to both standard out and log file */
void
vplog(const char *fmt, va_list ap)
{
	va_list copy_ap;

	va_copy(copy_ap, ap);

	setup_logging();

	/* Always print prefix to strings written to log files */
	need_prefix = 1;

	/* Handle output strings, print to stdout or store in-memory, if not running in XML mode */
	if (xmlControl == 0) {
		/*
		 * If running in XML mode do not put non-XML formatted output into stdout, as it may cause
		 * DiskMgmt to complain. 
		 */ 
		VOUT(stdout, fmt, ap);
	}

	/* Add log strings to the log file.  VLOG() handles live case internally */
	VLOG_INTERNAL(fmt, copy_ap);
}

/* Write to both the given stream (usually stderr!) and log file */
void
fplog(FILE *stream, const char *fmt, ...)
{
	va_list ap, copy_ap;
	va_start(ap, fmt);
	va_copy(copy_ap, ap);

	setup_logging();
	need_prefix = 1;

	/* Handle output strings, print to given stream or store in-memory */
	VOUT(stream, fmt, ap);

	/* Add log strings to the log file.  VLOG() handles live case internally */
	VLOG(fmt, copy_ap);

	va_end(ap);
}

#define kProgressToggle	"kern.progressmeterenable"
#define	kProgress	"kern.progressmeter"

void
start_progress(void)
{
	int rv;
	int enable = 1;
	if (hotroot == 0)
		return;
	rv = sysctlbyname(kProgressToggle, NULL, NULL, &enable, sizeof(enable));
	if (debug && rv == -1 && errno != ENOENT) {
		warn("sysctl(%s) failed", kProgressToggle);
	}
}

void
draw_progress(int pct)
{
	int rv;
	if (hotroot == 0)
		return;
	rv = sysctlbyname(kProgress, NULL, NULL, &pct, sizeof(pct));
	if (debug && rv == -1 && errno != ENOENT) {
		warn("sysctl(%s) failed", kProgress);
	}
}

void
end_progress(void)
{
	int rv;
	int enable = 0;
	if (hotroot == 0)
		return;
	rv = sysctlbyname(kProgressToggle, NULL, NULL, &enable, sizeof(enable));
	if (debug && rv == -1 && errno != ENOENT) {
		warn("sysctl(%s) failed", kProgressToggle);
	}
}

