/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2005-2010 Apple Inc. All rights reserved.
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
#ifndef __MACH_O_FILE_ABSTRACTION__
#define __MACH_O_FILE_ABSTRACTION__

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/fat.h>
#include <mach-o/stab.h>
#include <mach-o/reloc.h>
#include <mach-o/x86_64/reloc.h>
#include <mach-o/compact_unwind_encoding.h>
#include <mach/machine.h>
#include <stddef.h>
#include <libunwind.h>

#include "FileAbstraction.hpp"

#include "configure.h"



//#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e || SUPPORT_ARCH_arm64_32
  #include <mach-o/arm64/reloc.h>
///#endif

#ifndef BIND_SPECIAL_DYLIB_WEAK_LOOKUP
#define BIND_SPECIAL_DYLIB_WEAK_LOOKUP				-3
#endif

#ifndef BIND_OPCODE_THREADED
#define BIND_OPCODE_THREADED	0xD0
#endif

#ifndef BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB
#define BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB	0x00
#endif

#ifndef BIND_SUBOPCODE_THREADED_APPLY
#define BIND_SUBOPCODE_THREADED_APPLY								0x01
#endif

#ifndef EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE
	#define EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE			0x02
#endif
#ifndef MH_SIM_SUPPORT
	#define MH_SIM_SUPPORT 0x08000000
#endif
#ifndef PLATFORM_MACCATALYST
	#define PLATFORM_MACCATALYST 6
#endif
#ifndef SG_READ_ONLY
	#define SG_READ_ONLY    0x10
#endif

#ifndef N_COLD_FUNC
	#define N_COLD_FUNC 0x0400
#endif

#if __has_include(<mach-o/fixup-chains.h>)
  #include <mach-o/fixup-chains.h>
#else
  #define __MACH_O_FIXUP_CHAINS__

	// header of the LC_DYLD_CHAINED_FIXUPS payload
	struct dyld_chained_fixups_header
	{
		uint32_t    fixups_version;    // 0
		uint32_t    starts_offset;     // offset of dyld_chained_starts_in_image in chain_data
		uint32_t    imports_offset;    // offset of imports table in chain_data
		uint32_t    symbols_offset;    // offset of symbol strings in chain_data
		uint32_t    imports_count;     // number of imported symbol names
		uint32_t    imports_format;    // DYLD_CHAINED_IMPORT*
		uint32_t    symbols_format;    // 0 => uncompressed, 1 => zlib compressed
	};

	// This struct is embedded in LC_DYLD_CHAINED_FIXUPS payload
	struct dyld_chained_starts_in_image
	{
		uint32_t    seg_count;
		uint32_t    seg_info_offset[1];  // each entry is offset into this struct for that segment
		// followed by pool of dyld_chain_starts_in_segment data
	};

	// This struct is embedded in dyld_chain_starts_in_image
	// and passed down to the kernel for page-in linking
	struct dyld_chained_starts_in_segment
	{
		uint32_t    size;               // size of this (amount kernel needs to copy)
		uint16_t    page_size;          // 0x1000 or 0x4000
		uint16_t    pointer_format;     // DYLD_CHAINED_PTR_*
		uint64_t    segment_offset;     // offset in memory to start of segment
		uint32_t    max_valid_pointer;  // for 32-bit OS, any value beyond this is not a pointer
		uint16_t    page_count;         // how many pages are in array
		uint16_t    page_start[1];      // each entry is offset in each page of first element in chain
										// or DYLD_CHAINED_PTR_START_NONE if no fixups on page
	 // uint16_t    chain_starts[1];    // some 32-bit formats may require multiple starts per page.
										// for those, if high bit is set in page_starts[], then it
										// is index into chain_starts[] which is a list of starts
										// the last of which has the high bit set
	};

	enum {
		DYLD_CHAINED_PTR_START_NONE   = 0xFFFF, // used in page_start[] to denote a page with no fixups
		DYLD_CHAINED_PTR_START_MULTI  = 0x8000, // used in page_start[] to denote a page which has multiple starts
		DYLD_CHAINED_PTR_START_LAST   = 0x8000, // used in chain_starts[] to denote last start in list for page
	};


	// This struct is embedded in __TEXT,__chain_starts section in firmware
	struct dyld_chained_starts_offsets
	{
		uint32_t    pointer_format;     // DYLD_CHAINED_PTR_32_FIRMWARE
		uint32_t    starts_count;       // number of starts in array
		uint32_t    chain_starts[1];    // array chain start offsets
	};


	// values for dyld_chained_starts_in_segment.pointer_format
	enum {
		DYLD_CHAINED_PTR_ARM64E      	= 1,
		DYLD_CHAINED_PTR_64          	= 2,
		DYLD_CHAINED_PTR_32          	= 3,
		DYLD_CHAINED_PTR_32_CACHE    	= 4,
		DYLD_CHAINED_PTR_32_FIRMWARE 	= 5,
	};

	// DYLD_CHAINED_PTR_ARM64E
	struct dyld_chained_ptr_arm64e_rebase
	{
		uint64_t    target   : 43,    // (DYLD_CHAINED_PTR_ARM64E => vmAddr, DYLD_CHAINED_PTR_ARM64E_OFFSET => runtimeOffset)
					high8    :  8,
					next     : 11,    // 8-byte stide
					bind     :  1,    // == 0
					auth     :  1;    // == 0
	};

	// DYLD_CHAINED_PTR_ARM64E
	struct dyld_chained_ptr_arm64e_bind
	{
		uint64_t    ordinal   : 16,
					zero      : 16,
					addend    : 19,
					next      : 11,    // 8-byte stide
					bind      :  1,    // == 1
					auth      :  1;    // == 0
	};

	// DYLD_CHAINED_PTR_ARM64E
	struct dyld_chained_ptr_arm64e_auth_rebase
	{
		uint64_t    target    : 32,	   // runtimeOffset
					diversity : 16,
					addrDiv   :  1,
					key       :  2,
					next      : 11,    // 8-byte stide
					bind      :  1,    // == 0
					auth      :  1;    // == 1
	};

	// DYLD_CHAINED_PTR_ARM64E
	struct dyld_chained_ptr_arm64e_auth_bind
	{
		uint64_t    ordinal   : 16,
					zero      : 16,
					diversity : 16,
					addrDiv   :  1,
					key       :  2,
					next      : 11,    // 8-byte stide
					bind      :  1,    // == 1
					auth      :  1;    // == 1
	};

	// DYLD_CHAINED_PTR_64/DYLD_CHAINED_PTR_64_OFFSET
	struct dyld_chained_ptr_64_rebase
	{
		uint64_t    target    : 36,    // 64GB max image size (DYLD_CHAINED_PTR_64 => vmAddr, DYLD_CHAINED_PTR_64_OFFSET => runtimeOffset)
					high8     :  8,    // top 8 bits set to this (DYLD_CHAINED_PTR_64 => after slide added, DYLD_CHAINED_PTR_64_OFFSET => before slide added)
					reserved  :  7,    // all zeros
					next      : 12,    // 4-byte stride
					bind      :  1;    // == 0
	};

	// DYLD_CHAINED_PTR_64
	struct dyld_chained_ptr_64_bind
	{
		uint64_t    ordinal   : 24,
					addend    :  8,   // 0 thru 255
					reserved  : 19,   // all zeros
					next      : 12,   // 4-byte stride
					bind      :  1;   // == 1
	};

	// DYLD_CHAINED_PTR_32
	struct dyld_chained_ptr_32_rebase
	{
		uint32_t    target    : 26,   // 64MB max image size
					next      :  5,   // 4-byte stride
					bind      :  1;   // == 0
	};

	// DYLD_CHAINED_PTR_32
	struct dyld_chained_ptr_32_bind
	{
		uint32_t    ordinal   : 20,
					addend    :  6,   // 0 thru 63
					next      :  5,   // 4-byte stride
					bind      :  1;   // == 1
	};

	// DYLD_CHAINED_PTR_32_CACHE
	struct dyld_chained_ptr_32_cache_rebase
	{
		uint32_t    target    : 30,   // 1GB max dyld cache TEXT and DATA
					next      :  2;   // 4-byte stride
	};


	// DYLD_CHAINED_PTR_32_FIRMWARE
	struct dyld_chained_ptr_32_firmware_rebase
	{
		uint32_t    target   : 26,   // 64MB max firmware TEXT and DATA
					next     :  6;   // 4-byte stride
	};



	// values for dyld_chained_fixups_header.imports_format
	enum {
		DYLD_CHAINED_IMPORT          = 1,
		DYLD_CHAINED_IMPORT_ADDEND   = 2,
		DYLD_CHAINED_IMPORT_ADDEND64 = 3,
	};

	// DYLD_CHAINED_IMPORT
	struct dyld_chained_import
	{
		uint32_t    lib_ordinal :  8,
					weak_import :  1,
					name_offset : 23;
	};

	// DYLD_CHAINED_IMPORT_ADDEND
	struct dyld_chained_import_addend
	{
		uint32_t    lib_ordinal :  8,
					weak_import :  1,
					name_offset : 23;
		int32_t     addend;
	};

	// DYLD_CHAINED_IMPORT_ADDEND64
	struct dyld_chained_import_addend64
	{
		uint64_t    lib_ordinal : 16,
					weak_import :  1,
					reserved    : 15,
					name_offset : 32;
		uint64_t    addend;
	};

