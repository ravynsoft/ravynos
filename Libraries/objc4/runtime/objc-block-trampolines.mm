/*
 * Copyright (c) 2010 Apple Inc.  All Rights Reserved.
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
 * objc-block-trampolines.m
 * Author:	b.bum
 *
 **********************************************************************/

/***********************************************************************
 * Imports.
 **********************************************************************/
#include "objc-private.h"
#include "runtime.h"

#include <Block.h>
#include <Block_private.h>
#include <mach/mach.h>
#include <objc/objc-block-trampolines.h>

// fixme C++ compilers don't implemement memory_order_consume efficiently.
// Use memory_order_relaxed and cross our fingers.
#define MEMORY_ORDER_CONSUME std::memory_order_relaxed

// 8 bytes of text and data per trampoline on all architectures.
#define SLOT_SIZE 8

// The trampolines are defined in assembly files in libobjc-trampolines.dylib.
// We can't link to libobjc-trampolines.dylib directly because
// for security reasons it isn't in the dyld shared cache.

// Trampoline addresses are lazily looked up.
// All of them are hidden behind a single atomic pointer for lock-free init.

#ifdef __PTRAUTH_INTRINSICS__
#   define TrampolinePtrauth __ptrauth(ptrauth_key_function_pointer, 1, 0x3af1)
#else
#   define TrampolinePtrauth
#endif

class TrampolinePointerWrapper {
    struct TrampolinePointers {
        class TrampolineAddress {
            const void * TrampolinePtrauth storage;

        public:
            TrampolineAddress(void *dylib, const char *name) {
#define PREFIX "_objc_blockTrampoline"
                char symbol[strlen(PREFIX) + strlen(name) + 1];
                strcpy(symbol, PREFIX);
                strcat(symbol, name);
                // dlsym() from a text segment returns a signed pointer
                // Authenticate it manually and let the compiler re-sign it.
                storage = ptrauth_auth_data(dlsym(dylib, symbol),
                                            ptrauth_key_function_pointer, 0);
                if (!storage) {
                    _objc_fatal("couldn't dlsym %s", symbol);
                }
            }

            uintptr_t address() {
                return (uintptr_t)(void*)storage;
            }
        };

        TrampolineAddress impl;   // trampoline header code
        TrampolineAddress start;  // first trampoline
#if DEBUG
        // These symbols are only used in assertions.
        // fixme might be able to move the assertions to libobjc-trampolines itself
        TrampolineAddress last;    // start of the last trampoline
        // We don't use the address after the last trampoline because that
        // address might be in a different section, and then dlsym() would not
        // sign it as a function pointer.
# if SUPPORT_STRET
        TrampolineAddress impl_stret;
        TrampolineAddress start_stret;
        TrampolineAddress last_stret;
# endif
#endif

        uintptr_t textSegment;
        uintptr_t textSegmentSize;

        void check() {
#if DEBUG
            ASSERT(impl.address() == textSegment + PAGE_MAX_SIZE);
            ASSERT(impl.address() % PAGE_SIZE == 0);  // not PAGE_MAX_SIZE
            assert(impl.address() + PAGE_MAX_SIZE ==
                   last.address() + SLOT_SIZE);
            ASSERT(last.address()+8 < textSegment + textSegmentSize);
            ASSERT((last.address() - start.address()) % SLOT_SIZE == 0);
# if SUPPORT_STRET
            ASSERT(impl_stret.address() == textSegment + 2*PAGE_MAX_SIZE);
            ASSERT(impl_stret.address() % PAGE_SIZE == 0);  // not PAGE_MAX_SIZE
            assert(impl_stret.address() + PAGE_MAX_SIZE ==
                   last_stret.address() + SLOT_SIZE);
            assert(start.address() - impl.address() ==
                   start_stret.address() - impl_stret.address());
            assert(last_stret.address() + SLOT_SIZE <
                   textSegment + textSegmentSize);
            assert((last_stret.address() - start_stret.address())
                   % SLOT_SIZE == 0);
# endif
#endif
        }


