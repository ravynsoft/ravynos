#include <stdio.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <stdlib.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/arm64/reloc.h>
#include "stuff/bytesex.h"
#include "stuff/symbol.h"
#include "stuff/llvm.h"
#include "stuff/allocate.h"
#include "otool.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"
#include "arm64_disasm.h"
#include "cxa_demangle.h"

static struct disassemble_info {
  /* otool(1) specific stuff */
  enum bool verbose;
  /* Relocation information.  */
  struct relocation_info *relocs;
  uint32_t nrelocs;
  struct relocation_info *ext_relocs;
  uint32_t next_relocs;
  struct relocation_info *loc_relocs;
  uint32_t nloc_relocs;
  struct dyld_bind_info *dbi;
  uint64_t ndbi;
  /* Symbol table.  */
  struct nlist *symbols;
  struct nlist_64 *symbols64;
  uint32_t nsymbols;
  /* Symbols sorted by address.  */
  struct symbol *sorted_symbols;
  uint32_t nsorted_symbols;
  /* String table.  */
  char *strings;
  uint32_t strings_size;
  /* Other useful info.  */
  uint32_t ncmds;
  uint32_t sizeofcmds;
  struct load_command *load_commands;
  enum byte_sex object_byte_sex;
  uint32_t *indirect_symbols;
  uint32_t nindirect_symbols;
  cpu_type_t cputype;
  char *sect;
  uint32_t left;
  uint32_t addr;
  uint32_t sect_addr;
  uint64_t adrp_addr;
  uint32_t adrp_inst;
  char *object_addr;
  uint64_t object_size;
  const char *class_name;
  const char *selector_name;
  char *method;
  char *demangled_name;
  enum chain_format_t chain_format;
} dis_info;

/*
 * This is using the public llmv-c based interface.
 */

/*
 * set_OpInfo_from_reloc() is a helper routine for GetOpInfo() after it finds
 * a relocation entry for the operand it passes the relocation entry's
 * symbol index, r_symbolnum, and relocation type, r_type, and then this
 * routine fills in the op_info fields as needed using the symbol table info
 * from dis_info.  If symbolic information set into op_info then this function
 * returns 1 else it returns 0.
 */
static
int
set_OpInfo_from_reloc(
uint32_t r_symbolnum,
uint32_t r_type,
struct disassemble_info *dis_info,
struct LLVMOpInfo1 *op_info)
{
    uint32_t n_strx;

	if(r_symbolnum >= dis_info->nsymbols)
	    return(0);
	if(dis_info->cputype == CPU_TYPE_ARM64)
	    n_strx = dis_info->symbols64[r_symbolnum].n_un.n_strx;
	else
	    n_strx = dis_info->symbols[r_symbolnum].n_un.n_strx;
	if(n_strx <= 0 || n_strx >= dis_info->strings_size)
	    return(0);
	op_info->AddSymbol.Present = 1;
	op_info->AddSymbol.Name = dis_info->strings + n_strx;
	switch(r_type){
	case ARM64_RELOC_PAGE21:
	    /* @page */
	    op_info->VariantKind = LLVMDisassembler_VariantKind_ARM64_PAGE;
	    break;
	case ARM64_RELOC_PAGEOFF12:
	    /* @pageoff */
	    op_info->VariantKind = LLVMDisassembler_VariantKind_ARM64_PAGEOFF;
	    break;
	case ARM64_RELOC_GOT_LOAD_PAGE21:
	    /* @gotpage */
	    op_info->VariantKind = LLVMDisassembler_VariantKind_ARM64_GOTPAGE;
	    break;
	case ARM64_RELOC_GOT_LOAD_PAGEOFF12:
	    /* @gotpageoff */
	    op_info->VariantKind =LLVMDisassembler_VariantKind_ARM64_GOTPAGEOFF;
	    break;
	case ARM64_RELOC_TLVP_LOAD_PAGE21:
	    /* @tvlppage is not implemented in llvm-mc */
	    op_info->VariantKind = LLVMDisassembler_VariantKind_ARM64_TLVP;
	    break;
	case ARM64_RELOC_TLVP_LOAD_PAGEOFF12:
	    /* @tvlppageoff is not implemented in llvm-mc */
	    op_info->VariantKind = LLVMDisassembler_VariantKind_ARM64_TLVOFF;
	    break;
	default:
	case ARM64_RELOC_BRANCH26:
	    op_info->VariantKind = LLVMDisassembler_VariantKind_None;
	    break;
	}
	return(1);
}