#endif  // __has_include(<mach-o/fixup-chains.h>)

#if ((__MACH_O_FIXUP_CHAINS__ - 0) < 2)
// new fixup-chains.h content for version 2
enum {
    DYLD_CHAINED_PTR_64_OFFSET      = 6,
    DYLD_CHAINED_PTR_ARM64E_OFFSET  = 7,
};
#endif

#if ((__MACH_O_FIXUP_CHAINS__ - 0) < 3)
// new fixup-chains.h content for version 3
//enum {
//    DYLD_CHAINED_PTR_64_KERNEL_CACHE    =  8,
//};
#endif

#if ((__MACH_O_FIXUP_CHAINS__ - 0) < 4)
// new fixup-chains.h content for version 4
enum {
    DYLD_CHAINED_PTR_ARM64E_KERNEL  	= DYLD_CHAINED_PTR_ARM64E_OFFSET,
    DYLD_CHAINED_PTR_ARM64E_USERLAND    =  9,    // stride 8, unauth target is vm offset
    DYLD_CHAINED_PTR_ARM64E_FIRMWARE    = 10,    // stride 4, unauth target is vmaddr
};
#endif

#if ((__MACH_O_FIXUP_CHAINS__ - 0) < 6)
// new fixup-chains.h content for version 6
enum {
    DYLD_CHAINED_PTR_ARM64E_USERLAND24  = 12,    // stride 8, unauth target is vm offset, 24-bit bind
};
// DYLD_CHAINED_PTR_ARM64E_USERLAND24
struct dyld_chained_ptr_arm64e_bind24
{
    uint64_t    ordinal   : 24,
                zero      :  8,
                addend    : 19,    // +/-256K
                next      : 11,    // 8-byte stide
                bind      :  1,    // == 1
                auth      :  1;    // == 0
};

// DYLD_CHAINED_PTR_ARM64E_USERLAND24
struct dyld_chained_ptr_arm64e_auth_bind24
{
    uint64_t    ordinal   : 24,
                zero      :  8,
                diversity : 16,
                addrDiv   :  1,
                key       :  2,
                next      : 11,    // 8-byte stide
                bind      :  1,    // == 1
                auth      :  1;    // == 1
};


#endif



#define LC_DYLD_EXPORTS_TRIE     (0x33 | LC_REQ_DYLD)
#define LC_DYLD_CHAINED_FIXUPS   (0x34 | LC_REQ_DYLD)


#define GENERIC_RLEOC_TLV  5

// type internal to linker
#define BIND_TYPE_OVERRIDE_OF_WEAKDEF_IN_DYLIB 0


#ifndef CPU_TYPE_ARM64_32
	#ifndef CPU_ARCH_ABI64_32
		#define CPU_ARCH_ABI64_32			0x02000000
	#endif
	#define CPU_TYPE_ARM64_32			(CPU_TYPE_ARM | CPU_ARCH_ABI64_32)
#endif
#ifndef CPU_SUBTYPE_ARM64_32_V8
	#define CPU_SUBTYPE_ARM64_32_V8    1
#endif
#ifndef CPU_SUBTYPE_ARM64E
	#define CPU_SUBTYPE_ARM64E    2
	#define ARM64_RELOC_AUTHENTICATED_POINTER 11
#endif


#define UNW_ARM64_X0     0
#define UNW_ARM64_X1     1
#define UNW_ARM64_X2     2
#define UNW_ARM64_X3     3
#define UNW_ARM64_X4     4
#define UNW_ARM64_X5     5
#define UNW_ARM64_X6     6
#define UNW_ARM64_X7     7
#define UNW_ARM64_X8     8
#define UNW_ARM64_X9     9
#define UNW_ARM64_X10   10
#define UNW_ARM64_X11   11
#define UNW_ARM64_X12   12
#define UNW_ARM64_X13   13
#define UNW_ARM64_X14   14
#define UNW_ARM64_X15   15
#define UNW_ARM64_X16   16
#define UNW_ARM64_X17   17
#define UNW_ARM64_X18   18
#define UNW_ARM64_X19   19
#define UNW_ARM64_X20   20
#define UNW_ARM64_X21   21
#define UNW_ARM64_X22   22
#define UNW_ARM64_X23   23
#define UNW_ARM64_X24   24
#define UNW_ARM64_X25   25
#define UNW_ARM64_X26   26
#define UNW_ARM64_X27   27
#define UNW_ARM64_X28   28
#define UNW_ARM64_X29   29  
#define UNW_ARM64_FP    29
#define UNW_ARM64_X30   30  
#define UNW_ARM64_LR    30
#define UNW_ARM64_X31   31  
#define UNW_ARM64_SP    31
#define UNW_ARM64_D0    64
#define UNW_ARM64_D1    65
#define UNW_ARM64_D2    66
#define UNW_ARM64_D3    67
#define UNW_ARM64_D4    68
#define UNW_ARM64_D5    69
#define UNW_ARM64_D6    70
#define UNW_ARM64_D7    71
#define UNW_ARM64_D8    72
#define UNW_ARM64_D9    73
#define UNW_ARM64_D10   74
#define UNW_ARM64_D11   75
#define UNW_ARM64_D12   76
#define UNW_ARM64_D13   77
#define UNW_ARM64_D14   78
#define UNW_ARM64_D15   79
#define UNW_ARM64_D16   80
#define UNW_ARM64_D17   81
#define UNW_ARM64_D18   82
#define UNW_ARM64_D19   83
#define UNW_ARM64_D20   84
#define UNW_ARM64_D21   85
#define UNW_ARM64_D22   86
#define UNW_ARM64_D23   87
#define UNW_ARM64_D24   88
#define UNW_ARM64_D25   89
#define UNW_ARM64_D26   90
#define UNW_ARM64_D27   91
#define UNW_ARM64_D28   92
#define UNW_ARM64_D29   93
#define UNW_ARM64_D30   94
#define UNW_ARM64_D31   95

#define UNWIND_ARM64_MODE_MASK                          0x0F000000
#define UNWIND_ARM64_MODE_FRAME_OLD                     0x01000000
#define UNWIND_ARM64_MODE_FRAMELESS                     0x02000000
#define UNWIND_ARM64_MODE_DWARF                         0x03000000
#define UNWIND_ARM64_MODE_FRAME                         0x04000000
    
#define UNWIND_ARM64_FRAME_X19_X20_PAIR                 0x00000001
#define UNWIND_ARM64_FRAME_X21_X22_PAIR                 0x00000002
#define UNWIND_ARM64_FRAME_X23_X24_PAIR                 0x00000004
#define UNWIND_ARM64_FRAME_X25_X26_PAIR                 0x00000008
#define UNWIND_ARM64_FRAME_X27_X28_PAIR                 0x00000010
#define UNWIND_ARM64_FRAME_D8_D9_PAIR                   0x00000100
#define UNWIND_ARM64_FRAME_D10_D11_PAIR                 0x00000200
#define UNWIND_ARM64_FRAME_D12_D13_PAIR                 0x00000400
#define UNWIND_ARM64_FRAME_D14_D15_PAIR                 0x00000800

#define UNWIND_ARM64_FRAMELESS_STACK_SIZE_MASK			0x00FFF000

#define UNWIND_ARM64_FRAME_X21_X22_PAIR_OLD                 0x00000001
#define UNWIND_ARM64_FRAME_X23_X24_PAIR_OLD                 0x00000002
#define UNWIND_ARM64_FRAME_X25_X26_PAIR_OLD                 0x00000004
#define UNWIND_ARM64_FRAME_X27_X28_PAIR_OLD                 0x00000008
#define UNWIND_ARM64_FRAME_D8_D9_PAIR_OLD                   0x00000010
#define UNWIND_ARM64_FRAME_D10_D11_PAIR_OLD                 0x00000020
#define UNWIND_ARM64_FRAME_D12_D13_PAIR_OLD                 0x00000040
#define UNWIND_ARM64_FRAME_D14_D15_PAIR_OLD                 0x00000080


#define UNWIND_ARM64_DWARF_SECTION_OFFSET               0x00FFFFFF

#define UNW_ARM_D31 287

#ifndef LC_LINKER_OPTIMIZATION_HINTS
       #define LC_LINKER_OPTIMIZATION_HINTS   0x2E
       #define LOH_ARM64_ADRP_ADRP                             1
       #define LOH_ARM64_ADRP_LDR                              2
       #define LOH_ARM64_ADRP_ADD_LDR                  3
       #define LOH_ARM64_ADRP_LDR_GOT_LDR              4
       #define LOH_ARM64_ADRP_ADD_STR                  5
       #define LOH_ARM64_ADRP_LDR_GOT_STR              6
       #define LOH_ARM64_ADRP_ADD                              7
       #define LOH_ARM64_ADRP_LDR_GOT                  8
