/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_util.h>

#ifdef RTLD_PRELOAD_VAR
typedef void * (*sudo_alloc_fn_t)(size_t, size_t);
typedef void (*sudo_free_fn_t)(void *);

static void *
sudo_allocarray(size_t nmemb, size_t size)
{
    return reallocarray(NULL, nmemb, size);
}

/*
 * Allocate space for the string described by fmt and return it,
 * or NULL on error.
 * Currently only supports %%, %c, %d, and %s escapes.
 */
static char *
fmtstr(sudo_alloc_fn_t alloc_fn, sudo_free_fn_t free_fn, const char * restrict ofmt, ...)
{
    char *cp, *cur, *newstr = NULL;
    size_t len, size = 1;
    const char *fmt;
    va_list ap;
    debug_decl(fmtstr, SUDO_DEBUG_UTIL);

    /* Determine size. */
    va_start(ap, ofmt);
    for (fmt = ofmt; *fmt != '\0'; ) {
	if (fmt[0] == '%') {
	    switch (fmt[1]) {
	    case 'c':
		(void)va_arg(ap, int);
		FALLTHROUGH;
	    case '%':
		size++;
		fmt += 2;
		continue;
	    case 's':
		cp = va_arg(ap, char *);
		size += strlen(cp ? cp : "(NULL)");
		fmt += 2;
		continue;
	    case 'd': {
		char numbuf[STRLEN_MAX_SIGNED(int) + 1];
		len = (size_t)snprintf(numbuf, sizeof(numbuf), "%d",
		    va_arg(ap, int));
		if (len >= sizeof(numbuf)) {
		    goto oflow;
		}
		size += len;
		fmt += 2;
		continue;
	    }
	    default:
		/* Treat as literal. */
		break;
	    }
	}
	size++;
	fmt++;
    }
    va_end(ap);

    newstr = alloc_fn(1, size);
    if (newstr == NULL)
	debug_return_str(NULL);

    /* Format/copy data. */
    cur = newstr;
    va_start(ap, ofmt);
    for (fmt = ofmt; *fmt != '\0'; ) {
	if (fmt[0] == '%') {
	    switch (fmt[1]) {
	    case '%':
		if (size < 2) {
		    goto oflow;
		}
		*cur++ = '%';
		size--;
		fmt += 2;
		continue;
	    case 'c':
		if (size < 2) {
		    goto oflow;
		}
		*cur++ = (char )va_arg(ap, int);
		size--;
		fmt += 2;
		continue;
	    case 's':
		cp = va_arg(ap, char *);
		len = strlcpy(cur, cp ? cp : "(NULL)", size);
		if (len >= size) {
		    goto oflow;
		}
		cur += len;
		size -= len;
		fmt += 2;
		continue;
	    case 'd':
		len = (size_t)snprintf(cur, size, "%d", va_arg(ap, int));
		if (len >= size) {
		    goto oflow;
		}
		cur += len;
		size -= len;
		fmt += 2;
		continue;
	    default:
		/* Treat as literal. */
		break;
	    }
	}
	if (size < 2) {
	    goto oflow;
	}
	*cur++ = *fmt++;
	size++;
    }

    if (size < 1) {
	goto oflow;
    }
    *cur = '\0';
    va_end(ap);

    debug_return_str(newstr);

oflow:
    /* We pre-allocate enough space, so this should never happen. */
    va_end(ap);
    free_fn(newstr);
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    debug_return_str(NULL);
}

/*
 * Add a DSO file to LD_PRELOAD or the system equivalent.
 */
