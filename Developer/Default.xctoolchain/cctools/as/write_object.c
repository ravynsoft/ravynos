#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include <libc.h>
#include <mach/mach.h>
#include "arch64_32.h"
#include "stuff/openstep_mach.h"
#include <mach-o/loader.h>
#include <mach-o/reloc.h>
#include <mach-o/stab.h>
#ifdef I860
#include <mach-o/i860/reloc.h>
#endif
#ifdef M88K
#include <mach-o/m88k/reloc.h>
#endif
#ifdef PPC
#include <mach-o/ppc/reloc.h>
#endif
#ifdef HPPA
#include <mach-o/hppa/reloc.h>
#include "stuff/hppa.h"
#endif
#ifdef SPARC
#include <mach-o/sparc/reloc.h>
#endif
#ifdef ARM
#include <mach-o/arm/reloc.h>
#include "arm_reloc.h"
#endif
#if defined(I386) && defined(ARCH64)
#include <mach-o/x86_64/reloc.h>
#endif
#include "stuff/rnd.h"
#include "stuff/bytesex.h"
#include "stuff/errors.h"
#include "as.h"
#include "struc-symbol.h"
#include "symbols.h"
#include "frags.h"
#include "fixes.h"
#include "md.h"
#include "sections.h"
#include "messages.h"
#include "xmalloc.h"
#include "input-scrub.h"
#include "stuff/write64.h"
#if defined(I386) && defined(ARCH64)
#include "i386.h"
#endif
#ifdef I860
#define RELOC_SECTDIFF		I860_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	I860_RELOC_SECTDIFF
#define RELOC_PAIR		I860_RELOC_PAIR
#endif
#ifdef M88K
#define RELOC_SECTDIFF		M88K_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	M88K_RELOC_SECTDIFF
#define RELOC_PAIR		M88K_RELOC_PAIR
#endif
#ifdef PPC
#define RELOC_SECTDIFF		PPC_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	PPC_RELOC_LOCAL_SECTDIFF
#define RELOC_PAIR		PPC_RELOC_PAIR
#define PPC_RELOC_BR14_predicted (0x10 | PPC_RELOC_BR14)
#endif
#ifdef HPPA
#define RELOC_SECTDIFF		HPPA_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	HPPA_RELOC_SECTDIFF
#define RELOC_PAIR		HPPA_RELOC_PAIR
#endif
#ifdef SPARC
#define RELOC_SECTDIFF		SPARC_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	SPARC_RELOC_SECTDIFF
#define RELOC_PAIR		SPARC_RELOC_PAIR
#endif
#if defined(M68K) || defined(I386)
#undef  RELOC_SECTDIFF /* cctools-port: need to undef these to avoid warnings */
#undef  RELOC_LOCAL_SECTDIFF
#undef  RELOC_PAIR
#define RELOC_SECTDIFF		GENERIC_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	GENERIC_RELOC_LOCAL_SECTDIFF
#define RELOC_PAIR		GENERIC_RELOC_PAIR
#endif
#ifdef ARM
#undef  RELOC_SECTDIFF /* cctools-port: need to undef these to avoid warnings */
#undef  RELOC_LOCAL_SECTDIFF
#undef  RELOC_PAIR
#define RELOC_SECTDIFF		ARM_RELOC_SECTDIFF
#define RELOC_LOCAL_SECTDIFF	ARM_RELOC_SECTDIFF
#define RELOC_PAIR		ARM_RELOC_PAIR
#endif

/*
 * These variables are set by layout_symbols() to organize the symbol table and
 * string table in order the dynamic linker expects.  They are then used in
 * write_object() to put out the symbols and strings in that order.
 * The order of the symbol table is:
 *	local symbols
 *	defined external symbols (sorted by name)
 *	undefined external symbols (sorted by name)
 * The order of the string table is:
 *	strings for external symbols
 *	strings for local symbols
 */
/* index to and number of local symbols */
static uint32_t ilocalsym = 0;
static uint32_t nlocalsym = 0;
/* index to, number of and array of sorted externally defined symbols */
static uint32_t iextdefsym = 0;
static uint32_t nextdefsym = 0;
static symbolS **extdefsyms = NULL;
/* index to, number of and array of sorted undefined symbols */
static uint32_t iundefsym = 0;
static uint32_t nundefsym = 0;
static symbolS **undefsyms = NULL;

static uint32_t layout_indirect_symbols(
     void);
static void layout_symbols(
    int32_t *symbol_number,
    int32_t *string_byte_count);
static int qsort_compare(
    const symbolS **sym1,
    const symbolS **sym2);
static uint32_t nrelocs_for_fix(
    struct fix *fixP);
static uint32_t fix_to_relocation_entries(
    struct fix *fixP,
    uint64_t sect_addr,
    struct relocation_info *riP,
    uint32_t debug_section);
#ifdef I860
static void
    I860_tweeks(void);
#endif

/*
 * write_object() writes a Mach-O object file from the built up data structures.
 */
void
write_object(
char *out_file_name)
{
    /* The structures for Mach-O relocatables */
    mach_header_t		header;
    segment_command_t		reloc_segment;
    struct symtab_command	symbol_table;
    struct dysymtab_command	dynamic_symbol_table;
    uint32_t			section_type;
    uint32_t			*indirect_symbols;
    isymbolS			*isymbolP;
    uint32_t			i, j, nsects, nsyms, strsize, nindirectsyms;

    /* locals to fill in section struct fields */
    uint32_t offset, zero;

    /* The GAS data structures */
    struct frchain *frchainP, *p;
    struct symbol *symbolP;
    struct frag *fragP;
    struct fix *fixP;

    uint32_t output_size;
    char *output_addr;
    kern_return_t r;

    enum byte_sex host_byte_sex;
    uint32_t reloff, nrelocs;
    int32_t count;
    char *fill_literal;
    int32_t fill_size;
    int32_t num_bytes;
    char *symbol_name;
    int fd;
    uint32_t local;
    struct stat stat_buf;

#ifdef I860
	I860_tweeks();
#endif
	i = 0; /* to shut up a compiler "may be used uninitialized" warning */

	/*
	 * The first group of things to do is to set all the fields in the
	 * header structures which includes offsets and determining the final
	 * sizes of things.
	 */

	/* 
	 * Fill in the addr and size fields of each section structure and count
	 * the number of sections.
	 */
	nsects = 0;
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    frchainP->frch_section.addr = (uint32_t)
		frchainP->frch_root->fr_address;
	    frchainP->frch_section.size = (uint32_t)
		(frchainP->frch_last->fr_address -
		 frchainP->frch_root->fr_address);
	    nsects++;
	}

	/*
	 * Setup the indirect symbol tables by looking up or creating symbol
	 * from the indirect symbol names and recording the symbol pointers.
	 */
	nindirectsyms = layout_indirect_symbols();

	/*
	 * Setup the symbol table to include only those symbols that will be in
	 * the object file, assign the string table offsets into the symbols
	 * and size the string table.
	 */
	nsyms = 0;
	strsize = 0;
	layout_symbols((int32_t *)&nsyms, (int32_t *)&strsize);

	/* fill in the Mach-O header */
	header.magic = MH_MAGIC_VALUE;
	header.cputype = md_cputype;
	if(archflag_cpusubtype != -1)
	    header.cpusubtype = archflag_cpusubtype;
	else
	    header.cpusubtype = md_cpusubtype;

	header.filetype = MH_OBJECT;
	header.ncmds = 0;
	header.sizeofcmds = 0;
	if(nsects != 0){
	    header.ncmds += 1;
	    header.sizeofcmds += sizeof(segment_command_t) +
				 nsects * sizeof(section_t);
	}
	if(nsyms != 0){
	    header.ncmds += 1;
	    header.sizeofcmds += sizeof(struct symtab_command);
	    if(flagseen['k']){
		header.ncmds += 1;
		header.sizeofcmds += sizeof(struct dysymtab_command);
	    }
	}
	else
	    strsize = 0;
	header.flags = 0;
	if(subsections_via_symbols == TRUE)
	    header.flags |= MH_SUBSECTIONS_VIA_SYMBOLS;
