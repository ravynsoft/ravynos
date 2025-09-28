#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__APPLE__)
#include <dlfcn.h>
#include <link.h>
#endif

#include <unistd.h>

extern int real_close_for_test(int fd);

int
real_close_for_test(int fd)
{
#if defined(__APPLE__)
	return close(fd);
#else
	void *libc_handle;

#if defined(__OpenBSD__)
	struct r_debug *r_debug = NULL;
	for (Elf_Dyn *dyn = _DYNAMIC; dyn->d_tag != DT_NULL; ++dyn) {
		if (dyn->d_tag == DT_DEBUG) {
			r_debug = (struct r_debug *)dyn->d_un.d_ptr;
			break;
		}
	}
	if (!r_debug) {
		abort();
	}
	struct link_map *link_map = r_debug->r_map;
#else
	if ((libc_handle = dlopen(NULL, RTLD_NOW)) == NULL) {
		abort();
	}

#ifdef __linux__
	typedef struct link_map Link_map;
#endif

	Link_map *link_map;
	if (dlinfo(libc_handle, RTLD_DI_LINKMAP, &link_map) < 0) {
		abort();
	}
#endif

	libc_handle = NULL;
	for (; link_map != NULL; link_map = link_map->l_next) {
		char const *libname = strrchr(link_map->l_name, '/');
		libname = libname == NULL ? link_map->l_name : libname + 1;

		if (strncmp(libname, "libc.so", strlen("libc.so")) == 0) {
			libc_handle = dlopen(libname, RTLD_LAZY);
			break;
		}
	}
	if (libc_handle == NULL) {
		abort();
	}

	typeof(close) *real_close = dlsym(libc_handle, "close");
	if (real_close == NULL) {
		abort();
	}

	return real_close(fd);
#endif
}
