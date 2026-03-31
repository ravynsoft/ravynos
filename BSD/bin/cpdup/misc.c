/*
 * MISC.C
 */

#include "cpdup.h"

void
logstd(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vprintf(ctl, va);
    va_end(va);
}

void
logerr(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vfprintf(stderr, ctl, va);
    va_end(va);
}

char *
mprintf(const char *ctl, ...)
{
    char *ptr;
    va_list va;

    ptr = NULL;

    va_start(va, ctl);
    if (vasprintf(&ptr, ctl, va) < 0)
	fatal("malloc failed");
    va_end(va);
    assert(ptr != NULL);
    return(ptr);
}

char *
fextract(FILE *fi, int n, int *pc, int skip)
{
    int i;
    int c;
    int imax;
    char *s;

    i = 0;
    c = *pc;
    imax = (n < 0) ? 64 : n + 1;

    s = malloc(imax);
    if (s == NULL)
	fatal("out of memory");

    while (c != EOF) {
	if (n == 0 || (n < 0 && (c == ' ' || c == '\n')))
	    break;

	s[i++] = c;
	if (i == imax) {
	    imax += 64;
	    s = realloc(s, imax);
	    if (s == NULL)
		fatal("out of memory");
	}
	if (n > 0)
	    --n;
	c = getc(fi);
    }
    if (c == skip && skip != EOF)
	c = getc(fi);
    *pc = c;
    s[i] = 0;
    return(s);
}

int16_t
hc_bswap16(int16_t var)
{
    return ((var & 0xff) << 8 | (var >> 8 & 0xff));
}

int32_t
hc_bswap32(int32_t var)
{
    return ((var & 0xff) << 24 | (var & 0xff00) << 8
	    | (var >> 8 & 0xff00) | (var >> 24 & 0xff));
}

int64_t
hc_bswap64(int64_t var)
{
    return (hc_bswap32(var >> 32 & 0xffffffff)
	    | (int64_t) hc_bswap32(var & 0xffffffff) << 32);
}

#ifdef DEBUG_MALLOC

#undef malloc
#undef free

struct malloc_info {
	struct malloc_info *next;
	struct malloc_info *prev;
	const char *file;
	int magic;
	int line;
};

struct malloc_info DummyInfo = { &DummyInfo, &DummyInfo, NULL, 0, 0 };
struct malloc_info *InfoList = &DummyInfo;

void *
debug_malloc(size_t bytes, const char *file, int line)
{
	struct malloc_info *info = malloc(sizeof(*info) + bytes);

	info->magic = 0x5513A4C2;
	info->file = file;
	info->line = line;

	info->next = InfoList;
	info->prev = InfoList->prev;
	info->next->prev = info;
	info->prev->next = info;
	return(info + 1);
}

void
debug_free(void *ptr)
{
	struct malloc_info *info = (struct malloc_info *)ptr - 1;
	struct malloc_info *scan;
	static int report;

	for (scan = DummyInfo.next; scan != &DummyInfo; scan = scan->next) {
		if (info == scan) {
			assert(info->magic == 0x5513A4C2);
			info->magic = 0;
			info->next->prev = info->prev;
			info->prev->next = info->next;
			free(info);
			break;
		}
	}
	if (scan == &DummyInfo)
		free(ptr);

	if ((++report & 65535) == 0) {
		printf("--- report\n");
		for (scan = DummyInfo.next; scan != &DummyInfo; scan = scan->next) {
			printf("%-15s %d\n", scan->file, scan->line);
		}
	}
}

#endif

void
fatal(const char *ctl, ...)
{
    va_list va;

    if (ctl == NULL) {
	puts("usage: cpdup [options] src dest");
	puts("\n"
	     "options:\n"
	     "    -C          request compressed ssh link if remote operation\n"
	     "    -d          print directories being traversed\n"
	     "    -f          force update even if files look the same\n"
	     "    -F<ssh_opt> add <ssh_opt> to options passed to ssh\n"
	     "    -h          show this help\n"
	     "    -H path     hardlink from path to target instead of copying\n"
	     "    -I          display performance summary\n"
	     "    -i0         do NOT confirm when removing something\n"
	     "    -j0         do not try to recreate CHR or BLK devices\n"
	     "    -l          force line-buffered stdout/stderr"
	);
#ifndef NOMD5
	puts("    -m          maintain/generate MD5 checkfile on source,\n"
	     "                and compare with (optional) destination,\n"
	     "                copying if the compare fails\n"
	     "    -M file     -m+specify MD5 checkfile, else .MD5_CHECKSUMS\n"
	     "                copy if md5 check fails"
	);
#endif
	puts("    -n          do not make any real changes to the target\n"
	     "    -o          do not remove any files, just overwrite/add\n"
	     "    -q          quiet operation\n"
	     "    -R          read-only slave mode for ssh remotes\n"
	     "                source to target, if source matches path.\n"
	     "    -S          slave mode\n"
	     "    -s0         disable safeties - allow files to overwrite directories\n"
	     "    -u          use unbuffered output for -v[vv]\n"
	     "    -v[vv]      verbose level (-vv is typical)\n"
	     "    -V          verify file contents even if they appear\n"
	     "                to be the same.\n"
	     "    -VV         same as -V but ignore mtime entirely\n"
	     "    -x          use .cpignore as exclusion file\n"
	     "    -X file     specify exclusion file (can match full source\n"
	     "                path if the exclusion file is specified via\n"
	     "                an absolute path.\n"
	     "\n"
	     "Version " VERSION " by " AUTHORS "\n"
	);
	exit(0);
    } else {
	va_start(va, ctl);
	vfprintf(stderr, ctl, va);
	va_end(va);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
    }
}
