/*
 * CPDUP.H
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#include <signal.h>
#include <pwd.h>
#include <fnmatch.h>
#include <assert.h>

/*
 * See ./mklinux script
 *
 * This is a horrible hack.  lchmod also seems to be missing
 * on the Debian system I am testing compatibility on (which will
 * break the symlink handling code), so not sure what to do about
 * that.
 *
 * XXX TODO
 */
#ifdef __linux

#define __printflike(a, b)
#define __printf0like(a, b)
#define __aligned(n)
#define __dead2
#define __unused

#define lchmod	chmod	/* horrible hack */

size_t strlcpy(char *dst, const char *src, size_t size);

#endif

#define VERSION	"1.22"
#define AUTHORS	"Matt Dillon, Dima Ruban, & Oliver Fromme"

void logstd(const char *ctl, ...) __printflike(1, 2);
void logerr(const char *ctl, ...) __printflike(1, 2);
char *mprintf(const char *ctl, ...) __printflike(1, 2);
void fatal(const char *ctl, ...) __dead2 __printf0like(1, 2);
char *fextract(FILE *fi, int n, int *pc, int skip);

int16_t hc_bswap16(int16_t var);
int32_t hc_bswap32(int32_t var);
int64_t hc_bswap64(int64_t var);

#ifndef NOMD5
int md5_check(const char *spath, const char *dpath);
void md5_flush(void);
#endif

extern const char *UseCpFile;
extern const char *MD5CacheFile;
extern const char *UseHLPath;

extern int AskConfirmation;
extern int SafetyOpt;
extern int ForceOpt;
extern int DeviceOpt;
extern int VerboseOpt;
extern int DirShowOpt;
extern int QuietOpt;
extern int NotForRealOpt;
extern int NoRemoveOpt;
extern int UseMD5Opt;
extern int SlaveOpt;
extern int SummaryOpt;
extern int CompressOpt;
extern int ReadOnlyOpt;
extern int DstRootPrivs;
extern int ValidateOpt;

extern int ssh_argc;
extern const char *ssh_argv[];

extern int64_t CountSourceBytes;
extern int64_t CountSourceItems;
extern int64_t CountCopiedItems;
extern int64_t CountSourceReadBytes;
extern int64_t CountTargetReadBytes;
extern int64_t CountWriteBytes;
extern int64_t CountRemovedItems;
extern int64_t CountLinkedItems;

#ifdef DEBUG_MALLOC
void *debug_malloc(size_t bytes, const char *file, int line);
void debug_free(void *ptr);

#define malloc(bytes)	debug_malloc(bytes, __FILE__, __LINE__)
#define free(ptr)	debug_free(ptr)
#endif
