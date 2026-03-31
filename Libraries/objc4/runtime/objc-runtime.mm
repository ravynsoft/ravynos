/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/***********************************************************************
* objc-runtime.m
* Copyright 1988-1996, NeXT Software, Inc.
* Author:	s. naroff
*
**********************************************************************/



/***********************************************************************
* Imports.
**********************************************************************/

#include "objc-private.h"
#include "objc-loadmethod.h"
#include "objc-file.h"
#include "message.h"

/***********************************************************************
* Exports.
**********************************************************************/

/* Linker metadata symbols */

// NSObject was in Foundation/CF on macOS < 10.8.
#if TARGET_OS_OSX
#if __OBJC2__

const char __objc_nsobject_class_10_5 = 0;
const char __objc_nsobject_class_10_6 = 0;
const char __objc_nsobject_class_10_7 = 0;

const char __objc_nsobject_metaclass_10_5 = 0;
const char __objc_nsobject_metaclass_10_6 = 0;
const char __objc_nsobject_metaclass_10_7 = 0;

const char __objc_nsobject_isa_10_5 = 0;
const char __objc_nsobject_isa_10_6 = 0;
const char __objc_nsobject_isa_10_7 = 0;

#else

const char __objc_nsobject_class_10_5 = 0;
const char __objc_nsobject_class_10_6 = 0;
const char __objc_nsobject_class_10_7 = 0;

#endif
#endif

// Settings from environment variables
#define OPTION(var, env, help) bool var = false;
#include "objc-env.h"
#undef OPTION

struct option_t {
    bool* var;
    const char *env;
    const char *help;
    size_t envlen;
};

const option_t Settings[] = {
#define OPTION(var, env, help) option_t{&var, #env, help, strlen(#env)}, 
#include "objc-env.h"
#undef OPTION
};


// objc's key for pthread_getspecific
#if SUPPORT_DIRECT_THREAD_KEYS
#define _objc_pthread_key TLS_DIRECT_KEY
#else
static tls_key_t _objc_pthread_key;
#endif

// Selectors
SEL SEL_cxx_construct = NULL;
SEL SEL_cxx_destruct = NULL;

struct objc::SafeRanges objc::dataSegmentsRanges;
header_info *FirstHeader = 0;  // NULL means empty list
header_info *LastHeader  = 0;  // NULL means invalid; recompute it

// Set to true on the child side of fork() 
// if the parent process was multithreaded when fork() was called.
bool MultithreadedForkChild = false;


/***********************************************************************
* objc_noop_imp. Used when we need to install a do-nothing method somewhere.
**********************************************************************/
id objc_noop_imp(id self, SEL _cmd __unused) {
    return self;
}


/***********************************************************************
* _objc_isDebugBuild. Defined in debug builds only.
* Some test code looks for the presence of this symbol.
**********************************************************************/
#if DEBUG != OBJC_IS_DEBUG_BUILD
#error mismatch in debug-ness macros
// DEBUG is used in our code. OBJC_IS_DEBUG_BUILD is used in the
// header declaration of _objc_isDebugBuild() because that header
// is visible to other clients who might have their own DEBUG macro.
#endif

#if OBJC_IS_DEBUG_BUILD
void _objc_isDebugBuild(void) { }
#endif


/***********************************************************************
* objc_getClass.  Return the id of the named class.  If the class does
* not exist, call _objc_classLoader and then objc_classHandler, either of 
* which may create a new class.
* Warning: doesn't work if aClassName is the name of a posed-for class's isa!
**********************************************************************/
Class objc_getClass(const char *aClassName)
{
    if (!aClassName) return Nil;

    // NO unconnected, YES class handler
    return look_up_class(aClassName, NO, YES);
}


/***********************************************************************
* objc_getRequiredClass.  
* Same as objc_getClass, but kills the process if the class is not found. 
* This is used by ZeroLink, where failing to find a class would be a 
* compile-time link error without ZeroLink.
**********************************************************************/
Class objc_getRequiredClass(const char *aClassName)
{
    Class cls = objc_getClass(aClassName);
    if (!cls) _objc_fatal("link error: class '%s' not found.", aClassName);
    return cls;
}


/***********************************************************************
* objc_lookUpClass.  Return the id of the named class.
* If the class does not exist, call _objc_classLoader, which may create 
* a new class.
*
* Formerly objc_getClassWithoutWarning ()
**********************************************************************/
Class objc_lookUpClass(const char *aClassName)
{
    if (!aClassName) return Nil;

    // NO unconnected, NO class handler
    return look_up_class(aClassName, NO, NO);
}


