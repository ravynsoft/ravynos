/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef LIBRARY_API
/*
 * The redo_prebinding(1) program.  This redoes the prebinding of an executable
 * or dynamic library.
 *
 * redo_prebinding [-c|-p|-d] [-i] [-z] [-u] [-r rootdir] [-e executable_path]
 *		   [-seg_addr_table table_file_name]
 *		   [-seg_addr_table_filename pathname ]
 *		   [-seg1addr address]
 *		   [-o output_file] [-s] input_file
 *	-c check only and return status
 *	-p check only for prebound files and return status
 *	-d check only for dylibs and return status
 *	-i ignore non-prebound files
 *	-z zero out the prebind check sum
 *	-r prepend the next argument to dependent libraries
 *	-e replace "@executable_path" in dependent libraries with the next
 *	   argument
 *	-o write the output to the next argument instead of the input_file
 *	-s write the output to standard output
 *	-u unprebind, rather than reprebind (-c, -p, -d, -e ignored)
 *	-seg_addr_table the next argument is the file name of the table
 *	-seg_addr_table_filename the next argument is the pathname to use
 *				 instead of the install name
 * 	-seg1addr the next argument is a hex address at which to place the
 *		  input_file dylib
 * With no -c, -p or -d it exits 0 if sucessful and 2 means it could not be
 * done for reasons like a dependent library is missing.  An exit of 3 is for
 * the specific case when the dependent libraries are out of date with respect
 * to each other.
 * 
 * If -c, check only, is specified a 0 exit means the file's prebinding is
 * up to date, 1 means it needs to be redone and 2 means it could not be checked
 * for reasons like a dependent library is missing.
 *
 * If -p, check only for prebound files, is specified 1 exit means the file is
 * a Mach-O that could be prebound and is not otherwise the exit is 0.
 *
 * If -d, check only for dylib files, is specified a 0 exit means the file is a
 * dylib, 1 means the file is not a dylib and 2 means there is some mix in
 * the architectures.
 *
 * The option -seg_addr_table is used when the input a dynamic library and if
 * specified the table entry for the install_name (or -seg_addr_table_filename
 * pathname agrument) of the dynamic library is used for checking and the
 * address to slide the library to.
 *
 * The -seg1addr option is followed by a valid hexadecimal address at which to
 * place the input dynamic library.
 *
 * If -u, prebinding-specific information is removed from the binary or reset.
 * The -c, -p, -d, and -e arguments are ignored (as are -r, -seg_addr_table,
 * -seg_addr_table_filename, and -seg1addr which are irrelevant to unprebinding).
 * dylibs are slid to zero.
 */
#else /* defined(LIBRARY_API) */
/*
 * The library API for redo_prebinding is defined in <mach-o/redo_prebinding.h>
 * and below in the comments before each routine.
 */
#include <mach-o/redo_prebinding.h>
#endif /* defined(LIBRARY_API) */

#ifdef __linux__
#define __alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
#include <Availability.h>
#endif

#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <strings.h> /* cctools-port: For bcmp, bzero ... */
#import <limits.h>
#import <libc.h>
#import <malloc/malloc.h>
#import <sys/types.h>
#import <sys/stat.h>
#import <mach-o/stab.h>
#import <mach-o/loader.h>
#import <mach-o/reloc.h>
#import <mach-o/hppa/reloc.h>
#import <mach-o/sparc/reloc.h>
#import <mach-o/ppc/reloc.h>
#import <mach-o/arm/reloc.h>
#import <stuff/breakout.h>
#import <stuff/best_arch.h>
#import <stuff/allocate.h>
#import <stuff/errors.h>
#import <stuff/rnd.h>
#import <stuff/hppa.h>
#import <stuff/execute.h>
#import <stuff/guess_short_name.h>
#import <stuff/seg_addr_table.h> /* cctools-port: uncommented */
#import <stuff/macosx_deployment_target.h>

#include <mach-o/dyld.h>

#define U_ABS(l) (((int32_t)(l))<0 ? (uint32_t)(-(l)) : (l))

/* name of the program for error messages (argv[0]) */
__private_extern__ char *progname = NULL;

/* -c option, only check and return status */
static enum bool check_only = FALSE;
static enum bool seen_a_non_64_bit = FALSE;

/* -i option, ignore non-prebound files */
static enum bool ignore_non_prebound = FALSE;

/* -z option, zero out prebind checksum */
static enum bool zero_out_prebind_checksum = FALSE;

/* -p option, check for non-prebound files */
static enum bool check_for_non_prebound = FALSE;

/* -d option, check for dynamic library files */
static enum bool check_for_dylibs = FALSE;
static enum bool seen_a_dylib = FALSE;
static enum bool seen_a_non_dylib = FALSE;

/* -r option's argument, root directory to prepend to dependent libraries */
static char *root_dir = NULL;

/*
 * -e option's argument, executable_path is used to replace "@executable_path
 * for dependent libraries.
 */
static char *executable_path = NULL;

#ifndef LIBRARY_API
/*
 * -seg_addr_table option's argument, the file name of the segment address
 * table.  And the parsed seg_addr_table and its size.
 */
char *seg_addr_table_name = NULL;
struct seg_addr_table *seg_addr_table = NULL;
uint32_t table_size = 0;
/*
 * -seg_addr_table_filename option's argument, the pathame to use instead of the
 * install name.
 */
char *seg_addr_table_filename = NULL;
#endif /* !defined(LIBRARY_API) */

/* the address the input dylib is to have or be moved to if not zero */
static uint32_t new_dylib_address = 0;
/* the address the input dylib started out at */
static uint32_t old_dylib_address = 0;
/*
 * The amount to add to the old dylib address to get it to the new dylib
 * address. This will remain at zero if the address is not specified to be
 * changed.
 */
static uint32_t dylib_vmslide = 0;

/* -debug turn on debugging printf()'s */
static enum bool debug = FALSE;

/* -u enables the 'unprebind' operation, as opposed to redoing prebinding */
static enum bool unprebinding = FALSE;

/*
 * If some architecture was processed then the output file needs to be built
 * otherwise no output file is written.
 */
static enum bool arch_processed = FALSE;

/* the link state of each module */
enum link_state {
    UNLINKED,
    LINKED
};

/*
 * These are set to the current arch's symbolic info.
 */
static struct arch *arch = NULL;
static struct arch_flag arch_flag = { 0 };
static enum bool arch_swapped = FALSE;
static char *arch_name = NULL;
static struct nlist *arch_symbols = NULL;
static struct nlist_64 *arch_symbols64 = NULL;
static uint32_t arch_nsyms = 0;
static char *arch_strings = NULL;
static uint32_t arch_strsize = 0;
static struct dylib_table_of_contents *arch_tocs = NULL;
static uint32_t arch_ntoc = 0;
static struct dylib_module *arch_mods = NULL;
static struct dylib_module_64 *arch_mods64 = NULL;
static uint32_t arch_nmodtab = 0;
static struct dylib_reference *arch_refs = NULL;
static uint32_t arch_nextrefsyms = 0;
static struct twolevel_hint *arch_hints = NULL;
static uint32_t arch_nhints = 0;
static enum link_state arch_state = LINKED;

static uint32_t arch_seg1addr = 0;
static uint32_t arch_segs_read_write_addr = 0;
static enum bool arch_split_segs = FALSE;
static struct relocation_info *arch_extrelocs = NULL;
static struct relocation_info *arch_locrelocs = NULL;
static uint32_t arch_nextrel = 0;
static uint32_t arch_nlocrel = 0;
static uint32_t *arch_indirect_symtab = NULL;
static uint32_t arch_nindirectsyms = 0;

static enum bool arch_force_flat_namespace = FALSE;

static cpu_type_t arch_cant_be_missing = 0;

/*
 * These hold the dependent libraries for the arch currently being processed.
 * Their link edit information is used to update the arch currently being
 * processed.
 */
struct lib {
    char *dylib_name;
    char *file_name;
    struct ofile *ofile;
    dev_t dev;
    ino_t ino;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct routines_command *rc;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
    uint32_t nsyms;
    char *strings;
    uint32_t strsize;
    struct dylib_table_of_contents *tocs;
    uint32_t ntoc;
    struct dylib_module *mods;
    struct dylib_module_64 *mods64;
    uint32_t nmodtab;
    struct dylib_reference *refs;
    uint32_t nextrefsyms;
    enum link_state *module_states;
    enum bool LC_PREBOUND_DYLIB_found;
    uint32_t LC_PREBOUND_DYLIB_size;
    /*
     * For two-level namespace images this is the array of pointers to the
     * dependent images (indexes into the libs[] array) and the count of them.
     */
    uint32_t *dependent_images;
    uint32_t ndependent_images;
    /*
     * If this is a library image which has a framework name or library name
     * then this is the part that would be the umbrella name or library name
     * and the size of the name.  This points into the name and since framework
     * and library names may have suffixes the size is needed to exclude it.
     * This is only needed for two-level namespace images.  umbrella_name and
     * or library_name will be NULL and name_size will be 0 if there is no
     * umbrella name.
     */
    char *umbrella_name;
    char *library_name;
    uint32_t name_size;

    /*
     * array of pointers (indexes into the libs[] array) to sub-frameworks and
     * sub-umbrellas and count
     */
    enum bool sub_images_setup;
    uint32_t *sub_images;
    uint32_t nsub_images;

    enum bool two_level_debug_printed;
};
static struct lib *libs = NULL;
static uint32_t nlibs = 0;

/*
 * A fake lib struct for the arch being processed which is used if the arch
 * being processed is a two-level namespace image.
 */
static struct lib arch_lib;

/*
 * A fake lib struct for the missing weak libraries which is used if for
 * two-level namespace images.
 */
static struct lib weak_lib;
/*
 * The weak symbol used as the value for missing weak symbols.
 */
static struct nlist weak_symbol = {
    { 0 }, 	   /* n_un.strx */
    N_ABS | N_EXT, /* n_type */
    NO_SECT,       /* n_sect */
    0,             /* n_desc */
    0x0,           /* n_value */
};
/*
 * The module used for missing weak symbols.
 */
static enum link_state weak_module = LINKED;

/*
 * This is used by check_for_overlapping_segments() to create a list of segment
 * for overlap checking.
 */
struct segment {
    char *file_name;
    struct segment_command sg;
};

#ifndef LIBRARY_API
static void usage(
    void);
static char * get_install_name(
    struct arch *archs,
    uint32_t narchs);
#endif /* !defined(LIBRARY_API) */

static enum bool has_resource_fork(
    char *filename);

static void process_archs(
    struct arch *archs,
    uint32_t narchs,
    enum bool has_resource_fork);

static uint32_t get_dylib_address(
    void);

static void process_arch(void);

static void unprebind_arch(void);

static enum bool load_archs_libraries(void);

static enum bool load_library(
    char *file_name,
    struct dylib_command *dl_load,
    enum bool time_stamps_must_match,
    uint32_t *image_pointer);

static enum bool load_dependent_libraries(void);

static void print_two_level_info(
    struct lib *lib);

static enum bool setup_sub_images(
    struct lib *lib,
    struct mach_header *lib_mh,
    struct mach_header_64 *lib_mh64);

static void check_for_overlapping_segments(
    uint32_t vmslide);

static void check_overlap(
    struct segment *s1,
    struct segment *s2);

static void setup_symbolic_info(enum bool missing_arch);

static void swap_arch_for_output(void);

static void check_symbolic_info_tables(
    char *file_name,
    struct mach_header *mh,
    struct mach_header_64 *mh64,
    uint32_t nlibrefs,
    struct symtab_command *st,
    struct dysymtab_command *dyst,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsyms,
    char *strings,
    uint32_t strsize,
    struct dylib_table_of_contents *tocs,
    uint32_t ntoc,
    struct dylib_module *mods,
    struct dylib_module_64 *mods64,
    uint32_t nmodtab,
    struct dylib_reference *refs,
    uint32_t nextrefsyms);

static void check_for_dylib_override_symbols(void);

static void check_dylibs_for_definition(
    char *file_name,
    char *symbol_name);

static enum bool check_dylibs_for_reference(
    char *symbol_name);

/* these two variables are used by the bsearch routines */
static char *bsearch_strings = NULL;
static struct nlist *bsearch_symbols = NULL;

static int dylib_bsearch(
    const char *symbol_name,
    const struct dylib_table_of_contents *toc);

static int nlist_bsearch(
    const char *symbol_name,
    const struct nlist *symbol);

static void setup_initial_undefined_list(void);

static void link_in_need_modules(void);

/* fake index into the libs[] array to refer to the arch being processed */
#define ARCH_LIB 0xffffffff
/* fake index into the libs[] array to refer to the a missing weak library */
#define WEAK_LIB 0xfffffffe
/*
 * The structure of an element in a symbol list.
 */
struct symbol_list {
    char *name;			/* name of the symbol */
    /* for two-level references then next two fields are used */
    struct nlist *symbol;	/* the symbol, NULL for flat references */
    uint32_t ilib;		/* the library the symbol is from (index into
				   the libs[] array, or ARCH_LIB) */
    struct symbol_list *prev;	/* previous in the chain */
    struct symbol_list *next;	/* next in the chain */
};
/*
 * The head of the undefined list.  This is a circular list so it can be
 * searched from start to end and so new items can be put on the end.  This
 * structure never has its name filled in but only serves as the head and tail
 * of the list.
 */
static struct symbol_list undefined_list = {
    NULL, NULL, 0, &undefined_list, &undefined_list
};

static void add_to_undefined_list(
    char *name,
    struct nlist *symbol,
    uint32_t ilib);

static void link_library_module(
    enum link_state *module_state,
    struct lib *lib);

struct indr_loop_list {
    struct nlist *symbol;
    struct indr_loop_list *next;
};
#define NO_INDR_LOOP ((struct indr_loop_list *)1)

static struct lib *get_primary_lib(
    uint32_t ilib,
    struct nlist *symbol);

static struct lib *get_indr_lib(
    char *symbol_name,
    struct lib *lib);

static enum bool get_weak(
    struct nlist *symbol);

static void lookup_symbol(
    char *name,
    struct lib *primary_lib,
    enum bool weak,
    struct nlist **symbol,
    enum link_state **module_state,
    struct lib **lib,
    uint32_t *isub_image,
    uint32_t *itoc,
    struct indr_loop_list *indr_loop);

static enum bool lookup_symbol_in_arch(
    char *name,
    struct nlist **symbol,
    enum link_state **module_state,
    struct lib **lib,
    uint32_t *isub_image,
    uint32_t *itoc,
    struct indr_loop_list *indr_loop);

static enum bool lookup_symbol_in_lib(
    char *name,
    struct lib *primary_lib,
    struct nlist **symbol,
    enum link_state **module_state,
    struct lib **lib,
    uint32_t *isub_image,
    uint32_t *itoc,
    struct indr_loop_list *indr_loop);

static void build_new_symbol_table(
    uint32_t vmslide,
    enum bool missing_arch);

static void setup_r_address_base(
    void);

static void update_local_relocs(
    uint32_t vmslide);

static void update_generic_local_relocs(
    uint32_t vmslide);

static void update_hppa_local_relocs(
    uint32_t vmslide);

static void update_sparc_local_relocs(
    uint32_t vmslide);

static void update_ppc_local_relocs(
    uint32_t vmslide);

static void update_arm_local_relocs(
    uint32_t vmslide);

static void update_external_relocs(
    uint32_t vmslide);

static void update_generic_external_relocs(
    uint32_t vmslide);

static void update_hppa_external_relocs(
    uint32_t vmslide);

static void update_sparc_external_relocs(
    uint32_t vmslide);

static void update_ppc_external_relocs(
    uint32_t vmslide);

static void update_arm_external_relocs(
    uint32_t vmslide);

static char *contents_pointer_for_vmaddr(
    uint32_t vmaddr,
    uint32_t size);

static void update_symbol_pointers(
    uint32_t vmslide);

static void update_self_modifying_stubs(
    uint32_t vmslide);

static void reset_symbol_pointers(
    uint32_t vmslide);

static void reset_self_modifying_stubs(
    void);

static enum bool check_pb_la_ptr_reloc_cputype(
unsigned int reloc_type);

static void update_load_commands(
    uint32_t vmslide);
	
static void message(
    const char *format, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

/*
 * These routines are used to get/set values that might not be aligned correctly
 * which are being relocated.
 */
static
inline
uint32_t
get_arch_long(
void *addr)
{
    int32_t l;

	memcpy(&l, addr, sizeof(uint32_t));
	if(arch_swapped == TRUE)
	    return(SWAP_INT(l));
	else
	    return(l);
}

static
inline
short
get_arch_short(
void *addr)
{
    short s;

	memcpy(&s, addr, sizeof(short));
	if(arch_swapped == TRUE)
	    return(SWAP_SHORT(s));
	else
	    return(s);
}

static
inline
char
get_arch_byte(
char *addr)
{
	return(*addr);
}

static
inline
void
set_arch_long(
void *addr,
uint32_t value)
{
	if(arch_swapped == TRUE)
	    value = SWAP_INT(value);
	memcpy(addr, &value, sizeof(uint32_t));
}

static
inline
void
set_arch_short(
void *addr,
short value)
{
    if(arch_swapped == TRUE)
	value = SWAP_SHORT(value);
    memcpy(addr, &value, sizeof(short));
}

static
inline
void
set_arch_byte(
char *addr,
char value)
{
	*addr = value;
}

/*
 * cleanup_libs() unmaps the ofiles for all the libraries in the libs[] array.
 */
static
void
cleanup_libs()
{
    uint32_t i;

	for(i = 0; i < nlibs; i++){
	    if(libs[i].ofile != NULL)
		ofile_unmap(libs[i].ofile);
	}
}

#ifndef LIBRARY_API
/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * main() see top of file for program's description and options.
 */
int
main(
int argc,
char *argv[],
char *envp[])
{
    int i;
    char *input_file, *output_file, *objcunique;
    struct arch *archs;
    uint32_t narchs;
    struct stat stat_buf;
    enum bool verbose, calculate_input_prebind_cksum, write_to_stdout;
    unsigned short mode;
    uid_t uid;
    gid_t gid;
    struct seg_addr_table *entry;
    char *install_name, *seg1addr_str, *endp;


    	input_file = NULL;
	output_file = NULL;
	archs = NULL;
	narchs = 0;
	errors = 0;
	verbose = FALSE;
	seg1addr_str = NULL;
	write_to_stdout = FALSE;

	progname = argv[0];

	for(i = 1; i < argc; i++){
	    if(argv[i][0] == '-'){
		if(strcmp(argv[i], "-o") == 0){
		    if(write_to_stdout)
			fatal("-o cannot be used with the -s option");
		    if(i + 1 >= argc)
			fatal("-o requires an argument");
		    if(output_file != NULL)
			fatal("only one -o option allowed");
		    output_file = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-s") == 0){
		    if(output_file != NULL)
			fatal("-s cannot be used with the -o option");
		    write_to_stdout = TRUE;
		}
		else if(strcmp(argv[i], "-r") == 0){
		    if(i + 1 >= argc)
			fatal("-r requires an argument");
		    if(root_dir != NULL)
			fatal("only one -r option allowed");
		    root_dir = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-e") == 0){
		    if(i + 1 >= argc)
			fatal("-e requires an argument");
		    if(executable_path != NULL)
			fatal("only one -e option allowed");
		    executable_path = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table") == 0){
		    if(i + 1 >= argc)
			fatal("-seg_addr_table requires an argument");
		    if(seg_addr_table_name != NULL)
			fatal("only one -seg_addr_table option allowed");
		    if(seg1addr_str != NULL)
			fatal("-seg_addr_table can't be used with "
			      "-seg1addr");
		    seg_addr_table_name = argv[i + 1];
		    seg_addr_table = parse_seg_addr_table(argv[i+1],
			argv[i], argv[i+1], &table_size);
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table_filename") == 0){
		    if(i + 1 >= argc)
			fatal("-seg_addr_table_filename requires an argument");
		    if(seg_addr_table_filename != NULL)
			fatal("only one -seg_addr_table_filename option "
			      "allowed");
		    if(seg1addr_str != NULL)
			fatal("-seg_addr_table_filename can't be used with "
			      "-seg1addr");
		    seg_addr_table_filename = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-seg1addr") == 0){
		    if(i + 1 >= argc)
			fatal("-seg1addr requires an argument");
		    if(seg1addr_str != NULL)
			fatal("only one -seg1addr option allowed");
		    if(seg_addr_table_filename != NULL || 
			seg_addr_table_name != NULL)
			fatal("-seg1addr can't be used with -seg_addr_table"
			      " or -seg_addr_table_filename");
		    seg1addr_str = argv[i + 1];
		    new_dylib_address = strtoul(argv[i + 1], &endp, 16);
		    if(*endp != '\0')
			fatal("address for %s %s not a proper "
			      "hexadecimal number", argv[i], argv[i + 1]);
		    i++;
		}
		else if(strcmp(argv[i], "-c") == 0){
		    check_only = TRUE;
		}
		else if(strcmp(argv[i], "-i") == 0){
		    ignore_non_prebound = TRUE;
		}
		else if(strcmp(argv[i], "-z") == 0){
		    zero_out_prebind_checksum = TRUE;
		}
		else if(strcmp(argv[i], "-p") == 0){
		    check_for_non_prebound = TRUE;
		}
		else if(strcmp(argv[i], "-d") == 0){
		    check_for_dylibs = TRUE;
		}
		else if(strcmp(argv[i], "-debug") == 0){
		    debug = TRUE;
		}
		else if(strcmp(argv[i], "-v") == 0){
		    verbose = TRUE;
		}
		else if(strcmp(argv[i], "-u") == 0){
		    unprebinding = TRUE;
		}
		else{
		    fprintf(stderr, "%s: unknown option: %s\n", progname,
			    argv[i]);
		    usage();
		}
	    }
	    else{
		if(input_file != NULL)
		    fatal("only one input file allowed");
		input_file = argv[i];
	    }
	}
	if(input_file == NULL){
	    fprintf(stderr, "%s no input file specified\n", progname);
	    usage();
	}
	if(check_only + check_for_non_prebound + check_for_dylibs > 1){
	    fprintf(stderr, "%s only one of -c, -p or -d can be specified\n",
		    progname);
	    usage();
	}

	/* breakout the file for processing */
	if(zero_out_prebind_checksum == TRUE)
	    calculate_input_prebind_cksum = FALSE;
	else
	    calculate_input_prebind_cksum = TRUE;
	breakout(input_file, &archs, &narchs, calculate_input_prebind_cksum);
	if(errors)
	    exit(2);

	/* checkout the file for processing */
	checkout(archs, narchs);

	/*
	 * If the -seg_addr_table option was specified then get the
	 * install_name of this binary if it is a dynamic library.  If it is
	 * then get the entry in the table for it.  There must be an entry in
	 * the table for it when the -seg_addr_table option is specified and
	 * the entry must have a non-zero address.
	 */
	if(seg_addr_table != NULL && unprebinding == FALSE){
	    install_name = get_install_name(archs, narchs);
	    if(install_name != NULL || seg_addr_table_filename != NULL){
		if(seg_addr_table_filename != NULL)
		    entry = search_seg_addr_table(seg_addr_table,
						  seg_addr_table_filename);
		else
		    entry = search_seg_addr_table(seg_addr_table, install_name);
		if(entry == NULL){
		    fprintf(stderr, "%s: no entry in -seg_addr_table %s for "
			    "input file's (%s) %s %s\n", progname,
			    seg_addr_table_name, input_file,
			    seg_addr_table_filename != NULL ?
			    "-seg_addr_table_filename" : "install name:",
			    seg_addr_table_filename != NULL ?
			    seg_addr_table_filename : install_name);
		    exit(2);
		}
		if(entry->split == TRUE)
		    new_dylib_address = entry->segs_read_only_addr;
		else
		    new_dylib_address = entry->seg1addr;
		if(new_dylib_address == 0){
		    fprintf(stderr, "%s: entry in -seg_addr_table %s for "
			    "input file's (%s) %s %s on line %u "
			    "has an address of zero\n", progname,
			    seg_addr_table_name, input_file,
			    seg_addr_table_filename != NULL ?
			    "-seg_addr_table_filename" : "install name:",
			    seg_addr_table_filename != NULL ?
			    seg_addr_table_filename : install_name,
			    entry->line);
		    exit(2);
		}
	    }
	}

	/* process the input file */
	process_archs(archs, narchs, has_resource_fork(input_file));
	if(errors)
	    exit(2);

	/*
	 * If we are checking for dylibs and get back from process_archs() we
	 * either have all dylibs or all non-dylibs.  So exit with 0 for dylibs
	 * an 1 for non-dylibs.
	 */
	if(check_for_dylibs == TRUE){
	    if(seen_a_dylib == TRUE)
		exit(0);
	    exit(1);
	}

	/*
	 * If we are checking for non-prebound files and get back from
	 * process_archs() we don't have any Mach-O's that were not prebound
	 * so indicate this with an exit status of 0.
	 */
	if(check_for_non_prebound == TRUE)
	    exit(0);

	/*
	 * Create an output file if we processed any of the archs and we are
	 * not doing checking only.
	 */
	if(arch_processed == TRUE){
	    if(check_only == TRUE)
		exit(1);
	    if(stat(input_file, &stat_buf) == -1)
		system_error("can't stat input file: %s", input_file);
	    mode = stat_buf.st_mode & 07777;
	    uid = stat_buf.st_uid;
            gid = stat_buf.st_gid;

	    if(output_file != NULL || write_to_stdout){
                if(write_to_stdout)
                    output_file = NULL;
		writeout(archs, narchs, output_file, mode, TRUE, FALSE, FALSE,
		         FALSE, NULL);
		if(errors){
                    if(write_to_stdout == FALSE)
                        unlink(output_file);
		    return(2);
		}
	    }
	    else{
		output_file = makestr(input_file, ".redo_prebinding", NULL);
		writeout(archs, narchs, output_file, mode, TRUE, FALSE, FALSE,
			 FALSE, NULL);
		if(errors){
		    unlink(output_file);
		    return(2);
		}
		if(rename(output_file, input_file) == 1)
		    system_error("can't move temporary file: %s to input "
				 "file: %s\n", output_file, input_file);
		free(output_file);
		output_file = NULL;
	    }
	    /*
	     * Run objcunique on the output.
	     */
	    objcunique = cmd_with_prefix("objcunique");
	    if(stat(objcunique, &stat_buf) != -1){
		reset_execute_list();
		add_execute_list(objcunique);
		if(output_file != NULL)
		    add_execute_list(output_file);
		else
		    add_execute_list(input_file);
		add_execute_list("-prebind");
		if(ignore_non_prebound == TRUE)
		    add_execute_list("-i");
		if(root_dir != NULL){
		    add_execute_list("-r");
		    add_execute_list(root_dir);
		}
		if(execute_list(verbose) == 0)
		    fatal("internal objcunique command failed");
	    }
	    /*
	     * Call chmod(2) to insure set-uid, set-gid and sticky bits get set.
	     * Then call chown to insure the file has the same owner and group
	     * as the original file.
	     */
	    if(output_file != NULL){
		if(chmod(output_file, mode) == -1)
		    system_error("can't set permissions on file: %s",
			output_file);
		if(chown(output_file, uid, gid) == -1)
		    system_error("can't set owner and group on file: %s",
			output_file);
	    }
	    else if(write_to_stdout == FALSE){
		if(chmod(input_file, mode) == -1)
		    system_error("can't set permissions on file: %s",
			input_file);
		if(chown(input_file, uid, gid) == -1)
		    system_error("can't set owner and group on file: %s",
			input_file);
	    }
	}
	else{
	    if(check_only == TRUE)
		exit(0);
	}

	/* clean-up data structures */
	free_archs(archs, narchs);

	if(errors)
	    return(2);
	else
	    return(0);
}

/*
 * usage() prints the current usage message.
 */
static
void
usage(
void)
{
	fprintf(stderr, "Usage: %s [-c|-p|-d] [-i] [-z] [-u] [-r rootdir] "
		"[-e executable_path] [-seg_addr_table table_file_name] "
		"[-seg_addr_table_filename pathname] [-seg1addr address]"
		"[-o output_file] [-s] input_file\n", progname);
	exit(EXIT_FAILURE);
}

/*
 * redo_exit() simply calls exit for the non-library api interface.
 */
static
void
redo_exit(
int value)
{
	exit(value);
}

/*
 * message() simply calls vprintf() for the non-library api interface.
 */
static
void
message(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

#else /* defined(LIBRARY_API) */
#include <setjmp.h>
#include <errno.h>
#include <mach/mach_error.h>
/*
 * The jump buffer to get back to the library's api call to allow catching
 * of things like malformed files, etc.
 */
static jmp_buf library_env;

/*
 * A pointer to a malloc(3)'ed error message buffer for error messages allocated
 * and filled in by the error routines in here for the library apis.
 */
static char *error_message_buffer = NULL;
#define ERROR_MESSAGE_BUFFER_SIZE 8192
static char *last = NULL;
static uint32_t left = 0;

static enum object_file_type_retval object_file_type_archs(
    struct arch *archs,
    uint32_t narchs);

static
void
setup_error_message_buffer(
void)
{
	if(error_message_buffer == NULL){
	    error_message_buffer = malloc(ERROR_MESSAGE_BUFFER_SIZE);
	    if(error_message_buffer == NULL)
		system_fatal("virtual memory exhausted (malloc failed)");
	    error_message_buffer[0] = '\0';
	    error_message_buffer[ERROR_MESSAGE_BUFFER_SIZE - 1] = '\0';
	    last = error_message_buffer;
	    left = ERROR_MESSAGE_BUFFER_SIZE - 1;
	}
}

/*
 * The zone allocation is done from this zone for the library api so it can
 * be cleaned up.
 */
static malloc_zone_t *library_zone = NULL;

/*
 * These two variables are used to support redo_prebinding()'s only_if_needed
 * parameter.
 */
static enum bool check_if_needed = FALSE;
static enum bool redo_prebinding_needed = FALSE;
static enum redo_prebinding_retval only_if_needed_retval;

/*
 * reset_statics() is used by the library api's to get all the static variables
 * in this file back to their initial values.
 */
static
void
reset_statics(
void)
{
	check_only = FALSE;
	seen_a_non_64_bit = FALSE;
	ignore_non_prebound = FALSE;
	check_for_non_prebound = FALSE;
	check_for_dylibs = FALSE;
	seen_a_dylib = FALSE;
	seen_a_non_dylib = FALSE;
	root_dir = NULL;
	executable_path = NULL;
	new_dylib_address = 0;
	dylib_vmslide = 0;
	debug = FALSE;
	arch_processed = FALSE;
	arch = NULL;
	memset(&arch_flag, '\0', sizeof(struct arch_flag));
	arch_swapped = FALSE;
	arch_name = NULL;
	arch_symbols = NULL;
	arch_symbols64 = NULL;
	arch_nsyms = 0;
	arch_strings = NULL;
	arch_strsize = 0;
	arch_tocs = NULL;
	arch_ntoc = 0;
	arch_mods = NULL;
	arch_mods64 = NULL;
	arch_nmodtab = 0;
	arch_refs = NULL;
	arch_nextrefsyms = 0;
	arch_hints = NULL;
	arch_nhints = 0;
	arch_state = LINKED;
	arch_seg1addr = 0;
	arch_segs_read_write_addr = 0;
	arch_split_segs = FALSE;
	arch_extrelocs = NULL;
	arch_locrelocs = NULL;
	arch_nextrel = 0;
	arch_nlocrel = 0;
	arch_indirect_symtab = NULL;
	arch_nindirectsyms = 0;
	arch_force_flat_namespace = FALSE;
	arch_cant_be_missing = 0;
	libs = NULL;
	nlibs = 0;
	memset(&arch_lib, '\0', sizeof(struct lib));
	memset(&undefined_list, '\0', sizeof(struct symbol_list));
	undefined_list.name = NULL;
	undefined_list.symbol = NULL;
	undefined_list.ilib = 0;
	undefined_list.prev = &undefined_list;
	undefined_list.next = &undefined_list;
	error_message_buffer = NULL;
	last = NULL;
	left = 0;
	errors = 0;
	check_if_needed = FALSE;
	redo_prebinding_needed = FALSE;
	unprebinding = FALSE;
}

/*
 * cleanup() is called when recoverable error occurs or when a successful
 * library api has been completed.  So we deallocate anything we allocated from
 * the zone up to this point.  Allocated items to be returned to the user are
 * allocated with malloc(3) and not with our allocate() which uses the zone.
 */
static
void
cleanup(
void)
{
	cleanup_libs();
	if(library_zone != NULL)
	    malloc_destroy_zone(library_zone);
	library_zone = NULL;
}

/*
 * For all the LIBRARY_APIs the parameters program_name and error_message
 * are used the same.  For unrecoverable resource errors like being unable to
 * allocate memory each API prints a message to stderr precede with program_name
 * then calls exit(2) with the value EXIT_FAILURE.  If an API is unsuccessful
 * and if error_message pass to it is not NULL it is set to a malloc(3)'ed
 * buffer with a NULL terminated string with the error message.  For all APIs 
 * when they return they release all resources (memory, open file descriptors,
 * etc). 
 * 
 * The file_name parameter for these APIs may be of the form "foo(bar)" which is
 * NOT interpreted as an archive name and a member name in that archive.  As
 * these API deal with prebinding and prebound binaries ready for execution
 * can't be in archives.
 * 
 * If the executable_path parameter for these APIs is not NULL it is used for
 * any dependent library has a path that starts with "@executable_path". Then
 * "@executable_path" is replaced with executable_path. 
 * 
 * If the root_dir parameter is not NULL it is prepended to all the rooted
 * dependent library paths. 
 */

/*
 * dependent_libs() takes a file_name of a binary and returns a malloc(3)'ed
 * array of pointers (NULL terminated) to names (also malloc(3)'ed and '\0'
 * terminated names) of all the dependent libraries for that binary (not
 * recursive) for all of the architectures of that binary.  If successful
 * dependent_libs() returns a non NULL value (at minimum a pointer to one NULL
 * pointer). If unsuccessful dependent_libs() returns NULL.
 */ 
char **
dependent_libs(
const char *file_name,
const char *program_name,
char **error_message)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    uint32_t i, j, k;
    struct ofile * volatile ofile;
    uint32_t ndependents;
    char **dependents, *dylib_name;
    struct load_command *lc;
    struct dylib_command *dl_load;
    enum bool found;
    uint32_t ncmds;
    
	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	ofile = NULL;
	ndependents = 0;
	dependents = NULL;
	archs = NULL;
	narchs = 0;

	/*
	 * Set up to handle recoverable errors.
	 */
	if(setjmp(library_env) != 0){
	    /*
	     * It takes a longjmp() to get to this point.  So we got an error
	     * so clean up and return NULL to say we were unsuccessful.
	     */
	    goto error_return;
	}

	/* breakout the file for processing */
	ofile = breakout((char *)file_name, (struct arch **)&archs,
				 (uint32_t *)&narchs, FALSE);
	if(errors)
	    goto error_return;

	/* checkout the file for processing */
	checkout(archs, narchs);

	/*
	 * Count the number of dynamic librarys in the all of the archs which
	 * are executables and dynamic libraries.
	 */
	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type == OFILE_Mach_O &&
	       (arch->object->mh_filetype == MH_EXECUTE ||
	        arch->object->mh_filetype == MH_BUNDLE ||
	        arch->object->mh_filetype == MH_DYLIB)){
		lc = arch->object->load_commands;
                if(arch->object->mh != NULL)
                    ncmds = arch->object->mh->ncmds;
                else
                    ncmds = arch->object->mh64->ncmds;
		for(j = 0; j < ncmds; j++){
		    switch(lc->cmd){
		    case LC_LOAD_DYLIB:
		    case LC_LOAD_WEAK_DYLIB:
		    case LC_REEXPORT_DYLIB:
			ndependents++;
			break;
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
	    }
	}
	dependents = (char **)malloc(sizeof(char *) * (ndependents + 1));
	if(dependents == NULL)
	    system_fatal("virtual memory exhausted (malloc failed)");
	/*
	 * Now fill in the dependents[] array with the names of the libraries.
	 */
	ndependents = 0;
	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type == OFILE_Mach_O &&
	       (arch->object->mh_filetype == MH_EXECUTE ||
	        arch->object->mh_filetype == MH_BUNDLE ||
	        arch->object->mh_filetype == MH_DYLIB)){
		lc = arch->object->load_commands;
                if(arch->object->mh != NULL)
                    ncmds = arch->object->mh->ncmds;
                else
                    ncmds = arch->object->mh64->ncmds;
		for(j = 0; j < ncmds; j++){
		    switch(lc->cmd){
		    case LC_LOAD_DYLIB:
		    case LC_LOAD_WEAK_DYLIB:
		    case LC_REEXPORT_DYLIB:
			dl_load = (struct dylib_command *)lc;
			dylib_name = (char *)dl_load +
				     dl_load->dylib.name.offset;
			found = FALSE;
			for(k = 0; k < ndependents; k++){
			    if(strcmp(dependents[k], dylib_name) == 0){
				found = TRUE;
				break;
			    }
			}
			if(found == FALSE){
			    dependents[ndependents] =
				(char *)malloc(strlen(dylib_name) + 1);
			    if(dependents[ndependents] == NULL)
				system_fatal("virtual memory exhausted (malloc "
					     "failed)");
			    strcpy(dependents[ndependents], dylib_name);
			    ndependents++;
			}
			break;
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
	    }
	}
	dependents[ndependents] = NULL;

	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	return(dependents);

error_return:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	else if(error_message_buffer != NULL)
	    free(error_message_buffer);
	return(NULL);
}

/*
 * install_name() takes a file_name of a binary and returns a malloc(3)'ed
 * pointer to a NULL terminated string containing the install_name value for
 * the binary. If unsuccessful install_name() returns NULL.  In particular,
 * NULL is returned if the binary is not a dylib and there is no error_message
 * set.  If the all of the arch's are dylibs but all the install names don't
 * match NULL is returned and a error_message is set.  If some but not all of
 * the archs are dylibs NULL is returned and a error_message is set.
 */ 
char *
install_name(
const char *file_name,
const char *program_name,
char **error_message)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    uint32_t i, j;
    volatile struct ofile *ofile;
    char *install_name, *dylib_name;
    struct load_command *lc;
    struct dylib_command *dl_id;
    enum bool non_dylib_found;
    uint32_t ncmds;
    
	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	ofile = NULL;
	archs = NULL;
	narchs = 0;
	install_name = NULL;
	non_dylib_found = FALSE;

	/*
	 * Set up to handle recoverable errors.
	 */
	if(setjmp(library_env) != 0){
	    /*
	     * It takes a longjmp() to get to this point.  So we got an error
	     * so clean up and return NULL to say we were unsuccessful.
	     */
	    goto error_return;
	}

	/* breakout the file for processing */
	ofile = breakout((char *)file_name, (struct arch **)&archs,
			 (uint32_t *)&narchs, FALSE);
	if(errors)
	    goto error_return;

	/* checkout the file for processing */
	checkout(archs, narchs);

	/*
	 * Count the number of dynamic librarys in the all of the archs which
	 * are executables and dynamic libraries.
	 */
	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type == OFILE_Mach_O &&
	       arch->object->mh_filetype == MH_DYLIB){
		lc = arch->object->load_commands;
                if(arch->object->mh != NULL)
                    ncmds = arch->object->mh->ncmds;
                else
                    ncmds = arch->object->mh64->ncmds;
		for(j = 0; j < ncmds; j++){
		    switch(lc->cmd){
		    case LC_ID_DYLIB:
			dl_id = (struct dylib_command *)lc;
			dylib_name = (char *)dl_id +
				     dl_id->dylib.name.offset;
			if(install_name != NULL){
			    if(strcmp(install_name, dylib_name) != 0){
				error("install names in all arch's don't "
				      "match");
				goto error_return;
			    }
			}
			else{
			    install_name = dylib_name;
			}
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
	    }
	    else{
		non_dylib_found = TRUE;
	    }
	}
	if(install_name != NULL && non_dylib_found == TRUE){
	    error("not all arch's are dylibs");
	    goto error_return;
	}
	if(install_name != NULL){
	    dylib_name = malloc(strlen(install_name) + 1);
	    strcpy(dylib_name, install_name);
	    install_name = dylib_name;
	}

	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap((struct ofile *)ofile);
	cleanup();
	return(install_name);

error_return:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap((struct ofile *)ofile);
	cleanup();
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	else if(error_message_buffer != NULL)
	    free(error_message_buffer);
	return(NULL);
}