/*
 * GetOpInfo() is the operand information call back function.  This is called to
 * get the symbolic information for an operand of an arm64 instruction.  This
 * is done from the relocation information, symbol table, etc.  That block of
 * information is a pointer to the struct disassemble_info that was passed when
 * the disassembler context was created and passed to back to GetOpInfo() when
 * called back by LLVMDisasmInstruction().  For arm64 there the instruction
 * containing operand is at the PC parameter.  Since for arm64 it only has one
 * one operand with symbolic information the Offset parameter is zero and the
 * Size parameter is 4, the instruction width.  The information is returned in
 * TagBuf and for the arm64-apple-darwin10 Triple is the LLVMOpInfo1 struct
 * defined in "llvm-c/Disassembler.h".  The value of TagType for
 * arm64-apple-darwin10 is 1. If symbolic information is returned then this
 * function returns 1 else it returns 0.
 */
static
int
GetOpInfo(
void *DisInfo,
uint64_t Pc,
uint64_t Offset, /* should always be passed as 0 for arm64 */
uint64_t Size,   /* should always be passed as 4 for arm64 */
int TagType,     /* should always be passed as 1 for arm64-apple-darwin10 */
void *TagBuf)
{
    uint32_t i, sect_offset;
    uint64_t value;
    struct disassemble_info *dis_info;
    struct LLVMOpInfo1 *op_info;

	dis_info = (struct disassemble_info *)DisInfo;

	op_info = (struct LLVMOpInfo1 *)TagBuf;
	value = op_info->Value;
	/* make sure all feilds returned are zero if we don't set them */
	memset(op_info, '\0', sizeof(struct LLVMOpInfo1));
	op_info->Value = value;

	if(Offset != 0 || Size != 4 || TagType != 1 ||
	   dis_info->verbose == FALSE)
	    return(0);

	/* First look in the section relocations if any. */
	sect_offset = (uint32_t)(Pc - dis_info->sect_addr);
	for(i = 0; i < dis_info->nrelocs; i++){
	    if(dis_info->relocs[i].r_address == sect_offset){
		if(dis_info->relocs[i].r_type == ARM64_RELOC_ADDEND){
		    if(i+1 < dis_info->nrelocs &&
		       dis_info->relocs[i+1].r_address == sect_offset){
			if(value == 0){
			    value = dis_info->relocs[i].r_symbolnum;
			    op_info->Value = value;
			}
			i++;
		    }
		    else
			continue;
		}
		if(dis_info->relocs[i].r_extern)
		    return(set_OpInfo_from_reloc(
				dis_info->relocs[i].r_symbolnum,
				dis_info->relocs[i].r_type, dis_info, op_info));
	    }
	}

	/* Second look in the image's external relocations if any. */
	for(i = 0; i < dis_info->next_relocs; i++){
	    if(dis_info->ext_relocs[i].r_address == Pc){
		if(dis_info->ext_relocs[i].r_type == ARM64_RELOC_ADDEND){
		    if(i+1 < dis_info->next_relocs &&
		       dis_info->ext_relocs[i+1].r_address == Pc){
			if(value == 0){
			    value = dis_info->ext_relocs[i].r_symbolnum;
			    op_info->Value = value;
			}
			i++;
		    }
		    else
			continue;
		}
		return(set_OpInfo_from_reloc(
			    dis_info->ext_relocs[i].r_symbolnum,
			    dis_info->ext_relocs[i].r_type, dis_info,
			    op_info));
	    }
	}

	return(0);
}

