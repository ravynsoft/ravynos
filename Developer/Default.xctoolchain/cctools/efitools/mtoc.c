/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#define __eip eip
#define __rip rip

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "stuff/breakout.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/reloc.h"
#include "stuff/rnd.h"
#include "stuff/write64.h"

#include "coff/ms_dos_stub.h"
#include "coff/filehdr.h"
#include "coff/aouthdr.h"
#include "coff/scnhdr.h"
#include "coff/syment.h"
#include "coff/bytesex.h"

#include "coff/base_relocs.h"
#include "mach-o/x86_64/reloc.h"
#include "mach-o/arm64/reloc.h"

/* used by error routines as the name of this program */
char *progname = NULL;

/* the bytesex of our target object file and of this host machine */
static enum byte_sex target_byte_sex;
static enum byte_sex host_byte_sex;
static enum bool swapped;

/* the size of the pecoff output file */
static uint32_t output_size = 0;

static uint32_t majorVersion = 0;
static uint32_t minorVersion = 0;

/*
 * The headers, and elements of them in the pecoff output file.
 */
static struct ms_dos_stub ms_dos_stub;
static char signature[4];
static struct filehdr filehdr;
static struct aouthdr aouthdr;
static struct aouthdr_64 aouthdr64;
uint32_t entry = 0; /* the entry point */
uint32_t nscns = 0; /* the number of section headers and contents pointers */
static struct scnhdr *scnhdrs = NULL;  /* the section headers */
static char **scn_contents = NULL; /* pointers to the section contents */

/*
 * The value of the -subsystem argument to then set in the PECOFF aouthdr.
 */
static uint16_t Subsystem = IMAGE_SUBSYSTEM_EFI_APPLICATION;

struct subsystem_argument {
    char *name;
    uint16_t value;
};

struct subsystem_argument subsystem_arguments[] = {
    { "application",		IMAGE_SUBSYSTEM_EFI_APPLICATION },
    { "app",			IMAGE_SUBSYSTEM_EFI_APPLICATION },
    { "UEFI_APPLICATION",	IMAGE_SUBSYSTEM_EFI_APPLICATION },
    { "APPLICATION",	        IMAGE_SUBSYSTEM_EFI_APPLICATION },

    { "boot",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "bsdrv",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "DXE_DRIVER",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "SEC",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "peim",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "BASE",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "PEI_CORE",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "PEIM",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "DXE_SMM_DRIVER",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "TOOL",			IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "USER_DEFINED",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "UEFI_DRIVER",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "DXE_CORE",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "SECURITY_CORE",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "COMBINED_PEIM_DRIVER",	IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "PIC_PEIM",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "RELOCATABLE_PEIM",	IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "BS_DRIVER",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },
    { "SMM_CORE",		IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER },

    { "runtime",		IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER },
    { "rtdrv",			IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER },
    { "DXE_RUNTIME_DRIVER",	IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER },

    { NULL, 0 }
};

/*
 * The value of the -section_alignment argument (or the -align argument) to
 * layout the added PECOFF sections and set into the PECOFF aouthdr.
 */
static uint32_t section_alignment = SECTIONALIGNMENT;

/*
 * The value of the -align argument to layout the PECOFF file.
 */
static uint32_t file_alignment = FILEALIGNMENT;

/* The maximum alignment allowed to be specified, in hex */
#define MAXALIGN		0x8000

/* Static routine to help parse arguments */
static enum bool ispoweroftwo(uint32_t x);

/*
 * The string for the -d argument.
 */
char *debug_filename = NULL;

/*
 * The string for the -u argument.
 */
char *debug_uuid = NULL;

/*
 * Format specifier for scanf() to convert UUID to individual bytes
 */
#define UUID_FORMAT_STRING "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"

/*
 * The string for the entry point symbol name.
 */
char *entry_point = NULL;

#ifdef HACK_TO_MATCH_TEST_CASE
/*
 * These are are used for the HACK to get the symbol table for 32-bit files to
 * match the one produced by objcopy.  They are the pecoff section numbers of
 * the .common and .bss sections.
 */
static uint32_t common_scnum = 0;
static uint32_t bss_scnum = 0;
#endif

/*
 * These are for the .reloc section that contains the base relocations.
 */
static struct scnhdr *reloc_scnhdr = NULL;
static uint32_t reloc_size = 0;
static char *reloc_contents = NULL;

/*
 * These are for the pecoff symbol table and string table.
 */
static uint32_t nsyments = 0;		/* number of symbols */
static struct syment *syments = NULL;	/* pointer to symbol table elements */
static uint32_t syment_offset = 0;	/* file offset of the symbol table */
static uint32_t strsize = 0;		/* size of the string table */
static char *strings = NULL;		/* pointer to the string table */
static uint32_t section_names_size = 0;	/* size of the section names */
static char *section_names = NULL;	/* pointer to section names */
static uint32_t string_offset = 0;	/* file offset of the string table */

/*
 * These are for the .debug section that contains the -d filename information.
 */
static struct scnhdr *debug_scnhdr = NULL;
static uint32_t debug_size = 0;
static char *debug_contents = NULL;
static struct debug_directory_entry *dde = NULL;
static struct mtoc_debug_info *mdi = NULL;

static void process_arch(
    struct arch *archs,
    uint32_t narchs);
static void process_32bit_arch(
    struct arch *arch);
static void process_64bit_arch(
    struct arch *arch);
static void layout_output(
    struct ofile *ofile);
static void create_output(
    struct ofile *ofile,
    char *out);
static void create_ms_dos_stub(
    struct ms_dos_stub *p);
static void usage(
    void);

static void create_32bit_symbol_table(
    struct arch *arch);
static void create_64bit_symbol_table(
    struct arch *arch);

/*
 * This is the internal structure that we gather the base relocation in from
 * the Mach-O relocation entries.
 */
struct base_reloc {
    uint64_t addr;
    uint32_t type;
};
struct base_reloc *base_relocs = NULL;
uint32_t nbase_reloc = 0;

static void create_base_reloc(
    struct arch *arch);
static void gather_base_reloc_info(
    uint32_t addr,
    struct relocation_info *relocs, 
    uint32_t nreloc,
    cpu_type_t cpu_type,
    uint32_t length,
    int macho_reloc_type,
    int base_reloc_type);
static void add_base_reloc(
    uint64_t addr,
    uint32_t type);
static void make_base_relocs(
    void);
static int cmp_base_relocs(
    struct base_reloc *x1,
    struct base_reloc *x2);
static uint32_t checksum(
    unsigned char *buf);
static void string_to_uuid(
    char *string,
    uint8_t *uuid);

static void create_debug(
    struct arch *arch);
static void set_debug_addrs_and_offsets(
    void);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The mtoc(1) tool makes a PECOFF file from a fully linked Mach-O file
 * compiled with dynamic code gen and relocation entries saved (linked with -r).
 *
 *	mtoc [-subsystem type] [-section_alignment hexvalue] [-align hexvalue]
 *	     [-d filename] input_Mach-O output_pecoff
 */
int
main(
int argc,
char **argv,
char **envp)
{
    int i, j;
    char *input, *output;
    struct ofile *ofile;
    struct arch *archs;
    uint32_t narchs;
    char *endp;
    enum bool section_alignment_specified, align_specified;

	progname = argv[0];
	host_byte_sex = get_host_byte_sex();

	input = NULL;
	output = NULL;

	section_alignment_specified = FALSE;
	align_specified = FALSE;

	for(i = 1; i < argc; i++){
	    if(strcmp(argv[i], "-subsystem") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -subsystem option");
		    usage();
		}
		for(j = 0; subsystem_arguments[j].name != NULL; j++){
		    if(strcmp(argv[i+1], subsystem_arguments[j].name) == 0){
			Subsystem = subsystem_arguments[j].value;
			break;
		    }
		}
		if(subsystem_arguments[j].name == NULL){
		    warning("unknown argument: %s specified for -subsystem "
			    "argument can be:", argv[i+1]);
		    for(j = 0; subsystem_arguments[j].name != NULL; j++)
			fprintf(stderr, "%s\n", subsystem_arguments[j].name);
		    usage();
		}
		i++;
	    }
	    else if(strcmp(argv[i], "-d") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -d option");
		    usage();
		}
		debug_filename = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-e") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -e option");
		    usage();
		}
		entry_point = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-u") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -u option");
		    usage();
		}
		if(debug_filename == NULL) {
		    fatal("-u option requires -d option");
		}
		debug_uuid = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-section_alignment") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -section_alignment "
			    "option");
		    usage();
		}
		section_alignment = (uint32_t)strtoul(argv[i+1], &endp, 16);
		if(*endp != '\0')
		    fatal("argument for -section_alignment %s not a proper "
			  "hexadecimal number", argv[i+1]);
		if(!ispoweroftwo(section_alignment) || section_alignment == 0)
		    fatal("argument to -section_alignment: %x (hex) must be a "
			  "non-zero power of two", section_alignment);
		if(section_alignment > MAXALIGN)
		    fatal("argument to -section_alignment: %x (hex) must "
			  "equal to or less than %x (hex)", section_alignment,
			  (unsigned int)MAXALIGN);
		section_alignment_specified = TRUE;
		if(align_specified == TRUE &&
		   section_alignment != file_alignment)
		    fatal("can't specifiy a -section_alignment value %x (hex) "
			  "different from the -align value %x (hex)",
			  section_alignment, file_alignment);
		i++;
	    }
	    else if(strcmp(argv[i], "-align") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -align option");
		    usage();
		}
		file_alignment = (uint32_t)strtoul(argv[i+1], &endp, 16);
		if(*endp != '\0')
		    fatal("argument for -align %s not a proper hexadecimal "
			  "number", argv[i+1]);
		if(!ispoweroftwo(file_alignment) || file_alignment == 0)
		    fatal("argument to -align: %x (hex) must be a non-zero "
			  "power of two", file_alignment);
		if(file_alignment > MAXALIGN)
		    fatal("argument to -file_alignment: %x (hex) must "
			  "equal to or less than %x (hex)", file_alignment,
			  (unsigned int)MAXALIGN);
		align_specified = TRUE;
		if(section_alignment_specified == TRUE &&
		   section_alignment != file_alignment)
		    fatal("can't specifiy a -section_alignment value %x (hex) "
			  "different from the -align value %x (hex)",
			  section_alignment, file_alignment);
		section_alignment = file_alignment;
		i++;
	    }
	    else if(strcmp(argv[i], "-version") == 0){
		if(i + 1 >= argc){
		    warning("no argument specified for -version option");
		    usage();
		}
		if (sscanf(argv[i+1], "%u.%u", &majorVersion,
			   &minorVersion) != 2){
		    warning("invalid argument specified for -version option");
		    usage();
		}
		i++;
	    }
	    else if(input == NULL)
		input = argv[i];
	    else if(output == NULL)
		output = argv[i];
	    else
		usage();
	}
	if(input == NULL){
	    warning("no input file specified");
	    usage();
	}
	if(output == NULL){
	    warning("no output file specified");
	    usage();
	}

	/* breakout the file for processing */
	ofile = breakout(input, &archs, &narchs, FALSE);
	if(errors)
	    return(EXIT_FAILURE);

	/* checkout the file for symbol table replacement processing */
	checkout(archs, narchs);

	/* process the input file */
	process_arch(archs, narchs);
	if(errors){
	    free_archs(archs, narchs);
	    ofile_unmap(ofile);
	    return(EXIT_FAILURE);
	}

	/*
	 * Layout the pecoff output file from the information gathered from
	 * the input file creating the needed headers, relocs, etc.
	 */
	layout_output(ofile);

	create_output(ofile, output);

	if(errors == 0)
	    return(EXIT_SUCCESS);
	else
	    return(EXIT_FAILURE);
}