#ifdef ARCH64
	header.reserved = 0;
#endif

	/* fill in the segment command */
	memset(&reloc_segment, '\0', sizeof(segment_command_t));
	reloc_segment.cmd = LC_SEGMENT_VALUE;
	reloc_segment.cmdsize = sizeof(segment_command_t) +
				nsects * sizeof(section_t);
	/* leave reloc_segment.segname full of zeros */
	reloc_segment.vmaddr = 0;
	reloc_segment.vmsize = 0;
	reloc_segment.filesize = 0;
	offset = header.sizeofcmds + sizeof(mach_header_t);
	reloc_segment.fileoff = offset;
	reloc_segment.maxprot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
	reloc_segment.initprot= VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
	reloc_segment.nsects = nsects;
	reloc_segment.flags = 0;
	/*
	 * Set the offsets to the contents of the sections (for non-zerofill
	 * sections) and set the filesize and vmsize of the segment.  This is
	 * complicated by the fact that all the zerofill sections have addresses
	 * after the non-zerofill sections and that the alignment of sections
	 * produces gaps that are not in any section.  For the vmsize we rely on
	 * the fact the the sections start at address 0 so it is just the last
	 * zerofill section or the last not-zerofill section.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type == S_ZEROFILL ||
	       section_type == S_THREAD_LOCAL_ZEROFILL)
		continue;
	    for(p = frchainP->frch_next; p != NULL; p = p->frch_next){
		section_type = p->frch_section.flags & SECTION_TYPE;
		if(section_type != S_ZEROFILL &&
		   section_type != S_THREAD_LOCAL_ZEROFILL)
		    break;
	    }
	    if(p != NULL)
		i = (uint32_t)(p->frch_section.addr -
			       frchainP->frch_section.addr);
	    else
		i = (uint32_t)frchainP->frch_section.size;
	    reloc_segment.filesize += i;
	    frchainP->frch_section.offset = offset;
	    offset += i;
	    reloc_segment.vmsize = frchainP->frch_section.addr +
				   frchainP->frch_section.size;
	}
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type != S_ZEROFILL &&
	       section_type != S_THREAD_LOCAL_ZEROFILL)
		continue;
	    reloc_segment.vmsize = frchainP->frch_section.addr +
				   frchainP->frch_section.size;
	}
	offset = rnd32(offset, sizeof(int32_t));

	/*
	 * Count the number of relocation entries for each section.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    frchainP->frch_section.nreloc = 0;
	    for(fixP = frchainP->frch_fix_root; fixP; fixP = fixP->fx_next){
		frchainP->frch_section.nreloc += nrelocs_for_fix(fixP);
	    }
	}

	/*
	 * Fill in the offset to the relocation entries of the sections.
	 */
	offset = rnd32(offset, sizeof(int32_t));
	reloff = offset;
	nrelocs = 0;
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    if(frchainP->frch_section.nreloc == 0)
		frchainP->frch_section.reloff = 0;
	    else
		frchainP->frch_section.reloff = offset;
	    offset += frchainP->frch_section.nreloc *
		      sizeof(struct relocation_info);
	    nrelocs += frchainP->frch_section.nreloc;
	}

	if(flagseen['k']){
	    /* fill in the fields of the dysymtab_command */
	    dynamic_symbol_table.cmd = LC_DYSYMTAB;
	    dynamic_symbol_table.cmdsize = sizeof(struct dysymtab_command);

	    dynamic_symbol_table.ilocalsym = ilocalsym;
	    dynamic_symbol_table.nlocalsym = nlocalsym;
	    dynamic_symbol_table.iextdefsym = iextdefsym;
	    dynamic_symbol_table.nextdefsym = nextdefsym;
	    dynamic_symbol_table.iundefsym = iundefsym;
	    dynamic_symbol_table.nundefsym = nundefsym;

	    if(nindirectsyms == 0){
		dynamic_symbol_table.nindirectsyms = 0;
		dynamic_symbol_table.indirectsymoff = 0;
	    }
	    else{
		dynamic_symbol_table.nindirectsyms = nindirectsyms;
		dynamic_symbol_table.indirectsymoff = offset;
		offset += nindirectsyms * sizeof(uint32_t);
	    }

	    dynamic_symbol_table.tocoff = 0;
	    dynamic_symbol_table.ntoc = 0;
	    dynamic_symbol_table.modtaboff = 0;
	    dynamic_symbol_table.nmodtab = 0;
	    dynamic_symbol_table.extrefsymoff = 0;
	    dynamic_symbol_table.nextrefsyms = 0;
	    dynamic_symbol_table.extreloff = 0;
	    dynamic_symbol_table.nextrel = 0;
	    dynamic_symbol_table.locreloff = 0;
	    dynamic_symbol_table.nlocrel = 0;
	}

	/* fill in the fields of the symtab_command (except the string table) */
	symbol_table.cmd = LC_SYMTAB;
	symbol_table.cmdsize = sizeof(struct symtab_command);
	if(nsyms == 0)
	    symbol_table.symoff = 0;
	else
	    symbol_table.symoff = offset;
	symbol_table.nsyms = nsyms;
	offset += symbol_table.nsyms * sizeof(nlist_t);

	/* fill in the string table fields of the symtab_command */
	if(strsize == 0)
	    symbol_table.stroff = 0;
	else
	    symbol_table.stroff = offset;
	symbol_table.strsize = rnd32(strsize, sizeof(uint32_t));
	offset += rnd(strsize, sizeof(uint32_t));

	/*
	 * The second group of things to do is now with the size of everything
	 * known the object file and the offsets set in the various structures
	 * the contents of the object file can be created.
	 */

	/*
	 * Create the buffer to copy the parts of the output file into.
	 */
	output_size = offset;
	if((r = vm_allocate(mach_task_self(), (vm_address_t *)&output_addr,
			    output_size, TRUE)) != KERN_SUCCESS)
	    as_fatal("can't vm_allocate() buffer for output file of size %u",
		     output_size);

	/* put the headers in the output file's buffer */
	host_byte_sex = get_host_byte_sex();
	offset = 0;

	/* put the mach_header in the buffer */
	memcpy(output_addr + offset, &header, sizeof(mach_header_t));
	if(host_byte_sex != md_target_byte_sex)
	    swap_mach_header_t((mach_header_t *)(output_addr + offset),
			       md_target_byte_sex);
	offset += sizeof(mach_header_t);

	/* put the segment_command in the buffer */
	if(nsects != 0){
	    memcpy(output_addr + offset, &reloc_segment,
		   sizeof(segment_command_t));
	    if(host_byte_sex != md_target_byte_sex)
		swap_segment_command_t((segment_command_t *)
				       (output_addr + offset),
				       md_target_byte_sex);
	    offset += sizeof(segment_command_t);
	}

	/* put the segment_command's section structures in the buffer */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    memcpy(output_addr + offset, &(frchainP->frch_section),
		   sizeof(section_t));
	    if(host_byte_sex != md_target_byte_sex)
		swap_section_t((section_t *)(output_addr + offset), 1,
			       md_target_byte_sex);
	    offset += sizeof(section_t);
	}

	/* put the symbol_command in the buffer */
	if(nsyms != 0){
	    memcpy(output_addr + offset, &symbol_table,
		   sizeof(struct symtab_command));
	    if(host_byte_sex != md_target_byte_sex)
		swap_symtab_command((struct symtab_command *)
				     (output_addr + offset),
				     md_target_byte_sex);
	    offset += sizeof(struct symtab_command);
	}

	if(flagseen['k']){
	    /* put the dysymbol_command in the buffer */
	    if(nsyms != 0){
		memcpy(output_addr + offset, &dynamic_symbol_table,
		       sizeof(struct dysymtab_command));
		if(host_byte_sex != md_target_byte_sex)
		    swap_dysymtab_command((struct dysymtab_command *)
					  (output_addr + offset),
					  md_target_byte_sex);
		offset += sizeof(struct dysymtab_command);
	    }
	}

	/* put the section contents (frags) in the buffer */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    offset = frchainP->frch_section.offset;
	    for(fragP = frchainP->frch_root; fragP; fragP = fragP->fr_next){
		know(fragP->fr_type == rs_fill);
		/* put the fixed part of the frag in the buffer */
		memcpy(output_addr + offset, fragP->fr_literal, fragP->fr_fix);
		offset += fragP->fr_fix;

		/* put the variable repeated part of the frag in the buffer */
		fill_literal = fragP->fr_literal + fragP->fr_fix;
		fill_size = fragP->fr_var;
		num_bytes = fragP->fr_offset * fragP->fr_var;
		for(count = 0; count < num_bytes; count += fill_size){
		    memcpy(output_addr + offset, fill_literal, fill_size);
		    offset += fill_size;
		}
	    }
	}


	/* put the symbols in the output file's buffer */
	offset = symbol_table.symoff;
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    if((symbolP->sy_type & N_EXT) == 0){
		symbol_name = symbolP->sy_name;
		symbolP->sy_nlist.n_un.n_strx = symbolP->sy_name_offset;
		if(symbolP->expression != 0) {
		    expressionS *exp;

		    exp = (expressionS *)symbolP->expression;
		    if((exp->X_add_symbol->sy_type & N_TYPE) == N_UNDF)
	    		as_fatal("undefined symbol `%s' in operation setting "
				 "`%s'", exp->X_add_symbol->sy_name,
				 symbol_name);
		    if((exp->X_subtract_symbol->sy_type & N_TYPE) == N_UNDF)
	    		as_fatal("undefined symbol `%s' in operation setting "
				 "`%s'", exp->X_subtract_symbol->sy_name,
				 symbol_name);
		    if(exp->X_add_symbol->sy_other !=
		       exp->X_subtract_symbol->sy_other)
	    		as_fatal("invalid sections for operation on `%s' and "
				 "`%s' setting `%s'",exp->X_add_symbol->sy_name,
				 exp->X_subtract_symbol->sy_name, symbol_name);
		    symbolP->sy_nlist.n_value +=
			exp->X_add_symbol->sy_value -
			exp->X_subtract_symbol->sy_value;
		}
		memcpy(output_addr + offset, (char *)(&symbolP->sy_nlist),
		       sizeof(nlist_t));
		symbolP->sy_name = symbol_name;
		offset += sizeof(nlist_t);
	    }
	}
	for(i = 0; i < nextdefsym; i++){
	    symbol_name = extdefsyms[i]->sy_name;
	    extdefsyms[i]->sy_nlist.n_un.n_strx = extdefsyms[i]->sy_name_offset;
	    memcpy(output_addr + offset, (char *)(&extdefsyms[i]->sy_nlist),
	           sizeof(nlist_t));
	    extdefsyms[i]->sy_name = symbol_name;
	    offset += sizeof(nlist_t);
	}
	for(j = 0; j < nundefsym; j++){
	    symbol_name = undefsyms[j]->sy_name;
	    undefsyms[j]->sy_nlist.n_un.n_strx = undefsyms[j]->sy_name_offset;
	    memcpy(output_addr + offset, (char *)(&undefsyms[j]->sy_nlist),
	           sizeof(nlist_t));
	    undefsyms[j]->sy_name = symbol_name;
	    offset += sizeof(nlist_t);
	}
	if(host_byte_sex != md_target_byte_sex)
	    swap_nlist_t((nlist_t *)(output_addr + symbol_table.symoff),
		         symbol_table.nsyms, md_target_byte_sex);

	/*
	 * Put the relocation entries for each section in the buffer.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    offset = frchainP->frch_section.reloff;
	    for(fixP = frchainP->frch_fix_root; fixP; fixP = fixP->fx_next){
		offset += fix_to_relocation_entries(
					fixP,
					frchainP->frch_section.addr,
					(struct relocation_info *)(output_addr +
								   offset),
				        frchainP->frch_section.flags &
					  S_ATTR_DEBUG);
	    }
	}
	if(host_byte_sex != md_target_byte_sex)
	    swap_relocation_info((struct relocation_info *)
		(output_addr + reloff), nrelocs, md_target_byte_sex);

	if(flagseen['k']){
	    /* put the indirect symbol table in the buffer */
	    offset = dynamic_symbol_table.indirectsymoff;
	    for(frchainP = frchain_root;
		frchainP != NULL;
		frchainP = frchainP->frch_next){
		section_type = frchainP->frch_section.flags & SECTION_TYPE;
		if(section_type == S_NON_LAZY_SYMBOL_POINTERS ||
		   section_type == S_LAZY_SYMBOL_POINTERS ||
		   section_type == S_SYMBOL_STUBS){
		    /*
		     * For each indirect symbol put out the symbol number.
		     */
		    for(isymbolP = frchainP->frch_isym_root;
			isymbolP != NULL;
			isymbolP = isymbolP->isy_next){
			/*
			 * If this is a non-lazy pointer symbol section and
			 * if the symbol is a local symbol then put out
			 * INDIRECT_SYMBOL_LOCAL as the indirect symbol table
			 * entry.  This is used with code gen for fix-n-continue
			 * where the compiler generates indirection for static
			 * data references.  See the comments at the end of
			 * fixup_section() that explains the assembly code used.
			 */
			if(section_type == S_NON_LAZY_SYMBOL_POINTERS &&
			   (isymbolP->isy_symbol->sy_nlist.n_type & N_EXT) !=
			    N_EXT){
    			    local = INDIRECT_SYMBOL_LOCAL;
			    if((isymbolP->isy_symbol->sy_nlist.n_type &
				N_TYPE) == N_ABS)
				local |= INDIRECT_SYMBOL_ABS;
			    memcpy(output_addr + offset, (char *)(&local),
	   			   sizeof(uint32_t));
			}
			else{
			    memcpy(output_addr + offset,
				   (char *)(&isymbolP->isy_symbol->sy_number),
				   sizeof(uint32_t));
			}
			offset += sizeof(uint32_t);
		    }
		}
	    }
	    if(host_byte_sex != md_target_byte_sex){
		indirect_symbols = (uint32_t *)(output_addr +
				    dynamic_symbol_table.indirectsymoff);
		swap_indirect_symbols(indirect_symbols, nindirectsyms, 
				      md_target_byte_sex);
	    }
	}

	/* put the strings in the output file's buffer */
	offset = symbol_table.stroff;
	if(symbol_table.strsize != 0){
	    zero = 0;
	    memcpy(output_addr + offset, (char *)&zero, sizeof(char));
	    offset += sizeof(char);
	}
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    /* Ordinary case: not .stabd. */
	    if(symbolP->sy_name != NULL){
		if((symbolP->sy_type & N_EXT) != 0){
		    memcpy(output_addr + offset, symbolP->sy_name,
			   strlen(symbolP->sy_name) + 1);
		    offset += strlen(symbolP->sy_name) + 1;
		}
	    }
	}
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    /* Ordinary case: not .stabd. */
	    if(symbolP->sy_name != NULL){
		if((symbolP->sy_type & N_EXT) == 0){
		    memcpy(output_addr + offset, symbolP->sy_name,
			   strlen(symbolP->sy_name) + 1);
		    offset += strlen(symbolP->sy_name) + 1;
		}
	    }
	}
	/*
         * Create the output file.  The unlink() is done to handle the problem
         * when the out_file_name is not writable but the directory allows the
         * file to be removed (since the file may not be there the return code
         * of the unlink() is ignored).
         */
	if(bad_error != 0)
	    return;
	/*
	 * Avoid doing the unlink() on special files, just unlink regular files
	 * that exist.
	 */
	if(stat(out_file_name, &stat_buf) != -1){
	    if(stat_buf.st_mode & S_IFREG)
		(void)unlink(out_file_name);
	}
	if((fd = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
	    as_fatal("can't create output file: %s", out_file_name);
	if(write64(fd, output_addr, output_size) != (ssize_t)output_size)
	    as_fatal("can't write output file");
	if(close(fd) == -1)
	    as_fatal("can't close output file");
}

/*
 * layout_indirect_symbols() setups the indirect symbol tables by looking up or
 * creating symbol from the indirect symbol names and recording the symbol
 * pointers.  It returns the total count of indirect symbol table entries.
 */
static
uint32_t
layout_indirect_symbols(void)
{
    struct frchain *frchainP;
    uint32_t section_type, total, count, stride;
    isymbolS *isymbolP;
    symbolS *symbolP;

	/*
	 * Mark symbols that only appear in a lazy section with 
	 * REFERENCE_FLAG_UNDEFINED_LAZY.  To do this we first make sure a
	 * symbol exists for all non-lazy symbols.  Then we make a pass looking
	 * up the lazy symbols and if not there we make the symbol and mark it
	 * with REFERENCE_FLAG_UNDEFINED_LAZY.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type == S_NON_LAZY_SYMBOL_POINTERS){
		for(isymbolP = frchainP->frch_isym_root;
		    isymbolP != NULL;
		    isymbolP = isymbolP->isy_next){
/*
(void)symbol_find_or_make(isymbolP->isy_name);
*/
		    symbolP = symbol_find(isymbolP->isy_name);
		    if(symbolP == NULL){
			symbolP = symbol_new(isymbolP->isy_name, N_UNDF, 0, 0,
					     0, &zero_address_frag);
			symbol_table_insert(symbolP);
		    }
		}
	    }
	}
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type == S_LAZY_SYMBOL_POINTERS ||
	       section_type == S_SYMBOL_STUBS){
		for(isymbolP = frchainP->frch_isym_root;
		    isymbolP != NULL;
		    isymbolP = isymbolP->isy_next){

		    symbolP = symbol_find(isymbolP->isy_name);
		    if(symbolP == NULL){
			symbolP = symbol_find_or_make(isymbolP->isy_name);
			symbolP->sy_desc |= REFERENCE_FLAG_UNDEFINED_LAZY;
		    }
		}
	    }
	}

	total = 0;
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type == S_LAZY_SYMBOL_POINTERS ||
	       section_type == S_NON_LAZY_SYMBOL_POINTERS ||
	       section_type == S_SYMBOL_STUBS){
		count = 0;
		for(isymbolP = frchainP->frch_isym_root;
		    isymbolP != NULL;
		    isymbolP = isymbolP->isy_next){

/*
symbolP = symbol_find_or_make(isymbolP->isy_name);
*/
		    symbolP = symbol_find(isymbolP->isy_name);
		    if(symbolP == NULL){
			symbolP = symbol_new(isymbolP->isy_name, N_UNDF, 0, 0,
					     0, &zero_address_frag);
			symbol_table_insert(symbolP);
		    }
		    isymbolP->isy_symbol = symbolP;
		    count++;
		}
		/*
		 * Check for missing indirect symbols.
		 */
		if(section_type == S_SYMBOL_STUBS)
		    stride = frchainP->frch_section.reserved2;
		else
		    stride = sizeof(signed_target_addr_t);
		if(frchainP->frch_section.size / stride != count)
		    as_bad("missing indirect symbols for section (%s,%s)",
			    frchainP->frch_section.segname,
			    frchainP->frch_section.sectname);
		/*
		 * Set the index into the indirect symbol table for this
		 * section into the reserved1 field.
		 */
		frchainP->frch_section.reserved1 = total;
		total += count;
	    }
	}
	return(total);
}