static char **
sudo_preload_dso_alloc(char *const envp[], const char *preload_var,
    const char *dso_file, int intercept_fd,
    sudo_alloc_fn_t alloc_fn, sudo_free_fn_t free_fn)
{
    const size_t preload_var_len = strlen(preload_var);
    char *preload = NULL;
    char **nep, **nenvp = NULL;
    char *const *ep;
    char **preload_ptr = NULL;
    char **intercept_ptr = NULL;
    char *const empty[1] = { NULL };
    bool fd_present = false;
    bool dso_present = false;
# ifdef RTLD_PRELOAD_ENABLE_VAR
    bool dso_enabled = false;
# else
    const bool dso_enabled = true;
# endif
# ifdef _PATH_ASAN_LIB
    char *dso_buf = NULL;
# endif
    size_t env_size;
    debug_decl(sudo_preload_dso_alloc, SUDO_DEBUG_UTIL);

# ifdef _PATH_ASAN_LIB
    /*
     * The address sanitizer DSO needs to be first in the list.
     */
    dso_buf = fmtstr(alloc_fn, free_fn, "%s%c%s", _PATH_ASAN_LIB,
	RTLD_PRELOAD_DELIM, dso_file);
    if (dso_buf == NULL) {
	goto oom;
    }
    dso_file = dso_buf;
# endif

    /*
     * Preload a DSO file.  For a list of LD_PRELOAD-alikes, see
     * http://www.fortran-2000.com/ArnaudRecipes/sharedlib.html
     * XXX - need to support 32-bit and 64-bit variants
     */

    /* Treat a NULL envp as empty, thanks Linux. */
    if (envp == NULL)
	envp = empty;

    /* Determine max size for new envp. */
    for (env_size = 0; envp[env_size] != NULL; env_size++)
	continue;
    if (!dso_enabled)
	env_size++;
    if (intercept_fd != -1)
	env_size++;
    env_size += 2;	/* dso_file + terminating NULL */

    /* Allocate new envp. */
    nenvp = alloc_fn(env_size, sizeof(*nenvp));
    if (nenvp == NULL)
	goto oom;

    /*
     * Shallow copy envp, with special handling for preload_var,
     * RTLD_PRELOAD_ENABLE_VAR and SUDO_INTERCEPT_FD.
     */
    for (ep = envp, nep = nenvp; *ep != NULL; ep++) {
	if (strncmp(*ep, preload_var, preload_var_len) == 0 &&
		(*ep)[preload_var_len] == '=') {
	    const char *cp = *ep + preload_var_len + 1;
	    const size_t dso_len = strlen(dso_file);

	    /* Skip duplicates. */
	    if (preload_ptr != NULL)
		continue;

	    /*
	     * Check to see if dso_file is already first in the list.
	     * We don't bother checking for it later in the list.
	     */
	    if (strncmp(cp, dso_file, dso_len) == 0) {
		if (cp[dso_len] == '\0' || cp[dso_len] == RTLD_PRELOAD_DELIM)
		    dso_present = true;
	    }

	    /* Save pointer to LD_PRELOAD variable. */
	    preload_ptr = nep;

	    goto copy;
	}
	if (intercept_fd != -1 && strncmp(*ep, "SUDO_INTERCEPT_FD=",
		sizeof("SUDO_INTERCEPT_FD=") - 1) == 0) {
	    const char *cp = *ep + sizeof("SUDO_INTERCEPT_FD=") - 1;
	    const char *errstr;
	    int fd;

	    /* Skip duplicates. */
	    if (intercept_ptr != NULL)
		continue;

	    fd = (int)sudo_strtonum(cp, 0, INT_MAX, &errstr);
	    if (fd == intercept_fd && errstr == NULL)
		fd_present = true;

	    /* Save pointer to SUDO_INTERCEPT_FD variable. */
	    intercept_ptr = nep;

	    goto copy;
	}
# ifdef RTLD_PRELOAD_ENABLE_VAR
	if (strncmp(*ep, RTLD_PRELOAD_ENABLE_VAR "=",
		sizeof(RTLD_PRELOAD_ENABLE_VAR)) == 0) {
	    dso_enabled = true;
	}
# endif
copy:
	*nep++ = *ep;	/* shallow copy */
    }

    /* Prepend our LD_PRELOAD to existing value or add new entry at the end. */
    if (!dso_present) {
	if (preload_ptr == NULL) {
# ifdef RTLD_PRELOAD_DEFAULT
	    preload = fmtstr(alloc_fn, free_fn, "%s=%s%c%s", preload_var,
		dso_file, RTLD_PRELOAD_DELIM, RTLD_PRELOAD_DEFAULT);
	    if (preload == NULL) {
		goto oom;
	    }
# else
	    preload = fmtstr(alloc_fn, free_fn, "%s=%s", preload_var,
		dso_file);
	    if (preload == NULL) {
		goto oom;
	    }
# endif
	    *nep++ = preload;
	} else {
	    const char *old_val = *preload_ptr + preload_var_len + 1;
	    preload = fmtstr(alloc_fn, free_fn, "%s=%s%c%s", preload_var,
		dso_file, RTLD_PRELOAD_DELIM, old_val);
	    if (preload == NULL) {
		goto oom;
	    }
	    *preload_ptr = preload;
	}
    }
# ifdef RTLD_PRELOAD_ENABLE_VAR
    if (!dso_enabled) {
	*nenvp++ = RTLD_PRELOAD_ENABLE_VAR "=";
    }
# endif
    if (!fd_present && intercept_fd != -1) {
	char *fdstr = fmtstr(alloc_fn, free_fn, "SUDO_INTERCEPT_FD=%d",
	    intercept_fd);
	if (fdstr == NULL) {
	    goto oom;
	}
	if (intercept_ptr != NULL) {
	    *intercept_ptr = fdstr;
	} else {
	    *nep++ = fdstr;
	}
    }

    /* NULL terminate nenvp at last. */
    *nep = NULL;

# ifdef _PATH_ASAN_LIB
    free_fn(dso_buf);
# endif

    debug_return_ptr(nenvp);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
# ifdef _PATH_ASAN_LIB
    free_fn(dso_buf);
# endif
    free_fn(preload);
    free_fn(nenvp);
    debug_return_ptr(NULL);
}

