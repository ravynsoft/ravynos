/*
 * Copyright (c) 2012 Apple Inc.  All Rights Reserved.
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
  objc-opt.mm
  Management of optimizations in the dyld shared cache 
*/

#include "objc-private.h"
#include "objc-os.h"
#include "objc-file.h"


#if !SUPPORT_PREOPT
// Preoptimization not supported on this platform.

bool isPreoptimized(void) 
{
    return false;
}

bool noMissingWeakSuperclasses(void) 
{
    return false;
}

bool header_info::isPreoptimized() const
{
    return false;
}

bool header_info::hasPreoptimizedSelectors() const
{
    return false;
}

bool header_info::hasPreoptimizedClasses() const
{
    return false;
}

bool header_info::hasPreoptimizedProtocols() const
{
    return false;
}

Protocol *getPreoptimizedProtocol(const char *name)
{
    return nil;
}

unsigned int getPreoptimizedClassUnreasonableCount()
{
    return 0;
}

Class getPreoptimizedClass(const char *name)
{
    return nil;
}

Class* copyPreoptimizedClasses(const char *name, int *outCount)
{
    *outCount = 0;
    return nil;
}

header_info *preoptimizedHinfoForHeader(const headerType *mhdr)
{
    return nil;
}

header_info_rw *getPreoptimizedHeaderRW(const struct header_info *const hdr)
{
    return nil;
}

void preopt_init(void)
{
    disableSharedCacheOptimizations();
    
    if (PrintPreopt) {
        _objc_inform("PREOPTIMIZATION: is DISABLED "
                     "(not supported on ths platform)");
    }
}


// !SUPPORT_PREOPT
#else
// SUPPORT_PREOPT

#include <objc-shared-cache.h>

using objc_opt::objc_stringhash_offset_t;
using objc_opt::objc_protocolopt2_t;
using objc_opt::objc_clsopt_t;
using objc_opt::objc_headeropt_ro_t;
using objc_opt::objc_headeropt_rw_t;
using objc_opt::objc_opt_t;

__BEGIN_DECLS

// preopt: the actual opt used at runtime (nil or &_objc_opt_data)
// _objc_opt_data: opt data possibly written by dyld
// opt is initialized to ~0 to detect incorrect use before preopt_init()

static const objc_opt_t *opt = (objc_opt_t *)~0;
static bool preoptimized;

extern const objc_opt_t _objc_opt_data;  // in __TEXT, __objc_opt_ro

namespace objc_opt {
struct objc_headeropt_ro_t {
    uint32_t count;
    uint32_t entsize;
    header_info headers[0];  // sorted by mhdr address

    header_info& getOrEnd(uint32_t i) const {
        ASSERT(i <= count);
        return *(header_info *)((uint8_t *)&headers + (i * entsize));
    }

    header_info& get(uint32_t i) const {
        ASSERT(i < count);
        return *(header_info *)((uint8_t *)&headers + (i * entsize));
    }

    uint32_t index(const header_info* hi) const {
        const header_info* begin = &get(0);
        const header_info* end = &getOrEnd(count);
        ASSERT(hi >= begin && hi < end);
        return (uint32_t)(((uintptr_t)hi - (uintptr_t)begin) / entsize);
    }

    header_info *get(const headerType *mhdr)
    {
        int32_t start = 0;
        int32_t end = count;
        while (start <= end) {
            int32_t i = (start+end)/2;
            header_info &hi = get(i);
            if (mhdr == hi.mhdr()) return &hi;
            else if (mhdr < hi.mhdr()) end = i-1;
            else start = i+1;
        }

#if DEBUG
        for (uint32_t i = 0; i < count; i++) {
            header_info &hi = get(i);
            if (mhdr == hi.mhdr()) {
                _objc_fatal("failed to find header %p (%d/%d)",
                            mhdr, i, count);
            }
        }
#endif

        return nil;
    }
};

struct objc_headeropt_rw_t {
    uint32_t count;
    uint32_t entsize;
    header_info_rw headers[0];  // sorted by mhdr address
};
};