/*
 * set_BINCL_checksums() walks through all STABS and calculate BINCL checksums. This will improve
 * linking performance because the linker will not need to touch and sum STABS
 * strings to do the BINCL/EINCL duplicate removal.
 *
 * A BINCL checksum is a sum of all stabs strings within a BINCL/EINCL pair.
 * Since BINCL/EINCL can be nested, a stab string contributes to only the 
 * innermost BINCL/EINCL enclosing it. 
 *
 * The checksum excludes the first number after an open paren.
 *
 * Some stabs (e.g. SLINE) when found within a BINCL/EINCL disqualify the EXCL
 * optimization and therefore disable this checksumming.
 */
static
void
set_BINCL_checksums()
{
    struct HeaderRange { 
	symbolS*		bincl; 
	struct HeaderRange*	parentRange;
	unsigned int		sum;
	int			okToChecksum; 
    };
    symbolS *symbolP;
    struct HeaderRange* curRange = NULL;
	
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    if((symbolP->sy_nlist.n_type & N_STAB) != 0){
		switch(symbolP->sy_nlist.n_type){
		case N_BINCL:
		    {
			struct HeaderRange* range =
			    xmalloc(sizeof(struct HeaderRange));
			range->bincl = symbolP;
			range->parentRange = curRange;
			range->sum = 0; 
			range->okToChecksum = (symbolP->sy_nlist.n_value == 0);
			curRange = range;
		    }
		    break;
		case N_EINCL:
		    if(curRange != NULL){
			struct HeaderRange* tmp = curRange;
			if (curRange->okToChecksum)
			    curRange->bincl->sy_nlist.n_value = curRange->sum;
			curRange = tmp->parentRange;
			free(tmp);
		    }
		    break;				
		case N_FUN:
		case N_BNSYM:
		case N_ENSYM:
		case N_LBRAC:
		case N_RBRAC:
		case N_SLINE:
		case N_STSYM:
		case N_LCSYM:
		    if(curRange != NULL){
			curRange->okToChecksum = FALSE;
		    }
		    break;
		case N_EXCL:
			break;
		default:
		    if(curRange != NULL){
			if(curRange->okToChecksum){
			    unsigned int sum = 0;
			    const char* s = symbolP->sy_name;
			    char c;
			    while((c = *s++) != '\0'){
				sum += c;
				/*
				 * Don't checkusm first number (file index)
				 * after open paren in string.
				 */
				if(c == '('){
				    while(isdigit(*s))
					++s;
				}
			    }
			    curRange->sum += sum;
			}
		    }
		}
	    }
	}
}

