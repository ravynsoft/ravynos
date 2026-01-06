/*
 * Copyright (c) 2007-2009 Apple Inc.  All Rights Reserved.
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

// objcimageinfo.cpp
// Print or edit ObjC image info bits.
// This is used to verify ld's handling of these bits
// for values that are not emitted by current compilers.

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>

#include "MachOFileAbstraction.hpp"

#if __BIG_ENDIAN__
    typedef BigEndian CurrentEndian;
    typedef LittleEndian OtherEndian;
#elif __LITTLE_ENDIAN__
    typedef LittleEndian CurrentEndian;
    typedef BigEndian OtherEndian;
#else
#   error unknown endianness
#endif

static const bool debug = false;

static bool processFile(const char *filename, uint32_t set, uint32_t clear);

// fixme use objc/objc-abi.h instead
struct objc_image_info {
    uint32_t version;
    uint32_t flags;

    enum : uint32_t {
        IsReplacement       = 1<<0,  // used for Fix&Continue, now ignored
        SupportsGC          = 1<<1,  // image supports GC
        RequiresGC          = 1<<2,  // image requires GC
        OptimizedByDyld     = 1<<3,  // image is from an optimized shared cache
        CorrectedSynthesize = 1<<4,  // used for an old workaround, now ignored
        IsSimulated         = 1<<5,  // image compiled for a simulator platform
        HasCategoryClassProperties  = 1<<6,  // class properties in category_t
       
        SwiftVersionMask    = 0xff << 8  // Swift ABI version
    };
};

// objc_image_info flags and their names
static const struct {
    const char *name;
    uint32_t value;
} Flags[] = {
    { "supports-gc", objc_image_info::SupportsGC }, 
    { "requires-gc", objc_image_info::RequiresGC }, 
    { "has-category-class-properties", objc_image_info::HasCategoryClassProperties }, 
    { 0, 0 }
};

static void usage(const char *self)
{
    printf("usage: %s [+FLAG|-FLAG ...] file ...\n", self);
    printf("Use +FLAG to set and -FLAG to clear.\n");
    printf("Known flags: ");
    for (int i = 0; Flags[i].name != 0; i++) {
        printf("%s ", Flags[i].name);
    }
    printf("\n");
}

static uint32_t flagForName(const char *name)
{
    for (int i = 0; Flags[i].name != 0; i++) {
        if (0 == strcmp(Flags[i].name, name)) {
            return Flags[i].value;
        }
    }
    return 0;
}

static const char *nameForFlag(uint32_t flag)
{
    for (int i = 0; Flags[i].name != 0; i++) {
        if (Flags[i].value == flag) {
            return Flags[i].name;
        }
    }
    return 0;
}

static void printFlags(uint32_t flags)
{
    printf("0x%x", flags);

    // Print flags and unknown bits
    for (int i = 0; i < 24; i++) {
        uint32_t flag = 1<<i;
        if (flag & objc_image_info::SwiftVersionMask) continue;
        if (flags & flag) {
            const char *name = nameForFlag(flag);
            if (name) printf(" %s", name);
            else printf(" unknown-%u", flag);
        }
    }

    // Print Swift version
    uint32_t mask = objc_image_info::SwiftVersionMask;
    uint32_t shift = __builtin_ctzl(mask);
    uint32_t swift = (flags & mask) >> shift;
    if (swift > 0) {
        printf(" swift-version=%u", swift);
    }
}

static bool isFlagArgument(const char *arg)
{
    return (arg  &&  (arg[0] == '+' || arg[0] == '-'));
}

int main(int argc, const char *argv[]) {
    uint32_t set = 0;
    uint32_t clear = 0;

    // Find flag arguments (which are +FLAG or -FLAG).
    int i;
    for (i = 1; i < argc && isFlagArgument(argv[i]); i++) {
        const char *arg = argv[i];
        uint32_t flag = flagForName(arg+1);
        if (flag) {
            if (arg[0] == '+') {
                set |= flag;
            } else {
                clear |= flag;
            }
        } else {
            printf("error: unrecognized ObjC flag '%s'\n", arg);
            usage(argv[0]);
            return 1;
        }
    }

    // Complain if +FLAG and -FLAG are both set for some flag.
    uint32_t overlap = set & clear;
    if (overlap) {
        printf("error: conflicting changes specified: ");
        printFlags(overlap);
        printf("\n");
        usage(argv[0]);
        return 1;
    }

    // Complain if there are no filenames.
    if (i == argc) {
        printf("error: no files specified\n");
        usage(argv[0]);
        return 1;
    }

    // Process files.
    for (; i < argc; i++) {
        if (!processFile(argv[i], set, clear)) return 1;
    }
    return 0;
}


// Segment and section names are 16 bytes and may be un-terminated.
static bool segnameEquals(const char *lhs, const char *rhs)
{
    return 0 == strncmp(lhs, rhs, 16);
}

static bool segnameStartsWith(const char *segname, const char *prefix)
{
    return 0 == strncmp(segname, prefix, strlen(prefix));
}

static bool sectnameEquals(const char *lhs, const char *rhs)
{
    return segnameEquals(lhs, rhs);
}


template <typename P>
static void dosect(const char *filename, uint8_t *start, macho_section<P> *sect,
                   uint32_t set, uint32_t clear)
{
    if (debug) printf("section %.16s from segment %.16s\n",
                      sect->sectname(), sect->segname());

    if ((segnameStartsWith(sect->segname(),  "__DATA")  &&
         sectnameEquals(sect->sectname(), "__objc_imageinfo"))  ||
        (segnameEquals(sect->segname(),  "__OBJC")  &&
         sectnameEquals(sect->sectname(), "__image_info")))
    {
        objc_image_info *ii = (objc_image_info *)(start + sect->offset());
        uint32_t oldFlags = P::E::get32(ii->flags);
        uint32_t newFlags = (oldFlags | set) & ~clear;
        if (oldFlags != newFlags) {
            P::E::set32(ii->flags, newFlags);
            if (debug) printf("changed flags from 0x%x to 0x%x\n", 
                              oldFlags, newFlags);
        }

        printf("%s: ", filename);
        printFlags(newFlags);
        printf("\n");
    }
}

template <typename P>
static void doseg(const char *filename, 
                  uint8_t *start, macho_segment_command<P> *seg,
                  uint32_t set, uint32_t clear)
{
    if (debug) printf("segment name: %.16s, nsects %u\n",
                      seg->segname(), seg->nsects());
    macho_section<P> *sect = (macho_section<P> *)(seg + 1);
    for (uint32_t i = 0; i < seg->nsects(); ++i) {
        dosect(filename, start, &sect[i], set, clear);
    }
}


template<typename P>
static bool parse_macho(const char *filename, uint8_t *buffer, 
                        uint32_t set, uint32_t clear)
{
    macho_header<P>* mh = (macho_header<P>*)buffer;
    uint8_t *cmds = (uint8_t *)(mh + 1);
    for (uint32_t c = 0; c < mh->ncmds(); c++) {
        macho_load_command<P>* cmd = (macho_load_command<P>*)cmds;
        cmds += cmd->cmdsize();
        if (cmd->cmd() == LC_SEGMENT  ||  cmd->cmd() == LC_SEGMENT_64) {
            doseg(filename, buffer, (macho_segment_command<P>*)cmd, set, clear);
        }
    }

    return true;
}


static bool parse_macho(const char *filename, uint8_t *buffer, 
                        uint32_t set, uint32_t clear)
{
    uint32_t magic = *(uint32_t *)buffer;

    switch (magic) {
    case MH_MAGIC_64:
        return parse_macho<Pointer64<CurrentEndian>>
            (filename, buffer, set, clear);
    case MH_MAGIC:
        return parse_macho<Pointer32<CurrentEndian>>
            (filename, buffer, set, clear);
    case MH_CIGAM_64:
        return parse_macho<Pointer64<OtherEndian>>
            (filename, buffer, set, clear);
    case MH_CIGAM:
        return parse_macho<Pointer32<OtherEndian>>
            (filename, buffer, set, clear);
    default:
        printf("error: file '%s' is not mach-o (magic %x)\n", filename, magic);
        return false;
    }
}


static bool parse_fat(const char *filename, uint8_t *buffer, size_t size, 
                      uint32_t set, uint32_t clear)
{
    uint32_t magic;

    if (size < sizeof(magic)) {
        printf("error: file '%s' is too small\n", filename);
        return false;
    }

    magic = *(uint32_t *)buffer;
    if (magic != FAT_MAGIC && magic != FAT_CIGAM) {
        /* Not a fat file */
        return parse_macho(filename, buffer, set, clear);
    } else {
        struct fat_header *fh;
        uint32_t fat_magic, fat_nfat_arch;
        struct fat_arch *archs;
        
        if (size < sizeof(struct fat_header)) {
            printf("error: file '%s' is too small\n", filename);
            return false;
        }

        fh = (struct fat_header *)buffer;
        fat_magic = OSSwapBigToHostInt32(fh->magic);
        fat_nfat_arch = OSSwapBigToHostInt32(fh->nfat_arch);

        if (size < (sizeof(struct fat_header) + fat_nfat_arch * sizeof(struct fat_arch))) {
            printf("error: file '%s' is too small\n", filename);
            return false;
        }

        archs = (struct fat_arch *)(buffer + sizeof(struct fat_header));

        /* Special case hidden CPU_TYPE_ARM64 */
        if (size >= (sizeof(struct fat_header) + (fat_nfat_arch + 1) * sizeof(struct fat_arch))) {
            if (fat_nfat_arch > 0
                && OSSwapBigToHostInt32(archs[fat_nfat_arch].cputype) == CPU_TYPE_ARM64) {
                fat_nfat_arch++;
            }
        }
        /* End special case hidden CPU_TYPE_ARM64 */

        if (debug) printf("%d fat architectures\n", fat_nfat_arch);

        for (uint32_t i = 0; i < fat_nfat_arch; i++) {
            uint32_t arch_cputype = OSSwapBigToHostInt32(archs[i].cputype);
            uint32_t arch_cpusubtype = OSSwapBigToHostInt32(archs[i].cpusubtype);
            uint32_t arch_offset = OSSwapBigToHostInt32(archs[i].offset);
            uint32_t arch_size = OSSwapBigToHostInt32(archs[i].size);

            if (debug) printf("cputype %d cpusubtype %d\n", 
                              arch_cputype, arch_cpusubtype);

            /* Check that slice data is after all fat headers and archs */
            if (arch_offset < (sizeof(struct fat_header) + fat_nfat_arch * sizeof(struct fat_arch))) {
                printf("error: file is badly formed\n");
                return false;
            }

            /* Check that the slice ends before the file does */
            if (arch_offset > size) {
                printf("error: file '%s' is badly formed\n", filename);
                return false;
            }

            if (arch_size > size) {
                printf("error: file '%s' is badly formed\n", filename);
                return false;
            }

            if (arch_offset > (size - arch_size)) {
                printf("error: file '%s' is badly formed\n", filename);
                return false;
            }

            bool ok = parse_macho(filename, buffer + arch_offset, set, clear);
            if (!ok) return false;
        }
        return true;
    }
}

static bool processFile(const char *filename, uint32_t set, uint32_t clear)
{
    if (debug) printf("file %s\n", filename);
    int openPerm = O_RDONLY;
    int mmapPerm = PROT_READ;
    if (set || clear) {
        openPerm = O_RDWR;
        mmapPerm = PROT_READ | PROT_WRITE;
    }

    int fd = open(filename, openPerm);
    if (fd < 0) {
        printf("error: open %s: %s\n", filename, strerror(errno));
        return false;
    }
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        printf("error: fstat %s: %s\n", filename, strerror(errno));
        return false;
    }

    void *buffer = mmap(NULL, (size_t)st.st_size, mmapPerm, 
                        MAP_FILE|MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        printf("error: mmap %s: %s\n", filename, strerror(errno));
        return false;
    }

    bool result = 
        parse_fat(filename, (uint8_t *)buffer, (size_t)st.st_size, set, clear);
    munmap(buffer, (size_t)st.st_size);
    close(fd);
    return result;
}