        TrampolinePointers(void *dylib)
            : impl(dylib, "Impl")
            , start(dylib, "Start")
#if DEBUG
            , last(dylib, "Last")
# if SUPPORT_STRET
            , impl_stret(dylib, "Impl_stret")
            , start_stret(dylib, "Start_stret")
            , last_stret(dylib, "Last_stret")
# endif
#endif
        {
            const auto *mh =
                dyld_image_header_containing_address((void *)impl.address());
            unsigned long size = 0;
            textSegment = (uintptr_t)
                getsegmentdata((headerType *)mh, "__TEXT", &size);
            textSegmentSize = size;

            check();
        }
    };

    std::atomic<TrampolinePointers *> trampolines{nil};

    TrampolinePointers *get() {
        return trampolines.load(MEMORY_ORDER_CONSUME);
    }

public:
    void Initialize() {
        if (get()) return;

        // This code may be called concurrently.
        // In the worst case we perform extra dyld operations.
        void *dylib = dlopen("/usr/lib/libobjc-trampolines.dylib",
                             RTLD_NOW | RTLD_LOCAL | RTLD_FIRST);
        if (!dylib) {
            _objc_fatal("couldn't dlopen libobjc-trampolines.dylib: %s",
                        dlerror());
        }

        auto t = new TrampolinePointers(dylib);
        TrampolinePointers *old = nil;
        if (! trampolines.compare_exchange_strong(old, t, memory_order_release))
        {
            delete t;  // Lost an initialization race.
        }
    }

    uintptr_t textSegment() { return get()->textSegment; }
    uintptr_t textSegmentSize() { return get()->textSegmentSize; }

    // See comments below about PAGE_SIZE and PAGE_MAX_SIZE.
    uintptr_t dataSize() { return PAGE_MAX_SIZE; }

    uintptr_t impl() { return get()->impl.address(); }
    uintptr_t start() { return get()->start.address(); }
};

static TrampolinePointerWrapper Trampolines;

// argument mode identifier
// Some calculations assume that these modes are sequential starting from 0.
// This order must match the order of the trampoline's assembly code.
typedef enum {
    ReturnValueInRegisterArgumentMode,
#if SUPPORT_STRET
    ReturnValueOnStackArgumentMode,
#endif
    
    ArgumentModeCount
} ArgumentMode;

// We must take care with our data layout on architectures that support 
// multiple page sizes.
// 
// The trampoline template in __TEXT is sized and aligned with PAGE_MAX_SIZE.
// On some platforms this requires additional linker flags.
// 
// When we allocate a page group, we use PAGE_MAX_SIZE size. 
// This allows trampoline code to find its data by subtracting PAGE_MAX_SIZE.
// 
// When we allocate a page group, we use the process's page alignment. 
// This simplifies allocation because we don't need to force greater than 
// default alignment when running with small pages, but it also means 
// the trampoline code MUST NOT look for its data by masking with PAGE_MAX_MASK.

struct TrampolineBlockPageGroup
{
    TrampolineBlockPageGroup *nextPageGroup; // linked list of all pages
    TrampolineBlockPageGroup *nextAvailablePage; // linked list of pages with available slots

    uintptr_t nextAvailable; // index of next available slot, endIndex() if no more available

    const void * TrampolinePtrauth const text;  // text VM region; stored only for the benefit of the leaks tool

    TrampolineBlockPageGroup()
        : nextPageGroup(nil)
        , nextAvailablePage(nil)
        , nextAvailable(startIndex())
        , text((const void *)((uintptr_t)this + Trampolines.dataSize()))
    { }
    
    // Payload data: block pointers and free list.
    // Bytes parallel with trampoline header code are the fields above or unused
    // uint8_t payloads[PAGE_MAX_SIZE - sizeof(TrampolineBlockPageGroup)] 

    // Code: Mach-O header, then trampoline header followed by trampolines.
    // On platforms with struct return we have non-stret trampolines and
    //     stret trampolines. The stret and non-stret trampolines at a given
    //     index share the same data page.
    // uint8_t macho[PAGE_MAX_SIZE];
    // uint8_t trampolines[ArgumentModeCount][PAGE_MAX_SIZE];
    