/***********************************************************************
* objc_getMetaClass.  Return the id of the meta class the named class.
* Warning: doesn't work if aClassName is the name of a posed-for class's isa!
**********************************************************************/
Class objc_getMetaClass(const char *aClassName)
{
    Class cls;

    if (!aClassName) return Nil;

    cls = objc_getClass (aClassName);
    if (!cls)
    {
        _objc_inform ("class `%s' not linked into application", aClassName);
        return Nil;
    }

    return cls->ISA();
}

/***********************************************************************
 * objc::SafeRanges::find.  Find an image data segment that contains address
 **********************************************************************/
bool
objc::SafeRanges::find(uintptr_t ptr, uint32_t &pos)
{
    if (!sorted) {
        std::sort(ranges, ranges + count, [](const Range &s1, const Range &s2){
            return s1.start < s2.start;
        });
        sorted = true;
    }

    uint32_t l = 0, r = count;
    while (l < r) {
        uint32_t i = (l + r) / 2;

        if (ptr < ranges[i].start) {
            r = i;
        } else if (ptr >= ranges[i].end) {
            l = i + 1;
        } else {
            pos = i;
            return true;
        }
    }

    pos = UINT32_MAX;
    return false;
}

/***********************************************************************
 * objc::SafeRanges::add.  Register a new well known data segment.
 **********************************************************************/
void
objc::SafeRanges::add(uintptr_t start, uintptr_t end)
{
    if (count == size) {
        // Have a typical malloc growth:
        // - size <= 32:  grow by  4
        // - size <= 64:  grow by  8
        // - size <= 128: grow by 16
        // ... etc
        size += size < 16 ? 4 : 1 << (fls(size) - 3);
        ranges = (Range *)realloc(ranges, sizeof(Range) * size);
    }
    ranges[count++] = Range{ start, end };
    sorted = false;
}

/***********************************************************************
 * objc::SafeRanges::remove.  Remove a previously known data segment.
 **********************************************************************/
void
objc::SafeRanges::remove(uintptr_t start, uintptr_t end)
{
    uint32_t pos;

    if (!find(start, pos) || ranges[pos].end != end) {
        _objc_fatal("Cannot find range %#lx..%#lx", start, end);
    }
    if (pos < --count) {
        ranges[pos] = ranges[count];
        sorted = false;
    }
}

/***********************************************************************
* appendHeader.  Add a newly-constructed header_info to the list. 
**********************************************************************/
void appendHeader(header_info *hi)
{
    // Add the header to the header list. 
    // The header is appended to the list, to preserve the bottom-up order.
    hi->setNext(NULL);
    if (!FirstHeader) {
        // list is empty
        FirstHeader = LastHeader = hi;
    } else {
        if (!LastHeader) {
            // list is not empty, but LastHeader is invalid - recompute it
            LastHeader = FirstHeader;
            while (LastHeader->getNext()) LastHeader = LastHeader->getNext();
        }
        // LastHeader is now valid
        LastHeader->setNext(hi);
        LastHeader = hi;
    }

#if __OBJC2__
    if ((hi->mhdr()->flags & MH_DYLIB_IN_CACHE) == 0) {
        foreach_data_segment(hi->mhdr(), [](const segmentType *seg, intptr_t slide) {
            uintptr_t start = (uintptr_t)seg->vmaddr + slide;
            objc::dataSegmentsRanges.add(start, start + seg->vmsize);
        });
    }
#endif
}


/***********************************************************************
* removeHeader
* Remove the given header from the header list.
* FirstHeader is updated. 
* LastHeader is set to NULL. Any code that uses LastHeader must 
* detect this NULL and recompute LastHeader by traversing the list.
**********************************************************************/
void removeHeader(header_info *hi)
{
    header_info *prev = NULL;
    header_info *current = NULL;

    for (current = FirstHeader; current != NULL; current = current->getNext()) {
        if (current == hi) {
            header_info *deadHead = current;

            // Remove from the linked list.
            if (prev)
                prev->setNext(current->getNext());
            else
                FirstHeader = current->getNext(); // no prev so removing head
            
            // Update LastHeader if necessary.
            if (LastHeader == deadHead) {
                LastHeader = NULL;  // will be recomputed next time it's used
            }
            break;
        }
        prev = current;
    }

#if __OBJC2__
    if ((hi->mhdr()->flags & MH_DYLIB_IN_CACHE) == 0) {
        foreach_data_segment(hi->mhdr(), [](const segmentType *seg, intptr_t slide) {
            uintptr_t start = (uintptr_t)seg->vmaddr + slide;
            objc::dataSegmentsRanges.remove(start, start + seg->vmsize);
        });
    }
#endif
}


