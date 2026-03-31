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
/*
 *    objc-private.h
 *    Copyright 1988-1996, NeXT Software, Inc.
 */

#ifndef _OBJC_PRIVATE_H_
#define _OBJC_PRIVATE_H_

#include "objc-config.h"

/* Isolate ourselves from the definitions of id and Class in the compiler 
 * and public headers.
 */

#ifdef _OBJC_OBJC_H_
#error include objc-private.h before other headers
#endif

#define OBJC_TYPES_DEFINED 1
#undef OBJC_OLD_DISPATCH_PROTOTYPES
#define OBJC_OLD_DISPATCH_PROTOTYPES 0

#include <cstddef>  // for nullptr_t
#include <utility>  // for std::move
#include <stdint.h>
#include <assert.h>

// An assert that's disabled for release builds but still ensures the expression compiles.
#ifdef NDEBUG
#define ASSERT(x) (void)sizeof(!(x))
#else
#define ASSERT(x) assert(x)
#endif

struct objc_class;
struct objc_object;
struct category_t;

typedef struct objc_class *Class;
typedef struct objc_object *id;
typedef struct classref *classref_t;

namespace {
    struct SideTable;
};

#include "isa.h"

union isa_t {
    isa_t() { }
    isa_t(uintptr_t value) : bits(value) { }

    Class cls;
    uintptr_t bits;
#if defined(ISA_BITFIELD)
    struct {
        ISA_BITFIELD;  // defined in isa.h
    };
#endif
};


struct objc_object {
private:
    isa_t isa;

public:

    // ISA() assumes this is NOT a tagged pointer object
    Class ISA();

    // rawISA() assumes this is NOT a tagged pointer object or a non pointer ISA
    Class rawISA();

    // getIsa() allows this to be a tagged pointer object
    Class getIsa();
    
    uintptr_t isaBits() const;

    // initIsa() should be used to init the isa of new objects only.
    // If this object already has an isa, use changeIsa() for correctness.
    // initInstanceIsa(): objects with no custom RR/AWZ
    // initClassIsa(): class objects
    // initProtocolIsa(): protocol objects
    // initIsa(): other objects
    void initIsa(Class cls /*nonpointer=false*/);
    void initClassIsa(Class cls /*nonpointer=maybe*/);
    void initProtocolIsa(Class cls /*nonpointer=maybe*/);
    void initInstanceIsa(Class cls, bool hasCxxDtor);

    // changeIsa() should be used to change the isa of existing objects.
    // If this is a new object, use initIsa() for performance.
    Class changeIsa(Class newCls);

    bool hasNonpointerIsa();
    bool isTaggedPointer();
    bool isBasicTaggedPointer();
    bool isExtTaggedPointer();
    bool isClass();

    // object may have associated objects?
    bool hasAssociatedObjects();
    void setHasAssociatedObjects();

    // object may be weakly referenced?
    bool isWeaklyReferenced();
    void setWeaklyReferenced_nolock();

    // object may have -.cxx_destruct implementation?
    bool hasCxxDtor();

    // Optimized calls to retain/release methods
    id retain();
    void release();
    id autorelease();

    // Implementations of retain/release methods
    id rootRetain();
    bool rootRelease();
    id rootAutorelease();
    bool rootTryRetain();
    bool rootReleaseShouldDealloc();
    uintptr_t rootRetainCount();

    // Implementation of dealloc methods
    bool rootIsDeallocating();
    void clearDeallocating();
    void rootDealloc();

private:
    void initIsa(Class newCls, bool nonpointer, bool hasCxxDtor);

    // Slow paths for inline control
    id rootAutorelease2();
    uintptr_t overrelease_error();

#if SUPPORT_NONPOINTER_ISA
    // Unified retain count manipulation for nonpointer isa
    id rootRetain(bool tryRetain, bool handleOverflow);
    bool rootRelease(bool performDealloc, bool handleUnderflow);
    id rootRetain_overflow(bool tryRetain);
    uintptr_t rootRelease_underflow(bool performDealloc);

    void clearDeallocating_slow();

    // Side table retain count overflow for nonpointer isa
    void sidetable_lock();
    void sidetable_unlock();

    void sidetable_moveExtraRC_nolock(size_t extra_rc, bool isDeallocating, bool weaklyReferenced);
    bool sidetable_addExtraRC_nolock(size_t delta_rc);
    size_t sidetable_subExtraRC_nolock(size_t delta_rc);
    size_t sidetable_getExtraRC_nolock();
#endif