/*
 * layout_symbols() removes temporary symbols (symbols that are of the form L1
 * and 1:) if the -L flag is not seen so the symbol table has only the symbols
 * it will have in the output file.  Then each remaining symbol is given a
 * symbol number and a string offset for the symbol name which also sizes the
 * string table.
 * The order of the symbol table is:
 *	local symbols
 *	defined external symbols (sorted by name)
 *	undefined external symbols (sorted by name)
 * The order of the string table is:
 *	strings for external symbols
 *	strings for local symbols
 */
static
void
layout_symbols(
int32_t *symbol_number,
int32_t *string_byte_count)
{
    uint32_t i, j;
    symbolS *symbolP;
    symbolS **symbolPP;
    char *name;
	int seenBINCL = FALSE;

	*symbol_number = 0;
	*string_byte_count = sizeof(char);

	/*
	 * First pass through the symbols remove temporary symbols that are not
	 * going to be in the output file.  Also number the local symbols and
	 * assign string offset to external symbols.
	 */
	symbolPP = &symbol_rootP;
	while((symbolP = *symbolPP)){
	    name = symbolP->sy_name;
	    /*
	     * Deal with temporary symbols.  Temporary symbols start with 'L'
	     * but are not stabs.  It is an error if they are undefined.  They
	     * are removed if the -L flag is not seen else they are kept.
	     */
	    if(name != NULL &&
	       (symbolP->sy_nlist.n_type & N_STAB) == 0 &&
	       name[0] == 'L'){

	        if((symbolP->sy_nlist.n_type & N_TYPE) == N_UNDF){
		    if(name[1] != '\0' && name[2] == '\001'){
			as_bad("Undefined local symbol %c (%cf or %cb)",
				name[1], name[1], name[1]);
		    }
		    else{
			as_bad("Undefined local symbol %s", name);
		    }
		    /* don't keep this symbol */
		    *symbolPP = symbolP->sy_next;
		}
	        else if(flagseen['L'] || (symbolP->sy_type & N_EXT) != 0
#if defined(I386) && defined(ARCH64)
			|| is_section_cstring_literals(symbolP->sy_other)
#endif
		){
		    if((symbolP->sy_type & N_EXT) == 0){
			nlocalsym++;
			symbolP->sy_number = *symbol_number;
			*symbol_number = *symbol_number + 1;
		    }
		    else{
			nextdefsym++;
			symbolP->sy_name_offset = *string_byte_count;
			*string_byte_count += strlen(symbolP->sy_name) + 1;
		    }
		    symbolPP = &(symbolP->sy_next);
		}
		else{
		    /* don't keep this symbol */
		    *symbolPP = symbolP->sy_next;
		}
	    }
	    /*
	     * All non-temporary symbols will be the symbol table in the output
	     * file.
	     */
	    else{
		/* Any undefined symbols become N_EXT. */
		if(symbolP->sy_type == N_UNDF)
		    symbolP->sy_type |= N_EXT;

		if((symbolP->sy_type & N_EXT) == 0){
		    symbolP->sy_number = *symbol_number;
		    *symbol_number = *symbol_number + 1;
		    nlocalsym++;
		}
		else{
		    if((symbolP->sy_type & N_TYPE) != N_UNDF)
			nextdefsym++;
		    else
			nundefsym++;

		    if(name != NULL){
			/* the ordinary case (symbol has a name) */
			symbolP->sy_name_offset = *string_byte_count;
			*string_byte_count += strlen(symbolP->sy_name) + 1;
		    }
		    else{
			/* the .stabd case (symbol has no name) */
			symbolP->sy_name_offset = 0;
		    }
		}
		symbolPP = &(symbolP->sy_next);
	    }
	}

	/*
	 * Check to see that any symbol that is marked as a weak_definition
	 * is a global symbol defined in a coalesced section.
	 */
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    if((symbolP->sy_nlist.n_type & N_STAB) == 0 &&
	       (symbolP->sy_desc & N_WEAK_DEF) == N_WEAK_DEF){
		if((symbolP->sy_type & N_EXT) == 0){
		    as_bad("Non-global symbol: %s can't be a weak_definition",
			   symbolP->sy_name);
		}
		else if((symbolP->sy_type & N_TYPE) == N_UNDF){
		    as_bad("Undefined symbol: %s can't be a weak_definition",
			   symbolP->sy_name);
		}
	    }
	}

	/* Set the indexes for symbol groups into the symbol table */
	ilocalsym = 0;
	iextdefsym = nlocalsym;
	iundefsym = nlocalsym + nextdefsym;

	/* allocate arrays for sorting externals by name */
	extdefsyms = xmalloc(nextdefsym * sizeof(symbolS *));
	undefsyms = xmalloc(nundefsym * sizeof(symbolS *));

	i = 0;
	j = 0;
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    if((symbolP->sy_type & N_EXT) == 0){
		if(symbolP->sy_name != NULL){
		    /* the ordinary case (symbol has a name) */
		    symbolP->sy_name_offset = *string_byte_count;
		    *string_byte_count += strlen(symbolP->sy_name) + 1;
		    /* check for existance of BINCL/EINCL */
		    if(symbolP->sy_nlist.n_type == N_BINCL)
			seenBINCL = TRUE;
		}
		else{
		    /* the .stabd case (symbol has no name) */
		    symbolP->sy_name_offset = 0;
		}
	    }
	    else{
		if((symbolP->sy_type & N_TYPE) != N_UNDF)
		    extdefsyms[i++] = symbolP;
		else
		    undefsyms[j++] = symbolP;
	    }
	}
	qsort(extdefsyms, nextdefsym, sizeof(symbolS *),
	      (int (*)(const void *, const void *))qsort_compare);
	qsort(undefsyms, nundefsym, sizeof(symbolS *),
	      (int (*)(const void *, const void *))qsort_compare);
	for(i = 0; i < nextdefsym; i++){
	    extdefsyms[i]->sy_number = *symbol_number;
	    *symbol_number = *symbol_number + 1;
	}
	for(j = 0; j < nundefsym; j++){
	    undefsyms[j]->sy_number = *symbol_number;
	    *symbol_number = *symbol_number + 1;
	}
	
	/* calculate BINCL checksums */
	if(seenBINCL)
	    set_BINCL_checksums();	
}

