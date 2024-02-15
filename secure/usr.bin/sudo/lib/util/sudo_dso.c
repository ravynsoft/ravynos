/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2012-2014, 2021-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef __linux__
# include <sys/stat.h>
# include <sys/utsname.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_SHL_LOAD)
# include <dl.h>
#elif defined(HAVE_DLOPEN)
# include <dlfcn.h>
#endif
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_dso.h>
#include <sudo_util.h>

/*
 * Pointer for statically compiled symbols.
 */
static struct sudo_preload_table *preload_table;

void
sudo_dso_preload_table_v1(struct sudo_preload_table *table)
{
    preload_table = table;
}

#if defined(HAVE_SHL_LOAD)

# ifndef DYNAMIC_PATH
#  define DYNAMIC_PATH	0
# endif

void *
sudo_dso_load_v1(const char *path, int mode)
{
    struct sudo_preload_table *pt;
    int flags = DYNAMIC_PATH | BIND_VERBOSE;

    if (mode == 0)
	mode = SUDO_DSO_LAZY;	/* default behavior */

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->path != NULL && strcmp(path, pt->path) == 0)
		return pt->handle;
	}
    }

    /* We don't support SUDO_DSO_GLOBAL or SUDO_DSO_LOCAL yet. */
    if (ISSET(mode, SUDO_DSO_LAZY))
	flags |= BIND_DEFERRED;
    if (ISSET(mode, SUDO_DSO_NOW))
	flags |= BIND_IMMEDIATE;

    return (void *)shl_load(path, flags, 0L);
}

int
sudo_dso_unload_v1(void *handle)
{
    struct sudo_preload_table *pt;

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle)
		return 0;
	}
    }

    return shl_unload((shl_t)handle);
}

void *
sudo_dso_findsym_v1(void *vhandle, const char *symbol)
{
    struct sudo_preload_table *pt;
    shl_t handle = vhandle;
    void *value = NULL;

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle) {
		struct sudo_preload_symbol *sym;
		for (sym = pt->symbols; sym->name != NULL; sym++) {
		    if (strcmp(sym->name, symbol) == 0)
			return sym->addr;
		}
		errno = ENOENT;
		return NULL;
	    }
	}
    }

    /*
     * Note that the behavior of of SUDO_DSO_NEXT and SUDO_DSO_SELF
     * differs from most implementations when called from
     * a shared library.
     */
    if (vhandle == SUDO_DSO_NEXT) {
	/* Iterate over all shared libs looking for symbol. */
	shl_t myhandle = PROG_HANDLE;
	struct shl_descriptor *desc;
	int idx = 0;

	/* Find program's real handle. */
	if (shl_gethandle(PROG_HANDLE, &desc) == 0)
	    myhandle = desc->handle;
	while (shl_get(idx++, &desc) == 0) {
	    if (desc->handle == myhandle)
		continue;
	    if (shl_findsym(&desc->handle, symbol, TYPE_UNDEFINED, &value) == 0)
		break;
	}
    } else {
	if (vhandle == SUDO_DSO_DEFAULT)
	    handle = NULL;
	else if (vhandle == SUDO_DSO_SELF)
	    handle = PROG_HANDLE;
	(void)shl_findsym(&handle, symbol, TYPE_UNDEFINED, &value);
    }

    return value;
}

char *
sudo_dso_strerror_v1(void)
{
    return strerror(errno);
}

#elif defined(HAVE_DLOPEN)

# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL	0
# endif

/* Default member names for AIX when dlopen()ing an ar (.a) file. */
# ifdef RTLD_MEMBER
#  ifdef __LP64__
#   define SUDO_DSO_MEMBER	"shr_64.o"
#  else
#   define SUDO_DSO_MEMBER	"shr.o"
#  endif
# endif

# if defined(__linux__)
/*
 * On Linux systems that use multi-arch, the actual DSO may be
 * in a machine-specific subdirectory.  If the specified path
 * contains /lib/ or /libexec/, insert a multi-arch directory
 * after it.
 */
static void *
dlopen_multi_arch(const char *path, int flags)
{
    void *ret = NULL;
    struct stat sb;
    char *newpath;

    /* Only try multi-arch if the original path does not exist.  */
    if (stat(path, &sb) == -1 && errno == ENOENT) {
	newpath = sudo_stat_multiarch(path, &sb);
	if (newpath != NULL) {
	    ret = dlopen(newpath, flags);
	    free(newpath);
	}
    }
    return ret;
}
# else
static void *
dlopen_multi_arch(const char *path, int flags)
{
    return NULL;
}
# endif /* __linux__ */

void *
sudo_dso_load_v1(const char *path, int mode)
{
    struct sudo_preload_table *pt;
    int flags = 0;
    void *ret;
# ifdef RTLD_MEMBER
    char *cp;
    size_t pathlen;
# endif

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->path != NULL && strcmp(path, pt->path) == 0)
		return pt->handle;
	}
    }

    /* Map SUDO_DSO_* -> RTLD_* */
    if (ISSET(mode, SUDO_DSO_LAZY))
	SET(flags, RTLD_LAZY);
    if (ISSET(mode, SUDO_DSO_NOW))
	SET(flags, RTLD_NOW);
    if (ISSET(mode, SUDO_DSO_GLOBAL))
	SET(flags, RTLD_GLOBAL);
    if (ISSET(mode, SUDO_DSO_LOCAL))
	SET(flags, RTLD_LOCAL);

