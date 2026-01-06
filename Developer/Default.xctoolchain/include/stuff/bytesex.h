/*
 * Copyright (c) 2004, Apple Computer, Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/* bytesex.h */
#ifndef _STUFF_BYTESEX_H_
#define _STUFF_BYTESEX_H_

#if !defined(__private_extern__)
#define __private_extern__ extern __attribute__((visibility(("hidden"))))
#endif

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/i386/thread_status.h>
#undef MACHINE_THREAD_STATE
#undef THREAD_STATE_NONE
#undef MACHINE_THREAD_STATE_COUNT
#undef VALID_THREAD_STATE_FLAVOR
#include <mach/arm/thread_status.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/ranlib.h>
#include "xar/xar.h" /* cctools-port:
				   force the use of the bundled xar header */
#include "stuff/bool.h"

enum byte_sex {
    UNKNOWN_BYTE_SEX,
    BIG_ENDIAN_BYTE_SEX,
    LITTLE_ENDIAN_BYTE_SEX
};

#define SWAP_SHORT(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

#define SWAP_INT(a)  ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	 ((unsigned int)(a) >> 24) )

#ifndef __LP64__
#define SWAP_LONG(a) ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	((unsigned long)(a) >> 24) )
#endif

__private_extern__ long long SWAP_LONG_LONG(
    long long ll);

__private_extern__ float SWAP_FLOAT(
    float f);

__private_extern__ double SWAP_DOUBLE(
    double d);

__private_extern__ enum byte_sex get_host_byte_sex(
    void);

__private_extern__ void swap_fat_header(
    struct fat_header *fat_header,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_fat_arch(
    struct fat_arch *fat_archs,
    uint32_t nfat_arch,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_fat_arch_64(
    struct fat_arch_64 *fat_archs64,
    uint32_t nfat_arch,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_mach_header(
    struct mach_header *mh,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_mach_header_64(
    struct mach_header_64 *mh,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_load_command(
    struct load_command *lc,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_segment_command(
    struct segment_command *sg,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_segment_command_64(
    struct segment_command_64 *sg,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_section(
    struct section *s,
    uint32_t nsects,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_section_64(
    struct section_64 *s,
    uint32_t nsects,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_symtab_command(
    struct symtab_command *st,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dysymtab_command(
    struct dysymtab_command *dyst,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_symseg_command(
    struct symseg_command *ss,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_fvmlib_command(
    struct fvmlib_command *fl,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylib_command(
    struct dylib_command *dl,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_sub_framework_command(
    struct sub_framework_command *sub,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_sub_umbrella_command(
    struct sub_umbrella_command *usub,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_sub_library_command(
    struct sub_library_command *lsub,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_sub_client_command(
    struct sub_client_command *csub,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_prebound_dylib_command(
    struct prebound_dylib_command *pbdylib,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylinker_command(
    struct dylinker_command *dyld,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_fvmfile_command(
    struct fvmfile_command *ff,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_thread_command(
    struct thread_command *ut,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_i386_thread_state(
    i386_thread_state_t *cpu,
    enum byte_sex target_byte_sex);

/* current i386 thread states */
#if i386_THREAD_STATE == 1
__private_extern__ void swap_i386_float_state(
    struct __darwin_i386_float_state *fpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_i386_exception_state(
    i386_exception_state_t *exc,
    enum byte_sex target_byte_sex);
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
__private_extern__ void swap_i386_thread_fpstate(
    i386_thread_fpstate_t *fpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_i386_thread_exceptstate(
    i386_thread_exceptstate_t *exc,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_i386_thread_cthreadstate(
    i386_thread_cthreadstate_t *user,
    enum byte_sex target_byte_sex);
#endif /* i386_THREAD_STATE == -1 */

#ifdef x86_THREAD_STATE64
__private_extern__ void swap_x86_thread_state64(
    x86_thread_state64_t *cpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_x86_float_state64(
    x86_float_state64_t *fpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_x86_state_hdr(
    struct x86_state_hdr *hdr,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_x86_exception_state64(
    x86_exception_state64_t *exc,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_x86_debug_state32(
    x86_debug_state32_t *debug,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_x86_debug_state64(
    x86_debug_state64_t *debug,
    enum byte_sex target_byte_sex);
#endif /* x86_THREAD_STATE64 */

__private_extern__ void swap_arm_thread_state_t(
    arm_thread_state_t *cpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_arm_thread_state64_t(
    arm_thread_state64_t *cpu,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_arm_exception_state64_t(
    arm_exception_state64_t *except,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_ident_command(
    struct ident_command *id_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_routines_command(
    struct routines_command *r_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_routines_command_64(
    struct routines_command_64 *r_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_twolevel_hints_command(
    struct twolevel_hints_command *hints_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_prebind_cksum_command(
    struct prebind_cksum_command *cksum_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_uuid_command(
    struct uuid_command *uuid_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_linkedit_data_command(
    struct linkedit_data_command *ld,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_version_min_command(
    struct version_min_command *ver_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_rpath_command(
    struct rpath_command *rpath_cmd,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_encryption_command(
    struct encryption_info_command *ec,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_encryption_command_64(
    struct encryption_info_command_64 *ec,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_linker_option_command(
    struct linker_option_command *lo,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dyld_info_command(
    struct dyld_info_command *dc, 
    enum byte_sex target_byte_sex);

__private_extern__ void swap_entry_point_command(
    struct entry_point_command *ep,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_source_version_command(
    struct source_version_command *sv,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_note_command(
    struct note_command *nc,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_build_version_command(
    struct build_version_command *bv,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_build_tool_version(
    struct build_tool_version *bt,
    uint32_t ntools,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_nlist(
    struct nlist *symbols,
    uint32_t nsymbols,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_nlist_64(
    struct nlist_64 *symbols,
    uint32_t nsymbols,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_ranlib(
    struct ranlib *ranlibs,
    uint32_t nranlibs,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_ranlib_64(
    struct ranlib_64 *ranlibs,
    uint64_t nranlibs,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_relocation_info(
    struct relocation_info *relocs,
    uint32_t nrelocs,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_indirect_symbols(
    uint32_t *indirect_symbols,
    uint32_t nindirect_symbols,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylib_reference(
    struct dylib_reference *refs,
    uint32_t nrefs,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylib_module(
    struct dylib_module *mods,
    uint32_t nmods,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylib_module_64(
    struct dylib_module_64 *mods,
    uint32_t nmods,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_dylib_table_of_contents(
    struct dylib_table_of_contents *tocs,
    uint32_t ntocs,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_twolevel_hint(
    struct twolevel_hint *hints,
    uint32_t nhints,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_data_in_code_entry(
    struct data_in_code_entry *dices,
    uint32_t ndices,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_xar_header(
    struct xar_header *xar,
    enum byte_sex target_byte_sex);

/*
 * swap_object_headers() swaps the object file headers from the host byte sex
 * into the non-host byte sex.  It returns TRUE if it can and did swap the
 * headers else returns FALSE and does not touch the headers and prints an error
 * using the error() routine.
 */
__private_extern__ enum bool swap_object_headers(
    void *mach_header, /* either a mach_header or a mach_header_64 */
    struct load_command *load_commands);

/*
 * get_toc_byte_sex() guesses the byte sex of the table of contents of the
 * library mapped in at the address, addr, of size, size based on the first
 * object file's bytesex.  If it can't figure it out, because the library has
 * no object file members or is malformed it will return UNKNOWN_BYTE_SEX.
 */
__private_extern__ enum byte_sex get_toc_byte_sex(
    char *addr,
    uint64_t size);

#endif /* _STUFF_BYTESEX_H_ */