    // Side-table-only retain count
    bool sidetable_isDeallocating();
    void sidetable_clearDeallocating();

    bool sidetable_isWeaklyReferenced();
    void sidetable_setWeaklyReferenced_nolock();

    id sidetable_retain();
    id sidetable_retain_slow(SideTable& table);

    uintptr_t sidetable_release(bool performDealloc = true);
    uintptr_t sidetable_release_slow(SideTable& table, bool performDealloc = true);

    bool sidetable_tryRetain();

    uintptr_t sidetable_retainCount();
#if DEBUG
    bool sidetable_present();
#endif
};


#if __OBJC2__
typedef struct method_t *Method;
typedef struct ivar_t *Ivar;
typedef struct category_t *Category;
typedef struct property_t *objc_property_t;
#else
typedef struct old_method *Method;
typedef struct old_ivar *Ivar;
typedef struct old_category *Category;
typedef struct old_property *objc_property_t;
#endif

// Public headers

#include "objc.h"
#include "runtime.h"
#include "objc-os.h"
#include "objc-abi.h"
#include "objc-api.h"
#include "objc-config.h"
#include "objc-internal.h"
#include "maptable.h"
#include "hashtable2.h"

/* Do not include message.h here. */
/* #include "message.h" */

#define __APPLE_API_PRIVATE
#include "objc-gdb.h"
#undef __APPLE_API_PRIVATE


// Private headers

#include "objc-ptrauth.h"

#if __OBJC2__
#include "objc-runtime-new.h"
#else
#include "objc-runtime-old.h"
#endif

#include "objc-references.h"
#include "objc-initialize.h"
#include "objc-loadmethod.h"


#define STRINGIFY(x) #x
#define STRINGIFY2(x) STRINGIFY(x)

__BEGIN_DECLS

namespace objc {

struct SafeRanges {
private:
    struct Range {
        uintptr_t start;
        uintptr_t end;

        inline bool contains(uintptr_t ptr) const {
            uintptr_t m_start, m_end;
#if __arm64__
            // <rdar://problem/48304934> Force the compiler to use ldp
            // we really don't want 2 loads and 2 jumps.
            __asm__(
# if __LP64__
                    "ldp %x[one], %x[two], [%x[src]]"
# else
                    "ldp %w[one], %w[two], [%x[src]]"
# endif
                    : [one] "=r" (m_start), [two] "=r" (m_end)
                    : [src] "r" (this)
            );
#else
            m_start = start;
            m_end = end;
#endif
            return m_start <= ptr && ptr < m_end;
        }
    };

    struct Range *ranges;
    uint32_t count;
    uint32_t size : 31;
    uint32_t sorted : 1;

public:
    inline bool contains(uint16_t witness, uintptr_t ptr) const {
        return witness < count && ranges[witness].contains(ptr);
    }

    bool find(uintptr_t ptr, uint32_t &pos);
    void add(uintptr_t start, uintptr_t end);
    void remove(uintptr_t start, uintptr_t end);
};

extern struct SafeRanges dataSegmentsRanges;

} // objc

struct header_info;

// Split out the rw data from header info.  For now put it in a huge array
// that more than exceeds the space needed.  In future we'll just allocate
// this in the shared cache builder.
typedef struct header_info_rw {

    bool getLoaded() const {
        return isLoaded;
    }

    void setLoaded(bool v) {
        isLoaded = v ? 1: 0;
    }

    bool getAllClassesRealized() const {
        return allClassesRealized;
    }

    void setAllClassesRealized(bool v) {
        allClassesRealized = v ? 1: 0;
    }

    header_info *getNext() const {
        return (header_info *)(next << 2);
    }

    void setNext(header_info *v) {
        next = ((uintptr_t)v) >> 2;
    }

private:
#ifdef __LP64__
    uintptr_t isLoaded              : 1;
    uintptr_t allClassesRealized    : 1;
    uintptr_t next                  : 62;
#else
    uintptr_t isLoaded              : 1;
    uintptr_t allClassesRealized    : 1;
    uintptr_t next                  : 30;
#endif
} header_info_rw;

struct header_info_rw* getPreoptimizedHeaderRW(const struct header_info *const hdr);

