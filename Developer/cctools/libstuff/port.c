#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <mach-o/dyld.h>

#ifndef HAVE_UTIMENS
/*
 * utimens utility to set file times with sub-second resolution when available.
 * This is done by using utimensat if available at compile time.
 *
 * macOS is special cased: utimensat is only visible at compile time when
 * building for macOS >= 10.13, but with proper runtime checks we can make
 * builds targeted at older versions also work with sub-second resolution when
 * available. This is especially important because APFS introduces sub-second
 * timestamp resolution.
 */
#include <utime.h>

#ifdef HAVE_UTIMENSAT
#include <fcntl.h>
#endif

#if defined(__APPLE__) && defined(HAVE_UTIMENSAT)
#pragma weak utimensat
#endif

int utimens(const char *path, const struct timespec times[2])
{
#ifdef HAVE_UTIMENSAT
#ifdef __APPLE__
    if (utimensat != NULL)
#endif
	return utimensat(AT_FDCWD, path, times, 0);
#endif

    /* Fall back to truncating the timestamp to 1s resolution. */
#ifndef __OPENSTEP__
    struct utimbuf timep;
    timep.actime = times[0].tv_sec;
    timep.modtime = times[1].tv_sec;
    return utime(path, &timep);
#else
    time_t timep[2];
    timep[0] = times[0].tv_sec;
    timep[1] = times[1].tv_sec;
    return utime(path, timep);
#endif
}
#endif /* HAVE_UTIMENS */

#ifndef __APPLE__

#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_time.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/sysctl.h>
#endif

#ifdef __OpenBSD__
#include <sys/user.h>
#endif

int _NSGetExecutablePath(char *epath, unsigned int *size)
{
#if defined(__FreeBSD__) || defined(__DragonFly__)
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    size_t cb = *size;
    if (sysctl(mib, 4, epath, &cb, NULL, 0) != 0)
        return -1;
    *size = cb;
    return 0;
#elif defined(__OpenBSD__)
    int mib[4];
    char **argv;
    size_t len;
    const char *comm;
    int ok = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC_ARGS;
    mib[2] = getpid();
    mib[3] = KERN_PROC_ARGV;
    if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0)
        abort();
    if (!(argv = malloc(len)))
        abort();
    if (sysctl(mib, 4, argv, &len, NULL, 0) < 0)
        abort();
    comm = argv[0];
    if (*comm == '/' || *comm == '.')
    {
        char *rpath;
        if ((rpath = realpath(comm, NULL)))
        {
          strlcpy(epath, rpath, *size);
          free(rpath);
          ok = 1;
        }
    }
    else
    {
        char *sp;
        char *xpath = strdup(getenv("PATH"));
        char *path = strtok_r(xpath, ":", &sp);
        struct stat st;
        if (!xpath)
            abort();
        while (path)
        {
            snprintf(epath, *size, "%s/%s", path, comm);
            if (!stat(epath, &st) && (st.st_mode & S_IXUSR))
            {
                ok = 1;
                break;
            }
            path = strtok_r(NULL, ":", &sp);
        }
        free(xpath);
    }
    free(argv);
    if (ok)
    {
        *size = strlen(epath);
        return 0;
    }
    return -1;
#else
    int bufsize = *size;
    int ret_size;
    ret_size = readlink("/proc/self/exe", epath, bufsize-1);
    if (ret_size != -1)
    {
        *size = ret_size;
        epath[ret_size]=0;
        return 0;
    }
    else
        return -1;
#endif
}

char *mach_error_string(mach_error_t error_value)
{
    return "Unknown mach error";
}

mach_port_t mach_host_self(void)
{
    return 0;
}

#ifdef __linux__
#ifdef __x86_64__
#define EMULATED_HOST_CPU_TYPE CPU_TYPE_X86_64
#define EMULATED_HOST_CPU_SUBTYPE CPU_SUBTYPE_X86_64_ALL
#else // !__x86_64__
#define EMULATED_HOST_CPU_TYPE CPU_TYPE_ARM64
#define EMULATED_HOST_CPU_SUBTYPE CPU_SUBTYPE_ARM_ALL
#endif // !__x86_64__
#endif // __linux__
kern_return_t host_info(host_t host, host_flavor_t flavor,
                        host_info_t host_info_out,
                        mach_msg_type_number_t *host_info_outCnt)
{
    if (flavor == HOST_BASIC_INFO)
    {
        host_basic_info_t basic_info;

        basic_info = (host_basic_info_t) host_info_out;
        memset(basic_info, 0x00, sizeof(*basic_info));
        basic_info->cpu_type = EMULATED_HOST_CPU_TYPE;
        basic_info->cpu_subtype = EMULATED_HOST_CPU_SUBTYPE;
    }

  return 0;
}

mach_port_t mach_task_self_ = 0;

kern_return_t mach_port_deallocate(ipc_space_t task, mach_port_name_t name)
{
    return 0;
}

kern_return_t vm_allocate(vm_map_t target_task, vm_address_t *address,
                          vm_size_t size, int flags)
{

    vm_address_t addr = 0;

    addr = (vm_address_t)calloc(size, sizeof(char));

    if (addr == 0)
        return 1;

    *address = addr;

    return 0;
}