/*
 * guess_pointer_pointer() is passed the address of what might be a pointer to
 * a reference to an Objective-C class, selector, message ref or cfstring.
 */
static
uint64_t
guess_pointer_pointer(
const uint64_t value,
const uint32_t ncmds,
const uint32_t sizeofcmds,
const struct load_command *load_commands,
const enum byte_sex load_commands_byte_sex,
const char *object_addr,
const uint64_t object_size,
enum bool *classref,
enum bool *selref,
enum bool *msgref,
enum bool *cfstring)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, section_type, pointer_value;
    uint64_t sect_offset, object_offset, pointer_value64;
    const struct load_command *lc;
    struct load_command l;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct segment_command sg;
    struct section s;
    char *p;
    uint64_t big_load_end;

	*classref = FALSE;
	*selref = FALSE;
	*msgref = FALSE;
	*cfstring = FALSE;
	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		return(0);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		return(0);
	    switch(l.cmd){
	    case LC_SEGMENT_64:
		memcpy((char *)&sg64, (char *)lc,
		       sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    memcpy((char *)&s64, p, sizeof(struct section_64));
		    p += sizeof(struct section_64);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    section_type = s64.flags & SECTION_TYPE;
		    if((strncmp(s64.sectname, "__objc_selrefs", 16) == 0 ||
		        strncmp(s64.sectname, "__objc_classrefs", 16) == 0 ||
		        strncmp(s64.sectname, "__objc_superrefs", 16) == 0 ||
			strncmp(s64.sectname, "__objc_msgrefs", 16) == 0 ||
			strncmp(s64.sectname, "__cfstring", 16) == 0) &&
		       value >= s64.addr && value < s64.addr + s64.size){
			sect_offset = value - s64.addr;
			object_offset = s64.offset + sect_offset;
			if(object_offset < object_size){
			    memcpy(&pointer_value64, object_addr + object_offset,
				   sizeof(uint64_t));
			    if(swapped)
				pointer_value64 = SWAP_LONG_LONG(pointer_value64);
			    if(strncmp(s64.sectname,
				       "__objc_selrefs", 16) == 0)
				*selref = TRUE; 
			    else if(strncmp(s64.sectname,
				       "__objc_classrefs", 16) == 0 ||
			            strncmp(s64.sectname,
				       "__objc_superrefs", 16) == 0)
				*classref = TRUE; 
			    else if(strncmp(s64.sectname,
				       "__objc_msgrefs", 16) == 0 &&
			     value + 8 < s64.addr + s64.size){
				*msgref = TRUE; 
				memcpy(&pointer_value64,
				       object_addr + object_offset + 8,
				       sizeof(uint64_t));
				if(swapped)
				    pointer_value64 =
					SWAP_LONG_LONG(pointer_value64);
			    }
			    else if(strncmp(s64.sectname,
				       "__cfstring", 16) == 0)
				*cfstring = TRUE; 
			    return(pointer_value64);
			}
			else
			    return(0);
		    }
		}
		break;
	    case LC_SEGMENT:
		memcpy((char *)&sg, (char *)lc,
		       sizeof(struct segment_command));
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    memcpy((char *)&s, p, sizeof(struct section));
		    p += sizeof(struct section);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);
		    section_type = s.flags & SECTION_TYPE;
		    if((strncmp(s.sectname, "__objc_selrefs", 16) == 0 ||
		        strncmp(s.sectname, "__objc_classrefs", 16) == 0 ||
		        strncmp(s.sectname, "__objc_superrefs", 16) == 0 ||
			strncmp(s.sectname, "__objc_msgrefs", 16) == 0 ||
			strncmp(s.sectname, "__cfstring", 16) == 0) &&
		       value >= s.addr && value < s.addr + s.size){
			sect_offset = value - s.addr;
			object_offset = s.offset + sect_offset;
			if(object_offset < object_size){
			    memcpy(&pointer_value, object_addr + object_offset,
				   sizeof(uint32_t));
			    if(swapped)
				pointer_value = SWAP_INT(pointer_value);
			    if(strncmp(s.sectname,
				       "__objc_selrefs", 16) == 0)
				*selref = TRUE; 
			    else if(strncmp(s.sectname,
				       "__objc_classrefs", 16) == 0 ||
			            strncmp(s.sectname,
				       "__objc_superrefs", 16) == 0)
				*classref = TRUE; 
			    else if(strncmp(s.sectname,
				       "__objc_msgrefs", 16) == 0 &&
			     value + 4 < s.addr + s.size){
				*msgref = TRUE; 
				memcpy(&pointer_value,
				       object_addr + object_offset + 4,
				       sizeof(uint32_t));
				if(swapped)
				    pointer_value =
					SWAP_INT(pointer_value);
			    }
			    else if(strncmp(s.sectname,
				       "__cfstring", 16) == 0)
				*cfstring = TRUE; 
			    return(pointer_value);
			}
			else
			    return(0);
		    }
		}
		break;
	    }
	    if(l.cmdsize == 0){
		return(0);
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return(0);
	}
	return(0);
}

