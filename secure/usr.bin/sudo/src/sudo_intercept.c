/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#if defined(HAVE_SHL_LOAD)
# include <dl.h>
#elif defined(HAVE_DLOPEN)
# include <dlfcn.h>
#endif
#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>
#include <pathnames.h>

/* execl flavors */
#define SUDO_EXECL	0x0
#define SUDO_EXECLE	0x1
#define SUDO_EXECLP	0x2

#ifdef HAVE__NSGETENVIRON
# define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

extern bool command_allowed(const char *cmnd, char * const argv[], char * const envp[], char **ncmnd, char ***nargv, char ***nenvp);

typedef int (*sudo_fn_execve_t)(const char *, char *const *, char *const *);

static void
free_vector(char **vec)
{
    char **cur;
    debug_decl(free_vector, SUDO_DEBUG_EXEC);

    if (vec != NULL) {
	for (cur = vec; *cur != NULL; cur++) {
	    sudo_mmap_free(*cur);
	}
	sudo_mmap_free(vec);
    }

    debug_return;
}

static char **
copy_vector(char * const *src)
{
    char **copy;
    size_t i, len = 0;
    debug_decl(copy_vector, SUDO_DEBUG_EXEC);

    if (src != NULL) {
	while (src[len] != NULL)
	    len++;
    }
    copy = sudo_mmap_allocarray(len + 1, sizeof(char *));
    if (copy == NULL) {
	debug_return_ptr(NULL);
    }
    for (i = 0; i < len; i++) {
	copy[i] = sudo_mmap_strdup(src[i]);
	if (copy[i] == NULL) {
	    free_vector(copy);
	    debug_return_ptr(NULL);
	}
    }
    copy[i] = NULL;

    debug_return_ptr(copy);
}

/*
 * We do PATH resolution here rather than in the policy because we
 * want to use the PATH in the current environment.
 */
static bool
resolve_path(const char *cmnd, char *out_cmnd, size_t out_size)
{
    struct stat sb;
    int errval = ENOENT;
    char path[PATH_MAX];
    char **p, *cp, *endp;
    int dirlen, len;
    debug_decl(resolve_path, SUDO_DEBUG_EXEC);

    for (p = environ; (cp = *p) != NULL; p++) {
	if (strncmp(cp, "PATH=", sizeof("PATH=") - 1) == 0) {
	    cp += sizeof("PATH=") - 1;
	    break;
	}
    }
    if (cp == NULL) {
	errno = ENOENT;
	debug_return_bool(false);
    }

    endp = cp + strlen(cp);
    while (cp < endp) {
	char *colon = strchr(cp, ':');
	dirlen = colon ? (int)(colon - cp) : (int)(endp - cp);
	if (dirlen == 0) {
	    /* empty PATH component is the same as "." */
	    len = snprintf(path, sizeof(path), "./%s", cmnd);
	} else {
	    len = snprintf(path, sizeof(path), "%.*s/%s", dirlen, cp, cmnd);
	}
	cp = colon ? colon + 1 : endp;
	if (len >= ssizeof(path)) {
	    /* skip too long path */
	    errval = ENAMETOOLONG;
	    continue;
	}

	if (stat(path, &sb) == 0) {
	    if (!S_ISREG(sb.st_mode))
		continue;
	    if (strlcpy(out_cmnd, path, out_size) >= out_size) {
		errval = ENAMETOOLONG;
		break;
	    }
	    debug_return_bool(true);
	}
	switch (errno) {
	case EACCES:
	    errval = EACCES;
	    break;
	case ELOOP:
	case ENOTDIR:
	case ENOENT:
	    break;
	default:
	    debug_return_bool(false);
	}
    }
    errno = errval;
    debug_return_bool(false);
}