/***********************************************************************
* Return YES if we have a valid optimized shared cache.
**********************************************************************/
bool isPreoptimized(void) 
{
    return preoptimized;
}


/***********************************************************************
* Return YES if the shared cache does not have any classes with 
* missing weak superclasses.
**********************************************************************/
bool noMissingWeakSuperclasses(void) 
{
    if (!preoptimized) return NO;  // might have missing weak superclasses
    return opt->flags & objc_opt::NoMissingWeakSuperclasses;
}


/***********************************************************************
* Return YES if this image's dyld shared cache optimizations are valid.
**********************************************************************/
bool header_info::isPreoptimized() const
{
    // preoptimization disabled for some reason
    if (!preoptimized) return NO;

    // image not from shared cache, or not fixed inside shared cache
    if (!info()->optimizedByDyld()) return NO;

    return YES;
}

bool header_info::hasPreoptimizedSelectors() const
{
    // preoptimization disabled for some reason
    if (!preoptimized) return NO;

    return info()->optimizedByDyld() || info()->optimizedByDyldClosure();
}

bool header_info::hasPreoptimizedClasses() const
{
    // preoptimization disabled for some reason
    if (!preoptimized) return NO;

    return info()->optimizedByDyld() || info()->optimizedByDyldClosure();
}

bool header_info::hasPreoptimizedProtocols() const
{
    // preoptimization disabled for some reason
    if (!preoptimized) return NO;

    return info()->optimizedByDyld() || info()->optimizedByDyldClosure();
}

bool header_info::hasPreoptimizedSectionLookups() const
{
    objc_opt::objc_headeropt_ro_t *hinfoRO = opt->headeropt_ro();
    if (hinfoRO->entsize == (2 * sizeof(intptr_t)))
        return NO;

    return YES;
}

const classref_t *header_info::nlclslist(size_t *outCount) const
{
#if __OBJC2__
    // This field is new, so temporarily be resilient to the shared cache
    // not generating it
    if (isPreoptimized() && hasPreoptimizedSectionLookups()) {
          *outCount = nlclslist_count;
          const classref_t *list = (const classref_t *)(((intptr_t)&nlclslist_offset) + nlclslist_offset);
      #if DEBUG
          size_t debugCount;
          assert((list == _getObjc2NonlazyClassList(mhdr(), &debugCount)) && (*outCount == debugCount));
      #endif
          return list;
    }
    return _getObjc2NonlazyClassList(mhdr(), outCount);
#else
    return NULL;
#endif
}

category_t * const *header_info::nlcatlist(size_t *outCount) const
{
#if __OBJC2__
    // This field is new, so temporarily be resilient to the shared cache
    // not generating it
    if (isPreoptimized() && hasPreoptimizedSectionLookups()) {
        *outCount = nlcatlist_count;
        category_t * const *list = (category_t * const *)(((intptr_t)&nlcatlist_offset) + nlcatlist_offset);
        #if DEBUG
        size_t debugCount;
        assert((list == _getObjc2NonlazyCategoryList(mhdr(), &debugCount)) && (*outCount == debugCount));
        #endif
        return list;
    }
    return _getObjc2NonlazyCategoryList(mhdr(), outCount);
#else
    return NULL;
#endif
}

category_t * const *header_info::catlist(size_t *outCount) const
{
#if __OBJC2__
    // This field is new, so temporarily be resilient to the shared cache
    // not generating it
    if (isPreoptimized() && hasPreoptimizedSectionLookups()) {
      *outCount = catlist_count;
      category_t * const *list = (category_t * const *)(((intptr_t)&catlist_offset) + catlist_offset);
      #if DEBUG
      size_t debugCount;
      assert((list == _getObjc2CategoryList(mhdr(), &debugCount)) && (*outCount == debugCount));
      #endif
      return list;
    }
    return _getObjc2CategoryList(mhdr(), outCount);
#else
    return NULL;
#endif
}