/***********************************************************************
* environ_init
* Read environment variables that affect the runtime.
* Also print environment variable help, if requested.
**********************************************************************/
void environ_init(void) 
{
    if (issetugid()) {
        // All environment variables are silently ignored when setuid or setgid
        // This includes OBJC_HELP and OBJC_PRINT_OPTIONS themselves.
        return;
    } 

    bool PrintHelp = false;
    bool PrintOptions = false;
    bool maybeMallocDebugging = false;

    // Scan environ[] directly instead of calling getenv() a lot.
    // This optimizes the case where none are set.
    for (char **p = *_NSGetEnviron(); *p != nil; p++) {
        if (0 == strncmp(*p, "Malloc", 6)  ||  0 == strncmp(*p, "DYLD", 4)  ||  
            0 == strncmp(*p, "NSZombiesEnabled", 16))
        {
            maybeMallocDebugging = true;
        }

        if (0 != strncmp(*p, "OBJC_", 5)) continue;
        
        if (0 == strncmp(*p, "OBJC_HELP=", 10)) {
            PrintHelp = true;
            continue;
        }
        if (0 == strncmp(*p, "OBJC_PRINT_OPTIONS=", 19)) {
            PrintOptions = true;
            continue;
        }
        
        const char *value = strchr(*p, '=');
        if (!*value) continue;
        value++;
        
        for (size_t i = 0; i < sizeof(Settings)/sizeof(Settings[0]); i++) {
            const option_t *opt = &Settings[i];
            if ((size_t)(value - *p) == 1+opt->envlen  &&  
                0 == strncmp(*p, opt->env, opt->envlen))
            {
                *opt->var = (0 == strcmp(value, "YES"));
                break;
            }
        }            
    }

    // Special case: enable some autorelease pool debugging 
    // when some malloc debugging is enabled 
    // and OBJC_DEBUG_POOL_ALLOCATION is not set to something other than NO.
    if (maybeMallocDebugging) {
        const char *insert = getenv("DYLD_INSERT_LIBRARIES");
        const char *zombie = getenv("NSZombiesEnabled");
        const char *pooldebug = getenv("OBJC_DEBUG_POOL_ALLOCATION");
        if ((getenv("MallocStackLogging")
             || getenv("MallocStackLoggingNoCompact")
             || (zombie && (*zombie == 'Y' || *zombie == 'y'))
             || (insert && strstr(insert, "libgmalloc")))
            &&
            (!pooldebug || 0 == strcmp(pooldebug, "YES")))
        {
            DebugPoolAllocation = true;
        }
    }

    // Print OBJC_HELP and OBJC_PRINT_OPTIONS output.
    if (PrintHelp  ||  PrintOptions) {
        if (PrintHelp) {
            _objc_inform("Objective-C runtime debugging. Set variable=YES to enable.");
            _objc_inform("OBJC_HELP: describe available environment variables");
            if (PrintOptions) {
                _objc_inform("OBJC_HELP is set");
            }
            _objc_inform("OBJC_PRINT_OPTIONS: list which options are set");
        }
        if (PrintOptions) {
            _objc_inform("OBJC_PRINT_OPTIONS is set");
        }

        for (size_t i = 0; i < sizeof(Settings)/sizeof(Settings[0]); i++) {
            const option_t *opt = &Settings[i];            
            if (PrintHelp) _objc_inform("%s: %s", opt->env, opt->help);
            if (PrintOptions && *opt->var) _objc_inform("%s is set", opt->env);
        }
    }
}