/*
 * guess_cstring_pointer() is passed the address of what might be a pointer to a
 * literal string in a cstring section.  If that address is in a cstring section
 * it returns a pointer to that string.  Else it returns NULL.
 */
static
const char *
guess_cstring_pointer(
const uint64_t value,
const uint32_t ncmds,
const uint32_t sizeofcmds,
const struct load_command *load_commands,
const enum byte_sex load_commands_byte_sex,
const char *object_addr,
const uint64_t object_size)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, section_type;
    uint64_t sect_offset, object_offset;
    const struct load_command *lc;
    struct load_command l;
    struct segment_command_64 sg64;
    struct section_64 s64;
    char *p;
    uint64_t big_load_end;
    const char *name;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		return(NULL);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		return(NULL);
	    switch(l.cmd){
	    case LC_SEGMENT_64:
		memcpy((char *)&sg64, (char *)lc,
		       sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    memcpy((char *)&s64, p, sizeof(struct section_64));
		    p += sizeof(struct section_64);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    section_type = s64.flags & SECTION_TYPE;
		    if(section_type == S_CSTRING_LITERALS &&
		       value >= s64.addr && value < s64.addr + s64.size){
			sect_offset = value - s64.addr;
			object_offset = s64.offset + sect_offset;
			if(object_offset < object_size){
			    name = object_addr + object_offset;
			    return(name);
			}
			else
			    return(NULL);
		    }
		}
		break;
	    }
	    if(l.cmdsize == 0){
		return(NULL);
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return(NULL);
	}
	return(NULL);
}

/*
 * guess_literal_pointer() returns a pointer to a literal string if the value
 * passed in is the address of a literal pointer and the literal pointer's value
 * is an address of a cstring.  
 */
static
const char *
guess_literal_pointer(
uint64_t value,	  	  /* the value of the reference */
uint64_t *reference_type, /* type returned, symbol name or string literal*/
struct disassemble_info *info)
{
    uint32_t ncmds, sizeofcmds;
    struct load_command *load_commands;
    enum byte_sex object_byte_sex;
    char *object_addr;
    uint64_t object_size, pointer_value;
    const char *name, *class_name;
    enum bool classref, selref, msgref, cfstring;

	ncmds = info->ncmds;
	sizeofcmds = info->sizeofcmds;
	load_commands = info->load_commands;
	object_byte_sex = info->object_byte_sex;
	object_addr = info->object_addr;
	object_size = info->object_size;

	pointer_value = guess_pointer_pointer(value, ncmds, sizeofcmds,
			    load_commands, object_byte_sex, object_addr,
			    object_size, &classref, &selref, &msgref,
			    &cfstring);