kern_return_t vm_deallocate(vm_map_t target_task,
                            vm_address_t address, vm_size_t size)
{
    /* free((void *)address); leak it here */
    return 0;
}

kern_return_t host_statistics (host_t host_priv, host_flavor_t flavor,
                               host_info_t host_info_out,
                               mach_msg_type_number_t *host_info_outCnt)
{
    return ENOTSUP;
}

kern_return_t map_fd(int fd, vm_offset_t offset, vm_offset_t *va,
                     boolean_t findspace, vm_size_t size)
{
    void *addr = NULL;
    addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FILE, fd, offset);
    if (addr == (void *)-1)
        return 1;
    *va = (vm_offset_t)addr;
    return 0;
}

uint64_t mach_absolute_time(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL))
      return 0;
    return (tv.tv_sec*1000000ULL)+tv.tv_usec;
}

kern_return_t mach_timebase_info(mach_timebase_info_t info)
{
    info->numer = 1000;
    info->denom = 1;
    return 0;
}

int getattrlist(const char *a,void *b, void *c, size_t d, unsigned int e)
{
    errno = ENOTSUP;
    return -1;
}

vm_size_t vm_page_size = 4096; /* hardcoded to match expectations of darwin */


#ifndef HAVE_STRMODE
#include <sys/cdefs.h>

void strmode(/* mode_t */ int mode, char *p)
{
     /* print type */
    switch (mode & S_IFMT) {
    case S_IFDIR:           /* directory */
        *p++ = 'd';
        break;
    case S_IFCHR:           /* character special */
        *p++ = 'c';
        break;
    case S_IFBLK:           /* block special */
        *p++ = 'b';
        break;
    case S_IFREG:           /* regular */
        *p++ = '-';
        break;
    case S_IFLNK:           /* symbolic link */
        *p++ = 'l';
        break;
    case S_IFSOCK:          /* socket */
        *p++ = 's';
        break;
#ifdef S_IFIFO
    case S_IFIFO:           /* fifo */
        *p++ = 'p';
        break;
#endif
#ifdef S_IFWHT
    case S_IFWHT:           /* whiteout */
        *p++ = 'w';
        break;
#endif
    default:            /* unknown */
        *p++ = '?';
        break;
    }
    /* usr */
    if (mode & S_IRUSR)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWUSR)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXUSR | S_ISUID)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXUSR:
        *p++ = 'x';
        break;
    case S_ISUID:
        *p++ = 'S';
        break;
    case S_IXUSR | S_ISUID:
        *p++ = 's';
        break;
    }
    /* group */
    if (mode & S_IRGRP)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWGRP)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXGRP | S_ISGID)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXGRP:
        *p++ = 'x';
        break;
    case S_ISGID:
        *p++ = 'S';
        break;
    case S_IXGRP | S_ISGID:
        *p++ = 's';
        break;
    }
    /* other */
    if (mode & S_IROTH)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWOTH)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXOTH | S_ISVTX)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXOTH:
        *p++ = 'x';
        break;
    case S_ISVTX:
        *p++ = 'T';
        break;
    case S_IXOTH | S_ISVTX:
        *p++ = 't';
        break;
    }
    *p++ = ' ';
    *p = '\0';
}
#endif

/*      $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $        */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
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
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';                /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1);        /* count does not include NUL */
}

#ifndef HAVE_REALLOCF
/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 1998 M. Warner Losh <imp@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
void *reallocf(void *ptr, size_t size)
{
	void *nptr;

	nptr = realloc(ptr, size);

	/*
	 * When the System V compatibility option (malloc "V" flag) is
	 * in effect, realloc(ptr, 0) frees the memory and returns NULL.
	 * So, to avoid double free, call free() only when size != 0.
	 * realloc(ptr, 0) can't fail when ptr != NULL.
	 */
	if (!nptr && ptr && size != 0)
		free(ptr);
	return (nptr);
}
#endif /* !HAVE_REALLOCF */

#endif /* !__APPLE__ */

char *find_executable(const char *name)
{
    char *p;
    char path[8192];
    char epath[MAXPATHLEN];
    char cctools_path[MAXPATHLEN];
    const char *env_path = getenv("PATH");
    struct stat st;

    if (!env_path)
        return NULL;

    unsigned int bufsize = MAXPATHLEN;

    if (_NSGetExecutablePath(cctools_path, &bufsize) == -1)
        cctools_path[0] = '\0';

    if ((p = strrchr(cctools_path, '/')))
        *p = '\0';

    snprintf(path, sizeof(path), "%s:%s", cctools_path, env_path);

    p = strtok(path, ":");

    while (p != NULL)
    {
        snprintf(epath, sizeof(epath), "%s/%s", p, name);

        if ((p = realpath(epath, NULL)))
        {
            strlcpy(epath, p, sizeof(epath));
            free(p);
        }

        if (stat(epath, &st) == 0 && access(epath, F_OK|X_OK) == 0)
            return strdup(epath);

        p = strtok(NULL, ":");
    }

    return NULL;
}