category_t * const *header_info::catlist2(size_t *outCount) const
{
#if __OBJC2__
    // This field is new, so temporarily be resilient to the shared cache
    // not generating it
    if (isPreoptimized() && hasPreoptimizedSectionLookups()) {
      *outCount = catlist2_count;
      category_t * const *list = (category_t * const *)(((intptr_t)&catlist2_offset) + catlist2_offset);
      #if DEBUG
      size_t debugCount;
      assert((list == _getObjc2CategoryList2(mhdr(), &debugCount)) && (*outCount == debugCount));
      #endif
      return list;
    }
    return _getObjc2CategoryList2(mhdr(), outCount);
#else
    return NULL;
#endif
}


Protocol *getSharedCachePreoptimizedProtocol(const char *name)
{
    objc_protocolopt2_t *protocols = opt ? opt->protocolopt2() : nil;
    if (!protocols) return nil;

    // Note, we have to pass the lambda directly here as otherwise we would try
    // message copy and autorelease.
    return (Protocol *)protocols->getProtocol(name, [](const void* hi) -> bool {
      return ((header_info *)hi)->isLoaded();
    });
}


Protocol *getPreoptimizedProtocol(const char *name)
{
    objc_protocolopt2_t *protocols = opt ? opt->protocolopt2() : nil;
    if (!protocols) return nil;

    // Try table from dyld closure first.  It was built to ignore the dupes it
    // knows will come from the cache, so anything left in here was there when
    // we launched
    Protocol *result = nil;
    // Note, we have to pass the lambda directly here as otherwise we would try
    // message copy and autorelease.
    _dyld_for_each_objc_protocol(name, [&result](void* protocolPtr, bool isLoaded, bool* stop) {
        // Skip images which aren't loaded.  This supports the case where dyld
        // might soft link an image from the main binary so its possibly not
        // loaded yet.
        if (!isLoaded)
            return;

        // Found a loaded image with this class name, so stop the search
        result = (Protocol *)protocolPtr;
        *stop = true;
    });
    if (result) return result;

    return getSharedCachePreoptimizedProtocol(name);
}


unsigned int getPreoptimizedClassUnreasonableCount()
{
    objc_clsopt_t *classes = opt ? opt->clsopt() : nil;
    if (!classes) return 0;
    
    // This is an overestimate: each set of duplicates 
    // gets double-counted in `capacity` as well.
    return classes->capacity + classes->duplicateCount();
}


Class getPreoptimizedClass(const char *name)
{
    objc_clsopt_t *classes = opt ? opt->clsopt() : nil;
    if (!classes) return nil;

    // Try table from dyld closure first.  It was built to ignore the dupes it
    // knows will come from the cache, so anything left in here was there when
    // we launched
    Class result = nil;
    // Note, we have to pass the lambda directly here as otherwise we would try
    // message copy and autorelease.
    _dyld_for_each_objc_class(name, [&result](void* classPtr, bool isLoaded, bool* stop) {
        // Skip images which aren't loaded.  This supports the case where dyld
        // might soft link an image from the main binary so its possibly not
        // loaded yet.
        if (!isLoaded)
            return;

        // Found a loaded image with this class name, so stop the search
        result = (Class)classPtr;
        *stop = true;
    });
    if (result) return result;

    void *cls;
    void *hi;
    uint32_t count = classes->getClassAndHeader(name, cls, hi);
    if (count == 1  &&  ((header_info *)hi)->isLoaded()) {
        // exactly one matching class, and its image is loaded
        return (Class)cls;
    } 
    else if (count > 1) {
        // more than one matching class - find one that is loaded
        void *clslist[count];
        void *hilist[count];
        classes->getClassesAndHeaders(name, clslist, hilist);
        for (uint32_t i = 0; i < count; i++) {
            if (((header_info *)hilist[i])->isLoaded()) {
                return (Class)clslist[i];
            }
        }
    }

    // no match that is loaded
    return nil;
}


