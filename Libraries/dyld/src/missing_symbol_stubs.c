#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <mach/kern_return.h>
#include <mach-o/loader.h>
#include <uuid/uuid.h>
#include <pthread/pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

int __snprintf_chk(char * str, size_t maxlen, int flag, size_t strlen, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int result = __builtin___snprintf_chk(str, maxlen, 0, __builtin_object_size(str, 1), format, args);
    va_end(args);
    return result;
}

void* malloc(size_t size);  /* dyldNew.cpp */

int inDenyList(const char * path)
{
    (void)path;
    return 0;
}

#undef strstr
char* strstr(const char* haystack, const char* needle)
{
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            ++h;
            ++n;
        }
        if (*n == '\0') {
            return (char*)haystack;
        }
        ++haystack;
    }
    return NULL;
}

#undef strncmp
int strncmp(const char* s1, const char* s2, size_t len)
{
    size_t n = 0;
    while (n < len && *s1 && *s2) {
        if (*s1 != *s2) break;
        ++n;
        ++s1;
        ++s2;
    }
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

#undef strlcat
size_t strlcat(char* dst, const char* src, size_t len)
{
    size_t n = 0;
    while (n < len && *dst) {
        ++n;
        ++dst;
    }
    while (n < len && *src) {
        *dst = *src;
        ++n;
        ++dst;
        ++src;
    }
    if (n < len) {
        *dst = '\0';
    } else if (len > 0) {
        dst[-1] = '\0';
    }
    return n;
}

#undef strncpy
char* strncpy(char* dst, const char* src, size_t len)
{
    size_t n = 0;
    while (n < len && *src) {
        *dst = *src;
        ++n;
        ++dst;
        ++src;
    }
    return dst;
}

#undef snprintf
int snprintf(char* str, size_t size, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = __builtin___snprintf_chk(str, size, 0, __builtin_object_size(str, 1), format, args);
    va_end(args);
    return result;
}

void* dlopen(const char* path, int mode)
{
    return NULL;
}

/* absolutely in no way at all an aligned_alloc */
void* aligned_alloc(size_t alignment, size_t size)
{
        return malloc(size);
}

char* getenv(const char* name)
{
        return NULL;
}



/* c++ glue */
void __cxa_finalize(void *d)
{
    (void)d;
}

int __cxa_guard_acquire(uint64_t *guard)
{
        return 0;
}

void __cxa_guard_release(uint64_t *guard)
{
}

void __cxa_guard_abort(uint64_t *guard)
{
}



/* Stuff needed for stdio */
struct glue {
    struct glue * next;
    int           niobs;
    FILE        * iobs;
};

pthread_once_t  __sdidinit;
FILE *          __sF[3] = {NULL, NULL, NULL};
struct glue     __sglue = { NULL, 3, __sF };

void
_cleanup(void)
{
}

void
__sinit(void)
{
}

int fflush(FILE *stream)
{
        return 0;
}

/* called by libcpp_verbose_abort */
int
vfprintf(FILE* fp, const char* format, va_list ap) __printflike(2, 0)
{
	(void)fp;
        (void)format;
        (void)ap;
	return 0;
}

int
vasprintf(char** strp, const char* format, va_list ap) __printflike(2, 0)
{
        (void)strp;
        (void)format;
        (void)ap;
        return 0;
}

/* For objc-class.mm.o - mangled name for C++ linkage */
/**********************************************************************
* secure_open
* Securely open a file from a world-writable directory (like /tmp)
* If the file does not exist, it will be atomically created with mode 0600
* If the file exists, it must be, and remain after opening: 
*   1. a regular file (in particular, not a symlink)
*   2. owned by euid
*   3. permissions 0600
*   4. link count == 1
* Returns a file descriptor or -1. Errno may or may not be set on error.
**********************************************************************/
enum { NO = 0, YES = 1 };
int _Z11secure_openPKcij(const char *filename, int flags, uid_t euid)
{
    struct stat fs, ls;
    int fd = -1;
    boolean_t truncate = NO;
    boolean_t create = NO;

    if (flags & O_TRUNC) {
        // Don't truncate the file until after it is open and verified.
        truncate = YES;
        flags &= ~O_TRUNC;
    }
    if (flags & O_CREAT) {
        // Don't create except when we're ready for it
        create = YES;
        flags &= ~O_CREAT;
        flags &= ~O_EXCL;
    }

    if (lstat(filename, &ls) < 0) {
        if (errno == ENOENT  &&  create) {
            // No such file - create it
            fd = open(filename, flags | O_CREAT | O_EXCL, 0600);
            if (fd >= 0) {
                // File was created successfully.
                // New file does not need to be truncated.
                return fd;
            } else {
                // File creation failed.
                return -1;
            }
        } else {
            // lstat failed, or user doesn't want to create the file
            return -1;
        }
    } else {
        // lstat succeeded - verify attributes and open
        if (S_ISREG(ls.st_mode)  &&  // regular file?
            ls.st_nlink == 1  &&     // link count == 1?
            ls.st_uid == euid  &&    // owned by euid?
            (ls.st_mode & ALLPERMS) == (S_IRUSR | S_IWUSR))  // mode 0600?
        {
            // Attributes look ok - open it and check attributes again
            fd = open(filename, flags, 0000);
            if (fd >= 0) {
                // File is open - double-check attributes
                if (0 == fstat(fd, &fs)  &&  
                    fs.st_nlink == ls.st_nlink  &&  // link count == 1?
                    fs.st_uid == ls.st_uid  &&      // owned by euid?
                    fs.st_mode == ls.st_mode  &&    // regular file, 0600?
                    fs.st_ino == ls.st_ino  &&      // same inode as before?
                    fs.st_dev == ls.st_dev)         // same device as before?
                {
                    // File is open and OK
                    if (truncate) ftruncate(fd, 0);
                    return fd;
                } else {
                    // Opened file looks funny - close it
                    close(fd);
                    return -1;
                }
            } else {
                // File didn't open
                return -1;
            }
        } else {
            // Unopened file looks funny - don't open it
            return -1;
        }
    }
}

/* FIXME: get this from plist or makelist */
double       dyldVersionNumber = 732.2;
const char * dyldVersionString = "ravynOS dyld 732.2";

/* Stub to provide NSRange ABI from Foundation */
typedef struct {
    unsigned long location;
    unsigned long length;
} NSRange;

NSRange NSMakeRange(unsigned long location, unsigned long length)
{
    NSRange result = { location, length };
    return result;
}

void uuid_unparse(const uuid_t uu, uuid_string_t out)
{
    uuid_unparse_upper(uu, out);
}

/* FIXME: remove once we have amfi */
int amfi_check_dyld_policy_self(uint64_t inFlags, uint64_t* outFlags)
{
    *outFlags = 0x3F;  // on old kernel, simulator process get all flags
    return 0;
}

int dyld_shared_cache_extract_dylibs_progress(const char* shared_cache_file_path,
    const char* extraction_root_path,
    void (^progress)(unsigned current, unsigned total))
{
    (void)shared_cache_file_path;
    (void)extraction_root_path;
    (void)progress;
    return KERN_NOT_SUPPORTED;
}

/* FIXME: need libmacho */
uint8_t* getsectiondata(const struct mach_header_64* mhp,
                        const char* segname,
                        const char* sectname,
                        unsigned long* size)
{
    (void)mhp;
    (void)segname;
    (void)sectname;
    if ( size != NULL )
        *size = 0;
    return NULL;
}

uint8_t * getsegmentdata(const struct mach_header_64* mhp,
                         const char* segname,
                         unsigned long* size)
{
    (void)mhp;
    (void)segname;
    if ( size != NULL )
        *size = 0;
    return NULL;
}

/* DNSService stubs to be removed when mDNSResponder is built */
typedef struct _DNSServiceRef_t *DNSServiceRef;
typedef unsigned int DNSServiceFlags;
typedef int DNSServiceErrorType;
typedef void (*DNSServiceQueryRecordReply)(DNSServiceRef sdRef,
    DNSServiceFlags flags, unsigned int interfaceIndex,
    DNSServiceErrorType errorCode, const char *fullname,
    unsigned short rrtype, unsigned short rrclass,
    unsigned short rdlen, const void *rdata,
    unsigned int ttl, void *context);
#define kDNSServiceErr_Unsupported -65563
DNSServiceErrorType DNSServiceCreateConnection(DNSServiceRef *sdRef)
{
    (void)sdRef;
    return kDNSServiceErr_Unsupported;
}
DNSServiceErrorType DNSServiceProcessResult(DNSServiceRef sdRef)
{
    (void)sdRef;
    return kDNSServiceErr_Unsupported;
}
DNSServiceErrorType DNSServiceQueryRecord(
    DNSServiceRef              *sdRef,
    DNSServiceFlags             flags,
    unsigned int                interfaceIndex,
    const char                 *name,
    unsigned short              rrtype,
    unsigned short              rrclass,
    DNSServiceQueryRecordReply  callBack,
    void                       *context)
{
    (void)sdRef;
    (void)flags;
    (void)interfaceIndex;
    (void)name;
    (void)rrtype;
    (void)rrclass;
    (void)callBack;
    (void)context;
    return kDNSServiceErr_Unsupported;
}
void DNSServiceRefDeallocate(DNSServiceRef sdRef)
{
    (void)sdRef;
}
int DNSServiceRefSockFD(DNSServiceRef sdRef)
{
    (void)sdRef;
    return -1;
}
