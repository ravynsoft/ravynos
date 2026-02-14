//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include <dispatch/dispatch.h>
#include <stdio.h>

#define DISPATCH_RUNTIME_STDLIB_INTERFACE __attribute__((__visibility__("default")))

#if USE_OBJC
@protocol OS_dispatch_source;
@protocol OS_dispatch_source_mach_send;
@protocol OS_dispatch_source_mach_recv;
@protocol OS_dispatch_source_memorypressure;
@protocol OS_dispatch_source_proc;
@protocol OS_dispatch_source_read;
@protocol OS_dispatch_source_signal;
@protocol OS_dispatch_source_timer;
@protocol OS_dispatch_source_data_add;
@protocol OS_dispatch_source_data_or;
@protocol OS_dispatch_source_data_replace;
@protocol OS_dispatch_source_vnode;
@protocol OS_dispatch_source_write;

// #include <dispatch/private.h>
__attribute__((constructor))
static void _dispatch_overlay_constructor() {
  Class source = objc_lookUpClass("OS_dispatch_source");
  if (source) {
    class_addProtocol(source, @protocol(OS_dispatch_source));
    class_addProtocol(source, @protocol(OS_dispatch_source_mach_send));
    class_addProtocol(source, @protocol(OS_dispatch_source_mach_recv));
    class_addProtocol(source, @protocol(OS_dispatch_source_memorypressure));
    class_addProtocol(source, @protocol(OS_dispatch_source_proc));
    class_addProtocol(source, @protocol(OS_dispatch_source_read));
    class_addProtocol(source, @protocol(OS_dispatch_source_signal));
    class_addProtocol(source, @protocol(OS_dispatch_source_timer));
    class_addProtocol(source, @protocol(OS_dispatch_source_data_add));
    class_addProtocol(source, @protocol(OS_dispatch_source_data_or));
    class_addProtocol(source, @protocol(OS_dispatch_source_data_replace));
    class_addProtocol(source, @protocol(OS_dispatch_source_vnode));
    class_addProtocol(source, @protocol(OS_dispatch_source_write));
  }
}

#endif /* USE_OBJC */

#if !USE_OBJC
extern "C" void * objc_retainAutoreleasedReturnValue(void *obj);
#endif

#if !USE_OBJC

// For CF functions with 'Get' semantics, the compiler currently assumes that
// the result is autoreleased and must be retained. It does so on all platforms
// by emitting a call to objc_retainAutoreleasedReturnValue. On Darwin, this is
// implemented by the ObjC runtime. On non-ObjC platforms, there is no runtime,
// and therefore we have to stub it out here ourselves. The compiler will
// eventually call swift_release to balance the retain below. This is a
// workaround until the compiler no longer emits this callout on non-ObjC
// platforms.
extern "C" void swift_retain(void *);

DISPATCH_RUNTIME_STDLIB_INTERFACE
extern "C" void * objc_retainAutoreleasedReturnValue(void *obj) {
    if (obj) {
        swift_retain(obj);
        return obj;
    }
    else return NULL;
}

#endif // !USE_OBJC