/***********************************************************************
* logReplacedMethod
* OBJC_PRINT_REPLACED_METHODS implementation
**********************************************************************/
void 
logReplacedMethod(const char *className, SEL s, 
                  bool isMeta, const char *catName, 
                  IMP oldImp, IMP newImp)
{
    const char *oldImage = "??";
    const char *newImage = "??";

    // Silently ignore +load replacement because category +load is special
    if (s == @selector(load)) return;

#if TARGET_OS_WIN32
    // don't know dladdr()/dli_fname equivalent
#else
    Dl_info dl;

    if (dladdr((void*)oldImp, &dl)  &&  dl.dli_fname) oldImage = dl.dli_fname;
    if (dladdr((void*)newImp, &dl)  &&  dl.dli_fname) newImage = dl.dli_fname;
#endif
    
    _objc_inform("REPLACED: %c[%s %s]  %s%s  (IMP was %p (%s), now %p (%s))",
                 isMeta ? '+' : '-', className, sel_getName(s), 
                 catName ? "by category " : "", catName ? catName : "", 
                 oldImp, oldImage, newImp, newImage);
}


/***********************************************************************
* _objc_fetch_pthread_data
* Fetch objc's pthread data for this thread.
* If the data doesn't exist yet and create is NO, return NULL.
* If the data doesn't exist yet and create is YES, allocate and return it.
**********************************************************************/
_objc_pthread_data *_objc_fetch_pthread_data(bool create)
{
    _objc_pthread_data *data;

    data = (_objc_pthread_data *)tls_get(_objc_pthread_key);
    if (!data  &&  create) {
        data = (_objc_pthread_data *)
            calloc(1, sizeof(_objc_pthread_data));
        tls_set(_objc_pthread_key, data);
    }

    return data;
}


/***********************************************************************
* _objc_pthread_destroyspecific
* Destructor for objc's per-thread data.
* arg shouldn't be NULL, but we check anyway.
**********************************************************************/
extern void _destroyInitializingClassList(struct _objc_initializing_classes *list);
void _objc_pthread_destroyspecific(void *arg)
{
    _objc_pthread_data *data = (_objc_pthread_data *)arg;
    if (data != NULL) {
        _destroyInitializingClassList(data->initializingClasses);
        _destroySyncCache(data->syncCache);
        _destroyAltHandlerList(data->handlerList);
        for (int i = 0; i < (int)countof(data->printableNames); i++) {
            if (data->printableNames[i]) {
                free(data->printableNames[i]);  
            }
        }
        free(data->classNameLookups);

        // add further cleanup here...

        free(data);
    }
}


void tls_init(void)
{
#if SUPPORT_DIRECT_THREAD_KEYS
    pthread_key_init_np(TLS_DIRECT_KEY, &_objc_pthread_destroyspecific);
#else
    _objc_pthread_key = tls_create(&_objc_pthread_destroyspecific);
#endif
}


/***********************************************************************
* _objcInit
* Former library initializer. This function is now merely a placeholder 
* for external callers. All runtime initialization has now been moved 
* to map_images() and _objc_init.
**********************************************************************/
void _objcInit(void)
{
    // do nothing
}


/***********************************************************************
* objc_setForwardHandler
**********************************************************************/

#if !__OBJC2__

// Default forward handler (nil) goes to forward:: dispatch.
void *_objc_forward_handler = nil;
void *_objc_forward_stret_handler = nil;

#else

// Default forward handler halts the process.
__attribute__((noreturn, cold)) void
objc_defaultForwardHandler(id self, SEL sel)
{
    _objc_fatal("%c[%s %s]: unrecognized selector sent to instance %p "
                "(no message forward handler is installed)", 
                class_isMetaClass(object_getClass(self)) ? '+' : '-', 
                object_getClassName(self), sel_getName(sel), self);
}
void *_objc_forward_handler = (void*)objc_defaultForwardHandler;

#if SUPPORT_STRET
struct stret { int i[100]; };
__attribute__((noreturn, cold)) struct stret
objc_defaultForwardStretHandler(id self, SEL sel)
{
    objc_defaultForwardHandler(self, sel);
}
void *_objc_forward_stret_handler = (void*)objc_defaultForwardStretHandler;
#endif

#endif

void objc_setForwardHandler(void *fwd, void *fwd_stret)
{
    _objc_forward_handler = fwd;
#if SUPPORT_STRET
    _objc_forward_stret_handler = fwd_stret;
#endif
}


#if !__OBJC2__
// GrP fixme
extern "C" Class _objc_getOrigClass(const char *name);
#endif

static BOOL internal_class_getImageName(Class cls, const char **outName)
{
#if !__OBJC2__
    cls = _objc_getOrigClass(cls->demangledName());
#endif
    auto result = dyld_image_path_containing_address(cls);
    *outName = result;
    return (result != nil);
}


