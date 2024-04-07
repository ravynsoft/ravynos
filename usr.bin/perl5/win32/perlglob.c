/*
 * Globbing for NT.  Relies on the expansion done by the library
 * startup code (provided by Visual C++ by linking in setargv.obj).
 */

/* Enable wildcard expansion for gcc's C-runtime library if not enabled by
 * default (currently necessary with the automated build of the mingw-w64
 * cross-compiler, but there's no harm in making sure for others too). */
#ifdef __MINGW32__
#include <_mingw.h>
#if defined(__MINGW64_VERSION_MAJOR) && defined(__MINGW64_VERSION_MINOR)
    // MinGW-w64
    int _dowildcard = -1;
#else
    // MinGW
    int _CRT_glob = -1;
#endif
#endif

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <windows.h>

int
main(int argc, char *argv[])
{
    int i;
    size_t len;
    char root[MAX_PATH];
    char *dummy;
    char volname[MAX_PATH];
    DWORD serial, maxname, flags;
    BOOL downcase = TRUE;
    int fd;

    /* check out the file system characteristics */
    if (GetFullPathName(".", MAX_PATH, root, &dummy)) {
        dummy = strchr(root,'\\'); 
        if (dummy)
            *++dummy = '\0';
        if (GetVolumeInformation(root, volname, MAX_PATH, 
                                 &serial, &maxname, &flags, 0, 0)) {
            downcase = !(flags & FS_CASE_IS_PRESERVED);
        }
    }

    fd = fileno(stdout);
    /* rare VC linker bug causes uninit global FILE *s
       fileno() implementation in VC 2003 is 2 blind pointer derefs so it will
       never return -1 error as POSIX says, to be compliant fail for -1 and
       for absurdly high FDs which are actually pointers */
    assert(fd >= 0 && fd < SHRT_MAX);
    setmode(fd, O_BINARY);
    for (i = 1; i < argc; i++) {
        len = strlen(argv[i]);
        if (downcase)
            strlwr(argv[i]);
        if (i > 1) fwrite("\0", sizeof(char), 1, stdout);
        fwrite(argv[i], sizeof(char), len, stdout);
    }
    return 0;
}