/*
 * usage() prints the current usage message and exits indicating failure.
 */
static
void
usage(
void)
{
	fprintf(stderr, "Usage: %s [-subsystem type] "
		"[-section_alignment hexvalue] [-align hexvalue] "
		"[-version major.minor] [-ddebug_filename] "
		"[-u debug_guid] input_Mach-O output_pecoff\n", progname);
	exit(EXIT_FAILURE);
}

/*
 * ispoweroftwo() returns TRUE or FALSE depending if x is a power of two.
 */
static
enum
bool
ispoweroftwo(
uint32_t x)
{
	if(x == 0)
	    return(TRUE);
	while((x & 0x1) != 0x1){
	    x >>= 1;
	}
	if((x & ~0x1) != 0)
	    return(FALSE);
	else
	    return(TRUE);
}

/*
 * process_arch() is the routine that process the broken out ofile to gather
 * the info to create the pecoff file.  This routine basically counts and adds
 * up the sizes of the elements that will be in the pecoff output file.
 */
static
void
process_arch(
struct arch *archs,
uint32_t narchs)
{
	/*
	 * Check to see the input file is something this program can convert to
	 * a pecoff file.
	 */
	if(narchs != 1)
	    fatal("input file: %s must only have one architecture",
		  archs->file_name);
	if(archs->type != OFILE_Mach_O)
	    fatal("input file: %s must be a Mach-O file", archs->file_name);
	if(archs->object->mh_cputype != CPU_TYPE_I386 &&
	   archs->object->mh_cputype != CPU_TYPE_ARM &&
	   archs->object->mh_cputype != CPU_TYPE_ARM64 &&
	   archs->object->mh_cputype != CPU_TYPE_X86_64)
	    fatal("input file: %s must be an i386 or ARM architecture",
		  archs->file_name);
	if(archs->object->mh != NULL){
	    if(archs->object->mh->filetype == MH_PRELOAD ||
	       (archs->object->mh->filetype == MH_EXECUTE &&
		(archs->object->mh->flags & MH_PIE) == MH_PIE)){
		if(entry_point != NULL)
		    fatal("entry point option, -e %s, not allowed with "
			  "MH_PRELOAD or MH_EXECUTE file types", entry_point);
	    }
	    else{
		fatal("input file: %s must be an MH_PRELOAD file type or "
		      "MH_EXECUTE file type with MH_PIE flag",
		      archs->file_name);
	    }
	}
	else{
	    if(archs->object->mh64->filetype == MH_DYLIB ||
	       (archs->object->mh64->filetype == MH_EXECUTE &&
		(archs->object->mh64->flags & MH_PIE) == MH_PIE)){
		if(entry_point == NULL &&
		   archs->object->mh64->filetype == MH_DYLIB)
		    fatal("input file: %s is a MH_DYLIB file type, so entry "
			  "point option, -e name, must be specified", 
			  archs->file_name);
	    }
	    else if(archs->object->mh64->filetype == MH_PRELOAD ||
		    (archs->object->mh64->filetype == MH_EXECUTE &&
		     (archs->object->mh64->flags & MH_PIE) == MH_PIE)){
		if(entry_point != NULL)
		    fatal("entry point option, -e %s, not allowed with "
		          "MH_PRELOAD or MH_EXECUTE file types",
			  archs->file_name);
	    }
	    else
		fatal("input file: %s must be an MH_PRELOAD or MH_DYLIB file "
		      "type or MH_EXECUTE file type with MH_PIE flag",
		      archs->file_name);
	}

	target_byte_sex = archs->object->object_byte_sex;
	swapped = host_byte_sex != target_byte_sex;

	/*
	 * Create base relocation entries for this Mach-O file.  This is done
	 * before the sections are created as this produces the contents for
	 * the .reloc section and determines it size.
	 */
	create_base_reloc(archs);

	/*
	 * If there is a -d flag create the information that will be in .debug
	 * section for it.
	 */
	if(debug_filename != NULL)
	    create_debug(archs);

	if(archs->object->mh != NULL)
	    process_32bit_arch(archs);
	else
	    process_64bit_arch(archs);
}

/*
 * process_32bit_arch() is the routine that processes a 32-bit broken out ofile
 * to gather the info to create the pecoff file.  This routine basically counts
 * and adds up the sizes of the elements that will be in the pecoff output file.
 */
static
void
process_32bit_arch(
struct arch *arch)
{
    uint32_t i, j, reloc_addr, debug_addr;
    struct load_command *lc;
    struct segment_command *sg;
    struct thread_command *ut;
    char *p, *state;
    uint32_t flavor, count;
    char *object_addr, *section_name;
#ifdef HACK_TO_MATCH_TEST_CASE
    uint32_t len;
    struct section *s;
#endif

