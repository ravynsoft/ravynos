#import <stuff/bytesex.h>
#import <mach-o/reloc.h>
#import <mach-o/nlist.h>
#import <stuff/bool.h>
#include "stuff/symbol.h"
#include "llvm-c/Disassembler.h"

extern uint32_t arm64_disassemble(
    char *sect,
    uint32_t left,
    uint64_t addr,
    uint32_t sect_addr,
    enum byte_sex object_byte_sex,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    struct relocation_info *ext_relocs,
    uint32_t next_relocs,
    struct relocation_info *loc_relocs,
    uint32_t nloc_relocs,
    struct dyld_bind_info *dbi,
    uint64_t ndbi,
    enum chain_format_t chain_format,
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
    char *object_addr,
    uint64_t object_size,
    enum bool verbose,
    LLVMDisasmContextRef dc);

extern LLVMDisasmContextRef create_arm64_llvm_disassembler(
    cpu_subtype_t cpusubtype);
extern void delete_arm64_llvm_disassembler(LLVMDisasmContextRef dc);