typedef struct header_info {
private:
    // Note, this is no longer a pointer, but instead an offset to a pointer
    // from this location.
    intptr_t mhdr_offset;

    // Note, this is no longer a pointer, but instead an offset to a pointer
    // from this location.
    intptr_t info_offset;

    // Offset from this location to the non-lazy class list
    intptr_t nlclslist_offset;
    uintptr_t nlclslist_count;

    // Offset from this location to the non-lazy category list
    intptr_t nlcatlist_offset;
    uintptr_t nlcatlist_count;

    // Offset from this location to the category list
    intptr_t catlist_offset;
    uintptr_t catlist_count;

    // Offset from this location to the category list 2
    intptr_t catlist2_offset;
    uintptr_t catlist2_count;

    // Do not add fields without editing ObjCModernAbstraction.hpp
public:

    header_info_rw *getHeaderInfoRW() {
        header_info_rw *preopt =
            isPreoptimized() ? getPreoptimizedHeaderRW(this) : nil;
        if (preopt) return preopt;
        else return &rw_data[0];
    }

    const headerType *mhdr() const {
        return (const headerType *)(((intptr_t)&mhdr_offset) + mhdr_offset);
    }

    void setmhdr(const headerType *mhdr) {
        mhdr_offset = (intptr_t)mhdr - (intptr_t)&mhdr_offset;
    }

    const objc_image_info *info() const {
        return (const objc_image_info *)(((intptr_t)&info_offset) + info_offset);
    }

    void setinfo(const objc_image_info *info) {
        info_offset = (intptr_t)info - (intptr_t)&info_offset;
    }

    const classref_t *nlclslist(size_t *outCount) const;

    void set_nlclslist(const void *list) {
        nlclslist_offset = (intptr_t)list - (intptr_t)&nlclslist_offset;
    }

    category_t * const *nlcatlist(size_t *outCount) const;

    void set_nlcatlist(const void *list) {
        nlcatlist_offset = (intptr_t)list - (intptr_t)&nlcatlist_offset;
    }

    category_t * const *catlist(size_t *outCount) const;

    void set_catlist(const void *list) {
        catlist_offset = (intptr_t)list - (intptr_t)&catlist_offset;
    }

    category_t * const *catlist2(size_t *outCount) const;

    void set_catlist2(const void *list) {
        catlist2_offset = (intptr_t)list - (intptr_t)&catlist2_offset;
    }

    bool isLoaded() {
        return getHeaderInfoRW()->getLoaded();
    }

    void setLoaded(bool v) {
        getHeaderInfoRW()->setLoaded(v);
    }

    bool areAllClassesRealized() {
        return getHeaderInfoRW()->getAllClassesRealized();
    }

    void setAllClassesRealized(bool v) {
        getHeaderInfoRW()->setAllClassesRealized(v);
    }

    header_info *getNext() {
        return getHeaderInfoRW()->getNext();
    }

    void setNext(header_info *v) {
        getHeaderInfoRW()->setNext(v);
    }

    bool isBundle() {
        return mhdr()->filetype == MH_BUNDLE;
    }

    const char *fname() const {
        return dyld_image_path_containing_address(mhdr());
    }

    bool isPreoptimized() const;

    bool hasPreoptimizedSelectors() const;

    bool hasPreoptimizedClasses() const;

    bool hasPreoptimizedProtocols() const;

    bool hasPreoptimizedSectionLookups() const;

#if !__OBJC2__
    struct old_protocol **proto_refs;
    struct objc_module *mod_ptr;
    size_t              mod_count;
# if TARGET_OS_WIN32
    struct objc_module **modules;
    size_t moduleCount;
    struct old_protocol **protocols;
    size_t protocolCount;
    void *imageinfo;
    size_t imageinfoBytes;
    SEL *selrefs;
    size_t selrefCount;
    struct objc_class **clsrefs;
    size_t clsrefCount;    
    TCHAR *moduleName;
# endif
#endif

private:
    // Images in the shared cache will have an empty array here while those
    // allocated at run time will allocate a single entry.
    header_info_rw rw_data[];
} header_info;

extern header_info *FirstHeader;
extern header_info *LastHeader;

extern void appendHeader(header_info *hi);
extern void removeHeader(header_info *hi);

extern objc_image_info *_getObjcImageInfo(const headerType *head, size_t *size);
extern bool _hasObjcContents(const header_info *hi);


// Mach-O segment and section names are 16 bytes and may be un-terminated.

static inline bool segnameEquals(const char *lhs, const char *rhs) {
    return 0 == strncmp(lhs, rhs, 16);
}

static inline bool segnameStartsWith(const char *segname, const char *prefix) {
    return 0 == strncmp(segname, prefix, strlen(prefix));
}