	if(classref == TRUE && pointer_value == 0){
	    /*
	     * Note the value is a pointer into the __objc_classrefs section.
	     * And the pointer_value in that section is typically zero as it
	     * will be set by dyld as part of the "bind information".
	     */
	    name = get_dyld_bind_info_symbolname(value, info->dbi, info->ndbi,
						 info->chain_format,
						 NULL);
	    if(name != NULL){
		*reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Class_Ref;
		class_name = rindex(name, '$');
		if(class_name != NULL &&
		   class_name[1] == '_' && class_name[2] != '\0')
		    info->class_name = class_name + 2;
		return(name);
	    }
	}

	if(classref == TRUE){
	    *reference_type =
		LLVMDisassembler_ReferenceType_Out_Objc_Class_Ref;
	    if(info->cputype == CPU_TYPE_ARM64){
		name = get_objc2_64bit_class_name(pointer_value, value,
			    info->load_commands, info->ncmds, info->sizeofcmds,
			    info->object_byte_sex, info->object_addr,
			    info->object_size, info->symbols64, info->nsymbols,
			    info->strings, info->strings_size, CPU_TYPE_ARM64);
		if(name != NULL)
		    info->class_name = name;
		else
		    name = "bad class ref";
	    } else {
		name = "TODO: arm64_32 get_objc2_64_32bit_class_name()";
	    }
	    return(name);
	}

	if(cfstring == TRUE){
	    *reference_type =
		LLVMDisassembler_ReferenceType_Out_Objc_CFString_Ref;
	    if(info->cputype == CPU_TYPE_ARM64){
		name = get_objc2_64bit_cfstring_name(value,
			    info->load_commands, info->ncmds, info->sizeofcmds,
			    info->object_byte_sex, info->object_addr,
			    info->object_size, info->symbols64, info->nsymbols,
			    info->strings, info->strings_size, CPU_TYPE_ARM64);
		if(name == NULL)
		    name = "bad cfstring ref";
	    } else {
		name = "TODO: arm64_32 get_objc2_64_32bit_cfstring_name()";
	    }
	    return(name);
	}

	if(pointer_value != 0)
	    value = pointer_value;

	/*
	 * See if the value is pointing to a cstring.
	 */
	name = guess_cstring_pointer(value, ncmds, sizeofcmds, load_commands,
				     object_byte_sex, object_addr, object_size);
	if(name != NULL){
	    if(pointer_value != 0 && selref == TRUE){
	        *reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Selector_Ref;
		info->selector_name = name;
	    }
	    else if(pointer_value != 0 && msgref == TRUE){
		info->class_name = NULL;
	        *reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message_Ref;
		info->selector_name = name;
	    }
            else
		*reference_type =
		    LLVMDisassembler_ReferenceType_Out_LitPool_CstrAddr;
	    return(name);
	}

	name = guess_indirect_symbol(value, ncmds, sizeofcmds, load_commands,
		object_byte_sex, info->indirect_symbols,info->nindirect_symbols,
		info->symbols, info->symbols64, info->nsymbols, info->strings,
		info->strings_size);
	if(name != NULL){
	    *reference_type =
		    LLVMDisassembler_ReferenceType_Out_LitPool_SymAddr;
	    return(name);
	}

	return(NULL);
}

/*
 * method_reference() is called passing it the ReferenceName that might be
 * a reference it to an Objective-C method.  If so then it allocates and
 * assembles a method call string with the values last seen and saved in
 * the disassemble_info's class_name and selector_name fields.  This is saved
 * into the method field and any previous string is free'ed.  Then the
 * class_name field is NULL'ed out.
 */