/*
 * redo_prebinding() takes a file_name of a binary and redoes the prebinding on
 * it.  If output_file is not NULL the update file is written to output_file,
 * if not it is written to file_name.  If redo_prebinding() is successful it
 * returns REDO_PREBINDING_SUCCESS otherwise it returns REDO_PREBINDING_FAILURE
 * If the parameter allow_missing_architectures is zero and not all
 * architectures can be updated it is not successful and nothing is done and
 * this returns REDO_PREBINDING_FAILURE.  If the parameter
 * allow_missing_architectures is non-zero then only problems with missing
 * architectures for the architecure of the cputype specified by 
 * allow_missing_architectures will cause this call to fail.  Other
 * architectures that could not be prebound due to missing architectures in
 * depending libraries will not have their prebinding updated but will not
 * cause this call to fail.
 * If the slide_to_address parameter is non-zero and the binary is a
 * dynamic library it is relocated to have that has its prefered address.  If
 * only_if_needed is non-zero the prebinding checked first and only done if
 * needed.  The checking includes checking the prefered address against the
 * slide_to_address value if it is non-zero.  If only_if_needed is non-zero
 * and the prebinding does not have to be redone REDO_PREBINDING_NOT_NEEDED is
 * returned, if the binary is not prebound REDO_PREBINDING_NOT_PREBOUND is
 * returned and if the new load commands do not fit in the binary and it needs
 * to be rebuilt REDO_PREBINDING_NEED_REBUILDING is returned.
 * If zero_checksum is non-zero then the cksum field the LC_PREBIND_CKSUM load
 * command (if any) is set to zero on output.
 * If throttle is non-NULL it points to a value of the maximum bytes per second
 * to use for writting the output.  If the value is ULONG_MAX then the actual
 * bytes per second is returned indirectly through *throttle.
 */
enum redo_prebinding_retval
redo_prebinding(
const char *file_name,
const char *executable_path_arg,
const char *root_dir_arg,
const char *output_file,
const char *program_name,
char **error_message,
uint32_t slide_to_address,
int only_if_needed,
int zero_checksum,
cpu_type_t allow_missing_architectures,
uint32_t *throttle)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    struct ofile * volatile ofile;
    struct stat stat_buf;
    unsigned short mode;
    uid_t uid;
    gid_t gid;
    enum bool calculate_input_prebind_cksum;

	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	executable_path = (char *)executable_path_arg;
	root_dir = (char *)root_dir_arg;
	new_dylib_address = slide_to_address;
	zero_out_prebind_checksum = zero_checksum;
	arch_cant_be_missing = allow_missing_architectures;
	ofile = NULL;
	archs = NULL;
	narchs = 0;

	/*
	 * To avoid changing the library api, check for an environment variable
	 * here to ignore non-prebound archs when prebinding.
	 */
	if(getenv("RP_IGNORE_NON_PREBOUND") != NULL)
	   ignore_non_prebound = TRUE;

	/*
	 * If only_if_needed is non-zero then set check_if_needed to TRUE and
	 * assume that prebinding is not need and set only_if_needed_retval to
	 * indicate success.  These last two will get reset as things get
	 * checked and processed.
	 */
	if(only_if_needed != 0)
	    check_if_needed = TRUE;
	else
	    check_if_needed = FALSE;
	redo_prebinding_needed = FALSE;
	only_if_needed_retval = REDO_PREBINDING_SUCCESS;

	/*
	 * Set up to handle recoverable errors.
	 */
	if(setjmp(library_env) != 0){
	    /*
	     * It takes a longjmp() to get to this point.  So we got an error
	     * so clean up and return NULL to say we were unsuccessful.
	     */
	    goto error_return;
	}

	/* breakout the file for processing */
	if(zero_out_prebind_checksum == TRUE)
	    calculate_input_prebind_cksum = FALSE;
	else
	    calculate_input_prebind_cksum = TRUE;
	ofile = breakout((char *)file_name, (struct arch **)&archs,
			 (uint32_t *)&narchs,
			 calculate_input_prebind_cksum);
	if(errors)
	    goto error_return;

	/* checkout the file for processing */
	checkout(archs, narchs);
	if(errors)
	    goto error_return;

	/* process the archs redoing the prebinding */
	process_archs(archs, narchs, has_resource_fork((char *)file_name));
	if(errors)
	    goto error_return;

	if(check_if_needed == TRUE && redo_prebinding_needed == FALSE){
	    if(only_if_needed_retval == REDO_PREBINDING_SUCCESS)
		only_if_needed_retval = REDO_PREBINDING_NOT_NEEDED;
	    goto error_return;
	}

	/*
	 * Create an output file if we processed any of the archs.
	 */
	if(arch_processed == TRUE){
	    if(stat(file_name, &stat_buf) == -1)
		system_error("can't stat input file: %s", file_name);
	    mode = stat_buf.st_mode & 06777;
	    uid = stat_buf.st_uid;
            gid = stat_buf.st_gid;

	    if(output_file != NULL){
		writeout(archs, narchs, (char *)output_file, mode, TRUE, FALSE,
			 FALSE, FALSE, throttle);
		if(errors){
		    unlink(output_file);
		    goto error_return;
		}
	    }
	    else{
		output_file = makestr(file_name, ".redo_prebinding", NULL);
		writeout(archs, narchs, (char *)output_file, mode, TRUE, FALSE,
			 FALSE, FALSE, throttle);
		if(errors){
		    unlink(output_file);
		    goto error_return;
		}
		if(rename(output_file, file_name) == 1)
		    system_error("can't move temporary file: %s to input "
				 "file: %s\n", output_file, file_name);
		free((char *)output_file);
		output_file = NULL;
	    }
	    /*
	     * Call chmod(2) to insure set-uid, set-gid and sticky bits get set.
	     * Then call chown to insure the file has the same owner and group
	     * as the original file.
	     */
	    if(output_file != NULL){
		if(chmod(output_file, mode) == -1)
		    system_error("can't set permissions on file: %s",
			output_file);
		if(chown(output_file, uid, gid) == -1)
		    system_error("can't set owner and group on file: %s",
			output_file);
	    }
	    else{
		if(chmod(file_name, mode) == -1)
		    system_error("can't set permissions on file: %s",
			file_name);
		if(chown(file_name, uid, gid) == -1)
		    system_error("can't set owner and group on file: %s",
			file_name);
	    }
	}

	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();

	return(REDO_PREBINDING_SUCCESS); /* successful */

error_return:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	else if(error_message_buffer != NULL)
	    free(error_message_buffer);
	if(only_if_needed != 0 &&
	   only_if_needed_retval != REDO_PREBINDING_SUCCESS)
	    return(only_if_needed_retval);
	return(REDO_PREBINDING_FAILURE); /* unsuccessful */
}


/*
 * unprebind() takes a file_name of a binary and resets or removes prebinding
 * information from it.  If inbuf is non-NULL, the memory pointed to by inbuf 
 * is used as the input file contents.  Otherwise, the contents are loaded from
 * the file at path file_name.  Even if inbuf is non-NULL, a file_name parameter
 * should be specified for error reporting.  Similarly, if outbuf is non-NULL, 
 * upon return, outbuf will point to a buffer containing the unprebound binary 
 * and outlen will point to the length of the output buffer.  This buffer is 
 * vm_allocate'd and therefore should be vm_deallocate'd when it is no longer
 * needed.  If outbuf is NULL, and output_file is not NULL the update file is 
 * written to output_file, if outbuf is NULL and output_file is NULL, it is 
 * written to file_name.  
 * If unprebind() is successful it returns REDO_PREBINDING_SUCCESS otherwise it
 * returns REDO_PREBINDING_FAILURE If the binary is already unprebound (i.e. it
 * has the MH_PREBINDABLE flag set) then REDO_PREBINDING_NOT_NEEDED is returned.
 * If the binary is not prebound and not prebindable, 
 * REDO_PREBINDING_NOT_PREBOUND is returned.  If zero_checksum is non-zero then 
 * the cksum field the LC_PREBIND_CKSUM load command (if any) is set to zero on
 * output, otherwise it is left alone.
 * Unprebinding slides dynamic libraries to address zero, resets prebound 
 * symbols to address zero and type undefined, resets symbol pointers, removes 
 * LC_PREBOUND_DYLIB commands, resets library timestamps, resets two-level hints
 * and updates relocation entries if necessary.  Unprebound binaries have
 * the MH_PREBINDABLE flag set, but not MH_PREBOUND.  It will also set the the
 * MH_ALLMODSBOUND flag if all two-level libraries were used and all modules
 * were found to be bound in the LC_PREBOUND_DYLIB commands.
 * As unprebinding is intended to produce a canonical Mach-O
 * binary, bundles and non-prebound executables and dylibs are acceptable
 * as input.  For these files, the unprebind operation will zero library 
 * time stamps and version numbers and zero entries in the two-level hints
 * table.  These files will not gain the MH_PREBINDABLE flag.
 * All resulting binaries successfully processed by unprebind() will have
 * the MH_CANONICAL flag.
 */
/*
 * Note: unprebind API boundary will not support files larger than 2^32 bytes
 * in size. If the output size is too large to fit in outlen unprebind will
 * return REDO_PREBINDING_FAILURE.
 */
enum redo_prebinding_retval
unprebind(
const char *file_name,
const char *output_file,
const char *program_name,
char **error_message,
int zero_checksum,
void *inbuf,
uint32_t inlen,
void **outbuf,
uint32_t *outlen)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    struct ofile * volatile ofile;
    struct stat stat_buf;
    unsigned short mode;
    uid_t uid;
    gid_t gid;
    enum bool calculate_input_prebind_cksum, seen_archive;
    uint64_t outlen64;

	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	new_dylib_address = 0;
	zero_out_prebind_checksum = zero_checksum;
	arch_cant_be_missing = FALSE;
	ofile = NULL;
	archs = NULL;
	narchs = 0;

	/*
	 * for unprebind, "only if needed" is implicitly true -
	 * we return REDO_PREBINDING_NOT_NEEDED if unprebinding
	 * is not necessary.
	 */
	check_if_needed = TRUE; 
	only_if_needed_retval = REDO_PREBINDING_SUCCESS;

	/*
	 * Set up to handle recoverable errors.
	 */
	if(setjmp(library_env) != 0){
	    /*
	     * It takes a longjmp() to get to this point.  So we got an error
	     * so clean up and return NULL to say we were unsuccessful.
	     */
	    goto error_return;
	}

	/* breakout the file for processing */
	if(zero_out_prebind_checksum == TRUE)
	    calculate_input_prebind_cksum = FALSE;
	else
	    calculate_input_prebind_cksum = TRUE;
	if(inbuf != NULL){
	    /* 
	     * We will use the inbuf as the input file if it 
	     * is non-NULL.  However, we need to make sure the 
	     * file_name is non-NULL.  If it is NULL, we will 
	     * use a dummy file name.
	     */
	    if(file_name == NULL)
		file_name = "(from unprebind() call)";
		
	    ofile = breakout_mem(inbuf, inlen, (char *)file_name,
				 (struct arch **)&archs,
			         (uint32_t *)&narchs,
				 calculate_input_prebind_cksum);
	}
	else
	    ofile = breakout((char *)file_name, (struct arch **)&archs,
			     (uint32_t *)&narchs,
			     calculate_input_prebind_cksum);
	if(errors)
	    goto error_return;

	/* checkout the file for processing */
	checkout(archs, narchs);
	if(errors)
	    goto error_return;

        unprebinding = TRUE;

	/* process the archs redoing the prebinding */
	process_archs(archs, narchs, has_resource_fork((char *)file_name));
	if(errors)
	    goto error_return;

	if(check_if_needed == TRUE && redo_prebinding_needed == FALSE){
	    if(only_if_needed_retval == REDO_PREBINDING_SUCCESS)
		only_if_needed_retval = REDO_PREBINDING_NOT_NEEDED;
	    goto error_return;
	}

	/*
	 * Create an output file if we processed any of the archs.
	 */
	if(arch_processed == TRUE){
	    if(inbuf == NULL){
		if(stat(file_name, &stat_buf) == -1)
		    system_error("can't stat input file: %s", file_name);
		mode = stat_buf.st_mode & 06777;
		uid = stat_buf.st_uid;
		gid = stat_buf.st_gid;
	    }
	    else{
		mode = 00777;
		uid = getuid();
		gid = getgid();
	    }

	    if(output_file != NULL){
	    	if(outbuf != NULL){
		    writeout_to_mem(archs, narchs, (char *)output_file, outbuf,
	    			    &outlen64, TRUE, FALSE, FALSE, FALSE,
				    &seen_archive);
		    *outlen = (uint32_t)outlen64;
	    	}else
		    writeout(archs, narchs, (char *)output_file, mode, TRUE, 
			     FALSE, FALSE, FALSE, NULL);
		if(errors){
		    if(outbuf == NULL)
			unlink(output_file);
		    goto error_return;
		}
	    }
	    else{
		output_file = makestr(file_name, ".redo_prebinding", NULL);
		if(outbuf != NULL){
		    writeout_to_mem(archs, narchs, (char *)output_file, outbuf,
				    &outlen64, TRUE, FALSE, FALSE, FALSE,
				    &seen_archive);
		    /* error if outlen is too large */
		    if (outlen64 > 0xFFFFFFFF) {
			if(outbuf == NULL)
			    unlink(output_file);
			goto error_return;
		    }
		    *outlen = (uint32_t)outlen64;
		}else
		    writeout(archs, narchs, (char *)output_file, mode, TRUE, 
			     FALSE, FALSE, FALSE, NULL);
		if(errors){
		    if(outbuf == NULL)
			unlink(output_file);
		    goto error_return;
		}
		if(outbuf == NULL && rename(output_file, file_name) == 1)
		system_error("can't move temporary file: %s to input "
			     "file: %s\n", output_file, file_name);
		free((char *)output_file);
		output_file = NULL;
	    }
	    /*
	     * Call chmod(2) to insure set-uid, set-gid and sticky bits get set.
	     * Then call chown to insure the file has the same owner and group
	     * as the original file.
	     */
	    if(output_file != NULL && outbuf == NULL){
		if(chmod(output_file, mode) == -1)
		    system_error("can't set permissions on file: %s",
				 output_file);
		if(chown(output_file, uid, gid) == -1)
		    system_error("can't set owner and group on file: %s",
				 output_file);
	    }
	}

	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	if(error_message_buffer != NULL)
	    free(error_message_buffer);

	return(REDO_PREBINDING_SUCCESS); /* successful */

error_return:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	else if(error_message_buffer != NULL)
	    free(error_message_buffer);
	if(only_if_needed_retval != REDO_PREBINDING_SUCCESS)
	    return(only_if_needed_retval);
	return(REDO_PREBINDING_FAILURE); /* unsuccessful */
}


/*
 * The redo_exit() routine sets this value for the library apis.
 */
static enum needs_redo_prebinding_retval retval;

/*
 * redo_exit() for library api interface translates the value of
 * redo_prebinding(1) -c to the needs_redo_prebinding() return value then
 * longjmp()'s back.
 */
static
void
redo_exit(
int value)
{
	switch(value){
	case 1:
	     retval = PREBINDING_OUTOFDATE;
	     break;
	case 2:
	case 3:
	     retval = PREBINDING_UNKNOWN;
	     break;
	default:
	     fprintf(stderr, "%s: internal error redo_exit() called with (%d) "
		     "unexpected value\n", progname, value);
	     exit(1);
	}
	longjmp(library_env, 1);
}

/*
 * needs_redo_prebinding() takes a file_name and determines if it is a binary
 * and if its prebinding is up to date.  It returns one of the
 * needs_redo_prebinding_retval values depending on the state of the binary and
 * libraries.  The value of PREBINDING_UNKNOWN is returned if all architectures
 * are not in the same state.  If the parameter expected_address is not zero
 * and the binary is a dynamic library then the library is checked to see if it
 * is at the expected_address if not the prebinding is assumed to be out of
 * date and PREBINDING_OUTOFDATE is returned.  If the parameter
 * allow_missing_architectures is zero then the value returned is based on the
 * first architecture for fat files.  If the parameter
 * allow_missing_architectures is non-zero then the value returned is based on
 * the cputype specified by allow_missing_architectures.  If that architecture
 * is not present then PREBINDING_UPTODATE is returned.
 */
enum needs_redo_prebinding_retval
needs_redo_prebinding(
const char *file_name,
const char *executable_path_arg,
const char *root_dir_arg,
const char *program_name,
char **error_message,
uint32_t expected_address,
cpu_type_t allow_missing_architectures)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    struct ofile * volatile ofile;

	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	executable_path = (char *)executable_path_arg;
	root_dir = (char *)root_dir_arg;
	new_dylib_address = expected_address;
	arch_cant_be_missing = allow_missing_architectures;
	ofile = NULL;
	archs = NULL;
	narchs = 0;

	/*
	 * To avoid changing the library api, check for an environment variable
	 * here to ignore non-prebound archs when prebinding.
	 */
	if(getenv("RP_IGNORE_NON_PREBOUND") != NULL)
	   ignore_non_prebound = TRUE;

	/*
	 * The code when check_only is TRUE assumes the prebinding is up to
	 * date. If it is not the code will change the retval before returning.
	 */
	check_only = TRUE;
	retval = PREBINDING_UPTODATE;

	/*
	 * Set up to handle recoverable errors and longjmp's from the
	 * redo_exit() routine.
	 */
	if(setjmp(library_env) != 0){
	    goto return_point;
	}

	/* breakout the file for processing */
	ofile = breakout((char *)file_name, (struct arch **)&archs,
			 (uint32_t *)&narchs, FALSE);
	if(errors){
	    if(retval == PREBINDING_UPTODATE)
		retval = PREBINDING_UNKNOWN;
	    goto return_point;
	}

	/* checkout the file for processing */
	checkout(archs, narchs);
	if(errors){
	    if(retval == PREBINDING_UPTODATE)
		retval = PREBINDING_UNKNOWN;
	    goto return_point;
	}

	/*
	 * Now with check_only set to TRUE process the archs.  For error cases
	 * the retval will get set by process_archs() or one of the routines.
	 * If arch_processed is TRUE then set retval to PREBINDING_OUTOFDATE
	 * else used the assumed initialized value PREBINDING_UPTODATE.
	 */
	process_archs(archs, narchs, has_resource_fork((char *)file_name));

	/*
	 * If we have only seen 64-bit Mach-O files then we need to return
	 * NOT_PREBOUND instead of the assumed PREBINDING_UPTODATE if that is
	 * what we would have returned.
	 */
	if(retval == PREBINDING_UPTODATE && seen_a_non_64_bit == FALSE)
	    retval = NOT_PREBOUND;

return_point:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	cleanup();
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	else if(error_message_buffer != NULL)
	    free(error_message_buffer);
	return(retval);
}

/*
 * object_file_type() takes a file_name and determines what type of object
 * file it is.  If it is a fat file and the architectures are not of the same
 * type then OFT_INCONSISTENT is returned.  If the file_name can't be opened,
 * read or malformed then OFT_FILE_ERROR is returned.
 */
enum object_file_type_retval
object_file_type(
const char *file_name,
const char *program_name,
char **error_message)
{
    struct arch * volatile archs;
    volatile uint32_t narchs;
    struct ofile * volatile ofile;
    enum object_file_type_retval retval;

	reset_statics();
	ofile = NULL;
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;

	/*
	 * Set up to handle recoverable errors and longjmp's from the
	 * redo_exit() routine.
	 */
	if(setjmp(library_env) != 0){
	    retval = OFT_FILE_ERROR;
	    goto done;
	}

	/* breakout the file for processing */
	ofile = breakout((char *)file_name, (struct arch **)&archs,
			 (uint32_t *)&narchs, FALSE);
	if(errors){
	    retval = OFT_FILE_ERROR;
	    goto done;
	}

	/* checkout the file for processing */
	checkout(archs, narchs);
	if(errors){
	    retval = OFT_FILE_ERROR;
	    goto done;
	}

	/* process the archs determining the type */
	retval = object_file_type_archs(archs, narchs);

done:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	return(retval);
}

/*
 * object_file_type_archs() is passed a set of broken out archs and returns one
 * of the object_file_type_retval enum values corresponding to the type of the
 * archs.
 */
static
enum object_file_type_retval
object_file_type_archs(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i;
    struct arch *arch;
    enum bool type_determined;
    enum object_file_type_retval retval, current;

	retval = OFT_OTHER;
    	type_determined = FALSE;

	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type == OFILE_ARCHIVE){
		current = OFT_ARCHIVE;
	    }
	    else if(arch->type == OFILE_Mach_O){
		switch(arch->object->mh_filetype){
		case MH_EXECUTE:
		    current = OFT_EXECUTABLE;
		    break;
		case MH_DYLIB:
		    current = OFT_DYLIB;
		    break;
		case MH_BUNDLE:
		    current = OFT_BUNDLE;
		    break;
		default:
		    current = OFT_OTHER;
		    break;
		}
	    }
	    else
		current = OFT_OTHER;

	    if(type_determined == TRUE && retval != current)
		return(OFT_INCONSISTENT);
	    retval = current;
	    type_determined = TRUE;
	}
	return(retval);
}

/*
 * get_prebind_cksums() takes a file_name that is a Mach-O file or fat file
 * containing Mach-O files and returns a malloc(3)'ed array of
 * prebind_cksum_arch structs indirectly through the cksums parameter.
 * If successful it returns zero else it returns non-zero.
 */
int
get_prebind_cksums(
const char *file_name,
struct prebind_cksum_arch **cksums,
uint32_t *ncksums,
const char *program_name,
char **error_message)
{
    uint32_t i;
    struct arch * volatile archs;
    struct arch *arch;
    volatile uint32_t narchs;
    struct ofile * volatile ofile;
    int retval;

	reset_statics();
	progname = (char *)program_name;
	if(error_message != NULL)
	    *error_message = NULL;
	*cksums = NULL;
	ofile = NULL;

	/*
	 * Set up to handle recoverable errors and longjmp's from the
	 * redo_exit() routine.
	 */
	if(setjmp(library_env) != 0){
	    retval = 1;
	    goto done;
	}

	/* breakout the file for processing */
	ofile = breakout((char *)file_name, (struct arch **)&archs,
			 (uint32_t *)&narchs, FALSE);
	if(errors){
	    retval = 1;
	    goto done;
	}

	/* checkout the file for processing */
	checkout(archs, narchs);
	if(errors){
	    retval = 1;
	    goto done;
	}

	*cksums = malloc(sizeof(struct prebind_cksum_arch) * narchs);
	if(*cksums == NULL)
	    system_fatal("virtual memory exhausted (malloc failed)");
	memset(*cksums, '\0', sizeof(struct prebind_cksum_arch) * narchs);
	*ncksums = narchs;

	for(i = 0; i < narchs; i++){
	    arch = (struct arch *)(archs + i);
	    if(arch->type == OFILE_Mach_O){
		(*cksums)[i].cputype = arch->object->mh_cputype;
		(*cksums)[i].cpusubtype = arch->object->mh_cpusubtype;
		if(arch->object->cs != NULL){
		    (*cksums)[i].has_cksum = 1;
		    (*cksums)[i].cksum = arch->object->cs->cksum;
		}
	    }
	    else if(arch->fat_arch64 != NULL){
		(*cksums)[i].cputype = arch->fat_arch64->cputype;
		(*cksums)[i].cpusubtype = arch->fat_arch64->cpusubtype;
	    }
	    else if(arch->fat_arch != NULL){
		(*cksums)[i].cputype = arch->fat_arch->cputype;
		(*cksums)[i].cpusubtype = arch->fat_arch->cpusubtype;
	    }
	}

	retval = 0;

done:
	free_archs(archs, narchs);
	if(ofile != NULL)
	    ofile_unmap(ofile);
	if(error_message != NULL && error_message_buffer != NULL)
	    *error_message = error_message_buffer;
	return(retval);
}
#endif /* defined(LIBRARY_API) */

#ifndef LIBRARY_API
/*
 * get_install_name() is passed the broken out arch's and returns the
 * install_name if the arch's are all dynamic libraries and have the same
 * install_name.  Other errors for non Mach-O are left to process_archs().
 */
static
char *
get_install_name(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i,j;
    char *install_name;
    struct load_command *lc, *load_commands;
    struct dylib_command *dl_id;
    uint32_t ncmds;

	install_name = NULL;
	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type == OFILE_Mach_O){
		if(arch->object->mh_filetype == MH_DYLIB){
		    load_commands = arch->object->load_commands;
		    lc = load_commands;
		    dl_id = NULL;
                    if(arch->object->mh != NULL)
                        ncmds = arch->object->mh->ncmds;
                    else
                        ncmds = arch->object->mh64->ncmds;
		    for(j = 0;
			j < ncmds && dl_id == NULL;
			j++){
			switch(lc->cmd){
			case LC_ID_DYLIB:
			    dl_id = (struct dylib_command *)lc;
			    if(install_name != NULL){
				if(strcmp(install_name, (char *)dl_id +
					  dl_id->dylib.name.offset) != 0)
				    fatal_arch(arch, NULL, "fat archs have "
					"different install_names (%s and %s)",
					install_name, (char *)dl_id +
					dl_id->dylib.name.offset);
			    }
			    else
				install_name = (char *)dl_id +
						dl_id->dylib.name.offset;
			}
			lc = (struct load_command *)((char *)lc + lc->cmdsize);
		    }
		}
	    }
	}
	return(install_name);
}
#endif /* !defined(LIBRARY_API) */

/*
 * process_archs() is passed the broken out arch's and processes each of them
 * checking to make sure they are prebound and either executables or dylibs.
 */
static
void
process_archs(
struct arch *archs,
uint32_t narchs,
enum bool has_resource_fork)
{
    uint32_t i;
    uint32_t mh_flags;
    
	for(i = 0; i < narchs; i++){
	    arch = archs + i;
	    if(arch->type != OFILE_Mach_O){
		if(check_for_dylibs == TRUE){
		    if(seen_a_dylib == TRUE)
			exit(2);
		    seen_a_non_dylib = TRUE;
		    continue;
		}
#ifdef LIBRARY_API
		else if(check_only == TRUE){
		    retval = NOT_PREBINDABLE;
		    return;
		}
#endif
		else if(check_only == TRUE ||
		        ignore_non_prebound == TRUE ||
			check_for_non_prebound == TRUE)
		    continue;
		else{
		    fatal_arch(arch, NULL, "file is not a Mach-O file: ");
		}
	    }

	    /* 
	     * For now, prebinding of 64-bit Mach-O is not supported. 
	     * So continue and leave the file unchanged.
	     */
	    if(arch->object->mh == NULL){
		arch->dont_update_LC_ID_DYLIB_timestamp = TRUE;
		continue;
	    }
	    else{
		seen_a_non_64_bit = TRUE;
	    }

	    /*
	     * The statically linked executable case.
	     */
            if(arch->object->mh != NULL)
                mh_flags = arch->object->mh->flags;
            else 
                mh_flags = arch->object->mh64->flags;
	    if(arch->object->mh_filetype == MH_EXECUTE &&
	       (mh_flags & MH_DYLDLINK) != MH_DYLDLINK){
		if(check_for_dylibs == TRUE){
		    if(seen_a_dylib == TRUE)
			exit(2);
		    seen_a_non_dylib = TRUE;
		}
#ifdef LIBRARY_API
		else if(check_if_needed == TRUE){
		    only_if_needed_retval = REDO_PREBINDING_NOT_NEEDED;
		    return;
		}
		else if(check_only == TRUE){
		    retval = NOT_PREBINDABLE;
		    return;
		}
#endif
		else if(check_only == TRUE ||
		   ignore_non_prebound == TRUE ||
		   check_for_non_prebound == TRUE)
		    continue;
		else{
		    fatal_arch(arch, NULL, "file is Mach-O executable that is"
			       "not dynamically linked: ");
		}
	    }

	    /*
	     * For the unprebind operation for bundles we need to process them
	     * and zero out their dylib timestamps.
	     */
	    if(arch->object->mh_filetype != MH_EXECUTE &&
	       arch->object->mh_filetype != MH_DYLIB &&
	       ! (unprebinding && arch->object->mh_filetype == MH_BUNDLE)){
		if(check_for_dylibs == TRUE){
		    if(seen_a_dylib == TRUE)
			exit(2);
		    seen_a_non_dylib = TRUE;
		}
#ifdef LIBRARY_API
		else if(check_if_needed == TRUE){
		    only_if_needed_retval = REDO_PREBINDING_NOT_PREBOUND;
		    return;
		}
		else if(check_only == TRUE){
		    retval = NOT_PREBINDABLE;
		    return;
		}
#endif
		else if(check_only == TRUE ||
			ignore_non_prebound == TRUE ||
			check_for_non_prebound == TRUE)
		    continue;
		else{
		    fatal_arch(arch, NULL, "file is not a Mach-O "
				"executable or dynamic shared library file: ");
		}
	    }
	    if(check_for_dylibs == TRUE){
		if(arch->object->mh_filetype == MH_DYLIB){
		    if(seen_a_non_dylib == TRUE)
			exit(2);
		    seen_a_dylib = TRUE;
		}
		else{
		    if(seen_a_dylib == TRUE)
			exit(2);
		    seen_a_non_dylib = TRUE;
		}
		continue;
	    }
	    if((unprebinding == FALSE && 
		(mh_flags & MH_PREBOUND) != MH_PREBOUND &&
	        (mh_flags & MH_PREBINDABLE) != MH_PREBINDABLE)){
		if(check_for_non_prebound == TRUE){
		    if((mh_flags & MH_DYLDLINK) == MH_DYLDLINK)
			exit(1);
		    continue;
		}
#ifdef LIBRARY_API
		else if(check_if_needed == TRUE){
		    if(arch_cant_be_missing != arch->object->mh_cputype){
			continue;
		    }
		    only_if_needed_retval = REDO_PREBINDING_NOT_PREBOUND;
		    return;
		}
		else if(check_only == TRUE){
		    if(arch_cant_be_missing != arch->object->mh_cputype){
			continue;
		    }
		    retval = NOT_PREBOUND;
		    return;
		}
#endif
		else if(check_only == TRUE || ignore_non_prebound == TRUE){
		    continue;
		}
		else
		    fatal_arch(arch, NULL, "file is not prebound: ");
	    }
#ifdef LIBRARY_API
	    else if(unprebinding == TRUE && 
		    (mh_flags & MH_PREBOUND) != MH_PREBOUND &&
		    (mh_flags & MH_DYLDLINK) != MH_DYLDLINK){
		only_if_needed_retval = REDO_PREBINDING_NOT_PREBOUND;
		return;
	    }
#endif
	    else if(unprebinding == TRUE &&
		    (mh_flags & MH_PREBOUND) != MH_PREBOUND &&
		    (mh_flags & MH_DYLDLINK) != MH_DYLDLINK){
		continue;
	    }
	    if(check_for_non_prebound == TRUE)
		continue;

	    /*
	     * How we are actually ready to redo the prebinding on this file.
	     * If it has a resource fork then don't do it as we will loose the
	     * resource fork when creating the new output file.
	     */
	    if(has_resource_fork == TRUE)
		fatal_arch(arch, NULL, "redoing the prebinding can't be done "
			   "because file has a resource fork or "
			   "type/creator: ");

	    /* Now redo the prebinding for this arch[i] */
            if(unprebinding == TRUE)
                unprebind_arch();
            else
                process_arch();
#ifdef LIBRARY_API
	    /*
	     * for needs_redo_prebinding() we return if this is the
	     * arch that can't be missing.
	     */
	    if(check_only == TRUE &&
	       (arch_cant_be_missing != 0 &&
	        arch_cant_be_missing == arch->object->mh_cputype))
		return;
#endif
	}
}

/*
 * process_arch() takes one arch which is a prebound executable or dylib and
 * redoes the prebinding.
 */