static inline bool sectnameEquals(const char *lhs, const char *rhs) {
    return segnameEquals(lhs, rhs);
}

static inline bool sectnameStartsWith(const char *sectname, const char *prefix){
    return segnameStartsWith(sectname, prefix);
}


#if __OBJC2__
extern bool didCallDyldNotifyRegister;
#endif


/* selectors */
extern void sel_init(size_t selrefCount);
extern SEL sel_registerNameNoLock(const char *str, bool copy);

extern SEL SEL_cxx_construct;
extern SEL SEL_cxx_destruct;

/* preoptimization */
extern void preopt_init(void);
extern void disableSharedCacheOptimizations(void);
extern bool isPreoptimized(void);
extern bool noMissingWeakSuperclasses(void);
extern header_info *preoptimizedHinfoForHeader(const headerType *mhdr);

extern Protocol *getPreoptimizedProtocol(const char *name);
extern Protocol *getSharedCachePreoptimizedProtocol(const char *name);

extern unsigned getPreoptimizedClassUnreasonableCount();
extern Class getPreoptimizedClass(const char *name);
extern Class* copyPreoptimizedClasses(const char *name, int *outCount);

extern Class _calloc_class(size_t size);

/* method lookup */
enum {
    LOOKUP_INITIALIZE = 1,
    LOOKUP_RESOLVER = 2,
    LOOKUP_CACHE = 4,
    LOOKUP_NIL = 8,
};
extern IMP lookUpImpOrForward(id obj, SEL, Class cls, int behavior);

static inline IMP
lookUpImpOrNil(id obj, SEL sel, Class cls, int behavior = 0)
{
    return lookUpImpOrForward(obj, sel, cls, behavior | LOOKUP_CACHE | LOOKUP_NIL);
}

extern IMP lookupMethodInClassAndLoadCache(Class cls, SEL sel);

struct IMPAndSEL {
    IMP imp;
    SEL sel;
};

extern IMPAndSEL _method_getImplementationAndName(Method m);

extern BOOL class_respondsToSelector_inst(id inst, SEL sel, Class cls);
extern Class class_initialize(Class cls, id inst);

extern bool objcMsgLogEnabled;
extern bool logMessageSend(bool isClassMethod,
                    const char *objectsClass,
                    const char *implementingClass,
                    SEL selector);

/* message dispatcher */

#if !OBJC_OLD_DISPATCH_PROTOTYPES
extern void _objc_msgForward_impcache(void);
#else
extern id _objc_msgForward_impcache(id, SEL, ...);
#endif

/* errors */
extern id(*badAllocHandler)(Class);
extern id _objc_callBadAllocHandler(Class cls) __attribute__((cold, noinline));
extern void __objc_error(id, const char *, ...) __attribute__((cold, format (printf, 2, 3), noreturn));
extern void _objc_inform(const char *fmt, ...) __attribute__((cold, format(printf, 1, 2)));
extern void _objc_inform_on_crash(const char *fmt, ...) __attribute__((cold, format (printf, 1, 2)));
extern void _objc_inform_now_and_on_crash(const char *fmt, ...) __attribute__((cold, format (printf, 1, 2)));
extern void _objc_inform_deprecated(const char *oldname, const char *newname) __attribute__((cold, noinline));
extern void inform_duplicate(const char *name, Class oldCls, Class cls);

/* magic */
extern Class _objc_getFreedObjectClass (void);

/* map table additions */
extern void *NXMapKeyCopyingInsert(NXMapTable *table, const void *key, const void *value);
extern void *NXMapKeyFreeingRemove(NXMapTable *table, const void *key);

/* hash table additions */
extern unsigned _NXHashCapacity(NXHashTable *table);
extern void _NXHashRehashToCapacity(NXHashTable *table, unsigned newCapacity);

/* property attribute parsing */
extern const char *copyPropertyAttributeString(const objc_property_attribute_t *attrs, unsigned int count);
extern objc_property_attribute_t *copyPropertyAttributeList(const char *attrs, unsigned int *outCount);
extern char *copyPropertyAttributeValue(const char *attrs, const char *name);

/* locking */

class monitor_locker_t : nocopy_t {
    monitor_t& lock;
  public:
    monitor_locker_t(monitor_t& newLock) : lock(newLock) { lock.enter(); }
    ~monitor_locker_t() { lock.leave(); }
};

