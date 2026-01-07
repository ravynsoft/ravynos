/* 
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
 * objc-selopt.h
 * Interface between libobjc and dyld 
 * for selector uniquing in the dyld shared cache.
 *
 * When building the shared cache, dyld locates all selectors and selector 
 * references in the cached images. It builds a perfect hash table out of 
 * them and writes the table into the shared cache copy of libobjc.
 * libobjc then uses that table as the builtin selector list.
 *
 * Versioning
 * The table has a version number. dyld and objc can both ignore the table 
 * if the other used the wrong version number.
 *
 * Completeness
 * Not all libraries are in the shared cache. Libraries that are in the 
 * shared cache and were optimized are specially marked. Libraries on 
 * disk never include those marks.
 *
 * Coherency
 * Libraries optimized in the shared cache can be replaced by unoptimized 
 * copies from disk when loaded. The copy from disk is not marked and will 
 * be fixed up by libobjc. The shared cache copy is still mapped into the 
 * process, so the table can point to cstring data in that library's part 
 * of the shared cache without trouble.
 * 
 * Atomicity
 * dyld writes the table itself last. If dyld marks some metadata as 
 * updated but then fails to write a table for some reason, libobjc 
 * fixes up all metadata as if it were not marked.
 */

#ifndef _OBJC_SELOPT_H
#define _OBJC_SELOPT_H

#include <stdint.h>
#include <stdlib.h>

// Tell libobjc that this dyld is built for large caches
// This really means the dyld SPIs are going to visit shared cache hash tables
#define DYLD_LARGE_SHARED_CACHE_SUPPORT 1

namespace objc {
class SelectorHashTable;
class ClassHashTable;
class ProtocolHashTable;
}

namespace objc_opt {

// Precomputed image list.
struct objc_headeropt_ro_t;

// Precomputed image list.
struct objc_headeropt_rw_t;

// Edit objc-sel-table.s if you change this value.
// lldb and Symbolication read these structures. Inform them of any changes.
enum { VERSION = 16 };

// Values for objc_opt_t::flags
enum : uint32_t {
    IsProduction = (1 << 0),                // never set in development cache
    NoMissingWeakSuperclasses = (1 << 1),   // set in development cache and customer
    LargeSharedCache = (1 << 2),            // Shared cache was built with the new Large format
};

// Top-level optimization structure.
// Edit objc-sel-table.s if you change this structure.
struct alignas(alignof(uint64_t)) objc_opt_t {
    uint32_t version;
    uint32_t flags;
    int32_t selopt_offset;
    int32_t headeropt_ro_offset;
    int32_t unused_clsopt_offset;
    int32_t unused_protocolopt_offset; // This is now 0 as we've moved to the new protocolopt_offset
    int32_t headeropt_rw_offset;
    int32_t unused_protocolopt2_offset;
    int32_t largeSharedCachesClassOffset;
    int32_t largeSharedCachesProtocolOffset;
    int64_t relativeMethodSelectorBaseAddressOffset; // Relative method list selectors are offsets from this address

    const objc::SelectorHashTable* selectorOpt() const {
        if (selopt_offset == 0) return NULL;
        return (objc::SelectorHashTable *)((uint8_t *)this + selopt_offset);
    }

    struct objc_headeropt_ro_t* headeropt_ro() const {
        if (headeropt_ro_offset == 0) return NULL;
        return (struct objc_headeropt_ro_t *)((uint8_t *)this + headeropt_ro_offset);
    }

    void* oldClassOpt() const {
        if (unused_clsopt_offset == 0) return NULL;
        return (void *)((uint8_t *)this + unused_clsopt_offset);
    }

    void* protocolopt() const {
        return NULL;
    }

    void* oldProtocolOpt2() const {
        if (unused_protocolopt2_offset == 0) return NULL;
        return (void *)((uint8_t *)this + unused_protocolopt2_offset);
    }

    struct objc_headeropt_rw_t* headeropt_rw() const {
        if (headeropt_rw_offset == 0) return NULL;
        return (struct objc_headeropt_rw_t *)((uint8_t *)this + headeropt_rw_offset);
    }

    const objc::ClassHashTable* classOpt() const {
        if (largeSharedCachesClassOffset == 0) return NULL;
        return (objc::ClassHashTable *)((uint8_t *)this + largeSharedCachesClassOffset);
    }

    const objc::ProtocolHashTable* protocolOpt() const {
        if (largeSharedCachesProtocolOffset == 0) return NULL;
        return (objc::ProtocolHashTable *)((uint8_t *)this + largeSharedCachesProtocolOffset);
    }

    const void* relativeMethodListsBaseAddress() const {
        if (relativeMethodSelectorBaseAddressOffset == 0) return NULL;
        return (const void*)((uint8_t *)this + relativeMethodSelectorBaseAddressOffset);
    }
};

// sizeof(objc_opt_t) must be pointer-aligned
static_assert(sizeof(objc_opt_t) % sizeof(void*) == 0);


// List of offsets in libobjc that the shared cache optimization needs to use.
template <typename T>
struct objc_opt_pointerlist_tt {
    T protocolClass;
};
typedef struct objc_opt_pointerlist_tt<uintptr_t> objc_opt_pointerlist_t;

} // namespace objc_opt

#endif