Class* copyPreoptimizedClasses(const char *name, int *outCount)
{
    *outCount = 0;

    objc_clsopt_t *classes = opt ? opt->clsopt() : nil;
    if (!classes) return nil;

    void *cls;
    void *hi;
    uint32_t count = classes->getClassAndHeader(name, cls, hi);
    if (count == 0) return nil;

    Class *result = (Class *)calloc(count, sizeof(Class));
    if (count == 1  &&  ((header_info *)hi)->isLoaded()) {
        // exactly one matching class, and its image is loaded
        result[(*outCount)++] = (Class)cls;
        return result;
    } 
    else if (count > 1) {
        // more than one matching class - find those that are loaded
        void *clslist[count];
        void *hilist[count];
        classes->getClassesAndHeaders(name, clslist, hilist);
        for (uint32_t i = 0; i < count; i++) {
            if (((header_info *)hilist[i])->isLoaded()) {
                result[(*outCount)++] = (Class)clslist[i];
            }
        }

        if (*outCount == 0) {
            // found multiple classes with that name, but none are loaded
            free(result);
            result = nil;
        }
        return result;
    }

    // no match that is loaded
    return nil;
}


header_info *preoptimizedHinfoForHeader(const headerType *mhdr)
{
#if !__OBJC2__
    // fixme old ABI shared cache doesn't prepare these properly
    return nil;
#endif

    objc_headeropt_ro_t *hinfos = opt ? opt->headeropt_ro() : nil;
    if (hinfos) return hinfos->get(mhdr);
    else return nil;
}


header_info_rw *getPreoptimizedHeaderRW(const struct header_info *const hdr)
{
#if !__OBJC2__
    // fixme old ABI shared cache doesn't prepare these properly
    return nil;
#endif
    
    objc_headeropt_ro_t *hinfoRO = opt ? opt->headeropt_ro() : nil;
    objc_headeropt_rw_t *hinfoRW = opt ? opt->headeropt_rw() : nil;
    if (!hinfoRO || !hinfoRW) {
        _objc_fatal("preoptimized header_info missing for %s (%p %p %p)",
                    hdr->fname(), hdr, hinfoRO, hinfoRW);
    }
    int32_t index = hinfoRO->index(hdr);
    ASSERT(hinfoRW->entsize == sizeof(header_info_rw));
    return &hinfoRW->headers[index];
}


void preopt_init(void)
{
    // Get the memory region occupied by the shared cache.
    size_t length;
    const uintptr_t start = (uintptr_t)_dyld_get_shared_cache_range(&length);

    if (start) {
        objc::dataSegmentsRanges.add(start, start + length);
    }
    
    // `opt` not set at compile time in order to detect too-early usage
    const char *failure = nil;
    opt = &_objc_opt_data;

    if (DisablePreopt) {
        // OBJC_DISABLE_PREOPTIMIZATION is set
        // If opt->version != VERSION then you continue at your own risk.
        failure = "(by OBJC_DISABLE_PREOPTIMIZATION)";
    } 
    else if (opt->version != objc_opt::VERSION) {
        // This shouldn't happen. You probably forgot to edit objc-sel-table.s.
        // If dyld really did write the wrong optimization version, 
        // then we must halt because we don't know what bits dyld twiddled.
        _objc_fatal("bad objc preopt version (want %d, got %d)", 
                    objc_opt::VERSION, opt->version);
    }
    else if (!opt->selopt()  ||  !opt->headeropt_ro()) {
        // One of the tables is missing. 
        failure = "(dyld shared cache is absent or out of date)";
    }
    
    if (failure) {
        // All preoptimized selector references are invalid.
        preoptimized = NO;
        opt = nil;
        disableSharedCacheOptimizations();

        if (PrintPreopt) {
            _objc_inform("PREOPTIMIZATION: is DISABLED %s", failure);
        }
    }
    else {
        // Valid optimization data written by dyld shared cache
        preoptimized = YES;

        if (PrintPreopt) {
            _objc_inform("PREOPTIMIZATION: is ENABLED "
                         "(version %d)", opt->version);
        }
    }
}


__END_DECLS

// SUPPORT_PREOPT
#endif