static
void
process_arch(
void)
{
    uint32_t mh_flags;
    
	/*
	 * Clear out any libraries loaded for the previous arch that was
	 * processed.
	 */
	if(libs != NULL){
	    cleanup_libs();
	    free(libs);
	}
	libs = NULL;
	nlibs = 0;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;

	/*
	 * Clear out the fake lib struct for this arch which holds the
	 * two-level namespace stuff for the arch.
	 */
	memset(&arch_lib, '\0', sizeof(struct lib));
	arch_force_flat_namespace = (mh_flags & MH_FORCE_FLAT) == MH_FORCE_FLAT;

	/* set up an arch_flag for this arch's object */
	arch_flag.cputype = arch->object->mh_cputype;
	arch_flag.cpusubtype = arch->object->mh_cpusubtype;
	set_arch_flag_name(&arch_flag);
	arch_name = arch_flag.name;

	if(debug == TRUE)
	    printf("%s: processing file: %s (for architecture %s)\n",
		   progname, arch->file_name, arch_name);

	/*
	 * If this is a dynamic library get the existing address of the library.
	 * And if the new_dylib_address is non-zero calculate the value needed
	 * to be added to the old addresses to move them t the new addresses.
	 */
	if(arch->object->mh_filetype == MH_DYLIB){
	    old_dylib_address = get_dylib_address();
	    if(new_dylib_address != 0){
		dylib_vmslide = new_dylib_address - old_dylib_address;
#ifdef LIBRARY_API
		if(dylib_vmslide != 0)
		    redo_prebinding_needed = TRUE;
#endif
	    }
	}

	/*
	 * First load the dynamic libraries this arch directly depends on
	 * allowing the time stamps not to match since we are redoing the
	 * prebinding for this arch.
	 */
	if(load_archs_libraries() == FALSE){
	    /*
	     * If we are allowing missing architectures and this one is allowed
	     * to be missing then load_archs_libraries() will return FALSE.
	     * If that happens we need to set it up so this arch is still
	     * written out but with out the prebinding info updated.
	     */
	    setup_symbolic_info(TRUE);
	    build_new_symbol_table(0, TRUE);
	    if(arch_swapped == TRUE)
		swap_arch_for_output();
	    return;
	}

	/*
	 * Now load the dependent libraries who's time stamps much match.
	 */
        if(load_dependent_libraries() == FALSE){
	    /*
	     * If we are allowing missing architectures and this one is allowed
	     * to be missing then load_archs_libraries() will return FALSE.
	     * If that happens we need to set it up so this arch is still
	     * written out but with out the prebinding info updated.
	     */
	    setup_symbolic_info(TRUE);
	    build_new_symbol_table(0, TRUE);
	    if(arch_swapped == TRUE)
		swap_arch_for_output();
	    return;
	}

	/*
	 * To deal with libsys, in that it has no dependent libs and it's
	 * prebinding can't be redone as indr(1) has been run on it and it has
	 * undefineds for __NXArgc, __NXArgv, and __environ from crt code,
	 * we return if the arch has no dependent libraries.
	 */
	if(nlibs == 0 && dylib_vmslide == 0){
	    return;
	}

	/*
	 * Before we use the symbolic information we may need to swap everything
	 * into the host byte sex and check to see if it is all valid.
	 */
	setup_symbolic_info(FALSE);

	/*
	 * Check for overlaping segments in case a library now overlaps this
	 * arch.  We assume that the checks for the dependent libraries made
	 * by the link editor when the where prebound is still vaild.
	 */
	check_for_overlapping_segments(dylib_vmslide);

	/*
         * If this arch is not a two-level image check to make sure symbols		 * are not overridden in dependent dylibs when so prebinding can be
	 * redone.
	 */
	if((mh_flags & MH_TWOLEVEL) != MH_TWOLEVEL)
	    check_for_dylib_override_symbols();

	/*
	 * Setup the initial list of undefined symbols from the arch being
	 * processed.
	 */
	setup_initial_undefined_list();
 
	/*
	 * Link in the needed modules from the dependent libraries based on the
	 * undefined symbols setup above.  This will check for multiply defined
	 * symbols.  Then allow undefined symbols to be checked for.
	 */
	link_in_need_modules();

	/*
	 * If we are only checking and the new_dylib_address to be checked for
	 * is non-zero and this is a dynamic library check the dynamic library
	 * for the correct address.
	 */
	if(check_only == TRUE && new_dylib_address != 0){
	    /*
	     * If the address of this dynamic shared library is not the same as 
	     * the new_dylib_address value exit(1) to indicate this needs to
	     * have it's prebinding redone.
	     */
	    if(old_dylib_address != new_dylib_address)
		redo_exit(1);
	}

	/*
	 * If check_only is set then load_library() checked the time stamps and
	 * did an exit(1) if they did not match.  So if we get here this arch
	 * has been checked so just return so the other archs can be checked.
	 */
	if(check_only == TRUE)
	    return;

	/*
	 * Now that is possible to redo the prebinding as all the above checks
	 * have been done.  So this arch will be processed so set arch_processed
	 * to indicate an output file needs to be created.
	 */
	arch_processed = TRUE;

	/*
	 * Now that is possible to redo the prebinding build a new symbol table
	 * using the new values for prebound undefined symbols.
	 */
	build_new_symbol_table(dylib_vmslide, FALSE);

	/*
	 * Setup seg1addr or segs_read_write_addr which is the offset values for
	 * relocation entries' r_address fields.
	 */ 
	setup_r_address_base();

	/*
	 * Using the dylib_vmslide update the local relocation entries.
	 */
	if(dylib_vmslide != 0)
	    update_local_relocs(dylib_vmslide);

	/*
	 * Using the new and old symbol table update the external relocation
	 * entries.
	 */
	update_external_relocs(dylib_vmslide);

	/*
	 * Using the new and old symbol table update the symbol pointers.
	 */
	update_symbol_pointers(dylib_vmslide);

	/*
	 * Using the new symbol table update the any stubs that have jump
	 * instructions that dyld will modify.
	 */
	update_self_modifying_stubs(dylib_vmslide);

	/*
	 * Update the time stamps in the LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB and
	 * LC_REEXPORT_DYLIB commands and update the LC_PREBOUND_DYLIB is this
	 * is an excutable.
	 */
	update_load_commands(dylib_vmslide);
	
	/*
	 * If this is arch has MH_PREBINDABLE set, unset it and set MH_PREBOUND
	 */
	if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE){
            if(arch->object->mh != NULL){
                arch->object->mh->flags &= ~MH_PREBINDABLE;
                arch->object->mh->flags |= MH_PREBOUND;
            }
	    else{
                arch->object->mh64->flags &= ~MH_PREBINDABLE;
                arch->object->mh64->flags |= MH_PREBOUND;
            }
	}

	/*
	 * If the arch is swapped swap it back for output.
	 */
	if(arch_swapped == TRUE)
	    swap_arch_for_output();
}

/*
 * unprebind_arch() takes one arch which is a prebound executable or dylib (or a
 * bundle) and removes certain prebinding information, such that two binaries
 * that are the same except for prebinding information are returned to an
 * identical form. Unprebound binaries are still usable (and prebindable) but
 * lack prebinding.   For bundles and non-prebound executables and dylibs, 
 * the only things that are changed are the zeroing of dylib time stamps and
 * versions, and the zeroing of the two-level hints - the MH_PREBINDABLE
 * flag is not added to these files.
 */
static
void
unprebind_arch(
void)
{
    uint32_t mh_flags;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;

#ifdef LIBRARY_API
	redo_prebinding_needed = TRUE;
#endif
	arch_force_flat_namespace = (mh_flags & MH_FORCE_FLAT) ==
				    MH_FORCE_FLAT;

	/* set up an arch_flag for this arch's object */
	arch_flag.cputype = arch->object->mh_cputype;
	arch_flag.cpusubtype = arch->object->mh_cpusubtype;
	set_arch_flag_name(&arch_flag);
	arch_name = arch_flag.name;

	if(debug == TRUE)
	    printf("%s: unprebinding file: %s (for architecture %s)\n",
		   progname, arch->file_name, arch_name);

	/*
	 * If this is a dynamic library get the existing address of the library.
	 * Calculate the value needed to move it to address zero.
	 */
	if(arch->object->mh_filetype == MH_DYLIB){
	    old_dylib_address = get_dylib_address();
	    new_dylib_address = 0;
            dylib_vmslide = new_dylib_address - old_dylib_address;
	}
	else{
	    dylib_vmslide = 0;
	}

	/* load symbolic information for this arch */
	setup_symbolic_info(TRUE);

	/* 
	 * update the load commands by clearing timestamps and removing 
	 * LC_PREBOUND_DYLIB commands.  Since this can set MH_ALLMODSBOUND
	 * in the mach header flags of the arch we need to re-read the flags.
	 */
	update_load_commands(dylib_vmslide);
        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;
	
	/*
	 * build the new symbol table for this arch, setting prebound
	 * symbols to undefined.
	 */
	build_new_symbol_table(dylib_vmslide, FALSE);
	
	/*
	 * if we are unprebinding for Panther we need to halt at this point
	 * for binaries that are not prebound.  For post-Panther
	 * OSes we continue.
	 */
	struct macosx_deployment_target deployment_version;
	get_macosx_deployment_target(&deployment_version);
	if(deployment_version.major >= 3 && deployment_version.major < 4){
	    /*
	     * If this is a bundle or a non-prebound executable or dylib
	     * then all we need to do is update the load commands and
	     * zero the hints, so we are done.
	     */
	    if((mh_flags & MH_PREBOUND) != MH_PREBOUND){
	        arch_processed = TRUE;
		if(arch_swapped == TRUE)
		    swap_arch_for_output();
                if(arch->object->mh != NULL)
                    arch->object->mh->flags |= MH_CANONICAL;
                else
                    arch->object->mh64->flags |= MH_CANONICAL;
		return;
	    }
	}

	/*
	 * setup the base address for the relocation entries.
	 */
	setup_r_address_base();
	
	/*
	 * if the dylib_vmslide is nonzero then we need to slide the
	 * local relocation entries back to zero.
	 */
	if(dylib_vmslide != 0)
	    update_local_relocs(dylib_vmslide);

	/*
	 * reset the external relocation entries to zero for those 
	 * symbols that were prebound.
	 */
	if((mh_flags & MH_CANONICAL) != MH_CANONICAL)
	    update_external_relocs(dylib_vmslide);
	
	/*
	 * set symbol pointers back to their original values by sliding,
	 * zeroing, or setting back to the unprebound values found in 
	 * their corresponding local relocation entries.
	 */
	reset_symbol_pointers(dylib_vmslide);

	/*
	 * set any stubs that have jump instructions that dyld will modify back
	 * to halt instructions.
	 */
	reset_self_modifying_stubs();

	arch_processed = TRUE;

	/*
	 * if we are unprebinding for Panther we need to continue to set
	 * this unconditionally, otherwise, we only set MH_PREBINDABLE
	 * for previously prebound binaries that get to this point
	 */
	if(deployment_version.major >= 3 && deployment_version.major < 4){
	    mh_flags &= ~MH_PREBOUND;
	    mh_flags |= MH_PREBINDABLE;
	}
	else{
	    if((mh_flags & MH_PREBOUND) == MH_PREBOUND){
	        mh_flags &= ~MH_PREBOUND;
		mh_flags |= MH_PREBINDABLE;
	    }
	}
	mh_flags |= MH_CANONICAL;

        /* Write back any changes */
        if(arch->object->mh != NULL)
            arch->object->mh->flags = mh_flags;
        else
            arch->object->mh64->flags = mh_flags;

	/*
	 * If the arch is swapped swap it back for output.
	 */
	if(arch_swapped == TRUE)
	    swap_arch_for_output();
}

/*
 * get_dylib_address() is called when the arch is an MH_DYLIB file and returns
 * the dynamic shared library's address.
 */
static
uint32_t
get_dylib_address(
void)
{
    uint32_t i, addr;
    struct load_command *lc;
    struct segment_command *sg;
    uint32_t ncmds;
    
	/*
	 * Get the address of the dynamic shared library which is the address of
	 * the first (not the lowest address) segment.
	 */
	addr = 0;
	sg = NULL;
	lc = arch->object->load_commands;
        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;
	for(i = 0; i < ncmds && sg == NULL; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		addr = sg->vmaddr;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	return(addr);
}

/*
 * load_archs_libraries() loads the libraries referenced by the image arch.
 * If we are allowing missing architectures then arch_cant_be_missing is set to
 * non-zero indicating the one that is not allowed to be missing.  If one other
 * than that is missing this returns FALSE else this returns TRUE.
 */
static
enum bool
load_archs_libraries(
void)
{
    uint32_t i, ndependent_images;
    struct load_command *lc, *load_commands;
    struct dylib_command *dl_load, *dl_id;
    uint32_t *dependent_images, *image_pointer;
    char *suffix;
    enum bool is_framework;
    uint32_t ncmds;
    
	load_commands = arch->object->load_commands;
	/*
	 * If arch_force_flat_namespace is false count the number of dependent
	 * images and allocate the image pointers for them.
	 */
	ndependent_images = 0;
	dependent_images = NULL;
        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;
	if(arch_force_flat_namespace == FALSE){
	    lc = load_commands;
	    for(i = 0; i < ncmds; i++){
		switch(lc->cmd){
		case LC_LOAD_DYLIB:
		case LC_LOAD_WEAK_DYLIB:
		case LC_REEXPORT_DYLIB:
		    ndependent_images++;
		    break;

		case LC_ID_DYLIB:
		    dl_id = (struct dylib_command *)lc;
		    arch_lib.file_name = arch->file_name;
		    arch_lib.dylib_name = (char *)dl_id +
					  dl_id->dylib.name.offset;
		    arch_lib.umbrella_name =
			guess_short_name(arch_lib.dylib_name, &is_framework,
					 &suffix);
		    if(is_framework == TRUE){
			arch_lib.name_size =
			    (uint32_t)strlen(arch_lib.umbrella_name);
		    }
		    else{
			if(arch_lib.umbrella_name != NULL){
			    arch_lib.library_name = arch_lib.umbrella_name;
			    arch_lib.umbrella_name = NULL;
			    arch_lib.name_size =
				(uint32_t)strlen(arch_lib.library_name);
			}
		    }
		    if(suffix != NULL)
			free(suffix);
		    break;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	    dependent_images = allocate(sizeof(uint32_t) *
					ndependent_images);
	    arch_lib.dependent_images = dependent_images;
	    arch_lib.ndependent_images = ndependent_images;
	}
	if(arch_lib.dylib_name == NULL){
	    arch_lib.dylib_name = "not a dylib";
	    arch_lib.file_name = arch->file_name;
	}

	lc = load_commands;
	ndependent_images = 0;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_LOAD_DYLIB ||
	       lc->cmd == LC_LOAD_WEAK_DYLIB ||
	       lc->cmd == LC_REEXPORT_DYLIB){
		if(dependent_images != NULL)
		    image_pointer = &(dependent_images[ndependent_images++]);
		else
		    image_pointer = NULL;
		dl_load = (struct dylib_command *)lc;
		if(load_library(arch->file_name, dl_load, FALSE,
				image_pointer) == FALSE)
		    return(FALSE);
	    }
	    if(lc->cmd == LC_ID_DYLIB && check_only == TRUE){
		dl_load = (struct dylib_command *)lc;
		if(load_library(arch->file_name, dl_load, TRUE, NULL) == FALSE)
		    return(FALSE);
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	return(TRUE);
}

/*
 * load_dependent_libraries() now that the libraries of the arch being are
 * loaded now load the dependent libraries who's time stamps much match.
 * If we are allowing missing architectures then arch_cant_be_missing is set to
 * non-zero indicating the one that is not allowed to be missing.  If one other
 * than that is missing this returns FALSE else this returns TRUE.
 */
static
enum bool
load_dependent_libraries(
void)
{
    uint32_t i, j, ndependent_images;
    struct load_command *lc;
    struct dylib_command *dl_load;
    uint32_t *dependent_images, *image_pointer;
    enum bool some_images_setup;
    uint32_t ncmds;
    
	for(i = 0; i < nlibs; i++){
	    if(debug == TRUE)
		printf("%s: loading libraries for library %s\n",
		       progname, libs[i].file_name);
	    /*
	     * If arch_force_flat_namespace is FALSE count the number of
	     * dependent images and allocate the image pointers for them.
	     */
	    ndependent_images = 0;
	    dependent_images = NULL;
	    if(arch_force_flat_namespace == FALSE){
		lc = libs[i].ofile->load_commands;
                if(libs[i].ofile->mh != NULL)
                    ncmds = libs[i].ofile->mh->ncmds;
                else
                    ncmds = libs[i].ofile->mh64->ncmds;            
		for(j = 0; j < ncmds; j++){
		    switch(lc->cmd){
		    case LC_LOAD_DYLIB:
		    case LC_LOAD_WEAK_DYLIB:
		    case LC_REEXPORT_DYLIB:
			ndependent_images++;
			break;
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		dependent_images = allocate(sizeof(uint32_t *) *
					    ndependent_images);
		libs[i].dependent_images = dependent_images;
		libs[i].ndependent_images = ndependent_images;
	    }

	    ndependent_images = 0;
	    lc = libs[i].ofile->load_commands;
            if(libs[i].ofile->mh != NULL)
                ncmds = libs[i].ofile->mh->ncmds;
            else
                ncmds = libs[i].ofile->mh64->ncmds;            
	    for(j = 0; j < ncmds; j++){
		if(lc->cmd == LC_LOAD_DYLIB ||
		   lc->cmd == LC_LOAD_WEAK_DYLIB ||
		   lc->cmd == LC_REEXPORT_DYLIB){
		    dl_load = (struct dylib_command *)lc;
		    if(dependent_images != NULL)
			image_pointer = &(dependent_images[
					  ndependent_images++]);
		    else
			image_pointer = NULL;
		    if(load_library(libs[i].ofile->file_name, dl_load, TRUE,
				    image_pointer) == FALSE)
			return(FALSE);
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }

	}

	/*
	 * To support the "primary" library concept each image that has
	 * sub-frameworks and sub-umbrellas has a sub_images list created for
	 * it for other libraries to search in for symbol names.
	 *
	 * These lists are set up after all the dependent libraries are loaded
	 * in the loops above.
	 */
	if(arch_force_flat_namespace == FALSE){
	    /*
	     * Now with all the libraries loaded and the dependent_images set up
	     * set up the sub_images for any library that does not have this set
	     * up yet.  Since sub_images include sub_umbrellas any image that
	     * has sub_umbrellas must have the sub_umbrella images set up first.
	     * To do this setup_sub_images() will return FALSE for an image that
	     * needed one of its sub_umbrellas set up and we will loop here
	     * until we get a clean pass with no more images needing setup.
	     */
	    do{
		some_images_setup = FALSE;
		if(arch_lib.sub_images_setup == FALSE){
		    some_images_setup |= setup_sub_images(&arch_lib,
							  arch->object->mh,
							  arch->object->mh64);
		}
		for(i = 0; i < nlibs; i++){
		    if(libs[i].sub_images_setup == FALSE){
			some_images_setup |= setup_sub_images(&(libs[i]),
							libs[i].ofile->mh,
							libs[i].ofile->mh64);
		    }
		}
	    }while(some_images_setup == TRUE);

	    /*
	     * If debug is set print out the lists.
	     */
	    if(debug == TRUE){
		if(arch_lib.two_level_debug_printed == FALSE){
		    print_two_level_info(&arch_lib);
		}
		arch_lib.two_level_debug_printed = TRUE;
		for(i = 0; i < nlibs; i++){
		    if(libs[i].two_level_debug_printed == FALSE){
			print_two_level_info(libs + i);
		    }
		    libs[i].two_level_debug_printed = TRUE;
		}
	    }
	}
	return(TRUE);
}

/*
 * print_two_level_info() prints out the info for two-level libs, the name,
 * umbrella_name, library_name, dependent_images and sub_images lists.
 */
static
void
print_two_level_info(
struct lib *lib)
{
    uint32_t j;
    uint32_t *sp;

	printf("two-level library: %s (file_name %s)",
	       lib->dylib_name, lib->file_name);
	if(lib->umbrella_name != NULL)
	    printf(" umbrella_name = %.*s\n",
	       (int)(lib->name_size),
	       lib->umbrella_name);
	else
	    printf(" umbrella_name = NULL\n");

	if(lib->library_name != NULL)
	    printf(" library_name = %.*s\n",
	       (int)(lib->name_size),
	       lib->library_name);
	else
	    printf(" library_name = NULL\n");

	printf("    ndependent_images = %u\n",
	       lib->ndependent_images);
	sp = lib->dependent_images;
	for(j = 0;
	    j < lib->ndependent_images;
	    j++){
	    if(libs[sp[j]].umbrella_name != NULL)
	       printf("\t[%u] %.*s\n", j,
		      (int)libs[sp[j]].name_size,
		      libs[sp[j]].umbrella_name);
	    else if(libs[sp[j]].library_name != NULL)
	       printf("\t[%u] %.*s\n", j,
		      (int)libs[sp[j]].name_size,
		      libs[sp[j]].library_name);
	    else
	       printf("\t[%u] %s (file_name %s)\n", j,
		      libs[sp[j]].dylib_name,
		      libs[sp[j]].file_name);
	}

	printf("    nsub_images = %u\n",
	       lib->nsub_images);
	sp = lib->sub_images;
	for(j = 0; j < lib->nsub_images; j++){
	    if(libs[sp[j]].umbrella_name != NULL)
	       printf("\t[%u] %.*s\n", j,
		      (int)libs[sp[j]].name_size,
		      libs[sp[j]].umbrella_name);
	    else if(libs[sp[j]].library_name != NULL)
	       printf("\t[%u] %.*s\n", j,
		      (int)libs[sp[j]].name_size,
		      libs[sp[j]].library_name);
	    else
	       printf("\t[%u] %s (file_name %s)\n", j,
		      libs[sp[j]].dylib_name,
		      libs[sp[j]].file_name);
	}
}

/*
 * setup_sub_images() is called to set up the sub images that make up the
 * specified "primary" lib.  If not all of its sub_umbrella's and sub_library's
 * are set up then it will return FALSE and not set up the sub images.  The
 * caller will loop through all the libraries until all libraries are setup.
 * This routine will return TRUE when it sets up the sub_images and will also
 * set the sub_images_setup field to TRUE in the specified library.
 */
static
enum bool
setup_sub_images(
struct lib *lib,
struct mach_header *lib_mh,
struct mach_header_64 *lib_mh64)
{
    uint32_t i, j, k, l, n, max_libraries;
    struct mach_header *mh;
    struct mach_header_64 *mh64;
    struct load_command *lc, *load_commands;
    struct sub_umbrella_command *usub;
    struct sub_library_command *lsub;
    struct sub_framework_command *sub;
    struct dylib_command *dl_load;
    uint32_t *deps;
    char *sub_umbrella_name, *sub_library_name, *sub_framework_name,
	 *dylib_name;
    enum bool found;
    uint32_t ncmds;
    
	max_libraries = 0;
	deps = lib->dependent_images;

	/*
	 * First see if this library has any sub-umbrellas or sub-libraries and
	 * that they have had their sub-images set up.  If not return FALSE and
	 * wait for this to be set up.  If so add the count of sub-images to
	 * max_libraries value which will be used for allocating the array for
	 * the sub-images of this library.
	 */
	if(lib_mh != NULL){
	    mh = lib_mh;
	    mh64 = NULL;
	    load_commands = (struct load_command *)((char *)mh +
						sizeof(struct mach_header));
	    ncmds = mh->ncmds;
	}
	else{
	    mh = NULL;
	    mh64 = lib_mh64;
	    load_commands = (struct load_command *)((char *)mh64 +
						sizeof(struct mach_header_64));
	    ncmds = mh64->ncmds;
	}
	lc = load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc->cmd){
	    case LC_SUB_UMBRELLA:
		usub = (struct sub_umbrella_command *)lc;
		sub_umbrella_name = (char *)usub + usub->sub_umbrella.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].umbrella_name != NULL &&
		       strncmp(sub_umbrella_name, libs[deps[j]].umbrella_name,
			       libs[deps[j]].name_size) == 0 &&
		       sub_umbrella_name[libs[deps[j]].name_size] == '\0'){
			/*
			 * TODO: can't this logic (here and in our caller) hang
		         * if there is a circular loop?  And is that even
			 * possible to create?  See comments in our caller.
			 */
			if(libs[deps[j]].sub_images_setup == FALSE)
			    return(FALSE);
			max_libraries += 1 + libs[deps[j]].nsub_images;
		    }
		}
		break;
	    case LC_SUB_LIBRARY:
		lsub = (struct sub_library_command *)lc;
		sub_library_name = (char *)lsub + lsub->sub_library.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].library_name != NULL &&
		       strncmp(sub_library_name, libs[deps[j]].library_name,
			       libs[deps[j]].name_size) == 0 &&
		       sub_library_name[libs[deps[j]].name_size] == '\0'){
			/*
			 * TODO: can't this logic (here and in our caller) hang
		         * if there is a circular loop?  And is that even
			 * possible to create?  See comments in our caller.
			 */
			if(libs[deps[j]].sub_images_setup == FALSE)
			    return(FALSE);
			max_libraries += 1 + libs[deps[j]].nsub_images;
		    }
		}
		break;
	    case LC_REEXPORT_DYLIB:
		dl_load = (struct dylib_command *)lc;
		dylib_name = (char *)dl_load + dl_load->dylib.name.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].dylib_name != NULL &&
		       strcmp(dylib_name, libs[deps[j]].dylib_name) == 0){
			/*
			 * TODO: can't this logic (here and in our caller) hang
		         * if there is a circular loop?  And is that even
			 * possible to create?  See comments in our caller.
			 */
			if(libs[deps[j]].sub_images_setup == FALSE)
			    return(FALSE);
			max_libraries += 1 + libs[deps[j]].nsub_images;
		    }
		}
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/*
	 * Allocate the sub-images array of indexes into the libs[] array that
	 * make up this "primary" library.  Allocate enough to handle the max
	 * and then this allocation will be reallocated with the actual needed
	 * size.
	 */
	max_libraries += lib->ndependent_images;
	lib->sub_images = allocate(sizeof(uint32_t) * max_libraries);
	n = 0;

	/*
	 * First add the dependent images which are sub-frameworks of this
	 * image to the sub images list.
	 */
	if(lib->umbrella_name != NULL){
	    for(i = 0; i < lib->ndependent_images; i++){
                if(libs[deps[i]].ofile->mh != NULL){
		    mh = libs[deps[i]].ofile->mh;
		    load_commands = (struct load_command *)
			((char *)(libs[deps[i]].ofile->mh) +
			 sizeof(struct mach_header));
		    lc = load_commands;
                    ncmds = libs[deps[i]].ofile->mh->ncmds;
		}
                else{
		    mh64 = libs[deps[i]].ofile->mh64;
		    load_commands = (struct load_command *)
			((char *)(libs[deps[i]].ofile->mh64) +
			 sizeof(struct mach_header_64));
		    lc = load_commands;
                    ncmds = libs[deps[i]].ofile->mh64->ncmds;
		}
		for(j = 0; j < ncmds; j++){
		    if(lc->cmd == LC_SUB_FRAMEWORK){
			sub = (struct sub_framework_command *)lc;
			sub_framework_name = (char *)sub + sub->umbrella.offset;
			if(lib->umbrella_name != NULL &&
			   strncmp(sub_framework_name,
			       lib->umbrella_name,
			       lib->name_size) == 0 &&
			   sub_framework_name[lib->name_size] =='\0'){
			    lib->sub_images[n++] = deps[i];
			}
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
	    }
	}

	/*
	 * Second add the sub-umbrella's and sub-library's sub-images to the
	 * sub images list.
	 */
	if(lib_mh != NULL){
	    mh = lib_mh;
	    mh64 = NULL;
	    load_commands = (struct load_command *)((char *)mh +
						sizeof(struct mach_header));
	    ncmds = mh->ncmds;
	}
	else{
	    mh = NULL;
	    mh64 = lib_mh64;
	    load_commands = (struct load_command *)((char *)mh64 +
						sizeof(struct mach_header_64));
	    ncmds = mh64->ncmds;
	}
	lc = load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc->cmd){
	    case LC_SUB_UMBRELLA:
		usub = (struct sub_umbrella_command *)lc;
		sub_umbrella_name = (char *)usub + usub->sub_umbrella.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].umbrella_name != NULL &&
		       strncmp(sub_umbrella_name, libs[deps[j]].umbrella_name,
			       libs[deps[j]].name_size) == 0 &&
		       sub_umbrella_name[libs[deps[j]].name_size] == '\0'){

			/* make sure this image is not already on the list */
			found = FALSE;
			for(l = 0; l < n; l++){
			    if(lib->sub_images[l] == deps[j]){
				found = TRUE;
				break;
			    }
			}
			if(found == FALSE)
			    lib->sub_images[n++] = deps[j];

			for(k = 0; k < libs[deps[j]].nsub_images; k++){
			    /* make sure this image is not already on the list*/
			    found = FALSE;
			    for(l = 0; l < n; l++){
				if(lib->sub_images[l] ==
				   libs[deps[j]].sub_images[k]){
				    found = TRUE;
				    break;
				}
			    }
			    if(found == FALSE)
				lib->sub_images[n++] = 
				    libs[deps[j]].sub_images[k];
			}
		    }
		}
		break;
	    case LC_SUB_LIBRARY:
		lsub = (struct sub_library_command *)lc;
		sub_library_name = (char *)lsub + lsub->sub_library.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].library_name != NULL &&
		       strncmp(sub_library_name, libs[deps[j]].library_name,
			       libs[deps[j]].name_size) == 0 &&
		       sub_library_name[libs[deps[j]].name_size] == '\0'){

			/* make sure this image is not already on the list */
			found = FALSE;
			for(l = 0; l < n; l++){
			    if(lib->sub_images[l] == deps[j]){
				found = TRUE;
				break;
			    }
			}
			if(found == FALSE)
			    lib->sub_images[n++] = deps[j];

			for(k = 0; k < libs[deps[j]].nsub_images; k++){
			    /* make sure this image is not already on the list*/
			    found = FALSE;
			    for(l = 0; l < n; l++){
				if(lib->sub_images[l] ==
				   libs[deps[j]].sub_images[k]){
				    found = TRUE;
				    break;
				}
			    }
			    if(found == FALSE)
				lib->sub_images[n++] = 
				    libs[deps[j]].sub_images[k];
			}
		    }
		}
		break;
	    case LC_REEXPORT_DYLIB:
		dl_load = (struct dylib_command *)lc;
		dylib_name = (char *)dl_load + dl_load->dylib.name.offset;
		for(j = 0; j < lib->ndependent_images; j++){
		    if(libs[deps[j]].dylib_name != NULL &&
		       strcmp(dylib_name, libs[deps[j]].dylib_name) == 0){

			/* make sure this image is not already on the list */
			found = FALSE;
			for(l = 0; l < n; l++){
			    if(lib->sub_images[l] == deps[j]){
				found = TRUE;
				break;
			    }
			}
			if(found == FALSE)
			    lib->sub_images[n++] = deps[j];

			for(k = 0; k < libs[deps[j]].nsub_images; k++){
			    /* make sure this image is not already on the list*/
			    found = FALSE;
			    for(l = 0; l < n; l++){
				if(lib->sub_images[l] ==
				   libs[deps[j]].sub_images[k]){
				    found = TRUE;
				    break;
				}
			    }
			    if(found == FALSE)
				lib->sub_images[n++] = 
				    libs[deps[j]].sub_images[k];
			}
		    }
		}
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	/*
	 * Now reallocate the sub-images of this library to the actual size
	 * needed for it.  Note this just gives back the pointers we don't
	 * use when allocated from the block of preallocated pointers.
	 */
	lib->sub_images = reallocate(lib->sub_images, sizeof(uint32_t) *n);
	lib->nsub_images = n;

	lib->sub_images_setup = TRUE;
	return(TRUE);
}

/*
 * load_library() loads the library for the dl_load command for the current
 * architecture being processed in indicated by arch_flag.   This library is
 * being loaded because file_name depends in it.  If time_stamps_must_match is
 * TRUE then this library is not a direct dependent of what we are redoing the
 * prebinding for it must be correct. Since we are now processing a valid 
 * file any errors in loading the library are fatal except if we are allowing
 * missing architectures.  If we are allowing missing architectures then
 * arch_cant_be_missing is set to non-zero indicating the one that is not
 * allowed to be missing.  If one other than that is missing this returns FALSE
 * else this returns TRUE.
 */
static
enum bool
load_library(
char *file_name,
struct dylib_command *dl_load,
enum bool time_stamps_must_match,
uint32_t *image_pointer)
{
    uint32_t i;
    char *dylib_name;
    struct ofile *ofile;
    struct fat_arch *best_fat_arch;
    struct fat_arch_64 *best_fat_arch64;
    struct load_command *lc;
    struct dylib_command *dl_id;
    enum bool already_loaded, is_framework;
    char *suffix;
    struct stat stat_buf;
    uint32_t ncmds;
    
	/* get the name of the library from the load command */
	dylib_name = (char *)dl_load + dl_load->dylib.name.offset;

	/* if this library is already loaded just return */
	already_loaded = FALSE;
	ofile = NULL;
	for(i = 0; i < nlibs; i++){
	    if(strcmp(libs[i].dylib_name, dylib_name) == 0){
		if(time_stamps_must_match == FALSE)
		    return(TRUE);
		already_loaded = TRUE;
		ofile = libs[i].ofile;
		dylib_name = libs[i].file_name;
		if(image_pointer != NULL)
		    *image_pointer = i;
		break;
	    }
	}
	if(already_loaded == FALSE){
	    if(debug == TRUE)
		printf("%s: loading library: %s\n", progname, dylib_name);

	    /*
	     * If an executable_path option is used and the dylib_name starts
	     * with "@executable_path" change "@executable_path" to the value
	     * of the executable_path option.
	     */
	    if(executable_path != NULL &&
	       strncmp(dylib_name, "@executable_path",
		       sizeof("@executable_path") - 1) == 0)
		dylib_name = makestr(executable_path,
		    dylib_name + sizeof("@executable_path") - 1, NULL);
	    /*
	     * If a root_dir option is used prepend the directory for rooted
	     * names.
	     */
	    if(root_dir != NULL && *dylib_name == '/')
		dylib_name = makestr(root_dir, dylib_name, NULL);

	    if(debug == TRUE &&
	       dylib_name != (char *)dl_load + dl_load->dylib.name.offset)
		printf("%s: library name now: %s\n", progname, dylib_name);

	    /*
	     * If this is a weak library it may not exist.  So if not return
	     * and set the image_pointer to WEAK_LIB to indicate this is not
	     * present.
	     */
	    if(dl_load->cmd == LC_LOAD_WEAK_DYLIB){
		if(stat(dylib_name, &stat_buf) == -1){
		    if(image_pointer != NULL)
			*image_pointer = WEAK_LIB;
		    return(TRUE);
		}
	    }
	    /*
	     * We may have seen this library by another name so check for
	     * that.
	     */ 
	    if(stat(dylib_name, &stat_buf) != -1){
		for(i = 0; i < nlibs; i++){
		    if(stat_buf.st_dev == libs[i].dev && 
		       stat_buf.st_ino == libs[i].ino){
			if(time_stamps_must_match == FALSE)
			    return(TRUE);
			already_loaded = TRUE;
			ofile = libs[i].ofile;
			dylib_name = libs[i].file_name;
			if(image_pointer != NULL)
			    *image_pointer = i;
			break;
		    }
		}
	    }

	    if(already_loaded == FALSE){
		ofile = allocate(sizeof(struct ofile));

		/* now map in the library for this architecture */
		if(ofile_map(dylib_name, NULL, NULL, ofile, FALSE) == FALSE)
		    redo_exit(2);
	    }
	}

	/*
	 * Check to make sure the ofile is a dynamic library and for the
	 * the correct architecture.
	 */
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		best_fat_arch64 = cpusubtype_findbestarch_64(
		    arch_flag.cputype,
		    arch_flag.cpusubtype,
		    ofile->fat_archs64,
		    ofile->fat_header->nfat_arch);
		best_fat_arch = NULL;
	    }
	    else{
		best_fat_arch = cpusubtype_findbestarch(
		    arch_flag.cputype,
		    arch_flag.cpusubtype,
		    ofile->fat_archs,
		    ofile->fat_header->nfat_arch);
		best_fat_arch64 = NULL;
	    }
	    if(best_fat_arch == NULL && best_fat_arch64 == NULL){
		/*
		 * If we are allowing missing architectures except one see if
		 * this is not the one that can't be missing.
		 */
		if(arch_cant_be_missing != 0 &&
		   arch_cant_be_missing != arch_flag.cputype){
		    if(already_loaded == FALSE)
			ofile_unmap(ofile);
		    return(FALSE);
		}
		error("dynamic shared library file: %s does not contain an "
		      "architecture that can be used with %s (architecture %s)",
		      dylib_name, file_name, arch_name);
		redo_exit(2);
	    }

	    (void)ofile_first_arch(ofile);
	    do{
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    if(best_fat_arch64 != ofile->fat_archs64 + ofile->narch)
			continue;
		}
		else{
		    if(best_fat_arch != ofile->fat_archs + ofile->narch)
			continue;
		}
		if(ofile->arch_type == OFILE_ARCHIVE){
		    error("file: %s (for architecture %s) is an archive (not "
			  "a Mach-O dynamic shared library)", dylib_name,
			  ofile->arch_flag.name);
		    redo_exit(2);
		}
		else if(ofile->arch_type == OFILE_Mach_O){
		    if(ofile->mh_filetype != MH_DYLIB){
			error("file: %s (for architecture %s) is not a Mach-O "
			      "dynamic shared library", dylib_name,
			      arch_name);
			redo_exit(2);
		    }
		    goto good;
		}
		else if(ofile->arch_type == OFILE_UNKNOWN){
		    error("file: %s (for architecture %s) is not a Mach-O "
			  "dynamic shared library", dylib_name, arch_name);
		    redo_exit(2);
		}
	    }while(ofile_next_arch(ofile) == TRUE);
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    error("file: %s is an archive (not a Mach-O dynamic shared "
		  "library)", dylib_name);
	    redo_exit(2);
	}
	else if(ofile->file_type == OFILE_Mach_O){
	    if(arch_flag.cputype != ofile->mh_cputype){
		/*
		 * If we are allowing missing architectures except one see if
		 * this is not the one that can't be missing.
		 */
		if(arch_cant_be_missing != 0 &&
		   arch_cant_be_missing != arch_flag.cputype){
		    if(already_loaded == FALSE)
			ofile_unmap(ofile);
		    return(FALSE);
		}
		error("dynamic shared library: %s has the wrong CPU type for: "
		      "%s (architecture %s)", dylib_name, file_name,
		      arch_name);
		redo_exit(2);
	    }
	    if(cpusubtype_combine(arch_flag.cputype,
		arch_flag.cpusubtype, ofile->mh_cpusubtype) == -1){
		/*
		 * If we are allowing missing architectures except one see if
		 * this is not the one that can't be missing.
		 */
		if(arch_cant_be_missing != 0 &&
		   arch_cant_be_missing != arch_flag.cputype){
		    if(already_loaded == FALSE)
			ofile_unmap(ofile);
		    return(FALSE);
		}
		error("dynamic shared library: %s has the wrong CPU subtype "
		      "for: %s (architecture %s)", dylib_name, file_name,
		      arch_name);
		redo_exit(2);
	    }
	}
	else{ /* ofile->file_type == OFILE_UNKNOWN */
	    error("file: %s is not a Mach-O dynamic shared library",
		  dylib_name);
	    redo_exit(2);
	}