static int
exec_wrapper(const char *cmnd, char * const argv[], char * const envp[],
    bool is_execvp)
{
    char *cmnd_copy = NULL, **argv_copy = NULL, **envp_copy = NULL;
    char *ncmnd = NULL, **nargv = NULL, **nenvp = NULL;
    char cmnd_buf[PATH_MAX];
    void *fn = NULL;
    debug_decl(exec_wrapper, SUDO_DEBUG_EXEC);

    if (cmnd == NULL) {
	errno = EINVAL;
	debug_return_int(-1);
    }

    /* Only check PATH for the command for execlp/execvp/execvpe. */
    if (strchr(cmnd, '/') == NULL) {
	if (!is_execvp) {
	    errno = ENOENT;
	    goto bad;
	}
	if (!resolve_path(cmnd, cmnd_buf, sizeof(cmnd_buf))) {
	    goto bad;
	}
	cmnd = cmnd_buf;
    } else {
	struct stat sb;

	/* Absolute or relative path name. */
	if (stat(cmnd, &sb) == -1) {
	    /* Leave errno unchanged. */
	    goto bad;
	} else if (!S_ISREG(sb.st_mode)) {
	    errno = EACCES;
	    goto bad;
	}
    }

    /*
     * Make copies of cmnd, argv, and envp.
     */
    cmnd_copy = sudo_mmap_strdup(cmnd);
    if (cmnd_copy == NULL) {
	debug_return_int(-1);
    }
    sudo_mmap_protect(cmnd_copy);
    cmnd = cmnd_copy;

    argv_copy = copy_vector(argv);
    if (argv_copy == NULL) {
	goto bad;
    }
    sudo_mmap_protect(argv_copy);
    argv = argv_copy;

    envp_copy = copy_vector(envp);
    if (envp_copy == NULL) {
	goto bad;
    }
    sudo_mmap_protect(envp_copy);
    envp = envp_copy;

# if defined(HAVE___INTERPOSE)
    fn = execve;
# elif defined(HAVE_DLOPEN)
    fn = dlsym(RTLD_NEXT, "execve");
# elif defined(HAVE_SHL_LOAD)
    fn = sudo_shl_get_next("execve", TYPE_PROCEDURE);
# endif
    if (fn == NULL) {
        errno = EACCES;
	goto bad;
    }

    if (command_allowed(cmnd, argv, envp, &ncmnd, &nargv, &nenvp)) {
	/* Execute the command using the "real" execve() function. */
	((sudo_fn_execve_t)fn)(ncmnd, nargv, nenvp);

	/* Fall back to exec via shell for execvp and friends. */
	if (errno == ENOEXEC && is_execvp) {
	    int argc;
	    const char **shargv;

	    for (argc = 0; argv[argc] != NULL; argc++)
		continue;
	    shargv = sudo_mmap_allocarray((size_t)argc + 2, sizeof(char *));
	    if (shargv == NULL)
		goto bad;
	    shargv[0] = "sh";
	    shargv[1] = ncmnd;
	    memcpy(shargv + 2, nargv + 1, (size_t)argc * sizeof(char *));
	    ((sudo_fn_execve_t)fn)(_PATH_SUDO_BSHELL, (char **)shargv, nenvp);
	    sudo_mmap_free(shargv);
	}
    } else {
	errno = EACCES;
    }

bad:
    sudo_mmap_free(cmnd_copy);
    if (ncmnd != cmnd_copy)
	sudo_mmap_free(ncmnd);
    free_vector(argv_copy);
    if (nargv != argv_copy)
	free_vector(nargv);
    free_vector(envp_copy);
    /* Leaks allocated preload vars. */
    if (nenvp != envp_copy)
	sudo_mmap_free(nenvp);

    debug_return_int(-1);
}

static int
execl_wrapper(int type, const char *name, const char *arg, va_list ap)
{
    char * const *envp = environ;
    char **argv;
    int argc = 1;
    va_list ap2;
    debug_decl(execl_wrapper, SUDO_DEBUG_EXEC);

    if (name == NULL || arg == NULL) {
	errno = EINVAL;
	debug_return_int(-1);
    }

    va_copy(ap2, ap);
    while (va_arg(ap2, char *) != NULL)
	argc++;
    va_end(ap2);
    argv = sudo_mmap_allocarray((size_t)argc + 1, sizeof(char *));
    if (argv == NULL)
	debug_return_int(-1);

    argc = 0;
    argv[argc++] = (char *)arg;
    while ((argv[argc] = va_arg(ap, char *)) != NULL)
	argc++;
    if (type == SUDO_EXECLE)
	envp = va_arg(ap, char **);

    exec_wrapper(name, argv, envp, type == SUDO_EXECLP);
    sudo_mmap_free(argv);

    debug_return_int(-1);
}

static int
system_wrapper(const char *cmnd)
{
    const char * const argv[] = { "sh", "-c", cmnd, NULL };
    const char shell[] = _PATH_SUDO_BSHELL;
    struct sigaction saveint, savequit, sa;
    sigset_t mask, omask;
    pid_t child;
    int status;
    debug_decl(system_wrapper, SUDO_DEBUG_EXEC);

    /* Special case for NULL command, just check whether shell exists. */
    if (cmnd == NULL)
	debug_return_int(access(shell, X_OK) == 0);

    /* First, block signals to avoid potential race conditions. */
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &mask, &omask) == -1)
	debug_return_int(-1);

    switch (child = fork()) {
    case -1:
	/* error */
	(void)sigprocmask(SIG_SETMASK, &omask, NULL);
	debug_return_int(-1);
    case 0:
	/* child */
	if (sigprocmask(SIG_SETMASK, &omask, NULL) != -1)
	    exec_wrapper(shell, (char **)argv, environ, false);
	_exit(127);
    default:
	/* parent */
	break;
    }

    /* We must ignore SIGINT and SIGQUIT until the command finishes. */
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    (void)sigaction(SIGINT, &sa, &saveint);
    (void)sigaction(SIGQUIT, &sa, &savequit);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    (void)sigprocmask(SIG_UNBLOCK, &mask, NULL);

    for (;;) {
	if (waitpid(child, &status, 0) == -1) {
	    if (errno == EINTR)
		continue;
	    status = -1;
	}
	break;
    }

    /* Restore signal mask and handlers. */
    (void)sigprocmask(SIG_SETMASK, &omask, NULL);
    (void)sigaction(SIGINT, &saveint, NULL);
    (void)sigaction(SIGQUIT, &savequit, NULL);

    debug_return_int(status);
}