#endif

#define UNWIND_ARM_MODE_MASK                          0x0F000000
#define UNWIND_ARM_MODE_FRAME                         0x01000000
#define UNWIND_ARM_MODE_FRAME_D                       0x02000000
#define UNWIND_ARM_MODE_DWARF                         0x04000000
 
#define  UNWIND_ARM_FRAME_STACK_ADJUST_MASK           0x00C00000

#define UNWIND_ARM_FRAME_FIRST_PUSH_R4                0x00000001
#define UNWIND_ARM_FRAME_FIRST_PUSH_R5                0x00000002
#define UNWIND_ARM_FRAME_FIRST_PUSH_R6                0x00000004
  
#define UNWIND_ARM_FRAME_SECOND_PUSH_R8               0x00000008
#define UNWIND_ARM_FRAME_SECOND_PUSH_R9               0x00000010
#define UNWIND_ARM_FRAME_SECOND_PUSH_R10              0x00000020
#define UNWIND_ARM_FRAME_SECOND_PUSH_R11              0x00000040
#define UNWIND_ARM_FRAME_SECOND_PUSH_R12              0x00000080
 
#define UNWIND_ARM_FRAME_D_REG_COUNT_MASK             0x00000F00
 
#define UNWIND_ARM_DWARF_SECTION_OFFSET               0x00FFFFFF


// ( <opcode> (delta-uleb128)+ <zero> )+ <zero>
#define DYLD_CACHE_ADJ_V1_POINTER_32		0x01
#define DYLD_CACHE_ADJ_V1_POINTER_64		0x02
#define DYLD_CACHE_ADJ_V1_ADRP				0x03
#define DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT	0x10 // thru 0x1F
#define DYLD_CACHE_ADJ_V1_ARM_MOVT			0x20 // thru 0x2F


// Whole		 :== <new-marker> <count> FromToSection+
// FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
// ToOffset		 :== <to-sect-offset-delta> <count> FromOffset+
// FromOffset	 :== <kind> <count> <from-sect-offset-delta>
#define DYLD_CACHE_ADJ_V2_FORMAT				0x7F

#define DYLD_CACHE_ADJ_V2_POINTER_32			0x01
#define DYLD_CACHE_ADJ_V2_POINTER_64			0x02
#define DYLD_CACHE_ADJ_V2_DELTA_32			    0x03
#define DYLD_CACHE_ADJ_V2_DELTA_64			    0x04
#define DYLD_CACHE_ADJ_V2_ARM64_ADRP			0x05
#define DYLD_CACHE_ADJ_V2_ARM64_OFF12			0x06
#define DYLD_CACHE_ADJ_V2_ARM64_BR26			0x07
#define DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT			0x08
#define DYLD_CACHE_ADJ_V2_ARM_BR24				0x09
#define DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT		0x0A
#define DYLD_CACHE_ADJ_V2_THUMB_BR22			0x0B
#define DYLD_CACHE_ADJ_V2_IMAGE_OFF_32			0x0C
#define DYLD_CACHE_ADJ_V2_THREADED_POINTER_64			0x0D

#ifndef S_INIT_FUNC_OFFSETS
	#define S_INIT_FUNC_OFFSETS                 0x16
#endif



// kind target-address fixup-addr [adj] 



struct ArchInfo {
	const char*			archName;
	cpu_type_t			cpuType;
	cpu_subtype_t		cpuSubType;
	const char*			llvmTriplePrefix;
	const char*			llvmTriplePrefixAlt;
	bool				isSubType;
	bool				supportsThumb2;
};