/*
 * Function for qsort to sort symbol structs by their name
 */
static
int
qsort_compare(
const symbolS **sym1,
const symbolS **sym2)
{
        return(strcmp((*sym1)->sy_name, (*sym2)->sy_name));
}

/*
 * nrelocs_for_fix() returns the number of relocation entries needed for the
 * specified fix structure.
 */
static
uint32_t
nrelocs_for_fix(
struct fix *fixP)
{
	/*
	 * If fx_addsy is NULL then this fix needs no relocation entry.
	 */
	if(fixP->fx_addsy == NULL)
	    return(0);

	/*
	 * If this fix has a subtract symbol it is a SECTDIFF relocation which
	 * takes two relocation entries.
	 */
	if(fixP->fx_subsy != NULL)
	    return(2);

	/*
	 * For RISC machines whenever we have a relocation item using the half
	 * of an address a second a relocation item describing the other
	 * half of the address is used.
	 */
#ifdef I860
	if(fixP->fx_r_type == I860_RELOC_HIGH ||
	   fixP->fx_r_type == I860_RELOC_HIGHADJ)
	    return(2);
#endif
#ifdef M88K
	if(fixP->fx_r_type == M88K_RELOC_HI16 ||
	   fixP->fx_r_type == M88K_RELOC_LO16)
	    return(2);
#endif
#ifdef PPC
	if(fixP->fx_r_type == PPC_RELOC_HI16 ||
	   fixP->fx_r_type == PPC_RELOC_LO16 ||
	   fixP->fx_r_type == PPC_RELOC_HA16 ||
	   fixP->fx_r_type == PPC_RELOC_LO14 ||
	   fixP->fx_r_type == PPC_RELOC_JBSR)
	    return(2);
#endif
#ifdef HPPA
	if(fixP->fx_r_type == HPPA_RELOC_HI21 ||
	   fixP->fx_r_type == HPPA_RELOC_LO14 ||
	   fixP->fx_r_type == HPPA_RELOC_BR17 ||
	   fixP->fx_r_type == HPPA_RELOC_JBSR)
	    return(2);
#endif
#ifdef SPARC
	if(fixP->fx_r_type == SPARC_RELOC_HI22 ||
	   fixP->fx_r_type == SPARC_RELOC_LO10)
	    return(2);
#endif
#ifdef ARM
	if(fixP->fx_r_type == ARM_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_RELOC_HI16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_HI16)
	    return(2);
#endif
	return(1);
}

/*
 * fix_to_relocation_entries() creates the needed relocation entries for the
 * specified fix structure that is from a section who's address starts at
 * sect_addr.  It returns the number of bytes of relocation_info structs it
 * placed at riP.
 */