static ChainedHookFunction<objc_hook_getImageName>
GetImageNameHook{internal_class_getImageName};

void objc_setHook_getImageName(objc_hook_getImageName newValue,
                               objc_hook_getImageName *outOldValue)
{
    GetImageNameHook.set(newValue, outOldValue);
}

const char *class_getImageName(Class cls)
{
    if (!cls) return nil;

    const char *name;
    if (GetImageNameHook.get()(cls, &name)) return name;
    else return nil;
}


/**********************************************************************
* Fast Enumeration Support
**********************************************************************/

static void (*enumerationMutationHandler)(id);

/**********************************************************************
* objc_enumerationMutation
* called by compiler when a mutation is detected during foreach iteration
**********************************************************************/
void objc_enumerationMutation(id object) {
    if (enumerationMutationHandler == nil) {
        _objc_fatal("mutation detected during 'for(... in ...)'  enumeration of object %p.", (void*)object);
    }
    (*enumerationMutationHandler)(object);
}


/**********************************************************************
* objc_setEnumerationMutationHandler
* an entry point to customize mutation error handing
**********************************************************************/
void objc_setEnumerationMutationHandler(void (*handler)(id)) {
    enumerationMutationHandler = handler;
}


/**********************************************************************
* Associative Reference Support
**********************************************************************/

id
objc_getAssociatedObject(id object, const void *key)
{
    return _object_get_associative_reference(object, key);
}

static void
_base_objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy)
{
  _object_set_associative_reference(object, key, value, policy);
}

static ChainedHookFunction<objc_hook_setAssociatedObject> SetAssocHook{_base_objc_setAssociatedObject};

void
objc_setHook_setAssociatedObject(objc_hook_setAssociatedObject _Nonnull newValue,
                                 objc_hook_setAssociatedObject _Nullable * _Nonnull outOldValue) {
    SetAssocHook.set(newValue, outOldValue);
}

void
objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy)
{
    SetAssocHook.get()(object, key, value, policy);
}


void objc_removeAssociatedObjects(id object) 
{
    if (object && object->hasAssociatedObjects()) {
        _object_remove_assocations(object);
    }
}



#if SUPPORT_GC_COMPAT

#include <mach-o/fat.h>

// GC preflight for an app executable.

enum GCness {
    WithGC = 1, 
    WithoutGC = 0, 
    Error = -1
};

// Overloaded template wrappers around clang's overflow-checked arithmetic.

template <typename T> bool uadd_overflow(T x, T y, T* sum);
template <typename T> bool usub_overflow(T x, T y, T* diff);
template <typename T> bool umul_overflow(T x, T y, T* prod);

template <typename T> bool sadd_overflow(T x, T y, T* sum);
template <typename T> bool ssub_overflow(T x, T y, T* diff);
template <typename T> bool smul_overflow(T x, T y, T* prod);

template <> bool uadd_overflow(unsigned x, unsigned y, unsigned* sum) { return __builtin_uadd_overflow(x, y, sum); }
template <> bool uadd_overflow(unsigned long x, unsigned long y, unsigned long* sum) { return __builtin_uaddl_overflow(x, y, sum); }
template <> bool uadd_overflow(unsigned long long x, unsigned long long y, unsigned long long* sum) { return __builtin_uaddll_overflow(x, y, sum); }

template <> bool usub_overflow(unsigned x, unsigned y, unsigned* diff) { return __builtin_usub_overflow(x, y, diff); }
template <> bool usub_overflow(unsigned long x, unsigned long y, unsigned long* diff) { return __builtin_usubl_overflow(x, y, diff); }
template <> bool usub_overflow(unsigned long long x, unsigned long long y, unsigned long long* diff) { return __builtin_usubll_overflow(x, y, diff); }

template <> bool umul_overflow(unsigned x, unsigned y, unsigned* prod) { return __builtin_umul_overflow(x, y, prod); }
template <> bool umul_overflow(unsigned long x, unsigned long y, unsigned long* prod) { return __builtin_umull_overflow(x, y, prod); }
template <> bool umul_overflow(unsigned long long x, unsigned long long y, unsigned long long* prod) { return __builtin_umulll_overflow(x, y, prod); }

template <> bool sadd_overflow(signed x, signed y, signed* sum) { return __builtin_sadd_overflow(x, y, sum); }
template <> bool sadd_overflow(signed long x, signed long y, signed long* sum) { return __builtin_saddl_overflow(x, y, sum); }
template <> bool sadd_overflow(signed long long x, signed long long y, signed long long* sum) { return __builtin_saddll_overflow(x, y, sum); }

