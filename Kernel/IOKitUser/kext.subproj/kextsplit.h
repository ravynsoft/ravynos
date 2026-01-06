/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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
#ifndef SPLIT_KEXT_H
#define SPLIT_KEXT_H
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif
    /*
     * C functions provided by kcgen, used by segment adjusting C++ logic
     */
    //extern void kcgen_uniqueWarning(const char* format, ...) __printflike(1, 2);

    extern void _kcgen_warning(const char* format, ...) __printflike(1, 2);
    #define kcgen_warning(fmt, ...) \
            _kcgen_warning("kcgen[%s:%d] W: " fmt, __func__, __LINE__, ## __VA_ARGS__)

    extern void _kcgen_verboseLog(const char* format, ...) __printflike(1, 2);
    #define kcgen_verboseLog(fmt, ...) \
            _kcgen_verboseLog("kcgen[%s:%d] V: " fmt, __func__, __LINE__, ## __VA_ARGS__)

    extern void _kcgen_log(const char * __restrict format, ...) __printflike(1, 2);
    #define kcgen_log(fmt, ...) \
            _kcgen_log("kcgen[%s:%d] I: " fmt, __func__, __LINE__, ## __VA_ARGS__)

    extern void _kcgen_terminate(const char* format, ...) __printflike(1, 2) __attribute__((noreturn));
    #define kcgen_terminate(fmt,...) \
            _kcgen_terminate("kcgen[%s:%d] ERROR: " fmt, __func__, __LINE__, ## __VA_ARGS__)


    /*
     * kcgen_adjustKextSegmentLocations: main entry point for kcgen to
     * adjust the segment layout of a kext.
     *
     * This function is intended to be called once per kext.  All the input arrays
     * should be of equal length (numSegs elements), and a given index in each
     * array refers to the same dylib.
     *
     * Parameters:
     *	cputype,cpusubtype: architecture for the kernel/kernel cache
     *	                    e.g. CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_V8
     *	                    (see: mach/machine.h)
     *
     *	pkbuffer: Pointer to the base of a memory region containing a completely
     *	          laid out prelinked kernel including all kexts.
     *
     *	numSegs: Number of segments to be adjusted
     *
     *	segCacheFileOffsets: The offset(s) of the kext's segement(s) relative to
     *	                     'pkbuffer'. The first element in this array
     *	                     should point to the kext's MachO header.
     *	                     This is an array of numSegs elements.
     *
     *	segCacheFileSizes: The size(s) of the kext's segment(s). This is an
     *	                   array of numSegs elements.
     *
     *	segNewStartAddresses: The desired virtual addresses for each segment
     *	                      in the kext. This is an array of numSegs elements.
     *
     *	pointersForASLR: [OUTPUT] returns an array of pointers into pkbuffer
     *	                 that need to be added to the slideInfo for ASLR. This
     *	                 array is dynamically allocated, and must be free'd by
     *	                 the caller. The array will contain numPointers elements.
     *
     *	numPointers: [OUTPUT] The number of uintptr_t elements in pointersForASLR
     *
     * Return:
     *	 0 == Success
     *	<0 == Failure
     */
    extern int kcgen_adjustKextSegmentLocations(int cputype, int cpu_subtype,
                                                uint8_t          *pkbuffer,
                                                int               numSegs,
                                                const uint64_t   *segCacheFileOffsets,
                                                const uint64_t   *segCacheFileSizes,
                                                const uint64_t   *segOrigStartAddresses,
                                                const uint64_t   *segNewStartAddresses,
                                                uintptr_t       **pointersForASLR,
                                                size_t           *numPointers);
#if defined(__cplusplus)
} /* extern "C" */

/* C++ interface mirroring the C interface */
namespace kcgen {
    int adjustKextSegmentLocations(ArchPair arch, uint8_t *pkbuffer,
                                   const std::vector<uint64_t> &segCacheFileOffsets,
                                   const std::vector<uint64_t> &segCacheFileSizes,
                                   const std::vector<uint64_t> &segOrigStartAddresses,
                                   const std::vector<uint64_t> &segNewStartAddresses,
                                   std::vector<void*> &pointersForASLR);
}
#endif

#endif /* SPLIT_KEXT_H */