static
uint32_t
fix_to_relocation_entries(
struct fix *fixP,
uint64_t sect_addr,
struct relocation_info *riP,
uint32_t debug_section)
{
    struct symbol *symbolP;
    uint32_t count;
    struct scattered_relocation_info sri;
    uint32_t sectdiff;
#ifdef HPPA
    uint32_t left21, right14;
#endif

/* the pragmas that follow silence a noisy clang warning  */
#pragma unused (sri)
#pragma unused (sectdiff)
	/*
	 * If fx_addsy is NULL then this fix needs no relocation entry.
	 */
	if(fixP->fx_addsy == NULL)
	    return(0);

#ifdef TC_VALIDATE_FIX
	TC_VALIDATE_FIX(fixP, sect_addr, 0);
#endif

	memset(riP, '\0', sizeof(struct relocation_info));
	symbolP = fixP->fx_addsy;

#ifdef ARM
	/* see arm_reloc.h for the encodings in the low 2 bits */
	if(fixP->fx_r_type == ARM_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_RELOC_HI16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_HI16){
	    riP->r_length = fixP->fx_r_type & 0x3;
	}
	else
#endif
	switch(fixP->fx_size){
	    case 1:
		riP->r_length = 0;
		break;
	    case 2:
		riP->r_length = 1;
		break;
	    case 4:
#ifdef PPC
		if(fixP->fx_r_type == PPC_RELOC_BR14_predicted)
		    riP->r_length = 3;
		else
#endif
		riP->r_length = 2;
		break;
#if defined(ARCH64)
	    case 8:
		riP->r_length = 3;
		break;
#endif /* defined(ARCH64) */
	    default:
		layout_file = fixP->file;
		layout_line = fixP->line;
		as_fatal("Bad fx_size (0x%x) in fix_to_relocation_info()\n",
			 fixP->fx_size);
	}
	riP->r_pcrel = fixP->fx_pcrel;
	riP->r_address = (int32_t)(fixP->fx_frag->fr_address + fixP->fx_where -
			 sect_addr);
#ifdef ARM
	if(fixP->fx_r_type == ARM_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_RELOC_HI16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_HI16){
	    riP->r_type = ARM_RELOC_HALF;
	}
	else
#endif
	riP->r_type = fixP->fx_r_type;
	/*
	 * For undefined symbols this will be an external relocation entry.
	 * Or if this is an external coalesced symbol or weak symbol.
	 */
#if defined(I386) && defined(ARCH64)
	if(fixP->fx_subsy == NULL &&
	   (!debug_section || (symbolP->sy_type & N_TYPE) == N_UNDF) &&
	   (!is_local_symbol(symbolP) ||
	    ((symbolP->sy_type & N_TYPE) == N_SECT &&
	     is_section_cstring_literals(symbolP->sy_other)) ) ) {
#else
	if((symbolP->sy_type & N_TYPE) == N_UNDF ||
	   ((symbolP->sy_type & N_EXT) == N_EXT &&
	    (symbolP->sy_type & N_TYPE) == N_SECT &&
	    (is_section_coalesced(symbolP->sy_other) ||
	     (symbolP->sy_desc & N_WEAK_DEF) == N_WEAK_DEF) &&
	    fixP->fx_subsy == NULL)
#if defined(I386) && !defined(ARCH64)
	   ||
	   ((symbolP->sy_type & N_TYPE) == N_SECT &&
	    fixP->fx_r_type == GENERIC_RELOC_TLV)
#endif
        ){
#endif
	    riP->r_extern = 1;
	    riP->r_symbolnum = symbolP->sy_number;
	}
	else{
	    /*
	     * For defined symbols this will be a local relocation entry
	     * (possibly a section difference or a scattered relocation entry).
	     */
	    riP->r_extern = 0;
	    riP->r_symbolnum = symbolP->sy_other; /* nsect */

	    /*
	     * Determine if this is left as a local relocation entry or
	     * changed to a SECTDIFF relocation entry.  If this comes from a fix
	     * that has a subtract symbol it is a SECTDIFF relocation.  Which is
	     * "addsy - subsy + constant" where both symbols are defined in
	     * sections.  To encode all this information two scattered
	     * relocation entries are used.  The first has the add symbol value
	     * and the second has the subtract symbol value.
	     */
	    if(fixP->fx_subsy != NULL){
#if defined(I386) && defined(ARCH64)
		/* Encode fixP->fx_subsy (B) first, then symbolP (fixP->fx_addsy) (A). */
		if (is_local_symbol(fixP->fx_subsy))
		{
			riP->r_extern = 0;
			riP->r_symbolnum = fixP->fx_subsy->sy_other;
		}
		else
		{
			riP->r_extern = 1;
			riP->r_symbolnum = fixP->fx_subsy->sy_number;
		}
		riP->r_type = X86_64_RELOC_SUBTRACTOR;
		
		/* Now write out the unsigned relocation entry. */
		riP++;
		*riP = *(riP - 1);
		if (is_local_symbol(fixP->fx_addsy))
		{
			riP->r_extern = 0;
			riP->r_symbolnum = fixP->fx_addsy->sy_other;
		}
		else
		{
			riP->r_extern = 1;
			riP->r_symbolnum = fixP->fx_addsy->sy_number;
		}
		riP->r_type = X86_64_RELOC_UNSIGNED;
		return(2 * sizeof(struct relocation_info));
#endif
/* the #if that follows is to silence a noisy "unreachable code" warning */
#if defined(ARM) || defined(SPARC) || defined(HPPA) || defined (PPC)
#ifdef PPC
		if(fixP->fx_r_type == PPC_RELOC_HI16)
		    sectdiff = PPC_RELOC_HI16_SECTDIFF;
		else if(fixP->fx_r_type == PPC_RELOC_LO16)
		    sectdiff = PPC_RELOC_LO16_SECTDIFF;
		else if(fixP->fx_r_type == PPC_RELOC_HA16)
		    sectdiff = PPC_RELOC_HA16_SECTDIFF;
		else if(fixP->fx_r_type == PPC_RELOC_LO14)
		    sectdiff = PPC_RELOC_LO14_SECTDIFF;
		else
#endif
#ifdef HPPA
		if(fixP->fx_r_type == HPPA_RELOC_HI21)
		    sectdiff = HPPA_RELOC_HI21_SECTDIFF;
		else if(fixP->fx_r_type == HPPA_RELOC_LO14)
		    sectdiff = HPPA_RELOC_LO14_SECTDIFF;
		else
#endif
#ifdef SPARC
		if(fixP->fx_r_type == SPARC_RELOC_HI22)
		    sectdiff = SPARC_RELOC_HI22_SECTDIFF;
		else if(fixP->fx_r_type == SPARC_RELOC_LO10)
		    sectdiff = SPARC_RELOC_LO10_SECTDIFF;
		else
#endif
#ifdef ARM
		if(fixP->fx_r_type == ARM_RELOC_LO16 ||
		   fixP->fx_r_type == ARM_RELOC_HI16 ||
		   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
		   fixP->fx_r_type == ARM_THUMB_RELOC_HI16)
		    sectdiff = ARM_RELOC_HALF_SECTDIFF;
		else
#endif
		{
		    if(fixP->fx_r_type != 0 && fixP->fx_r_type != NO_RELOC){
			layout_file = fixP->file;
			layout_line = fixP->line;
			as_fatal("Internal error: incorrect fx_r_type (%u) for "
				 "fx_subsy != 0 in fix_to_relocation_info()",
				 fixP->fx_r_type);
		    }
		    if((!(fixP->fx_addsy->sy_type & N_EXT)) && flagseen['k'])
			sectdiff = RELOC_LOCAL_SECTDIFF;
		    else
			sectdiff = RELOC_SECTDIFF;
		}
		memset(&sri, '\0',sizeof(struct scattered_relocation_info));
		sri.r_scattered = 1;
#ifdef ARM
		/* see arm_reloc.h for the encodings in the low 2 bits */
		if(fixP->fx_r_type == ARM_RELOC_LO16 ||
		   fixP->fx_r_type == ARM_RELOC_HI16 ||
		   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
		   fixP->fx_r_type == ARM_THUMB_RELOC_HI16)
		    sri.r_length = fixP->fx_r_type & 0x3;
		else
#endif
		sri.r_length    = riP->r_length;
		sri.r_pcrel     = riP->r_pcrel;
		sri.r_address   = riP->r_address;
                if(sri.r_address != riP->r_address)
		    as_fatal("Section too large, can't encode r_address (0x%x) "
			     "into 24-bits of scattered relocation entry",
			     riP->r_address);
		sri.r_type      = sectdiff;
		sri.r_value     = symbolP->sy_value;
		*riP = *((struct relocation_info *)&sri);
		riP++;

		sri.r_type      = RELOC_PAIR;
		sri.r_value     = fixP->fx_subsy->sy_value;
		if(sectdiff == RELOC_SECTDIFF ||
		   sectdiff == RELOC_LOCAL_SECTDIFF)
		    sri.r_address = 0;
#ifdef PPC
		else if(sectdiff == PPC_RELOC_HI16_SECTDIFF ||
		        sectdiff == PPC_RELOC_HA16_SECTDIFF){
		    sri.r_address = (symbolP->sy_value -
				     fixP->fx_subsy->sy_value
				     + fixP->fx_offset) & 0xffff;
		}
		else if(sectdiff == PPC_RELOC_LO16_SECTDIFF ||
			sectdiff == PPC_RELOC_LO14_SECTDIFF){
		    sri.r_address = ((symbolP->sy_value -
				      fixP->fx_subsy->sy_value +
				      fixP->fx_offset) >> 16) & 0xffff;
		}
#endif
#ifdef HPPA
		else if(sectdiff == HPPA_RELOC_HI21_SECTDIFF){
		    calc_hppa_HILO(symbolP->sy_value - fixP->fx_subsy->sy_value,
				   fixP->fx_offset, &left21, &right14);
		    sri.r_address = right14 & 0x3fff;
		}
		else if(sectdiff == HPPA_RELOC_LO14_SECTDIFF){
		    calc_hppa_HILO(symbolP->sy_value - fixP->fx_subsy->sy_value,
				   fixP->fx_offset, &left21, &right14);
		    sri.r_address = left21 >> 11;
		}
#endif
#ifdef SPARC
		else if(sectdiff == SPARC_RELOC_HI22_SECTDIFF){
		    sri.r_address = (symbolP->sy_value -
				     fixP->fx_subsy->sy_value
				     + fixP->fx_offset) & 0x3ff;
		}
		else if(sectdiff == SPARC_RELOC_LO10_SECTDIFF){
		    sri.r_address = ((symbolP->sy_value -
				      fixP->fx_subsy->sy_value +
				      fixP->fx_offset) >> 10) & 0x3fffff;
		}
#endif
#ifdef ARM
		else if(sectdiff == ARM_RELOC_HALF_SECTDIFF){
		    if((sri.r_length & 0x1) == 0x1)
			sri.r_address = (symbolP->sy_value -
					 fixP->fx_subsy->sy_value
					 + fixP->fx_offset) & 0xffff;
		    else
			sri.r_address = ((symbolP->sy_value -
					  fixP->fx_subsy->sy_value +
					  fixP->fx_offset) >> 16) & 0xffff;
		}
#endif
		*riP = *((struct relocation_info *)&sri);
		return(2 * sizeof(struct relocation_info));
#endif /* unreachable code */
	    }
	    /*
	     * Determine if this is left as a local relocation entry or must be
	     * changed to a scattered relocation entry.  These entries allow
	     * the link editor to scatter the contents of a section and a local
	     * relocation can't be used when an offset is added to the symbol's
	     * value (symbol + offset).  This is because the relocation must be
	     * based on the value of the symbol not the value of the expression.
	     * Thus a scattered relocation entry that encodes the address of the
	     * symbol is used when the offset is non-zero.  Unfortunately this
	     * encoding only allows for 24 bits in the r_address field and can
	     * overflow.  So it if it would overflow we don't create a
	     * scattered relocation entry and hope the offset does not reach
	     * out of the block or the linker will not be doing scattered
	     * loading on this symbol in this object file.
	     */
#if !defined(I860) && !(defined(I386) && defined(ARCH64))
	    /*
	     * For processors that don't have all references as unique 32 bits
	     * wide references scattered relocation entries are not generated.
	     * This is so that the link editor does not get stuck not being able
	     * to do the relocation if the high half of the reference is shared
	     * by two references to two different symbols.
	     */
	    if(fixP->fx_offset != 0 &&
	       (riP->r_address & 0xff000000) == 0 &&
	       ((symbolP->sy_type & N_TYPE) & ~N_EXT) != N_ABS
#ifdef M68K
	       /*
		* Since the m68k's pc relative branch instructions use the
		* address of the beginning of the displacement (except for
		* byte) the code in m68k.c when generating fixes adds to the
		* offset 2 for word and 4 for long displacements.
		*/
	       && !(fixP->fx_pcrel &&
	            ((fixP->fx_size == 2 && fixP->fx_offset == 2) ||
	             (fixP->fx_size == 4 && fixP->fx_offset == 4)) )
#endif /* M68K */
	       ){
		memset(&sri, '\0',sizeof(struct scattered_relocation_info));
		sri.r_scattered = 1;
		sri.r_length    = riP->r_length;
		sri.r_pcrel     = riP->r_pcrel;
		sri.r_address   = riP->r_address;
                if(sri.r_address != riP->r_address)
		    as_fatal("Section too large, can't encode r_address (0x%x) "
			     "into 24-bits of scattered relocation entry",
			     riP->r_address);
		sri.r_type      = riP->r_type;
		sri.r_value     = symbolP->sy_value;
		*riP = *((struct relocation_info *)&sri);
	    }
#endif /* !defined(I860) && !(defined(I386) && defined(ARCH64)) */
	}
	count = 1;
	riP++;

#if !defined(M68K) && !defined(I386)
	/*
	 * For RISC machines whenever we have a relocation item using the half
	 * of an address we also emit a relocation item describing the other
	 * half of the address so the linker can reconstruct the address to do
	 * the relocation.
	 */
#ifdef I860
	if(fixP->fx_r_type == I860_RELOC_HIGH ||
	   fixP->fx_r_type == I860_RELOC_HIGHADJ)
#endif
#ifdef M88K
	if(fixP->fx_r_type == M88K_RELOC_HI16 ||
	   fixP->fx_r_type == M88K_RELOC_LO16)
#endif
#ifdef PPC
	if(fixP->fx_r_type == PPC_RELOC_HI16 ||
	   fixP->fx_r_type == PPC_RELOC_LO16 ||
	   fixP->fx_r_type == PPC_RELOC_HA16 ||
	   fixP->fx_r_type == PPC_RELOC_LO14 ||
	   fixP->fx_r_type == PPC_RELOC_JBSR)
#endif
#ifdef HPPA
	if(fixP->fx_r_type == HPPA_RELOC_HI21 ||
	   fixP->fx_r_type == HPPA_RELOC_LO14 ||
	   fixP->fx_r_type == HPPA_RELOC_BR17 ||
	   fixP->fx_r_type == HPPA_RELOC_JBSR)
#endif
#ifdef SPARC
	if(fixP->fx_r_type == SPARC_RELOC_HI22 ||
	   fixP->fx_r_type == SPARC_RELOC_LO10)
#endif
#ifdef ARM
	if(fixP->fx_r_type == ARM_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_RELOC_HI16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
	   fixP->fx_r_type == ARM_THUMB_RELOC_HI16)
#endif
	{
	    memset(riP, '\0', sizeof(struct relocation_info));
#ifdef ARM
	    /* see arm_reloc.h for the encodings in the low 2 bits */
	    if(fixP->fx_r_type == ARM_RELOC_LO16 ||
	       fixP->fx_r_type == ARM_RELOC_HI16 ||
	       fixP->fx_r_type == ARM_THUMB_RELOC_LO16 ||
	       fixP->fx_r_type == ARM_THUMB_RELOC_HI16){
		riP->r_length = fixP->fx_r_type & 0x3;
	    }
	    else
#endif
	    switch(fixP->fx_size){
		case 1:
		    riP->r_length = 0;
		    break;
		case 2:
		    riP->r_length = 1;
		    break;
		case 4:
		    riP->r_length = 2;
		    break;
#if defined(ARCH64)
                case 8:
                    riP->r_length = 3;
                    break;
#endif /* defined(ARCH64) */
		default:
		    as_fatal("Bad fx_size (0x%x) in fix_to_relocation_info()\n",
			     fixP->fx_size);
	    }
	    riP->r_pcrel = fixP->fx_pcrel;
	    /*
	     * We set r_extern to 0, so other apps won't try to use r_symbolnum
	     * as a symbol table indice.  We set all the bits of r_symbolnum so 
	     * it is all but guaranteed to be outside the range we use for non-
	     * external types to denote what section the relocation is in.
	     */
	    riP->r_extern = 0;
	    riP->r_symbolnum = 0x00ffffff;
#ifdef I860
	    riP->r_type	= I860_RELOC_PAIR;
	    riP->r_address = 0xffff & fixP->fx_value;
#endif
#ifdef M88K
	    riP->r_type = M88K_RELOC_PAIR;
	    if(fixP->fx_r_type == M88K_RELOC_HI16)
		riP->r_address = 0xffff & fixP->fx_value;
	    else if(fixP->fx_r_type == M88K_RELOC_LO16)
		riP->r_address = 0xffff & (fixP->fx_value >> 16);
#endif
#ifdef PPC
	    riP->r_type = PPC_RELOC_PAIR;
	    if(fixP->fx_r_type == PPC_RELOC_HI16 ||
	       fixP->fx_r_type == PPC_RELOC_HA16)
		riP->r_address = 0xffff & fixP->fx_value;
	    else if(fixP->fx_r_type == PPC_RELOC_LO16 ||
		    fixP->fx_r_type == PPC_RELOC_LO14)
		riP->r_address = 0xffff & (fixP->fx_value >> 16);
	    else if (fixP->fx_r_type == PPC_RELOC_JBSR){
		/*
		 * To allow the "true target address" to use the full 32 bits
		 * we convert this PAIR relocation entry to a scattered
		 * relocation entry if the true target address has the
		 * high bit (R_SCATTERED) set and store the "true target
		 * address" in the r_value field.  Or for an external relocation
		 * entry if the "offset" to the symbol has the high bit set
		 * we also use a scattered relocation entry.
		 */
		if((fixP->fx_value & R_SCATTERED) == 0){
		    riP->r_address = fixP->fx_value;
		}
		else{
		    memset(&sri, '\0',sizeof(struct scattered_relocation_info));
		    sri.r_scattered = 1;
		    sri.r_pcrel     = riP->r_pcrel;
		    sri.r_length    = riP->r_length;
		    sri.r_type      = riP->r_type;
		    sri.r_address   = 0;
		    sri.r_value     = fixP->fx_value;
		    *riP = *((struct relocation_info *)&sri);
		}
	    }
#endif
#ifdef HPPA
	    riP->r_type	 = HPPA_RELOC_PAIR;
	    calc_hppa_HILO(fixP->fx_value - fixP->fx_offset,
			   fixP->fx_offset, &left21, &right14);
	    if (fixP->fx_r_type == HPPA_RELOC_LO14 ||
		fixP->fx_r_type == HPPA_RELOC_BR17)
		riP->r_address = left21 >> 11;
	    else if (fixP->fx_r_type == HPPA_RELOC_HI21)
		riP->r_address = right14 & 0x3fff;
	    else if (fixP->fx_r_type == HPPA_RELOC_JBSR){
		if((symbolP->sy_type & N_TYPE) == N_UNDF)
		    riP->r_address = fixP->fx_value & 0xffffff;
		else
		    riP->r_address = (fixP->fx_value - sect_addr) & 0xffffff;
	    }
#endif
#ifdef SPARC
	    riP->r_type	 = SPARC_RELOC_PAIR;
	    if (fixP->fx_r_type == SPARC_RELOC_HI22)
		riP->r_address = fixP->fx_value & 0x3ff;
	    else if (fixP->fx_r_type == SPARC_RELOC_LO10)
		riP->r_address = (fixP->fx_value >> 10) & 0x3fffff;
#endif
#ifdef ARM
	    riP->r_type = ARM_RELOC_PAIR;
	    if(fixP->fx_r_type == ARM_RELOC_HI16 ||
	       fixP->fx_r_type == ARM_THUMB_RELOC_HI16)
		riP->r_address = 0xffff & fixP->fx_value;
	    else if(fixP->fx_r_type == ARM_RELOC_LO16 ||
		    fixP->fx_r_type == ARM_THUMB_RELOC_LO16)
		riP->r_address = 0xffff & (fixP->fx_value >> 16);
#endif
	    count = 2;
	}
#endif /* !defined(M68K) && !defined(I386) */
	return(count * sizeof(struct relocation_info));
}

#ifdef I860
/*
 * set_default_section_align() is used to set a default minimum section
 * alignment if the section exist.
 */
static
void
set_default_section_align(
char *segname,
char *sectname,
uint32_t align)
{
    frchainS *frcP;

	for(frcP = frchain_root; frcP != NULL; frcP = frcP->frch_next){
	    if(strncmp(frcP->frch_section.segname, segname,
		       sizeof(frcP->frch_section.segname)) == 0 &&
	       strncmp(frcP->frch_section.sectname, sectname,
		       sizeof(frcP->frch_section.sectname)) == 0){
		if(align > frcP->frch_section.align)
		    frcP->frch_section.align = align;
		return;
	    }
	}
}

/* 
 * clear_section_flags() clears the section types for literals from the section
 * flags field.  This is needed for processors that don't have all references
 * to sections as unique 32 bits wide references.  In this case the literal
 * flags are not set.  This is so that the link editor does not merge them and
 * get stuck not being able to fit the relocated address in the item to be
 * relocated or if the high half of the reference is shared by two references
 * to different symbols (which can also stick the link editor).
 */
static
void
clear_section_flags(void)
{
    frchainS *frcP;

	for(frcP = frchain_root; frcP != NULL; frcP = frcP->frch_next)
	    if(frcP->frch_section.flags != S_ZEROFILL &&
	       frcP->frch_section.flags != S_THREAD_LOCAL_ZEROFILL)
		frcP->frch_section.flags = 0;
}

/*
 * I860_tweeks() preforms the tweeks needed by the I860 processor to get minimum
 * section alignments and no merging of literals by the link editor.
 */
static
void
I860_tweeks(void)
{
	set_default_section_align("__TEXT", "__text", 5);
	set_default_section_align("__DATA", "__data", 4);
	set_default_section_align("__DATA", "__bss",  4);

	clear_section_flags();
}
#endif

/* FROM write.c line 2764 */
void
number_to_chars_bigendian (char *buf, signed_expr_t val, int n)
{
  if (n <= 0)
    abort ();
  while (n--)
    {
      buf[n] = val & 0xff;
      val >>= 8;
    }
}

void
number_to_chars_littleendian (char *buf, signed_expr_t val, int n)
{
  if (n <= 0)
    abort ();
  while (n--)
    {
      *buf++ = val & 0xff;
      val >>= 8;
    }
}