template <> bool ssub_overflow(signed x, signed y, signed* diff) { return __builtin_ssub_overflow(x, y, diff); }
template <> bool ssub_overflow(signed long x, signed long y, signed long* diff) { return __builtin_ssubl_overflow(x, y, diff); }
template <> bool ssub_overflow(signed long long x, signed long long y, signed long long* diff) { return __builtin_ssubll_overflow(x, y, diff); }

template <> bool smul_overflow(signed x, signed y, signed* prod) { return __builtin_smul_overflow(x, y, prod); }
template <> bool smul_overflow(signed long x, signed long y, signed long* prod) { return __builtin_smull_overflow(x, y, prod); }
template <> bool smul_overflow(signed long long x, signed long long y, signed long long* prod) { return __builtin_smulll_overflow(x, y, prod); }


// Range-checking subview of a file.
class FileSlice {
    int fd;
    uint64_t sliceOffset;
    uint64_t sliceSize;

public:
    FileSlice() : fd(-1), sliceOffset(0), sliceSize(0) { }

    FileSlice(int newfd, uint64_t newOffset, uint64_t newSize) 
        : fd(newfd) , sliceOffset(newOffset) , sliceSize(newSize) { }

    // Read bytes from this slice. 
    // Returns YES if all bytes were read successfully.
    bool pread(void *buf, uint64_t readSize, uint64_t readOffset = 0) {
        uint64_t readEnd;
        if (uadd_overflow(readOffset, readSize, &readEnd)) return NO;
        if (readEnd > sliceSize) return NO;

        uint64_t preadOffset;
        if (uadd_overflow(sliceOffset, readOffset, &preadOffset)) return NO;

        int64_t readed = ::pread(fd, buf, (size_t)readSize, preadOffset);
        if (readed < 0  ||  (uint64_t)readed != readSize) return NO;
        return YES;
    }

    // Create a new slice that is a subset of this slice.
    // Returnes YES if successful.
    bool slice(uint64_t newOffset, uint64_t newSize, FileSlice& result) {
        // fixme arithmetic overflow
        uint64_t newEnd;
        if (uadd_overflow(newOffset, newSize, &newEnd)) return NO;
        if (newEnd > sliceSize) return NO;

        if (uadd_overflow(sliceOffset, newOffset, &result.sliceOffset)) {
            return NO;
        }
        result.sliceSize = newSize;
        result.fd = fd;
        return YES;
    }

    // Shorten this slice in place by removing a range from the start.
    bool advance(uint64_t distance) {
        if (distance > sliceSize) return NO;
        if (uadd_overflow(sliceOffset, distance, &sliceOffset)) return NO;
        if (usub_overflow(sliceSize, distance, &sliceSize)) return NO;
        return YES;
    }
};


// Arch32 and Arch64 are used to specialize sliceRequiresGC()
// to interrogate old-ABI i386 and new-ABI x86_64 files.

struct Arch32 {
    using mh_t = struct mach_header;
    using segment_command_t = struct segment_command;
    using section_t = struct section;

    enum : cpu_type_t { cputype = CPU_TYPE_X86 };
    enum : int { segment_cmd = LC_SEGMENT };

    static bool isObjCSegment(const char *segname) {
        return segnameEquals(segname, "__OBJC");
    }

    static bool isImageInfoSection(const char *sectname) {
        return sectnameEquals(sectname, "__image_info");
    }

    static bool countClasses(FileSlice file, section_t& sect, 
                             int& classCount, int& classrefCount)
    {
        if (sectnameEquals(sect.sectname, "__cls_refs")) {
            classrefCount += sect.size / 4;
        }
        else if (sectnameEquals(sect.sectname, "__module_info")) {
            struct module_t {
                uint32_t version;
                uint32_t size;
                uint32_t name;    // not bound
                uint32_t symtab;  // not bound
            };
            size_t mod_count = sect.size / sizeof(module_t);
            if (mod_count == 0) {
                // no classes defined
            } else if (mod_count > 1) {
                // AppleScriptObjC apps only have one module.
                // Disqualify this app by setting classCount to non-zero.
                // We don't actually need an accurate count.
                classCount = 1;
            } else if (mod_count == 1) {
                FileSlice moduleSlice;
                if (!file.slice(sect.offset, sect.size, moduleSlice)) return NO;
                module_t module;
                if (!moduleSlice.pread(&module, sizeof(module))) return NO;
                if (module.symtab) {
                    // AppleScriptObjC apps only have a module with no symtab.
                    // Disqualify this app by setting classCount to non-zero.
                    // We don't actually need an accurate count.
                    classCount = 1;
                }
            }
            
        }
        return YES;
    }

};

