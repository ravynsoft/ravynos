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
#import <stuff/bytesex.h>
#import <mach-o/reloc.h>
#import <mach-o/nlist.h>
#import <stuff/bool.h>
#include "otool.h"
#include "stuff/symbol.h"
#include "llvm-c/Disassembler.h"

extern uint32_t i386_disassemble(
    char *sect,
    uint32_t left,
    uint64_t addr,
    uint64_t sect_addr,
    enum byte_sex object_byte_sex,
    struct relocation_info *sorted_relocs,
    uint32_t nsorted_relocs,
    struct relocation_info *ext_relocs,
    uint32_t next_relocs,
    struct relocation_info *loc_relocs,
    uint32_t nloc_relocs,
    struct dyld_bind_info *dbi,
    uint64_t ndbi,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    uint32_t *indirect_symbols,
    uint32_t nindirect_symbols,
    cpu_type_t cputype,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum bool verbose,
    enum bool llvm_mc,
    LLVMDisasmContextRef i386_dc,
    LLVMDisasmContextRef x86_64_dc,
    char *object_addr,
    uint64_t object_size,
    struct inst *inst,
    struct inst *insts,
    uint32_t ninsts);

extern LLVMDisasmContextRef create_i386_llvm_disassembler(void);
extern void delete_i386_llvm_disassembler(LLVMDisasmContextRef dc);
extern LLVMDisasmContextRef create_x86_64_llvm_disassembler(void);
extern void delete_x86_64_llvm_disassembler(LLVMDisasmContextRef dc);
