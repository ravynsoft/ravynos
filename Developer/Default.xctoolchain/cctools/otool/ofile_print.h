/*
 * Copyright © 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#import <ar.h>
#import <mach-o/fat.h>
#import <mach-o/loader.h>
#import <mach-o/nlist.h>
#import <mach-o/reloc.h>
#import <stuff/bytesex.h>
#import <stuff/bool.h>
#include "stuff/symbol.h"

extern void print_fat_headers(
    struct fat_header *fat_header,
    struct fat_arch *fat_archs,
    struct fat_arch_64 *fat_archs64,
    uint64_t filesize,
    enum bool verbose);

extern void print_ar_hdr(
    struct ar_hdr *ar_hdr,
    char *member_name,
    uint32_t member_name_size,
    uint64_t member_offset,
    enum bool verbose,
    enum bool print_offset);

extern void print_mode_verbose(
    uint32_t mode);

extern void print_library_toc(
    struct ar_hdr *toc_ar_hdr,
    char *toc_name,
    uint32_t toc_name_size,
    char *toc_addr,
    uint32_t toc_size,
    enum byte_sex toc_byte_sex,
    char *library_name,
    char *library_addr,
    uint64_t library_size,
    char *arch_name,
    enum bool verbose);

extern void print_sysv_library_toc(
    struct ar_hdr *toc_ar_hdr,
    char *toc_name,
    uint32_t toc_name_size,
    char *toc_addr,
    uint32_t toc_size,
    enum byte_sex toc_byte_sex,
    char *library_name,
    char *library_addr,
    uint64_t library_size,
    char *arch_name,
    enum bool verbose);

extern void print_mach_header(
    uint32_t magic,
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    uint32_t filetype,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    uint32_t flags,
    enum bool verbose);

extern void print_loadcmds(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    cpu_type_t cputype,
    uint32_t filetype,
    enum byte_sex load_commands_byte_sex,
    uint64_t object_size,
    enum bool verbose,
    enum bool very_verbose);

extern void print_libraries(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex load_commands_byte_sex,
    enum bool just_id,
    enum bool verbose);

extern void print_reloc(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    cpu_type_t cputype,
    enum byte_sex load_commands_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_toc(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex load_commands_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct dylib_table_of_contents *tocs,
    uint32_t ntocs,
    struct dylib_module *mods,
    struct dylib_module_64 *mods64,
    uint32_t nmods,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_module_table(
    struct dylib_module *mods,
    uint32_t nmods,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_module_table_64(
    struct dylib_module_64 *mods64,
    uint32_t nmods,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

void print_refs(
    struct dylib_reference *refs,
    uint32_t nrefs,
    struct dylib_module *mods,
    struct dylib_module_64 *mods64,
    uint32_t nmods,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_indirect_symbols(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    cpu_type_t cputype,
    enum byte_sex load_commands_byte_sex,
    uint32_t *indirect_symbols,
    uint32_t nindirect_symbols,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_hints(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex load_commands_byte_sex,
    struct twolevel_hint *hints,
    uint32_t nhints,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern void print_link_opt_hints(
    char *loh,
    uint32_t nloh);

extern void print_dices(
    struct data_in_code_entry *dices,
    uint32_t ndices,
    enum bool verbose);

extern void print_segment_command(
    uint32_t cmd,
    uint32_t cmdsize,
    char *segname,
    uint64_t vmaddr,
    uint64_t vmsize,
    uint64_t fileoff,
    uint64_t filesize,
    vm_prot_t maxprot,
    vm_prot_t initprot,
    uint32_t nsects,
    uint32_t flags,
    uint64_t object_size,
    enum bool verbose);

extern void print_section(
    char *sectname,
    char *segname,
    uint64_t addr,
    uint64_t size,
    uint32_t offset,
    uint32_t align,
    uint32_t reloff,
    uint32_t nreloc,
    uint32_t flags,
    uint32_t reserved1,
    uint32_t reserved2,
    uint32_t cmd,
    char *sg_segname,
    uint32_t filetype,
    uint64_t object_size,
    enum bool verbose);

extern void print_symtab_command(
    struct symtab_command *sg,
    cpu_type_t cputype,
    uint64_t object_size);

extern void print_dysymtab_command(
    struct dysymtab_command *dyst,
    uint32_t nsyms,
    uint64_t object_size,
    cpu_type_t cputype);

extern void print_symseg_command(
    struct symseg_command *ss,
    uint64_t object_size);

extern void print_fvmlib_command(
    struct fvmlib_command *fl,
    struct load_command *lc,
    uint32_t left);

extern void print_dylib_command(
    struct dylib_command *dl,
    struct load_command *lc,
    uint32_t left);

extern void print_sub_framework_command(
    struct sub_framework_command *sub,
    struct load_command *lc,
    uint32_t left);

extern void print_sub_umbrella_command(
    struct sub_umbrella_command *usub,
    struct load_command *lc,
    uint32_t left);

extern void print_sub_library_command(
    struct sub_library_command *lsub,
    struct load_command *lc,
    uint32_t left);

extern void print_sub_client_command(
    struct sub_client_command *csub,
    struct load_command *lc,
    uint32_t left);

extern void print_prebound_dylib_command(
    struct prebound_dylib_command *pbdylib,
    struct load_command *lc,
    uint32_t left,
    enum bool verbose);

extern void print_dylinker_command(
    struct dylinker_command *dyld,
    struct load_command *lc,
    uint32_t left);

extern void print_fvmfile_command(
    struct fvmfile_command *ff,
    struct load_command *lc,
    uint32_t left);

extern void print_routines_command(
    struct routines_command *rc);

extern void print_routines_command_64(
    struct routines_command_64 *rc64);

extern void print_twolevel_hints_command(
    struct twolevel_hints_command *hints,
    uint64_t object_size);

extern void print_prebind_cksum_command(
    struct prebind_cksum_command *cksum);

extern void print_uuid_command(
    struct uuid_command *uuid);

extern void print_linkedit_data_command(
    struct linkedit_data_command *ld,
    uint64_t object_size);

extern void print_version_min_command(
    struct version_min_command *vd);

extern void print_build_version_command(
    struct build_version_command *bv,
    enum bool verbose);

extern void print_build_tool_version(
    uint32_t tool,
    uint32_t version,
    enum bool verbose);

extern void print_source_version_command(
    struct source_version_command *sv);

extern void print_note_command(
    struct note_command *nc,
    uint64_t object_size);

extern void print_entry_point_command(
    struct entry_point_command *ep);

extern void print_rpath_command(
    struct rpath_command *rpath,
    struct load_command *lc);

extern void print_encryption_info_command(
    struct encryption_info_command *ec,
    uint64_t object_size);

extern void print_encryption_info_command_64(
    struct encryption_info_command_64 *ec,
    uint64_t object_size);

extern void print_linker_option_command(
    struct linker_option_command *lo,
    struct load_command *lc,
    uint32_t cmdleft);

extern void print_dyld_info_info_command(
    struct dyld_info_command *dc,
    uint64_t object_size);

extern void print_thread_states(
    char *begin, 
    char *end,
    cpu_type_t cputype,
    enum byte_sex thread_states_byte_sex);

extern void print_cstring_section(
    cpu_type_t cputype,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    enum bool print_addresses);

extern void print_literal4_section(
    cpu_type_t cputype,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    enum byte_sex literal_byte_sex,
    enum bool print_addresses);

extern void print_literal8_section(
    cpu_type_t cputype,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    enum byte_sex literal_byte_sex,
    enum bool print_addresses);

extern void print_literal16_section(
    cpu_type_t cputype,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    enum byte_sex literal_byte_sex,
    enum bool print_addresses);

extern void print_literal_pointer_section(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    struct load_command *lc,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    uint32_t filetype,
    enum byte_sex object_byte_sex,
    char *addr,
    uint64_t object_size,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    enum bool print_addresses);

extern void print_init_term_pointer_section(
    cpu_type_t cputype,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    enum byte_sex object_byte_sex,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    enum bool verbose);

extern void print_shlib_init(
    enum byte_sex object_byte_sex,
    char *sect,
    uint64_t sect_size,
    uint64_t sect_addr,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    enum bool verbose);

extern enum bool print_symbol(
    uint64_t value,
    uint32_t r_address,
    uint32_t dot_value,
    struct relocation_info *sorted_relocs,
    uint32_t nsorted_relocs,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

extern const char *guess_symbol(
    const uint64_t value,
    const struct symbol *sorted_symbols,
    const uint32_t nsorted_symbols,
    const enum bool verbose);

extern const char * guess_indirect_symbol(
    const uint64_t value,
    const uint32_t ncmds,
    const uint32_t sizeofcmds,
    const struct load_command *load_commands,
    const enum byte_sex load_commands_byte_sex,
    const uint32_t *indirect_symbols,
    const uint32_t nindirect_symbols,
    const struct nlist *symbols,
    const struct nlist_64 *symbols64,
    const uint32_t nsymbols,
    const char *strings,
    const uint32_t strings_size);

extern void print_sect(
    cpu_type_t cputype,
    enum byte_sex object_byte_sex,
    char *sect,
    uint64_t size,
    uint64_t addr);

extern enum bool print_objc_segment(
    cpu_type_t mh_cputype,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    enum bool verbose);

extern void print_objc2_64bit(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    struct relocation_info *ext_relocs,
    uint32_t next_relocs,
    struct relocation_info *loc_relocs,
    uint32_t nloc_relocs,
    struct dyld_bind_info *dbi,
    uint64_t ndbi,
    enum chain_format_t chain_format,
    enum bool verbose,
    enum bool Vflag);

extern void print_objc2_32bit(
    cpu_type_t cputype,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist *symbols,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    struct relocation_info *ext_relocs,
    uint32_t next_relocs,
    struct relocation_info *loc_relocs,
    uint32_t nloc_relocs,
    struct dyld_bind_info* dbi,
    uint64_t ndbi,
    enum chain_format_t chain_format,
    enum bool verbose);

extern void print_objc_protocol_section(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    enum bool verbose);

extern void print_objc_string_object_section(
    char *sectname,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    enum bool verbose);

extern void print_objc_string_object_section_64(
    char *sectname,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    cpu_type_t cputype,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    const uint32_t strings_size,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    enum bool verbose);

extern void print_objc_runtime_setup_section(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    enum bool verbose);

extern void print_bitcode_section(
    char *sect,
    uint64_t sect_size,
    enum bool verbose,
    enum bool print_xar_header,
    enum bool print_xar_file_headers,
    const char *xar_member_name);

extern char *get_objc2_64bit_cfstring_name(
    uint64_t p,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    cpu_type_t cputype);

extern char *get_objc2_64bit_class_name(
    uint64_t p,
    uint64_t address_of_p,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist_64 *symbols64,
    uint32_t nsymbols, 
    char *strings,          
    uint32_t strings_size,  
    cpu_type_t cputype);

extern uint64_t get_objc2_64bit_selref(
    uint64_t address_of_p,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    cpu_type_t cputype);

extern void print_coff_reloc_section(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    uint32_t filetype,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    enum bool verbose);

extern char *get_label(
    uint64_t addr,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols);

extern enum bool print_label(
    uint64_t addr,
    enum bool colon_and_newline,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols);

extern enum bool get_sect_info(
    char *segname,
    char *sectname,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    uint32_t filetype,
    enum byte_sex load_commands_byte_sex,
    char *object_addr,
    uint64_t object_size,
    char **sect_pointer,
    uint64_t *sect_size,
    uint64_t *sect_addr,
    struct relocation_info **sect_relocs,
    uint32_t *sect_nrelocs,
    uint32_t *sect_flags,
    uint64_t *seg_addr);

extern void get_seg_names(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex load_commands_byte_sex,
    char* sectname,
    char*** segnames,
    uint32_t* nsegname);