static
void
method_reference(
struct disassemble_info *info,
uint64_t *ReferenceType,
const char **ReferenceName)
{
	if(*ReferenceName != NULL){
	    if(strcmp(*ReferenceName, "_objc_msgSend") == 0){
		*ReferenceType =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message;
		if(info->selector_name != NULL){
		    if(info->method != NULL)
			free(info->method);
		    if(info->class_name != NULL){
			info->method =
			    allocate(5 + strlen(info->class_name) +
				    strlen(info->selector_name));
			strcpy(info->method, "+[");
			strcat(info->method, info->class_name);
			strcat(info->method, " ");
			strcat(info->method, info->selector_name);
			strcat(info->method, "]");
			*ReferenceName = info->method;
			info->class_name = NULL;
		    }
		    else{
			info->method =
			    allocate(7 + strlen(info->selector_name));
			strcpy(info->method, "-[x0 ");
			strcat(info->method, info->selector_name);
			strcat(info->method, "]");
			*ReferenceName = info->method;
		    }
		}
	    }
	    else if(strcmp(*ReferenceName, "_objc_msgSendSuper2") == 0){
		*ReferenceType =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message;
		if(info->selector_name != NULL){
		    if(info->method != NULL)
			free(info->method);
		    info->method =
			allocate(15 + strlen(info->selector_name));
		    strcpy(info->method, "-[[x0 super] ");
		    strcat(info->method, info->selector_name);
		    strcat(info->method, "]");
		    *ReferenceName = info->method;
		    info->class_name = NULL;
		}
	    }
	}
}

/*
 * The symbol lookup function passed to LLVMCreateDisasm().  It looks up the
 * SymbolValue using the info passed vis the pointer to the struct
 * disassemble_info that was passed when disassembler context is created and
 * returns the symbol name that matches or NULL if none.
 *
 * When this is called to get a symbol name for a branch target then the
 * ReferenceType can be LLVMDisassembler_ReferenceType_In_Branch and then
 * SymbolValue will be looked for in the indirect symbol table to determine if
 * it is an address for a symbol stub.  If so then the symbol name for that
 * stub is returned indirectly through ReferenceName and then ReferenceType is
 * set to LLVMDisassembler_ReferenceType_Out_SymbolStub.
 */