static const ArchInfo archInfoArray[] = {
#if SUPPORT_ARCH_x86_64
	{ "x86_64", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL, "x86_64-",  "", true, false },
#endif
#if SUPPORT_ARCH_x86_64h
	{ "x86_64h", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H,	 "x86_64h-",  "", true, false },
#endif
#if SUPPORT_ARCH_i386
	{ "i386",   CPU_TYPE_I386,   CPU_SUBTYPE_I386_ALL,   "i386-",    "", false, false },
#endif
#if SUPPORT_ARCH_armv4t
	{ "armv4t", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V4T,    "armv4t-",  "", true,  false },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv5
	{ "armv5", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V5TEJ,  "armv5e-",  "", true,  false },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv6
	{ "armv6", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6,     "armv6-",   "", true,  false },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv7
	{ "armv7", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7,     "thumbv7-", "armv7-", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv7f
	{ "armv7f", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V7F,    "thumbv7f-", "", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif 
#if SUPPORT_ARCH_armv7k
	{ "armv7k", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V7K,    "thumbv7k-", "", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv7s
	{ "armv7s", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V7S,    "thumbv7s-", "armv7s", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv6m
	{ "armv6m", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V6M,    "thumbv6m-", "", true,  false },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv7m
	{ "armv7m", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V7M,    "thumbv7m-", "armv7m", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv7em
	{ "armv7em", CPU_TYPE_ARM,   CPU_SUBTYPE_ARM_V7EM,   "thumbv7em-", "armv7em", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_armv8
	{ "armv8", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V8,     "thumbv8-", "armv8", true,  true },
	#define SUPPORT_ARCH_arm_any 1
#endif
#if SUPPORT_ARCH_arm64
	{ "arm64", CPU_TYPE_ARM64,   CPU_SUBTYPE_ARM64_ALL,  "arm64-",  "aarch64-",  true,  false },
#endif
#if SUPPORT_ARCH_arm64e
	{ "arm64e", CPU_TYPE_ARM64,   CPU_SUBTYPE_ARM64E,    "arm64e-",  "aarch64e-",  true,  false },
#endif
#if SUPPORT_ARCH_arm64v8
	{ "arm64v8", CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_V8,   "arm64v8-",  "aarch64-",   true,  false },
#endif
#if SUPPORT_ARCH_arm64_32
	{ "arm64_32", CPU_TYPE_ARM64_32,   CPU_SUBTYPE_ARM64_32_V8,  "arm64_32-",  "aarch64_32-",  true,  false },
#endif
	{ NULL, 0, 0, NULL, NULL, false, false }
};



// weird, but this include must wait until after SUPPORT_ARCH_arm_any is set up
#if SUPPORT_ARCH_arm_any
#include <mach-o/arm/reloc.h>
#endif

// hack until newer <mach-o/arm/reloc.h> everywhere
#define ARM_RELOC_HALF 8
#define ARM_RELOC_HALF_SECTDIFF 9



//
// This abstraction layer makes every mach-o file look like a 64-bit mach-o file with native endianness
//



//
// mach-o file header
//
template <typename P> struct macho_header_content {};
template <> struct macho_header_content<Pointer32<BigEndian> >    { mach_header		fields; };
template <> struct macho_header_content<Pointer64<BigEndian> >	  { mach_header_64	fields; };
template <> struct macho_header_content<Pointer32<LittleEndian> > { mach_header		fields; };
template <> struct macho_header_content<Pointer64<LittleEndian> > { mach_header_64	fields; };

template <typename P>
class macho_header {
public:
	uint32_t		magic() const					INLINE { return E::get32(header.fields.magic); }
	void			set_magic(uint32_t value)		INLINE { E::set32(header.fields.magic, value); }

	uint32_t		cputype() const					INLINE { return E::get32(header.fields.cputype); }
	void			set_cputype(uint32_t value)		INLINE { E::set32((uint32_t&)header.fields.cputype, value); }

	uint32_t		cpusubtype() const				INLINE { return (E::get32(header.fields.cpusubtype) & ~CPU_SUBTYPE_MASK); }
	void			set_cpusubtype(uint32_t value)	INLINE { E::set32((uint32_t&)header.fields.cpusubtype, (value & ~CPU_SUBTYPE_MASK) | (cpusubtypeflags() << 24)); }

	uint8_t			cpusubtypeflags() const			INLINE { return ((E::get32(header.fields.cpusubtype) & CPU_SUBTYPE_MASK) >> 24); }
	void			set_cpusubtypeflags(uint8_t value)	INLINE { E::set32((uint32_t&)header.fields.cpusubtype, cpusubtype() | (((uint32_t)value) << 24)); }

	uint32_t		filetype() const				INLINE { return E::get32(header.fields.filetype); }
	void			set_filetype(uint32_t value)	INLINE { E::set32(header.fields.filetype, value); }

	uint32_t		ncmds() const					INLINE { return E::get32(header.fields.ncmds); }
	void			set_ncmds(uint32_t value)		INLINE { E::set32(header.fields.ncmds, value); }

	uint32_t		sizeofcmds() const				INLINE { return E::get32(header.fields.sizeofcmds); }
	void			set_sizeofcmds(uint32_t value)	INLINE { E::set32(header.fields.sizeofcmds, value); }

	uint32_t		flags() const					INLINE { return E::get32(header.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(header.fields.flags, value); }

	uint32_t		reserved() const				INLINE { return E::get32(header.fields.reserved); }
	void			set_reserved(uint32_t value)	INLINE { E::set32(header.fields.reserved, value); }

	typedef typename P::E		E;
private:
	macho_header_content<P>	header;
};


//
// mach-o load command
//
template <typename P>
class macho_load_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(command.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(command.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(command.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(command.cmdsize, value); }

	typedef typename P::E		E;
private:
	load_command	command;
};


//
// mach-o segment load command
//
template <typename P> struct macho_segment_content {};
template <> struct macho_segment_content<Pointer32<BigEndian> >    { segment_command	fields; enum { CMD = LC_SEGMENT		}; };
template <> struct macho_segment_content<Pointer64<BigEndian> >	   { segment_command_64	fields; enum { CMD = LC_SEGMENT_64	}; };
template <> struct macho_segment_content<Pointer32<LittleEndian> > { segment_command	fields; enum { CMD = LC_SEGMENT		}; };
template <> struct macho_segment_content<Pointer64<LittleEndian> > { segment_command_64	fields; enum { CMD = LC_SEGMENT_64	}; };

template <typename P>
class macho_segment_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(segment.fields.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(segment.fields.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(segment.fields.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(segment.fields.cmdsize, value); }

	const char*		segname() const					INLINE { return segment.fields.segname; }
	void			set_segname(const char* value)	INLINE { strncpy(segment.fields.segname, value, 16); }
	
	uint64_t		vmaddr() const					INLINE { return P::getP(segment.fields.vmaddr); }
	void			set_vmaddr(uint64_t value)		INLINE { P::setP(segment.fields.vmaddr, value); }

	uint64_t		vmsize() const					INLINE { return P::getP(segment.fields.vmsize); }
	void			set_vmsize(uint64_t value)		INLINE { P::setP(segment.fields.vmsize, value); }

	uint64_t		fileoff() const					INLINE { return P::getP(segment.fields.fileoff); }
	void			set_fileoff(uint64_t value)		INLINE { P::setP(segment.fields.fileoff, value); }

	uint64_t		filesize() const				INLINE { return P::getP(segment.fields.filesize); }
	void			set_filesize(uint64_t value)	INLINE { P::setP(segment.fields.filesize, value); }

	uint32_t		maxprot() const					INLINE { return E::get32(segment.fields.maxprot); }
	void			set_maxprot(uint32_t value)		INLINE { E::set32((uint32_t&)segment.fields.maxprot, value); }

	uint32_t		initprot() const				INLINE { return E::get32(segment.fields.initprot); }
	void			set_initprot(uint32_t value)	INLINE { E::set32((uint32_t&)segment.fields.initprot, value); }

	uint32_t		nsects() const					INLINE { return E::get32(segment.fields.nsects); }
	void			set_nsects(uint32_t value)		INLINE { E::set32(segment.fields.nsects, value); }

	uint32_t		flags() const					INLINE { return E::get32(segment.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(segment.fields.flags, value); }

	enum {
		CMD = macho_segment_content<P>::CMD
	};

	typedef typename P::E		E;
private:
	macho_segment_content<P>	segment;
};


//
// mach-o section 
//
template <typename P> struct macho_section_content {};
template <> struct macho_section_content<Pointer32<BigEndian> >    { section	fields; };
template <> struct macho_section_content<Pointer64<BigEndian> >	   { section_64	fields; };
template <> struct macho_section_content<Pointer32<LittleEndian> > { section	fields; };
template <> struct macho_section_content<Pointer64<LittleEndian> > { section_64	fields; };

template <typename P>
class macho_section {
public:
	const char*		sectname() const				INLINE { return section.fields.sectname; }
	void			set_sectname(const char* value)	INLINE { strncpy(section.fields.sectname, value, 16); }
	
	const char*		segname() const					INLINE { return section.fields.segname; }
	void			set_segname(const char* value)	INLINE { strncpy(section.fields.segname, value, 16); }
	
	uint64_t		addr() const					INLINE { return P::getP(section.fields.addr); }
	void			set_addr(uint64_t value)		INLINE { P::setP(section.fields.addr, value); }

	uint64_t		size() const					INLINE { return P::getP(section.fields.size); }
	void			set_size(uint64_t value)		INLINE { P::setP(section.fields.size, value); }

	uint32_t		offset() const					INLINE { return E::get32(section.fields.offset); }
	void			set_offset(uint32_t value)		INLINE { E::set32(section.fields.offset, value); }

	uint32_t		align() const					INLINE { return E::get32(section.fields.align); }
	void			set_align(uint32_t value)		INLINE { E::set32(section.fields.align, value); }

	uint32_t		reloff() const					INLINE { return E::get32(section.fields.reloff); }
	void			set_reloff(uint32_t value)		INLINE { E::set32(section.fields.reloff, value); }

	uint32_t		nreloc() const					INLINE { return E::get32(section.fields.nreloc); }
	void			set_nreloc(uint32_t value)		INLINE { E::set32(section.fields.nreloc, value); }

	uint32_t		flags() const					INLINE { return E::get32(section.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(section.fields.flags, value); }

	uint32_t		reserved1() const				INLINE { return E::get32(section.fields.reserved1); }
	void			set_reserved1(uint32_t value)	INLINE { E::set32(section.fields.reserved1, value); }

	uint32_t		reserved2() const				INLINE { return E::get32(section.fields.reserved2); }
	void			set_reserved2(uint32_t value)	INLINE { E::set32(section.fields.reserved2, value); }

	typedef typename P::E		E;
private:
	macho_section_content<P>	section;
};


//
// mach-o dylib load command
//
template <typename P>
class macho_dylib_command {
public:
	uint32_t		cmd() const									INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)						INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const								INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)					INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		name_offset() const							INLINE { return E::get32(fields.dylib.name.offset); }
	void			set_name_offset(uint32_t value)				INLINE { E::set32(fields.dylib.name.offset, value);  }
	
	uint32_t		timestamp() const							INLINE { return E::get32(fields.dylib.timestamp); }
	void			set_timestamp(uint32_t value)				INLINE { E::set32(fields.dylib.timestamp, value); }

	uint32_t		current_version() const						INLINE { return E::get32(fields.dylib.current_version); }
	void			set_current_version(uint32_t value)			INLINE { E::set32(fields.dylib.current_version, value); }

	uint32_t		compatibility_version() const				INLINE { return E::get32(fields.dylib.compatibility_version); }
	void			set_compatibility_version(uint32_t value)	INLINE { E::set32(fields.dylib.compatibility_version, value); }

	const char*		name() const								INLINE { return (const char*)&fields + name_offset(); }
	void			set_name_offset()							INLINE { set_name_offset(sizeof(fields)); }
	
	typedef typename P::E		E;
private:
	dylib_command	fields;
};


//
// mach-o dylinker load command
//
template <typename P>
class macho_dylinker_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		name_offset() const					INLINE { return E::get32(fields.name.offset); }
	void			set_name_offset(uint32_t value)		INLINE { E::set32(fields.name.offset, value);  }
	
	const char*		name() const						INLINE { return (const char*)&fields + name_offset(); }
	void			set_name_offset()					INLINE { set_name_offset(sizeof(fields)); }
	
	typedef typename P::E		E;
private:
	dylinker_command	fields;
};


//
// mach-o sub_framework load command
//
template <typename P>
class macho_sub_framework_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		umbrella_offset() const				INLINE { return E::get32(fields.umbrella.offset); }
	void			set_umbrella_offset(uint32_t value)	INLINE { E::set32(fields.umbrella.offset, value);  }
	
	const char*		umbrella() const					INLINE { return (const char*)&fields + umbrella_offset(); }
	void			set_umbrella_offset()				INLINE { set_umbrella_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_framework_command	fields;
};


//
// mach-o sub_client load command
//
template <typename P>
class macho_sub_client_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		client_offset() const				INLINE { return E::get32(fields.client.offset); }
	void			set_client_offset(uint32_t value)	INLINE { E::set32(fields.client.offset, value);  }
	
	const char*		client() const						INLINE { return (const char*)&fields + client_offset(); }
	void			set_client_offset()					INLINE { set_client_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_client_command	fields;
};


//
// mach-o sub_umbrella load command
//
template <typename P>
class macho_sub_umbrella_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		sub_umbrella_offset() const				INLINE { return E::get32(fields.sub_umbrella.offset); }
	void			set_sub_umbrella_offset(uint32_t value)	INLINE { E::set32(fields.sub_umbrella.offset, value);  }
	
	const char*		sub_umbrella() const					INLINE { return (const char*)&fields + sub_umbrella_offset(); }
	void			set_sub_umbrella_offset()				INLINE { set_sub_umbrella_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_umbrella_command	fields;
};


//
// mach-o sub_library load command
//
template <typename P>
class macho_sub_library_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		sub_library_offset() const				INLINE { return E::get32(fields.sub_library.offset); }
	void			set_sub_library_offset(uint32_t value)	INLINE { E::set32(fields.sub_library.offset, value);  }
	
	const char*		sub_library() const						INLINE { return (const char*)&fields + sub_library_offset(); }
	void			set_sub_library_offset()				INLINE { set_sub_library_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_library_command	fields;
};


//
// mach-o uuid load command
//
template <typename P>
class macho_uuid_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	const uint8_t*	uuid() const							INLINE { return fields.uuid; }
	void			set_uuid(const uint8_t u[16])			INLINE { memcpy(&fields.uuid, u, 16); }
			
	typedef typename P::E		E;
private:
	uuid_command	fields;
};


//
// mach-o routines load command
//
template <typename P> struct macho_routines_content {};
template <> struct macho_routines_content<Pointer32<BigEndian> >    { routines_command		fields; enum { CMD = LC_ROUTINES	}; };
template <> struct macho_routines_content<Pointer64<BigEndian> >	{ routines_command_64	fields; enum { CMD = LC_ROUTINES_64	}; };
template <> struct macho_routines_content<Pointer32<LittleEndian> > { routines_command		fields; enum { CMD = LC_ROUTINES	}; };
template <> struct macho_routines_content<Pointer64<LittleEndian> > { routines_command_64	fields; enum { CMD = LC_ROUTINES_64	}; };

template <typename P>
class macho_routines_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(routines.fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(routines.fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(routines.fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(routines.fields.cmdsize, value); }

	uint64_t		init_address() const				INLINE { return P::getP(routines.fields.init_address); }
	void			set_init_address(uint64_t value)	INLINE { P::setP(routines.fields.init_address, value); }

	uint64_t		init_module() const					INLINE { return P::getP(routines.fields.init_module); }
	void			set_init_module(uint64_t value)		INLINE { P::setP(routines.fields.init_module, value); }

	uint64_t		reserved1() const					INLINE { return P::getP(routines.fields.reserved1); }
	void			set_reserved1(uint64_t value)		INLINE { P::setP(routines.fields.reserved1, value); }
	
	uint64_t		reserved2() const					INLINE { return P::getP(routines.fields.reserved2); }
	void			set_reserved2(uint64_t value)		INLINE { P::setP(routines.fields.reserved2, value); }
	
	uint64_t		reserved3() const					INLINE { return P::getP(routines.fields.reserved3); }
	void			set_reserved3(uint64_t value)		INLINE { P::setP(routines.fields.reserved3, value); }
	
	uint64_t		reserved4() const					INLINE { return P::getP(routines.fields.reserved4); }
	void			set_reserved4(uint64_t value)		INLINE { P::setP(routines.fields.reserved4, value); }
	
	uint64_t		reserved5() const					INLINE { return P::getP(routines.fields.reserved5); }
	void			set_reserved5(uint64_t value)		INLINE { P::setP(routines.fields.reserved5, value); }
	
	uint64_t		reserved6() const					INLINE { return P::getP(routines.fields.reserved6); }
	void			set_reserved6(uint64_t value)		INLINE { P::setP(routines.fields.reserved6, value); }
	
	typedef typename P::E		E;
	enum {
		CMD = macho_routines_content<P>::CMD
	};
private:
	macho_routines_content<P>	routines;
};


//
// mach-o symbol table load command
//
template <typename P>
class macho_symtab_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		symoff() const				INLINE { return E::get32(fields.symoff); }
	void			set_symoff(uint32_t value)	INLINE { E::set32(fields.symoff, value);  }
	
	uint32_t		nsyms() const				INLINE { return E::get32(fields.nsyms); }
	void			set_nsyms(uint32_t value)	INLINE { E::set32(fields.nsyms, value);  }
	
	uint32_t		stroff() const				INLINE { return E::get32(fields.stroff); }
	void			set_stroff(uint32_t value)	INLINE { E::set32(fields.stroff, value);  }
	
	uint32_t		strsize() const				INLINE { return E::get32(fields.strsize); }
	void			set_strsize(uint32_t value)	INLINE { E::set32(fields.strsize, value);  }
	
	
	typedef typename P::E		E;
private:
	symtab_command	fields;
};


//
// mach-o dynamic symbol table load command
//
template <typename P>
class macho_dysymtab_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		ilocalsym() const					INLINE { return E::get32(fields.ilocalsym); }
	void			set_ilocalsym(uint32_t value)		INLINE { E::set32(fields.ilocalsym, value);  }
	
	uint32_t		nlocalsym() const					INLINE { return E::get32(fields.nlocalsym); }
	void			set_nlocalsym(uint32_t value)		INLINE { E::set32(fields.nlocalsym, value);  }
	
	uint32_t		iextdefsym() const					INLINE { return E::get32(fields.iextdefsym); }
	void			set_iextdefsym(uint32_t value)		INLINE { E::set32(fields.iextdefsym, value);  }
	
	uint32_t		nextdefsym() const					INLINE { return E::get32(fields.nextdefsym); }
	void			set_nextdefsym(uint32_t value)		INLINE { E::set32(fields.nextdefsym, value);  }
	
	uint32_t		iundefsym() const					INLINE { return E::get32(fields.iundefsym); }
	void			set_iundefsym(uint32_t value)		INLINE { E::set32(fields.iundefsym, value);  }
	
	uint32_t		nundefsym() const					INLINE { return E::get32(fields.nundefsym); }
	void			set_nundefsym(uint32_t value)		INLINE { E::set32(fields.nundefsym, value);  }
	
	uint32_t		tocoff() const						INLINE { return E::get32(fields.tocoff); }
	void			set_tocoff(uint32_t value)			INLINE { E::set32(fields.tocoff, value);  }
	
	uint32_t		ntoc() const						INLINE { return E::get32(fields.ntoc); }
	void			set_ntoc(uint32_t value)			INLINE { E::set32(fields.ntoc, value);  }
	
	uint32_t		modtaboff() const					INLINE { return E::get32(fields.modtaboff); }
	void			set_modtaboff(uint32_t value)		INLINE { E::set32(fields.modtaboff, value);  }
	
	uint32_t		nmodtab() const						INLINE { return E::get32(fields.nmodtab); }
	void			set_nmodtab(uint32_t value)			INLINE { E::set32(fields.nmodtab, value);  }
	
	uint32_t		extrefsymoff() const				INLINE { return E::get32(fields.extrefsymoff); }
	void			set_extrefsymoff(uint32_t value)	INLINE { E::set32(fields.extrefsymoff, value);  }
	
	uint32_t		nextrefsyms() const					INLINE { return E::get32(fields.nextrefsyms); }
	void			set_nextrefsyms(uint32_t value)		INLINE { E::set32(fields.nextrefsyms, value);  }
	
	uint32_t		indirectsymoff() const				INLINE { return E::get32(fields.indirectsymoff); }
	void			set_indirectsymoff(uint32_t value)	INLINE { E::set32(fields.indirectsymoff, value);  }
	
	uint32_t		nindirectsyms() const				INLINE { return E::get32(fields.nindirectsyms); }
	void			set_nindirectsyms(uint32_t value)	INLINE { E::set32(fields.nindirectsyms, value);  }
	
	uint32_t		extreloff() const					INLINE { return E::get32(fields.extreloff); }
	void			set_extreloff(uint32_t value)		INLINE { E::set32(fields.extreloff, value);  }
	
	uint32_t		nextrel() const						INLINE { return E::get32(fields.nextrel); }
	void			set_nextrel(uint32_t value)			INLINE { E::set32(fields.nextrel, value);  }
	
	uint32_t		locreloff() const					INLINE { return E::get32(fields.locreloff); }
	void			set_locreloff(uint32_t value)		INLINE { E::set32(fields.locreloff, value);  }
	
	uint32_t		nlocrel() const						INLINE { return E::get32(fields.nlocrel); }
	void			set_nlocrel(uint32_t value)			INLINE { E::set32(fields.nlocrel, value);  }
	
	typedef typename P::E		E;
private:
	dysymtab_command	fields;
};




//
// mach-o module table entry (for compatibility with old ld/dyld)
//
template <typename P> struct macho_dylib_module_content {};
template <> struct macho_dylib_module_content<Pointer32<BigEndian> >    { struct dylib_module		fields; };
template <> struct macho_dylib_module_content<Pointer32<LittleEndian> > { struct dylib_module		fields; };
template <> struct macho_dylib_module_content<Pointer64<BigEndian> >    { struct dylib_module_64	fields; };
template <> struct macho_dylib_module_content<Pointer64<LittleEndian> > { struct dylib_module_64	fields; };

template <typename P>
class macho_dylib_module {
public:
	uint32_t		module_name() const				INLINE { return E::get32(module.fields.module_name); }
	void			set_module_name(uint32_t value)	INLINE { E::set32(module.fields.module_name, value);  }
	
	uint32_t		iextdefsym() const				INLINE { return E::get32(module.fields.iextdefsym); }
	void			set_iextdefsym(uint32_t value)	INLINE { E::set32(module.fields.iextdefsym, value);  }
	
	uint32_t		nextdefsym() const				INLINE { return E::get32(module.fields.nextdefsym); }
	void			set_nextdefsym(uint32_t value)	INLINE { E::set32(module.fields.nextdefsym, value);  }
	
	uint32_t		irefsym() const					INLINE { return E::get32(module.fields.irefsym); }
	void			set_irefsym(uint32_t value)		INLINE { E::set32(module.fields.irefsym, value);  }
	
	uint32_t		nrefsym() const					INLINE { return E::get32(module.fields.nrefsym); }
	void			set_nrefsym(uint32_t value)		INLINE { E::set32(module.fields.nrefsym, value);  }
	
	uint32_t		ilocalsym() const				INLINE { return E::get32(module.fields.ilocalsym); }
	void			set_ilocalsym(uint32_t value)	INLINE { E::set32(module.fields.ilocalsym, value);  }
	
	uint32_t		nlocalsym() const				INLINE { return E::get32(module.fields.nlocalsym); }
	void			set_nlocalsym(uint32_t value)	INLINE { E::set32(module.fields.nlocalsym, value);  }
	
	uint32_t		iextrel() const					INLINE { return E::get32(module.fields.iextrel); }
	void			set_iextrel(uint32_t value)		INLINE { E::set32(module.fields.iextrel, value);  }
	
	uint32_t		nextrel() const					INLINE { return E::get32(module.fields.nextrel); }
	void			set_nextrel(uint32_t value)		INLINE { E::set32(module.fields.nextrel, value);  }
	
	uint16_t		iinit() const					INLINE { return E::get32(module.fields.iinit_iterm) & 0xFFFF; }
	uint16_t		iterm() const					INLINE { return E::get32(module.fields.iinit_iterm) > 16; }
	void			set_iinit_iterm(uint16_t init, uint16_t term)	INLINE { E::set32(module.fields.iinit_iterm, (term<<16) | (init &0xFFFF));  }
	
	uint16_t		ninit() const					INLINE { return E::get32(module.fields.ninit_nterm) & 0xFFFF; }
	uint16_t		nterm() const					INLINE { return E::get32(module.fields.ninit_nterm) > 16; }
	void			set_ninit_nterm(uint16_t init, uint16_t term)	INLINE { E::set32(module.fields.ninit_nterm, (term<<16) | (init &0xFFFF));  }
	
	uint64_t		objc_module_info_addr() const				INLINE { return P::getP(module.fields.objc_module_info_addr); }
	void			set_objc_module_info_addr(uint64_t value)	INLINE { P::setP(module.fields.objc_module_info_addr, value);  }
	
	uint32_t		objc_module_info_size() const				INLINE { return E::get32(module.fields.objc_module_info_size); }
	void			set_objc_module_info_size(uint32_t value)	INLINE { E::set32(module.fields.objc_module_info_size, value);  }
	
	
	typedef typename P::E		E;
private:
	macho_dylib_module_content<P>	module;
};


//
// mach-o dylib_reference entry
//
template <typename P>
class macho_dylib_reference {
public:
	uint32_t		isym() const				INLINE { return E::getBits(fields, 0, 24); }
	void			set_isym(uint32_t value)	INLINE { E::setBits(fields, value, 0, 24); }
	
	uint8_t			flags() const				INLINE { return E::getBits(fields, 24, 8); }
	void			set_flags(uint8_t value)	INLINE { E::setBits(fields, value, 24, 8); }
	
	typedef typename P::E		E;
private:
	uint32_t		fields;
};



//
// mach-o two-level hints load command
//
template <typename P>
class macho_dylib_table_of_contents {
public:
	uint32_t		symbol_index() const				INLINE { return E::get32(fields.symbol_index); }
	void			set_symbol_index(uint32_t value)	INLINE { E::set32(fields.symbol_index, value); }

	uint32_t		module_index() const				INLINE { return E::get32(fields.module_index); }
	void			set_module_index(uint32_t value)	INLINE { E::set32(fields.module_index, value);  }
		
	typedef typename P::E		E;
private:
	dylib_table_of_contents	fields;
};



//
// mach-o two-level hints load command
//
template <typename P>
class macho_twolevel_hints_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		offset() const				INLINE { return E::get32(fields.offset); }
	void			set_offset(uint32_t value)	INLINE { E::set32(fields.offset, value);  }
	
	uint32_t		nhints() const				INLINE { return E::get32(fields.nhints); }
	void			set_nhints(uint32_t value)	INLINE { E::set32(fields.nhints, value);  }
	
	typedef typename P::E		E;
private:
	twolevel_hints_command	fields;
};


//
// mach-o threads load command
//
template <typename P>
class macho_thread_command {
public:
	uint32_t		cmd() const											INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)								INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const										INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)							INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		flavor() const										INLINE { return E::get32(fields_flavor); }
	void			set_flavor(uint32_t value)							INLINE { E::set32(fields_flavor, value);  }
	
	uint32_t		count() const										INLINE { return E::get32(fields_count); }
	void			set_count(uint32_t value)							INLINE { E::set32(fields_count, value);  }
	
	uint64_t		thread_register(uint32_t index) const				INLINE { return P::getP(thread_registers[index]); }
	void			set_thread_register(uint32_t index, uint64_t value)	INLINE { P::setP(thread_registers[index], value); }
	
	typedef typename P::E		E;
	typedef typename P::uint_t	pint_t;
private:
	struct thread_command	fields;
	uint32_t				fields_flavor;
	uint32_t				fields_count;
	pint_t					thread_registers[1];
};


//
// mach-o misc data 
//
template <typename P>
class macho_linkedit_data_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		dataoff() const				INLINE { return E::get32(fields.dataoff); }
	void			set_dataoff(uint32_t value)	INLINE { E::set32(fields.dataoff, value);  }
	
	uint32_t		datasize() const			INLINE { return E::get32(fields.datasize); }
	void			set_datasize(uint32_t value)INLINE { E::set32(fields.datasize, value);  }
	
	
	typedef typename P::E		E;
private:
	struct linkedit_data_command	fields;
};


//
// mach-o rpath  
//
template <typename P>
class macho_rpath_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		path_offset() const				INLINE { return E::get32(fields.path.offset); }
	void			set_path_offset(uint32_t value)	INLINE { E::set32(fields.path.offset, value);  }
	
	const char*		path() const					INLINE { return (const char*)&fields + path_offset(); }
	void			set_path_offset()				INLINE { set_path_offset(sizeof(fields)); }
	
	
	typedef typename P::E		E;
private:
	struct rpath_command	fields;
};



//
// mach-o symbol table entry 
//
template <typename P> struct macho_nlist_content {};
template <> struct macho_nlist_content<Pointer32<BigEndian> >    { struct nlist		fields; };
template <> struct macho_nlist_content<Pointer64<BigEndian> >	 { struct nlist_64	fields; };
template <> struct macho_nlist_content<Pointer32<LittleEndian> > { struct nlist		fields; };
template <> struct macho_nlist_content<Pointer64<LittleEndian> > { struct nlist_64	fields; };

template <typename P>
class macho_nlist {
public:
	uint32_t		n_strx() const					INLINE { return E::get32(entry.fields.n_un.n_strx); }
	void			set_n_strx(uint32_t value)		INLINE { E::set32((uint32_t&)entry.fields.n_un.n_strx, value); }

	uint8_t			n_type() const					INLINE { return entry.fields.n_type; }
	void			set_n_type(uint8_t value)		INLINE { entry.fields.n_type = value; }

	uint8_t			n_sect() const					INLINE { return entry.fields.n_sect; }
	void			set_n_sect(uint8_t value)		INLINE { entry.fields.n_sect = value; }

	uint16_t		n_desc() const					INLINE { return E::get16(entry.fields.n_desc); }
	void			set_n_desc(uint16_t value)		INLINE { E::set16((uint16_t&)entry.fields.n_desc, value); }

	uint64_t		n_value() const					INLINE { return P::getP(entry.fields.n_value); }
	void			set_n_value(uint64_t value)		INLINE { P::setP(entry.fields.n_value, value); }

	typedef typename P::E		E;
private:
	macho_nlist_content<P>	entry;
};



//
// mach-o relocation info
//
template <typename P>
class macho_relocation_info {
public:
	uint32_t		r_address() const				INLINE { return E::get32(address); }
	void			set_r_address(uint32_t value)	INLINE { E::set32(address, value); }

	uint32_t		r_symbolnum() const				INLINE { return E::getBits(other, 0, 24); }
	void			set_r_symbolnum(uint32_t value) INLINE { E::setBits(other, value, 0, 24); }

	bool			r_pcrel() const					INLINE { return E::getBits(other, 24, 1); }
	void			set_r_pcrel(bool value)			INLINE { E::setBits(other, value, 24, 1); }	
	
	uint8_t			r_length() const				INLINE { return E::getBits(other, 25, 2); }
	void			set_r_length(uint8_t value)		INLINE { E::setBits(other, value, 25, 2); }
	
	bool			r_extern() const				INLINE { return E::getBits(other, 27, 1); }
	void			set_r_extern(bool value)		INLINE { E::setBits(other, value, 27, 1); }
	
	uint8_t			r_type() const					INLINE { return E::getBits(other, 28, 4); }
	void			set_r_type(uint8_t value)		INLINE { E::setBits(other, value, 28, 4); }
		
	void			set_r_length()					INLINE { set_r_length((sizeof(typename P::uint_t)==8) ? 3 : 2); }

	typedef typename P::E		E;
private:
	uint32_t		address;
	uint32_t		other;
};


//
// mach-o scattered relocation info
// The bit fields are always in big-endian order (see mach-o/reloc.h)
//
template <typename P>
class macho_scattered_relocation_info {
public:
	bool			r_scattered() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 0, 1); }
	void			set_r_scattered(bool x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 0, 1);  E::set32(other, temp); }

	bool			r_pcrel() const				INLINE { return BigEndian::getBitsRaw(E::get32(other), 1, 1); }
	void			set_r_pcrel(bool x)			INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 1, 1);  E::set32(other, temp); }

	uint8_t			r_length() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 2, 2); }
	void			set_r_length(uint8_t x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 2, 2);  E::set32(other, temp); }

	uint8_t			r_type() const				INLINE { return BigEndian::getBitsRaw(E::get32(other), 4, 4); }
	void			set_r_type(uint8_t x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 4, 4);  E::set32(other, temp); }

	uint32_t		r_address() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 8, 24); }
	void			set_r_address(uint32_t x)			{ if ( x > 0x00FFFFFF ) throw "scattered reloc r_address too large"; 
														uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 8, 24);  E::set32(other, temp); }

	uint32_t		r_value() const				INLINE { return E::get32(value); }
	void			set_r_value(uint32_t x)		INLINE { E::set32(value, x); }

	uint32_t		r_other() const				INLINE { return other; }
	
	// void			set_r_length()				INLINE { set_r_length((sizeof(typename P::uint_t)==8) ? 3 : 2); }

	typedef typename P::E		E;