struct Arch64 {
    using mh_t = struct mach_header_64;
    using segment_command_t = struct segment_command_64;
    using section_t = struct section_64;

    enum : cpu_type_t { cputype = CPU_TYPE_X86_64 };
    enum : int { segment_cmd = LC_SEGMENT_64 };

    static bool isObjCSegment(const char *segname) {
        return 
            segnameEquals(segname, "__DATA")  ||  
            segnameEquals(segname, "__DATA_CONST")  ||  
            segnameEquals(segname, "__DATA_DIRTY");
    }

    static bool isImageInfoSection(const char *sectname) {
        return sectnameEquals(sectname, "__objc_imageinfo");
    }

    static bool countClasses(FileSlice, section_t& sect, 
                             int& classCount, int& classrefCount)
    {
        if (sectnameEquals(sect.sectname, "__objc_classlist")) {
            classCount += sect.size / 8;
        }
        else if (sectnameEquals(sect.sectname, "__objc_classrefs")) {
            classrefCount += sect.size / 8;
        }
        return YES;
    }
};


#define SANE_HEADER_SIZE (32*1024)

template <typename Arch>
static int sliceRequiresGC(typename Arch::mh_t mh, FileSlice file)
{
    // We assume there is only one arch per pointer size that can support GC.
    // (i386 and x86_64)
    if (mh.cputype != Arch::cputype) return 0;

    // We only check the main executable.
    if (mh.filetype != MH_EXECUTE) return 0;

    // Look for ObjC segment.
    // Look for AppleScriptObjC linkage.
    FileSlice cmds;
    if (!file.slice(sizeof(mh), mh.sizeofcmds, cmds)) return Error;

    // Exception: Some AppleScriptObjC apps built for GC can run without GC.
    // 1. executable defines no classes
    // 2. executable references NSBundle only
    // 3. executable links to AppleScriptObjC.framework
    // Note that shouldRejectGCApp() also knows about this.
    bool wantsGC = NO;
    bool linksToAppleScriptObjC = NO;
    int classCount = 0;
    int classrefCount = 0;

    // Disallow abusively-large executables that could hang this checker.
    // dyld performs similar checks (MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE)
    if (mh.sizeofcmds > SANE_HEADER_SIZE) return Error;
    if (mh.ncmds > mh.sizeofcmds / sizeof(struct load_command)) return Error;

    for (uint32_t cmdindex = 0; cmdindex < mh.ncmds; cmdindex++) {
        struct load_command lc;
        if (!cmds.pread(&lc, sizeof(lc))) return Error;

        // Disallow abusively-small load commands that could hang this checker.
        // dyld performs a similar check.
        if (lc.cmdsize < sizeof(lc)) return Error;

        if (lc.cmd == LC_LOAD_DYLIB  ||  lc.cmd == LC_LOAD_UPWARD_DYLIB  ||  
            lc.cmd == LC_LOAD_WEAK_DYLIB  ||  lc.cmd == LC_REEXPORT_DYLIB) 
        {
            // Look for AppleScriptObjC linkage.
            FileSlice dylibSlice;
            if (!cmds.slice(0, lc.cmdsize, dylibSlice)) return Error;
            struct dylib_command dylib;
            if (!dylibSlice.pread(&dylib, sizeof(dylib))) return Error;

            const char *asoFramework = 
                "/System/Library/Frameworks/AppleScriptObjC.framework"
                "/Versions/A/AppleScriptObjC";
            size_t asoLen = strlen(asoFramework);

            FileSlice nameSlice;
            if (dylibSlice.slice(dylib.dylib.name.offset, asoLen, nameSlice)) {
                char name[asoLen];
                if (!nameSlice.pread(name, asoLen)) return Error;
                if (0 == memcmp(name, asoFramework, asoLen)) {
                    linksToAppleScriptObjC = YES;
                }
            }
        }
        else if (lc.cmd == Arch::segment_cmd) {
            typename Arch::segment_command_t seg;
            if (!cmds.pread(&seg, sizeof(seg))) return Error;

            if (Arch::isObjCSegment(seg.segname)) {
                // ObjC segment. 
                // Look for image info section.
                // Look for class implementations and class references.
                FileSlice sections;
                if (!cmds.slice(0, seg.cmdsize, sections)) return Error;
                if (!sections.advance(sizeof(seg))) return Error;
                
                for (uint32_t segindex = 0; segindex < seg.nsects; segindex++) {
                    typename Arch::section_t sect;
                    if (!sections.pread(&sect, sizeof(sect))) return Error;
                    if (!Arch::isObjCSegment(sect.segname)) return Error;

                    if (!Arch::countClasses(file, sect, 
                                            classCount, classrefCount)) 
                    {
                        return Error;
                    }

                    if ((sect.flags & SECTION_TYPE) == S_REGULAR  &&  
                        Arch::isImageInfoSection(sect.sectname))
                    {
                        // ObjC image info section.
                        // Check its contents.
                        FileSlice section;
                        if (!file.slice(sect.offset, sect.size, section)) {
                            return Error;
                        }
                        // The subset of objc_image_info that was in use for GC.
                        struct {
                            uint32_t version;
                            uint32_t flags;
                        } ii;
                        if (!section.pread(&ii, sizeof(ii))) return Error;
                        if (ii.flags & (1<<1)) {
                            // App wants GC. 
                            // Don't return yet because we need to 
                            // check the AppleScriptObjC exception.
                            wantsGC = YES;
                        }
                    }

                    if (!sections.advance(sizeof(sect))) return Error;
                }
            }
        }

        if (!cmds.advance(lc.cmdsize)) return Error;
    }

    if (!wantsGC) {
        // No GC bit set.
        return WithoutGC;
    }
    else if (linksToAppleScriptObjC && classCount == 0 && classrefCount == 1) {
        // Has GC bit but falls under the AppleScriptObjC exception.
        return WithoutGC;
    }
    else {
        // Has GC bit and is not AppleScriptObjC.
        return WithGC;
    }
}


