#ifndef HAVE_UTIMENS
#include <time.h>
int utimens(const char *path, const struct timespec times[2]);
#endif /* !HAVE_UTIMENS */

#ifndef HAVE_STRMODE
void strmode(/* mode_t */ int mode, char *p);
#endif /* !__APPLE__ */

#ifndef HAVE_REALLOCF
void *reallocf(void *ptr, size_t size);
#elif defined(HAVE_BSD_STDLIB_H)
#include <bsd/stdlib.h>
#endif /* !HAVE_REALLOCF */

#include <sys/param.h>	/* MAXPATHLEN */
#include <stdlib.h> /* system(), free() & getenv() */
#include <stdio.h> /* snprintf() & fprintf() */

int asprintf(char **strp, const char *fmt, ...);

char *find_executable(const char *name);

#define ARCHS_CONTAIN(archs, narchs, arch_wanted) \
({ \
	int result; \
	result = 0; \
	do { \
		uint32_t i; \
		for (i = 0; i < narchs; i++) { \
			cpu_type_t cputype; \
			if (archs[i].object->mh != NULL) { \
				cputype = archs[i].object->mh->cputype; \
			} else { \
				cputype = archs[i].object->mh64->cputype; \
			} \
			if (cputype == arch_wanted) { \
				result = 1; \
				break; \
			} \
		} \
	} while (0); \
	result; \
})

#define FAKE_SIGN_BINARY(filename, verbose) \
do { \
	const char *ldid_debug; \
	char ldid_command[MAXPATHLEN]; \
	char *ldid; \
	ldid_debug = getenv("LDID_DEBUG"); \
	ldid = find_executable("ldid"); \
	if (!ldid) { \
		if (ldid_debug) { \
			fprintf(stderr, "[cctools-port]: " \
			                "cannot find 'ldid' executable in PATH\n", ldid_command); \
		} \
		break; \
	} \
	snprintf(ldid_command, sizeof(ldid_command), "%s -S %s", ldid, filename); \
	if (ldid_debug || verbose) { \
		fprintf(stderr, "[cctools-port]: " \
		                "generating fake signature for '%s'\n", filename); \
		if (ldid_debug) { \
			fprintf(stderr, "[cctools-port]: %s\n", ldid_command); \
		} \
	} \
	system(ldid_command); \
	free(ldid); \
} while (0)

#define FAKE_SIGN_ARM_BINARY(archs, narchs, filename) \
do { \
	uint32_t i; \
	enum bool is_archive; \
	is_archive = FALSE; \
	if (getenv("NO_LDID")) { \
		break; \
	} \
	for (i = 0; i < narchs; i++) { \
		if (archs[i].type == OFILE_ARCHIVE) { \
			is_archive = TRUE; \
			break; \
		} \
	} \
	if (!is_archive && \
	    (ARCHS_CONTAIN(archs, narchs, CPU_TYPE_ARM) || \
	     ARCHS_CONTAIN(archs, narchs, CPU_TYPE_ARM64) || \
	     ARCHS_CONTAIN(archs, narchs, CPU_TYPE_ARM64_32))) { \
	    FAKE_SIGN_BINARY(filename, 1); \
	} \
} while (0)