    // Per-trampoline block data format:
    // initial value is 0 while page data is filled sequentially 
    // when filled, value is reference to Block_copy()d block
    // when empty, value is index of next available slot OR 0 if never used yet
    
    union Payload {
        id block;
        uintptr_t nextAvailable;  // free list
    };
    
    static uintptr_t headerSize() {
        return (uintptr_t) (Trampolines.start() - Trampolines.impl());
    }
    
    static uintptr_t slotSize() {
        return SLOT_SIZE;
    }

    static uintptr_t startIndex() {
        // headerSize is assumed to be slot-aligned
        return headerSize() / slotSize();
    }

    static uintptr_t endIndex() {
        return (uintptr_t)Trampolines.dataSize() / slotSize();
    }

    static bool validIndex(uintptr_t index) {
        return (index >= startIndex() && index < endIndex());
    }

    Payload *payload(uintptr_t index) {
        ASSERT(validIndex(index));
        return (Payload *)((char *)this + index*slotSize());
    }

    uintptr_t trampolinesForMode(int aMode) {
        // Skip over the data area, one page of Mach-O headers,
        // and one text page for each mode before this one.
        return (uintptr_t)this + Trampolines.dataSize() +
            PAGE_MAX_SIZE * (1 + aMode);
    }
    
    IMP trampoline(int aMode, uintptr_t index) {
        ASSERT(validIndex(index));
        char *base = (char *)trampolinesForMode(aMode);
        char *imp = base + index*slotSize();
#if __arm__
        imp++;  // trampoline is Thumb instructions
#endif
#if __has_feature(ptrauth_calls)
        imp = ptrauth_sign_unauthenticated(imp,
                                           ptrauth_key_function_pointer, 0);
#endif
        return (IMP)imp;
    }

    uintptr_t indexForTrampoline(uintptr_t tramp) {
        for (int aMode = 0; aMode < ArgumentModeCount; aMode++) {
            uintptr_t base  = trampolinesForMode(aMode);
            uintptr_t start = base + startIndex() * slotSize();
            uintptr_t end   = base + endIndex() * slotSize();
            if (tramp >= start  &&  tramp < end) {
                return (uintptr_t)(tramp - base) / slotSize();
            }
        }
        return 0;
    }

    static void check() {
        ASSERT(TrampolineBlockPageGroup::headerSize() >= sizeof(TrampolineBlockPageGroup));
        ASSERT(TrampolineBlockPageGroup::headerSize() % TrampolineBlockPageGroup::slotSize() == 0);
    }

};

static TrampolineBlockPageGroup *HeadPageGroup;

#pragma mark Utility Functions

#if !__OBJC2__
#define runtimeLock classLock
#endif

