// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c  -o $BUILD_DIR/NSCreateObjectFileImageFromMemory-basic.exe -Wno-deprecated-declarations
// BUILD:  $CC foo.c   -o $BUILD_DIR/foo.bundle -bundle

// RUN:  ./NSCreateObjectFileImageFromMemory-basic.exe $RUN_DIR/foo.bundle



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

#include "test_support.h"

static void checkBundle(const char* path, bool unlinkBeforeDestroy)
{
	int fd = open(path, O_RDONLY, 0);
	if ( fd == -1 ) {
		FAIL("open(%s) failed", path);
	}

	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1) {
		FAIL("fstat() failed");
	}

	void* loadAddress = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( loadAddress == ((void*)(-1)) ) {
		FAIL("mmap() failed");
	}

	close(fd);

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromMemory(loadAddress, stat_buf.st_size, &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromMemory failed");
	}

	NSModule mod = NSLinkModule(ofi, path, NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
	}
	
   if ( !unlinkBeforeDestroy ) {
        // API lets you destroy ofi and NSModule lives on
        if ( !NSDestroyObjectFileImage(ofi) ) {
            FAIL("NSDestroyObjectFileImage failed");
        }
    }

	NSSymbol sym = NSLookupSymbolInModule(mod, "_fooInBundle");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
	}

	void* func = NSAddressOfSymbol(sym);
	if ( func == NULL ) {
		FAIL("NSAddressOfSymbol failed");
	}

    Dl_info info;
    if ( dladdr(func, &info) == 0 ) {
        FAIL("dladdr(&p, xx) failed");
    }
    LOG("_fooInBundle found in %s", info.dli_fname);

    if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
            FAIL("NSUnLinkModule failed");
    }

    if ( dladdr(func, &info) != 0 ) {
        FAIL("dladdr(&p, xx) found but should not have");
    }

    if ( unlinkBeforeDestroy ) {
        if ( !NSDestroyObjectFileImage(ofi) ) {
            FAIL("NSDestroyObjectFileImage failed");
        }
    }
}


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    checkBundle(argv[1], true);
    checkBundle(argv[1], false);

    // Now go again enough times to flush out any limits in our dlopen encodings.
    for (unsigned i = 0; i != 255; ++i)
      checkBundle(argv[1], false);

    PASS("Success");
}