class recursive_mutex_locker_t : nocopy_t {
    recursive_mutex_t& lock;
  public:
    recursive_mutex_locker_t(recursive_mutex_t& newLock) 
        : lock(newLock) { lock.lock(); }
    ~recursive_mutex_locker_t() { lock.unlock(); }
};


/* Exceptions */
struct alt_handler_list;
extern void exception_init(void);
extern void _destroyAltHandlerList(struct alt_handler_list *list);

/* Class change notifications (gdb only for now) */
#define OBJC_CLASS_ADDED (1<<0)
#define OBJC_CLASS_REMOVED (1<<1)
#define OBJC_CLASS_IVARS_CHANGED (1<<2)
#define OBJC_CLASS_METHODS_CHANGED (1<<3)
extern void gdb_objc_class_changed(Class cls, unsigned long changes, const char *classname)
    __attribute__((noinline));


// Settings from environment variables
#define OPTION(var, env, help) extern bool var;
#include "objc-env.h"
#undef OPTION

extern void environ_init(void);
extern void runtime_init(void);

extern void logReplacedMethod(const char *className, SEL s, bool isMeta, const char *catName, IMP oldImp, IMP newImp);


// objc per-thread storage
typedef struct {
    struct _objc_initializing_classes *initializingClasses; // for +initialize
    struct SyncCache *syncCache;  // for @synchronize
    struct alt_handler_list *handlerList;  // for exception alt handlers
    char *printableNames[4];  // temporary demangled names for logging
    const char **classNameLookups;  // for objc_getClass() hooks
    unsigned classNameLookupsAllocated;
    unsigned classNameLookupsUsed;

    // If you add new fields here, don't forget to update 
    // _objc_pthread_destroyspecific()

} _objc_pthread_data;

extern _objc_pthread_data *_objc_fetch_pthread_data(bool create);
extern void tls_init(void);

// encoding.h
extern unsigned int encoding_getNumberOfArguments(const char *typedesc);
extern unsigned int encoding_getSizeOfArguments(const char *typedesc);
extern unsigned int encoding_getArgumentInfo(const char *typedesc, unsigned int arg, const char **type, int *offset);
extern void encoding_getReturnType(const char *t, char *dst, size_t dst_len);
extern char * encoding_copyReturnType(const char *t);
extern void encoding_getArgumentType(const char *t, unsigned int index, char *dst, size_t dst_len);
extern char *encoding_copyArgumentType(const char *t, unsigned int index);

// sync.h
extern void _destroySyncCache(struct SyncCache *cache);

// arr
extern void arr_init(void);
extern id objc_autoreleaseReturnValue(id obj);

// block trampolines
extern void _imp_implementationWithBlock_init(void);
extern IMP _imp_implementationWithBlockNoCopy(id block);

// layout.h
typedef struct {
    uint8_t *bits;
    size_t bitCount;
    size_t bitsAllocated;
    bool weak;
} layout_bitmap;
extern layout_bitmap layout_bitmap_create(const unsigned char *layout_string, size_t layoutStringInstanceSize, size_t instanceSize, bool weak);
extern layout_bitmap layout_bitmap_create_empty(size_t instanceSize, bool weak);
extern void layout_bitmap_free(layout_bitmap bits);
extern const unsigned char *layout_string_create(layout_bitmap bits);
extern void layout_bitmap_set_ivar(layout_bitmap bits, const char *type, size_t offset);
extern void layout_bitmap_grow(layout_bitmap *bits, size_t newCount);
extern void layout_bitmap_slide(layout_bitmap *bits, size_t oldPos, size_t newPos);
extern void layout_bitmap_slide_anywhere(layout_bitmap *bits, size_t oldPos, size_t newPos);
extern bool layout_bitmap_splat(layout_bitmap dst, layout_bitmap src, 
                                size_t oldSrcInstanceSize);
extern bool layout_bitmap_or(layout_bitmap dst, layout_bitmap src, const char *msg);
extern bool layout_bitmap_clear(layout_bitmap dst, layout_bitmap src, const char *msg);
extern void layout_bitmap_print(layout_bitmap bits);


// fixme runtime
extern bool MultithreadedForkChild;
extern id objc_noop_imp(id self, SEL _cmd);
extern Class look_up_class(const char *aClassName, bool includeUnconnected, bool includeClassHandler);
extern "C" void map_images(unsigned count, const char * const paths[],
                           const struct mach_header * const mhdrs[]);
extern void map_images_nolock(unsigned count, const char * const paths[],
                              const struct mach_header * const mhdrs[]);