#pragma mark Trampoline Management Functions
static TrampolineBlockPageGroup *_allocateTrampolinesAndData()
{
    runtimeLock.assertLocked();

    vm_address_t dataAddress;
    
    TrampolineBlockPageGroup::check();

    // Our final mapping will look roughly like this:
    //   r/w data
    //   r/o text mapped from libobjc-trampolines.dylib
    // with fixed offsets from the text to the data embedded in the text.
    //
    // More precisely it will look like this:
    //   1 page r/w data
    //   1 page libobjc-trampolines.dylib Mach-O header
    //   N pages trampoline code, one for each ArgumentMode
    //   M pages for the rest of libobjc-trampolines' TEXT segment.
    // The kernel requires that we remap the entire TEXT segment every time.
    // We assume that our code begins on the second TEXT page, but are robust
    // against other additions to the end of the TEXT segment.

    ASSERT(HeadPageGroup == nil  ||  HeadPageGroup->nextAvailablePage == nil);

    auto textSource = Trampolines.textSegment();
    auto textSourceSize = Trampolines.textSegmentSize();
    auto dataSize = Trampolines.dataSize();

    // Allocate a single contiguous region big enough to hold data+text.
    kern_return_t result;
    result = vm_allocate(mach_task_self(), &dataAddress,
                         dataSize + textSourceSize,
                         VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_FOUNDATION));
    if (result != KERN_SUCCESS) {
        _objc_fatal("vm_allocate trampolines failed (%d)", result);
    }

    // Remap libobjc-trampolines' TEXT segment atop all
    // but the first of the pages we just allocated:
    vm_address_t textDest = dataAddress + dataSize;
    vm_prot_t currentProtection, maxProtection;
    result = vm_remap(mach_task_self(), &textDest,
                      textSourceSize,
                      0, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE,
                      mach_task_self(), textSource, TRUE, 
                      &currentProtection, &maxProtection, VM_INHERIT_SHARE);
    if (result != KERN_SUCCESS) {
        _objc_fatal("vm_remap trampolines failed (%d)", result);
    }

    auto *pageGroup = new ((void*)dataAddress) TrampolineBlockPageGroup;
    
    if (HeadPageGroup) {
        TrampolineBlockPageGroup *lastPageGroup = HeadPageGroup;
        while(lastPageGroup->nextPageGroup) {
            lastPageGroup = lastPageGroup->nextPageGroup;
        }
        lastPageGroup->nextPageGroup = pageGroup;
        HeadPageGroup->nextAvailablePage = pageGroup;
    } else {
        HeadPageGroup = pageGroup;
    }
    
    return pageGroup;
}

static TrampolineBlockPageGroup *
getOrAllocatePageGroupWithNextAvailable() 
{
    runtimeLock.assertLocked();
    
    if (!HeadPageGroup)
        return _allocateTrampolinesAndData();
    
    // make sure head page is filled first
    if (HeadPageGroup->nextAvailable != HeadPageGroup->endIndex())
        return HeadPageGroup;
    
    if (HeadPageGroup->nextAvailablePage) // check if there is a page w/a hole
        return HeadPageGroup->nextAvailablePage;
    
    return _allocateTrampolinesAndData(); // tack on a new one
}

static TrampolineBlockPageGroup *
pageAndIndexContainingIMP(IMP anImp, uintptr_t *outIndex) 
{
    runtimeLock.assertLocked();

    // Authenticate as a function pointer, returning an un-signed address.
    uintptr_t trampAddress =
            (uintptr_t)ptrauth_auth_data((const char *)anImp,
                                         ptrauth_key_function_pointer, 0);

    for (TrampolineBlockPageGroup *pageGroup = HeadPageGroup; 
         pageGroup;
         pageGroup = pageGroup->nextPageGroup)
    {
        uintptr_t index = pageGroup->indexForTrampoline(trampAddress);
        if (index) {
            if (outIndex) *outIndex = index;
            return pageGroup;
        }
    }
    
    return nil;
}


static ArgumentMode 
argumentModeForBlock(id block) 
{
    ArgumentMode aMode = ReturnValueInRegisterArgumentMode;

#if SUPPORT_STRET
    if (_Block_has_signature(block) && _Block_use_stret(block))
        aMode = ReturnValueOnStackArgumentMode;
#else
    ASSERT(! (_Block_has_signature(block) && _Block_use_stret(block)));
#endif
    
    return aMode;
}

/// Initialize the trampoline machinery. Normally this does nothing, as
/// everything is initialized lazily, but for certain processes we eagerly load
/// the trampolines dylib.
void
_imp_implementationWithBlock_init(void)
{
#if TARGET_OS_OSX
    // Eagerly load libobjc-trampolines.dylib in certain processes. Some
    // programs (most notably QtWebEngineProcess used by older versions of
    // embedded Chromium) enable a highly restrictive sandbox profile which
    // blocks access to that dylib. If anything calls
    // imp_implementationWithBlock (as AppKit has started doing) then we'll
    // crash trying to load it. Loading it here sets it up before the sandbox
    // profile is enabled and blocks it.
    //
    // This fixes EA Origin (rdar://problem/50813789)
    // and Steam (rdar://problem/55286131)
    if (__progname &&
        (strcmp(__progname, "QtWebEngineProcess") == 0 ||
         strcmp(__progname, "Steam Helper") == 0)) {
        Trampolines.Initialize();
    }
#endif
}