static int sliceRequiresGC(FileSlice file)
{
    // Read mach-o header.
    struct mach_header_64 mh;
    if (!file.pread(&mh, sizeof(mh))) return Error;

    // Check header magic. We assume only host-endian slices can support GC.
    switch (mh.magic) {
    case MH_MAGIC:
        return sliceRequiresGC<Arch32>(*(struct mach_header *)&mh, file);
    case MH_MAGIC_64:
        return sliceRequiresGC<Arch64>(mh, file);
    default:
        return WithoutGC;
    }
}


// Returns 1 if any slice requires GC.
// Returns 0 if no slice requires GC.
// Returns -1 on any I/O or file format error.
int objc_appRequiresGC(int fd)
{
    struct stat st;
    if (fstat(fd, &st) < 0) return Error;

    FileSlice file(fd, 0, st.st_size);

    // Read fat header, if any.
    struct fat_header fh;

    if (! file.pread(&fh, sizeof(fh))) return Error;

    int result;

    if (OSSwapBigToHostInt32(fh.magic) == FAT_MAGIC) {
        // Fat header.

        size_t nfat_arch = OSSwapBigToHostInt32(fh.nfat_arch);
        // Disallow abusively-large files that could hang this checker.
        if (nfat_arch > SANE_HEADER_SIZE/sizeof(struct fat_arch)) return Error;

        size_t fat_size;
        if (umul_overflow(nfat_arch, sizeof(struct fat_arch), &fat_size)) {
            return Error;
        }

        FileSlice archlist;
        if (!file.slice(sizeof(fh), fat_size, archlist)) return Error;

        result = WithoutGC;
        for (size_t i = 0; i < nfat_arch; i++) {
            struct fat_arch fa;
            if (!archlist.pread(&fa, sizeof(fa))) return Error;
            if (!archlist.advance(sizeof(fa))) return Error;

            FileSlice thin;
            if (!file.slice(OSSwapBigToHostInt32(fa.offset), 
                            OSSwapBigToHostInt32(fa.size), thin)) 
            {
                return Error;
            }
            switch (sliceRequiresGC(thin)) {
            case WithoutGC: break; // no change
            case WithGC: if (result != Error) result = WithGC; break;
            case Error: result = Error; break;
            }
        }
    }
    else {
        // Thin header or not a header.
        result = sliceRequiresGC(file);
    }
    
    return result;
}

// SUPPORT_GC_COMPAT
#endif