extern void load_images(const char *path, const struct mach_header *mh);
extern void unmap_image(const char *path, const struct mach_header *mh);
extern void unmap_image_nolock(const struct mach_header *mh);
extern void _read_images(header_info **hList, uint32_t hCount, int totalClasses, int unoptimizedTotalClass);
extern void _unload_image(header_info *hi);

extern const header_info *_headerForClass(Class cls);

extern Class _class_remap(Class cls);
extern Ivar _class_getVariable(Class cls, const char *name);

extern unsigned _class_createInstancesFromZone(Class cls, size_t extraBytes, void *zone, id *results, unsigned num_requested);

extern const char *_category_getName(Category cat);
extern const char *_category_getClassName(Category cat);
extern Class _category_getClass(Category cat);
extern IMP _category_getLoadMethod(Category cat);

enum {
    OBJECT_CONSTRUCT_NONE = 0,
    OBJECT_CONSTRUCT_FREE_ONFAILURE = 1,
    OBJECT_CONSTRUCT_CALL_BADALLOC = 2,
};
extern id object_cxxConstructFromClass(id obj, Class cls, int flags);
extern void object_cxxDestruct(id obj);

extern void fixupCopiedIvars(id newObject, id oldObject);
extern Class _class_getClassForIvar(Class cls, Ivar ivar);


#define OBJC_WARN_DEPRECATED \
    do { \
        static int warned = 0; \
        if (!warned) { \
            warned = 1; \
            _objc_inform_deprecated(__FUNCTION__, NULL); \
        } \
    } while (0) \

__END_DECLS


#ifndef STATIC_ASSERT
#   define STATIC_ASSERT(x) _STATIC_ASSERT2(x, __LINE__)
#   define _STATIC_ASSERT2(x, line) _STATIC_ASSERT3(x, line)
#   define _STATIC_ASSERT3(x, line)                                     \
        typedef struct {                                                \
            int _static_assert[(x) ? 0 : -1];                           \
        } _static_assert_ ## line __attribute__((unavailable)) 
#endif

#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))


static __inline uint32_t _objc_strhash(const char *s) {
    uint32_t hash = 0;
    for (;;) {
    int a = *s++;
    if (0 == a) break;
    hash += (hash << 8) + a;
    }
    return hash;
}

#if __cplusplus

template <typename T>
static inline T log2u(T x) {
    return (x<2) ? 0 : log2u(x>>1)+1;
}

template <typename T>
static inline T exp2u(T x) {
    return (1 << x);
}

template <typename T>
static T exp2m1u(T x) { 
    return (1 << x) - 1; 
}

#endif

// Misalignment-safe integer types
__attribute__((aligned(1))) typedef uintptr_t unaligned_uintptr_t;
__attribute__((aligned(1))) typedef  intptr_t unaligned_intptr_t;
__attribute__((aligned(1))) typedef  uint64_t unaligned_uint64_t;
__attribute__((aligned(1))) typedef   int64_t unaligned_int64_t;
__attribute__((aligned(1))) typedef  uint32_t unaligned_uint32_t;
__attribute__((aligned(1))) typedef   int32_t unaligned_int32_t;
__attribute__((aligned(1))) typedef  uint16_t unaligned_uint16_t;
__attribute__((aligned(1))) typedef   int16_t unaligned_int16_t;


// Global operator new and delete. We must not use any app overrides.
// This ALSO REQUIRES each of these be in libobjc's unexported symbol list.
#if __cplusplus
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-new-delete"
#include <new>
inline void* operator new(std::size_t size) throw (std::bad_alloc) { return malloc(size); }
inline void* operator new[](std::size_t size) throw (std::bad_alloc) { return malloc(size); }
inline void* operator new(std::size_t size, const std::nothrow_t&) throw() { return malloc(size); }
inline void* operator new[](std::size_t size, const std::nothrow_t&) throw() { return malloc(size); }
inline void operator delete(void* p) throw() { free(p); }
inline void operator delete[](void* p) throw() { free(p); }
inline void operator delete(void* p, const std::nothrow_t&) throw() { free(p); }
inline void operator delete[](void* p, const std::nothrow_t&) throw() { free(p); }
#pragma clang diagnostic pop
#endif


class TimeLogger {
    uint64_t mStart;
    bool mRecord;
 public:
    TimeLogger(bool record = true) 
     : mStart(nanoseconds())
     , mRecord(record) 
    { }