private:
	uint32_t		other;
	uint32_t		value;
};



//
// mach-o encyrption info load command
//
template <typename P> struct macho_encryption_info_content {};
template <> struct macho_encryption_info_content<Pointer32<BigEndian> >    { struct encryption_info_command		fields; };
template <> struct macho_encryption_info_content<Pointer64<BigEndian> >	   { struct encryption_info_command_64	fields; };
template <> struct macho_encryption_info_content<Pointer32<LittleEndian> > { struct encryption_info_command		fields; };
template <> struct macho_encryption_info_content<Pointer64<LittleEndian> > { struct encryption_info_command_64	fields; };


template <typename P>
class macho_encryption_info_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(entry.fields.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(entry.fields.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(entry.fields.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(entry.fields.cmdsize, value); }

	uint32_t		cryptoff() const				INLINE { return E::get32(entry.fields.cryptoff); }
	void			set_cryptoff(uint32_t value)	INLINE { E::set32(entry.fields.cryptoff, value);  }
	
	uint32_t		cryptsize() const				INLINE { return E::get32(entry.fields.cryptsize); }
	void			set_cryptsize(uint32_t value)	INLINE { E::set32(entry.fields.cryptsize, value);  }
	
	uint32_t		cryptid() const					INLINE { return E::get32(entry.fields.cryptid); }
	void			set_cryptid(uint32_t value)		INLINE { E::set32(entry.fields.cryptid, value);  }
	
	uint32_t		pad() const						INLINE { return E::get32(entry.fields.pad); }
	void			set_pad(uint32_t value)			INLINE { E::set32(entry.fields.pad, value);  }

	typedef typename P::E		E;
private:
	macho_encryption_info_content<P>	entry;
};


//
// start of __unwind_info section  
//
template <typename P>
class macho_unwind_info_section_header {
public:
	uint32_t		version() const											INLINE { return E::get32(fields.version); }
	void			set_version(uint32_t value)								INLINE { E::set32(fields.version, value); }

	uint32_t		commonEncodingsArraySectionOffset() const				INLINE { return E::get32(fields.commonEncodingsArraySectionOffset); }
	void			set_commonEncodingsArraySectionOffset(uint32_t value)	INLINE { E::set32(fields.commonEncodingsArraySectionOffset, value); }

	uint32_t		commonEncodingsArrayCount() const						INLINE { return E::get32(fields.commonEncodingsArrayCount); }
	void			set_commonEncodingsArrayCount(uint32_t value)			INLINE { E::set32(fields.commonEncodingsArrayCount, value); }

	uint32_t		personalityArraySectionOffset() const					INLINE { return E::get32(fields.personalityArraySectionOffset); }
	void			set_personalityArraySectionOffset(uint32_t value)		INLINE { E::set32(fields.personalityArraySectionOffset, value); }

	uint32_t		personalityArrayCount() const							INLINE { return E::get32(fields.personalityArrayCount); }
	void			set_personalityArrayCount(uint32_t value)				INLINE { E::set32(fields.personalityArrayCount, value); }

	uint32_t		indexSectionOffset() const								INLINE { return E::get32(fields.indexSectionOffset); }
	void			set_indexSectionOffset(uint32_t value)					INLINE { E::set32(fields.indexSectionOffset, value); }

	uint32_t		indexCount() const										INLINE { return E::get32(fields.indexCount); }
	void			set_indexCount(uint32_t value)							INLINE { E::set32(fields.indexCount, value); }

	typedef typename P::E		E;
private:
	unwind_info_section_header	fields;
};



//
// uwind first level index entry  
//
template <typename P>
class macho_unwind_info_section_header_index_entry {
public:
	uint32_t		functionOffset() const								INLINE { return E::get32(fields.functionOffset); }
	void			set_functionOffset(uint32_t value)					INLINE { E::set32(fields.functionOffset, value); }

	uint32_t		secondLevelPagesSectionOffset() const				INLINE { return E::get32(fields.secondLevelPagesSectionOffset); }
	void			set_secondLevelPagesSectionOffset(uint32_t value)	INLINE { E::set32(fields.secondLevelPagesSectionOffset, value); }

	uint32_t		lsdaIndexArraySectionOffset() const					INLINE { return E::get32(fields.lsdaIndexArraySectionOffset); }
	void			set_lsdaIndexArraySectionOffset(uint32_t value)		INLINE { E::set32(fields.lsdaIndexArraySectionOffset, value); }

	typedef typename P::E		E;
private:
	unwind_info_section_header_index_entry	fields;
};


//
// LSDA table entry  
//
template <typename P>
class macho_unwind_info_section_header_lsda_index_entry {
public:
	uint32_t		functionOffset() const								INLINE { return E::get32(fields.functionOffset); }
	void			set_functionOffset(uint32_t value)					INLINE { E::set32(fields.functionOffset, value); }

	uint32_t		lsdaOffset() const									INLINE { return E::get32(fields.lsdaOffset); }
	void			set_lsdaOffset(uint32_t value)						INLINE { E::set32(fields.lsdaOffset, value); }

	typedef typename P::E		E;
private:
	unwind_info_section_header_lsda_index_entry	fields;
};


//
// regular second level entry  
//
template <typename P>
class macho_unwind_info_regular_second_level_entry {
public:
	uint32_t		functionOffset() const								INLINE { return E::get32(fields.functionOffset); }
	void			set_functionOffset(uint32_t value)					INLINE { E::set32(fields.functionOffset, value); }

	uint32_t		encoding() const									INLINE { return E::get32(fields.encoding); }
	void			set_encoding(uint32_t value)						INLINE { E::set32(fields.encoding, value); }

	typedef typename P::E		E;
private:
	unwind_info_regular_second_level_entry	fields;
};


//
// start of second level regular page  
//
template <typename P>
class macho_unwind_info_regular_second_level_page_header {
public:
	uint32_t		kind() const								INLINE { return E::get32(fields.kind); }
	void			set_kind(uint32_t value)					INLINE { E::set32(fields.kind, value); }

	uint16_t		entryPageOffset() const						INLINE { return E::get16(fields.entryPageOffset); }
	void			set_entryPageOffset(uint16_t value)			INLINE { E::set16((uint16_t&)fields.entryPageOffset, value); }

	uint16_t		entryCount() const							INLINE { return E::get16(fields.entryCount); }
	void			set_entryCount(uint16_t value)				INLINE { E::set16((uint16_t&)fields.entryCount, value); }

	typedef typename P::E		E;
private:
	unwind_info_regular_second_level_page_header	fields;
};


//
// start of second level compressed page  
//
template <typename P>
class macho_unwind_info_compressed_second_level_page_header {
public:
	uint32_t		kind() const								INLINE { return E::get32(fields.kind); }
	void			set_kind(uint32_t value)					INLINE { E::set32(fields.kind, value); }

	uint16_t		entryPageOffset() const						INLINE { return E::get16(fields.entryPageOffset); }
	void			set_entryPageOffset(uint16_t value)			INLINE { E::set16((uint16_t&)fields.entryPageOffset, value); }

	uint16_t		entryCount() const							INLINE { return E::get16(fields.entryCount); }
	void			set_entryCount(uint16_t value)				INLINE { E::set16((uint16_t&)fields.entryCount, value); }

	uint16_t		encodingsPageOffset() const					INLINE { return E::get16(fields.encodingsPageOffset); }
	void			set_encodingsPageOffset(uint16_t value)		INLINE { E::set16((uint16_t&)fields.encodingsPageOffset, value); }

	uint16_t		encodingsCount() const						INLINE { return E::get16(fields.encodingsCount); }
	void			set_encodingsCount(uint16_t value)			INLINE { E::set16((uint16_t&)fields.encodingsCount, value); }

	typedef typename P::E		E;
private:
	unwind_info_compressed_second_level_page_header	fields;
};


//
// compressed dyld info load command
//
template <typename P>
class macho_dyld_info_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		rebase_off() const				INLINE { return E::get32(fields.rebase_off); }
	void			set_rebase_off(uint32_t value)	INLINE { E::set32(fields.rebase_off, value);  }
	
	uint32_t		rebase_size() const				INLINE { return E::get32(fields.rebase_size); }
	void			set_rebase_size(uint32_t value)	INLINE { E::set32(fields.rebase_size, value);  }
	
	uint32_t		bind_off() const				INLINE { return E::get32(fields.bind_off); }
	void			set_bind_off(uint32_t value)	INLINE { E::set32(fields.bind_off, value);  }
	
	uint32_t		bind_size() const				INLINE { return E::get32(fields.bind_size); }
	void			set_bind_size(uint32_t value)	INLINE { E::set32(fields.bind_size, value);  }
	
	uint32_t		weak_bind_off() const				INLINE { return E::get32(fields.weak_bind_off); }
	void			set_weak_bind_off(uint32_t value)	INLINE { E::set32(fields.weak_bind_off, value);  }
	
	uint32_t		weak_bind_size() const				INLINE { return E::get32(fields.weak_bind_size); }
	void			set_weak_bind_size(uint32_t value)	INLINE { E::set32(fields.weak_bind_size, value);  }
	
	uint32_t		lazy_bind_off() const				INLINE { return E::get32(fields.lazy_bind_off); }
	void			set_lazy_bind_off(uint32_t value)	INLINE { E::set32(fields.lazy_bind_off, value);  }
	
	uint32_t		lazy_bind_size() const				INLINE { return E::get32(fields.lazy_bind_size); }
	void			set_lazy_bind_size(uint32_t value)	INLINE { E::set32(fields.lazy_bind_size, value);  }
	
	uint32_t		export_off() const				INLINE { return E::get32(fields.export_off); }
	void			set_export_off(uint32_t value)	INLINE { E::set32(fields.export_off, value);  }
	
	uint32_t		export_size() const				INLINE { return E::get32(fields.export_size); }
	void			set_export_size(uint32_t value)	INLINE { E::set32(fields.export_size, value);  }
	
	
	typedef typename P::E		E;
private:
	dyld_info_command	fields;
};


