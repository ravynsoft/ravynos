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
#include "stuff/symbol.h"
#include "llvm-c/Disassembler.h"

/* Used by otool(1) to stay or switch out of thumb mode */
extern enum bool in_thumb;

extern uint32_t arm_disassemble(
    char *sect,
    uint32_t left,
    uint32_t addr,
    uint32_t sect_addr,
    enum byte_sex object_byte_sex,
    struct relocation_info *sorted_relocs,
    uint32_t nsorted_relocs,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    uint32_t *indirect_symbols,
    uint32_t nindirect_symbols,
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    cpu_subtype_t cpu_subtype,
    enum bool verbose,
    LLVMDisasmContextRef arm_dc,
    LLVMDisasmContextRef thumb_dc,
    char *object_addr,
    uint64_t object_size,
    struct data_in_code_entry *dices,
    uint32_t ndices,
    uint64_t seg_addr,
    struct inst *inst,
    struct inst *insts,
    uint32_t ninsts);

extern LLVMDisasmContextRef create_arm_llvm_disassembler(
    cpu_subtype_t cpusubtype);
extern LLVMDisasmContextRef create_thumb_llvm_disassembler(
    cpu_subtype_t cpusubtype);
extern void delete_arm_llvm_disassembler(LLVMDisasmContextRef dc);
extern void delete_thumb_llvm_disassembler(LLVMDisasmContextRef dc);