    void log(const char *msg) {
        if (mRecord) {
            uint64_t end = nanoseconds();
            _objc_inform("%.2f ms: %s", (end - mStart) / 1000000.0, msg);
            mStart = nanoseconds();
        }
    }
};

enum { CacheLineSize = 64 };

// StripedMap<T> is a map of void* -> T, sized appropriately 
// for cache-friendly lock striping. 
// For example, this may be used as StripedMap<spinlock_t>
// or as StripedMap<SomeStruct> where SomeStruct stores a spin lock.
template<typename T>
class StripedMap {
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
    enum { StripeCount = 8 };
#else
    enum { StripeCount = 64 };
#endif

    struct PaddedT {
        T value alignas(CacheLineSize);
    };

    PaddedT array[StripeCount];

    static unsigned int indexForPointer(const void *p) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        return ((addr >> 4) ^ (addr >> 9)) % StripeCount;
    }

 public:
    T& operator[] (const void *p) { 
        return array[indexForPointer(p)].value; 
    }
    const T& operator[] (const void *p) const { 
        return const_cast<StripedMap<T>>(this)[p]; 
    }

    // Shortcuts for StripedMaps of locks.
    void lockAll() {
        for (unsigned int i = 0; i < StripeCount; i++) {
            array[i].value.lock();
        }
    }

    void unlockAll() {
        for (unsigned int i = 0; i < StripeCount; i++) {
            array[i].value.unlock();
        }
    }

    void forceResetAll() {
        for (unsigned int i = 0; i < StripeCount; i++) {
            array[i].value.forceReset();
        }
    }

    void defineLockOrder() {
        for (unsigned int i = 1; i < StripeCount; i++) {
            lockdebug_lock_precedes_lock(&array[i-1].value, &array[i].value);
        }
    }

    void precedeLock(const void *newlock) {
        // assumes defineLockOrder is also called
        lockdebug_lock_precedes_lock(&array[StripeCount-1].value, newlock);
    }

    void succeedLock(const void *oldlock) {
        // assumes defineLockOrder is also called
        lockdebug_lock_precedes_lock(oldlock, &array[0].value);
    }

    const void *getLock(int i) {
        if (i < StripeCount) return &array[i].value;
        else return nil;
    }
    
#if DEBUG
    StripedMap() {
        // Verify alignment expectations.
        uintptr_t base = (uintptr_t)&array[0].value;
        uintptr_t delta = (uintptr_t)&array[1].value - base;
        ASSERT(delta % CacheLineSize == 0);
        ASSERT(base % CacheLineSize == 0);
    }
#else
    constexpr StripedMap() {}
#endif
};


// DisguisedPtr<T> acts like pointer type T*, except the 
// stored value is disguised to hide it from tools like `leaks`.
// nil is disguised as itself so zero-filled memory works as expected, 
// which means 0x80..00 is also disguised as itself but we don't care.
// Note that weak_entry_t knows about this encoding.
template <typename T>
class DisguisedPtr {
    uintptr_t value;

    static uintptr_t disguise(T* ptr) {
        return -(uintptr_t)ptr;
    }

    static T* undisguise(uintptr_t val) {
        return (T*)-val;
    }

 public:
    DisguisedPtr() { }
    DisguisedPtr(T* ptr) 
        : value(disguise(ptr)) { }
    DisguisedPtr(const DisguisedPtr<T>& ptr) 
        : value(ptr.value) { }

    DisguisedPtr<T>& operator = (T* rhs) {
        value = disguise(rhs);
        return *this;
    }
    DisguisedPtr<T>& operator = (const DisguisedPtr<T>& rhs) {
        value = rhs.value;
        return *this;
    }

    operator T* () const {
        return undisguise(value);
    }
    T* operator -> () const { 
        return undisguise(value);
    }
    T& operator * () const { 
        return *undisguise(value);
    }
    T& operator [] (size_t i) const {
        return undisguise(value)[i];
    }

    // pointer arithmetic operators omitted 
    // because we don't currently use them anywhere
};

// fixme type id is weird and not identical to objc_object*
static inline bool operator == (DisguisedPtr<objc_object> lhs, id rhs) {
    return lhs == (objc_object *)rhs;
}
static inline bool operator != (DisguisedPtr<objc_object> lhs, id rhs) {
    return lhs != (objc_object *)rhs;
}