//
// mach-o version load command
//
template <typename P>
class macho_version_min_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		version() const							INLINE { return fields.version; }
	void			set_version(uint32_t value)				INLINE { E::set32(fields.version, value); }

	uint32_t		sdk() const								INLINE { return fields.sdk; }
	void			set_sdk(uint32_t value)					INLINE { E::set32(fields.sdk, value); }

	typedef typename P::E		E;
private:
	version_min_command	fields;
};



//
// mach-o build version load command
//
template <typename P>
class macho_build_version_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		platform() const						INLINE { return fields.platform; }
	void			set_platform(uint32_t value)			INLINE { E::set32(fields.platform, value); }

	uint32_t		minos() const							INLINE { return fields.minos; }
	void			set_minos(uint32_t value)				INLINE { E::set32(fields.minos, value); }

	uint32_t		sdk() const								INLINE { return fields.sdk; }
	void			set_sdk(uint32_t value)					INLINE { E::set32(fields.sdk, value); }

	uint32_t		ntools() const							INLINE { return fields.ntools; }
	void			set_ntools(uint32_t value)				INLINE { E::set32(fields.ntools, value); }


	typedef typename P::E		E;
private:
	build_version_command	fields;
};


//
// mach-o build version load command
//
template <typename P>
class macho_build_tool_version {
public:
	uint32_t		tool() const							INLINE { return E::get32(fields.tool); }
	void			set_tool(uint32_t value)				INLINE { E::set32(fields.tool, value); }