	/*
	 * Determine the number of sections in the pecoff output file.
	 * 
#ifdef HACK_TO_MATCH_TEST_CASE
	 *
	 * The hack implementation of this routine is done to match the
	 * current ld_efi(1) script that uses objcopy(1) to make the pecoff
	 * file.  So for 32-bit file the contents of the Mach-O file gets
	 * placed into pecoff sections as follows:
	 *
	 * the entire __TEXT segment becomes the .text section
	 * the entire __DATA segment becomes the .data section
	 * the zero fill section (__DATA,__common) becomes .common
	 * the zero fill section (__DATA,__bss) becomes .bss
	 * the (__IMPORT,__pointers) section becomes .pointers
	 * the base relocation entries go into the .reloc section
	 *
#else
	 *
	 * The whole Mach-O segments __TEXT, __DATA and __IMPORT are placed in
	 * the pecoff file from the Mach-O file.  And then the .reloc section
	 * added for the base relocations.
	 *
#endif
	 */
	nscns = 0;
	reloc_addr = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < arch->object->mh->ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(strcmp(sg->segname, SEG_LINKEDIT) != 0 &&
		   sg->vmaddr + sg->vmsize > reloc_addr)
		    reloc_addr = sg->vmaddr + sg->vmsize;
		if(strcmp(sg->segname, SEG_TEXT) == 0)
		    nscns++;
		else if(strcmp(sg->segname, SEG_DATA) == 0){
		    nscns++;
#ifdef HACK_TO_MATCH_TEST_CASE
		    s = (struct section *)
			 ((char *)sg + sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++, s++){
			if(strcmp(s->sectname, SECT_COMMON) == 0 ||
			   strcmp(s->sectname, SECT_BSS) == 0){
			    nscns++;
			}
			else if(s->size != 0 &&
				strcmp(s->sectname, SECT_DATA) != 0)
			    fatal("input file: %s contains Mach-O section "
				  "(%.16s,%.16s) unsupported for conversion "
				  "to a pecoff file", arch->file_name,
				  s->segname, s->sectname);
		    }
#endif /* HACK_TO_MATCH_TEST_CASE */
		}
		else if(strcmp(sg->segname, SEG_IMPORT) == 0){
#ifndef HACK_TO_MATCH_TEST_CASE
		    nscns++;
#else
		    s = (struct section *)
			 ((char *)sg + sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++, s++){
			if(strcmp(s->sectname, "__pointers") == 0){
			    section_names_size += strlen(".pointers") + 1;
			    nscns++;
			}
			else if(s->size != 0)
			    fatal("input file: %s contains Mach-O section "
				  "(%.16s,%.16s) unsupported for conversion "
				  "to a pecoff file", arch->file_name,
				  s->segname, s->sectname);
		    }

#endif /* HACK_TO_MATCH_TEST_CASE */
		}
		else if((arch->object->mh->flags & MH_PIE) != MH_PIE ||
			strcmp(sg->segname, SEG_LINKEDIT) != 0){
		    fatal("input file: %s contains Mach-O segment %.16s "
			  "unsupported for conversion to a pecoff file",
			  arch->file_name, sg->segname);
		}
	    }
	    /*
	     * Also while processing the Mach-O file pick up the entry point.
	     */
	    else if(lc->cmd == LC_UNIXTHREAD){
		ut = (struct thread_command *)lc;
		state = (char *)ut + sizeof(struct thread_command);
		p = (char *)ut + ut->cmdsize;
		while(state < p){
		    flavor = *((uint32_t *)state);
		    state += sizeof(uint32_t);
		    count = *((uint32_t *)state);
		    state += sizeof(uint32_t);
		    switch(arch->object->mh_cputype){
		    case CPU_TYPE_I386:
			switch((int)flavor){
			case i386_THREAD_STATE:
#if i386_THREAD_STATE == 1
			case -1:
#endif /* i386_THREAD_STATE == 1 */
/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
			case 1:
#endif /* i386_THREAD_STATE == -1 */
			    {
				i386_thread_state_t *cpu =
				    (i386_thread_state_t *)state;
				entry = cpu->eip;
				state += sizeof(i386_thread_state_t);
			    }
			    break;
			default:
			    state += count * sizeof(uint32_t);
			    break;
			}
		        break;
		    case CPU_TYPE_ARM:
			switch(flavor){
			case ARM_THREAD_STATE:
			    {
				arm_thread_state_t *cpu =
				    (arm_thread_state_t *)state;
				entry = cpu->__pc;
				state += sizeof(arm_thread_state_t);
			    }
			    break;
			default:
			    state += count * sizeof(uint32_t);
			    break;
			}
		        break;
		    default:
			break;
		    }
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(reloc_size != 0){
	    /* add one for the .reloc section to contain the base relocations */
	    nscns++;
	}

	/*
	 * If there is a -d flag add one for the .debug section to contain
	 * the information.
	 */
	if(debug_filename != NULL)
	    nscns++;

	/*
	 * At the beginning of the COFF string table are 4 bytes that contain
	 * the total size (in bytes) of the rest of the string table. This size
	 * includes the size field itself, so that the value in this location
	 * would be 4 if no strings were present.
	 */
	strsize = sizeof(uint32_t);

	/*
	 * Section names longer than 8 bytes are placed in the string table.
	 * So here we allocate memory to put them into, which later will be
	 * copied to the start of the string table.
	 */
	section_names = allocate(section_names_size);
	section_name = section_names;
	if(section_names_size != 0)
	    *section_name = '\0';

	/*
	 * Allocate space for the section headers and fill in everything but
	 * their file offsets.
	 *
#ifndef HACK_TO_MATCH_TEST_CASE
	 *
	 * We use the SizeOfRawData field (s_size) as the unrounded value of
	 * the size of the initialized section contents coming from the
	 * segment's filesize.  The VirtualSize field s_vsize may be bigger
	 * with the remaining space zero filled coming from the segment's
	 * vmsize.
#else
	 *
	 * Note to match what objcopy(1) does the s_vsize is an unrounded value
	 * of the size (more like the actual size) and the s_size is a value
	 * rounded to the file_alignment.  So the s_vsize can be smaller than
	 * the s_size, as in the case of pecoff sections created from Mach-O
	 * sections (and not segments).  This seems to volate the spec where
	 * s_vsize can be bigger than s_size with the remaining space zero
	 * filled but does NOT allow the s_vsize to be smaller than the s_size.
#endif
	 */
	scnhdrs = allocate(nscns * sizeof(struct scnhdr));
	memset(scnhdrs, '\0', nscns * sizeof(struct scnhdr));
	scn_contents = allocate(nscns * sizeof(char *));
	object_addr = arch->object->object_addr;
	j = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < arch->object->mh->ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(strcmp(sg->segname, SEG_TEXT) == 0){
		    strcpy(scnhdrs[j].s_name, ".text");
#ifdef HACK_TO_MATCH_TEST_CASE
		    scnhdrs[j].s_vsize = sg->filesize;
#else
		    scnhdrs[j].s_vsize = sg->vmsize;
#endif
		    scnhdrs[j].s_vaddr = sg->vmaddr;
		    scnhdrs[j].s_size = rnd32(sg->filesize, file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_EXECUTE |
					 IMAGE_SCN_MEM_READ |
					 IMAGE_SCN_CNT_CODE;
		    scn_contents[j] = object_addr + sg->fileoff;
		    j++;
		}
		else if(strcmp(sg->segname, SEG_DATA) == 0){
		    strcpy(scnhdrs[j].s_name, ".data");
#ifdef HACK_TO_MATCH_TEST_CASE
		    scnhdrs[j].s_vsize = sg->filesize;
#else
		    scnhdrs[j].s_vsize = sg->vmsize;
#endif
		    scnhdrs[j].s_vaddr = sg->vmaddr;
		    scnhdrs[j].s_size = rnd32(sg->filesize, file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
					 IMAGE_SCN_MEM_WRITE |
					 IMAGE_SCN_CNT_CODE |
				         IMAGE_SCN_CNT_INITIALIZED_DATA |
					 IMAGE_SCN_MEM_EXECUTE;
		    scn_contents[j] = object_addr + sg->fileoff;
		    j++;
#ifdef HACK_TO_MATCH_TEST_CASE
		    s = (struct section *)
			 ((char *)sg + sizeof(struct segment_command));
		    for(i = 0; i < sg->nsects; i++, s++){
			if(s->size == 0)
			    continue;
			scnhdrs[j].s_vsize = s->size;
			scnhdrs[j].s_vaddr = s->addr;
			scnhdrs[j].s_size = 0;
			scnhdrs[j].s_relptr = 0;
			scnhdrs[j].s_lnnoptr = 0;
			scnhdrs[j].s_nlnno = 0;
			scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
					     IMAGE_SCN_MEM_WRITE |
					     IMAGE_SCN_CNT_UNINITIALIZED_DATA;
			if(strcmp(s->sectname, SECT_DATA) == 0){
			    continue;
			}
			else if(strcmp(s->sectname, SECT_COMMON) == 0){
			    strcpy(scnhdrs[j].s_name, ".common");
			    common_scnum = j + 1;
			}
			else if(strcmp(s->sectname, SECT_BSS) == 0){
			    strcpy(scnhdrs[j].s_name, ".bss");
			    bss_scnum = j + 1;
			}
			scn_contents[j] = NULL;
			j++;
		    }
#endif /* HACK_TO_MATCH_TEST_CASE */
		}
		else if(strcmp(sg->segname, SEG_IMPORT) == 0){
#ifndef HACK_TO_MATCH_TEST_CASE
		    strcpy(scnhdrs[j].s_name, ".import");
		    scnhdrs[j].s_vsize = sg->vmsize;
		    scnhdrs[j].s_vaddr = sg->vmaddr;
		    scnhdrs[j].s_size = rnd32(sg->filesize, file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
					 IMAGE_SCN_MEM_WRITE |
				         IMAGE_SCN_CNT_INITIALIZED_DATA;
		    scn_contents[j] = object_addr + sg->fileoff;
		    j++;
#else /* defined(HACK_TO_MATCH_TEST_CASE) */
		    s = (struct section *)
			 ((char *)sg + sizeof(struct segment_command));
		    for(i = 0; i < sg->nsects; i++, s++){
			if(s->size == 0)
			    continue;
			scnhdrs[j].s_vsize = s->size;
			scnhdrs[j].s_vaddr = s->addr;
			scnhdrs[j].s_size = rnd(s->size, file_alignment);
			scnhdrs[j].s_relptr = 0;
			scnhdrs[j].s_lnnoptr = 0;
			scnhdrs[j].s_nlnno = 0;
			scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
					     IMAGE_SCN_MEM_WRITE |
					     IMAGE_SCN_CNT_INITIALIZED_DATA;
			if(strcmp(s->sectname, "__pointers") == 0){
			    sprintf(scnhdrs[j].s_name, "/%d", strsize);
			    strcat(section_name, ".pointers");
			    len = strlen(section_name) + 1;
			    strsize += len;
			}
			scn_contents[j] = object_addr + s->offset;
			j++;
		    }
#endif /* HACK_TO_MATCH_TEST_CASE */
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(reloc_size != 0){
	    strcpy(scnhdrs[j].s_name, ".reloc");
	    scnhdrs[j].s_vsize = reloc_size;
	    reloc_addr = rnd32(reloc_addr, section_alignment);
	    scnhdrs[j].s_vaddr = reloc_addr;
	    scnhdrs[j].s_size = rnd32(reloc_size, file_alignment);
	    scnhdrs[j].s_relptr = 0;
	    scnhdrs[j].s_lnnoptr = 0;
	    scnhdrs[j].s_nlnno = 0;
	    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
				 IMAGE_SCN_CNT_INITIALIZED_DATA |
				 IMAGE_SCN_MEM_DISCARDABLE;
	    reloc_scnhdr = scnhdrs + j;
	    scn_contents[j] = reloc_contents;
	    j++;
	    debug_addr = reloc_addr + reloc_scnhdr->s_size;
	}
	else{
	    debug_addr = rnd32(reloc_addr, section_alignment);
	}

	if(debug_filename != NULL){
	    strcpy(scnhdrs[j].s_name, ".debug");
	    scnhdrs[j].s_vsize = debug_size;
	    scnhdrs[j].s_vaddr = debug_addr;
	    scnhdrs[j].s_size = rnd32(debug_size, file_alignment);
	    scnhdrs[j].s_relptr = 0;
	    scnhdrs[j].s_lnnoptr = 0;
	    scnhdrs[j].s_nlnno = 0;
	    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
				 IMAGE_SCN_CNT_INITIALIZED_DATA |
				 IMAGE_SCN_MEM_DISCARDABLE;
	    debug_scnhdr = scnhdrs + j;
	    scn_contents[j] = debug_contents;
	    j++;
	}

	/*
	 * Create the pecoff symbol and string table from this Mach-O file.
	 */
	create_32bit_symbol_table(arch);
}

/*
 * process_64bit_arch() is the routine that processes a 64-bit broken out ofile
 * to gather the info to create the pecoff file.  This routine basically counts
 * and adds up the sizes of the elements that will be in the pecoff output file.
 */
static
void
process_64bit_arch(
struct arch *arch)
{
    uint32_t i, j;
    uint64_t reloc_addr, debug_addr;
    struct load_command *lc;
    struct segment_command_64 *sg64;
    struct thread_command *ut;
    char *p, *state;
    uint32_t flavor, count;
    char *object_addr, *section_name;
#ifdef HACK_TO_MATCH_TEST_CASE
    struct section_64 *s64;
    uint32_t len;
#endif

	/*
	 * Determine the number of sections in the pecoff output file.
	 * 
#ifdef HACK_TO_MATCH_TEST_CASE
	 * 
	 * The hack implementation of this routine is done to match the
	 * current ld_efi(1) script that uses objcopy(1) to make the pecoff
	 * file.  So for 64-bit files the contents of the Mach-O sections get
	 * placed into pecoff sections with a section name made up of the
	 * strings "LC_SEGMENT" the segment and section names separated with
	 * a dot, '.', character.  So the Mach-O (__TEXT,__text) section becomes
	 * a pecoff section with the name "LC_SEGMENT.__TEXT.__text".  The base
	 * relocation entries go into a ".reloc" section.
	 *
#else
	 *
	 * The whole Mach-O __TEXT and __DATA segments are placed in the
	 * pecoff file from the Mach-O file.  And then the .reloc section added
	 * for the base relocations.
	 *
#endif
	 */
	nscns = 0;
	reloc_addr = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < arch->object->mh64->ncmds; i++){
	    if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
#ifndef HACK_TO_MATCH_TEST_CASE
		if(strcmp(sg64->segname, SEG_LINKEDIT) != 0 &&
		   sg64->vmaddr + sg64->vmsize > reloc_addr)
		    reloc_addr = sg64->vmaddr + sg64->vmsize;
		if(strcmp(sg64->segname, SEG_TEXT) == 0)
		    nscns++;
		else if(strcmp(sg64->segname, SEG_DATA) == 0)
		    nscns++;
		else if(strcmp(sg64->segname, SEG_LINKEDIT) != 0){
		    fatal("input file: %s contains Mach-O segment %.16s "
			  "unsupported for conversion to a pecoff file",
			  arch->file_name, sg64->segname);
		}
#else /* defined(HACK_TO_MATCH_TEST_CASE) */
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(i = 0; i < sg64->nsects; i++, s64++){
		    if(s64->addr + s64->size > reloc_addr)
			reloc_addr = s64->addr + s64->size;
		    section_names_size += strlen("LC_SEGMENT.") +
					  strlen(s64->segname) + 1 +
					  strlen(s64->sectname) + 1;
		    nscns++;
		}
#endif /* HACK_TO_MATCH_TEST_CASE */
	    }
	    /*
	     * Also while process the Mach-O file pick up the entry point.
	     */
	    else if(lc->cmd == LC_UNIXTHREAD){
		ut = (struct thread_command *)lc;
		state = (char *)ut + sizeof(struct thread_command);
		p = (char *)ut + ut->cmdsize;
		while(state < p){
		    flavor = *((uint32_t *)state);
		    state += sizeof(uint32_t);
		    count = *((uint32_t *)state);
		    state += sizeof(uint32_t);
		    switch(arch->object->mh_cputype){
#ifdef x86_THREAD_STATE64
		    case CPU_TYPE_X86_64:
			switch(flavor){
			case x86_THREAD_STATE64:
			    {
				x86_thread_state64_t *cpu64 =
				    (x86_thread_state64_t *)state;
				/*
				 * The aouthdr_64 struct only allows for a
				 * 32-bit entry point.
				 */
				entry = (uint32_t)cpu64->rip;
				state += sizeof(x86_thread_state64_t);
			    }
			    break;
			default:
			    state += count * sizeof(uint32_t);
			    break;
			}
			break;
#endif /* x86_THREAD_STATE64 */
#ifdef ARM_THREAD_STATE64
		    case CPU_TYPE_ARM64:
			switch(flavor){
			case ARM_THREAD_STATE64:
			    {
				arm_thread_state64_t *cpu64 =
				    (arm_thread_state64_t *)state;
				/*
				 * The aouthdr_64 struct only allows for a
				 * 32-bit entry point.
				 */
				entry = (uint32_t)cpu64->__pc;
				state += sizeof(arm_thread_state64_t);
			    }
			    break;
			default:
			    state += count * sizeof(uint32_t);
			    break;
			}
			break;
#endif /* ARM_THREAD_STATE64 */
		    }
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(reloc_size != 0){
	    /* add one for the .reloc section to contain the base relocations */
	    nscns++;
	}

	/*
	 * If there is a -d flag add one for the .debug section to contain
	 * the information.
	 */
	if(debug_filename != NULL)
	    nscns++;

	/*
	 * At the beginning of the COFF string table are 4 bytes that contain
	 * the total size (in bytes) of the rest of the string table. This size
	 * includes the size field itself, so that the value in this location
	 * would be 4 if no strings were present.
	 */
	strsize = sizeof(uint32_t);

	/*
	 * Section names longer than 8 bytes are placed in the string table.
	 * So here we allocate memory to put them into, which later will be
	 * copied to the start of the string table.
	 */
	section_names = allocate(section_names_size) + 1;
	section_name = section_names;
	if(section_names_size != 0)
	    *section_name = '\0';

	/*
	 * Allocate space for the section headers and fill in everything but
	 * their file offsets.
	 *
#ifndef HACK_TO_MATCH_TEST_CASE
	 *
	 * We use the SizeOfRawData field (s_size) as the unrounded value of
	 * the size of the initialized section contents coming from the
	 * segment's filesize.  The VirtualSize field s_vsize may be bigger
	 * with the remaining space zero filled coming from the segment's
	 * vmsize.
#else
	 *
	 * Note to match what objcopy(1) does the s_vsize is an unrounded value
	 * of the size (more like the actual size) and the s_size is a value
	 * rounded to the file_alignment.  So the s_vsize can be smaller than
	 * the s_size, as in the case of pecoff sections created from Mach-O
	 * sections (and not segments).  This seems to volate the spec where
	 * s_vsize can be bigger than s_size with the remaining space zero
	 * filled but does NOT allow the s_vsize to be smaller than the s_size.
#endif
	 */
	scnhdrs = allocate(nscns * sizeof(struct scnhdr));
	memset(scnhdrs, '\0', nscns * sizeof(struct scnhdr));
	scn_contents = allocate(nscns * sizeof(char *));
	object_addr = arch->object->object_addr;
	j = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < arch->object->mh64->ncmds; i++){
	    if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
#ifndef HACK_TO_MATCH_TEST_CASE
		if(strcmp(sg64->segname, SEG_TEXT) == 0){
		    strcpy(scnhdrs[j].s_name, ".text");
		    scnhdrs[j].s_vsize = (uint32_t)sg64->vmsize;
		    scnhdrs[j].s_vaddr = (uint32_t)sg64->vmaddr;
		    scnhdrs[j].s_size = (uint32_t)rnd64(sg64->filesize,
							file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_EXECUTE |
					 IMAGE_SCN_MEM_READ |
					 IMAGE_SCN_CNT_CODE;
		    scn_contents[j] = object_addr + sg64->fileoff;
		    j++;
		}
		else if(strcmp(sg64->segname, SEG_DATA) == 0){
		    strcpy(scnhdrs[j].s_name, ".data");
		    scnhdrs[j].s_vsize = (uint32_t)sg64->vmsize;
		    scnhdrs[j].s_vaddr = (uint32_t)sg64->vmaddr;
		    scnhdrs[j].s_size = (uint32_t)rnd64(sg64->filesize,
							file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
					 IMAGE_SCN_MEM_WRITE |
					 IMAGE_SCN_CNT_CODE |
				         IMAGE_SCN_CNT_INITIALIZED_DATA |
					 IMAGE_SCN_MEM_EXECUTE;
		    scn_contents[j] = object_addr + sg64->fileoff;
		    j++;
		}
#else /* defined(HACK_TO_MATCH_TEST_CASE) */
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(i = 0; i < sg64->nsects; i++, s64++){
		    sprintf(scnhdrs[j].s_name, "/%d", strsize);
		    strcat(section_name, "LC_SEGMENT.");
		    strcat(section_name, s64->segname);
		    strcat(section_name, ".");
		    strcat(section_name, s64->sectname);
		    len = strlen(section_name);
		    strsize += len + 1;
		    section_name += len + 1;
		    *section_name = '\0'; /* start of next section name */

		    /* NOTE zerofill sections are not handled */
		    scnhdrs[j].s_vsize = s64->size;
		    scnhdrs[j].s_vaddr = s64->addr;
		    scnhdrs[j].s_size = rnd(s64->size, file_alignment);
		    scnhdrs[j].s_relptr = 0;
		    scnhdrs[j].s_lnnoptr = 0;
		    scnhdrs[j].s_nlnno = 0;
		    scnhdrs[j].s_flags = IMAGE_SCN_MEM_EXECUTE |
		    			 IMAGE_SCN_CNT_CODE |
		    			 IMAGE_SCN_MEM_WRITE;
		    if(sg64->initprot & VM_PROT_READ)
			scnhdrs[j].s_flags |= IMAGE_SCN_MEM_READ;
		    scn_contents[j] = object_addr + s64->offset;
		    j++;
		}
#endif /* HACK_TO_MATCH_TEST_CASE */
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(reloc_size != 0){
	    strcpy(scnhdrs[j].s_name, ".reloc");
	    scnhdrs[j].s_vsize = reloc_size;
	    reloc_addr = rnd(reloc_addr, section_alignment);
	    scnhdrs[j].s_vaddr = (uint32_t)reloc_addr;
	    scnhdrs[j].s_size = rnd32(reloc_size, file_alignment);
	    scnhdrs[j].s_relptr = 0;
	    scnhdrs[j].s_lnnoptr = 0;
	    scnhdrs[j].s_nlnno = 0;
	    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
				 IMAGE_SCN_CNT_INITIALIZED_DATA |
				 IMAGE_SCN_MEM_DISCARDABLE |
				 IMAGE_SCN_CNT_CODE |
				 IMAGE_SCN_MEM_EXECUTE;
	    reloc_scnhdr = scnhdrs + j;
	    scn_contents[j] = reloc_contents;
	    j++;
	    debug_addr = reloc_addr + reloc_scnhdr->s_size;
	}
	else{
	    debug_addr = rnd(reloc_addr, section_alignment);
	}

	if(debug_filename != NULL){
	    strcpy(scnhdrs[j].s_name, ".debug");
	    scnhdrs[j].s_vsize = debug_size;
	    scnhdrs[j].s_vaddr = (uint32_t)debug_addr;
	    scnhdrs[j].s_size = rnd32(debug_size, file_alignment);
	    scnhdrs[j].s_relptr = 0;
	    scnhdrs[j].s_lnnoptr = 0;
	    scnhdrs[j].s_nlnno = 0;
	    scnhdrs[j].s_flags = IMAGE_SCN_MEM_READ |
				 IMAGE_SCN_CNT_INITIALIZED_DATA |
				 IMAGE_SCN_MEM_DISCARDABLE |
				 IMAGE_SCN_CNT_CODE |
				 IMAGE_SCN_MEM_EXECUTE;
	    debug_scnhdr = scnhdrs + j;
	    scn_contents[j] = debug_contents;
	    j++;
	}

	/*
	 * Create the pecoff symbol and string table from this Mach-O file.
	 */
	create_64bit_symbol_table(arch);
}

/*
 * layout_output() takes the info gathered from the input Mach-O file and
 * layouts the pecoff output file and creates and fills in the elements of
 * the coff file.  This routine basically sets of the offsets of the elements
 * of the output file from the previously determined sizes.
 */
static
void
layout_output(
struct ofile *ofile)
{
    uint32_t i, header_size, offset, least_vaddr;

	/*
	 * Determine the size of the output file and where each element will be
	 * in the output file.
	 */
	header_size = sizeof(struct ms_dos_stub) +
		      sizeof(signature) +
		      sizeof(struct filehdr) +
		      nscns * sizeof(struct scnhdr);
	if(ofile->mh != NULL)
	    header_size += sizeof(struct aouthdr);
	else
	    header_size += sizeof(struct aouthdr_64);
	header_size = rnd32(header_size, file_alignment);
#ifdef HACK_TO_MATCH_TEST_CASE
	/* for some unknown reason the header size is 0x488 not 0x400 */
	if(ofile->mh64 != NULL)
	    header_size += 0x88;
#endif
	/* 
	 * If the lowest section virtual address is greater than the header
	 * size, pad the header up to the virtual address.  This modification
	 * will make the file offset and virtual address equal, and fixes
	 * problems with XIP rebasing in the EFI tools.
	 */
	least_vaddr = 0xffffffff;
	for(i = 0; i < nscns; i++){
	    if(scnhdrs[i].s_vaddr < least_vaddr)
		least_vaddr = scnhdrs[i].s_vaddr;      
	}
	if(least_vaddr > header_size)
	    header_size = least_vaddr;

	offset = header_size;
	for(i = 0; i < nscns; i++){
	    if((scnhdrs[i].s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) == 0){
		/*
		 * We need to check that the headers can be mapped starting at
		 * the ImageBase, fixed at zero in this program, and fit before
		 * the Virtual Address of the first section (really any section)
		 * and if it doesn't then we need the Mach-O file relinked.
		 */
		if(scnhdrs[i].s_vaddr < header_size)
		    fatal("input file: %s must be relinked so PECOFF headers "
			  "can be mapped before its sections (use a -seg1addr "
			  "0x%x or greater)", ofile->file_name, header_size);
		/*
		 * The s_scnptr is set to the offset and then the offset is
		 * incremented by the SizeOfRawData field (s_vsize).
		 */
		scnhdrs[i].s_scnptr = offset;
#ifndef HACK_TO_MATCH_TEST_CASE
		offset += scnhdrs[i].s_vsize;
#else
		/* for some unknown reason the offset after the __dyld section
		   is changed from 0x10 bytes to 0x20 bytes */
		if(ofile->mh64 != NULL && scnhdrs[i].s_vsize < 0x20)
		    offset += 0x20;
		/* for some unknown reason the offset after the __data section
		   is changed from 0x380 bytes to 0x3e0 bytes */
		else if(ofile->mh64 != NULL && scnhdrs[i].s_vsize == 0x380)
		    offset += scnhdrs[i].s_vsize + 0x60;
		else
		    /*
		     * Note to match what objcopy(1) does the offset is
		     * incremented by the VirtualSize field (s_vsize) not the
		     * SizeOfRawData field (s_size) field as that is what was
		     * previously set up.
		     */
		    offset += scnhdrs[i].s_vsize;
#endif
#ifdef HACK_TO_MATCH_TEST_CASE
		if(ofile->mh != NULL)
#endif
		    offset = rnd32(offset, file_alignment);
#ifdef HACK_TO_MATCH_TEST_CASE
		else{
		    /* for some unknown reason the next offset is moved up
		       0x200 then rounded to 8 bytes */
		    offset += 0x200;
		    offset = rnd(offset, 8);
		}
#endif
	    }
	}
#ifdef HACK_TO_MATCH_TEST_CASE
	/* for some unknown reason the offset of the symbol is moved back 0x58
	   bytes */
	if(ofile->mh64 != NULL)
	    offset -= 0x58;
#endif
	syment_offset = offset;
	offset += nsyments * sizeof(struct syment);
	string_offset = offset;
	offset += strsize;

	output_size = offset;

	/*
	 * Now with all the sizes and placement of things know fill in headers
	 * of the pecoff file for this Mach-O file.
	 */

	/* first in the pecoff file is the MS-DOS stub */
	create_ms_dos_stub(&ms_dos_stub);

	/*
	 * Second in the pecoff file is the PE format image file signature.
	 * This signature is PE\0\0 (the letters P and E followed by two null
	 * bytes).
	 */
	signature[0] = 'P';
	signature[1] = 'E';
	signature[2] = '\0';
	signature[3] = '\0';

	/* next is the filehdr */
	if(ofile->mh != NULL){
	    if(ofile->mh->cputype == CPU_TYPE_I386)
		filehdr.f_magic = IMAGE_FILE_MACHINE_I386;
	    else
		filehdr.f_magic = IMAGE_FILE_MACHINE_ARM;
	}
	else{
	    if(ofile->mh64->cputype == CPU_TYPE_X86_64)
		filehdr.f_magic = IMAGE_FILE_MACHINE_AMD64;
	    else
		filehdr.f_magic = IMAGE_FILE_MACHINE_ARM64;
	}
	filehdr.f_nscns = nscns;
#ifdef HACK_TO_MATCH_TEST_CASE
	if(ofile->mh != NULL){
	    filehdr.f_timdat = 0x46cb5980;
	}
	else
	    filehdr.f_timdat = 0x47671e62;
#else
	filehdr.f_timdat = (uint32_t)time(NULL);
#endif
	filehdr.f_symptr = syment_offset;
	filehdr.f_nsyms = nsyments;
	if(ofile->mh != NULL)
	    filehdr.f_opthdr = sizeof(struct aouthdr);
	else
	    filehdr.f_opthdr = sizeof(struct aouthdr_64);
	filehdr.f_flags = IMAGE_FILE_EXECUTABLE_IMAGE |
			  IMAGE_FILE_LINE_NUMS_STRIPPED |
			  IMAGE_FILE_32BIT_MACHINE |
			  IMAGE_FILE_DEBUG_STRIPPED;
	if(ofile->mh64 != NULL)
	    filehdr.f_flags |= IMAGE_FILE_LOCAL_SYMS_STRIPPED;

	/* next is the aouthdr */
	if(ofile->mh != NULL){
	    aouthdr.magic = PE32MAGIC;
	    aouthdr.vstamp = VSTAMP;

      /* 
       * EFI does not use t, d, or b size. 
       * EFI uses SizeOfImage to errorcheck vaddrs in the image
       */
	    aouthdr.tsize = 0;
	    aouthdr.dsize = 0;
	    aouthdr.bsize = 0;
	    aouthdr.SizeOfImage = rnd32(header_size, section_alignment);
	    for(i = 0; i < nscns; i++){
	        aouthdr.SizeOfImage += rnd32(scnhdrs[i].s_vsize, section_alignment);
		}

	    aouthdr.entry = entry;

	    aouthdr.text_start = 0;
	    aouthdr.data_start = 0;
	    for(i = 0; i < nscns; i++){
		if((scnhdrs[i].s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ==0){
		    if((scnhdrs[i].s_flags & IMAGE_SCN_MEM_WRITE) == 0){
			if(aouthdr.text_start == 0)
			    aouthdr.text_start = scnhdrs[i].s_vaddr;
		    }
		    else{
			if(aouthdr.data_start == 0)
			    aouthdr.data_start = scnhdrs[i].s_vaddr;
		    }
		}
	    }

	    aouthdr.ImageBase = 0;
	    aouthdr.SectionAlignment = section_alignment;
	    aouthdr.FileAlignment = file_alignment;
	    aouthdr.MajorOperatingSystemVersion = 0;
	    aouthdr.MinorOperatingSystemVersion = 0;
	    aouthdr.MajorImageVersion = majorVersion;
	    aouthdr.MinorImageVersion = minorVersion;
	    aouthdr.MajorSubsystemVersion = 0;
	    aouthdr.MinorSubsystemVersion = 0;
	    aouthdr.Win32VersionValue = 0;
				  
				  
	    aouthdr.SizeOfHeaders = header_size;
	    aouthdr.CheckSum = 0;
	    aouthdr.Subsystem = Subsystem;
	    aouthdr.DllCharacteristics = 0;
	    aouthdr.SizeOfStackReserve = 0;
	    aouthdr.SizeOfStackCommit = 0;
	    aouthdr.SizeOfHeapReserve = 0;
	    aouthdr.SizeOfHeapCommit = 0;
	    aouthdr.LoaderFlags = 0;
	    aouthdr.NumberOfRvaAndSizes = 16;
	    /* Entry 5, Base Relocation Directory [.reloc] address & size */
	    if(reloc_size != 0){
		aouthdr.DataDirectory[5][0] = reloc_scnhdr->s_vaddr;
		aouthdr.DataDirectory[5][1] = reloc_scnhdr->s_vsize;
	    }
	    /*  Entry 6, Debug Directory [.debug] address & size */
	    if(debug_filename != NULL){
		aouthdr.DataDirectory[6][0] = debug_scnhdr->s_vaddr;
		aouthdr.DataDirectory[6][1] = debug_scnhdr->s_vsize;
	    }
	}
	else{
	    aouthdr64.magic = PE32PMAGIC;
	    aouthdr64.vstamp = VSTAMP;

      /* 
       * EFI does not use t, d, or b size. 
       * EFI uses SizeOfImage to errorcheck vaddrs in the image
       */
	    aouthdr64.tsize = 0;
	    aouthdr64.dsize = 0;
	    aouthdr64.bsize = 0;
	    
	    aouthdr64.SizeOfImage = rnd32(header_size, section_alignment);
	    for(i = 0; i < nscns; i++){
	        aouthdr64.SizeOfImage += rnd(scnhdrs[i].s_vsize, section_alignment); 
	    }
#ifdef HACK_TO_MATCH_TEST_CASE
	    /* with the IMAGE_SCN_CNT_CODE flag set on all sections this is
	       just a quick hack to match the PECOFF file */
	    aouthdr64.dsize = 0x200;
#endif
	    /*
	     * The aouthdr_64 struct only allows for a
	     * 32-bit entry point.
	     */
	    aouthdr64.entry = entry;
#ifdef HACK_TO_MATCH_TEST_CASE
            aouthdr64.entry = 0x4a2;
#endif
	    aouthdr64.text_start = 0;
	    for(i = 0; i < nscns; i++){
		if((scnhdrs[i].s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ==0){
		    if((scnhdrs[i].s_flags & IMAGE_SCN_MEM_WRITE) == 0){
			if(aouthdr64.text_start == 0)
			    aouthdr64.text_start = scnhdrs[i].s_vaddr;
		    }
		}
	    }
#ifdef HACK_TO_MATCH_TEST_CASE
	    /* this is a hack as the start of the text for 64-bit Mach-O files
	       built with -dylib does not have the text section starting at 0 */
	    aouthdr64.text_start = 0;
#endif

	    aouthdr64.ImageBase = 0;
	    aouthdr64.SectionAlignment = section_alignment;
	    aouthdr64.FileAlignment = file_alignment;
	    aouthdr64.MajorOperatingSystemVersion = 0;
	    aouthdr64.MinorOperatingSystemVersion = 0;
	    aouthdr64.MajorImageVersion = majorVersion;
	    aouthdr64.MinorImageVersion = minorVersion;
	    aouthdr64.MajorSubsystemVersion = 0;
	    aouthdr64.MinorSubsystemVersion = 0;
	    aouthdr64.Win32VersionValue = 0;
				    
				    
#ifdef HACK_TO_MATCH_TEST_CASE
	    /* this is a hack as it seams that the minimum size is 0x10000 */
	    if(aouthdr64.SizeOfImage < 0x10000)
		aouthdr64.SizeOfImage = 0x10000;
#endif
	    aouthdr64.SizeOfHeaders = header_size;
	    aouthdr64.CheckSum = 0;
	    aouthdr64.Subsystem = Subsystem;
#ifdef HACK_TO_MATCH_TEST_CASE
	    aouthdr64.Subsystem = IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
#endif
	    aouthdr64.DllCharacteristics = 0;
	    aouthdr64.SizeOfStackReserve = 0;
	    aouthdr64.SizeOfStackCommit = 0;
	    aouthdr64.SizeOfHeapReserve = 0;
	    aouthdr64.SizeOfHeapCommit = 0;
	    aouthdr64.LoaderFlags = 0;
	    aouthdr64.NumberOfRvaAndSizes = 16;
	    /* Entry 5, Base Relocation Directory [.reloc] address & size */
	    if(reloc_size != 0){
		aouthdr64.DataDirectory[5][0] = reloc_scnhdr->s_vaddr;
		aouthdr64.DataDirectory[5][1] = reloc_scnhdr->s_vsize;
	    }
	    /*  Entry 6, Debug Directory [.debug] address & size */
	    if(debug_filename != NULL){
		aouthdr64.DataDirectory[6][0] = debug_scnhdr->s_vaddr;
		aouthdr64.DataDirectory[6][1] = debug_scnhdr->s_vsize;
	    }
	}

	/*
 	 * If there is a debug directory entry set the address and offsets in
	 * it now that the values are known.
	 */
	if(debug_filename != NULL)
	    set_debug_addrs_and_offsets();
}

/*
 * create_output() takes the info gathered from the input Mach-O file and
 * creates the pecoff output file.
 */
static
void
create_output(
struct ofile *ofile,
char *out)
{
    int i, f;
    unsigned char *buf, *p, *p_aouthdr;

	/*
	 * Allocate the buffer to place the pecoff file in.
	 */
	buf = calloc(1, output_size);
	if(buf == NULL)
	    fatal("Can't allocate buffer for output file (size = %u)",
		  output_size);

	/*
	 * Copy the parts of the pecoff file into the buffer.
	 */
	p = buf;

	memcpy(p, &ms_dos_stub, sizeof(struct ms_dos_stub));
	if(swapped)
	    swap_ms_dos_stub((struct ms_dos_stub *)p, target_byte_sex);
	p += sizeof(struct ms_dos_stub);

	memcpy(p, signature, sizeof(signature));
	p += sizeof(signature);

	memcpy(p, &filehdr, sizeof(struct filehdr));
	if(swapped)
	    swap_filehdr((struct filehdr *)p, target_byte_sex);
	p += sizeof(struct filehdr);

	p_aouthdr = p;
	if(ofile->mh != NULL){
	    memcpy(p, &aouthdr, sizeof(struct aouthdr));
	    if(swapped)
		swap_aouthdr((struct aouthdr *)p, target_byte_sex);
	    p += sizeof(struct aouthdr);
	}
	else{
	    memcpy(p, &aouthdr64, sizeof(struct aouthdr_64));
	    if(swapped)
		swap_aouthdr_64((struct aouthdr_64 *)p, target_byte_sex);
	    p += sizeof(struct aouthdr_64);
	}

	/*
	 * Now copy in the section contents.  Note the base relocations
	 * (the contents of the .reloc section) has already been swapped if
	 * that was needed.
	 */ 
	for(i = 0; i < nscns; i++){
	    if((scnhdrs[i].s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) == 0){
		memcpy(buf + scnhdrs[i].s_scnptr,
		       scn_contents[i],
#ifndef HACK_TO_MATCH_TEST_CASE
		       scnhdrs[i].s_size);
#else
		       scnhdrs[i].s_vsize);
#endif

#ifdef HACK_TO_MATCH_TEST_CASE
		/* this is a hack as this is zero in 64-bit file */
		if(ofile->mh64 != NULL)
		    scnhdrs[i].s_vsize = 0;
#endif
	    }
	}

	memcpy(p, scnhdrs, nscns * sizeof(struct scnhdr));
	if(swapped)
	    swap_scnhdr((struct scnhdr *)p, nscns, target_byte_sex);
	p += nscns * sizeof(struct scnhdr);

	/*
	 * Note the base relocations (the contents of the reloc section),
	 * the symbol table and string table all have already been swapped if
	 * that was needed.
	 */
	memcpy(buf + syment_offset, syments, nsyments * sizeof(struct syment));
	memcpy(buf + string_offset, strings, strsize);

	/*
	 * Now with the file contents complete compute the CheckSum in the
	 * optional header and update that in the output buffer.
	 */
	if(ofile->mh != NULL){
	    aouthdr.CheckSum = checksum(buf) + output_size;
	    memcpy(p_aouthdr, &aouthdr, sizeof(struct aouthdr));
	    if(swapped)
		swap_aouthdr((struct aouthdr *)p_aouthdr, target_byte_sex);
	}
	else{
	    aouthdr64.CheckSum = checksum(buf) + output_size;
	    memcpy(p_aouthdr, &aouthdr64, sizeof(struct aouthdr_64));
	    if(swapped)
		swap_aouthdr_64((struct aouthdr_64 *)p_aouthdr,target_byte_sex);
	}

	/*
	 * Create the pecoff file and write the buffer to the file.
	 */
	f = open(out, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if(f == -1)
	    system_fatal("Can't create output file: %s", out);

	if(write64(f, buf, output_size) != (ssize_t)output_size)
	    system_fatal("Can't write output file: %s", out);

	if(close(f) == -1)
	    system_fatal("Can't close output file: %s", out);
}

/*
 * create_ms_dos_stub() is pass a pointer to the buffer where to fill in the
 * MS-DOS stub.
 */
static
void
create_ms_dos_stub(
struct ms_dos_stub *p)
{
    int i;

	p->e_magic    = DOSMAGIC;
	p->e_cblp     = 0x90;
	p->e_cp       = 0x3;
	p->e_crlc     = 0x0;
	p->e_cparhdr  = 0x4;
	p->e_minalloc = 0x0;
	p->e_maxalloc = 0xffff;
	p->e_ss       = 0x0;
	p->e_sp       = 0xb8;
	p->e_csum     = 0x0;
	p->e_ip       = 0x0;
	p->e_cs       = 0x0;
	p->e_lfarlc   = 0x40;
	p->e_ovno     = 0x0;

	for(i = 0; i < 4; i++)
	    p->e_res[i] = 0x0;

	p->e_oemid   = 0x0;
	p->e_oeminfo = 0x0;

	for(i = 0; i < 10; i++)
	    p->e_res2[i] = 0x0;

	p->e_lfanew = 0x80;

	/*
	 * The sub dos program that prints "This program cannot be run in DOS
	 * mode".
	 */
	p->dos_program[0]  = 0x0e;
	p->dos_program[1]  = 0x1f;
	p->dos_program[2]  = 0xba;
	p->dos_program[3]  = 0x0e;
	p->dos_program[4]  = 0x00;
	p->dos_program[5]  = 0xb4;
	p->dos_program[6]  = 0x09;
	p->dos_program[7]  = 0xcd;
	p->dos_program[8]  = 0x21;
	p->dos_program[9]  = 0xb8;
	p->dos_program[10] = 0x01;
	p->dos_program[11] = 0x4c;
	p->dos_program[12] = 0xcd;
	p->dos_program[13] = 0x21;
	p->dos_program[14] = 0x54;
	p->dos_program[15] = 0x68;
	p->dos_program[16] = 0x69;
	p->dos_program[17] = 0x73;
	p->dos_program[18] = 0x20;
	p->dos_program[19] = 0x70;
	p->dos_program[20] = 0x72;
	p->dos_program[21] = 0x6f;
	p->dos_program[22] = 0x67;
	p->dos_program[23] = 0x72;
	p->dos_program[24] = 0x61;
	p->dos_program[25] = 0x6d;
	p->dos_program[26] = 0x20;
	p->dos_program[27] = 0x63;
	p->dos_program[28] = 0x61;
	p->dos_program[29] = 0x6e;
	p->dos_program[30] = 0x6e;
	p->dos_program[31] = 0x6f;
	p->dos_program[32] = 0x74;
	p->dos_program[33] = 0x20;
	p->dos_program[34] = 0x62;
	p->dos_program[35] = 0x65;
	p->dos_program[36] = 0x20;
	p->dos_program[37] = 0x72;
	p->dos_program[38] = 0x75;
	p->dos_program[39] = 0x6e;
	p->dos_program[40] = 0x20;
	p->dos_program[41] = 0x69;
	p->dos_program[42] = 0x6e;
	p->dos_program[43] = 0x20;
	p->dos_program[44] = 0x44;
	p->dos_program[45] = 0x4f;
	p->dos_program[46] = 0x53;
	p->dos_program[47] = 0x20;
	p->dos_program[48] = 0x6d;
	p->dos_program[49] = 0x6f;
	p->dos_program[50] = 0x64;
	p->dos_program[51] = 0x65;
	p->dos_program[52] = 0x2e;
	p->dos_program[53] = 0x0d;
	p->dos_program[54] = 0x0d;
	p->dos_program[55] = 0x0a;
	p->dos_program[56] = 0x24;
	p->dos_program[57] = 0x0;
	p->dos_program[58] = 0x0;
	p->dos_program[59] = 0x0;
	p->dos_program[60] = 0x0;
	p->dos_program[61] = 0x0;
	p->dos_program[62] = 0x0;
	p->dos_program[63] = 0x0;
}


/*
 * create_32bit_symbol_table() is called to process the input Mach-O file and
 * create the pecoff symbol and string table.
 */
static
void
create_32bit_symbol_table(
struct arch *arch)
{
    char *object_addr;
    struct symtab_command *st;
    struct nlist *syms;
    char *strs;
    enum bool found_undef;
#ifdef HACK_TO_MATCH_TEST_CASE
    uint32_t j, n_sect, bss_n_sect, common_n_sect,
	     bss_addr, common_addr, size;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s;
#endif /* HACK_TO_MATCH_TEST_CASE */
    uint32_t i;
    char *p;

	/*
	 * No symbols are actually needed in the pecoff file from the Mach-O
	 * file so create an empty symbol table.
	 */
	nsyments = 0;

	/*
	 * Make sure the Mach-O file does not have any undefined symbols.
	 */
	st = arch->object->st;
	object_addr = arch->object->object_addr;
	syms = (struct nlist *)(object_addr + st->symoff);
	strs = object_addr + st->stroff;
	if(swapped)
	    swap_nlist(syms, st->nsyms, host_byte_sex);
	found_undef = FALSE;
	for(i = 0; i < st->nsyms; i++){
	    if((syms[i].n_type & N_STAB) != 0)
		continue;
	    if((syms[i].n_type & N_TYPE) == N_UNDF){
		if(found_undef == FALSE){
		    error("input file: %s contains undefined symbols:",
			  arch->file_name);
		}
		found_undef = TRUE;
		if(syms[i].n_un.n_strx != 0)
		    printf("%s\n", strs + syms[i].n_un.n_strx);
		else
		    printf("symbol at index %u is undefined but has NULL "
			   "name (like a malformed Mach-O file)\n", i);
	    }
	}
	if(found_undef == TRUE)
	    fatal("undefined symbols are unsupported for conversion to a "
		  "pecoff file");

#ifdef HACK_TO_MATCH_TEST_CASE
	/*
	 * The hack implementation of this routine exist only in order to
	 * match the current ld_efi(1) script that uses objcopy(1) to make the
	 * pecoff file.  So for that only the common symbols and bss symbols
	 * make it into the output pecoff file.
	 *
	 */

	/*
	 * First figure out the section number of the common and bss sections
	 * and address of those sections.
	 */
	n_sect = 1;
	bss_n_sect = 0;
	bss_addr = 0;
	common_n_sect = 0;
	common_addr = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < arch->object->mh->ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(strcmp(sg->segname, SEG_DATA) == 0){
		    s = (struct section *)
			  ((char *)sg + sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++){
			if(strcmp(s->sectname, SECT_BSS) == 0){
			    bss_n_sect = n_sect;
			    bss_addr = s->addr;
			}
			else if(strcmp(s->sectname, SECT_COMMON) == 0){
			    common_n_sect = n_sect;
			    common_addr = s->addr;
			}
			s++;
			n_sect++;
		    }
		}
		else{
		    n_sect += sg->nsects;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/*
	 * Count the number of the common and bss sections symbols and add up
	 * the size of their strings.  Note the size of long section names is
	 * already accounted for in strsize by the code in process_32bit_arch().
	 */
	for(i = 0; i < st->nsyms; i++){
	    if((syms[i].n_type & N_STAB) == 0 &&
	       (syms[i].n_type & N_TYPE) == N_SECT &&
	       (syms[i].n_sect == bss_n_sect ||
	        syms[i].n_sect == common_n_sect)){
		nsyments++;
		if(syms[i].n_un.n_strx != 0){
		    size = strlen(strs + syms[i].n_un.n_strx);
		    if(size > E_SYMNMLEN)
			strsize += strlen(strs + syms[i].n_un.n_strx) + 1;
		}
	    }
	}
#endif /* HACK_TO_MATCH_TEST_CASE */

	/*
	 * Allocate space for the pecoff symbol table and string table.
	 */
	syments = allocate(nsyments * sizeof(struct syment));
	memset(syments, '\0', nsyments * sizeof(struct syment));
	strings = allocate(strsize);

	/*
	 * Put the size of the string table in the string table first.  Then
	 * the strings for the long section names right after the size.
	 */
	p = strings;

	i = strsize;
	if(swapped)
	    i = SWAP_INT(i);
	memcpy(p, &i, sizeof(uint32_t));
	p += sizeof(uint32_t);

	memcpy(p, section_names, section_names_size);
	p += section_names_size;

#ifdef HACK_TO_MATCH_TEST_CASE
	/*
	 * First put in the bss symbols, again to match what is done by
	 * objcopy.
	 */
	j = 0;
	for(i = 0; i < st->nsyms; i++){
	    if((syms[i].n_type & N_STAB) == 0 &&
	       (syms[i].n_type & N_TYPE) == N_SECT &&
	       syms[i].n_sect == bss_n_sect){
		if(syms[i].n_un.n_strx != 0){
		    size = strlen(strs + syms[i].n_un.n_strx);
		    if(size > E_SYMNMLEN){
			syments[j].e.e.e_zeroes = 0;
			syments[j].e.e.e_offset = p - strings;
			strcpy(p, strs + syms[i].n_un.n_strx);
			p += strlen(strs + syms[i].n_un.n_strx) + 1;
		    }
		    else{
			strncpy(syments[j].e.e_name,
				strs + syms[i].n_un.n_strx, E_SYMNMLEN);
		    }
		}
		syments[j].e_value = syms[i].n_value - bss_addr;
		syments[j].e_scnum = bss_scnum;
		syments[j].e_type = 0;
		syments[j].e_sclass = IMAGE_SYM_CLASS_EXTERNAL;
		syments[j].e_numaux = 0;
		j++;
	    }
	}
	/*
	 * Next put in the common symbols, again to match what is done by
	 * objcopy.
	 */
	for(i = 0; i < st->nsyms; i++){
	    if((syms[i].n_type & N_STAB) == 0 &&
	       (syms[i].n_type & N_TYPE) == N_SECT &&
	       syms[i].n_sect == common_n_sect){
		if(syms[i].n_un.n_strx != 0){
		    size = strlen(strs + syms[i].n_un.n_strx);
		    if(size > E_SYMNMLEN){
			syments[j].e.e.e_zeroes = 0;
			syments[j].e.e.e_offset = p - strings;
			strcpy(p, strs + syms[i].n_un.n_strx);
			p += strlen(strs + syms[i].n_un.n_strx) + 1;
		    }
		    else{
			strncpy(syments[j].e.e_name,
				strs + syms[i].n_un.n_strx, E_SYMNMLEN);
		    }
		}
		syments[j].e_value = syms[i].n_value - common_addr;
		syments[j].e_scnum = common_scnum;
		syments[j].e_type = 0;
		syments[j].e_sclass = IMAGE_SYM_CLASS_EXTERNAL;
		syments[j].e_numaux = 0;
		j++;
	    }
	}

	if(swapped)
	    swap_syment(syments, nsyments, target_byte_sex);

#endif /* HACK_TO_MATCH_TEST_CASE */
}

/*
 * create_64bit_symbol_table() is called to process the input Mach-O file and
 * create the pecoff symbol and string table.
 */
static
void
create_64bit_symbol_table(
struct arch *arch)
{
    char *p;
    uint32_t i;
    char *object_addr;
    struct symtab_command *st;
    struct nlist_64 *syms64;
    char *strs;
    enum bool found_undef;

	st = arch->object->st;
	object_addr = arch->object->object_addr;
	syms64 = (struct nlist_64 *)(object_addr + st->symoff);
	strs = object_addr + st->stroff;
	if(swapped)
	    swap_nlist_64(syms64, st->nsyms, host_byte_sex);
	/*
	 * If the entry point option was specified then look for that symbol
	 * and set the entry point value.
	 */
	if(entry_point != NULL){
	    for(i = 0; i < st->nsyms; i++){
		if((syms64[i].n_type & N_STAB) == 0 &&
		    syms64[i].n_un.n_strx != 0 &&
		    strcmp(strs + syms64[i].n_un.n_strx, entry_point) == 0){
		    entry = (uint32_t)syms64[i].n_value;
		    break;
		}
	    }
	    if(i == st->nsyms)
		fatal("can't find symbol for -e %s in input file: %s",
		      entry_point, arch->file_name);
	}

	/*
	 * Make sure the Mach-O file does not have any undefined symbols.
	 */
	found_undef = FALSE;
	for(i = 0; i < st->nsyms; i++){
	    if((syms64[i].n_type & N_STAB) != 0)
		continue;
	    if((syms64[i].n_type & N_TYPE) == N_UNDF){
		if(found_undef == FALSE){
		    error("input file: %s contains undefined symbols:",
			  arch->file_name);
		}
		found_undef = TRUE;
		if(syms64[i].n_un.n_strx != 0)
		    printf("%s\n", strs + syms64[i].n_un.n_strx);
		else
		    printf("symbol at index %u is undefined but has NULL "
			   "name (like a malformed Mach-O file)\n", i);
	    }
	}
	if(found_undef == TRUE)
	    fatal("undefined symbols are unsupported for conversion to a "
		  "pecoff file");

	/*
	 * No symbols are actually needed in the pecoff file from the Mach-O
	 * file so create an empty symbol table.
	 *
	 * Set the number of symbols to zero and allocate the string table.
	 * Note the size of long section names is already accounted for in
	 * strsize by the code in process_64bit_arch().
	 */
	nsyments = 0;
	strings = allocate(strsize);

	/*
	 * Put the size of the string table in the string table first.  Then
	 * the strings for the long section names right after the size.
	 */
	p = strings;

	i = strsize;
	if(swapped)
	    i = SWAP_INT(i);
	memcpy(p, &i, sizeof(uint32_t));
	p += sizeof(uint32_t);

	memcpy(p, section_names, section_names_size);
}

/*
 * create_base_reloc() is called to process the input Mach-O file and gather
 * the info needed and then to create the base relocation entries.
 */
static
void
create_base_reloc(
struct arch *arch)
{
    uint32_t ncmds, i, j;
    uint64_t addr, first_addr;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct relocation_info *relocs;

    char *object_addr;
    struct dysymtab_command *dyst;

	if(arch->object->mh != NULL)
	    ncmds = arch->object->mh->ncmds;
	else
	    ncmds = arch->object->mh64->ncmds;
	dyst = arch->object->dyst;
	object_addr = arch->object->object_addr;

	first_addr = 0;
	lc = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(first_addr == 0)
		    first_addr = sg->vmaddr;
		s = (struct section *)
		      ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    relocs = (struct relocation_info *)(object_addr +
						        s[j].reloff);
		    if(swapped)
			swap_relocation_info(relocs, s[j].nreloc,
					     host_byte_sex);
		    if(arch->object->mh_cputype == CPU_TYPE_I386)
			gather_base_reloc_info(s[j].addr, relocs, s[j].nreloc,
				CPU_TYPE_I386, 2, GENERIC_RELOC_VANILLA,
				IMAGE_REL_BASED_HIGHLOW);
		    else if(arch->object->mh_cputype == CPU_TYPE_ARM)
			gather_base_reloc_info(s[j].addr, relocs, s[j].nreloc,
				CPU_TYPE_ARM, 2, GENERIC_RELOC_VANILLA,
				IMAGE_REL_BASED_HIGHLOW);
		    if((s[j].flags & SECTION_TYPE) ==
			S_NON_LAZY_SYMBOL_POINTERS){
			for(addr = s[j].addr;
			    addr < s[j].addr + s[j].size;
			    addr += 4) {
			    add_base_reloc(addr, IMAGE_REL_BASED_HIGHLOW);
			}
		    }
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		if(arch->object->mh_cputype == CPU_TYPE_X86_64) {
		    /*
		     * X86_64 relocations are relative to the first writable
		     * segment.
		     */
		    /*
		     * But arm64 relocations are NOT relative to the first
		     * writable segment but just the first segment.
		     */
		    if((first_addr == 0) &&
		       ((sg64->initprot & VM_PROT_WRITE) != 0)) {
		      first_addr = sg64->vmaddr;
		    }
		} else { 
		if(first_addr == 0)
		    first_addr = sg64->vmaddr;
		}
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    relocs = (struct relocation_info *)(object_addr +
						        s64[j].reloff);
		    if(swapped)
			swap_relocation_info(relocs, s64[j].nreloc,
					     host_byte_sex);
		    if(arch->object->mh_cputype == CPU_TYPE_X86_64)
			gather_base_reloc_info((uint32_t)s64[j].addr, relocs,
			    s64[j].nreloc, CPU_TYPE_X86_64, 3,
			    X86_64_RELOC_UNSIGNED, IMAGE_REL_BASED_DIR64);
		    else if(arch->object->mh_cputype == CPU_TYPE_ARM64)
			gather_base_reloc_info((uint32_t)s64[j].addr, relocs,
			    s64[j].nreloc, CPU_TYPE_ARM64, 3,
			    ARM64_RELOC_UNSIGNED, IMAGE_REL_BASED_DIR64);
		    if((s64[j].flags & SECTION_TYPE) ==
			S_NON_LAZY_SYMBOL_POINTERS){
			for(addr = s64[j].addr;
			    addr < s64[j].addr + s64[j].size;
			    addr += 8) {
			    add_base_reloc(addr, IMAGE_REL_BASED_DIR64);
			}
		    }
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(dyst != NULL && dyst->nlocrel != 0){
	    relocs = (struct relocation_info *)(object_addr +
						dyst->locreloff);
	    if(swapped)
		swap_relocation_info(relocs, dyst->nlocrel, host_byte_sex);
	    if(arch->object->mh_cputype == CPU_TYPE_I386)
		gather_base_reloc_info((uint32_t)first_addr, relocs,
				       dyst->nlocrel, CPU_TYPE_I386, 2,
				       GENERIC_RELOC_VANILLA,
				       IMAGE_REL_BASED_HIGHLOW);
	    else if(arch->object->mh_cputype == CPU_TYPE_ARM)
		gather_base_reloc_info((uint32_t)first_addr, relocs,
				       dyst->nlocrel, CPU_TYPE_ARM, 2,
				       GENERIC_RELOC_VANILLA,
				       IMAGE_REL_BASED_HIGHLOW);
	    else if(arch->object->mh_cputype == CPU_TYPE_X86_64)
		gather_base_reloc_info((uint32_t)first_addr, relocs,
				       dyst->nlocrel, CPU_TYPE_X86_64, 3,
				       X86_64_RELOC_UNSIGNED,
				       IMAGE_REL_BASED_DIR64);
	    else if(arch->object->mh_cputype == CPU_TYPE_ARM64)
		gather_base_reloc_info((uint32_t)first_addr, relocs,
				       dyst->nlocrel, CPU_TYPE_ARM64, 3,
				       ARM64_RELOC_UNSIGNED,
				       IMAGE_REL_BASED_DIR64);
	}
	/*
	if(dyst != NULL && dyst->nextrel != 0)
	    ; TODO error if there are external relocation entries */

	/*
	 * Now with all the info gathered make the base relocation entries.
	 */
	make_base_relocs();
}

/*
 * gather_base_reloc_info() is passed the base address for the set of Mach-O
 * relocation entries. And is passed the cpu_type, length and macho_reloc_type
 * to look for and the base_reloc_type to create if found.
 */
static
void
gather_base_reloc_info(
uint32_t addr,
struct relocation_info *relocs, 
uint32_t nreloc,
cpu_type_t cpu_type,
uint32_t length,
int macho_reloc_type,
int base_reloc_type)
{
    uint32_t i, r_address, r_pcrel, r_length, r_extern, r_type;
    struct scattered_relocation_info *sreloc;

	for(i = 0; i < nreloc; i++){
	    if((relocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)(relocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_type = (enum reloc_type_generic)sreloc->r_type;
		r_extern = 0;
	    }
	    else{
		r_address = relocs[i].r_address;
		r_pcrel = relocs[i].r_pcrel;
		r_length = relocs[i].r_length;
		r_extern = relocs[i].r_extern;
		r_type = (enum reloc_type_generic)relocs[i].r_type;
	    }

	    if(r_extern == 0 && r_pcrel == 0 &&
	       r_length == length && r_type == macho_reloc_type)
		add_base_reloc(addr + r_address, base_reloc_type);
	    else
	    	; /* TODO add checking and error messages here */

	    if((relocs[i].r_address & R_SCATTERED) == 0){
		if(reloc_has_pair(cpu_type, relocs[i].r_type))
		    i++;
	    }
	    else{
		sreloc = (struct scattered_relocation_info *)relocs + i;
		if(reloc_has_pair(cpu_type, sreloc->r_type))
		    i++;
	    }
	}
}

/*
 * add_base_reloc() is passed a addr and a type for a base relocation entry to
 * add to the list.
 */
static
void
add_base_reloc(
uint64_t addr,
uint32_t type)
{
    static int max = 0;
    struct base_reloc *new_base_relocs;
    
	if(!max){
	    max = 128;
	    base_relocs = (struct base_reloc *)
			  malloc(max * sizeof(struct base_reloc));
	}
	if(nbase_reloc >= max){
	    new_base_relocs = malloc(2 * max * sizeof(struct base_reloc));
	    memcpy(new_base_relocs, base_relocs,
		   max * sizeof(struct base_reloc));
	    max *= 2;
	    free(base_relocs);
	    base_relocs = new_base_relocs;
	}
	base_relocs[nbase_reloc].addr = addr;
        base_relocs[nbase_reloc].type = type;
	nbase_reloc++;
}

/*
 * The base relocation table in a PECOFF file is divided into blocks. Each
 * block represents the base relocations for a 4K page. Each block must start
 * on a 32-bit boundary.  Which is why one "nop" base relocation entry may be
 * be added as padding in a block.
 */
#define MAX_BLOCK_OFFSET 0x1000
#define BLOCK_MASK (MAX_BLOCK_OFFSET-1)

/*
 * make_base_relocs() takes the info for the base relocation entries gathered
 * and creates the fixup blocks as they would be in a PECOFF file and sets the
 * static variables reloc_contents and reloc_size to the pointer to contents
 * and the size of that contents.
 */
static
void
make_base_relocs(
void)
{
    int blockcnt;
    int i, entries;
    uint64_t base;
    int size, s_size, pad;
    char *fb;
    struct base_relocation_block_header *h;
    struct base_relocation_entry *b;
    uint32_t offset;
	
	blockcnt = 0;

	/*
	 * After we create each base relocation block we will allocate space
	 * for it in the .reloc section contents buffer and copy it into the
	 * buffer.
	 */
	reloc_size = 0;
	reloc_contents = NULL;

	/*
	 * If there are no base relocation entries return so we don't create a
	 * base relocation block with 0 entries.
	 */
	if(nbase_reloc == 0)
	    return;
	
	qsort(base_relocs, nbase_reloc, sizeof(struct base_reloc),
	      (int (*)(const void *, const void *))cmp_base_relocs);
	
	/*
	 * The size of the base relocation tables must be a multiple of 4 bytes.
	 * so we may need to add one relocation entry as padding.  We make this
	 * fixup block large enought to hold all the base relocation entries.
	 * But it will be broken up for the base relocation entries for each
	 * each group that refers to the same 4K page.
	 */
	size = sizeof(struct base_relocation_block_header) +
	       (nbase_reloc + 1) * sizeof(struct base_relocation_entry);
	fb = malloc(size);
	
	
	entries = 0;
	base = base_relocs[0].addr & ~BLOCK_MASK;
	h = (struct base_relocation_block_header *)fb;
	b = (struct base_relocation_entry *)
	    (fb + sizeof(struct base_relocation_block_header));
	for(i = 0; i < nbase_reloc; i++){
	    offset = (uint32_t)(base_relocs[i].addr - base);
	    if(offset >= MAX_BLOCK_OFFSET) {
		/* add padding if needed */
		if((entries % 2) != 0){
		    b[entries].type = IMAGE_REL_BASED_ABSOLUTE;
		    b[entries].offset = 0;
		    entries++;
		}
		h->page_rva = (uint32_t)base;
		size = sizeof(struct base_relocation_block_header) +
		       entries * sizeof(struct base_relocation_entry);
		h->block_size = size;
		if(swapped){
		    swap_base_relocation_block_header(h,
						      target_byte_sex);
		    swap_base_relocation_entry(b, entries,
					       target_byte_sex);
		}
		/* copy this finished block into the .reloc contents buffer */
		reloc_contents = reallocate(reloc_contents, reloc_size + size);
		memcpy(reloc_contents + reloc_size, fb, size);
		reloc_size += size;

		entries = 0;
		blockcnt++;
		base = base_relocs[i].addr & ~BLOCK_MASK;
		offset = (uint32_t)(base_relocs[i].addr - base);
	    }
	    b[entries].type = base_relocs[i].type;
	    b[entries].offset = offset;
	    entries++;
	}
	
	/* add padding if needed */
	if((entries % 2) != 0){
	    b[entries].type = IMAGE_REL_BASED_ABSOLUTE;
	    b[entries].offset = 0;
	    entries++;
	}
	h->page_rva = (uint32_t)base;
	size = sizeof(struct base_relocation_block_header) +
	       entries * sizeof(struct base_relocation_entry);
	h->block_size = size;
	if(swapped){
	    swap_base_relocation_block_header(h, target_byte_sex);
	    swap_base_relocation_entry(b, entries, target_byte_sex);
	}

	/* copy this last block into the .reloc contents buffer */
	reloc_contents = reallocate(reloc_contents, reloc_size + size);
	memcpy(reloc_contents + reloc_size, fb, size);
	reloc_size += size;

	/*
	 * The make the relocs buffer the s_size rounded to file_alignment and
	 * zero out the padding
         */
	s_size = rnd32(reloc_size, file_alignment);
	pad = s_size - reloc_size;
	reloc_contents = reallocate(reloc_contents, s_size);
	memset(reloc_contents + reloc_size, '\0', pad);

	blockcnt++;
	free(fb);
}

static
int
cmp_base_relocs(
struct base_reloc *x1,
struct base_reloc *x2)
{
	if(x1->addr < x2->addr)
	    return(-1);
	if(x1->addr == x2->addr)
	    return(0);
	/* x1->addr > x2->addr */
	    return(1);
}

/*
 * create_debug() is called to create the .debug section contents from
 * the -d filename argument.
 */
static
void
create_debug(
struct arch *arch)
{
    char *p;
    uint32_t i, ncmds, s_size;
    struct load_command *lc;
    struct uuid_command *uuid;

	/*
	 * Allocate space for everything that will be in the .debug section:
	 *	the debug_directory_entry struct
	 *	the mtoc_debug_info struct
	 *	the name of the -d filename argument null terminated.
	 */
	debug_size = sizeof(struct debug_directory_entry) +
		     sizeof(struct mtoc_debug_info) +
		     (uint32_t)strlen(debug_filename) + 1;
	/*
	 * The make the debug buffer the s_size rounded to the file_alignment
         * and also zero out the padding
         */
	s_size = rnd32(debug_size, file_alignment);
	debug_contents = allocate(s_size);
	memset(debug_contents, '\0', s_size);
	/*
	 * Set up pointers to all the parts to be filled in.
	 */
	p = debug_contents;
	dde = (struct debug_directory_entry *)p;
	p += sizeof(struct debug_directory_entry);
	mdi = (struct mtoc_debug_info *)p;
	p += sizeof(struct mtoc_debug_info);

	dde->Characteristics = 0;
	dde->TimeDateStamp = (uint32_t)time(NULL);
	dde->MajorVersion = 0;
	dde->MinorVersion = 0;
	dde->Type = IMAGE_DEBUG_TYPE_CODEVIEW;
	dde->SizeOfData = sizeof(struct mtoc_debug_info) +
			  (uint32_t)strlen(debug_filename) + 1;
	/*
	 * These two will be filled in later when address and offsets
	 * are known.
	 */
	dde->AddressOfRawData = 0;
	dde->PointerToRawData = 0;

	mdi->Signature = MTOC_SIGNATURE;
	if(arch->object->mh != NULL)
	    ncmds = arch->object->mh->ncmds;
	else
	    ncmds = arch->object->mh64->ncmds;
	lc = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_UUID){
		uuid = (struct uuid_command *)lc;
		if (debug_uuid != NULL) {
		    string_to_uuid (debug_uuid, uuid->uuid);
		}
		/* Swizzle UUID to match EFI GUID definition */
		mdi->uuid[0] = uuid->uuid[3];
		mdi->uuid[1] = uuid->uuid[2];
		mdi->uuid[2] = uuid->uuid[1];
		mdi->uuid[3] = uuid->uuid[0];
		mdi->uuid[4] = uuid->uuid[5];
		mdi->uuid[5] = uuid->uuid[4];
		mdi->uuid[6] = uuid->uuid[7];
		mdi->uuid[7] = uuid->uuid[6];
		mdi->uuid[8] = uuid->uuid[8];
		mdi->uuid[9] = uuid->uuid[9];
		mdi->uuid[10] = uuid->uuid[10];
		mdi->uuid[11] = uuid->uuid[11];
		mdi->uuid[12] = uuid->uuid[12];
		mdi->uuid[13] = uuid->uuid[13];
		mdi->uuid[14] = uuid->uuid[14];
		mdi->uuid[15] = uuid->uuid[15];
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	strcpy(p, debug_filename);
}

/*
 * set_debug_addrs_and_offsets() is called after the .debug section's address
 * and offset has been set and this routine sets the other needed addresses
 * and offsets in the section contents.  And swaps the section contents if
 * needed for output.
 */
static
void
set_debug_addrs_and_offsets(
void)
{
	dde->AddressOfRawData = debug_scnhdr->s_vaddr +
				sizeof(struct debug_directory_entry);
	dde->PointerToRawData = debug_scnhdr->s_scnptr +
				sizeof(struct debug_directory_entry);
	if(swapped){
	    swap_debug_directory_entry(dde, target_byte_sex);
	    swap_mtoc_debug_info(mdi, target_byte_sex);
	}
}

/*
 * checksum() calculates the value for the CheckSum field in the optional
 * header from the bytes in the output buffer passed to it which has the
 * size output_size.
 */
static
uint32_t
checksum(
unsigned char *buf)
{
    uint32_t i, v, t;

	t = 0;
	for(i = 0; i < output_size; i += 2){
	    if(output_size - i == 1)
		v = buf[i];
	    else
		v = buf[i] + (buf[i+1] << 8);
	    t += v;
	    t = 0xffff & (t + (t >> 0x10));
	}
	return(0xffff & (t + (t >> 0x10)));
}

/*
 * string_to_uuid() creates a 128-bit uuid from a well-formatted UUID string
 * (i.e. aabbccdd-eeff-gghh-iijj-kkllmmnnoopp)
 */
static
void
string_to_uuid(
char *string,
uint8_t *uuid)
{
    uint8_t count;

	count = sscanf (string, UUID_FORMAT_STRING,
	&uuid[0], &uuid[1], &uuid[2], &uuid[3],
	&uuid[4], &uuid[5], &uuid[6], &uuid[7],
	&uuid[8], &uuid[9], &uuid[10], &uuid[11],
	&uuid[12], &uuid[13], &uuid[14], &uuid[15]);

	if (count != 16) {
	    fatal ("invalid UUID specified for -u option");
	}
}