static
const char *
SymbolLookUp(
void *DisInfo,
uint64_t SymbolValue,
uint64_t *ReferenceType,
uint64_t ReferencePC,
const char **ReferenceName)
{
    struct disassemble_info *info;
    const char *symbol_name, *indirect_symbol_name;

	info = (struct disassemble_info *)DisInfo;
	symbol_name = guess_symbol(SymbolValue, info->sorted_symbols,
				   info->nsorted_symbols, TRUE);

	if(*ReferenceType == LLVMDisassembler_ReferenceType_In_Branch){
	    indirect_symbol_name = guess_indirect_symbol(SymbolValue,
		    info->ncmds, info->sizeofcmds, info->load_commands,
		    info->object_byte_sex, info->indirect_symbols,
		    info->nindirect_symbols, info->symbols, info->symbols64,
		    info->nsymbols, info->strings, info->strings_size);
	    if(indirect_symbol_name != NULL){
		*ReferenceName = indirect_symbol_name;
		method_reference(info, ReferenceType, ReferenceName);
		if(*ReferenceType !=
			LLVMDisassembler_ReferenceType_Out_Objc_Message)
		    *ReferenceType =
			LLVMDisassembler_ReferenceType_Out_SymbolStub;
	    }
	    else if(symbol_name != NULL && strncmp(symbol_name, "__Z", 3) == 0){
		if(info->demangled_name != NULL)
		    free(info->demangled_name);
		info->demangled_name = __cxa_demangle(symbol_name + 1, 0, 0, 0);
		if(info->demangled_name != NULL){
		    *ReferenceName = info->demangled_name;
		    *ReferenceType =
			LLVMDisassembler_ReferenceType_DeMangled_Name;
		}
		else{
		    *ReferenceName = NULL;
		    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
		}
	    }
	    else{
		*ReferenceName = NULL;
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	    }
	}
	/*
	 * If this reference is an adrp instruction save the instruction, pass
	 * in SymbolValue and the address of the instruction for use later if
	 * we see and add immediate instruction.
	 */
	else if(*ReferenceType == LLVMDisassembler_ReferenceType_In_ARM64_ADRP){
	    info->adrp_inst = (uint32_t)SymbolValue;
	    info->adrp_addr = ReferencePC;
	    symbol_name = NULL;
	    *ReferenceName = NULL;
	    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	/*
	 * If this reference is an add immediate instruction and we have seen
	 * an adrp instruction just before it and the adrp's Xd register
	 * matches this add's Xn register reconstruct the value being
	 * referenced and look to see if it is a literal pointer.  Note the
	 * add immediate instruction is passed in SymbolValue.
	 */
	else if(*ReferenceType ==
		LLVMDisassembler_ReferenceType_In_ARM64_ADDXri &&
		ReferencePC - 4 == info->adrp_addr &&
		(info->adrp_inst & 0x9f000000) == 0x90000000 &&
		(info->adrp_inst & 0x1f) == ((SymbolValue >> 5) & 0x1f) ){
	    uint64_t addxri_inst;
	    uint64_t adrp_imm, addxri_imm;

	    adrp_imm = ((info->adrp_inst & 0x00ffffe0) >> 3) |
		       ((info->adrp_inst >> 29) & 0x3);
	    if(info->adrp_inst & 0x0200000)
		adrp_imm |= 0xfffffffffc000000LL;

	    addxri_inst = SymbolValue;
	    addxri_imm = (addxri_inst >> 10) & 0xfff;
	    if(((addxri_inst >> 22) & 0x3) == 1)
		addxri_imm <<= 12;

	    SymbolValue = (info->adrp_addr & 0xfffffffffffff000LL) +
			  (adrp_imm << 12) + addxri_imm;

	    *ReferenceName = guess_literal_pointer(SymbolValue, ReferenceType,
						   info);
	    if(*ReferenceName == NULL)
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	/*
	 * If this reference is a load register instruction and we have seen
	 * an adrp instruction just before it and the adrp's Xd register
	 * matches this add's Xn register reconstruct the value being
	 * referenced and look to see if it is a literal pointer.  Note the
	 * load register instruction is passed in SymbolValue.
	 */
	else if(*ReferenceType ==
		LLVMDisassembler_ReferenceType_In_ARM64_LDRXui &&
		ReferencePC - 4 == info->adrp_addr &&
		(info->adrp_inst & 0x9f000000) == 0x90000000 &&
		(info->adrp_inst & 0x1f) == ((SymbolValue >> 5) & 0x1f) ){
	    uint64_t ldrxui_inst;
	    uint64_t adrp_imm, ldrxui_imm;

	    adrp_imm = ((info->adrp_inst & 0x00ffffe0) >> 3) |
		       ((info->adrp_inst >> 29) & 0x3);
	    if(info->adrp_inst & 0x0200000)
		adrp_imm |= 0xfffffffffc000000LL;

	    ldrxui_inst = SymbolValue;
	    ldrxui_imm = (ldrxui_inst >> 10) & 0xfff;

	    SymbolValue = (info->adrp_addr & 0xfffffffffffff000LL) +
			  (adrp_imm << 12) + (ldrxui_imm << 3);

	    *ReferenceName = guess_literal_pointer(SymbolValue, ReferenceType,
						   info);
	    if(*ReferenceName == NULL)
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	/*
	 * If this is an load register (PC-relative) instruction the SymbolValue
	 * is the PC plus the immediate value.
	 */
	else if(*ReferenceType ==
		LLVMDisassembler_ReferenceType_In_ARM64_LDRXl ||
		*ReferenceType ==
		LLVMDisassembler_ReferenceType_In_ARM64_ADR){
	    *ReferenceName = guess_literal_pointer(SymbolValue, ReferenceType,
						   info);
	    if(*ReferenceName == NULL)
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	else if(symbol_name != NULL && strncmp(symbol_name, "__Z", 3) == 0){
	    if(info->demangled_name != NULL)
		free(info->demangled_name);
	    info->demangled_name = __cxa_demangle(symbol_name + 1, 0, 0, 0);
	    if(info->demangled_name != NULL){
		*ReferenceName = info->demangled_name;
		*ReferenceType = LLVMDisassembler_ReferenceType_DeMangled_Name;
	    }
	}
	else{
	    *ReferenceName = NULL;
	    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	return(symbol_name);
}

LLVMDisasmContextRef
create_arm64_llvm_disassembler(
cpu_subtype_t cpusubtype)
{
    LLVMDisasmContextRef dc;
    char *mcpu_default;

	mcpu_default = mcpu;
	switch(cpusubtype){
	case CPU_SUBTYPE_ARM64_ALL:
	    if(*mcpu_default == '\0')
		mcpu_default = "cyclone";
	    break;
	case CPU_SUBTYPE_ARM64E:
	    if(*mcpu_default == '\0')
		mcpu_default = "vortex";
	    break;
	}

	dc = 
#ifdef STATIC_LLVM
	LLVMCreateDisasm
#else
	llvm_create_disasm
#endif
	    ("arm64-apple-darwin10", mcpu_default, &dis_info, 1, GetOpInfo,
	     SymbolLookUp);
	return(dc);
}

void
delete_arm64_llvm_disassembler(
LLVMDisasmContextRef dc)
{
#ifdef STATIC_LLVM
	LLVMDisasmDispose
#else
	llvm_disasm_dispose
#endif
	    (dc);
}

uint32_t
arm64_disassemble(
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
LLVMDisasmContextRef dc)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t opcode;
    uint32_t sect_offset;
    char dst[4096];

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;
	sect_offset = (uint32_t)(addr - sect_addr);

	if(left < sizeof(uint32_t)){
	   if(left != 0){
		memcpy(&opcode, sect, left);
		if(swapped)
		    opcode = SWAP_INT(opcode);
		printf(".long\t0x%08x\n", (unsigned int)opcode);
	   }
	   printf("(end of section)\n");
	   return(left);
	}

	memcpy(&opcode, sect, sizeof(uint32_t));
	if(swapped)
	    opcode = SWAP_INT(opcode);

	if(!Xflag && jflag && !no_show_raw_insn)
	    printf("\t%08x", opcode);

	dis_info.verbose = verbose;
	dis_info.relocs = relocs;
	dis_info.nrelocs = nrelocs;
	dis_info.ext_relocs = ext_relocs;
	dis_info.next_relocs = next_relocs;
	dis_info.loc_relocs = loc_relocs;
	dis_info.nloc_relocs = nloc_relocs;
	dis_info.dbi = dbi;
	dis_info.ndbi = ndbi;
	dis_info.symbols = symbols;
	dis_info.symbols64 = symbols64;
	dis_info.nsymbols = nsymbols;
	dis_info.sorted_symbols = sorted_symbols;
	dis_info.nsorted_symbols = nsorted_symbols;
	dis_info.strings = strings;
	dis_info.strings_size = strings_size;
	dis_info.load_commands = load_commands;
	dis_info.object_byte_sex = object_byte_sex;
	dis_info.indirect_symbols = indirect_symbols;
	dis_info.nindirect_symbols = nindirect_symbols;
	dis_info.cputype = cputype;
	dis_info.ncmds = ncmds;
	dis_info.sizeofcmds = sizeofcmds;
	dis_info.object_addr = object_addr;
	dis_info.object_size = object_size;
	dis_info.sect = sect;
	dis_info.left = left;
	dis_info.addr = (uint32_t)addr;
	dis_info.sect_addr = sect_addr;
	dis_info.method = NULL;
	dis_info.demangled_name = NULL;
	dis_info.chain_format = chain_format;

	dst[4095] = '\0';
	if(llvm_disasm_instruction(dc, (uint8_t *)sect, 4, addr, dst, 4095) != 0)
	    printf("%s\n", dst);
	else
	    printf("\t.long\t0x%08x\n", opcode);
	return(4);
}