// Storage for a thread-safe chained hook function.
// get() returns the value for calling.
// set() installs a new function and returns the old one for chaining.
// More precisely, set() writes the old value to a variable supplied by
// the caller. get() and set() use appropriate barriers so that the
// old value is safely written to the variable before the new value is
// called to use it.
//
// T1: store to old variable; store-release to hook variable
// T2: load-acquire from hook variable; call it; called hook loads old variable

template <typename Fn>
class ChainedHookFunction {
    std::atomic<Fn> hook{nil};

public:
    constexpr ChainedHookFunction(Fn f) : hook{f} { };

    Fn get() {
        return hook.load(std::memory_order_acquire);
    }

    void set(Fn newValue, Fn *oldVariable)
    {
        Fn oldValue = hook.load(std::memory_order_relaxed);
        do {
            *oldVariable = oldValue;
        } while (!hook.compare_exchange_weak(oldValue, newValue,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
    }
};


// A small vector for use as a global variable. Only supports appending and
// iteration. Stores up to N elements inline, and multiple elements in a heap
// allocation. There is no attempt to amortize reallocation cost; this is
// intended to be used in situation where a small number of elements is
// common, more might happen, and significantly more is very rare.
//
// This does not clean up its allocation, and thus cannot be used as a local
// variable or member of something with limited lifetime.

template <typename T, unsigned InlineCount>
class GlobalSmallVector {
    static_assert(std::is_pod<T>::value, "SmallVector requires POD types");
    
protected:
    unsigned count{0};
    union {
        T inlineElements[InlineCount];
        T *elements{nullptr};
    };
    
public:
    void append(const T &val) {
        if (count < InlineCount) {
            // We have space. Store the new value inline.
            inlineElements[count] = val;
        } else if (count == InlineCount) {
            // Inline storage is full. Switch to a heap allocation.
            T *newElements = (T *)malloc((count + 1) * sizeof(T));
            memcpy(newElements, inlineElements, count * sizeof(T));
            newElements[count] = val;
            elements = newElements;
        } else {
            // Resize the heap allocation and append.
            elements = (T *)realloc(elements, (count + 1) * sizeof(T));
            elements[count] = val;
        }
        count++;
    }
    
    const T *begin() const {
        return count <= InlineCount ? inlineElements : elements;
    }
    
    const T *end() const {
        return begin() + count;
    }
};

// A small vector that cleans up its internal memory allocation when destroyed.
template <typename T, unsigned InlineCount>
class SmallVector: public GlobalSmallVector<T, InlineCount> {
public:
    ~SmallVector() {
        if (this->count > InlineCount)
            free(this->elements);
    }

    template <unsigned OtherCount>
    void initFrom(const GlobalSmallVector<T, OtherCount> &other) {
        ASSERT(this->count == 0);
        this->count = (unsigned)(other.end() - other.begin());
        if (this->count > InlineCount) {
            this->elements = (T *)memdup(other.begin(), this->count * sizeof(T));
        } else {
            memcpy(this->inlineElements, other.begin(), this->count * sizeof(T));
        }
    }
};

// Pointer hash function.
// This is not a terrific hash, but it is fast 
// and not outrageously flawed for our purposes.

// Based on principles from http://locklessinc.com/articles/fast_hash/
// and evaluation ideas from http://floodyberry.com/noncryptohashzoo/
#if __LP64__
static inline uint32_t ptr_hash(uint64_t key)
{
    key ^= key >> 4;
    key *= 0x8a970be7488fda55;
    key ^= __builtin_bswap64(key);
    return (uint32_t)key;
}
#else
static inline uint32_t ptr_hash(uint32_t key)
{
    key ^= key >> 4;
    key *= 0x5052acdb;
    key ^= __builtin_bswap32(key);
    return key;
}
#endif

/*
  Higher-quality hash function. This is measurably slower in some workloads.
#if __LP64__
 uint32_t ptr_hash(uint64_t key)
{
    key -= __builtin_bswap64(key);
    key *= 0x8a970be7488fda55;
    key ^= __builtin_bswap64(key);
    key *= 0x8a970be7488fda55;
    key ^= __builtin_bswap64(key);
    return (uint32_t)key;
}
#else
static uint32_t ptr_hash(uint32_t key)
{
    key -= __builtin_bswap32(key);
    key *= 0x5052acdb;
    key ^= __builtin_bswap32(key);
    key *= 0x5052acdb;
    key ^= __builtin_bswap32(key);
    return key;
}
#endif
*/



// Lock declarations
#include "objc-locks.h"

// Inlined parts of objc_object's implementation
#include "objc-object.h"

#endif /* _OBJC_PRIVATE_H_ */