# ifdef RTLD_MEMBER
    /* Check for AIX shlib.a(member) syntax and dlopen() with RTLD_MEMBER. */
    pathlen = strlen(path);
    if (pathlen > 2 && path[pathlen - 1] == ')') {
	cp = strrchr(path, '(');
	if (cp != NULL && cp > path + 2 && cp[-2] == '.' && cp[-1] == 'a') {
	    /* Only for archive files (e.g. sudoers.a). */
	    SET(flags, RTLD_MEMBER);
	}
    }
# endif /* RTLD_MEMBER */
    ret = dlopen(path, flags);
# if defined(RTLD_MEMBER)
    /* Special fallback handling for AIX shared objects. */
    if (ret == NULL && !ISSET(flags, RTLD_MEMBER)) {
	switch (errno) {
	case ENOEXEC:
	    /*
	     * If we try to dlopen() an AIX .a file without an explicit member
	     * it will fail with ENOEXEC.  Try again using the default member.
	     */
	    if (pathlen > 2 && strcmp(&path[pathlen - 2], ".a") == 0) {
		int len = asprintf(&cp, "%s(%s)", path, SUDO_DSO_MEMBER);
		if (len != -1) {
		    ret = dlopen(cp, flags|RTLD_MEMBER);
		    free(cp);
		}
		if (ret == NULL) {
		    /* Retry with the original path to get the correct error. */
		    ret = dlopen(path, flags);
		}
	    }
	    break;
	case ENOENT:
	    /*
	     * If the .so file is missing but the .a file exists, try to
	     * dlopen() the AIX .a file using the .so name as the member.
	     * This is for compatibility with versions of sudo that use
	     * SVR4-style shared libs, not AIX-style shared libs.
	     */
	    if (pathlen > 3 && strcmp(&path[pathlen - 3], ".so") == 0) {
		int len = asprintf(&cp, "%.*s.a(%s)", (int)(pathlen - 3),
		    path, sudo_basename(path));
		if (len != -1) {
		    ret = dlopen(cp, flags|RTLD_MEMBER);
		    free(cp);
		}
		if (ret == NULL) {
		    /* Retry with the original path to get the correct error. */
		    ret = dlopen(path, flags);
		}
	    }
	    break;
	}
    }
# endif /* RTLD_MEMBER */
    /* On failure, try again with a multi-arch path where possible. */
    if (ret == NULL)
	ret = dlopen_multi_arch(path, flags); // -V1048

    return ret;
}

int
sudo_dso_unload_v1(void *handle)
{
    struct sudo_preload_table *pt;

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle)
		return 0;
	}
    }

    return dlclose(handle);
}

void *
sudo_dso_findsym_v1(void *handle, const char *symbol)
{
    struct sudo_preload_table *pt;

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle) {
		struct sudo_preload_symbol *sym;
		for (sym = pt->symbols; sym->name != NULL; sym++) {
		    if (strcmp(sym->name, symbol) == 0)
			return sym->addr;
		}
		errno = ENOENT;
		return NULL;
	    }
	}
    }

    /*
     * Not all implementations support the special handles.
     */
    if (handle == SUDO_DSO_NEXT) {
# ifdef RTLD_NEXT
	handle = RTLD_NEXT;
# else
	errno = ENOENT;
	return NULL;
# endif
    } else if (handle == SUDO_DSO_DEFAULT) {
# ifdef RTLD_DEFAULT
	handle = RTLD_DEFAULT;
# else
	errno = ENOENT;
	return NULL;
# endif
    } else if (handle == SUDO_DSO_SELF) {
# ifdef RTLD_SELF
	handle = RTLD_SELF;
# else
	errno = ENOENT;
	return NULL;
# endif
    }

    return dlsym(handle, symbol);
}

char *
sudo_dso_strerror_v1(void)
{
    return dlerror();
}

#else /* !HAVE_SHL_LOAD && !HAVE_DLOPEN */

/*
 * Emulate dlopen() using a static list of symbols compiled into sudo.
 */
void *
sudo_dso_load_v1(const char *path, int mode)
{
    struct sudo_preload_table *pt;

    /* Check prelinked symbols first. */
    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->path != NULL && strcmp(path, pt->path) == 0)
		return pt->handle;
	}
    }
    return NULL;
}

int
sudo_dso_unload_v1(void *handle)
{
    struct sudo_preload_table *pt;

    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle)
		return 0;
	}
    }
    return -1;
}

void *
sudo_dso_findsym_v1(void *handle, const char *symbol)
{
    struct sudo_preload_table *pt;

    if (preload_table != NULL) {
	for (pt = preload_table; pt->handle != NULL; pt++) {
	    if (pt->handle == handle) {
		struct sudo_preload_symbol *sym;
		for (sym = pt->symbols; sym->name != NULL; sym++) {
		    if (strcmp(sym->name, symbol) == 0)
			return sym->addr;
		}
	    }
	}
    }
    errno = ENOENT;
    return NULL;
}

char *
sudo_dso_strerror_v1(void)
{
    return strerror(errno);
}
#endif /* !HAVE_SHL_LOAD && !HAVE_DLOPEN */