// `block` must already have been copied 
IMP 
_imp_implementationWithBlockNoCopy(id block)
{
    runtimeLock.assertLocked();

    TrampolineBlockPageGroup *pageGroup = 
        getOrAllocatePageGroupWithNextAvailable();

    uintptr_t index = pageGroup->nextAvailable;
    ASSERT(index >= pageGroup->startIndex()  &&  index < pageGroup->endIndex());
    TrampolineBlockPageGroup::Payload *payload = pageGroup->payload(index);
    
    uintptr_t nextAvailableIndex = payload->nextAvailable;
    if (nextAvailableIndex == 0) {
        // First time through (unused slots are zero). Fill sequentially.
        // If the page is now full this will now be endIndex(), handled below.
        nextAvailableIndex = index + 1;
    }
    pageGroup->nextAvailable = nextAvailableIndex;
    if (nextAvailableIndex == pageGroup->endIndex()) {
        // PageGroup is now full (free list or wilderness exhausted)
        // Remove from available page linked list
        TrampolineBlockPageGroup *iterator = HeadPageGroup;
        while(iterator && (iterator->nextAvailablePage != pageGroup)) {
            iterator = iterator->nextAvailablePage;
        }
        if (iterator) {
            iterator->nextAvailablePage = pageGroup->nextAvailablePage;
            pageGroup->nextAvailablePage = nil;
        }
    }
    
    payload->block = block;
    return pageGroup->trampoline(argumentModeForBlock(block), index);
}


#pragma mark Public API
IMP imp_implementationWithBlock(id block) 
{
    // Block object must be copied outside runtimeLock
    // because it performs arbitrary work.
    block = Block_copy(block);

    // Trampolines must be initialized outside runtimeLock
    // because it calls dlopen().
    Trampolines.Initialize();
    
    mutex_locker_t lock(runtimeLock);

    return _imp_implementationWithBlockNoCopy(block);
}


id imp_getBlock(IMP anImp) {
    uintptr_t index;
    TrampolineBlockPageGroup *pageGroup;
    
    if (!anImp) return nil;
    
    mutex_locker_t lock(runtimeLock);
    
    pageGroup = pageAndIndexContainingIMP(anImp, &index);
    
    if (!pageGroup) {
        return nil;
    }

    TrampolineBlockPageGroup::Payload *payload = pageGroup->payload(index);
    
    if (payload->nextAvailable <= TrampolineBlockPageGroup::endIndex()) {
        // unallocated
        return nil;
    }
    
    return payload->block;
}

BOOL imp_removeBlock(IMP anImp) {
    
    if (!anImp) return NO;

    id block;
    
    {
        mutex_locker_t lock(runtimeLock);
    
        uintptr_t index;
        TrampolineBlockPageGroup *pageGroup =
            pageAndIndexContainingIMP(anImp, &index);
        
        if (!pageGroup) {
            return NO;
        }
        
        TrampolineBlockPageGroup::Payload *payload = pageGroup->payload(index);
        block = payload->block;
        // block is released below, outside the lock
        
        payload->nextAvailable = pageGroup->nextAvailable;
        pageGroup->nextAvailable = index;
        
        // make sure this page is on available linked list
        TrampolineBlockPageGroup *pageGroupIterator = HeadPageGroup;
        
        // see if page is the next available page for any existing pages
        while (pageGroupIterator->nextAvailablePage && 
               pageGroupIterator->nextAvailablePage != pageGroup)
        {
            pageGroupIterator = pageGroupIterator->nextAvailablePage;
        }
        
        if (! pageGroupIterator->nextAvailablePage) {
            // if iteration stopped because nextAvail was nil
            // add to end of list.
            pageGroupIterator->nextAvailablePage = pageGroup;
            pageGroup->nextAvailablePage = nil;
        }
    }

    // do this AFTER dropping the lock
    Block_release(block);
    return YES;
}