	uint32_t		version() const							INLINE { return E::get32(fields.version); }
	void			set_version(uint32_t value)				INLINE { E::set32(fields.version, value); }

	typedef typename P::E		E;
private:
	build_tool_version	fields;
};




//
// mach-o __LD, __compact_unwind section in object files
//
template <typename P>
class macho_compact_unwind_entry {
public:
	typedef typename P::E		E;
	typedef typename P::uint_t	pint_t;

	pint_t			codeStart() const						INLINE { return P::getP(_codeStart); }
	void			set_codeStart(pint_t value)				INLINE { P::setP(_codeStart, value); }

	uint32_t		codeLen() const							INLINE { return E::get32(_codeLen); }
	void			set_codeLen(uint32_t value)				INLINE { E::set32(_codeLen, value); }

	uint32_t		compactUnwindInfo() const				INLINE { return E::get32(_compactUnwindInfo); }
	void			set_compactUnwindInfo(uint32_t value)	INLINE { E::set32(_compactUnwindInfo, value);  }
	
	pint_t			personality() const						INLINE { return P::getP(_personality); }
	void			set_personality(pint_t value)			INLINE { P::setP(_personality, value);  }
	
	pint_t			lsda() const							INLINE { return P::getP(_lsda); }
	void			set_lsda(pint_t value)					INLINE { P::setP(_lsda, value);  }
	