static char **
sudo_preload_dso_path(char *const envp[], const char *dso_file,
    int intercept_fd, sudo_alloc_fn_t alloc_fn, sudo_free_fn_t free_fn)
{
    char **ret = NULL;
    const char *ep;
    debug_decl(sudo_preload_dso_path, SUDO_DEBUG_UTIL);

    ep = strchr(dso_file, ':');
    if (ep == NULL) {
	/* Use default LD_PRELOAD */
	return sudo_preload_dso_alloc(envp, RTLD_PRELOAD_VAR, dso_file,
	    intercept_fd, alloc_fn, free_fn);
    }

    /* Add 32-bit LD_PRELOAD if present. */
    if (ep != dso_file) {
#ifdef RTLD_PRELOAD_VAR_32
	const size_t len = (size_t)(ep - dso_file);
	char name[PATH_MAX];

	if (len >= sizeof(name)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%.*s: path too long", (int)len, dso_file);
	} else {
	    memcpy(name, dso_file, len);
	    name[len] = '\0';
	    ret = sudo_preload_dso_alloc(envp, RTLD_PRELOAD_VAR_32, name,
		intercept_fd, alloc_fn, free_fn);
	    envp = ret;
	}
#endif /* RTLD_PRELOAD_VAR_32 */
	dso_file = ep + 1;
    }

#ifdef RTLD_PRELOAD_VAR_64
    /* Add 64-bit LD_PRELOAD if present. */
    if (*dso_file != '\0') {
	char **new_envp = sudo_preload_dso_alloc(envp, RTLD_PRELOAD_VAR_64,
	    dso_file, intercept_fd, alloc_fn, free_fn);
	free_fn(ret);
	ret = new_envp;
    }
#endif /* RTLD_PRELOAD_VAR_64 */

    debug_return_ptr(ret);
}

char **
sudo_preload_dso_mmap(char *const envp[], const char *dso_file,
    int intercept_fd)
{
    return sudo_preload_dso_path(envp, dso_file, intercept_fd,
	sudo_mmap_allocarray_v1, sudo_mmap_free_v1);
}

char **
sudo_preload_dso(char *const envp[], const char *dso_file,
    int intercept_fd)
{
    return sudo_preload_dso_path(envp, dso_file, intercept_fd,
	sudo_allocarray, free);
}
#endif /* RTLD_PRELOAD_VAR */
