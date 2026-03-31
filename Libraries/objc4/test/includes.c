// TEST_CONFIG

// Verify that all headers can be included in any language.
// See also test/include-warnings.c which checks for warnings in these headers.
// See also test/includes-objc2.c which checks for safety even if 
// the client is C code that defined __OBJC2__.

#ifndef NAME
#define NAME "includes.c"
#endif

#include <objc/objc.h>

#include <objc/List.h>
#include <objc/NSObjCRuntime.h>
#include <objc/NSObject.h>
#include <objc/Object.h>
#include <objc/Protocol.h>
#include <objc/message.h>
#include <objc/objc-api.h>
#include <objc/objc-auto.h>
#include <objc/objc-class.h>
#include <objc/objc-exception.h>
#include <objc/objc-load.h>
#include <objc/objc-runtime.h>
#include <objc/objc-sync.h>
#include <objc/runtime.h>

#include <objc/objc-abi.h>
#include <objc/objc-gdb.h>
#include <objc/objc-internal.h>

#if TARGET_OS_OSX
#include <objc/hashtable.h>
#include <objc/hashtable2.h>
#include <objc/maptable.h>
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "test.h"
#pragma clang diagnostic pop

int main()
{
    succeed(NAME);
}