	static uint32_t	codeStartFieldOffset()					INLINE { return offsetof(macho_compact_unwind_entry<P>,_codeStart); }
	static uint32_t	personalityFieldOffset()				INLINE { return offsetof(macho_compact_unwind_entry<P>,_personality); }
	static uint32_t	lsdaFieldOffset()						INLINE { return offsetof(macho_compact_unwind_entry<P>,_lsda); }
	
private:
	pint_t		_codeStart;
	uint32_t	_codeLen;
	uint32_t	_compactUnwindInfo;
	pint_t		_personality;
	pint_t		_lsda;
};


//
// mach-o source version load command
//
template <typename P>
class macho_source_version_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint64_t		version() const							INLINE { return fields.version; }
	void			set_version(uint64_t value)				INLINE { E::set64(fields.version, value); }

	typedef typename P::E		E;
private:
	source_version_command	fields;
};


//
// mach-o source version load command
//
template <typename P>
class macho_entry_point_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint64_t		entryoff() const						INLINE { return fields.entryoff; }
	void			set_entryoff(uint64_t value)			INLINE { E::set64(fields.entryoff, value); }

	uint64_t		stacksize() const						INLINE { return fields.stacksize; }
	void			set_stacksize(uint64_t value)			INLINE { E::set64(fields.stacksize, value); }

	typedef typename P::E		E;
private:
	entry_point_command	fields;
};



template <typename P>
class macho_data_in_code_entry {
public:
	uint32_t		offset() const								INLINE { return E::get32(fields.offset); }
	void			set_offset(uint32_t value)					INLINE { E::set32(fields.offset, value); }

	uint16_t		length() const								INLINE { return E::get16(fields.length); }
	void			set_length(uint16_t value)					INLINE { E::set16((uint16_t&)fields.length, value); }

	uint16_t		kind() const								INLINE { return E::get16(fields.kind); }
	void			set_kind(uint16_t value)					INLINE { E::set16((uint16_t&)fields.kind, value); }

	typedef typename P::E		E;
private:
	data_in_code_entry	fields;
};

template <typename P>
class macho_linker_option_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint64_t		count() const							INLINE { return fields.count; }
	void			set_count(uint32_t value)				INLINE { E::set32(fields.count, value); }

	const char*		buffer() const							INLINE { return ((char*)&fields) + sizeof(linker_option_command); }
	char*			buffer()								INLINE { return ((char*)&fields) + sizeof(linker_option_command); }

	typedef typename P::E		E;
private:
	linker_option_command	fields;
};



#endif	// __MACH_O_FILE_ABSTRACTION__