good:
	/*
	 * At this point ofile is infact a dynamic library and the right part
	 * of the ofile selected for the arch passed into here.
	 */

        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;

	/*
	 * If the time stamps must match check for matching time stamps.
	 */
	if(time_stamps_must_match == TRUE
#ifdef LIBRARY_API
	   || check_if_needed == TRUE
#endif
	   ){
	    lc = ofile->load_commands;
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_ID_DYLIB){
		    dl_id = (struct dylib_command *)lc;
		    if(dl_load->dylib.timestamp != dl_id->dylib.timestamp){
#ifdef LIBRARY_API
			/*
			 * If we are allowing missing architectures except one
			 * see if this is not the one that can't be missing.
			 */
			if(arch_cant_be_missing != 0 &&
			   arch_cant_be_missing != arch_flag.cputype){
			    if(already_loaded == FALSE)
				ofile_unmap(ofile);
			    return(FALSE);
			}
			redo_prebinding_needed = TRUE;
#endif
			if(time_stamps_must_match == TRUE){
			    if(dl_load->cmd == LC_ID_DYLIB){
				error("library: %s (architecture %s) prebinding"
				      " not up to date with installed dynamic "
				      " shared library: %s", file_name,
				      arch_name, dylib_name);
				redo_exit(1);
			    }
			    else{
				error("library: %s (architecture %s) prebinding"
				      " not up to date with dependent dynamic "
				      "shared library: %s", file_name,arch_name,
				      dylib_name);
				redo_exit(3);
			    }
			}
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}
	/*
	 * When the time stamps don't need to match we are processing the
	 * arch we are trying to redo the prebinding for.  So if we are just
	 * checking then see if the time stamps are out of date and if so
	 * exit(1) to indicate this needs to have it's prepinding redone.
	 */
	else if(check_only == TRUE){
	    lc = ofile->load_commands;
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_ID_DYLIB){
		    dl_id = (struct dylib_command *)lc;
		    if(dl_load->dylib.timestamp != dl_id->dylib.timestamp){
			redo_exit(1);
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}

	/*
	 * To allow the check_only to check the installed library load_library()
	 * can be called with an LC_ID_DYLIB for the install library to check
	 * its time stamp.  This is not put into the list however as it would
	 * overlap.
	 */
	if(already_loaded == FALSE &&
	   (dl_load->cmd == LC_LOAD_DYLIB ||
	    dl_load->cmd == LC_LOAD_WEAK_DYLIB ||
	    dl_load->cmd == LC_REEXPORT_DYLIB)){
	    /*
	     * Add this library's ofile to the list of libraries the current
	     * arch depends on.
	     */
	    libs = reallocate(libs, (nlibs + 1) * sizeof(struct lib));
	    memset(libs + nlibs, '\0', sizeof(struct lib));
	    libs[nlibs].file_name = dylib_name;
	    /*
	     * We must get the install_name as the name in the LC_DYLIB_LOAD
	     * command may not be the install_name.
	     */
	    lc = ofile->load_commands;
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_ID_DYLIB){
		    dl_id = (struct dylib_command *)lc;
		    libs[nlibs].dylib_name = (char *)dl_id +
					     dl_id->dylib.name.offset;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	    libs[nlibs].umbrella_name = guess_short_name(libs[nlibs].dylib_name,
							 &is_framework,
							 &suffix);
	    if(is_framework == TRUE){
		libs[nlibs].name_size =
		    (uint32_t)strlen(libs[nlibs].umbrella_name);
	    }
	    else{
		if(libs[nlibs].umbrella_name != NULL){
		    libs[nlibs].library_name = libs[nlibs].umbrella_name;
		    libs[nlibs].umbrella_name = NULL;
		    libs[nlibs].name_size =
			(uint32_t)strlen(libs[nlibs].library_name);
		}
	    }
	    if(suffix != NULL)
		free(suffix);
	    libs[nlibs].ofile = ofile;
	    libs[nlibs].dev = stat_buf.st_dev;
	    libs[nlibs].ino = stat_buf.st_ino;
	    if(image_pointer != NULL)
		*image_pointer = nlibs;
	    nlibs++;
	}
	return(TRUE);
}

/*
 * check_for_overlapping_segments() checks to make sure the segments in the
 * arch and all the dependent libraries do not overlap.  If they do the
 * prebinding can't be redone.  If vmslide is not zero it is the amount the
 * arch will be slid.
 */
static
void
check_for_overlapping_segments(
uint32_t vmslide)
{
    uint32_t i, j;
    struct segment *segments;
    uint32_t nsegments;
    struct load_command *lc;
    struct segment_command *sg;
    uint32_t ncmds;
    
	segments = NULL;
	nsegments = 0;

	/* put each segment of the arch in the segment list */
	lc = arch->object->load_commands;
        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		segments = reallocate(segments, 
				      (nsegments + 1) * sizeof(struct segment));
		segments[nsegments].file_name = arch->file_name;
		segments[nsegments].sg = *sg;
		segments[nsegments].sg.vmaddr += vmslide;
		nsegments++;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/* put each segment of each library in the segment list */
	for(i = 0; i < nlibs; i++){
	    lc = libs[i].ofile->load_commands;
            if(libs[i].ofile->mh != NULL)
                ncmds = libs[i].ofile->mh->ncmds;
            else
                ncmds = libs[i].ofile->mh64->ncmds;
	    for(j = 0; j < ncmds; j++){
		if(lc->cmd == LC_SEGMENT){
		    sg = (struct segment_command *)lc;
		    segments = reallocate(segments, 
				      (nsegments + 1) * sizeof(struct segment));
		    segments[nsegments].file_name = libs[i].file_name;
		    segments[nsegments].sg = *sg;
		    nsegments++;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}

	/* check each segment against all others */
	for(i = 0; i < nsegments; i++){
	    for(j = i + 1; j < nsegments; j++){
		check_overlap(segments + i, segments + j);
	    }
	}
}

/*
 * check_overlap() checks if the two segments passed to it overlap and if so
 * prints an error message and exit with a value of 2 indicating the prebinding
 * can't be redone.
 */
static
void
check_overlap(
struct segment *s1,
struct segment *s2)
{
	if(s1->sg.vmsize == 0 || s2->sg.vmsize == 0)
	    return;

	if(s1->sg.vmaddr > s2->sg.vmaddr){
	    if(s2->sg.vmaddr + s2->sg.vmsize <= s1->sg.vmaddr)
		return;
	}
	else{
	    if(s1->sg.vmaddr + s1->sg.vmsize <= s2->sg.vmaddr)
		return;
	}
	error("prebinding can't be redone because %.16s segment (address = 0x%x"
	      " size = 0x%x) of %s overlaps with %.16s segment (address = 0x%x "
	      "size = 0x%x) of %s (for architecture %s)",
	      s1->sg.segname, (unsigned int)(s1->sg.vmaddr),
	      (unsigned int)(s1->sg.vmsize), s1->file_name,
	      s2->sg.segname, (unsigned int)(s2->sg.vmaddr),
	      (unsigned int)(s2->sg.vmsize), s2->file_name,
	      arch_name);
	redo_exit(2);
}

/*
 * setup_symbolic_info() sets up all the symbolic info in the arch and loaded
 * libraries by swapping it into the host bytesex if needed and checking it to
 * be valid.  If we are allowing missing architecures and some of the dependent
 * libraries are missing then missing_arch is TRUE and we set it up so that
 * this arch is still written out but with out the prebinding info updated.
 */
static
void
setup_symbolic_info(
enum bool missing_arch)
{
    uint32_t i, j, nlibrefs;
    enum byte_sex host_byte_sex;
    struct load_command *lc;
    uint32_t ncmds;
    
	host_byte_sex = get_host_byte_sex();
	arch_swapped = arch->object->object_byte_sex != host_byte_sex;

	if(arch->object->st == NULL){
	    error("malformed file: %s (no LC_SYMTAB load command) (for"
		  " architecture %s)", arch->file_name, arch_name);
	    redo_exit(2);
	}
	if(arch->object->dyst == NULL){
	    error("malformed file: %s (no LC_DYSYMTAB load command) (for"
		  " architecture %s)", arch->file_name, arch_name);
	    redo_exit(2);
	}

	arch_nsyms = arch->object->st->nsyms;
	if(arch->object->mh != NULL){
	    arch_symbols = (struct nlist *)(arch->object->object_addr +
				       arch->object->st->symoff);
	    arch_symbols64 = NULL;
	    if(arch_swapped == TRUE)
		swap_nlist(arch_symbols, arch_nsyms, host_byte_sex);
	}
	else{
	    arch_symbols = NULL;
	    arch_symbols64 = (struct nlist_64 *)(arch->object->object_addr +
				       arch->object->st->symoff);
	    if(arch_swapped == TRUE)
		swap_nlist_64(arch_symbols64, arch_nsyms, host_byte_sex);
	}

	arch_strings = arch->object->object_addr + arch->object->st->stroff;
	arch_strsize = arch->object->st->strsize;

	if(arch->object->hints_cmd != NULL &&
	   arch->object->hints_cmd->nhints != 0){
	    arch_hints = (struct twolevel_hint *)
		    (arch->object->object_addr +
		     arch->object->hints_cmd->offset);
	    arch_nhints = arch->object->hints_cmd->nhints;
	    if(arch_swapped == TRUE)
		swap_twolevel_hint(arch_hints, arch_nhints, host_byte_sex);
	}

	arch_extrelocs = (struct relocation_info *)
		(arch->object->object_addr +
		 arch->object->dyst->extreloff);
	arch_nextrel = arch->object->dyst->nextrel;
	if(arch_swapped == TRUE)
	    swap_relocation_info(arch_extrelocs, arch_nextrel, host_byte_sex);

	arch_locrelocs = (struct relocation_info *)
		(arch->object->object_addr +
		 arch->object->dyst->locreloff);
	arch_nlocrel = arch->object->dyst->nlocrel;
	if(arch_swapped == TRUE)
	    swap_relocation_info(arch_locrelocs, arch_nlocrel, host_byte_sex);

	arch_indirect_symtab = (uint32_t *)
		(arch->object->object_addr +
		 arch->object->dyst->indirectsymoff);
	arch_nindirectsyms = arch->object->dyst->nindirectsyms;
	if(arch_swapped == TRUE)
	    swap_indirect_symbols(arch_indirect_symtab, arch_nindirectsyms,
		host_byte_sex);
	
	if(arch->object->mh_filetype == MH_DYLIB){
	    arch_tocs = (struct dylib_table_of_contents *)
		    (arch->object->object_addr +
		     arch->object->dyst->tocoff);
	    arch_ntoc = arch->object->dyst->ntoc;
	    if(arch->object->mh != NULL){
		arch_mods = (struct dylib_module *)
			(arch->object->object_addr +
			 arch->object->dyst->modtaboff);
		arch_mods64 = NULL;
	    }
	    else{
		arch_mods = NULL;
		arch_mods64 = (struct dylib_module_64 *)
			(arch->object->object_addr +
			 arch->object->dyst->modtaboff);
	    }
	    arch_nmodtab = arch->object->dyst->nmodtab;
	    arch_refs = (struct dylib_reference *)
		    (arch->object->object_addr +
		     arch->object->dyst->extrefsymoff);
	    arch_nextrefsyms = arch->object->dyst->nextrefsyms;
	    if(arch_swapped == TRUE){
		swap_dylib_table_of_contents(
		    arch_tocs, arch_ntoc, host_byte_sex);
		if(arch->object->mh != NULL)
		    swap_dylib_module(
			arch_mods, arch_nmodtab, host_byte_sex);
		else
		    swap_dylib_module_64(
			arch_mods64, arch_nmodtab, host_byte_sex);
		swap_dylib_reference(
		    arch_refs, arch_nextrefsyms, host_byte_sex);
	    }
	}
	else{
	    arch_tocs = NULL;
	    arch_ntoc = 0;;
	    arch_mods = NULL;
	    arch_mods64 = NULL;
	    arch_nmodtab = 0;;
	    arch_refs = NULL;
	    arch_nextrefsyms = 0;;
	}
	nlibrefs = 0;
	lc = arch->object->load_commands;
        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_LOAD_DYLIB ||
	       lc->cmd == LC_LOAD_WEAK_DYLIB ||
	       lc->cmd == LC_REEXPORT_DYLIB){
		nlibrefs++;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	check_symbolic_info_tables(
	    arch->file_name,
	    arch->object->mh,
	    arch->object->mh64,
	    nlibrefs,
	    arch->object->st,
	    arch->object->dyst,
	    arch_symbols,
	    arch_symbols64,
	    arch_nsyms,
	    arch_strings,
	    arch_strsize,
	    arch_tocs,
	    arch_ntoc,
	    arch_mods,
	    arch_mods64,
	    arch_nmodtab,
	    arch_refs,
	    arch_nextrefsyms);

	/*
	 * If we are simply setting up the symbolic info for an arch that has
	 * missing architecures for its dependent libraries we are done and
	 * can return.
	 */
	if(missing_arch == TRUE)
	    return;

	/*
	 * Get all the symbolic info for the libraries the arch uses in the
	 * correct byte sex.
	 */
	for(i = 0; i < nlibs; i++){
	    libs[i].st = NULL;
	    libs[i].dyst = NULL;
	    nlibrefs = 0;
	    lc = libs[i].ofile->load_commands;
            if(libs[i].ofile->mh != NULL)
                ncmds = libs[i].ofile->mh->ncmds;
            else
                ncmds = libs[i].ofile->mh64->ncmds;
	    for(j = 0; j < ncmds; j++){
		if(lc->cmd == LC_SYMTAB){
		    if(libs[i].st != NULL){
			error("malformed library: %s (more than one LC_SYMTAB "
			      "load command) (for architecture %s)",
			      libs[i].file_name, arch_name);
			redo_exit(2);
		    }
		    libs[i].st = (struct symtab_command *)lc;
		}
		else if(lc->cmd == LC_DYSYMTAB){
		    if(libs[i].dyst != NULL){
			error("malformed library: %s (more than one LC_DYSYMTAB"
			      " load command) (for architecture %s)",
			      libs[i].file_name, arch_name);
			redo_exit(2);
		    }
		    libs[i].dyst = (struct dysymtab_command *)lc;
		}
		else if(lc->cmd == LC_ROUTINES){
		    if(libs[i].rc != NULL){
			error("malformed library: %s (more than one LC_ROUTINES"
			      " load command) (for architecture %s)",
			      libs[i].file_name, arch_name);
			redo_exit(2);
		    }
		    libs[i].rc = (struct routines_command *)lc;
		}
		else if(lc->cmd == LC_LOAD_DYLIB ||
			lc->cmd == LC_LOAD_WEAK_DYLIB ||
			lc->cmd == LC_REEXPORT_DYLIB){
		    nlibrefs++;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	    if(libs[i].st == NULL){
		error("malformed file: %s (no LC_SYMTAB load command) (for"
		      " architecture %s)", libs[i].file_name, arch_name);
		redo_exit(2);
	    }
	    if(libs[i].dyst == NULL){
		error("malformed file: %s (no LC_DYSYMTAB load command) (for"
		      " architecture %s)", libs[i].file_name, arch_name);
		redo_exit(2);
	    }
	    if(libs[i].ofile->mh != NULL){
		libs[i].symbols = (struct nlist *)
			(libs[i].ofile->object_addr + libs[i].st->symoff);
		libs[i].symbols64 = NULL;
		libs[i].mods = (struct dylib_module *)
			(libs[i].ofile->object_addr + libs[i].dyst->modtaboff);
		libs[i].mods64 = NULL;
	    }
	    else{
		libs[i].symbols = NULL;
		libs[i].symbols64 = (struct nlist_64 *)
			(libs[i].ofile->object_addr + libs[i].st->symoff);
		libs[i].mods = NULL;
		libs[i].mods64 = (struct dylib_module_64 *)
			(libs[i].ofile->object_addr + libs[i].dyst->modtaboff);
	    }
	    libs[i].nsyms = libs[i].st->nsyms;
	    libs[i].strings = libs[i].ofile->object_addr + libs[i].st->stroff;
	    libs[i].strsize = libs[i].st->strsize;
	    libs[i].tocs = (struct dylib_table_of_contents *)
		    (libs[i].ofile->object_addr + libs[i].dyst->tocoff);
	    libs[i].ntoc = libs[i].dyst->ntoc;
	    libs[i].nmodtab = libs[i].dyst->nmodtab;
	    libs[i].refs = (struct dylib_reference *)
		    (libs[i].ofile->object_addr +
		     libs[i].dyst->extrefsymoff);
	    libs[i].nextrefsyms = libs[i].dyst->nextrefsyms;
	    if(arch_swapped == TRUE){
		if(libs[i].ofile->mh != NULL){
		    swap_nlist(
		       libs[i].symbols, libs[i].nsyms, host_byte_sex);
		    swap_dylib_module(
			libs[i].mods, libs[i].nmodtab, host_byte_sex);
		}
		else{
		    swap_nlist_64(
		       libs[i].symbols64, libs[i].nsyms, host_byte_sex);
		    swap_dylib_module_64(
			libs[i].mods64, libs[i].nmodtab, host_byte_sex);
		}
		swap_dylib_table_of_contents(
		    libs[i].tocs, libs[i].ntoc, host_byte_sex);
		swap_dylib_reference(
		    libs[i].refs, libs[i].nextrefsyms, host_byte_sex);
	    }
	    check_symbolic_info_tables(
		libs[i].file_name,
		libs[i].ofile->mh,
		libs[i].ofile->mh64,
		nlibrefs,
		libs[i].st,
		libs[i].dyst,
		libs[i].symbols,
		libs[i].symbols64,
		libs[i].nsyms,
		libs[i].strings,
		libs[i].strsize,
		libs[i].tocs,
		libs[i].ntoc,
		libs[i].mods,
		libs[i].mods64,
		libs[i].nmodtab,
		libs[i].refs,
		libs[i].nextrefsyms);
	    libs[i].module_states = allocate(libs[i].nmodtab *
					     sizeof(enum link_state));
	    memset(libs[i].module_states, '\0',
		   libs[i].nmodtab * sizeof(enum link_state));
	}
}

static
void
swap_arch_for_output(
void)
{
	if(arch_symbols != NULL)
	    swap_nlist(arch_symbols, arch_nsyms,
		arch->object->object_byte_sex);
	if(arch_symbols64 != NULL)
	    swap_nlist_64(arch_symbols64, arch_nsyms,
		arch->object->object_byte_sex);
	swap_relocation_info(arch_extrelocs, arch_nextrel,
	    arch->object->object_byte_sex);
	swap_relocation_info(arch_locrelocs, arch_nlocrel,
	    arch->object->object_byte_sex);
	swap_indirect_symbols(arch_indirect_symtab, arch_nindirectsyms,
	    arch->object->object_byte_sex);
	swap_dylib_table_of_contents(arch_tocs, arch_ntoc,
	    arch->object->object_byte_sex);
	if(arch_mods != NULL)
	    swap_dylib_module(arch_mods, arch_nmodtab,
		arch->object->object_byte_sex);
	if(arch_mods64 != NULL)
	    swap_dylib_module_64(arch_mods64, arch_nmodtab,
		arch->object->object_byte_sex);
	swap_dylib_reference(arch_refs, arch_nextrefsyms,
	    arch->object->object_byte_sex);
	swap_twolevel_hint(arch_hints, arch_nhints,
	    arch->object->object_byte_sex);
}

/*
 * check_symbolic_info_tables() checks to see that the parts of the symbolic
 * info used to redo the prebinding is valid.
 */
static
void
check_symbolic_info_tables(
char *file_name,
struct mach_header *mh,
struct mach_header_64 *mh64,
uint32_t nlibrefs,
struct symtab_command *st,
struct dysymtab_command *dyst,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsyms,
char *strings,
uint32_t strsize,
struct dylib_table_of_contents *tocs,
uint32_t ntoc,
struct dylib_module *mods,
struct dylib_module_64 *mods64,
uint32_t nmodtab,
struct dylib_reference *refs,
uint32_t nextrefsyms)
{
    uint32_t i;
    uint32_t mh_flags, mh_filetype, n_strx, module_name, nextdefsym, iextdefsym;
    uint8_t n_type;
    uint64_t n_value;
    uint16_t n_desc;

	if(mh != NULL){
	    mh_filetype = mh->filetype;
	    mh_flags = mh->flags;
	}
	else{
	    mh_filetype = mh64->filetype;
	    mh_flags = mh64->flags;
	}
	/*
	 * Check the symbol table's offsets into the string table and the
	 * library ordinals.
	 */
	for(i = 0; i < nsyms; i++){
	    if(mh != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
	        n_value = symbols[i].n_value;
		n_desc = symbols[i].n_desc;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
	        n_value = symbols64[i].n_value;
		n_desc = symbols64[i].n_desc;
	    }
	    if(n_strx > strsize){
		error("malformed file: %s (bad string table index (%d) for "
		      "symbol %u) (for architecture %s)", file_name,
		      n_strx, i, arch_name);
		redo_exit(2);
	    }
	    if((n_type & N_TYPE) == N_INDR && n_value > strsize){
		error("malformed file: %s (bad string table index (%llu) for "
		      "N_INDR symbol %u) (for architecture %s)", file_name,
		      n_value, i, arch_name);
		redo_exit(2);
	    }
	    if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
		(n_type & N_STAB) == 0 &&
		(n_type & N_TYPE) == N_PBUD){
		if(mh_filetype == MH_DYLIB){
		    if(GET_LIBRARY_ORDINAL(n_desc) !=
			   SELF_LIBRARY_ORDINAL &&
		       (uint32_t)GET_LIBRARY_ORDINAL(n_desc)
			   - 1 > nlibrefs){
			error("malformed file: %s (bad LIBRARY_ORDINAL (%d) "
			      "for symbol %u %s) (for architecture %s)",
			      file_name, GET_LIBRARY_ORDINAL(n_desc),
			       i, strings + n_strx, arch_name);
			redo_exit(2);
		    }
		}
		else if(mh_filetype == MH_EXECUTE){
		   if(GET_LIBRARY_ORDINAL(n_desc) ==
			   SELF_LIBRARY_ORDINAL ||
		      (uint32_t)GET_LIBRARY_ORDINAL(n_desc)
			- 1 > nlibrefs){
			error("malformed file: %s (bad LIBRARY_ORDINAL (%d) "
			      "for symbol %u %s) (for architecture %s)",
			      file_name, GET_LIBRARY_ORDINAL(n_desc),
			       i, strings + n_strx, arch_name);
			redo_exit(2);
		    }
		}
	    }
	}

	/* check toc's symbol and module indexes */
	for(i = 0; i < ntoc; i++){
	    if(tocs[i].symbol_index > nsyms){
		error("malformed file: %s (bad symbol table index (%d) for "
		      "table of contents entry %u) (for architecture %s)",
		      file_name, tocs[i].symbol_index, i, arch_name);
		redo_exit(2);
	    }
	    if(tocs[i].module_index > nmodtab){
		error("malformed file: %s (bad module table index (%d) for "
		      "table of contents entry %u) (for architecture %s)",
		      file_name, tocs[i].module_index, i, arch_name);
		redo_exit(2);
	    }
	}

	/* check module table's string index for module names */
	for(i = 0; i < nmodtab; i++){
	    if(mh != NULL){
		module_name = mods[i].module_name;
		nextdefsym = mods[i].nextdefsym;
		iextdefsym = mods[i].iextdefsym;
		nextdefsym = mods[i].nextdefsym;
		iextdefsym = mods[i].iextdefsym;
	    }
	    else{
		module_name = mods64[i].module_name;
		nextdefsym = mods64[i].nextdefsym;
		iextdefsym = mods64[i].iextdefsym;
		nextdefsym = mods[i].nextdefsym;
		iextdefsym = mods[i].iextdefsym;
	    }
	    if(module_name > strsize){
		error("malformed file: %s (bad string table index (%d) for "
		      "module_name in module table entry %u ) (for "
		      "architecture %s)", file_name, module_name, i,
		      arch_name);
		redo_exit(2);
	    }
	    if(nextdefsym != 0 &&
	       (iextdefsym < dyst->iextdefsym ||
	        iextdefsym >= dyst->iextdefsym + dyst->nextdefsym)){
		error("malformed file: %s (bad external symbol table index for"
		      " for module table entry %u) (for architecture %s)",
		      file_name, i, arch_name);
		redo_exit(2);
	    }
	    if(nextdefsym != 0 &&
	       iextdefsym + nextdefsym > dyst->iextdefsym + dyst->nextdefsym){
		error("malformed file: %s (bad number of external symbol table"
		      " entries for module table entry %u) (for architecture "
		      "%s)", file_name, i, arch_name);
		redo_exit(2);
	    }
	}

	/* check refernce table's symbol indexes */
	for(i = 0; i < nextrefsyms; i++){
	    if(refs[i].isym > nsyms){
		error("malformed file: %s (bad external symbol table index "
		      "reference table entry %u) (for architecture %s)",
		      file_name, i, arch_name);
		redo_exit(2);
	    }
	}
}

/*
 * check_for_dylib_override_symbols() checks to make sure symbols in this arch
 * are not overriding symbols in dependent dylibs which the dependent library
 * also uses.  This is to verify prebinding can be redone.
 */
static
void
check_for_dylib_override_symbols(
void)
{
    uint32_t i;

	for(i = arch->object->dyst->iextdefsym;
	    i < arch->object->dyst->iextdefsym + arch->object->dyst->nextdefsym;
	    i++){
	    check_dylibs_for_definition(
		arch->file_name, arch_strings + arch_symbols[i].n_un.n_strx);
	}
}

/*
 * check_dylibs_for_definition() checks to see if the symbol name is defined
 * in any of the dependent dynamic shared libraries.  If it is a an error
 * message is printed and exit(2) is done to indicate the prebinding can't be
 * redone.
 */
static
void
check_dylibs_for_definition(
char *file_name,
char *symbol_name)
{
    uint32_t i;
    struct dylib_table_of_contents *toc;
    uint32_t mh_flags;
    
	for(i = 0; i < nlibs; i++){
	    /*
	     * If the library is a two-level namespace library then there is
	     * no problem defining the same symbols in it.
	     */
            if(libs[i].ofile->mh != NULL)
                mh_flags = libs[i].ofile->mh->flags;
            else 
                mh_flags = libs[i].ofile->mh64->flags;
	    if(arch_force_flat_namespace == FALSE &&
	       (mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL)
		continue;
	    bsearch_strings = libs[i].strings;
	    bsearch_symbols = libs[i].symbols;
	    toc = bsearch(symbol_name, libs[i].tocs, libs[i].ntoc,
			  sizeof(struct dylib_table_of_contents),
			  (int (*)(const void *, const void *))dylib_bsearch);
	    if(toc != NULL){
		/*
		 * There is a module that defineds this symbol.  If this
		 * symbol is also referenced by the libraries then we
		 * can't redo the prebindng.
		 */
		if(check_dylibs_for_reference(symbol_name) == TRUE){
		    error("prebinding can't be redone because of symbols "
		       "overridden in dependent dynamic shared libraries (%s "
		       "defined in: %s and in %s(%s)) (for architecture %s)",
		       symbol_name, file_name, libs[i].file_name,
		       libs[i].strings +
			    libs[i].mods[toc->module_index].module_name,
		       arch_name);
		    redo_exit(2);
		}
	    }
	}
}

/*
 * check_dylibs_for_reference() checks the flat namespace dependent dynamic
 * shared libraries to see if the specified merged symbol is referenced.  If it
 * is TRUE is returned else FALSE is returned.
 */
static
enum bool
check_dylibs_for_reference(
char *symbol_name)
{
    uint32_t i, j, symbol_index;
    struct dylib_table_of_contents *toc;
    struct nlist *symbol;
    uint32_t mh_flags;
    
	for(i = 0; i < nlibs; i++){
	    /*
	     * If the library is a two-level namespace library then there is
	     * no problem defining the same symbols in it.
	     */
            if(libs[i].ofile->mh != NULL)
                mh_flags = libs[i].ofile->mh->flags;
            else 
                mh_flags = libs[i].ofile->mh64->flags;
	    if(arch_force_flat_namespace == FALSE &&
	       (mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL)
		continue;
	    /*
	     * See if this symbol appears at all (defined or undefined)
	     * in this library.
	     */
	    bsearch_strings = libs[i].strings;
	    bsearch_symbols = libs[i].symbols;
	    toc = bsearch(symbol_name, libs[i].tocs, libs[i].ntoc,
			  sizeof(struct dylib_table_of_contents),
			  (int (*)(const void *, const void *))dylib_bsearch);
	    if(toc != NULL){
		symbol_index = toc->symbol_index;
	    }
	    else{
		symbol = bsearch(symbol_name,
			 libs[i].symbols + libs[i].dyst->iundefsym,
			 libs[i].dyst->nundefsym,
			 sizeof(struct nlist),
			 (int (*)(const void *,const void *))nlist_bsearch);
		if(symbol == NULL)
		    continue;
		symbol_index = (uint32_t)(symbol - libs[i].symbols);
	    }
	    /*
	     * The symbol appears in this library.  Now see if it is
	     * referenced by a module in the library.
	     */
	    for(j = 0; j < libs[i].nextrefsyms; j++){
		if(libs[i].refs[j].isym == symbol_index &&
		   (libs[i].refs[j].flags ==
			REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
		    libs[i].refs[j].flags ==
			REFERENCE_FLAG_UNDEFINED_LAZY))
		return(TRUE);
	    }
	}
	return(FALSE);
}

/*
 * Function for bsearch() for finding a symbol name in a dylib table of
 * contents.
 */
static
int
dylib_bsearch(
const char *symbol_name,
const struct dylib_table_of_contents *toc)
{
        return(strcmp(symbol_name,
                      bsearch_strings +
		      bsearch_symbols[toc->symbol_index].n_un.n_strx));
}

/*
 * Function for bsearch() for finding a symbol name in the sorted list of
 * undefined symbols.
 */
static
int
nlist_bsearch(
const char *symbol_name,
const struct nlist *symbol)
{
	return(strcmp(symbol_name, bsearch_strings + symbol->n_un.n_strx));
}

/*
 * setup_initial_undefined_list() builds the initial list of undefined symbol
 * references based on the arch's undefined symbols.
 */
static
void
setup_initial_undefined_list(
void)
{
    uint32_t i;
    uint32_t mh_flags;
    
	for(i = arch->object->dyst->iundefsym;
	    i < arch->object->dyst->iundefsym + arch->object->dyst->nundefsym;
	    i++){
		add_to_undefined_list(
		    arch_strings + arch_symbols[i].n_un.n_strx,
		    arch_symbols + i,
		    ARCH_LIB);
                /*
                 * If this binary was unprebound, then we have to reset all
                 * undefined symbols to type N_PBUD because they were unset
                 * during the unprebinding process.
                 */
                if(arch->object->mh != NULL)
                    mh_flags = arch->object->mh->flags;
                else
                    mh_flags = arch->object->mh64->flags;
                if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE){
                    arch_symbols[i].n_type &= ~N_TYPE;
                    arch_symbols[i].n_type |= N_PBUD;
                }
	}
}

/*
 * link_in_need_modules() causes any needed modules to be linked into the
 * program.
 */
static
void
link_in_need_modules(
void)
{
    struct symbol_list *undefined, *next_undefined;
    struct nlist *symbol;
    enum link_state *module_state;
    struct lib *lib;

	for(undefined = undefined_list.next;
	    undefined != &undefined_list;
	    /* no increment expression */){

	    /*
	     * Look up the symbol, if is not found we can't redo the prebinding.
	     * So leave it on the undefined list and if there are undefined
	     * symbols on the list after all the undefined symbols have been
	     * searched for a message will be printed and exit(2) will be done
	     * to indicate this.
	     */
	    lookup_symbol(undefined->name,
			  get_primary_lib(undefined->ilib, undefined->symbol),
			  get_weak(undefined->symbol),
			  &symbol, &module_state, &lib, NULL, NULL,
			  NO_INDR_LOOP);
	    if(symbol != NULL){
		/*
		 * The symbol was found so remove it from the undefined_list.
		 * Then if the module that defined this symbol is unlinked
		 * then link it in checking for multiply defined symbols.
		 */
		/* take this off the undefined list */
		next_undefined = undefined->next;
		undefined->prev->next = undefined->next;
		undefined->next->prev = undefined->prev;
		undefined = next_undefined;

		if(*module_state == UNLINKED)
		    link_library_module(module_state, lib);

		if(undefined == &undefined_list &&
		   undefined->next != &undefined_list)
		    undefined = undefined->next;
	    }
	    else{
		undefined = undefined->next;
	    }
	}

	if(undefined_list.next != &undefined_list){
#ifndef LIBRARY_API
	    printf("%s: ", progname);
#endif
	    message("prebinding can't be redone for: %s (for architecture "
		    "%s) because of undefined symbols:\n", 
		    arch->file_name, arch_name);
	    for(undefined = undefined_list.next;
		undefined != &undefined_list;
		undefined = undefined->next){
		    message("%s\n", undefined->name);
		}
	    redo_exit(2);
	}
}

/*
 * link_library_module() links in the specified library module. It checks the
 * module for symbols that are already defined and reports multiply defined
 * errors.  Then it adds it's undefined symbols to the undefined list.
 */
static
void
link_library_module(
enum link_state *module_state,
struct lib *lib)
{
    uint32_t i, j, module_index, ilib;
    struct dylib_module *dylib_module;
    char *name;
    struct nlist *prev_symbol;
    enum link_state *prev_module_state;
    struct lib *prev_lib;
    struct nlist *ref_symbol;
    enum link_state *ref_module_state;
    struct lib *ref_lib;
    uint32_t mh_flags;
    
	module_index = (uint32_t)(module_state - lib->module_states);
	dylib_module = lib->mods + module_index;
	ilib = (uint32_t)(lib - libs);

        if(lib->ofile->mh != NULL)
            mh_flags = lib->ofile->mh->flags;
        else
            mh_flags = lib->ofile->mh64->flags;
            
	/*
	 * If we are not forcing the flat namespace and this is a two-level
	 * namespace image its defined symbols can't cause any multiply defined 
	 * so we can skip checking for them and go on to adding undefined
	 * symbols.
	 */
	if(arch_force_flat_namespace == FALSE &&
	   (mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL){
	    goto add_undefineds;
	}

	/*
	 * For each defined symbol check to see if it is not defined in a module
	 * that is already linked (or being linked).
	 */
	for(i = dylib_module->iextdefsym;
	    i < dylib_module->iextdefsym + dylib_module->nextdefsym;
	    i++){

	    name = lib->strings + lib->symbols[i].n_un.n_strx;
	    lookup_symbol(name,
			  get_primary_lib(ilib, lib->symbols + i),
			  get_weak(lib->symbols + i),
			  &prev_symbol, &prev_module_state, &prev_lib,
			  NULL, NULL, NO_INDR_LOOP);
	    if(prev_symbol != NULL &&
	       module_state != prev_module_state &&
	       *prev_module_state != UNLINKED){
#ifndef LIBRARY_API
		printf("%s: ", progname);
#endif
		message("prebinding can't be redone for: %s (for "
		        "architecture %s) because of multiply defined "
		        "symbol: %s\n", arch->file_name, arch_name, name);
		if(prev_module_state == &arch_state)
		    message("%s definition of %s\n", arch->file_name, name);
		else
		    message("%s(%s) definition of %s\n", prev_lib->file_name,
			    prev_lib->strings +
			    prev_lib->mods[
				prev_module_state - prev_lib->module_states].
				module_name, name);
		if(module_state == &arch_state)
		    message("%s definition of %s\n", arch->file_name, name);
		else
		    message("%s(%s) definition of %s\n", lib->file_name,
			    lib->strings + dylib_module->module_name, name);
		redo_exit(2);
	    }
	}

add_undefineds:
	/*
	 * For each reference to an undefined symbol look it up to see if it is
	 * defined in an already linked module.  If it is not then add it to
	 * the undefined list.
	 */
	for(i = dylib_module->irefsym;
	    i < dylib_module->irefsym + dylib_module->nrefsym;
	    i++){

	    if(lib->refs[i].flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
	       lib->refs[i].flags == REFERENCE_FLAG_UNDEFINED_LAZY){
		name = lib->strings +
		       lib->symbols[lib->refs[i].isym].n_un.n_strx;
		lookup_symbol(name,
			      get_primary_lib(ilib, lib->symbols +
						    lib->refs[i].isym),
			      get_weak(lib->symbols + lib->refs[i].isym),
			      &ref_symbol, &ref_module_state, &ref_lib,
			      NULL, NULL, NO_INDR_LOOP);
		if(ref_symbol != NULL){
		    if(*ref_module_state == UNLINKED)
			add_to_undefined_list(name,
					      lib->symbols + lib->refs[i].isym,
					      ilib);

		}
		else{
		    add_to_undefined_list(name,
					  lib->symbols + lib->refs[i].isym,
					  ilib);
		}
	    }
	    else{
		/*
		 * If this is a reference to a private extern make sure the
		 * module that defineds it is linked and if not set cause it
		 * to be linked.  References to private externs in a library
		 * only are resolved to symbols in the same library and modules
		 * in a library that have private externs can't have any global
		 * symbols (this is done by the static link editor).  The reason
		 * this is done at all is so that module is marked as linked.
		 */
		if(lib->refs[i].flags ==
				   REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY ||
		   lib->refs[i].flags ==
				   REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY){
		    for(j = 0; j < lib->nmodtab; j++){
			if(lib->refs[i].isym >= lib->mods[j].ilocalsym &&
			   lib->refs[i].isym <
			       lib->mods[j].ilocalsym + lib->mods[j].nlocalsym)
			    break;
		    }
		    if(j < lib->nmodtab){
			if(lib->module_states[j] == UNLINKED){
			    lib->module_states[j] = LINKED;
			    link_library_module(lib->module_states + j, lib);
			}
		    }
		}
	    }
	}

	*module_state = LINKED;

	/*
	 * If this library has a shared library initialization routine then
	 * make sure this module is linked in.  If not link it in.
	 */
	if(lib->rc != NULL &&
	   lib->module_states[lib->rc->init_module] == UNLINKED){
	   link_library_module(lib->module_states + lib->rc->init_module, lib);
	}
}

/*
 * add_to_undefined_list() adds an item to the list of undefined symbols.
 */
static
void
add_to_undefined_list(
char *name,
struct nlist *symbol,
uint32_t ilib)
{
    struct symbol_list *undefined, *new;

	for(undefined = undefined_list.next;
	    undefined != &undefined_list;
	    undefined = undefined->next){
	    if(undefined->name == name)
		return;
	}

	/* get a new symbol list entry */
	new = allocate(sizeof(struct symbol_list));

	/* fill in the pointers for the undefined symbol */
	new->name = name;
	new->symbol = symbol;
	new->ilib = ilib;

	/* put this at the end of the undefined list */
	new->prev = undefined_list.prev;
	new->next = &undefined_list;
	undefined_list.prev->next = new;
	undefined_list.prev = new;
}

/*
 * get_primary_lib() gets the primary library for a two-level symbol reference
 * from the library specified by ilib (in index into the libs[] array).  The
 * value of ilib may be ARCH_LIB which then refers to the arch being processed.
 * If the library specified by ilib is not a two-level namespace library or if 
 * arch_force_flat_namespace is TRUE then NULL is returned.  Otherwise the
 * pointer to the primary library for the reference is returned.
 */
static
struct lib *
get_primary_lib(
uint32_t ilib,
struct nlist *symbol)
{
    struct lib *lib;
    uint32_t mh_flags;

	if(arch_force_flat_namespace == TRUE)
	    return(NULL);
	if(ilib == ARCH_LIB){
	    lib = &arch_lib;
            if(arch->object->mh != NULL)
                mh_flags = arch->object->mh->flags;
            else
                mh_flags = arch->object->mh64->flags;
	}
	else if(ilib == WEAK_LIB){
	    return(&weak_lib);
	}
	else{
	    lib = libs + ilib;
            if(lib->ofile->mh != NULL)
                mh_flags = lib->ofile->mh->flags;
            else
                mh_flags = lib->ofile->mh64->flags;
	}
	if((mh_flags & MH_TWOLEVEL) != MH_TWOLEVEL)
	    return(NULL);
	/*
	 * Note for prebinding: no image should have a LIBRARY_ORDINAL of
	 * EXECUTABLE_ORDINAL or DYNAMIC_LOOKUP_ORDINAL. These values are only
	 * used for bundles and images that have undefined symbols that are
	 * looked up dynamically both of which are not prebound.
	 *
	 * But care has to be taken for compatibility as the ordinal for
	 * DYNAMIC_LOOKUP_ORDINAL use to be the maximum number of libraries an
	 * image could have.  So if this is an old image with that number of
	 * images then DYNAMIC_LOOKUP_ORDINAL is a valid library ordinal and
	 * needs to be treated as a normal library ordinal.
	 */
	if(GET_LIBRARY_ORDINAL(symbol->n_desc) == EXECUTABLE_ORDINAL)
	    return(NULL);
	if(lib->ndependent_images != DYNAMIC_LOOKUP_ORDINAL &&
	   GET_LIBRARY_ORDINAL(symbol->n_desc) == DYNAMIC_LOOKUP_ORDINAL)
	    return(NULL);
	
	/*
	 * For two-level libraries that reference symbols defined in the
	 * same library then the LIBRARY_ORDINAL will be
	 * SELF_LIBRARY_ORDINAL as the symbol is the defined symbol.
	 */
	if(GET_LIBRARY_ORDINAL(symbol->n_desc) == SELF_LIBRARY_ORDINAL)
	    return(libs + ilib);

	if(lib->dependent_images[GET_LIBRARY_ORDINAL(symbol->n_desc) - 1] ==
	   WEAK_LIB)
	    return(&weak_lib);
		
	return(libs +
	       lib->dependent_images[GET_LIBRARY_ORDINAL(symbol->n_desc) - 1]);
}

/*
 * get_indr_lib() is passed the the indirect name of an N_INDR symbol and the
 * library it came from.  It returns the library to look this indirect name up
 * in.  For flat libraries it returns NULL.  For two-level images it finds the
 * corresponding undefined symbol for the indirect name and returns the primary
 * library that undefined symbol is bound to.
 */
static
struct lib *
get_indr_lib(
char *symbol_name,
struct lib *lib)
{
    struct dysymtab_command *dyst;
    struct nlist *symbols, *symbol;
    struct dylib_table_of_contents *tocs, *toc;
    uint32_t symbol_index;
    struct lib *indr_lib;
    char *file_name;
    uint32_t mh_flags, mh_filetype;
    
	if(lib == &arch_lib) {
            if(arch->object->mh != NULL)
                mh_flags = arch->object->mh->flags;
            else
                mh_flags = arch->object->mh64->flags;
            mh_filetype = arch->object->mh_filetype;
	} else {
            if(lib->ofile->mh != NULL)
                mh_flags = lib->ofile->mh->flags;
            else
                mh_flags = lib->ofile->mh64->flags;
            mh_filetype = lib->ofile->mh_filetype;
        }

	/*
	 * If this is a flat library then the indr library is NULL.
	 */
	if(arch_force_flat_namespace == TRUE ||
	   (mh_flags & MH_TWOLEVEL) != MH_TWOLEVEL)
	    return(NULL);

	/*
	 * The only non-dynamic library could be the arch being processed.
	 */
	if(lib == &arch_lib && mh_filetype != MH_DYLIB){
	    bsearch_strings = arch_strings;
	    symbol = bsearch(symbol_name,
			     arch_symbols + arch->object->dyst->iundefsym,
			     arch->object->dyst->nundefsym,
			     sizeof(struct nlist),
			     (int (*)(const void *,const void *))nlist_bsearch);
	    /* if this fails we really have a malformed symbol table */
	    if(symbol == NULL){
		error("malformed file: %s (table of contents or "				      "undefined symbol list) N_INDR symbol %s not "
		      "found (for architecture %s)", arch->file_name,
		      symbol_name, arch_name);
		redo_exit(2);
	    }
	    indr_lib = get_primary_lib(ARCH_LIB, symbol);
	}
	else{
	    /*
	     * We need the "undefined symbol" in this image for this
	     * symbol_name so we can get the primary image for its lookup.
	     * Since this image is a library the "undefined symbol" maybe
	     * defined in this library but in a different module so first
	     * look in the defined symbols then in the undefined symbols.
	     */
	    if(lib == &arch_lib){
		tocs = arch_tocs;
		bsearch_strings = arch_strings;
		bsearch_symbols = arch_symbols;
		symbols = arch_symbols;
		dyst = arch->object->dyst;
		file_name = arch->file_name;
	    }
	    else{
		tocs = lib->tocs;
		bsearch_strings = lib->strings;
		bsearch_symbols = lib->symbols;
		symbols = lib->symbols;
		dyst = lib->dyst;
		file_name = lib->file_name;
	    }
	    toc = bsearch(symbol_name, tocs, dyst->ntoc,
			  sizeof(struct dylib_table_of_contents),
			  (int (*)(const void *, const void *))dylib_bsearch);
	    if(toc != NULL){
		symbol_index = toc->symbol_index;
	    }
	    else{
		symbol = bsearch(symbol_name, symbols + dyst->iundefsym,
				 dyst->nundefsym, sizeof(struct nlist),
			     (int (*)(const void *,const void *))nlist_bsearch);
		/* if this fails we really have a malformed symbol table */
		if(symbol == NULL){
		    error("malformed file: %s (table of contents or "				      "undefined symbol list) N_INDR symbol %s not "
			  "found (for architecture %s)", file_name,
			  symbol_name, arch_name);
		    redo_exit(2);
		}
		symbol_index = (uint32_t)(symbol - symbols);
	    }
	    indr_lib = get_primary_lib((uint32_t)(libs - lib),
				       symbols + symbol_index);
	}
	return(indr_lib);
}

/*
 * get_weak() is passed a symbol pointer and it the pointer is not NULL and the
 * weak bit of the symbol is set then TRUE is returned indicating the symbol is
 * weak.  Else FALSE is returned.
 */
static
enum bool
get_weak(
struct nlist *symbol)
{
	if(symbol != NULL && (symbol->n_desc & N_WEAK_REF) == N_WEAK_REF)
	    return(TRUE);
	return(FALSE);
}

/*
 * lookup_symbol() is passed a name of a symbol.  The name is looked up in the
 * current arch and the libs.  If found symbol, module_state and lib is set
 * to indicate where the symbol is defined.
 *
 * For two-level namespace lookups the primary_lib is not NULL and the symbol
 * is only looked up in that lib and its sub-images.  Note that primary_lib may
 * point to arch_lib in which case arch is used.
 */
static
void
lookup_symbol(
char *name,
struct lib *primary_lib,
enum bool weak, /* the symbol is allowed to be missing, weak */
struct nlist **symbol,
enum link_state **module_state,
struct lib **lib,
uint32_t *isub_image,
uint32_t *itoc,
struct indr_loop_list *indr_loop)
{
    uint32_t i;

	if(isub_image != NULL)
	    *isub_image = 0;
	if(itoc != NULL)
	    *itoc = 0;
	/*
	 * If primary_image is non-NULL this is a two-level name space lookup.
	 * So look this symbol up only in the primary_image and its sub-images.
	 */
	if(primary_lib != NULL){
	    if(primary_lib == &weak_lib){
		weak = TRUE;
		goto weak_library_symbol;
	    }
	    if(primary_lib == &arch_lib){
		if(lookup_symbol_in_arch(name, symbol, module_state, lib,
    					 isub_image, itoc, indr_loop) == TRUE)
		    return;
	    }
	    else{
		if(lookup_symbol_in_lib(name, primary_lib, symbol, module_state,
				    lib, isub_image, itoc, indr_loop) == TRUE)
		    return;
	    }
	    for(i = 0; i < primary_lib->nsub_images; i++){
		if(lookup_symbol_in_lib(name, libs + primary_lib->sub_images[i],
		 			symbol, module_state, lib, isub_image,
					itoc, indr_loop) == TRUE){
		    if(isub_image != NULL)
			*isub_image = i + 1;
		    return;
		}
	    }
	    /*
	     * If we get here the symbol was not found in the primary_image and 
	     * its sub-images so it is undefined for a two-level name space
	     * lookup. Or the symbol could have been a weak symbol that is
	     * missing and if so return the constant values for a weak symbol
	     * lookup.
	     */
weak_library_symbol:
	    if(weak == TRUE){
		*symbol = &weak_symbol;
		*module_state = &weak_module;
		*lib = primary_lib; /* could be &weak_lib */
		return;
	    }
	    *symbol = NULL;
	    *module_state = NULL;
	    *lib = NULL;
	    return;
	}

	/*
	 * This is a flat namespace lookup so first search the current arch for
	 * the named symbol as a defined external symbol.
	 */
	if(lookup_symbol_in_arch(name, symbol, module_state, lib, isub_image,
				 itoc, indr_loop) == TRUE)
	    return;

	/*
	 * The symbol was not found in the current arch so next look through the
	 * libs for the a definition of the named symbol.
	 */
	for(i = 0; i < nlibs; i++){
	    if(lookup_symbol_in_lib(name, libs + i, symbol, module_state, lib,
				    isub_image, itoc, indr_loop) == TRUE)
		return;
	}

	/* the symbol was not found */
	if(weak == TRUE){
	    *symbol = &weak_symbol;
	    *module_state = &weak_module;
	    *lib = primary_lib; /* could be &weak_lib */
	    return;
	}
	*symbol = NULL;
	*module_state = NULL;
	*lib = NULL;
	return;
}

/*
 * lookup_symbol_in_arch() is a sub-routine for lookup_symbol().  It looks up
 * the symbol name in the current arch.  If it finds it it returns TRUE else it
 * returns FALSE.
 */
static
enum bool
lookup_symbol_in_arch(
char *name,
struct nlist **symbol,
enum link_state **module_state,
struct lib **lib,
uint32_t *isub_image,
uint32_t *itoc,
struct indr_loop_list *indr_loop)
{
    struct dylib_table_of_contents *toc;
    struct nlist *s;
    struct indr_loop_list new_indr_loop, *loop;

	/*
	 * Search the current arch for the named symbol as a defined external
	 * symbol.  If the current arch is a dylib look in the table of contents
	 * else look in the sorted external symbols.
	 */
	if(arch->object->mh_filetype == MH_DYLIB){
	    bsearch_strings = arch_strings;
	    bsearch_symbols = arch_symbols;
	    toc = bsearch(name, arch_tocs, arch_ntoc,
			  sizeof(struct dylib_table_of_contents),
			  (int (*)(const void *, const void *))dylib_bsearch);
	    if(toc != NULL){
		*symbol = arch_symbols + toc->symbol_index;
		if(((*symbol)->n_type & N_TYPE) == N_INDR){
		    name = (*symbol)->n_value + arch_strings;
		    goto indr;
		}
		*module_state = &arch_state;
		*lib = NULL;
		if(itoc != NULL)
		    *itoc = (uint32_t)(toc - arch_tocs);
		return(TRUE);
	    }
	}
	else{
	    bsearch_strings = arch_strings;
	    s = bsearch(name,
			arch_symbols + arch->object->dyst->iextdefsym,
			arch->object->dyst->nextdefsym,
			sizeof(struct nlist),
			(int (*)(const void *,const void *))nlist_bsearch);
	    if(s != NULL){
		*symbol = s;
		if(((*symbol)->n_type & N_TYPE) == N_INDR){
		    name = (*symbol)->n_value + arch_strings;
		    goto indr;
		}
		*module_state = &arch_state;
		*lib = NULL;
		return(TRUE);
	    }
	}
	*symbol = NULL;
	*module_state = NULL;
	*lib = NULL;
	return(FALSE);

indr:
	if(indr_loop != NO_INDR_LOOP){
	    for(loop = indr_loop; loop != NULL; loop = loop->next){
		if(loop->symbol == *symbol){
		    /* this is an indirect loop */
		    *symbol = NULL;
		    *module_state = NULL;
		    *lib = NULL;
		    return(FALSE);
		}
	    }
	}
	new_indr_loop.symbol = *symbol;
	new_indr_loop.next = indr_loop;
	lookup_symbol(name, get_indr_lib(name, &arch_lib), FALSE, symbol,
		      module_state, lib, isub_image, itoc, &new_indr_loop);
	return(symbol != NULL);
}

/*
 * lookup_symbol_in_lib() is a sub-routine for lookup_symbol().  It looks up
 * the symbol name in the specified primary library.  If it finds it it returns
 * TRUE else it returns FALSE.
 */
static
enum bool
lookup_symbol_in_lib(
char *name,
struct lib *primary_lib,
struct nlist **symbol,
enum link_state **module_state,
struct lib **lib,
uint32_t *isub_image,
uint32_t *itoc,
struct indr_loop_list *indr_loop)
{
    struct dylib_table_of_contents *toc;
    struct indr_loop_list new_indr_loop, *loop;

	bsearch_strings = primary_lib->strings;
	bsearch_symbols = primary_lib->symbols;
	toc = bsearch(name, primary_lib->tocs, primary_lib->ntoc,
		      sizeof(struct dylib_table_of_contents),
		      (int (*)(const void *, const void *))dylib_bsearch);
	if(toc != NULL){
	    *symbol = primary_lib->symbols + toc->symbol_index;
	    if(((*symbol)->n_type & N_TYPE) == N_INDR){
		name = (*symbol)->n_value + primary_lib->strings;
		goto indr;
	    }
	    *module_state = primary_lib->module_states + toc->module_index;
	    *lib = primary_lib;
	    if(itoc != NULL)
		*itoc = (uint32_t)(toc - primary_lib->tocs);
	    return(TRUE);
	}
	*symbol = NULL;
	*module_state = NULL;
	*lib = NULL;
	return(FALSE);

indr:
	if(indr_loop != NO_INDR_LOOP){
	    for(loop = indr_loop; loop != NULL; loop = loop->next){
		if(loop->symbol == *symbol){
		    /* this is an indirect loop */
		    *symbol = NULL;
		    *module_state = NULL;
		    *lib = NULL;
		    return(FALSE);
		}
	    }
	}
	new_indr_loop.symbol = *symbol;
	new_indr_loop.next = indr_loop;
	lookup_symbol(name, get_indr_lib(name, primary_lib), FALSE, symbol,
		      module_state, lib, isub_image, itoc, &new_indr_loop);
	return(symbol != NULL);
}

/*
 * build_new_symbol_table() builds a new symbol table for the current arch
 * using the new values for prebound undefined symbols from the dependent
 * libraries.  Also adjusts defined symbol values in the symbol table by the
 * vmslide for symbols in sections as well as the objc_module_info_addr field
 * of the module table.  If we are allowing missing architecures and some of
 * the dependent libraries are missing then missing_arch is TRUE and we set it
 * up so that this arch is still written out but with out the prebinding info
 * updated.
 */
static
void
build_new_symbol_table(
uint32_t vmslide,
enum bool missing_arch)
{
    uint32_t i, j, sym_info_size, ihint, isub_image, itoc, objc_slide;
    uint32_t lowest_objc_module_info_addr;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s, *s_objc;
    char *symbol_name, *dot;
    struct nlist *new_symbols;
    struct nlist_64 *new_symbols64;
    struct nlist *symbol;
    enum link_state *module_state;
    struct lib *lib;

	/* silence compiler warnings */
	isub_image = 0;
	itoc = 0;

	/* the size of the symbol table will not change just the contents */
	sym_info_size =
	    arch_nextrel * sizeof(struct relocation_info) +
	    arch_nlocrel * sizeof(struct relocation_info) +
	    arch_ntoc * sizeof(struct dylib_table_of_contents) +
	    arch_nextrefsyms * sizeof(struct dylib_reference) +
	    arch_strsize;

	if(arch->object->mh != NULL){
	    sym_info_size +=
		arch_nmodtab * sizeof(struct dylib_module) +
		arch_nsyms * sizeof(struct nlist) +
		arch_nindirectsyms * sizeof(uint32_t);
	}
	else{
	    sym_info_size +=
		arch_nmodtab * sizeof(struct dylib_module_64) +
		arch_nsyms * sizeof(struct nlist_64) +
		rnd(arch_nindirectsyms * sizeof(uint32_t), 8);
	}

	if(arch->object->hints_cmd != NULL){
	    sym_info_size +=
		arch->object->hints_cmd->nhints *
		sizeof(struct twolevel_hint);
	}

	if(arch->object->split_info_cmd != NULL){
	    sym_info_size +=
		arch->object->split_info_cmd->datasize;
	}

	if(arch->object->func_starts_info_cmd != NULL){
	    sym_info_size +=
		arch->object->func_starts_info_cmd->datasize;
	}

	if(arch->object->data_in_code_cmd != NULL){
	    sym_info_size +=
		arch->object->data_in_code_cmd->datasize;
	}

	if(arch->object->code_sign_drs_cmd != NULL){
	    sym_info_size +=
		arch->object->code_sign_drs_cmd->datasize;
	}

	if(arch->object->link_opt_hint_cmd != NULL){
	    sym_info_size +=
		arch->object->link_opt_hint_cmd->datasize;
	}

	if(arch->object->dyld_chained_fixups != NULL){
	    sym_info_size +=
		arch->object->dyld_chained_fixups->datasize;
	}

	if(arch->object->dyld_exports_trie != NULL){
	    sym_info_size +=
		arch->object->dyld_exports_trie->datasize;
	}

	if(arch->object->code_sig_cmd != NULL){
	    sym_info_size =
		rnd32(sym_info_size, 16);
	    sym_info_size +=
		arch->object->code_sig_cmd->datasize;
	}

	arch->object->input_sym_info_size = sym_info_size;
	arch->object->output_sym_info_size = sym_info_size;

	arch->object->output_nsymbols = arch_nsyms;
	arch->object->output_strings_size = arch_strsize;

	arch->object->output_ilocalsym = arch->object->dyst->ilocalsym;
	arch->object->output_nlocalsym = arch->object->dyst->nlocalsym;
	arch->object->output_iextdefsym = arch->object->dyst->iextdefsym;
	arch->object->output_nextdefsym = arch->object->dyst->nextdefsym;
	arch->object->output_iundefsym = arch->object->dyst->iundefsym;
	arch->object->output_nundefsym = arch->object->dyst->nundefsym;

	arch->object->output_loc_relocs = arch_locrelocs;
	arch->object->output_ext_relocs = arch_extrelocs;
	arch->object->output_indirect_symtab = arch_indirect_symtab;

	arch->object->output_tocs = arch_tocs;
	arch->object->output_ntoc = arch_ntoc;
	arch->object->output_mods = arch_mods;
	arch->object->output_mods64 = arch_mods64;
	arch->object->output_nmodtab = arch_nmodtab;
	arch->object->output_refs = arch_refs;
	arch->object->output_nextrefsyms = arch_nextrefsyms;

	if(arch->object->hints_cmd != NULL &&
	   arch->object->hints_cmd->nhints != 0){
	    arch->object->output_hints = (struct twolevel_hint *)
		    (arch->object->object_addr +
		     arch->object->hints_cmd->offset);
	}
	if(arch->object->split_info_cmd != NULL){
	    arch->object->output_split_info_data = arch->object->object_addr +
		arch->object->split_info_cmd->dataoff;
	    arch->object->output_split_info_data_size = 
		arch->object->split_info_cmd->datasize;
	}
	if(arch->object->func_starts_info_cmd != NULL){
	    arch->object->output_func_start_info_data =
		arch->object->object_addr +
		arch->object->func_starts_info_cmd->dataoff;
	    arch->object->output_func_start_info_data_size = 
		arch->object->func_starts_info_cmd->datasize;
	}
	if(arch->object->data_in_code_cmd != NULL){
	    arch->object->output_data_in_code_info_data =
		arch->object->object_addr +
		arch->object->data_in_code_cmd->dataoff;
	    arch->object->output_data_in_code_info_data_size = 
		arch->object->data_in_code_cmd->datasize;
	}
	if(arch->object->code_sign_drs_cmd != NULL){
	    arch->object->output_code_sign_drs_info_data =
		arch->object->object_addr +
		arch->object->code_sign_drs_cmd->dataoff;
	    arch->object->output_code_sign_drs_info_data_size = 
		arch->object->code_sign_drs_cmd->datasize;
	}
	if(arch->object->link_opt_hint_cmd != NULL){
	    arch->object->output_link_opt_hint_info_data =
		arch->object->object_addr +
		arch->object->link_opt_hint_cmd->dataoff;
	    arch->object->output_link_opt_hint_info_data_size = 
		arch->object->link_opt_hint_cmd->datasize;
	}
	if(arch->object->dyld_chained_fixups != NULL){
	    arch->object->output_dyld_chained_fixups_data =
		arch->object->object_addr +
		arch->object->dyld_chained_fixups->dataoff;
	    arch->object->output_dyld_chained_fixups_data_size =
		arch->object->dyld_chained_fixups->datasize;
	}
	if(arch->object->dyld_exports_trie != NULL){
	    arch->object->output_dyld_exports_trie_data =
		arch->object->object_addr +
		arch->object->dyld_exports_trie->dataoff;
	    arch->object->output_dyld_exports_trie_data_size =
		arch->object->dyld_exports_trie->datasize;
	}
	if(arch->object->code_sig_cmd != NULL){
	    arch->object->output_code_sig_data = arch->object->object_addr +
		arch->object->code_sig_cmd->dataoff;
	    arch->object->output_code_sig_data_size = 
		arch->object->code_sig_cmd->datasize;
	}

	if(arch->object->mh != NULL){
	    new_symbols = allocate(arch_nsyms * sizeof(struct nlist));
	    memcpy(new_symbols, arch_symbols,
		   arch_nsyms * sizeof(struct nlist));
	    arch->object->output_symbols = new_symbols;
	    arch->object->output_symbols64 = NULL;
	    new_symbols64 = NULL;
	}
	else{
	    new_symbols = NULL;
	    arch->object->output_symbols = NULL;
	    new_symbols64 = allocate(arch_nsyms * sizeof(struct nlist_64));
	    memcpy(new_symbols64, arch_symbols64,
		   arch_nsyms * sizeof(struct nlist_64));
	    arch->object->output_symbols64 = new_symbols64;
	}

	/* the strings don't change so just use the existing string table */
	arch->object->output_strings = arch_strings;

	/*
	 * If we are simply building a copy of the symbol table for an arch that
	 * has missing architecures for its dependent libraries we are done and
	 * can return.
	 */
	if(missing_arch == TRUE){
	    arch->dont_update_LC_ID_DYLIB_timestamp = TRUE;
	    return;
	}

	/*
	 * Update the objc_module_info_addr fields if this is slid.
	 *
	 * The FCS Tiger dyld does not update these fields when prebinding.
	 * So in order to get them correct when unprebinding we base the
	 * adjustment value on the difference between the the address of the
	 * (__OBJC,__module_info) section and the module table entry with the
	 * lowest objc_module_info_addr value.
	 */
	objc_slide = 0;
	if(vmslide != 0){
	    if(arch->object->mh != NULL){
		if(unprebinding && arch_nmodtab != 0){
		    lowest_objc_module_info_addr = UINT_MAX;
		    for(i = 0; i < arch_nmodtab; i++){
			if(arch_mods[i].objc_module_info_size != 0){
			    if(arch_mods[i].objc_module_info_addr <
			       lowest_objc_module_info_addr){
				lowest_objc_module_info_addr =
				    arch_mods[i].objc_module_info_addr;
			    }
			}
		    }
		    s_objc = NULL;
		    lc = arch->object->load_commands;
		    for(i = 0;
			i < arch->object->mh->ncmds && s_objc == NULL;
			i++){
			if(lc->cmd == LC_SEGMENT){
			    sg = (struct segment_command *)lc;
			    if(strcmp(sg->segname, SEG_OBJC) == 0){
				s = (struct section *)((char *)sg +
					sizeof(struct segment_command));
				for(j = 0 ; j < sg->nsects; j++){
				    if(strcmp(s[j].sectname,
					      SECT_OBJC_MODULES) == 0){
					s_objc = s + j;
					break;
				    }
				}
			    }
			}
			lc = (struct load_command *)((char *)lc + lc->cmdsize);
		    }
		    if(lowest_objc_module_info_addr != UINT_MAX &&
		       s_objc != NULL){
			objc_slide = s_objc->addr -
				     lowest_objc_module_info_addr;
		    }
		    else{
			objc_slide = vmslide;
		    }
		}
		else{
		    objc_slide = vmslide;
		}
		for(i = 0; i < arch_nmodtab; i++){
		    if(arch_mods[i].objc_module_info_size != 0)
			arch_mods[i].objc_module_info_addr += objc_slide;
		}
	    }
	    else{
		for(i = 0; i < arch_nmodtab; i++){
		    if(arch_mods64[i].objc_module_info_size != 0)
			arch_mods64[i].objc_module_info_addr += vmslide;
		}
	    }
	}

	/*
	 * The new symbol table is just a copy of the old symbol table with
	 * the n_value's of the prebound undefined symbols updated and the
	 * n_value's of the N_SECT updated if slid.
	 */
	ihint = 0;
	for(i = arch->object->dyst->iundefsym;
	    i < arch->object->dyst->iundefsym + arch->object->dyst->nundefsym;
	    i++){

            if(unprebinding == TRUE){
		if(arch->object->mh != NULL){
		    new_symbols[i].n_value = 0;
		    new_symbols[i].n_type &= ~N_TYPE;
		    new_symbols[i].n_type |= N_UNDF;
		}
		else{
		    new_symbols64[i].n_value = 0;
		    new_symbols64[i].n_type &= ~N_TYPE;
		    new_symbols64[i].n_type |= N_UNDF;
		}
            }
            else{
		if(arch->object->mh != NULL){
		    symbol_name = arch_strings + arch_symbols[i].n_un.n_strx;
		    lookup_symbol(symbol_name,
				get_primary_lib(ARCH_LIB, arch_symbols + i),
				get_weak(arch_symbols + i),
				&symbol, &module_state, &lib, &isub_image,
				&itoc, NO_INDR_LOOP);
		    new_symbols[i].n_value = symbol->n_value;
		    if(symbol->n_desc & N_ARM_THUMB_DEF)
			new_symbols[i].n_value |= 1;
		}
		else{
		    fatal_arch(arch, NULL, "code does not yet support 64-bit "
			       "Mach-O binaries");
		}
            }
	    /*
	     * Also update the hints table.
	     */
	    if(arch_hints != NULL){
		if(unprebinding == TRUE){
		    arch_hints[ihint].isub_image = 0;
		    arch_hints[ihint].itoc = 0;
		}
		else{
		    arch_hints[ihint].isub_image = isub_image;
		    arch_hints[ihint].itoc = itoc;
		}
		ihint++;
	    }
	}
	/*
	 * Adjust defined symbol values in the symbol table by the vmslide for
	 * symbols in sections if the vmslide is not zero.
	 */
	if(vmslide != 0){
	    for(i = arch->object->dyst->iextdefsym;
		i < arch->object->dyst->iextdefsym +
		    arch->object->dyst->nextdefsym;
		i++){
		if(arch->object->mh != NULL){
		    if(arch_symbols[i].n_sect != NO_SECT)
			new_symbols[i].n_value += vmslide;
		}
		else{
		    if(arch_symbols64[i].n_sect != NO_SECT)
			new_symbols64[i].n_value += vmslide;
		}
	    }
	    for(i = arch->object->dyst->ilocalsym;
		i < arch->object->dyst->ilocalsym +
		    arch->object->dyst->nlocalsym;
		i++){
		if(arch->object->mh != NULL){
		    if(arch_symbols[i].n_sect != NO_SECT)
			new_symbols[i].n_value += vmslide;
		}
		else{
		    if(arch_symbols64[i].n_sect != NO_SECT)
			new_symbols64[i].n_value += vmslide;
		}
	    }
	}
	/*
	 * The FCS Tiger dyld had a bug, rdar://4108674, which incorrectly slid
	 * absolute symbols.  We should set them back to there correct values
	 * but that information has been lost.  So here we fix up what we know
	 * we can do safely and correctly.  Which is setting the compiler
	 * generated global absolute symbols that start with ".objc" and end
	 * with ".eh" to zero.
	 */
	if(unprebinding){
           for(i = arch->object->dyst->iextdefsym;
               i < arch->object->dyst->iextdefsym +
                   arch->object->dyst->nextdefsym;
               i++){
		/* this fix up is only done for 32-bit Mach-O files */
		if(arch->object->mh != NULL){
		    if((arch_symbols[i].n_type & N_TYPE) == N_ABS){
			symbol_name = arch_strings +
				      arch_symbols[i].n_un.n_strx;
			dot = rindex(symbol_name, '.');
			if(strncmp(symbol_name, ".objc",
			   sizeof(".objc") - 1) == 0 ||
			   (dot != NULL && dot[1] == 'e' &&
			    dot[2] == 'h' && dot[3] == '\0') )
			new_symbols[i].n_value = 0;
		    }
		}
	    }
	}
	/*
	 * Update the STSYM and SO stabs if this is slid.
	 *
	 * The FCS Tiger dyld does not update these stabs when prebinding.
	 * If this is an objective-c dylib, we can correct when unprebinding
	 * by the same adjustment used for objc_module_info_addr.
	 *
	 * This fix up is only done for 32-bit Mach-O files when unprebinding 
	 */
	if(arch->object->mh != NULL && unprebinding == TRUE){
	    if(vmslide != 0 && objc_slide != vmslide){
		for(i = arch->object->dyst->ilocalsym;
		    i < arch->object->dyst->ilocalsym +
			arch->object->dyst->nlocalsym;
		    i++){
		    if((new_symbols[i].n_type & N_STAB) != 0){
			if(new_symbols[i].n_type == N_STSYM ||
			   new_symbols[i].n_type == N_SO){
			    new_symbols[i].n_value += objc_slide - vmslide;
			}
		    }
		}
	    }
	}
}

/*
 * setup_r_address_base() is called to set this arch's seg1addr or
 * segs_read_write_addr which is the base for the offset values in relocation
 * entries' r_address fields.
 */
static
void
setup_r_address_base(
void)
{
    uint32_t i;
    struct load_command *lc;
    struct segment_command *sg;
    uint32_t ncmds, mh_flags;
    
	/*
	 * Figure out what this arch's seg1addr or segs_read_write_addr is
	 * which is the base for the offset values in relocation entries.
	 * It is the address of the first segment or the first read-write
	 * segment for MH_SPLIT_SEGS images.
	 */
        if(arch->object->mh != NULL) {
            ncmds = arch->object->mh->ncmds;
            mh_flags = arch->object->mh->flags;
        } else {
            ncmds = arch->object->mh64->ncmds;
            mh_flags = arch->object->mh64->flags;
        }
	if((mh_flags & MH_SPLIT_SEGS) == MH_SPLIT_SEGS)
	    arch_split_segs = TRUE;
	else
	    arch_split_segs = FALSE;
	sg = NULL;
	lc = arch->object->load_commands;
	for(i = 0; i < ncmds && sg == NULL; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(arch_split_segs == FALSE){
		    arch_seg1addr = sg->vmaddr;
		}
		/*
		 * Pickup the address of the first read-write segment for
		 * MH_SPLIT_SEGS images.
		 */
		else{
		    if((sg->initprot & VM_PROT_WRITE) == VM_PROT_WRITE)
			arch_segs_read_write_addr = sg->vmaddr;
		    else
			sg = NULL;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}

/*
 * update_local_relocs() drives the updating of the items with local
 * relocation entries for the current arch.
 */
static
void
update_local_relocs(
uint32_t vmslide)
{
	switch(arch->object->mh_cputype){
	case CPU_TYPE_MC680x0:
	    update_generic_local_relocs(vmslide);
	    break;
	case CPU_TYPE_I386:
	    update_generic_local_relocs(vmslide);
	    break;
	case CPU_TYPE_HPPA:
	    update_hppa_local_relocs(vmslide);
	    break;
	case CPU_TYPE_SPARC:
	    update_sparc_local_relocs(vmslide);
	    break;
	case CPU_TYPE_POWERPC:
	    update_ppc_local_relocs(vmslide);
	    break;
	case CPU_TYPE_ARM:
	    update_arm_local_relocs(vmslide);
	    break;
	default:
	    error("can't redo prebinding for: %s (for architecture %s) because "
		  "of unknown cputype", arch->file_name, arch_name);
	}
}

/*
 * update_generic_local_relocs() updates of the items with local relocation
 * entries for the architectures that use generic relocation entries
 * (the i386 and m68k architectures).
 */
static
void
update_generic_local_relocs(
uint32_t vmslide)
{
    uint32_t i, r_address, r_pcrel, r_length, r_type, r_value, value;
    char *p;
    enum bool no_sect;
    struct scattered_relocation_info *sreloc;

	sreloc = NULL;
	r_value = 0;

	for(i = 0; i < arch_nlocrel; i++){
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)
			 (arch_locrelocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_type = sreloc->r_type;
		r_value = sreloc->r_value;
		no_sect = FALSE;
	    }
	    else{
		r_address = arch_locrelocs[i].r_address;
		r_pcrel = arch_locrelocs[i].r_pcrel;
		r_length = arch_locrelocs[i].r_length;
		r_type = arch_locrelocs[i].r_type;
		no_sect = arch_locrelocs[i].r_symbolnum == NO_SECT;
	    }

	    /*
	     * If this relocation entry pc relative, which means the value of
	     * the pc will get added to it when it is executed, the item being
	     * relocated has the value of the pc subtracted from it.  So to
	     * relocate this, the amount the image has been slid has to be
	     * subtracted from it also.
	     */
	    value = 0;
	    if(r_pcrel)
		value -= vmslide;
	    /*
	     * Since this is a local relocation entry and all sections are
	     * moving by the same amount everything gets moved except those
	     * things that are defined that are not in a section.  We are
	     * counting on not seeing any section difference relocation entries
	     * and pcrel section based (which would work but be nops).
	     */
	    if(no_sect == FALSE){
		value += vmslide;
		r_value += vmslide;
	    }

	    p = contents_pointer_for_vmaddr(r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for local relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    /*
	     * If the relocation entry is for a prebound lazy pointer (r_type is
	     * GENERIC_RELOC_PB_LA_PTR) then just the r_value needs to be
	     * updated.  The value of the symbol pointer is updated with the
	     * other symbol pointers.
	     */
	    if(r_type == GENERIC_RELOC_PB_LA_PTR){
		/* note r_value is incremented by vmslide above */
		;
	    }
	    else{
		switch(r_length){
		case 0: /* byte */
		    value += get_arch_byte(p);
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 1 "
			    "byte)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value += get_arch_short(p);
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 2 "
			    "bytes)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value += get_arch_long(p);
		    set_arch_long(p, value);
		    break;
		}
	    }

	    /*
	     * Update the parts of the relocation entries that are effected by
	     * sliding this to a different address.
	     */
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc->r_value = r_value;
	    }
	}
}

/*
 * update_hppa_local_relocs() updates of the items with local relocation
 * entries for the hppa architecture.
 */
static
void
update_hppa_local_relocs(
uint32_t vmslide)
{
    uint32_t i, r_address, r_pcrel, r_length, r_value, value;
    char *p;
    uint32_t instruction, immediate;
    enum bool no_sect;
    struct scattered_relocation_info *sreloc;
    struct relocation_info *pair_reloc;
    struct scattered_relocation_info *spair_reloc;
    enum reloc_type_hppa r_type, pair_r_type;
    uint32_t other_half;
    uint32_t hi21, lo14;
    uint32_t w, w1, w2;

	sreloc = NULL;
	pair_reloc = NULL;
	spair_reloc = NULL;
	other_half = 0;
	r_value = 0;

	for(i = 0; i < arch_nlocrel; i++){
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)
			 (arch_locrelocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_value = sreloc->r_value;
		r_type = (enum reloc_type_hppa)sreloc->r_type;
		no_sect = FALSE;
	    }
	    else{
		r_address = arch_locrelocs[i].r_address;
		r_pcrel = arch_locrelocs[i].r_pcrel;
		r_length = arch_locrelocs[i].r_length;
		r_type = (enum reloc_type_hppa)arch_locrelocs[i].r_type;
		no_sect = arch_locrelocs[i].r_symbolnum == NO_SECT;
	    }
	    /*
	     * If this relocation type has a pair break out it's fields.
	     */
	    pair_r_type = 0;
	    if(r_type == HPPA_RELOC_HI21 ||
	       r_type == HPPA_RELOC_LO14 ||
	       r_type == HPPA_RELOC_BR17){
		if(i + 1 == arch_nlocrel){
		    error("malformed file: %s (missing pair local "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		pair_reloc = arch_locrelocs + i + 1;
		if((pair_reloc->r_address & R_SCATTERED) != 0){
		    spair_reloc = (struct scattered_relocation_info *)
				  pair_reloc;
		    pair_r_type = spair_reloc->r_type;
		    other_half  = spair_reloc->r_address;
		}
		else{
		    pair_r_type = pair_reloc->r_type;
		    other_half  = pair_reloc->r_address;
		}
		i++;
		if(pair_r_type != HPPA_RELOC_PAIR){
		    error("malformed file: %s (pair local relocation entry "
			  "for entry %u is not of r_type HPPA_RELOC_PAIR) "
			  "(for architecture %s)", arch->file_name, i,
			  arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If this relocation entry pc relative, which means the value of
	     * the pc will get added to it when it is executed, the item being
	     * relocated has the value of the pc subtracted from it.  So to
	     * relocate this, the amount the image has been slid has to be
	     * subtracted from it also.
	     */
	    value = 0;
	    if(r_pcrel)
		value -= vmslide;
	    /*
	     * Since this is a local relocation entry and all sections are
	     * moving by the same amount everything gets moved except those
	     * things that are defined that are not in a section.  We are
	     * counting on not seeing any section difference relocation entries
	     * and pcrel section based (which would work but be nops).
	     */
	    if(no_sect == FALSE){
		value += vmslide;
		r_value += vmslide;
	    }

	    p = contents_pointer_for_vmaddr(r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for local relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }
	    
	    if(r_type == HPPA_RELOC_VANILLA){
		switch(r_length){
		case 0: /* byte */
		    value += get_arch_byte(p);
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 1 "
			    "byte)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value += get_arch_short(p);
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 2 "
			    "bytes)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value += get_arch_long(p);
		    set_arch_long(p, value);
		    break;
		}
	    }
	    /*
	     * Do hppa specific relocation based on the r_type.
	     */
	    else{
		switch(r_type){
		case HPPA_RELOC_PB_LA_PTR:
		    /* note r_value is incremented by vmslide above */
		    break;
		case HPPA_RELOC_HI21:
		    instruction = get_arch_long(p);
		    immediate = sign_ext(other_half, 14) + 
		               (assemble_21(instruction & 0x1fffff) << 11);
		    calc_hppa_HILO(value + immediate, 0, &hi21, &lo14);
		    instruction = (instruction & 0xffe00000) |
				  dis_assemble_21(hi21 >> 11);
		    set_arch_long(p, instruction);
		    other_half = lo14 & 0x3fff;
		    break;
		case HPPA_RELOC_LO14:
		    instruction = get_arch_long(p);
		    immediate = low_sign_ext(instruction & 0x3fff, 14) +
		    	        (other_half << 11);
		    calc_hppa_HILO(value + immediate, 0, &hi21, &lo14);
		    lo14 = low_sign_unext(lo14, 14);
		    instruction = (instruction & 0xffffc000) |
				  (lo14 & 0x3fff);
		    set_arch_long(p, instruction);
		    other_half = hi21 >> 11;
		    break;
		case HPPA_RELOC_BR17:
		    instruction = get_arch_long(p);
		    immediate = assemble_17((instruction & 0x1f0000) >> 16,
			                    (instruction & 0x1ffc) >> 2,
				             instruction & 1);
		    immediate = (sign_ext(immediate, 17) << 2) +
				(other_half << 11);
		    calc_hppa_HILO(value + immediate, 0, &hi21, &lo14);
		    lo14 >>= 2;
		    dis_assemble_17(lo14, &w1, &w2, &w);
		    instruction = (instruction & 0xffe0e002) |
				  (w1 << 16) | (w2 << 2) | w;
		    set_arch_long(p, instruction);
		    other_half = hi21 >> 11;
		    break;
		case HPPA_RELOC_BL17:
		    instruction = get_arch_long(p);
		    immediate = assemble_17((instruction & 0x1f0000) >> 16,
			                    (instruction & 0x1ffc) >> 2,
				             instruction & 1);
		    if((immediate & 0x10000) != 0)
			immediate |= 0xfffe0000;
		    immediate <<= 2;
		    immediate += value;
		    if(U_ABS(immediate) > 0x3ffff){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u displacement "
			    "too large to fit)", arch->file_name, arch_name, i);
		    }
		    immediate >>= 2;
		    dis_assemble_17(immediate, &w1, &w2, &w);
		    instruction = (instruction & 0xffe0e002) |
				  (w1 << 16) | (w2 << 2) | w;
		    set_arch_long(p, instruction);
		    break;
		default:
		    error("malformed file: %s (local relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		    break;
		}
	    }
	    /*
	     * Update the parts of the relocation entries that are effected by
	     * sliding this to a different address.
	     */
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc->r_value = r_value;
	    }
	    if(pair_r_type == HPPA_RELOC_PAIR){
		if((pair_reloc->r_address & R_SCATTERED) != 0)
		    spair_reloc->r_address = other_half;
		else
		    pair_reloc->r_address = other_half;
	    }
	}
}

/*
 * update_sparc_local_relocs() updates of the items with local relocation
 * entries for the sparc architecture.
 */
static
void
update_sparc_local_relocs(
uint32_t vmslide)
{
    uint32_t i, r_address, r_pcrel, r_length, r_value, value;
    char *p;
    uint32_t instruction, immediate;
    enum bool no_sect;
    struct scattered_relocation_info *sreloc;
    struct relocation_info *pair_reloc;
    struct scattered_relocation_info *spair_reloc;
    enum reloc_type_sparc r_type, pair_r_type;
    uint32_t other_half;

	sreloc = NULL;
	pair_reloc = NULL;
	spair_reloc = NULL;
	other_half = 0;
	r_value = 0;

	for(i = 0; i < arch_nlocrel; i++){
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)
			 (arch_locrelocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_value = sreloc->r_value;
		r_type = (enum reloc_type_sparc)sreloc->r_type;
		no_sect = FALSE;
	    }
	    else{
		r_address = arch_locrelocs[i].r_address;
		r_pcrel = arch_locrelocs[i].r_pcrel;
		r_length = arch_locrelocs[i].r_length;
		r_type = (enum reloc_type_sparc)arch_locrelocs[i].r_type;
		no_sect = arch_locrelocs[i].r_symbolnum == NO_SECT;
	    }
	    /*
	     * If this relocation type has a pair break out it's fields.
	     */
	    pair_r_type = 0;
	    if(r_type == SPARC_RELOC_HI22 ||
	       r_type == SPARC_RELOC_LO10 ){
		if(i + 1 == arch_nlocrel){
		    error("malformed file: %s (missing pair local "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		pair_reloc = arch_locrelocs + i + 1;
		if((pair_reloc->r_address & R_SCATTERED) != 0){
		    spair_reloc = (struct scattered_relocation_info *)
				  pair_reloc;
		    pair_r_type = spair_reloc->r_type;
		    other_half  = spair_reloc->r_address;
		}
		else{
		    pair_r_type = pair_reloc->r_type;
		    other_half  = pair_reloc->r_address;
		}
		i++;
		if(pair_r_type != SPARC_RELOC_PAIR){
		    error("malformed file: %s (pair local relocation entry "
			  "for entry %u is not of r_type SPARC_RELOC_PAIR) "
			  "(for architecture %s)", arch->file_name, i,
			  arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If this relocation entry pc relative, which means the value of
	     * the pc will get added to it when it is executed, the item being
	     * relocated has the value of the pc subtracted from it.  So to
	     * relocate this, the amount the image has been slid has to be
	     * subtracted from it also.
	     */
	    value = 0;
	    if(r_pcrel)
		value -= vmslide;
	    /*
	     * Since this is a local relocation entry and all sections are
	     * moving by the same amount everything gets moved except those
	     * things that are defined that are not in a section.  We are
	     * counting on not seeing any section difference relocation entries
	     * and pcrel section based (which would work but be nops).
	     */
	    if(no_sect == FALSE){
		value += vmslide;
		r_value += vmslide;
	    }

	    p = contents_pointer_for_vmaddr(r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for local relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }
	    
	    if(r_type == SPARC_RELOC_VANILLA){
		switch(r_length){
		case 0: /* byte */
		    value += get_arch_byte(p);
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 1 "
			    "byte)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value += get_arch_short(p);
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 2 "
			    "bytes)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value += get_arch_long(p);
		    set_arch_long(p, value);
		    break;
		}
	    }
	    /*
	     * Do SPARC specific relocation based on the r_type.
	     */
	    else {
		switch(r_type) {
		case SPARC_RELOC_PB_LA_PTR:
		    /* note r_value is incremented by vmslide above */
		    break;
		case SPARC_RELOC_HI22:
		    instruction = get_arch_long(p);
		    immediate = ((instruction & 0x3fffff) << 10) | other_half;
		    immediate += value;
		    instruction = (instruction & 0xffc00000) |
				  ((immediate >> 10) & 0x3fffff);
		    set_arch_long(p, instruction);
		    other_half = immediate & 0x3ff;
		    break;

		case SPARC_RELOC_LO10:
		    instruction = get_arch_long(p);
		    immediate = (instruction & 0x3ff) | (other_half << 10);
		    immediate += value;
		    instruction = (instruction & 0xfffffc00) | 
					(immediate & 0x3ff);
		    set_arch_long(p, instruction);
		    other_half = (immediate >> 10) & 0x3fffff;
		    break;

		case SPARC_RELOC_WDISP22:
		    instruction = get_arch_long(p);
		    immediate = (instruction & 0x3fffff);
		    if((immediate & 0x200000) != 0)
			    immediate |= 0xffc00000;
		    immediate <<= 2;
		    immediate += value;
		    if((immediate & 0xff800000) != 0xff800000 &&
				    (immediate & 0xff800000) != 0x00) {
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u displacement too "
			    "large to fit)", arch->file_name, arch_name, i);
		    }
		    immediate >>= 2;
		    instruction = (instruction & 0xffc00000) | 
				    (immediate & 0x3fffff);
		    set_arch_long(p, instruction);
		    break;

		case SPARC_RELOC_WDISP30:
		    instruction = get_arch_long(p);
		    immediate = (instruction & 0x3fffffff);
		    immediate <<= 2;
		    immediate += value;
		    immediate >>= 2;
		    instruction = (instruction & 0xc0000000) |
				    (immediate & 0x3fffffff);
		    set_arch_long(p, instruction);
		    break;

		default:
		    error("malformed file: %s (local relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		    break;
		}
	    }

	    /*
	     * Update the parts of the relocation entries that are effected by
	     * sliding this to a different address.
	     */
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc->r_value = r_value;
	    }
	    if(pair_r_type == SPARC_RELOC_PAIR){
		if((pair_reloc->r_address & R_SCATTERED) != 0)
		    spair_reloc->r_address = other_half;
		else
		    pair_reloc->r_address = other_half;
	    }
	}
}

/*
 * update_ppc_local_relocs() updates of the items with local relocation
 * entries for the ppc architecture.
 */
static
void
update_ppc_local_relocs(
uint32_t vmslide)
{
    uint32_t i, r_address, r_pcrel, r_length, r_value, value;
    char *p;
    uint32_t instruction, immediate;
    enum bool no_sect;
    struct scattered_relocation_info *sreloc;
    struct relocation_info *pair_reloc;
    struct scattered_relocation_info *spair_reloc;
    enum reloc_type_ppc r_type, pair_r_type;
    uint32_t other_half;

	sreloc = NULL;
	pair_reloc = NULL;
	spair_reloc = NULL;
	other_half = 0;
	r_value = 0;

	for(i = 0; i < arch_nlocrel; i++){
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)
			 (arch_locrelocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_value = sreloc->r_value;
		r_type = (enum reloc_type_ppc)sreloc->r_type;
		no_sect = FALSE;
	    }
	    else{
		r_address = arch_locrelocs[i].r_address;
		r_pcrel = arch_locrelocs[i].r_pcrel;
		r_length = arch_locrelocs[i].r_length;
		r_type = (enum reloc_type_ppc)arch_locrelocs[i].r_type;
		no_sect = arch_locrelocs[i].r_symbolnum == NO_SECT;
	    }
	    /*
	     * If this relocation type has a pair break out it's fields.
	     */
	    pair_r_type = 0;
	    if(r_type == PPC_RELOC_HI16 || r_type == PPC_RELOC_LO16 ||
	       r_type == PPC_RELOC_HA16 || r_type == PPC_RELOC_LO14){
		if(i + 1 == arch_nlocrel){
		    error("malformed file: %s (missing pair local "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		pair_reloc = arch_locrelocs + i + 1;
		if((pair_reloc->r_address & R_SCATTERED) != 0){
		    spair_reloc = (struct scattered_relocation_info *)
				    pair_reloc;
		    pair_r_type = spair_reloc->r_type;
		    other_half  = spair_reloc->r_address;
		}
		else{
		    pair_r_type = pair_reloc->r_type;
		    other_half  = pair_reloc->r_address;
		}
		i++;
		if(pair_r_type != PPC_RELOC_PAIR){
		    error("malformed file: %s (pair local relocation entry "
			  "for entry %u is not of r_type PPC_RELOC_PAIR) "
			  "(for architecture %s)", arch->file_name, i,
			  arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If this relocation entry pc relative, which means the value of
	     * the pc will get added to it when it is executed, the item being
	     * relocated has the value of the pc subtracted from it.  So to
	     * relocate this, the amount the image is slid has to be subtracted
	     * from it also.
	     */
	    value = 0;
	    if(r_pcrel)
		value -= vmslide;
	    /*
	     * Since this is a local relocation entry and all sections are
	     * moving by the same amount everything gets moved except those
	     * things that are defined that are not in a section.  We are
	     * counting on not seeing any section difference relocation entries
	     * and pcrel section based (which would work but be nops).
	     */
	    if(no_sect == FALSE){
		value += vmslide;
		r_value += vmslide;
	    }

	    p = contents_pointer_for_vmaddr(r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
				 1 << r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for local relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(r_type == PPC_RELOC_VANILLA){
		switch(r_length){
		case 0: /* byte */
		    value += get_arch_byte(p);
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 1 "
			    "byte)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value += get_arch_short(p);
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 2 "
			    "bytes)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value += get_arch_long(p);
		    set_arch_long(p, value);
		    break;
		}
	    }
	    /*
	     * Do ppc specific relocation based on the r_type.
	     */
	    else{
		switch(r_type){
		case PPC_RELOC_PB_LA_PTR:
		    /* note r_value is incremented by vmslide above */
		    break;
		case PPC_RELOC_HI16:
		    instruction = get_arch_long(p);
		    immediate = ((instruction & 0xffff) << 16) | other_half;
		    immediate += value;
		    instruction = (instruction & 0xffff0000) |
				 ((immediate >> 16) & 0xffff);
		    set_arch_long(p, instruction);
		    other_half = immediate & 0xffff;
		    break;
		case PPC_RELOC_LO16:
		    instruction = get_arch_long(p);
		    immediate = (other_half << 16) | (instruction & 0xffff);
		    immediate += value;
		    instruction = (instruction & 0xffff0000) |
				  (immediate & 0xffff);
		    set_arch_long(p, instruction);
		    other_half = (immediate >> 16) & 0xffff;
		    break;
		case PPC_RELOC_HA16:
		    instruction = get_arch_long(p);
		    if((other_half & 0x00008000) != 0)
			immediate = ((instruction & 0xffff) << 16) +
				    (0xffff0000 + other_half);
		    else
			immediate = ((instruction & 0xffff) << 16) +
				    (other_half);
		    immediate += value;
		    if((immediate & 0x00008000) != 0)
			instruction = (instruction & 0xffff0000) |
				      (((immediate + 0x00008000) >> 16) & 0xffff);
		    else
			instruction = (instruction & 0xffff0000) |
				      ((immediate >> 16) & 0xffff);
		    set_arch_long(p, instruction);
		    other_half = immediate & 0xffff;
		    break;
		case PPC_RELOC_LO14:
		    instruction = get_arch_long(p);
		    immediate = (other_half << 16) | (instruction & 0xfffc);
		    immediate += value;
		    if((immediate & 0x3) != 0){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocated value "
			    "not a multiple of 4 bytes for local relocation "
			    "entry %u", arch->file_name, arch_name, i);
		    }
		    instruction = (instruction & 0xffff0003) |
				  (immediate & 0xfffc);
		    set_arch_long(p, instruction);
		    other_half = (immediate >> 16) & 0xffff;
		    break;
		case PPC_RELOC_BR14:
		    instruction = get_arch_long(p);
		    immediate = instruction & 0xfffc;
		    if((immediate & 0x8000) != 0)
			immediate |= 0xffff0000;
		    immediate += value;
		    if((immediate & 0x3) != 0){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocated value "
			    "not a multiple of 4 bytes for local relocation "
			    "entry %u", arch->file_name, arch_name, i);
		    }
		    if((immediate & 0xfffe0000) != 0xfffe0000 &&
		       (immediate & 0xfffe0000) != 0x00000000){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u displacement "
			    "too large to fit)", arch->file_name, arch_name, i);
		    }
		    instruction = (instruction & 0xffff0003) |
				  (immediate & 0xfffc);
		    set_arch_long(p, instruction);
		    break;
		case PPC_RELOC_BR24:
		    instruction = get_arch_long(p);
		    immediate = instruction & 0x03fffffc;
		    if((immediate & 0x02000000) != 0)
			immediate |= 0xfc000000;
		    immediate += value;
		    if((immediate & 0x3) != 0){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocated value "
			    "not a multiple of 4 bytes for local relocation "
			    "entry %u", arch->file_name, arch_name, i);
		    }
		    if((immediate & 0xfe000000) != 0xfe000000 &&
		       (immediate & 0xfe000000) != 0x00000000){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u displacement too "
			    "large to fit)", arch->file_name, arch_name, i);
		    }
		    instruction = (instruction & 0xfc000003) |
				  (immediate & 0x03fffffc);
		    set_arch_long(p, instruction);
		    break;
		default:
		    error("malformed file: %s (local relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		    break;
		}
	    }

	    /*
	     * Update the parts of the relocation entries that are effected by
	     * sliding this to a different address.
	     */
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc->r_value = r_value;
	    }
	    if(pair_r_type == PPC_RELOC_PAIR){
		if((pair_reloc->r_address & R_SCATTERED) != 0)
		    spair_reloc->r_address = other_half;
		else
		    pair_reloc->r_address = other_half;
	    }
	}
}

/*
 * update_arm_local_relocs() updates of the items with local relocation
 * entries for the arm architecture.
 */
static
void
update_arm_local_relocs(
uint32_t vmslide)
{
    uint32_t i, r_address, r_pcrel, r_length, r_value, value;
    char *p;
    enum bool no_sect;
    struct scattered_relocation_info *sreloc;
    struct relocation_info *pair_reloc;
    struct scattered_relocation_info *spair_reloc;
    enum reloc_type_arm r_type;
    uint32_t other_half;

	sreloc = NULL;
	pair_reloc = NULL;
	spair_reloc = NULL;
	other_half = 0;
	r_value = 0;

	for(i = 0; i < arch_nlocrel; i++){
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)
			 (arch_locrelocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_value = sreloc->r_value;
		r_type = (enum reloc_type_arm)sreloc->r_type;
		no_sect = FALSE;
	    }
	    else{
		r_address = arch_locrelocs[i].r_address;
		r_pcrel = arch_locrelocs[i].r_pcrel;
		r_length = arch_locrelocs[i].r_length;
		r_type = (enum reloc_type_arm)arch_locrelocs[i].r_type;
		no_sect = arch_locrelocs[i].r_symbolnum == NO_SECT;
	    }

	    /*
	     * If this relocation entry is pc relative, which means the value of
	     * the pc will get added to it when it is executed, the item being
	     * relocated has the value of the pc subtracted from it.  So to
	     * relocate this, the amount the image is slid has to be subtracted
	     * from it also.
	     */
	    value = 0;
	    if(r_pcrel)
		    value -= vmslide;
	    /*
	     * Since this is a local relocation entry and all sections are
	     * moving by the same amount everything gets moved except those
	     * things that are defined that are not in a section.  We are
	     * counting on not seeing any section difference relocation entries
	     * and pcrel section based (which would work but be nops).
	     */
	    if(no_sect == FALSE){
		value += vmslide;
		r_value += vmslide;
	    }

	    p = contents_pointer_for_vmaddr(r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
				 1 << r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for local relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(r_type == ARM_RELOC_VANILLA){
		switch(r_length){
		case 0: /* byte */
		    value += get_arch_byte(p);
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 1 "
			    "byte)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value += get_arch_short(p);
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(local relocation entry %u does not fit in 2 "
			    "bytes)", arch->file_name, arch_name, i);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value += get_arch_long(p);
		    set_arch_long(p, value);
		    break;
		}
	    }
	    /*
	     * Do arm specific relocation based on the r_type.
	     */
	    else{
		switch(r_type){
		case ARM_RELOC_PB_LA_PTR:
		    /* note r_value is incremented by vmslide above */
		    break;
		case ARM_THUMB_RELOC_BR22:
		case ARM_RELOC_BR24:
            if (!r_pcrel) {
                error("prebinding can't be redone for: %s (for "
                    "architecture %s) because the relocation is not "
                    "pc-relative",
                    arch->file_name, arch_name);
                redo_exit(2);
            }
            if (value != 0) {
                error("prebinding can't be redone for: %s (for "
                    "architecture %s) because the slide of a pc-relative "
                    "relocation is not zero",
                    arch->file_name, arch_name);
                redo_exit(2);
            }
            /* hooray, nothing to do! */
		    break;
		default:
		    error("malformed file: %s (local relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		    break;
		}
	    }

	    /*
	     * Update the parts of the relocation entries that are effected by
	     * sliding this to a different address.
	     */
	    if((arch_locrelocs[i].r_address & R_SCATTERED) != 0){
		sreloc->r_value = r_value;
	    }
	}
}

/*
 * update_external_relocs() drives the updating of the items with external
 * relocation entries for the current arch.
 */
static
void
update_external_relocs(
uint32_t vmslide)
{
	switch(arch->object->mh_cputype){
	case CPU_TYPE_MC680x0:
	    update_generic_external_relocs(vmslide);
	    break;
	case CPU_TYPE_I386:
	    update_generic_external_relocs(vmslide);
	    break;
	case CPU_TYPE_HPPA:
	    update_hppa_external_relocs(vmslide);
	    break;
	case CPU_TYPE_SPARC:
	    update_sparc_external_relocs(vmslide);
	    break;
	case CPU_TYPE_POWERPC:
	    update_ppc_external_relocs(vmslide);
	    break;
	case CPU_TYPE_ARM:
	    update_arm_external_relocs(vmslide);
	    break;
	default:
	    error("can't redo prebinding for: %s (for architecture %s) because "
		  "of unknown cputype", arch->file_name, arch_name);
	}
}

/*
 * update_generic_external_relocs() updates of the items with external
 * relocation entries for the architectures that use generic relocation entries
 * (the i386 and m68k architectures).  It only deals with external relocation
 * entries that are using prebound undefined symbols.
 */
static
void
update_generic_external_relocs(
uint32_t vmslide)
{
    uint32_t i, value, symbol_slide;
    char *name, *p;
    struct nlist *defined_symbol, *arch_symbol;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t mh_flags;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;
    
	for(i = 0; i < arch_nextrel; i++){
	    /* check the r_symbolnum field */
	    if(arch_extrelocs[i].r_symbolnum > arch_nsyms){
		error("malformed file: %s (bad symbol table index for "
		      "external relocation entry %u) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }

	    /*
	     * If the symbol this relocation entry is refering to is not in a
	     * section then its slide is 0 otherwise it is slid by the the
	     * vmslide.
	     */ 
	    arch_symbol = arch_symbols + arch_extrelocs[i].r_symbolnum;
	    if(arch_symbol->n_sect == NO_SECT)
		symbol_slide = 0;
	    else
		symbol_slide = vmslide;

	    /*
	     * If this is a prebound undefined symbol look up the symbol being
	     * referenced by this relocation entry to get the defined symbol's
	     * value to be used.  If it is not a prebound undefined symbol use
	     * the arch_symbol.
	     */
	    name = arch_strings + arch_symbol->n_un.n_strx;
	    if((arch_symbol->n_type & N_TYPE) == N_PBUD){
                if(unprebinding == TRUE){
                    /* 
                     * If we are unprebinding, we need to use the newly zeroed
                     * symbol table entry, rather than looking up the symbol
                     */
                     defined_symbol = arch->object->output_symbols +
				      arch_extrelocs[i].r_symbolnum;
                }
                else{
                    lookup_symbol(name,
                                get_primary_lib(ARCH_LIB, arch_symbol),
                                get_weak(arch_symbol),
                                &defined_symbol, &module_state, &lib,
                                NULL, NULL, NO_INDR_LOOP);
                }
	    }
	    else{
		defined_symbol = arch_symbol;
	    }

	    p = contents_pointer_for_vmaddr(arch_extrelocs[i].r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << arch_extrelocs[i].r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for external relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    switch(arch_extrelocs[i].r_length){
	    case 0: /* byte */
		value = get_arch_byte(p);
		if(unprebinding == TRUE)
		    value = value - arch_symbol->n_value;
		else{
		    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			value = value + defined_symbol->n_value + symbol_slide;
		    else
			value = (value - arch_symbol->n_value) +
				defined_symbol->n_value + symbol_slide;
		    if(arch_extrelocs[i].r_pcrel)
			value -= vmslide;
		}
		if( (value & 0xffffff00) &&
		   ((value & 0xffffff80) != 0xffffff80)){
		    error("prebinding can't be redone for: %s (for architecture"
			" %s) because of relocation overflow (external "
			"relocation for symbol %s does not fit in 1 byte)",
			arch->file_name, arch_name, name);
		    redo_exit(2);
		}
		set_arch_byte(p, value);
		break;
	    case 1: /* word (2 byte) */
		value = get_arch_short(p);
		if(unprebinding == TRUE)
		    value = value - arch_symbol->n_value;
		else{
		    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			value = value + defined_symbol->n_value + symbol_slide;
		    else
			value = (value - arch_symbol->n_value) +
				defined_symbol->n_value + symbol_slide;
		    if(arch_extrelocs[i].r_pcrel)
			value -= vmslide;
		}
		if( (value & 0xffff0000) &&
		   ((value & 0xffff8000) != 0xffff8000)){
		    error("prebinding can't be redone for: %s (for architecture"
			" %s) because of relocation overflow (external "
			"relocation for symbol %s does not fit in 2 bytes)",
			arch->file_name, arch_name, name);
		    redo_exit(2);
		}
		set_arch_short(p, value);
		break;
	    case 2: /* long (4 byte) */
		value = get_arch_long(p);
		if(unprebinding == TRUE)
		    value = value - arch_symbol->n_value;
		else{
		    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			value = value + defined_symbol->n_value + symbol_slide;
		    else
			value = (value - arch_symbol->n_value) +
		    defined_symbol->n_value + symbol_slide;
		    if(arch_extrelocs[i].r_pcrel)
			value -= vmslide;
		}
		set_arch_long(p, value);
		break;
	    default:
		error("malformed file: %s (external relocation entry "
		      "%u has bad r_length) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }
	}
}

/*
 * update_hppa_external_relocs() updates of the items with external relocation
 * entries for the hppa architecture.  It only deals with external relocation
 * entries that are using prebound undefined symbols.
 */
static
void
update_hppa_external_relocs(
uint32_t vmslide)
{
    uint32_t i, value, symbol_slide;
    char *name, *p;
    struct nlist *defined_symbol, *arch_symbol;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t instruction, immediate;
    uint32_t other_half;
    uint32_t hi21, lo14;
    uint32_t w, w1, w2;
    uint32_t mh_flags;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;

	for(i = 0; i < arch_nextrel; i++){
	    /* check the r_symbolnum field */
	    if(arch_extrelocs[i].r_symbolnum > arch_nsyms){
		error("malformed file: %s (bad symbol table index for "
		      "external relocation entry %u) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }
	    /* check to see if it needs a pair and has a correct one */
	    if(arch_extrelocs[i].r_type == HPPA_RELOC_HI21 ||
	       arch_extrelocs[i].r_type == HPPA_RELOC_LO14 ||
	       arch_extrelocs[i].r_type == HPPA_RELOC_BR17){
		if(i + 1 == arch_nextrel){
		    error("malformed file: %s (missing pair external "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		if(arch_extrelocs[i + 1].r_type != HPPA_RELOC_PAIR){
		    error("malformed file: %s (pair external relocation entry "
			  "for entry %u is not of r_type HPPA_RELOC_PAIR) (for"
			  " architecture %s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If the symbol this relocation entry is refering to is not in a
	     * section then its slide is 0 otherwise it is slid by the the
	     * vmslide.
	     */ 
	    arch_symbol = arch_symbols + arch_extrelocs[i].r_symbolnum;
	    if(arch_symbol->n_sect == NO_SECT)
		symbol_slide = 0;
	    else
		symbol_slide = vmslide;

	    /*
	     * If this is a prebound undefined symbol look up the symbol being
	     * referenced by this relocation entry to get the defined symbol's
	     * value to be used.  If it is not a prebound undefined symbol use
	     * the arch_symbol.
	     */
	    name = arch_strings + arch_symbol->n_un.n_strx;
	    if((arch_symbol->n_type & N_TYPE) == N_PBUD){
                if(unprebinding == TRUE){
                    /* 
                     * If we are unprebinding, we need to use the newly zeroed
                     * symbol table entry, rather than looking up the symbol
                     */
                     defined_symbol = arch->object->output_symbols +
				      arch_extrelocs[i].r_symbolnum;
                }
                else{
                    lookup_symbol(name,
                                get_primary_lib(ARCH_LIB, arch_symbol),
                                get_weak(arch_symbol),
                                &defined_symbol, &module_state, &lib,
                                NULL, NULL, NO_INDR_LOOP);
                }
	    }
	    else
		defined_symbol = arch_symbol;

	    p = contents_pointer_for_vmaddr(arch_extrelocs[i].r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << arch_extrelocs[i].r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for external relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(arch_extrelocs[i].r_type == HPPA_RELOC_VANILLA){
		switch(arch_extrelocs[i].r_length){
		case 0: /* byte */
		    value = get_arch_byte(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 1 byte)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value = get_arch_short(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 2 bytes)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value = get_arch_long(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    set_arch_long(p, value);
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has bad r_length) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
	    }
	    /*
	     * Do hppa specific relocation based on the r_type.
	     */
	    else{
		instruction = get_arch_long(p);
		switch(arch_extrelocs[i].r_type){
		case HPPA_RELOC_HI21:
		    other_half  = arch_extrelocs[i + 1].r_address;
		    immediate = sign_ext(other_half, 14) + 
		               (assemble_21(instruction & 0x1fffff) << 11);
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
			    immediate -= vmslide;
		    }
		    calc_hppa_HILO(immediate, 0, &hi21, &lo14);
		    instruction = (instruction & 0xffe00000) |
				  dis_assemble_21(hi21 >> 11);
		    arch_extrelocs[i + 1].r_address = lo14 & 0x3fff;
		    break;
		case HPPA_RELOC_LO14:
		    other_half  = arch_extrelocs[i + 1].r_address;
		    immediate = low_sign_ext(instruction & 0x3fff, 14) +
		    	        (other_half << 11);
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
			    immediate -= vmslide;
		    }
		    calc_hppa_HILO(immediate, 0, &hi21, &lo14);
		    lo14 = low_sign_unext(lo14, 14);
		    instruction = (instruction & 0xffffc000) |
				  (lo14 & 0x3fff);
		    arch_extrelocs[i + 1].r_address = hi21 >> 11;
		    break;
		case HPPA_RELOC_BR17:
		    other_half  = arch_extrelocs[i + 1].r_address;
		    immediate = assemble_17((instruction & 0x1f0000) >> 16,
			                    (instruction & 0x1ffc) >> 2,
				             instruction & 1);
		    immediate = (sign_ext(immediate, 17) << 2) +
				(other_half << 11);
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
			    immediate -= vmslide;
		    }
		    calc_hppa_HILO(immediate, 0, &hi21, &lo14);
		    lo14 >>= 2;
		    dis_assemble_17(lo14, &w1, &w2, &w);
		    instruction = (instruction & 0xffe0e002) |
				  (w1 << 16) | (w2 << 2) | w;
		    arch_extrelocs[i + 1].r_address = hi21 >> 11;
		    break;
		case HPPA_RELOC_BL17:
		    immediate = assemble_17((instruction & 0x1f0000) >> 16,
			                    (instruction & 0x1ffc) >> 2,
				             instruction & 1);
		    if((immediate & 0x10000) != 0)
			immediate |= 0xfffe0000;
		    immediate <<= 2;
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
			    immediate -= vmslide;
		    }
		    if(U_ABS(immediate) > 0x3ffff){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s displacement "
			    "too large to fit)", arch->file_name, arch_name,
			    name);
			redo_exit(2);
		    }
		    immediate >>= 2;
		    dis_assemble_17(immediate, &w1, &w2, &w);
		    instruction = (instruction & 0xffe0e002) |
				  (w1 << 16) | (w2 << 2) | w;
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		set_arch_long(p, instruction);
	    }
	    /*
	     * If the relocation entry had a pair step over it.
	     */
	    if(arch_extrelocs[i].r_type == HPPA_RELOC_HI21 ||
	       arch_extrelocs[i].r_type == HPPA_RELOC_LO14 ||
	       arch_extrelocs[i].r_type == HPPA_RELOC_BR17)
		i++;
	}
}

/*
 * update_sparc_external_relocs() updates of the items with external relocation
 * entries for the sparc architecture.  It only deals with external relocation
 * entries that are using prebound undefined symbols.
 */
static
void
update_sparc_external_relocs(
uint32_t vmslide)
{
    uint32_t i, value, symbol_slide;
    char *name, *p;
    struct nlist *defined_symbol, *arch_symbol;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t instruction, immediate;
    uint32_t other_half;
    uint32_t mh_flags;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;

	for(i = 0; i < arch_nextrel; i++){
	    /* check the r_symbolnum field */
	    if(arch_extrelocs[i].r_symbolnum > arch_nsyms){
		error("malformed file: %s (bad symbol table index for "
		      "external relocation entry %u) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }
	    /* check to see if it needs a pair and has a correct one */
	    if(arch_extrelocs[i].r_type == SPARC_RELOC_LO10 ||
	       arch_extrelocs[i].r_type == SPARC_RELOC_HI22){
		if(i + 1 == arch_nextrel){
		    error("malformed file: %s (missing pair external "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		if(arch_extrelocs[i + 1].r_type != SPARC_RELOC_PAIR){
		    error("malformed file: %s (pair external relocation entry "
			  "for entry %u is not of r_type SPARC_RELOC_PAIR) "
			  "(for architecture %s)", arch->file_name, i,
			  arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If the symbol this relocation entry is refering to is not in a
	     * section then its slide is 0 otherwise it is slid by the the
	     * vmslide.
	     */ 
	    arch_symbol = arch_symbols + arch_extrelocs[i].r_symbolnum;
	    if(arch_symbol->n_sect == NO_SECT)
		symbol_slide = 0;
	    else
		symbol_slide = vmslide;

	    /*
	     * If this is a prebound undefined symbol look up the symbol being
	     * referenced by this relocation entry to get the defined symbol's
	     * value to be used.  If it is not a prebound undefined symbol use
	     * the arch_symbol.
	     */
	    name = arch_strings + arch_symbol->n_un.n_strx;
	    if((arch_symbol->n_type & N_TYPE) == N_PBUD){
                if(unprebinding == TRUE){
                    /* 
                     * If we are unprebinding, we need to use the newly zeroed
                     * symbol table entry, rather than looking up the symbol
                     */
                     defined_symbol = arch->object->output_symbols +
				      arch_extrelocs[i].r_symbolnum;
                }
                else{
                    lookup_symbol(name,
                                get_primary_lib(ARCH_LIB, arch_symbol),
                                get_weak(arch_symbol),
                                &defined_symbol, &module_state, &lib,
                                NULL, NULL, NO_INDR_LOOP);
                }
	    }
	    else
		defined_symbol = arch_symbol;

	    p = contents_pointer_for_vmaddr(arch_extrelocs[i].r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
					    1 << arch_extrelocs[i].r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for external relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(arch_extrelocs[i].r_type == SPARC_RELOC_VANILLA){
		switch(arch_extrelocs[i].r_length){
		case 0: /* byte */
		    value = get_arch_byte(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 1 byte)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value = get_arch_short(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 2 bytes)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value = get_arch_long(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    set_arch_long(p, value);
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has bad r_length) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
	    }
	    /*
	     * Do SPARC specific relocation based on the r_type.
	     */
	    else{
		instruction = get_arch_long(p);
		switch(arch_extrelocs[i].r_type){
		case SPARC_RELOC_HI22:
		    other_half = (arch_extrelocs[i + 1].r_address) & 0x3ff;
		    immediate = ((instruction & 0x3fffff) << 10) | other_half;
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
		    }
		    instruction = (instruction & 0xffc00000) |
				  ((immediate >> 10) & 0x3fffff);
		    arch_extrelocs[i + 1].r_address = immediate & 0x3ff;
		    break;
		case SPARC_RELOC_LO10:
		    other_half = ((arch_extrelocs[i + 1].r_address) >> 10) &
				 0x3fffff;
		    immediate = (instruction & 0x3ff) | (other_half << 10);
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
		    }
		    instruction = (instruction & 0xfffffc00) |
				  (immediate & 0x3ff);
		    arch_extrelocs[i + 1].r_address =
				  (immediate >> 10) & 0x3fffff;
		    break;

		case SPARC_RELOC_WDISP22:
		    immediate = (instruction & 0x3fffff);
		    if((immediate & 0x200000) != 0)
			    immediate |= 0xffc00000;
		    immediate <<= 2;
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
		    }
		    if((immediate & 0xff800000) != 0xff800000 &&
				    (immediate & 0xff800000) != 0x00) {
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s displacement "
			    "too large to fit)", arch->file_name, arch_name,
			    name);
			redo_exit(2);
		    }
		    immediate >>= 2;
		    instruction = (instruction & 0xffc00000) | 
				    (immediate & 0x3fffff);
		    break;
		case SPARC_RELOC_WDISP30:
		    immediate = (instruction & 0x3fffffff);
		    immediate <<= 2;
		    if(unprebinding == TRUE)
			immediate -= arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    immediate += defined_symbol->n_value + symbol_slide;
			else
			    immediate += defined_symbol->n_value + symbol_slide 
					 - arch_symbol->n_value;
			if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
		    }
		    immediate >>= 2;
		    instruction = (instruction & 0xc0000000) |
				    (immediate & 0x3fffffff);
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has unknown r_type) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		set_arch_long(p, instruction);
	    }
	    /*
	     * If the relocation entry had a pair step over it.
	     */
	    if(arch_extrelocs[i].r_type == SPARC_RELOC_LO10 ||
	       arch_extrelocs[i].r_type == SPARC_RELOC_HI22)
		i++;
	}
}

/*
 * update_ppc_external_relocs() updates of the items with external relocation
 * entries for the ppc architecture.  It only deals with external relocation
 * entries that are using prebound undefined symbols.
 */
static
void
update_ppc_external_relocs(
uint32_t vmslide)
{
    uint32_t i, value, symbol_slide;
    char *name, *p;
    struct nlist *defined_symbol, *arch_symbol;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t instruction, immediate;
    uint32_t other_half, br14_disp_sign;
    uint32_t mh_flags;

        if(arch->object->mh != NULL)
            mh_flags = arch->object->mh->flags;
        else
            mh_flags = arch->object->mh64->flags;

	for(i = 0; i < arch_nextrel; i++){
	    /* check the r_symbolnum field */
	    if(arch_extrelocs[i].r_symbolnum > arch_nsyms){
		error("malformed file: %s (bad symbol table index for "
		      "external relocation entry %u) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }
	    /* check to see if it needs a pair and has a correct one */
	    if(arch_extrelocs[i].r_type == PPC_RELOC_HI16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_LO16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_HA16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_LO14){
		if(i + 1 == arch_nextrel){
		    error("malformed file: %s (missing pair external "
			  "relocation entry for entry %u) (for architecture "
			  "%s)", arch->file_name, i, arch_name);
		    redo_exit(2);
		}
		if(arch_extrelocs[i + 1].r_type != PPC_RELOC_PAIR){
		    error("malformed file: %s (pair external relocation entry "
			  "for entry %u is not of r_type PPC_RELOC_PAIR) "
			  "(for architecture %s)", arch->file_name, i,
			  arch_name);
		    redo_exit(2);
		}
	    }

	    /*
	     * If the symbol this relocation entry is refering to is not in a
	     * section then its slide is 0 otherwise it is slid by the the
	     * vmslide.
	     */ 
	    arch_symbol = arch_symbols + arch_extrelocs[i].r_symbolnum;
	    if(arch_symbol->n_sect == NO_SECT)
		symbol_slide = 0;
	    else
		symbol_slide = vmslide;

	    /*
	     * If this is a prebound undefined symbol look up the symbol being
	     * referenced by this relocation entry to get the defined symbol's
	     * value to be used.  If it is not a prebound undefined symbol use
	     * the arch_symbol.
	     */
	    name = arch_strings + arch_symbol->n_un.n_strx;
	    if((arch_symbol->n_type & N_TYPE) == N_PBUD)
		if(unprebinding == TRUE){
		    /* 
		     * If we are unprebinding, we need to use the newly zeroed
		     * symbol table entry, rather than looking up the symbol
		     */
		     defined_symbol = arch->object->output_symbols + 
				      arch_extrelocs[i].r_symbolnum;
		}
		else{
		    lookup_symbol(name,
				get_primary_lib(ARCH_LIB, arch_symbol),
				get_weak(arch_symbol),
				&defined_symbol, &module_state, &lib,
				NULL, NULL, NO_INDR_LOOP);
		}
	    else
		defined_symbol = arch_symbol;
	
	    p = contents_pointer_for_vmaddr(arch_extrelocs[i].r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
				 1 << arch_extrelocs[i].r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for external relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(arch_extrelocs[i].r_type == PPC_RELOC_VANILLA){
		switch(arch_extrelocs[i].r_length){
		case 0: /* byte */
		    value = get_arch_byte(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 1 byte)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value = get_arch_short(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 2 bytes)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value = get_arch_long(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				   defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    set_arch_long(p, value);
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has bad r_length) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
	    }
	    /*
	     * Do PPC specific relocation based on the r_type.
	     */
	    else{
		instruction = get_arch_long(p);
		switch(arch_extrelocs[i].r_type){
		    case PPC_RELOC_HI16:
			other_half = (arch_extrelocs[i + 1].r_address) & 0xffff;
			immediate = ((instruction & 0xffff) << 16) | other_half;
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value + 
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			instruction = (instruction & 0xffff0000) |
				      ((immediate >> 16) & 0xffff);
			arch_extrelocs[i + 1].r_address = immediate & 0xffff;
			break;
		    case PPC_RELOC_LO16:
			other_half = (arch_extrelocs[i + 1].r_address) & 0xffff;
			immediate = (other_half << 16) | (instruction & 0xffff);
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value +
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			instruction = (instruction & 0xffff0000) |
				      (immediate & 0xffff);
			arch_extrelocs[i + 1].r_address =
					  (immediate >> 16) & 0xffff;
			break;
		    case PPC_RELOC_HA16:
			other_half = (arch_extrelocs[i + 1].r_address) & 0xffff;
			immediate = ((instruction & 0xffff) << 16) | other_half;
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value +
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			if((immediate & 0x00008000) != 0)
			    instruction = (instruction & 0xffff0000) |
					  (((immediate + 0x00008000) >> 16) &
					    0xffff);
			else
			    instruction = (instruction & 0xffff0000) |
					  ((immediate >> 16) & 0xffff);
			arch_extrelocs[i + 1].r_address = immediate & 0xffff;
			break;
		    case PPC_RELOC_LO14:
			other_half = (arch_extrelocs[i + 1].r_address) & 0xffff;
			immediate = (other_half << 16) | (instruction & 0xfffc);
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value +
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			if((immediate & 0x3) != 0){
			    error("prebinding can't be redone for: %s (for "
				"architecture %s) because of relocated value "
				"not a multiple of 4 bytes", arch->file_name,
				arch_name);
			    redo_exit(2);
			}
			instruction = (instruction & 0xffff0003) |
				      (immediate & 0xfffc);
			arch_extrelocs[i + 1].r_address =
				      (immediate >> 16) & 0xffff;
			break;
		    case PPC_RELOC_BR14:
			br14_disp_sign = (instruction & 0x8000);
			immediate = instruction & 0xfffc;
			if((immediate & 0x8000) != 0)
			    immediate |= 0xffff0000;
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value +
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			if((immediate & 0x3) != 0){
			    error("prebinding can't be redone for: %s (for "
				"architecture %s) because of relocated value "
				"not a multiple of 4 bytes", arch->file_name,
				arch_name);
			    redo_exit(2);
			}
			if((immediate & 0xfffe0000) != 0xfffe0000 &&
			   (immediate & 0xfffe0000) != 0x00000000){
			    error("prebinding can't be redone for: %s (for "
				"architecture %s) because of relocation "
				"overflow (external relocation for symbol %s "
				"displacement too large to fit)",
				arch->file_name, arch_name, name);
			    redo_exit(2);
			}
			instruction = (instruction & 0xffff0003) |
				      (immediate & 0xfffc);
			/*
			 * If this is a branch conditional B-form where
			 * the branch condition is not branch always and
			 * the sign of the displacement is different
			 * after relocation then flip the Y-bit to
			 * preserve the sense of the branch prediction. 
			 */
			if((instruction & 0xfc000000) == 0x40000000 &&
			  (instruction & 0x03e00000) != 0x02800000 &&
			   (instruction & 0x00008000) != br14_disp_sign)
			    instruction ^= (1 << 21);
			break;
		    case PPC_RELOC_BR24:
			immediate = instruction & 0x03fffffc;
			if((immediate & 0x02000000) != 0)
			    immediate |= 0xfc000000;
			if(unprebinding == TRUE)
			    immediate -= arch_symbol->n_value;
			else{
			    if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
				immediate += defined_symbol->n_value +
					     symbol_slide;
			    else
				immediate += defined_symbol->n_value +
					     symbol_slide -
					     arch_symbol->n_value;
			    if(arch_extrelocs[i].r_pcrel)
				immediate -= vmslide;
			}
			if((immediate & 0x3) != 0){
			    error("prebinding can't be redone for: %s (for "
				"architecture %s) because of relocated value "
				"not a multiple of 4 bytes", arch->file_name,
				arch_name);
			    redo_exit(2);
			}
			if((immediate & 0xfe000000) != 0xfe000000 &&
			  (immediate & 0xfe000000) != 0x00000000){
			    error("prebinding can't be redone for: %s (for "
				"architecture %s) because of relocation "
				"overflow (external relocation for symbol %s "
				"displacement too large to fit)",
				arch->file_name, arch_name, name);
			    redo_exit(2);
			}
			instruction = (instruction & 0xfc000003) |
				      (immediate & 0x03fffffc);
			break;
		    default:
			error("malformed file: %s (external relocation entry "
			      "%u has unknown r_type) (for architecture %s)",
			      arch->file_name, i, arch_name);
			redo_exit(2);
		    }
		    set_arch_long(p, instruction);
	    }
	    /*
	     * If the relocation entry had a pair step over it.
	     */
	    if(arch_extrelocs[i].r_type == PPC_RELOC_HI16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_LO16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_HA16 ||
	       arch_extrelocs[i].r_type == PPC_RELOC_LO14)
		i++;
	}
}

/*
 * update_arm_external_relocs() updates of the items with external relocation
 * entries for the arm architecture.  It only deals with external relocation
 * entries that are using prebound undefined symbols.
 */
static
void
update_arm_external_relocs(
uint32_t vmslide)
{
    uint32_t i, value, symbol_slide;
    char *name, *p;
    struct nlist *defined_symbol, *arch_symbol;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t mh_flags;

    if(arch->object->mh != NULL)
        mh_flags = arch->object->mh->flags;
    else
        mh_flags = arch->object->mh64->flags;

	for(i = 0; i < arch_nextrel; i++){
	    /* check the r_symbolnum field */
	    if(arch_extrelocs[i].r_symbolnum > arch_nsyms){
		error("malformed file: %s (bad symbol table index for "
		      "external relocation entry %u) (for architecture %s)",
		      arch->file_name, i, arch_name);
		redo_exit(2);
	    }

	    /*
	     * If the symbol this relocation entry is refering to is not in a
	     * section then its slide is 0 otherwise it is slid by the the
	     * vmslide.
	     */ 
	    arch_symbol = arch_symbols + arch_extrelocs[i].r_symbolnum;
	    if(arch_symbol->n_sect == NO_SECT)
		symbol_slide = 0;
	    else
		symbol_slide = vmslide;

	    /*
	     * If this is a prebound undefined symbol look up the symbol being
	     * referenced by this relocation entry to get the defined symbol's
	     * value to be used.  If it is not a prebound undefined symbol use
	     * the arch_symbol.
	     */
	    name = arch_strings + arch_symbol->n_un.n_strx;
	    if((arch_symbol->n_type & N_TYPE) == N_PBUD)
		if(unprebinding == TRUE){
		    /* 
		     * If we are unprebinding, we need to use the newly zeroed
		     * symbol table entry, rather than looking up the symbol
		     */
		     defined_symbol = arch->object->output_symbols + 
				      arch_extrelocs[i].r_symbolnum;
		}
		else{
		    lookup_symbol(name,
				get_primary_lib(ARCH_LIB, arch_symbol),
				get_weak(arch_symbol),
				&defined_symbol, &module_state, &lib,
				NULL, NULL, NO_INDR_LOOP);
		}
	    else
		defined_symbol = arch_symbol;
	
	    p = contents_pointer_for_vmaddr(arch_extrelocs[i].r_address +
				(arch_split_segs == TRUE ?
				 arch_segs_read_write_addr : arch_seg1addr),
				 1 << arch_extrelocs[i].r_length);
	    if(p == NULL){
		error("malformed file: %s (for architecture %s) (bad r_address"
		      " field for external relocation entry %u)",
		      arch->file_name, arch_name, i);
		redo_exit(2);
	    }

	    if(arch_extrelocs[i].r_type == ARM_RELOC_VANILLA){
		switch(arch_extrelocs[i].r_length){
		case 0: /* byte */
		    value = get_arch_byte(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffffff00) &&
		       ((value & 0xffffff80) != 0xffffff80)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 1 byte)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_byte(p, value);
		    break;
		case 1: /* word (2 byte) */
		    value = get_arch_short(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				    defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
		    if( (value & 0xffff0000) &&
		       ((value & 0xffff8000) != 0xffff8000)){
			error("prebinding can't be redone for: %s (for "
			    "architecture %s) because of relocation overflow "
			    "(external relocation for symbol %s does not fit "
			    "in 2 bytes)", arch->file_name, arch_name, name);
			redo_exit(2);
		    }
		    set_arch_short(p, value);
		    break;
		case 2: /* long (4 byte) */
		    value = get_arch_long(p);
		    if(unprebinding == TRUE)
			value = value - arch_symbol->n_value;
		    else{
			if((mh_flags & MH_PREBINDABLE) == MH_PREBINDABLE)
			    value = value + defined_symbol->n_value +
				    symbol_slide;
			else
			    value = (value - arch_symbol->n_value) +
				   defined_symbol->n_value + symbol_slide;
			if(arch_extrelocs[i].r_pcrel)
			    value -= vmslide;
		    }
            /*
             * If this is an ARM Thumb symbol, we need to set the low bit of
             * the symbol pointer so the hardware knows the function is
             * Thumb.
             */
            if (arch->object->mh_cputype == CPU_TYPE_ARM &&
			   (defined_symbol->n_desc & N_ARM_THUMB_DEF) != 0) {
		        set_arch_long(p, value | 1L);
    		} else {
    		    set_arch_long(p, value);
			}
		    break;
		default:
		    error("malformed file: %s (external relocation entry "
			  "%u has bad r_length) (for architecture %s)",
			  arch->file_name, i, arch_name);
		    redo_exit(2);
		}
	    }
	    /*
	     * Do arm specific relocation based on the r_type.
	     */
	    else{
		error("prebinding can't be redone for: %s (for "
		    "architecture %s) because of non-vanilla external relocation",
		    arch->file_name, arch_name);
		redo_exit(2);
	    }
	}
}

/*
 * contents_pointer_for_vmaddr() returns a pointer in memory for the vmaddr
 * of the current arch.  If the vmaddr is out of range return NULL.
 */
static
char *
contents_pointer_for_vmaddr(
uint32_t vmaddr,
uint32_t size)
{
    uint32_t i, j, header_size, offset;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s;
    uint32_t ncmds;
    
	lc = arch->object->load_commands;
        if(arch->object->mh != NULL){
            ncmds = arch->object->mh->ncmds;
	    header_size = sizeof(struct mach_header) +
			  arch->object->mh->sizeofcmds;
	}
        else{
            ncmds = arch->object->mh64->ncmds;
	    header_size = sizeof(struct mach_header_64) +
			  arch->object->mh64->sizeofcmds;
	}
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(vmaddr >= sg->vmaddr &&
		   vmaddr + size <= sg->vmaddr + sg->vmsize){
		    offset = vmaddr - sg->vmaddr;
		    if(offset + size > sg->filesize)
			return(NULL);
		    s = (struct section *)((char *)sg +
					   sizeof(struct segment_command));
		    for(j = 0 ; j < sg->nsects; j++){
			if(vmaddr >= s->addr &&
			   vmaddr + size <= s->addr + s->size){
			    /*
			     * Don't return pointers into the headers or
			     * link edit information for bad relocation info.
			     */
			    if(sg->fileoff + offset < header_size ||
			       sg->fileoff + offset >=
				arch->object->object_size -
				arch->object->input_sym_info_size)
				return(NULL);
			    return(arch->object->object_addr +
				   sg->fileoff + offset);
			}
			s++;
		    }
		    return(NULL);
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	return(NULL);
}

/*
 * update_symbol_pointers() updates the symbol pointers using the new and old
 * symbol table and the vmslide.
 */
static
void
update_symbol_pointers(
uint32_t vmslide)
{
    uint32_t i, j, k, section_type, symbol_pointer;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s;
    struct nlist *arch_symbol, *defined_symbol;
    char *name, *p;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t ncmds;
    
	/*
	 * For each symbol pointer section update the symbol pointers using
	 * prebound undefined symbols to their new values.
	 */
	lc = arch->object->load_commands;
        if(arch->object->mh != NULL)
            ncmds = arch->object->mh->ncmds;
        else
            ncmds = arch->object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		for(j = 0 ; j < sg->nsects ; j++){
		    section_type = s->flags & SECTION_TYPE;
		    if(section_type == S_NON_LAZY_SYMBOL_POINTERS ||
		       section_type == S_LAZY_SYMBOL_POINTERS){
			if(s->reserved1 + s->size / sizeof(uint32_t) >
			   arch_nindirectsyms){
			    error("malformed file: %s (for architecture %s) "
				"(indirect symbol table entries for section "
				"(%.16s,%.16s) extends past the end of the "
				"indirect symbol table)", arch->file_name,
				arch_name, s->segname, s->sectname);
			    redo_exit(2);
			}
			for(k = 0; k < s->size / sizeof(uint32_t); k++){
			    p = contents_pointer_for_vmaddr(
				s->addr + (k * sizeof(uint32_t)),
				sizeof(uint32_t));
			    if(p == NULL){
				error("malformed file: %s (for architecture "
				    "%s) (bad indirect section (%.16s,%.16s))",
				    arch->file_name, arch_name, s->segname,
				    s->sectname);
				redo_exit(2);
			    }
			    symbol_pointer = get_arch_long(p);

			    /*
			     * If this indirect symbol table entry is for a
 			     * non-lazy symbol pointer section for a defined
			     * symbol which is an absolute symbol skip it.
			     */
			    if(section_type == S_NON_LAZY_SYMBOL_POINTERS &&
			       (arch_indirect_symtab[s->reserved1 + k] & 
			        INDIRECT_SYMBOL_ABS) == INDIRECT_SYMBOL_ABS){
				continue;
			    }
			    /*
			     * If this indirect symbol table entry is for a
 			     * non-lazy symbol pointer section for a defined
			     * symbol which strip(1) has removed slide it.
			     */
			    if(section_type == S_NON_LAZY_SYMBOL_POINTERS &&
			       (arch_indirect_symtab[s->reserved1 + k] & 
			        INDIRECT_SYMBOL_LOCAL) ==INDIRECT_SYMBOL_LOCAL){
				set_arch_long(p, symbol_pointer + vmslide);
				continue;
			    }

			    /* check symbol index of indirect symbol table */
			    if(arch_indirect_symtab[s->reserved1 + k] >
			       arch_nsyms){
				error("malformed file: %s (for architecture "
				    "%s) (bad indirect symbol table entry %u)",
				    arch->file_name, arch_name, i);
				redo_exit(2);
			    }
	
			    /*
			     * If the symbol this indirect symbol table entry is
			     * refering to is not a prebound undefined symbol
			     * then if this indirect symbol table entry is for a
			     * symbol in a section slide it.
			     */ 
			    arch_symbol = arch_symbols +
				     arch_indirect_symtab[s->reserved1 + k];
			    if((arch_symbol->n_type & N_TYPE) != N_PBUD){
                                if(arch_symbol->n_sect != NO_SECT)
				    set_arch_long(p, symbol_pointer + vmslide);
				continue;
			    }

			    /*
			     * Look up the symbol being referenced by this
			     * indirect symbol table entry to get the defined
			     * symbol's value to be used.
			     */
			    name = arch_strings + arch_symbol->n_un.n_strx;
			    lookup_symbol(name,
					  get_primary_lib(ARCH_LIB,arch_symbol),
					  get_weak(arch_symbol),
					  &defined_symbol, &module_state,
					  &lib, NULL, NULL, NO_INDR_LOOP);
			    /*
			     * If this is an ARM Thumb symbol, we need to set
			     * the low bit of the symbol pointer so the hardware
			     * knows the function is Thumb.
			     */
			    if (arch->object->mh_cputype == CPU_TYPE_ARM &&
			        (defined_symbol->n_desc & N_ARM_THUMB_DEF) != 0)
			        set_arch_long(p, defined_symbol->n_value | 1L);
			    else
			        set_arch_long(p, defined_symbol->n_value);
			}
		    }
		    s++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}

/*
 * update_self_modifying_stubs() updates i386 5-byte self modifying stubs that
 * are JMP instructions to their indirect symbol address using the new and old
 * symbol table and the vmslide.
 */
static
void
update_self_modifying_stubs(
uint32_t vmslide)
{
    uint32_t i, j, k, section_type, displacement, symbol_slide;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s;
    struct nlist *arch_symbol, *defined_symbol;
    char *name, *p;
    enum link_state *module_state;
    struct lib *lib;
    uint32_t ncmds;
    
	/*
	 * If this is not the 32-bit i386 architecture then just return.
	 */
	if(arch->object->mh->cputype != CPU_TYPE_I386 ||
	   arch->object->mh == NULL)
	    return;

	/*
	 * For each stub section that has 5-byte entries and the self modifying
	 * code attribute update the JMP instructions in them using prebound
	 * undefined symbols to their new values.
	 */
	lc = arch->object->load_commands;
	ncmds = arch->object->mh->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		for(j = 0 ; j < sg->nsects ; j++){
		    section_type = s->flags & SECTION_TYPE;
		    if(section_type == S_SYMBOL_STUBS &&
		       (s->flags & S_ATTR_SELF_MODIFYING_CODE) == 
			S_ATTR_SELF_MODIFYING_CODE &&
			s->reserved2 == 5){

			if(s->reserved1 + s->size / 5 > arch_nindirectsyms){
			    error("malformed file: %s (for architecture %s) "
				"(indirect symbol table entries for section "
				"(%.16s,%.16s) extends past the end of the "
				"indirect symbol table)", arch->file_name,
				arch_name, s->segname, s->sectname);
			    redo_exit(2);
			}

			for(k = 0; k < s->size / 5; k++){
			    /*
			     * Get a pointer to the JMP instruction in memory.
			     */
			    p = contents_pointer_for_vmaddr(
				s->addr + (k * 5), 4);
			    if(p == NULL){
				error("malformed file: %s (for architecture "
				    "%s) (bad indirect section (%.16s,%.16s))",
				    arch->file_name, arch_name, s->segname,
				    s->sectname);
				redo_exit(2);
			    }
			    /* get the displacement from the JMP instruction */
			    displacement = get_arch_long(p + 1);

			    /* check symbol index of indirect symbol table */
			    if(arch_indirect_symtab[s->reserved1 + k] >
			       arch_nsyms){
				error("malformed file: %s (for architecture "
				      "%s) (bad indirect symbol table entry "
				      "%u)", arch->file_name, arch_name, i);
				    redo_exit(2);
				}
	    
				/*
				 * If the symbol this indirect symbol table
				 * entry is refering to is not a prebound
				 * undefined symbol then this symbol's value is
				 * used for the displacement of the JMP.
				 */ 
				arch_symbol = arch_symbols +
					 arch_indirect_symtab[s->reserved1 + k];
				if((arch_symbol->n_type & N_TYPE) != N_PBUD){
				    if(arch_symbol->n_sect == NO_SECT)
					symbol_slide = 0;
				    else
					symbol_slide = vmslide;
				    displacement = arch_symbol->n_value +
					       symbol_slide -
					       (vmslide + s->addr + (k * 5) + 5);
				}
				else{
				    /*
				     * Look up the symbol being referenced by this
				     * indirect symbol table entry to get the
				     * defined symbol's value to be used.
				     */
				    name = arch_strings + arch_symbol->n_un.n_strx;
				    lookup_symbol(name,
						  get_primary_lib(ARCH_LIB,
								  arch_symbol),
						  get_weak(arch_symbol),
						  &defined_symbol, &module_state,
						  &lib, NULL, NULL, NO_INDR_LOOP);
				    displacement = defined_symbol->n_value -
					       (vmslide + s->addr + (k * 5) + 5);
				}
				/*
				 * Now set the JMP opcode and the displacement.  We
				 * must set the opcode in case we are prebinding an
				 * unprebound() binary that has HLT instructions
				 * for the 5 bytes of the JMP instruction.
				 */
				*p = 0xE9; /* JMP rel32 */
				set_arch_long(p + 1, displacement);
			    }
			}
			s++;
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
    }

    /*
     * reset_symbol_pointers() sets lazy and non-lazy symbol pointers back to their
     * original, pre-prebinding values.
     */
    static
    void
    reset_symbol_pointers(
    uint32_t vmslide)
    {
	uint32_t i, j, k, m, section_type;
	uint32_t symbol_pointer;
	struct load_command *lc;
	struct segment_command *sg;
	struct section *s;
	struct nlist *arch_symbol;
	char *p;
	struct scattered_relocation_info *sreloc;
	uint32_t ncmds, mh_flags;
	
	    /*
	     * For each symbol pointer section update the symbol pointers by
	     * setting lazy symbol pointers to their original values, as defined
	     * in the corresponding local relocation entry. non-lazy symbol pointers
	     * will be set to zero, except those that are absolute or local
	     */
	    lc = arch->object->load_commands;
	    if(arch->object->mh != NULL){
		ncmds = arch->object->mh->ncmds;
		mh_flags = arch->object->mh->flags;
	    }
	    else{
		ncmds = arch->object->mh64->ncmds;
		mh_flags = arch->object->mh64->flags;
	    }
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_SEGMENT){
		    sg = (struct segment_command *)lc;
		    s = (struct section *)
			    ((char *)sg + sizeof(struct segment_command));
		    for(j = 0 ; j < sg->nsects ; j++){
			section_type = s->flags & SECTION_TYPE;
			if(section_type == S_NON_LAZY_SYMBOL_POINTERS ||
			   section_type == S_LAZY_SYMBOL_POINTERS){
			    if(s->reserved1 + s->size / sizeof(uint32_t) >
			       arch_nindirectsyms){
				error("malformed file: %s (for architecture "
				      "%s) (indirect symbol table entries for "
				      "section (%.16s,%.16s) extends past the "
				      "end of the indirect symbol table)",
				      arch->file_name, arch_name, s->segname,
				      s->sectname);
				redo_exit(2);
			    }
			    for(k = 0; k < s->size / sizeof(uint32_t); k++){
				p = contents_pointer_for_vmaddr(
				    s->addr + (k * sizeof(uint32_t)),
				    sizeof(uint32_t));
				if(p == NULL){
				    error("malformed file: %s (for architecture "
					"%s) (bad indirect section (%.16s,%.16s))",
				    arch->file_name, arch_name, s->segname,
				    s->sectname);
				redo_exit(2);
			    }
			    symbol_pointer = get_arch_long(p);
		
			    /*
			     * If this indirect symbol table entry is for a
			     * non-lazy symbol pointer section for a defined
			     * symbol which is an absolute symbol skip it.
			     */
			    if(section_type == S_NON_LAZY_SYMBOL_POINTERS &&
			       (arch_indirect_symtab[s->reserved1 + k] & 
				INDIRECT_SYMBOL_ABS) == INDIRECT_SYMBOL_ABS){
				continue;
			    }
			    /*
			     * If this indirect symbol table entry is for a
			     * non-lazy symbol pointer section for a defined
			     * symbol which strip(1) has removed slide it.
			     */
			    if(section_type == S_NON_LAZY_SYMBOL_POINTERS &&
			       (arch_indirect_symtab[s->reserved1 + k] & 
				INDIRECT_SYMBOL_LOCAL) ==INDIRECT_SYMBOL_LOCAL){
				set_arch_long(p, symbol_pointer + vmslide);
				continue;
			    }
		
			    /* check symbol index of indirect symbol table */
			    if(arch_indirect_symtab[s->reserved1 + k] >
			       arch_nsyms){
				error("malformed file: %s (for architecture "
				    "%s) (bad indirect symbol table entry %u)",
				    arch->file_name, arch_name, i);
				    redo_exit(2);
			    }
			
			    /*
			     * The Tiger dyld will for images containing the
			     * flags MH_WEAK_DEFINES or MH_BINDS_TO_WEAK can
			     * cause symbol pointers for indirect symbols
			     * defined in the image not to be used and prebound
			     * to addresses in other images.  So in this case
			     * they can't be assumed to be values from this
			     * image and slid.  So we let them fall through and
			     * get set to zero or what they be when lazily
			     * bound.
			     */
			    if((mh_flags & MH_WEAK_DEFINES) == 0 &&
			       (mh_flags & MH_BINDS_TO_WEAK) == 0){
				/*
				 * If the symbol this indirect symbol table
				 * entry is refering to is not a prebound
				 * undefined symbol then if this indirect
				 * symbol table entry is for a symbol in a
				 * section slide it.
				 */ 
				arch_symbol = arch_symbols +
					 arch_indirect_symtab[s->reserved1 + k];
				if((arch_symbol->n_type & N_TYPE) != N_PBUD){
				    if(arch_symbol->n_sect != NO_SECT)
					set_arch_long(p, symbol_pointer +
							 vmslide);
				    continue;
				}
			    }
		
			    if(section_type == S_NON_LAZY_SYMBOL_POINTERS){
				/*
				 * We reset the symbol pointer to zero
				 */
				 set_arch_long(p, 0);
			    }
			    else{
				/*
				 * This is a lazy symbol pointer and we must 
				 * find its original value by looking in the 
				 * r_value field of its corresponding local 
				 * relocation entry.  We need to look at the
				 * new relocation entries which have already 
				 * been slid.
				 */
				for(m = 0; m < arch_nlocrel; m++){
				    if(arch_locrelocs[m].r_address & 
				       R_SCATTERED){
					sreloc = (struct
						scattered_relocation_info *)
						(arch_locrelocs + m);
					if(sreloc->r_address + 
					   (arch_split_segs == TRUE ?
						arch_segs_read_write_addr : 
						arch_seg1addr) == 
					   s->addr + (k * sizeof(uint32_t)) && 
					   check_pb_la_ptr_reloc_cputype(
						sreloc->r_type)){
					    set_arch_long(p, sreloc->r_value);
					    break;
					}
				    }
				}
				if(m > arch_nlocrel){
				    error("malformed file: %s (for " 
					  "architecture %s) "
					  "(no local relocation entry for lazy "
					  "symbol pointer %u)", 
				    arch->file_name, arch_name, k);
				    redo_exit(2);
				}
							 
			    }
			}
		    }
		    s++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}

/*
 * reset_self_modifying_stubs() resets the i386 5-byte self modifying stubs that
 * were JMP instructions back to 5 halt instructions.
 */
static
void
reset_self_modifying_stubs(
void)
{
    uint32_t i, j, section_type;
    struct load_command *lc;
    struct segment_command *sg;
    struct section *s;
    char *p;
    uint32_t ncmds;
    
	/*
	 * If this is not the 32-bit i386 architecture then just return.
	 */
	if(arch->object->mh->cputype != CPU_TYPE_I386 ||
	   arch->object->mh == NULL)
	    return;

	/*
	 * For each stub section that has 5-byte entries and the self modifying
	 * code attribute reset the contents to be 5 HLT instructons.
	 */
	lc = arch->object->load_commands;
	ncmds = arch->object->mh->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		for(j = 0 ; j < sg->nsects ; j++){
		    section_type = s->flags & SECTION_TYPE;
		    if(section_type == S_SYMBOL_STUBS &&
		       (s->flags & S_ATTR_SELF_MODIFYING_CODE) == 
			S_ATTR_SELF_MODIFYING_CODE &&
			s->reserved2 == 5){
			/*
			 * Set the HLT opcode in all the bytes of this section.
			 */
			p = contents_pointer_for_vmaddr(s->addr, 1);
			memset(p, 0xf4, s->size);
		    }
		    s++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}

/*
 * check_pb_la_ptr_cputype() checks that the reloc_type (from the r_type field
 * of a scattered_relocation_info struct) is of the corresponding 
 * xxx_RELOC_PB_LA_PTR type.
 */
static
enum
bool
check_pb_la_ptr_reloc_cputype(
unsigned int reloc_type)
{
    switch(arch->object->mh_cputype){
	case CPU_TYPE_MC680x0:
	    return (reloc_type == GENERIC_RELOC_PB_LA_PTR);
	case CPU_TYPE_I386:
	    return (reloc_type == GENERIC_RELOC_PB_LA_PTR);
	case CPU_TYPE_HPPA:
	    return (reloc_type == HPPA_RELOC_PB_LA_PTR);
	case CPU_TYPE_SPARC:
	    return (reloc_type == SPARC_RELOC_PB_LA_PTR);
	case CPU_TYPE_POWERPC:
	    return (reloc_type == PPC_RELOC_PB_LA_PTR);
	}
    return FALSE;
}

/*
 * update_load_commands() updates the time stamps in the LC_LOAD_DYLIB,
 * LC_LOAD_WEAK_DYLIB and LC_REEXPORT_DYLIB commands and updates (and adds) the
 * LC_PREBOUND_DYLIB commands if this is an excutable.  It also updates the
 * addresses in the headers if the vmslide is not zero.
 */
static
void
update_load_commands(
uint32_t vmslide)
{
    uint32_t i, j, k, nmodules, size, sizeofcmds, ncmds, low_fileoff;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct dylib_command *dl_load, *dl_id;
    struct prebound_dylib_command *pbdylib1, *pbdylib2;
    struct segment_command *sg;
    struct section *s;
    char *dylib_name, *linked_modules;
    struct routines_command *rc;
    struct uuid_command *uuid;
    struct linkedit_data_command *code_sig;
    enum bool found, prebind_all_twolevel_modules;
    uint32_t ncmds1, ncmds2, mh_flags, mh_sizeofcmds;
    
	/*
	 * We need to figure out if this executable was built with the
	 * -prebind_all_twolevel_modules flag so to preserve this. We assume
	 * that it is and check all the linked_modules bit vectors in the
	 * existing LC_PREBOUND_DYLIB in the next loop.  If we find a module
	 * that is not bound in a two-level namespace image this is set to
	 * FALSE.  It is also set to FALSE here if the -force_flat_namespace
	 * was used.  If MH_ALLMODSBOUND is present in the header flags then
	 * we bypass this process and assume the executable was built with 
	 * -prebind_all_twolevel_modules.
	 */
	prebind_all_twolevel_modules = TRUE;
	if(arch_force_flat_namespace == TRUE)
	    prebind_all_twolevel_modules = FALSE;

	/*
	 * First copy the time stamps for the dependent libraries from the
	 * library's ID commands to the arch's load_command.  Also size the
	 * non LC_PREBOUND_DYLIB commands.
	 */
	ncmds = 0;
	sizeofcmds = 0;
	lc1 = arch->object->load_commands;
        if(arch->object->mh != NULL){
            ncmds1 = arch->object->mh->ncmds;
            mh_flags = arch->object->mh->flags;
        }
	else{
            ncmds1 = arch->object->mh64->ncmds;
            mh_flags = arch->object->mh64->flags;
        }
	for(i = 0; i < ncmds1; i++){
	    if(lc1->cmd == LC_LOAD_DYLIB ||
	       lc1->cmd == LC_LOAD_WEAK_DYLIB ||
	       lc1->cmd == LC_REEXPORT_DYLIB){
		dl_load = (struct dylib_command *)lc1;
		dylib_name = (char *)dl_load + dl_load->dylib.name.offset;
                if(unprebinding == TRUE){
                    dl_load->dylib.timestamp = 0;
                    /*
                     * We zero the version info for canonicalization.
                     */
		    dl_load->dylib.current_version = 0;
		    dl_load->dylib.compatibility_version = 0;
                }
                else{
                    found = FALSE;
                    for(j = 0; j < nlibs; j++){
                        if(strcmp(libs[j].dylib_name, dylib_name) == 0){
                            lc2 = libs[j].ofile->load_commands;
                            if(arch->object->mh != NULL)
                                ncmds2 = libs[j].ofile->mh->ncmds;
                            else
                                ncmds2 = libs[j].ofile->mh64->ncmds;
                            for(k = 0; k < ncmds2; k++){
                                if(lc2->cmd == LC_ID_DYLIB){
                                    dl_id = (struct dylib_command *)lc2;
                                    dl_load->dylib.timestamp =
                                        dl_id->dylib.timestamp;
                                    found = TRUE;
                                    break;
                                }
                                lc2 = (struct load_command *)
                                    ((char *)lc2 + lc2->cmdsize);
                            }
                            break;
                        }
                    }
                    /*
                    * A weak library may be missing if so clear it's fields.
                    */
                    if(found == FALSE){
                        dl_load->dylib.timestamp = 0;
                        dl_load->dylib.current_version = 0;
                        dl_load->dylib.compatibility_version = 0;
                    }
                }
	    }
	    else if(lc1->cmd == LC_ID_DYLIB){
	        /*
		 * If we are unprebinding, we need to set the timestamp to
		 * zero and make sure it doesn't get re-adjusted when we 
		 * write out the file.
		 */
		if(unprebinding == TRUE){
		    dl_load = (struct dylib_command *)lc1;
		    dl_load->dylib.timestamp = 0;
		    arch->dont_update_LC_ID_DYLIB_timestamp = TRUE;
		}
	    }
	    /*
	     * For the load commands update with address fields update the
	     * address for by the slide amount.
	     */
	    else if(lc1->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc1;
		sg->vmaddr += vmslide;
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    s->addr += vmslide;
		    s++;
		}
	    }
	    else if(lc1->cmd == LC_ROUTINES){
		rc = (struct routines_command *)lc1;
		rc->init_address += vmslide;
	    }
	    /*
	     * We zero the UUID and Code Signing info for canonicalization.
	     */
	    else if(lc1->cmd == LC_UUID && unprebinding == TRUE){
		uuid = (struct uuid_command *)lc1;
		memset(uuid->uuid, '\0', sizeof(uuid->uuid));
	    }
	    else if(lc1->cmd == LC_CODE_SIGNATURE && unprebinding == TRUE){
		code_sig = (struct linkedit_data_command *)lc1;
		memset(arch->object->object_addr + code_sig->dataoff, '\0',
			    code_sig->datasize);
	    }
	    if(lc1->cmd != LC_PREBOUND_DYLIB){
		ncmds += 1;
		sizeofcmds += lc1->cmdsize;
	    }
	    else{
		/*
		 * See if all two-level modules were linked.
		 */
		if(prebind_all_twolevel_modules == TRUE){
		    pbdylib1 = (struct prebound_dylib_command *)lc1;
		    linked_modules = (char *)pbdylib1 +
					    pbdylib1->linked_modules.offset;
		    for(j = 0; j < pbdylib1->nmodules; j++){
			if(((linked_modules[j/8] >> (j%8)) & 1) == 0){
			    /* found a module that is not linked */
			    prebind_all_twolevel_modules = FALSE;
			    break;
			}
		    }
		}
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * If this object has an LC_PREBIND_CKSUM load command update it.
	 */
	if(arch->object->cs != NULL){
	    if(zero_out_prebind_checksum == TRUE)
		arch->object->cs->cksum = 0;
	    else{
		/*
		 * We are not zeroing out the prebind checksum so if the
		 * existing cksum value is zero then set it to the value
		 * breakout calcualted as the input's prebind check sum.
		 */
		if(arch->object->cs->cksum == 0)
		    arch->object->cs->cksum =
			arch->object->calculated_input_prebind_cksum;
	    }
	}

	/*
	 * Only executables have LC_PREBOUND_DYLIB commands so if this is not
	 * an executable (a library) then we are done here.
	 */
	if(arch->object->mh_filetype != MH_EXECUTE)
	    return;

	/*
	 * An executable targeting 10.4 or later doesn't need LC_PREBOUND_DYLIB
	 * load commands.
	 */
	if(unprebinding == FALSE){
	    struct macosx_deployment_target deployment_version;
	    get_macosx_deployment_target(&deployment_version);
	    if(deployment_version.major >= 4)
		return;
	}

        if(mh_flags & MH_ALLMODSBOUND){
            if((mh_flags & MH_PREBINDABLE) != MH_PREBINDABLE){
                error("malformed file: %s (MH_ALLMODSBOUND is set without "
		      "MH_PREBINDABLE)",
                    arch->file_name);
                redo_exit(2);
            }
            if(arch->object->mh != NULL)
                arch->object->mh->flags &= ~MH_ALLMODSBOUND;
            else 
                arch->object->mh64->flags &= ~MH_ALLMODSBOUND;
            prebind_all_twolevel_modules = TRUE;
        }
        else{
            if(mh_flags & MH_PREBINDABLE){
                /*
                 * This was an unprebound binary that does not have 
		 * MH_ALLMODSBOUND set so all two level modules must not 
		 * have been bound.
                 */
                 prebind_all_twolevel_modules = FALSE;
            }
        }
        /*
        * For each library the executable uses determine the size we need for
        * the LC_PREBOUND_DYLIB load command for it.  If their is an exising
        * LC_PREBOUND_DYLIB command use it if there is enough space in the
        * command for the current number of modules.  If not calculate the
        * size ld(1) would use for it.
        */
        if(unprebinding == FALSE){
            for(i = 0; i < nlibs; i++){
                lc1 = arch->object->load_commands;
                for(j = 0; j < ncmds1; j++){
                    if(lc1->cmd == LC_PREBOUND_DYLIB){
                        pbdylib1 = (struct prebound_dylib_command *)lc1;
                        dylib_name = (char *)pbdylib1 + pbdylib1->name.offset;
                        if(strcmp(libs[i].dylib_name, dylib_name) == 0){
                            libs[i].LC_PREBOUND_DYLIB_found = TRUE;
                            if(libs[i].nmodtab <= pbdylib1->nmodules){
                                libs[i].LC_PREBOUND_DYLIB_size =
				    pbdylib1->cmdsize;
                            }
                            else{
                                /*
                                * Figure out the size left in the command for 
                                * the linked_modules bit vector.  When this is
				* first created by ld(1) extra space is left so
				* this this program can grow the vector if the
				* library changes.
                                */
                                size = pbdylib1->cmdsize -
                                        (sizeof(struct prebound_dylib_command) +
                                        rnd32((uint32_t)strlen(dylib_name) + 1,
					      sizeof(uint32_t)));
                                /*
                                 * Now see if the size left has enought space to
				 * fit the linked_modules bit vector for the
				 * number of modules this library currently has.
                                */
                                if((libs[i].nmodtab + 7)/8 <= size){
                                        libs[i].LC_PREBOUND_DYLIB_size =
                                            pbdylib1->cmdsize;
                                }
                                else{
                                    /*
				     * The existing space in not enough so
				     * calculate the new size as ld(1) would.
				     * 125% of the modules with a minimum size
				     * of 64 modules.
                                     */
                                    nmodules = libs[i].nmodtab +
                                    (libs[i].nmodtab >> 2);
                                    if(nmodules < 64)
                                        nmodules = 64;
                                    size = sizeof(struct
						  prebound_dylib_command) +
                                     rnd32((uint32_t)strlen(dylib_name)+1,
					   (uint32_t)sizeof(uint32_t)) +
                                     rnd32(nmodules / 8, sizeof(uint32_t));
                                    libs[i].LC_PREBOUND_DYLIB_size = size;
                                }
                            }
                            ncmds += 1;
                            sizeofcmds += libs[i].LC_PREBOUND_DYLIB_size;
			    break;
                        }
                    }
                    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
                }
            }
        }
        if(arch->object->mh != NULL)
            mh_sizeofcmds = arch->object->mh->sizeofcmds;
        else
            mh_sizeofcmds = arch->object->mh64->sizeofcmds;

        /*
         * If we are unprebinding and all two level modules were bound, then
         * we need to set MH_ALLMODSBOUND.  We don't want to set this flag
         * on binaries that don't have LC_PREBOUND_DYLIB commands.
         */
         if(unprebinding == TRUE){
             if(prebind_all_twolevel_modules && 
                (mh_flags & MH_PREBOUND) == MH_PREBOUND){
                if(arch->object->mh != NULL)
                    arch->object->mh->flags |= MH_ALLMODSBOUND;
                else
                    arch->object->mh64->flags |= MH_ALLMODSBOUND;
	     }
         }
         else{
	    /*
	     * Make a pass through the libraries and pick up any of them that
	     * did not appear in the load commands and then size their
	     * LC_PREBOUND_DYLIB command.
	     */
            for(i = 0; i < nlibs; i++){
                if(libs[i].LC_PREBOUND_DYLIB_found == FALSE){
                    /*
                    * Calculate the size as ld(1) would.  125% of the
                    * modules with a minimum size of 64 modules.
                    */
                    nmodules = libs[i].nmodtab + (libs[i].nmodtab >> 2);
                    if(nmodules < 64)
                        nmodules = 64;
                    size = sizeof(struct prebound_dylib_command) +
                    rnd32((uint32_t)strlen(libs[i].dylib_name) + 1,
			  sizeof(uint32_t)) +
			rnd32(nmodules / 8, sizeof(uint32_t));
                    libs[i].LC_PREBOUND_DYLIB_size = size;
                    sizeofcmds += libs[i].LC_PREBOUND_DYLIB_size;
                    ncmds++;
                }
            }
    
            /*
	     * If the size of the load commands that includes the updated
	     * LC_PREBOUND_DYLIB commands is larger than the existing load
	     * commands then see if they can be fitted in before the contents
	     * of the first section (or segment in the case of a LINKEDIT
	     * segment only file).
	     */
            if(sizeofcmds > mh_sizeofcmds){
                low_fileoff = UINT_MAX;
                lc1 = arch->object->load_commands;
                for(i = 0; i < ncmds1; i++){
                    if(lc1->cmd == LC_SEGMENT){
                        sg = (struct segment_command *)lc1;
                        s = (struct section *)
                            ((char *)sg + sizeof(struct segment_command));
                        if(sg->nsects != 0){
                            for(j = 0; j < sg->nsects; j++){
                                if(s->size != 0 &&
                                   (s->flags & S_ZEROFILL) != S_ZEROFILL &&
                                   (s->flags & S_THREAD_LOCAL_ZEROFILL) !=
					       S_THREAD_LOCAL_ZEROFILL &&
                                   s->offset < low_fileoff)
                                    low_fileoff = s->offset;
                                s++;
                            }
                        }
                        else{
                            if(sg->filesize != 0 && sg->fileoff < low_fileoff)
                                low_fileoff = sg->fileoff;
                        }
                    }
                    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
                }
                if(sizeofcmds + sizeof(struct mach_header) > low_fileoff){
                    error("prebinding can't be redone for: %s (for architecture"
                        " %s) because larger updated load commands do not fit "
                        "(the program must be relinked)", arch->file_name,
                        arch_name);
#ifdef LIBRARY_API
                    if(check_if_needed == TRUE){
                        only_if_needed_retval =
			    REDO_PREBINDING_NEEDS_REBUILDING;
                        return;
                    }
#endif
                    redo_exit(2);
                }
            }
        }
    
	/*
	 * Allocate space for the new load commands as zero it out so any holes
	 * will be zero bytes.
	 */
	new_load_commands = allocate(sizeofcmds);
	memset(new_load_commands, '\0', sizeofcmds);

	/*
	 * Fill in the new load commands by copying in the non-LC_PREBOUND_DYLIB
	 * commands and updating the LC_PREBOUND_DYLIB commands.  If we are
	 * unprebinding, do not bring the LC_PREBOUND_DYLIB commands with us.
	 */
	lc1 = arch->object->load_commands;
	lc2 = new_load_commands;
	for(i = 0; i < ncmds1; i++){
	    if(lc1->cmd == LC_PREBOUND_DYLIB){
                if(unprebinding == FALSE){
                    pbdylib1 = (struct prebound_dylib_command *)lc1;
                    pbdylib2 = (struct prebound_dylib_command *)lc2;
                    dylib_name = (char *)pbdylib1 + pbdylib1->name.offset;
                    for(j = 0; j < nlibs; j++){
                        if(strcmp(libs[j].dylib_name, dylib_name) == 0){
                            pbdylib2->cmd = LC_PREBOUND_DYLIB;
                            pbdylib2->cmdsize = libs[j].LC_PREBOUND_DYLIB_size;
                            pbdylib2->name.offset =
                                    sizeof(struct prebound_dylib_command);
                            strcpy(((char *)pbdylib2) +
                                    sizeof(struct prebound_dylib_command),
                                    dylib_name);
                            pbdylib2->nmodules = libs[j].nmodtab;
                            pbdylib2->linked_modules.offset =
                                    sizeof(struct prebound_dylib_command) +
                                    rnd32((uint32_t)strlen(dylib_name) + 1,
				    sizeof(uint32_t));
                            linked_modules = ((char *)pbdylib2) +
                                    sizeof(struct prebound_dylib_command) +
                                    rnd32((uint32_t)strlen(dylib_name) + 1,
				    sizeof(uint32_t));
                            if(libs[j].ofile->mh != NULL)
                                mh_flags = libs[j].ofile->mh->flags;
                            else
                                mh_flags = libs[j].ofile->mh64->flags;
                            for(k = 0; k < libs[j].nmodtab; k++){
                                if(libs[j].module_states[k] == LINKED ||
                                (prebind_all_twolevel_modules == TRUE &&
                                    (mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL))
                                    linked_modules[k / 8] |= 1 << k % 8;
                            }
                            lc2 = (struct load_command *)
                                    ((char *)lc2 + lc2->cmdsize);
                            break;
                        }
                    }
                }
	    }
	    else{
		memcpy(lc2, lc1, lc1->cmdsize);
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

        if(unprebinding == FALSE){
            /*
	     * Add any new LC_PREBOUND_DYLIB load commands.
	     */
            for(i = 0; i < nlibs; i++){
                if(libs[i].LC_PREBOUND_DYLIB_found == FALSE){
                    pbdylib2 = (struct prebound_dylib_command *)lc2;
                    pbdylib2->cmd = LC_PREBOUND_DYLIB;
                    pbdylib2->cmdsize = libs[i].LC_PREBOUND_DYLIB_size;
                    pbdylib2->name.offset =
                            sizeof(struct prebound_dylib_command);
                    strcpy(((char *)pbdylib2) +
                            sizeof(struct prebound_dylib_command),
                            libs[i].dylib_name);
                    pbdylib2->nmodules = libs[i].nmodtab;
                    pbdylib2->linked_modules.offset =
                            sizeof(struct prebound_dylib_command) +
                            rnd32((uint32_t)strlen(libs[i].dylib_name) + 1,
			    sizeof(uint32_t));
                    linked_modules = ((char *)pbdylib2) +
                            sizeof(struct prebound_dylib_command) +
                            rnd32((uint32_t)strlen(libs[i].dylib_name) + 1,
			    sizeof(uint32_t));
                    if(libs[i].ofile->mh != NULL)
                        mh_flags = libs[i].ofile->mh->flags;
                    else
                        mh_flags = libs[i].ofile->mh64->flags;
                    for(j = 0; j < libs[i].nmodtab; j++){
                        if(libs[i].module_states[j] == LINKED ||
                        (prebind_all_twolevel_modules == TRUE &&
                            (mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL))
                            linked_modules[j / 8] |= 1 << j % 8;
                    }
                    lc2 = (struct load_command *)
                            ((char *)lc2 + lc2->cmdsize);
                }
            }
        }
    
	/*
	 * Finally copy the updated load commands over the existing load
	 * commands.
	 */
	memcpy(arch->object->load_commands, new_load_commands, sizeofcmds);
	if(mh_sizeofcmds > sizeofcmds){
		memset((char *)arch->object->load_commands + sizeofcmds, '\0', 
			   (mh_sizeofcmds - sizeofcmds));
	}
        if(arch->object->mh != NULL) {
            arch->object->mh->sizeofcmds = sizeofcmds;
            arch->object->mh->ncmds = ncmds;
        } else {
            arch->object->mh64->sizeofcmds = sizeofcmds;
            arch->object->mh64->ncmds = ncmds;
        }
	free(new_load_commands);

	/* reset the pointers into the load commands */
	lc1 = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		arch->object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		arch->object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		arch->object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		arch->object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    arch->object->seg_linkedit = sg;
		break;
	    case LC_CODE_SIGNATURE:
		arch->object->code_sig_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_SEGMENT_SPLIT_INFO:
		arch->object->split_info_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_FUNCTION_STARTS:
		arch->object->func_starts_info_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		arch->object->data_in_code_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		arch->object->code_sign_drs_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		arch->object->link_opt_hint_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_CHAINED_FIXUPS:
		arch->object->dyld_chained_fixups =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_EXPORTS_TRIE:
		arch->object->dyld_exports_trie =
		    (struct linkedit_data_command *)lc1;
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}

#ifndef LIBRARY_API
/*
 * Print the warning message and the input file.
 */
__private_extern__
void
warning_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
}

/*
 * Print the error message the input file and increment the error count
 */
__private_extern__
void
error_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
	errors++;
}

/*
 * Print the fatal error message the input file and exit non-zero.
 */
__private_extern__
void
fatal_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
	if(check_for_non_prebound == TRUE)
	    exit(0);
	exit(2);
}

#else /* defined(LIBRARY_API) */

/*
 * vmessage() is a sub routine used by warning(), error(), system_error() for
 * the library api to get the varariable argument message in to the error
 * message buffer.
 */
static
void
vmessage(
const char *format,
va_list ap)
{
    uint32_t new;

	setup_error_message_buffer();
        /* for the __OPENSTEP__ case hope the string does not overflow */
#ifdef __OPENSTEP__
        new = vsprintf(last, format, ap);
#else
        new = vsnprintf(last, left, format, ap);
#endif
        last += new;
        left -= new;
}

/*
 * message() is a sub routine used by warning(), error(), system_error() or
 * directly (for multi line message) for the library api to get the message in
 * to the error message buffer.
 */
static
void
message(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vmessage(format, ap);
	va_end(ap);
}

/*
 * message_with_arch() is a sub routine used by warning_arch(), error_arch()
 * and fatal_arch() for the library api to get the message in to the error
 * message buffer.
 */
static
void
message_with_arch(
struct arch *arch,
struct member *member,
const char *format,
va_list ap)
{
    uint32_t new;

	setup_error_message_buffer();
        /* for the __OPENSTEP__ case hope the string does not overflow */
#ifdef __OPENSTEP__
        new = vsprintf(last, format, ap);
#else
        new = vsnprintf(last, left, format, ap);
#endif
        last += new;
        left -= new;

	if(member != NULL){
#ifdef __OPENSTEP__
	    new = sprintf(last, 
#else
	    new = snprintf(last, left, 
#endif
		    "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	    last += new;
	    left -= new;
	}
	else{
#ifdef __OPENSTEP__
	    new = sprintf(last, 
#else
	    new = snprintf(last, left, 
#endif
	    	    "%s", arch->file_name);
	    last += new;
	    left -= new;
	}
	if(arch->fat_arch_name != NULL){
#ifdef __OPENSTEP__
	    new = sprintf(last, 
#else
	    new = snprintf(last, left, 
#endif
		    " (for architecture %s)", arch->fat_arch_name);
	    last += new;
	    left -= new;
	}
}

/*
 * Put the warning message and the input file into the error message buffer.
 */
__private_extern__
void
warning_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	message_with_arch(arch, member, format, ap);
	va_end(ap);
}

/*
 * Put the error message and the input file into the error message buffer
 * and increment the error count
 */
__private_extern__
void
error_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	message_with_arch(arch, member, format, ap);
	va_end(ap);
	errors++;
}

/*
 * Put the warning message and the input file into the error message buffer and
 * then longjmp back to the library api so it can return unsuccessfully.
 */
__private_extern__
void
fatal_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	message_with_arch(arch, member, format, ap);
	va_end(ap);
	if(check_only == TRUE)
	    retval = PREBINDING_UNKNOWN;
	longjmp(library_env, 1);
}

__private_extern__ uint32_t errors = 0;	/* number of calls to error() */

/*
 * Just put the message into the error message buffer without setting errors.
 */
__private_extern__
void
warning(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vmessage(format, ap);
	va_end(ap);
}

/*
 * Put the message into the error message buffer and return to the caller
 * after setting the error indication.
 */
__private_extern__
void
error(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vmessage(format, ap);
	va_end(ap);
	errors++;
}

/*
 * Put the message into the error message buffer and the architecture if not
 * NULL and return to the caller after setting the error indication.
 */
__private_extern__
void
error_with_arch(
const char *arch_name,
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	if(arch_name != NULL)
	    message("for architecture: %s ", arch_name);
	vmessage(format, ap);
	va_end(ap);
	errors++;
}

/*
 * Put the message into the error message buffer along with the system error
 * message and return to the caller after setting the error indication.
 */
__private_extern__
void
system_error(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vmessage(format, ap);
	message(" (%s)", strerror(errno));
	va_end(ap);
	errors++;
}

/*
 * Put the message into the error message buffer along with the mach error
 * string and return to the caller after setting the error indication.
 */
__private_extern__
void
my_mach_error(
kern_return_t r,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vmessage(format, ap);
	message(" (%s)", mach_error_string(r));
	va_end(ap);
	errors++;
}

/*
 * allocate() is used to allocate temporary memory for the library api and is
 * allocated in the library zone.  If the allocation fails it is a fatal error.
 */
__private_extern__
void *
allocate(
size_t size)
{
    void *p;

	if(library_zone == NULL){
	    library_zone = malloc_create_zone(vm_page_size, 0);
	    if(library_zone == NULL)
		fatal("malloc_create_zone() failed");
	    malloc_set_zone_name(library_zone, "redo_prebinding");
	}
	if(size == 0)
	    return(NULL);
	if((p = malloc_zone_malloc(library_zone, size)) == NULL)
	    system_fatal("virtual memory exhausted (malloc_zone_malloc() "
			 " failed)");
	return(p);
}

/*
 * reallocate() is used to reallocate temporary memory for the library api and
 * is allocated in the library zone.  If the allocation fails it is a fatal
 * error.
 */
__private_extern__
void *
reallocate(
void *p,
size_t size)
{
	if(library_zone == NULL){
	    library_zone = malloc_create_zone(vm_page_size, 1);
	    if(library_zone == NULL)
		fatal("malloc_create_zone() failed");
	    malloc_set_zone_name(library_zone, "redo_prebinding");
	}
	if(p == NULL)
	    return(allocate(size));
	if((p = malloc_zone_realloc(library_zone, p, size)) == NULL)
	    system_fatal("virtual memory exhausted (malloc_zone_realloc() "
			 " failed)");
	return(p);
}

/*
 * savestr() allocate space for the string passed to it, copys the string into
 * the space and returns a pointer to that space.
 */
__private_extern__
char *
savestr(
const char *s)
{
    size_t len;
    char *r;

	len = strlen(s) + 1;
	r = (char *)allocate(len);
	strcpy(r, s);
	return(r);
}

/*
 * Makestr() creates a string that is the concatenation of a variable number of
 * strings.  It is pass a variable number of pointers to strings and the last
 * pointer is NULL.  It returns the pointer to the string it created.  The
 * storage for the string is allocated()'ed can be free()'ed when nolonger
 * needed.
 */
__private_extern__
char *
makestr(
const char *args,
...)
{
    va_list ap;
    char *s, *p;
    uint32_t size;

	size = 0;
	if(args != NULL){
	    size += strlen(args);
	    va_start(ap, args);
	    p = (char *)va_arg(ap, char *);
	    while(p != NULL){
		size += strlen(p);
		p = (char *)va_arg(ap, char *);
	    }
	}
	s = allocate(size + 1);
	*s = '\0';

	if(args != NULL){
	    (void)strcat(s, args);
	    va_start(ap, args);
	    p = (char *)va_arg(ap, char *);
	    while(p != NULL){
		(void)strcat(s, p);
		p = (char *)va_arg(ap, char *);
	    }
	    va_end(ap);
	}
	return(s);
}

__private_extern__
void
archive_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    message("%s: for architecture %s archive: %s ",
		    progname, ofile->arch_flag.name, ofile->file_name);
	}
	else{
	    message("%s: archive: %s ", progname, ofile->file_name);
	}
	vmessage(format, ap);
        message("\n");
	va_end(ap);
	errors++;
}

__private_extern__
void
archive_member_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    message("%s: for architecture %s archive member: %s(%.*s) ",
		    progname, ofile->arch_flag.name, ofile->file_name,
		    (int)ofile->member_name_size, ofile->member_name);
	}
	else{
	    message("%s: archive member: %s(%.*s) ", progname, ofile->file_name,
		    (int)ofile->member_name_size, ofile->member_name);
	}
	vmessage(format, ap);
        message("\n");
	va_end(ap);
	errors++;
}

__private_extern__
void
Mach_O_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type == OFILE_ARCHIVE){
		message("%s: for architecture %s object: %s(%.*s) ", progname,
		        ofile->arch_flag.name, ofile->file_name,
		        (int)ofile->member_name_size, ofile->member_name);
	    }
	    else{
		message("%s: for architecture %s object: %s ", progname,
		        ofile->arch_flag.name, ofile->file_name);
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    if(ofile->member_type == OFILE_FAT){
		message("%s: for object: %s(%.*s) architecture %s ", progname,
		        ofile->file_name, (int)ofile->member_name_size,
		        ofile->arch_flag.name, ofile->member_name);
	    }
	    else{
		message("%s: object: %s(%.*s) ", progname, ofile->file_name,
		        (int)ofile->member_name_size, ofile->member_name);
	    }
	}
	else{
	    message("%s: object: %s ", progname, ofile->file_name);
	}
	vmessage(format, ap);
        message("\n");
	va_end(ap);
	errors++;
}
#endif /* defined(LIBRARY_API) */

#include <sys/attr.h>
/*
 * Structure defining what's returned from getattrlist.  It returns all the
 * values we want in some order (probably from largest bit representation to
 * smallest.
 */
struct fileinfobuf {   
    uint32_t info_length;
    /*
     * The first two words contain the type and creator.  I have no idea what's
     * in the rest of the info.
     */
    uint32_t finderinfo[8];
    /*
     * Note that the file lengths appear to be long long.  I have no idea where
     * the sizes of different values are defined.
     */
    char data_length[sizeof(uint64_t)];
    char resource_length[sizeof(uint64_t)];
};

/*
 * has_resource_fork() returns TRUE if the filename contains a resource fork or
 * type/creator, FALSE otherwise.
 */
static
enum bool
has_resource_fork(
char *filename)
{
    int err;
    struct attrlist alist;
    struct fileinfobuf finfo;
    uint64_t data_length;
    uint64_t resource_length;

	/*
	 * Set up the description of what info we want.  We'll want the finder
	 * info on this file as well as info on the resource fork's length.
	 */
	alist.bitmapcount = 5;
	alist.reserved = 0;
	alist.commonattr = ATTR_CMN_FNDRINFO;
	alist.volattr = 0;
	alist.dirattr = 0;
	alist.fileattr = ATTR_FILE_DATALENGTH | ATTR_FILE_RSRCLENGTH;
	alist.forkattr = 0;

	err = getattrlist(filename, &alist, &finfo, sizeof(finfo), 0);
	if(debug == TRUE){
	    printf("getattrlist() returned = %d\n", err);
	    if(err == 0){
		printf("finfo.info_length = %u\n", finfo.info_length);
		printf("sizeof(finfo) = %lu\n", sizeof(finfo));
	    }
	}
	/*
	 * If non-zero either not a file on an HFS disk, file does not exist, 
	 * or something went wrong.
	 */
	if(err != 0)
	    return(FALSE);

	memcpy(&resource_length, finfo.resource_length, sizeof(uint64_t));
	memcpy(&data_length, finfo.data_length, sizeof(uint64_t));
	if(debug == TRUE){
	    printf("Resource fork len is %llu\n", resource_length);
	    printf("Data fork len is %llu\n", data_length);
	}

    	/* see if it has a resource fork */
	if(resource_length != 0)
	    return(TRUE);

	/*
	 * If the type/creator wasn't just zero -- probably has a value.
	 * type/creator represented by spaces would also count as having a
	 * value.
	 */
	if((finfo.finderinfo[0] != 0) || (finfo.finderinfo[1] != 0))
	    return(TRUE);

	return(FALSE);
}