#ifdef HAVE___INTERPOSE
/*
 * Mac OS X 10.4 and above has support for library symbol interposition.
 * There is a good explanation of this in the Mac OS X Internals book.
 */
typedef struct interpose_s {
    void *new_func;
    void *orig_func;
} interpose_t;

static int
my_system(const char *cmnd)
{
    return system_wrapper(cmnd);
}

static int
my_execve(const char *cmnd, char * const argv[], char * const envp[])
{
    return exec_wrapper(cmnd, argv, envp, false);
}

static int
my_execv(const char *cmnd, char * const argv[])
{
    return exec_wrapper(cmnd, argv, environ, false);
}

#ifdef HAVE_EXECVPE
static int
my_execvpe(const char *cmnd, char * const argv[], char * const envp[])
{
    return exec_wrapper(cmnd, argv, envp, true);
}
#endif

static int
my_execvp(const char *cmnd, char * const argv[])
{
    return exec_wrapper(cmnd, argv, environ, true);
}

static int
my_execl(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECL, name, arg, ap);
    va_end(ap);

    return -1;
}

static int
my_execle(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECLE, name, arg, ap);
    va_end(ap);

    return -1;
}

static int
my_execlp(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECLP, name, arg, ap);
    va_end(ap);

    return -1;
}

/* Magic to tell dyld to do symbol interposition. */
__attribute__((__used__)) static const interpose_t interposers[]
__attribute__((__section__("__DATA,__interpose"))) = {
    { (void *)my_system, (void *)system },
    { (void *)my_execl, (void *)execl },
    { (void *)my_execle, (void *)execle },
    { (void *)my_execlp, (void *)execlp },
    { (void *)my_execv, (void *)execv },
    { (void *)my_execve, (void *)execve },
    { (void *)my_execvp, (void *)execvp },
#ifdef HAVE_EXECVPE
    { (void *)my_execvpe, (void *)execvpe }
#endif
};

#else /* HAVE___INTERPOSE */

# if defined(HAVE_SHL_LOAD)
static void *
sudo_shl_get_next(const char *symbol, short type)
{
    const char *name, *myname;
    struct shl_descriptor *desc;
    void *fn = NULL;
    int idx = 0;
    debug_decl(sudo_shl_get_next, SUDO_DEBUG_EXEC);

    /* Search for symbol but skip this shared object. */
    /* XXX - could be set to a different path in sudo.conf */
    myname = sudo_basename(_PATH_SUDO_INTERCEPT);
    while (shl_get(idx++, &desc) == 0) {
        name = sudo_basename(desc->filename);
        if (strcmp(name, myname) == 0)
            continue;
        if (shl_findsym(&desc->handle, symbol, type, &fn) == 0)
            break;
    }

    debug_return_ptr(fn);
}
# endif /* HAVE_SHL_LOAD */

sudo_dso_public int system(const char *cmnd);
sudo_dso_public int execve(const char *cmnd, char * const argv[], char * const envp[]);
sudo_dso_public int execv(const char *cmnd, char * const argv[]);
#ifdef HAVE_EXECVPE
sudo_dso_public int execvpe(const char *cmnd, char * const argv[], char * const envp[]);
#endif
sudo_dso_public int execvp(const char *cmnd, char * const argv[]);
sudo_dso_public int execl(const char *name, const char *arg, ...);
sudo_dso_public int execle(const char *name, const char *arg, ...);
sudo_dso_public int execlp(const char *name, const char *arg, ...);

int
system(const char *cmnd)
{
    return system_wrapper(cmnd);
}

int
execve(const char *cmnd, char * const argv[], char * const envp[])
{
    return exec_wrapper(cmnd, argv, envp, false);
}

int
execv(const char *cmnd, char * const argv[])
{
    return execve(cmnd, argv, environ);
}

#ifdef HAVE_EXECVPE
int
execvpe(const char *cmnd, char * const argv[], char * const envp[])
{
    return exec_wrapper(cmnd, argv, envp, true);
}
#endif

int
execvp(const char *cmnd, char * const argv[])
{
    return exec_wrapper(cmnd, argv, environ, true);
}

int
execl(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECL, name, arg, ap);
    va_end(ap);

    return -1;
}

int
execle(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECLE, name, arg, ap);
    va_end(ap);

    return -1;
}

int
execlp(const char *name, const char *arg, ...)
{
    va_list ap;

    va_start(ap, arg);
    execl_wrapper(SUDO_EXECLP, name, arg, ap);
    va_end(ap);

    return -1;
}
#endif /* HAVE___INTERPOSE) */
