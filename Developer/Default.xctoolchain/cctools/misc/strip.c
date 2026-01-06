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
/*
 * The strip(1) and nmedit(l) program.  This understands only Mach-O format
 * files (with the restriction the symbol table is at the end of the file) and
 * fat files with Mach-O files in them.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <limits.h>
#include <ctype.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mach-o/loader.h>
#include <mach-o/reloc.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include "stuff/port.h" /* cctools-port: fake signing */
#include "stuff/breakout.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/rnd.h"
#include "stuff/reloc.h"
#include "stuff/reloc.h"
#include "stuff/symbol_list.h"
#include "stuff/unix_standard_mode.h"
#include "stuff/execute.h"
#include "stuff/write64.h"
#ifdef TRIE_SUPPORT
#include <mach-o/prune_trie.h>
#endif /* TRIE_SUPPORT */

/* These are set from the command line arguments */
__private_extern__
char *progname = NULL;	/* name of the program for error messages (argv[0]) */
static char *output_file;/* name of the output file */
static char *sfile;	/* filename of global symbol names to keep */
static char *Rfile;	/* filename of global symbol names to remove */
static uint32_t Aflag;	/* save only absolute symbols with non-zero value and
			   .objc_class_name_* symbols */
static uint32_t iflag;	/* -i ignore symbols in -s file not in object */
#ifdef NMEDIT
static uint32_t pflag;	/* make all defined global symbols private extern */
#else /* !defined(NMEDIT) */
static char *dfile;	/* filename of filenames of debugger symbols to keep */
static uint32_t uflag;	/* save undefined symbols */
static uint32_t rflag;	/* save symbols referenced dynamically */
static uint32_t nflag;	/* save N_SECT global symbols */
static uint32_t Sflag;	/* -S strip only debugger symbols N_STAB */
static uint32_t xflag;	/* -x strip non-globals */
static uint32_t Xflag;	/* -X strip local symbols with 'L' names */
static uint32_t Tflag;	/* -T strip Swift symbols: symbols that start with
                           '_$S' or '_$s' */
static uint32_t Nflag;	/* -N strip all nlist symbols and strings */
static uint32_t cflag;	/* -c strip section contents from dynamic libraries
			   files to create stub libraries */
static uint32_t no_uuid;/* -no_uuid strip LC_UUID load commands */
static uint32_t no_split_info; /* -no_split_info strip LC_SEGMENT_SPLIT_INFO
				  load command and its payload */
static uint32_t no_code_signature_warning;
		/* -no_code_signature_warning then don't warn when the code
		   signature would be invalid */
static uint32_t vflag;	/* -v for verbose debugging ld -r executions */
static uint32_t lflag;	/* -l do ld -r executions even if it has bugs */
static enum bool toc64flag = FALSE; /* -toc64 for a 64-bit toc in archives */
static uint32_t strip_all = 1;
/*
 * This is set on an object by object basis if the strip_all flag is still set
 * and the object is an executable that is for use with the dynamic linker.
 * This has the same effect as -r and -u.
 */
static enum bool default_dyld_executable = FALSE;

/*
 * When the -N flag is used it may not be possible to strip all nlists because
 * the file is not used by dyld, an MH_KEXT_BUNDLE filetype or has external
 * relocations in the LC_DYSYMTAB.
 */
static enum bool strip_all_nlists = FALSE;
#endif /* NMEDIT */

/*
 * Data structures to perform selective stripping of symbol table entries.
 * save_symbols is the names of the symbols from the -s <file> argument.
 * remove_symbols is the names of the symbols from the -R <file> argument.
 */
static struct symbol_list *save_symbols = NULL;
static uint32_t nsave_symbols = 0;
static struct symbol_list *remove_symbols = NULL;
static uint32_t nremove_symbols = 0;

/*
 * saves points to an array of uint32_t's that is allocated.  This array is a
 * map of old symbol indexes to new symbol indexes.  The new symbol indexes are
 * plus 1 and zero value means that old symbol is not in the new symbol table.
 * ref_saves is used in the same way but for the reference table.
 * nmedits is an array and indexed by the symbol index the value indicates if
 * the symbol was edited and turned into a non-global.
 */
static int32_t *saves = NULL;
#ifndef NMEDIT
static int32_t *ref_saves = NULL;
#else
static enum bool *nmedits = NULL;
#endif

/*
 * These hold pointers to the symbol, string and indirect tables being worked on
 * by strip_object and strip_symtab() from an input object file or possiblity
 * changed to an ld -r (-S or -x) file by make_ld_r_object().
 */
static struct nlist *symbols = NULL;
static struct nlist_64 *symbols64 = NULL;
static uint32_t nsyms = 0;
static char *strings = NULL;
static uint32_t strsize = 0;
static uint32_t *indirectsyms = NULL;
static uint32_t nindirectsyms = 0;

/*
 * These hold the new symbol and string table created by strip_symtab()
 * and the new counts of local, defined external and undefined symbols.
 */
static struct nlist *new_symbols = NULL;
static struct nlist_64 *new_symbols64 = NULL;
static uint32_t new_nsyms = 0;
static char *new_strings = NULL;
static uint32_t new_strsize = 0;
static uint32_t new_nlocalsym = 0;
static uint32_t new_nextdefsym = 0;
static uint32_t new_nundefsym = 0;
#if defined(TRIE_SUPPORT) && !defined(NMEDIT)
/*
 * The index into the new symbols where the defined external start.
 */
static uint32_t inew_nextdefsym = 0;
#endif

/*
 * These hold the new table of contents, reference table and module table for
 * dylibs.
 */
static struct dylib_table_of_contents *new_tocs = NULL;
static uint32_t new_ntoc = 0;
static struct dylib_reference *new_refs = NULL;
static uint32_t new_nextrefsyms = 0;
#ifdef NMEDIT
static struct dylib_module *new_mods = NULL;
static struct dylib_module_64 *new_mods64 = NULL;
static uint32_t new_nmodtab = 0;
#endif

#ifndef NMEDIT
/*
 * The list of file names to save debugging symbols from.
 */
static char **debug_filenames = NULL;
static uint32_t ndebug_filenames = 0;
struct undef_map {
    uint32_t index;
    struct nlist symbol;
};
struct undef_map64 {
    uint32_t index;
    struct nlist_64 symbol64;
};
static char *qsort_strings = NULL;

struct strx_map {
    uint32_t old_strx;
    uint32_t new_strx;
};
#endif /* !defined(NMEDIT) */


/* Internal routines */
static void usage(
    void);

static void strip_file(
    char *input_file,
    struct arch_flag *arch_flags,
    uint32_t narch_flags,
    enum bool all_archs);

static void strip_arch(
    struct arch *archs,
    uint32_t narchs,
    struct arch_flag *arch_flags,
    uint32_t narch_flags,
    enum bool all_archs);

static void strip_object(
    struct arch *arch,
    struct member *member,
    struct object *object);

static uint32_t get_starting_syminfo_offset(
    struct object *object);

static void check_object_relocs(
    struct arch *arch,
    struct member *member,
    struct object *object,
    char *segname,
    char *sectname,
    uint64_t sectsize,
    char *contents,
    struct relocation_info *relocs,
    uint32_t nreloc,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsyms,
    char *strings,
    int32_t *missing_reloc_symbols,
    enum byte_sex host_byte_sex);

static void check_indirect_symtab(
    struct arch *arch,
    struct member *member,
    struct object *object,
    uint32_t nitems,
    uint32_t reserved1,
    uint32_t section_type,
    char *contents,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsyms,
    char *strings,
    int32_t *missing_reloc_symbols,
    uint32_t swift_version,
    enum byte_sex host_byte_sex);

#ifndef NMEDIT
static enum bool strip_symtab(
    struct arch *arch,
    struct member *member,
    struct object *object,
    struct dylib_table_of_contents *tocs,
    uint32_t ntoc,
    struct dylib_module *mods,
    struct dylib_module_64 *mods64,
    uint32_t nmodtab,
    struct dylib_reference *refs,
    uint32_t nextrefsyms,
    uint32_t *p_swift_version,
    enum bool *nlist_outofsync_with_dyldinfo);

#ifdef TRIE_SUPPORT
static int prune(
    const char *name);
#endif /* TRIE_SUPPORT */

static void make_ld_r_object(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void strip_LC_UUID_commands(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void strip_LC_SEGMENT_SPLIT_INFO_command(
    struct arch *arch,
    struct member *member,
    struct object *object);

#ifndef NMEDIT
static void strip_LC_CODE_SIGNATURE_commands(
    struct arch *arch,
    struct member *member,
    struct object *object);
#endif /* !(NMEDIT) */

static enum bool private_extern_reference_by_module(
    uint32_t symbol_index,
    struct dylib_reference *refs,
    uint32_t nextrefsyms);

static enum bool symbol_pointer_used(
    uint32_t symbol_index,
    uint32_t *indirectsyms,
    uint32_t nindirectsyms);

static int cmp_qsort_strx_map(
    const struct strx_map* a,
    const struct strx_map* b);

static int cmp_bsearch_strx_map(
    const uint32_t* old_strx,
    const struct strx_map *strx_map);

static int cmp_qsort_undef_map(
    const struct undef_map *sym1,
    const struct undef_map *sym2);

static int cmp_qsort_undef_map_64(
    const struct undef_map64 *sym1,
    const struct undef_map64 *sym2);
#endif /* !defined(NMEDIT) */

#ifdef NMEDIT
static enum bool edit_symtab(
    struct arch *arch,
    struct member *member,
    struct object *object,
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
#endif /* NMEDIT */

#ifndef NMEDIT
static void setup_debug_filenames(
    char *dfile);

static int cmp_qsort_filename(
    const char **name1,
    const char **name2);

static int cmp_bsearch_filename(
    const char *name1,
    const char **name2);
#endif /* NMEDIT */

#ifdef NMEDIT
/*
 * This variable and routines are used for nmedit(1) only.
 */
static char *global_strings = NULL;

static int cmp_qsort_global(
    const struct nlist **sym1,
    const struct nlist **sym2);

static int cmp_qsort_global_64(
    const struct nlist_64 **sym1,
    const struct nlist_64 **sym2);

static int cmp_bsearch_global_stab(
    const char *name,
    const struct nlist **sym);

static int cmp_bsearch_global_stab_64(
    const char *name,
    const struct nlist_64 **sym);

static int cmp_bsearch_global(
    const char *name,
    const struct nlist **sym);

static int cmp_bsearch_global_64(
    const char *name,
    const struct nlist_64 **sym);
#endif /* NMEDIT */

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int
main(
int argc,
char *argv[],
char *envp[])
{
    int i;
    uint32_t j, args_left, files_specified;
    struct arch_flag *arch_flags;
    uint32_t narch_flags;
    enum bool all_archs;
    struct symbol_list *sp;

	progname = argv[0];

	arch_flags = NULL;
	narch_flags = 0;
	all_archs = FALSE;

	files_specified = 0;
	args_left = 1;
	for (i = 1; i < argc; i++){
	    if(argv[i][0] == '-'){
		if(argv[i][1] == '\0'){
		    args_left = 0;
		    break;
		}
		if(strcmp(argv[i], "-o") == 0){
		    if(i + 1 >= argc)
			fatal("-o requires an argument");
		    if(output_file != NULL)
			fatal("only one -o option allowed");
		    output_file = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-s") == 0){
		    if(i + 1 >= argc)
			fatal("-s requires an argument");
		    if(sfile != NULL)
			fatal("only one -s option allowed");
		    sfile = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-R") == 0){
		    if(i + 1 >= argc)
			fatal("-R requires an argument");
		    if(Rfile != NULL)
			fatal("only one -R option allowed");
		    Rfile = argv[i + 1];
		    i++;
		}
#ifndef NMEDIT
		else if(strcmp(argv[i], "-d") == 0){
		    if(i + 1 >= argc)
			fatal("-d requires an argument");
		    if(dfile != NULL)
			fatal("only one -d option allowed");
		    dfile = argv[i + 1];
		    i++;
		}
		else if(strcmp(argv[i], "-no_uuid") == 0){
		    no_uuid = 1;
		}
		else if(strcmp(argv[i], "-no_split_info") == 0){
		    no_split_info = 1;
		}
		else if(strcmp(argv[i], "-no_code_signature_warning") == 0){
		    no_code_signature_warning = 1;
		}
		else if(strcmp(argv[i], "-toc64") == 0){
		    toc64flag = TRUE;
		}
#endif /* !defined(NMEDIT) */
		else if(strcmp(argv[i], "-arch") == 0){
		    if(i + 1 == argc){
			error("missing argument(s) to %s option", argv[i]);
			usage();
		    }
		    if(strcmp("all", argv[i+1]) == 0){
			all_archs = TRUE;
		    }
		    else{
			arch_flags = reallocate(arch_flags,
				(narch_flags + 1) * sizeof(struct arch_flag));
			if(get_arch_from_flag(argv[i+1],
					      arch_flags + narch_flags) == 0){
			    error("unknown architecture specification flag: "
				  "%s %s", argv[i], argv[i+1]);
			    arch_usage();
			    usage();
			}
			for(j = 0; j < narch_flags; j++){
			    if(arch_flags[j].cputype ==
				    arch_flags[narch_flags].cputype &&
			       (arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK) ==
				    (arch_flags[narch_flags].cpusubtype &
				    ~CPU_SUBTYPE_MASK) &&
			       strcmp(arch_flags[j].name,
				    arch_flags[narch_flags].name) == 0)
				break;
			}
			if(j == narch_flags)
			    narch_flags++;
		    }
		    i++;
		}
		else{
		    for(j = 1; argv[i][j] != '\0'; j++){
			switch(argv[i][j]){
#ifdef NMEDIT
			case 'p':
			    pflag = 1;
			    break;
#else /* !defined(NMEDIT) */
			case 'S':
			    Sflag = 1;
			    strip_all = 0;
			    break;
			case 'X':
			    Xflag = 1;
			    strip_all = 0;
			    break;
			case 'T':
			    Tflag = 1;
			    strip_all = 0;
			    break;
			case 'N':
			    Nflag = 1;
			    break;
			case 'x':
			    xflag = 1;
			    strip_all = 0;
			    break;
			case 'i':
			    iflag = 1;
			    break;
			case 'u':
			    uflag = 1;
			    strip_all = 0;
			    break;
			case 'r':
			    rflag = 1;
			    strip_all = 0;
			    break;
			case 'n':
			    nflag = 1;
			    strip_all = 0;
			    break;
#endif /* !defined(NMEDIT) */
			case 'A':
			    Aflag = 1;
#ifndef NMEDIT
			    strip_all = 0;
#endif /* !defined(NMEDIT) */
			    break;
#ifndef NMEDIT
			case 'c':
			    cflag = 1;
			    strip_all = 0;
			    break;
			case 'v':
			    vflag = 1;
			    break;
			case 'l':
			    lflag = 1;
			    break;
#endif /* NMEDIT */
			default:
			    error("unrecognized option: %s", argv[i]);
			    usage();
			}
		    }
		}
	    }
	    else
		files_specified++;
	}
	/*
	 * This allows testing of stripping all nlists and string tables.
	 */
#ifndef NMEDIT
	if(getenv("STRIP_NLISTS") != NULL)
	    Nflag = 1;
#endif /* !defined(NMEDIT) */
	if(args_left == 0)
	    files_specified += argc - (i + 1);
	
	if(files_specified > 1 && output_file != NULL){
	    error("-o <filename> can only be used when one file is specified");
	    usage();
	}

	if(sfile){
	    setup_symbol_list(sfile, &save_symbols, &nsave_symbols);
	}
#ifdef NMEDIT
	else{
	    if(Rfile == NULL && pflag == 0){
		error("-s <filename>, -R <filename> or -p argument required");
		usage();
	    }
	}
#endif /* NMEDIT */

	if(Rfile){
	    setup_symbol_list(Rfile, &remove_symbols, &nremove_symbols);
	    if(sfile){
		for(j = 0; j < nremove_symbols ; j++){
		    sp = bsearch(remove_symbols[j].name,
				 save_symbols, nsave_symbols,
				 sizeof(struct symbol_list),
				 (int (*)(const void *, const void *))
				    symbol_list_bsearch);
		    if(sp != NULL){
			error("symbol name: %s is listed in both -s %s and -R "
			      "%s files (can't be both saved and removed)",
			      remove_symbols[j].name, sfile, Rfile);
		    }
		}
		if(errors)
		    exit(EXIT_FAILURE);
	    }
	}

	/* the default when no -arch flags is present is to strip all archs */
	if(narch_flags == 0)
	   all_archs = TRUE;

#ifndef NMEDIT
	if(dfile){
	    setup_debug_filenames(dfile);
	}
#endif /* !defined(NMEDIT) */

	files_specified = 0;
	args_left = 1;
	for (i = 1; i < argc; i++) {
	    if(args_left && argv[i][0] == '-'){
		if(argv[i][1] == '\0')
		    args_left = 0;
		else if(strcmp(argv[i], "-o") == 0 ||
			strcmp(argv[i], "-s") == 0 ||
			strcmp(argv[i], "-R") == 0 ||
#ifndef NMEDIT
			strcmp(argv[i], "-d") == 0 ||
#endif /* !defined(NMEDIT) */
			strcmp(argv[i], "-arch") == 0)
		    i++;
	    }
	    else{
		char resolved_path[PATH_MAX + 1];

		if(realpath(argv[i], resolved_path) == NULL)
		    strip_file(argv[i], arch_flags, narch_flags, all_archs);
		else
		    strip_file(resolved_path, arch_flags,narch_flags,all_archs);
		files_specified++;
	    }
	}
	if(files_specified == 0)
	    fatal("no files specified");

	if(errors)
	    return(EXIT_FAILURE);
	else
	    return(EXIT_SUCCESS);
}

static
void
usage(
void)
{
#ifndef NMEDIT
	fprintf(stderr, "Usage: %s [-AnuSXx] [-] [-d filename] [-s filename] "
		"[-R filename] [-o output] file [...] \n", progname);
#else /* defined(NMEDIT) */
	fprintf(stderr, "Usage: %s -s filename [-R filename] [-p] [-A] [-] "
		"[-o output] file [...] \n",
		progname);
#endif /* NMEDIT */
	exit(EXIT_FAILURE);
}

static
void
strip_file(
char *input_file,
struct arch_flag *arch_flags,
uint32_t narch_flags,
enum bool all_archs)
{
    struct ofile *ofile;
    struct arch *archs;
    uint32_t narchs;
    struct stat stat_buf;
    uint32_t previous_errors;
    enum bool unix_standard_mode;
    int cwd_fd;
    char *rename_file;
#ifndef NMEDIT
    char *p;
#endif

	archs = NULL;
	narchs = 0;
	previous_errors = errors;
	errors = 0;

	/* breakout the file for processing */
	ofile = breakout(input_file, &archs, &narchs, FALSE);
	if(errors)
	    return;

	/* checkout the file for symbol table replacement processing */
	checkout(archs, narchs);

	/* process the symbols in the input file */
	strip_arch(archs, narchs, arch_flags, narch_flags, all_archs);
	if(errors){
	    free_archs(archs, narchs);
	    ofile_unmap(ofile);
	    return;
	}

	/* create the output file */
	if(stat(input_file, &stat_buf) == -1)
	    system_error("can't stat input file: %s", input_file);
	if(output_file != NULL){
	    writeout(archs, narchs, output_file, stat_buf.st_mode & 0777,
		     TRUE, FALSE,
#ifdef NMEDIT
		     FALSE,
#else
		     toc64flag,
#endif
		     FALSE, NULL);
	    FAKE_SIGN_ARM_BINARY(archs, narchs, output_file); /* cctools-port */
	}
	else{
	    unix_standard_mode = get_unix_standard_mode();
	    rename_file = NULL;
	    cwd_fd = -1;
#ifdef NMEDIT
	    output_file = makestr(input_file, ".nmedit", NULL);
#else /* !defined(NMEDIT) */
	    /*
	     * In UNIX standard conformance mode we are not allowed to replace
	     * a file that is not writeable.
	     */
	    if(unix_standard_mode == TRUE && 
	       access(input_file, W_OK) == -1){
		system_error("file: %s is not writable", input_file);
		goto strip_file_return;
	    }
	    output_file = makestr(input_file, ".strip", NULL);

	    /*
	     * The UNIX standard conformance test suite expects files of
	     * MAXPATHLEN to work.
	     */
	    if(strlen(output_file) >= MAXPATHLEN){
		/*
		 * If there is a directory path in the name try to change
		 * the current working directory to that path.
		 */
		if((p = rindex(output_file, '/')) != NULL){
		    if((cwd_fd = open(".", O_RDONLY, 0)) == -1){
			system_error("can't open current working directory");
			goto strip_file_return;
		    }
		    *p = '\0';
		    if(chdir(output_file) == -1){
			system_error("can't change current working directory "
				     "to: %s", output_file);
			goto strip_file_return;
		    }
		    p = rindex(input_file, '/');
		    rename_file = makestr(p + 1, NULL);
		}
		/*
		 * Create what might be a short enough name.
		 */
		free(output_file);
		output_file = makestr("strip.XXXXXX", NULL);
		output_file = mktemp(output_file);
	    }
#endif /* NMEDIT */
	    writeout(archs, narchs, output_file, stat_buf.st_mode & 0777,
		     TRUE, FALSE,
#ifdef NMEDIT
		     FALSE,
#else
		     toc64flag,
#endif
		     FALSE, NULL);
	    FAKE_SIGN_ARM_BINARY(archs, narchs, output_file); /* cctools-port */
	    if(rename_file != NULL){
		if(rename(output_file, rename_file) == -1)
		    system_error("can't move temporary file: %s to file: %s",
				 output_file, rename_file);
		free(rename_file);
	    }
	    else{
		if(rename(output_file, input_file) == -1)
		    system_error("can't move temporary file: %s to input "
				 "file: %s", output_file, input_file);
	    }
	    free(output_file);
	    output_file = NULL;

	    /*
	     * If we changed the current working directory change back to
	     * the previous working directory.
	     */
	    if(cwd_fd != -1){
		if(fchdir(cwd_fd) == -1)
		    system_error("can't change back to previous working "
				 "directory");
		if(close(cwd_fd) == -1)
		    system_error("can't close previous working directory");
	    }
	}

#ifndef NMEDIT
strip_file_return:
#endif /* !defined(NMEDIT) */
	/* clean-up data structures */
	free_archs(archs, narchs);
	ofile_unmap(ofile);

	errors += previous_errors;
}

static
void
strip_arch(
struct arch *archs,
uint32_t narchs,
struct arch_flag *arch_flags,
uint32_t narch_flags,
enum bool all_archs)
{
    uint32_t i, j, k, offset, size, missing_syms;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    struct arch_flag host_arch_flag;
    enum bool arch_process, any_processing, *arch_flag_processed, family;
    const struct arch_flag *family_arch_flag;
    struct ar_hdr h;
    char size_buf[sizeof(h.ar_size) + 1];
    char date_buf[sizeof(h.ar_date) + 1];
    enum bool zero_ar_date;

	zero_ar_date = getenv("ZERO_AR_DATE") ? TRUE : FALSE;

	/*
	 * Using the specified arch_flags process specified objects for those
	 * architecures.
	 */
	any_processing = FALSE;
	arch_flag_processed = NULL;
	if(narch_flags != 0)
	    arch_flag_processed = allocate(narch_flags * sizeof(enum bool));
	memset(arch_flag_processed, '\0', narch_flags * sizeof(enum bool));
	for(i = 0; i < narchs; i++){
	    /*
	     * Determine the architecture (cputype and cpusubtype) of arch[i]
	     */
	    cputype = 0;
	    cpusubtype = 0;
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			cputype = archs[i].members[j].object->mh_cputype;
			cpusubtype = archs[i].members[j].object->mh_cpusubtype;
			break;
		    }
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		cputype = archs[i].object->mh_cputype;
		cpusubtype = archs[i].object->mh_cpusubtype;
	    }
	    else if(archs[i].fat_arch64 != NULL){
		cputype = archs[i].fat_arch64->cputype;
		cpusubtype = archs[i].fat_arch64->cpusubtype;
	    }
	    else if(archs[i].fat_arch != NULL){
		cputype = archs[i].fat_arch->cputype;
		cpusubtype = archs[i].fat_arch->cpusubtype;
	    }
	    arch_process = FALSE;
	    if(all_archs == TRUE){
		arch_process = TRUE;
	    }
	    else if(narch_flags != 0){
		family = FALSE;
		if(narch_flags == 1){
		    family_arch_flag =
			get_arch_family_from_cputype(arch_flags[0].cputype);
		    if(family_arch_flag != NULL)
			family = (enum bool)
			  ((family_arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			   (arch_flags[0].cpusubtype & ~CPU_SUBTYPE_MASK));
		}
		for(j = 0; j < narch_flags; j++){
		    if(arch_flags[j].cputype == cputype &&
		       ((arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			(cpusubtype & ~CPU_SUBTYPE_MASK) ||
			family == TRUE)){
			arch_process = TRUE;
			arch_flag_processed[j] = TRUE;
			break;
		    }
		}
	    }
	    else{
		(void)get_arch_from_host(&host_arch_flag, NULL);
		if(host_arch_flag.cputype == cputype &&
		   (host_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   (cpusubtype & ~CPU_SUBTYPE_MASK))
		    arch_process = TRUE;
	    }
	    if(narchs != 1 && arch_process == FALSE)
		continue;
	    any_processing = TRUE;

	    /*
	     * Now this arch[i] has been selected to be processed so process it
	     * according to its type.
	     */
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			strip_object(archs + i, archs[i].members + j,
				     archs[i].members[j].object);
		    }
		}
		missing_syms = 0;
		if(iflag == 0){
		    for(k = 0; k < nsave_symbols; k++){
			if(save_symbols[k].seen == FALSE){
			    if(missing_syms == 0){
				error_arch(archs + i, NULL, "symbols names "
					   "listed in: %s not in: ", sfile);
				missing_syms = 1;
			    }
			    fprintf(stderr, "%s\n", save_symbols[k].name);
			}
		    }
		}
		for(k = 0; k < nsave_symbols; k++){
		    save_symbols[k].seen = FALSE;
		}
		missing_syms = 0;
		if(iflag == 0){
		    for(k = 0; k < nremove_symbols; k++){
			if(remove_symbols[k].seen == FALSE){
			    if(missing_syms == 0){
				error_arch(archs + i, NULL, "symbols names "
					   "listed in: %s not defined in: ",
					   Rfile);
				missing_syms = 1;
			    }
			    fprintf(stderr, "%s\n", remove_symbols[k].name);
			}
		    }
		}
		for(k = 0; k < nremove_symbols; k++){
		    remove_symbols[k].seen = FALSE;
		}
		/*
		 * Reset the library offsets and size.
		 */
		offset = 0;
		for(j = 0; j < archs[i].nmembers; j++){
		    archs[i].members[j].offset = offset;
		    size = 0;
		    if(archs[i].members[j].member_long_name == TRUE){
			size = rnd32(archs[i].members[j].member_name_size, 8) +
			       (rnd32(sizeof(struct ar_hdr), 8) -
				sizeof(struct ar_hdr));
			archs[i].toc_long_name = TRUE;
		    }
		    if(archs[i].members[j].object != NULL){
			size += 
			   rnd(archs[i].members[j].object->object_size -
			     archs[i].members[j].object->input_sym_info_size +
			     archs[i].members[j].object->output_sym_info_size, 
			     8);
			sprintf(size_buf, "%-*ld",
			   (int)sizeof(archs[i].members[j].ar_hdr->ar_size),
			   (long)(size));
			/*
			 * This has to be done by hand because sprintf puts a
			 * null at the end of the buffer.
			 */
			memcpy(archs[i].members[j].ar_hdr->ar_size, size_buf,
			   (int)sizeof(archs[i].members[j].ar_hdr->ar_size));
		    }
		    else{
			size += archs[i].members[j].unknown_size;
		    }
		    offset += sizeof(struct ar_hdr) + size;
		}
		archs[i].library_size = offset;
		/*
		 * Reset the library date, if needed
		 */
		if (zero_ar_date == TRUE) {
		    sprintf(date_buf, "%-*ld", (int)sizeof(h.ar_date),
			    (unsigned long)0);
		    /*
		     * This has to be done by hand because sprintf puts a
		     * null at the end of the buffer.
		     */
		    for(j = 0; j < archs[i].nmembers; j++){
			memcpy(archs[i].members[j].ar_hdr->ar_date, date_buf,
			   (int)sizeof(archs[i].members[j].ar_hdr->ar_date));
		    }
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		strip_object(archs + i, NULL, archs[i].object);
	    }
	    else {
		warning_arch(archs + i, NULL, "can't process non-object and "
			   "non-archive file: ");
		return;
	    }
	}
	if(all_archs == FALSE && narch_flags != 0){
	    for(i = 0; i < narch_flags; i++){
		if(arch_flag_processed[i] == FALSE)
		    error("file: %s does not contain architecture: %s",
			  archs[0].file_name, arch_flags[i].name);
	    }
	    free(arch_flag_processed);
	}
	if(any_processing == FALSE)
	    fatal("no processing done on input file: %s (specify a -arch flag)",
		  archs[0].file_name);
}

static
void
strip_object(
struct arch *arch,
struct member *member,
struct object *object)
{
    enum byte_sex host_byte_sex;
    uint32_t offset;
    struct dylib_table_of_contents *tocs;
    uint32_t ntoc;
    struct dylib_module *mods;
    struct dylib_module_64 *mods64;
    uint32_t nmodtab;
    struct dylib_reference *refs;
    uint32_t nextrefsyms;
    uint32_t i, j;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct relocation_info *relocs;
    struct scattered_relocation_info *sreloc;
    int32_t missing_reloc_symbols;
    uint32_t stride, section_type, nitems;
    char *contents;
    uint32_t dyld_info_start;
    uint32_t dyld_info_end;
#ifndef NMEDIT
    uint32_t flags;
    uint32_t k;
#endif
    uint32_t ncmds;
    uint32_t swift_version;
    enum bool nlist_outofsync_with_dyldinfo;
    uint32_t mh_flags;

	if(object->mh != NULL)
	    mh_flags = object->mh->flags;
	else
	    mh_flags = object->mh64->flags;

	host_byte_sex = get_host_byte_sex();
	swift_version = 0;
	nlist_outofsync_with_dyldinfo = FALSE;

	/* Don't do anything to stub dylibs which have no load commands. */
	if(object->mh_filetype == MH_DYLIB_STUB){
	    if((object->mh != NULL && object->mh->ncmds == 0) ||
	       (object->mh64 != NULL && object->mh64->ncmds == 0)){
		return;
	    }
	}
	if(object->mh_filetype == MH_DSYM)
	    fatal_arch(arch, member, "can't process dSYM companion file: ");
	if(object->st == NULL || object->st->nsyms == 0){
	    warning_arch(arch, member, "input object file stripped: ");
	    return;
	}

	nsyms = object->st->nsyms;
	if(object->mh != NULL){
	    symbols = (struct nlist *)
		      (object->object_addr + object->st->symoff);
	    if(object->object_byte_sex != host_byte_sex)
		swap_nlist(symbols, nsyms, host_byte_sex);
	    symbols64 = NULL;
	}
	else{
	    symbols = NULL;
	    symbols64 = (struct nlist_64 *)
		        (object->object_addr + object->st->symoff);
	    if(object->object_byte_sex != host_byte_sex)
		swap_nlist_64(symbols64, nsyms, host_byte_sex);
	}
	strings = object->object_addr + object->st->stroff;
	strsize = object->st->strsize;

#ifndef NMEDIT
	if(object->mh != NULL)
	    flags = object->mh->flags;
	else
	    flags = object->mh64->flags;
	if(object->mh_filetype == MH_DYLIB &&
	   (flags & MH_PREBOUND) != MH_PREBOUND){
	    arch->dont_update_LC_ID_DYLIB_timestamp = TRUE;
	}
	if(object->mh_filetype != MH_DYLIB && cflag)
	    fatal_arch(arch, member, "-c can't be used on non-dynamic "
		       "library: ");
#endif /* !(NMEDIT) */
	if(object->mh_filetype == MH_DYLIB_STUB)
	    fatal_arch(arch, member, "dynamic stub library can't be changed "
		       "once created: ");

	if(object->mh_filetype == MH_DYLIB){
	    tocs = (struct dylib_table_of_contents *)
		    (object->object_addr + object->dyst->tocoff);
	    ntoc = object->dyst->ntoc;
	    nmodtab = object->dyst->nmodtab;
	    if(object->mh != NULL){
		mods = (struct dylib_module *)
			(object->object_addr + object->dyst->modtaboff);
		if(object->object_byte_sex != host_byte_sex)
		    swap_dylib_module(mods, nmodtab, host_byte_sex);
		mods64 = NULL;
	    }
	    else{
		mods = NULL;
		mods64 = (struct dylib_module_64 *)
			  (object->object_addr + object->dyst->modtaboff);
		if(object->object_byte_sex != host_byte_sex)
		    swap_dylib_module_64(mods64, nmodtab, host_byte_sex);
	    }
	    refs = (struct dylib_reference *)
		    (object->object_addr + object->dyst->extrefsymoff);
	    nextrefsyms = object->dyst->nextrefsyms;
	    if(object->object_byte_sex != host_byte_sex){
		swap_dylib_table_of_contents(tocs, ntoc, host_byte_sex);
		swap_dylib_reference(refs, nextrefsyms, host_byte_sex);
	    }
#ifndef NMEDIT
	    /* 
	     * In the -c flag is specified then strip the section contents of
	     * this dynamic library and change it into a stub library.  When
	     * creating a stub library the timestamp is not changed.
	     */
	    if(cflag){
		arch->dont_update_LC_ID_DYLIB_timestamp = TRUE;

		lc = object->load_commands;
		if(object->mh != NULL){
		    ncmds = object->mh->ncmds;
		    object->mh_filetype = MH_DYLIB_STUB;
		    object->mh->filetype = MH_DYLIB_STUB;
		}
		else{
		    ncmds = object->mh64->ncmds;
		    object->mh_filetype = MH_DYLIB_STUB;
		    object->mh64->filetype = MH_DYLIB_STUB;
		}
		for(i = 0; i < ncmds; i++){
		    if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			if(strcmp(sg->segname, SEG_LINKEDIT) != 0){
			    /*
			     * Zero out the section offset, reloff, and size
			     * fields as the section contents are being removed.
			     */
			    s = (struct section *)
				 ((char *)sg + sizeof(struct segment_command));
			    for(j = 0; j < sg->nsects; j++){
				/*
				 * For section types with indirect tables we
				 * change the type to S_REGULAR and clear the
				 * reserved1 and reserved2 fields so not to
				 * confuse tools that would be trying to look
				 * at the indirect table since it is now removed
				 * when creating a stub library.
				 */ 
				section_type = s[j].flags & SECTION_TYPE;
				if(section_type == S_SYMBOL_STUBS ||
				   section_type == S_LAZY_SYMBOL_POINTERS ||
				   section_type ==
						S_LAZY_DYLIB_SYMBOL_POINTERS ||
				   section_type == S_NON_LAZY_SYMBOL_POINTERS){
				    s[j].flags = S_REGULAR;
				    s[j].reserved1 = 0;
				    s[j].reserved2 = 0;
				}
				s[j].size    = 0;
				s[j].addr    = 0;
				s[j].offset  = 0;
				s[j].reloff  = 0;
			    }
			    /* zero out file offset and size in the segment */
			    sg->fileoff = 0;
			    sg->filesize = 0;
			}
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			if(strcmp(sg64->segname, SEG_LINKEDIT) != 0){
			    /*
			     * Zero out the section offset, reloff, and size
			     * fields as the section contents are being removed.
			     */
			    s64 = (struct section_64 *)
				  ((char *)sg64 +
				   sizeof(struct segment_command_64));
			    for(j = 0; j < sg64->nsects; j++){
				/*
				 * For section types with indirect tables we
				 * change the type to S_REGULAR and clear the
				 * reserved1 and reserved2 fields so not to
				 * confuse tools that would be trying to look
				 * at the indirect table since it is now removed
				 * when creating a stub library.
				 */ 
				section_type = s64[j].flags & SECTION_TYPE;
				if(section_type == S_SYMBOL_STUBS ||
				   section_type == S_LAZY_SYMBOL_POINTERS ||
				   section_type ==
						S_LAZY_DYLIB_SYMBOL_POINTERS ||
				   section_type == S_NON_LAZY_SYMBOL_POINTERS){
				    s64[j].flags = S_REGULAR;
				    s64[j].reserved1 = 0;
				    s64[j].reserved2 = 0;
				}
				s64[j].size    = 0;
				s64[j].addr    = 0;
				s64[j].offset  = 0;
				s64[j].reloff  = 0;
			    }
			    /* zero out file offset and size in the segment */
			    sg64->fileoff = 0;
			    sg64->filesize = 0;
			}
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		/*
		 * To get the right amount of the file copied out by writeout()
		 * for the case when we are stripping out the section contents
		 * we reduce the object size by the size of the section contents
		 * including the padding after the load commands.  Then this
		 * size minus the size of the input symbolic information is
		 * copied out.
		 */
		if(object->mh != NULL){
		    object->object_size -= (object->seg_linkedit->fileoff -
			(sizeof(struct mach_header) +
			object->mh->sizeofcmds));
		    /*
		     * Set the file offset to the link edit information to be
		     * right after the load commands.
		     */
		    object->seg_linkedit->fileoff = 
			sizeof(struct mach_header) +
			object->mh->sizeofcmds;
		}
		else{
		    object->object_size -= (object->seg_linkedit64->fileoff -
			(sizeof(struct mach_header_64) +
			 object->mh64->sizeofcmds));
		    /*
		     * Set the file offset to the link edit information to be
		     * right after the load commands.
		     */
		    object->seg_linkedit64->fileoff = 
			sizeof(struct mach_header_64) +
			object->mh64->sizeofcmds;
		}
	    }
#endif /* !(NMEDIT) */
	}
	else{
	    tocs = NULL;
	    ntoc = 0;
	    mods = NULL;
	    mods64 = NULL;
	    nmodtab = 0;
	    refs = NULL;
	    nextrefsyms = 0;
	}

	/*
	 * coalesced symbols can be stripped only if they are not used via an
	 * symbol pointer.  So to know that strip_symtab() needs to be passed
	 * the indirect symbol table.
	 */
	if(object->dyst != NULL && object->dyst->nindirectsyms != 0){
	    nindirectsyms = object->dyst->nindirectsyms;
	    indirectsyms = (uint32_t *)
		(object->object_addr + object->dyst->indirectsymoff);
	    if(object->object_byte_sex != host_byte_sex)
		swap_indirect_symbols(indirectsyms, nindirectsyms,
				      host_byte_sex);
	}
	else{
	    indirectsyms = NULL;
	    nindirectsyms = 0;
	}

	if(object->mh != NULL)
	    object->input_sym_info_size =
		nsyms * sizeof(struct nlist) +
		strsize;
	else
	    object->input_sym_info_size =
		nsyms * sizeof(struct nlist_64) +
		strsize;
#ifndef NMEDIT
	if(object->mh != NULL)
	    flags = object->mh->flags;
	else
	    flags = object->mh64->flags;
	if(strip_all &&
	   (flags & MH_DYLDLINK) == MH_DYLDLINK &&
	   object->mh_filetype == MH_EXECUTE)
	    default_dyld_executable = TRUE;
	else
	    default_dyld_executable = FALSE;
	if(Nflag &&
           (mh_flags & MH_DYLDLINK) == MH_DYLDLINK &&
           object->mh_filetype != MH_KEXT_BUNDLE &&
	   object->dyst != NULL && object->dyst->nextrel == 0)
	    strip_all_nlists = TRUE;
	else
	    strip_all_nlists = FALSE;

#endif /* !defined(NMEDIT) */

#ifndef NMEDIT
	if(sfile != NULL || Rfile != NULL || dfile != NULL || Aflag || uflag ||
	   Sflag || xflag || Xflag || Tflag || nflag || rflag ||
	   default_dyld_executable || object->mh_filetype == MH_DYLIB ||
	   object->mh_filetype == MH_DYLINKER)
#endif /* !defined(NMEDIT) */
	    {
#ifdef NMEDIT
	    if(edit_symtab(arch, member, object, symbols, symbols64, nsyms,
		strings, strsize, tocs, ntoc, mods, mods64, nmodtab, refs,
		nextrefsyms) == FALSE)
		return;
#else /* !defined(NMEDIT) */
	    if(strip_symtab(arch, member, object, tocs, ntoc, mods, mods64,
			    nmodtab, refs, nextrefsyms, &swift_version,
			    &nlist_outofsync_with_dyldinfo) == FALSE)
		return;
	    if(no_uuid == TRUE)
		strip_LC_UUID_commands(arch, member, object);
#endif /* !defined(NMEDIT) */
	    /*
	     * The parts that make up output_sym_info_size must be added up in
	     * the output order so that when the sizes of things are rounded up
	     * before parts that must be aligned the final output_sym_info_size
	     * is correct.
	     *
	     * Also the parts that make up input_sym_info_size must be added up
	     * in the same way.  And must be done here as the input file may
	     * have been changed to an "ld -r" file and may not be the
	     * the original input file.
	     */
	    object->output_sym_info_size = 0;
	    object->input_sym_info_size = 0;
	    if(object->dyld_info != NULL){
		/* there are five parts to the dyld info, but
		 strip does not alter them, so copy as a block */
		dyld_info_start = 0;
		if (object->dyld_info->rebase_off != 0)
		    dyld_info_start = object->dyld_info->rebase_off;
		else if (object->dyld_info->bind_off != 0)
		    dyld_info_start = object->dyld_info->bind_off;
		else if (object->dyld_info->weak_bind_off != 0)
		    dyld_info_start = object->dyld_info->weak_bind_off;
		else if (object->dyld_info->lazy_bind_off != 0)
		    dyld_info_start = object->dyld_info->lazy_bind_off;
		else if (object->dyld_info->export_off != 0)
		    dyld_info_start = object->dyld_info->export_off;
		dyld_info_end = 0;
		if (object->dyld_info->export_size != 0)
		    dyld_info_end = object->dyld_info->export_off
			+ object->dyld_info->export_size;
		else if (object->dyld_info->lazy_bind_size != 0)
		    dyld_info_end = object->dyld_info->lazy_bind_off
			+ object->dyld_info->lazy_bind_size;
		else if (object->dyld_info->weak_bind_size != 0)
		    dyld_info_end = object->dyld_info->weak_bind_off
			+ object->dyld_info->weak_bind_size;
		else if (object->dyld_info->bind_size != 0)
		    dyld_info_end = object->dyld_info->bind_off
			+ object->dyld_info->bind_size;
		else if (object->dyld_info->rebase_size != 0)
		    dyld_info_end = object->dyld_info->rebase_off
			+ object->dyld_info->rebase_size;
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub we only keep the dyld export data.
		 */
		if(cflag){
		    if(object->dyld_info->export_off != 0){
			dyld_info_start = object->dyld_info->export_off;
			dyld_info_end = object->dyld_info->export_off
			    + object->dyld_info->export_size;
		    }
		    else{
			dyld_info_start = 0;
			dyld_info_end = 0;
		    }
		}
#endif /* !(NMEDIT) */
		object->output_dyld_info = object->object_addr +dyld_info_start; 
		object->output_dyld_info_size = dyld_info_end - dyld_info_start;
		object->output_sym_info_size += object->output_dyld_info_size;
		/*
		 * Warn about strip -s or -R on a final linked image with
		 * dyld_info.
		 */
		if(nsave_symbols != 0){
		    warning_arch(arch, NULL, "removing global symbols from a "
			         "final linked no longer supported.  Use "
				 "-exported_symbols_list at link time when "
				 "building: ");
		}
		object->input_sym_info_size += object->dyld_info->rebase_size
					    + object->dyld_info->bind_size
					    + object->dyld_info->weak_bind_size
					    + object->dyld_info->lazy_bind_size
					    + object->dyld_info->export_size;
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub we only keep the dyld export data,
		 * so zero the offsets and sizes of the other dyld data items.
		 */
		if(cflag){
		    object->dyld_info->rebase_off = 0;
		    object->dyld_info->rebase_size = 0;
		    object->dyld_info->bind_off = 0;
		    object->dyld_info->bind_size = 0;
		    object->dyld_info->weak_bind_off = 0;
		    object->dyld_info->weak_bind_size = 0;
		    object->dyld_info->lazy_bind_off = 0;
		    object->dyld_info->lazy_bind_size = 0;
		}
#endif /* !(NMEDIT) */
	    }

	    if(object->dyld_chained_fixups != NULL) {
#ifndef NMEDIT
		if(!cflag)
#endif /* !(NMEDIT) */
		{
		    object->output_sym_info_size +=
			object->dyld_chained_fixups->datasize;
		    object->output_dyld_chained_fixups_data =
			object->object_addr +
			object->dyld_chained_fixups->dataoff;
		    object->output_dyld_chained_fixups_data_size =
			object->dyld_chained_fixups->datasize;
		}
		object->input_sym_info_size +=
		    object->dyld_chained_fixups->datasize;
	    }

	    if(object->dyld_exports_trie != NULL) {
		object->input_sym_info_size +=
		    object->dyld_exports_trie->datasize;
		object->output_sym_info_size +=
		    object->dyld_exports_trie->datasize;
		object->output_dyld_exports_trie_data =
		    object->object_addr + object->dyld_exports_trie->dataoff;
		object->output_dyld_exports_trie_data_size =
		    object->dyld_exports_trie->datasize;
	    }

	    if(object->dyst != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub the relocation info also gets
		 * stripped.
		 */
		if(!cflag) 
#endif /* !(NMEDIT) */
		{
		    object->output_sym_info_size +=
			object->dyst->nlocrel * sizeof(struct relocation_info);
		}
		object->input_sym_info_size +=
		    object->dyst->nlocrel * sizeof(struct relocation_info);
	    }

	    if(object->split_info_cmd != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub we also remove the split info data.
		 * And we also remove the split info data if the -no_split_info
		 * option is specified
		 */
		if(!cflag && !no_split_info)
#endif /* !(NMEDIT) */
		{
		    object->output_split_info_data = object->object_addr +
			object->split_info_cmd->dataoff;
		    object->output_split_info_data_size = 
			object->split_info_cmd->datasize;
		    object->output_sym_info_size +=
			object->split_info_cmd->datasize;
		}
		object->input_sym_info_size += object->split_info_cmd->datasize;
	    }
#ifndef NMEDIT
	    if(no_split_info == TRUE)
		strip_LC_SEGMENT_SPLIT_INFO_command(arch, member, object);
#endif /* !(NMEDIT) */

	    if(object->func_starts_info_cmd != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub we also remove the function starts info.
		 */
		if(!cflag)
#endif /* !(NMEDIT) */
		{
		    object->output_func_start_info_data = object->object_addr +
			object->func_starts_info_cmd->dataoff;
		    object->output_func_start_info_data_size = 
			object->func_starts_info_cmd->datasize;
		    object->output_sym_info_size +=
			object->func_starts_info_cmd->datasize;
		}
		object->input_sym_info_size +=
		    object->func_starts_info_cmd->datasize;
	    }

	    if(object->data_in_code_cmd != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub we also remove the data in code info.
		 */
		if(!cflag)
#endif /* !(NMEDIT) */
		{
		    object->output_data_in_code_info_data =
			object->object_addr +
			object->data_in_code_cmd->dataoff;
		    object->output_data_in_code_info_data_size = 
			object->data_in_code_cmd->datasize;
		    object->output_sym_info_size +=
			object->data_in_code_cmd->datasize;
		}
		object->input_sym_info_size +=
		    object->data_in_code_cmd->datasize;
	    }

	    if(object->code_sign_drs_cmd != NULL){
		object->output_code_sign_drs_info_data = object->object_addr +
		    object->code_sign_drs_cmd->dataoff;
		object->output_code_sign_drs_info_data_size = 
		    object->code_sign_drs_cmd->datasize;
		object->input_sym_info_size +=
		    object->code_sign_drs_cmd->datasize;
		object->output_sym_info_size +=
		    object->code_sign_drs_cmd->datasize;
	    }

	    if(object->link_opt_hint_cmd != NULL){
		object->output_link_opt_hint_info_data = object->object_addr +
		    object->link_opt_hint_cmd->dataoff;
		object->output_link_opt_hint_info_data_size = 
		    object->link_opt_hint_cmd->datasize;
		object->input_sym_info_size +=
		    object->link_opt_hint_cmd->datasize;
		object->output_sym_info_size +=
		    object->link_opt_hint_cmd->datasize;
	    }

	    if(object->mh != NULL){
		object->input_sym_info_size += nsyms * sizeof(struct nlist);
		object->output_symbols = new_symbols;
		object->output_sym_info_size +=
		    new_nsyms * sizeof(struct nlist);
	    }
	    else{
		object->input_sym_info_size += nsyms * sizeof(struct nlist_64);
		object->output_symbols64 = new_symbols64;
		object->output_sym_info_size +=
		    new_nsyms * sizeof(struct nlist_64);
	    }
	    object->output_nsymbols = new_nsyms;
	    object->st->nsyms = new_nsyms; 

	    if(object->hints_cmd != NULL){
		object->input_sym_info_size +=
		    object->hints_cmd->nhints *
		    sizeof(struct twolevel_hint);
		object->output_sym_info_size +=
		    object->hints_cmd->nhints *
		    sizeof(struct twolevel_hint);
	    }

	    if(object->dyst != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub the relocation info also gets
		 * stripped.
		 */
		if(!cflag) 
#endif /* !(NMEDIT) */
		{
		    object->output_sym_info_size +=
			object->dyst->nextrel * sizeof(struct relocation_info);
		}
		object->input_sym_info_size +=
		    object->dyst->nextrel * sizeof(struct relocation_info);
	    }

	    if(object->dyst != NULL){
#ifndef NMEDIT
		/*
		 * When stripping out the section contents to create a
		 * dynamic library stub the indirect symbol table also gets
		 * stripped.
		 */
		if(!cflag) 
#endif /* !(NMEDIT) */
		{
		    object->output_sym_info_size +=
			object->dyst->nindirectsyms * sizeof(uint32_t) +
			object->input_indirectsym_pad;
		}
		if(object->mh != NULL){
		    object->input_sym_info_size +=
			object->dyst->nindirectsyms * sizeof(uint32_t);
		}
		else{
		    object->input_sym_info_size +=
			object->dyst->nindirectsyms * sizeof(uint32_t) +
			object->input_indirectsym_pad;
		}
	    }

	    if(object->dyst != NULL){
		object->output_sym_info_size +=
		    new_ntoc * sizeof(struct dylib_table_of_contents);
		object->input_sym_info_size +=
		    object->dyst->ntoc * sizeof(struct dylib_table_of_contents);
	    }

	    if(object->dyst != NULL){
		if(object->mh != NULL){
		    object->output_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module);
		    object->input_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module);
		}
		else{
		    object->output_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module_64);
		    object->input_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module_64);
		}
	    }

	    if(object->dyst != NULL){
		object->output_sym_info_size +=
		    new_nextrefsyms * sizeof(struct dylib_reference);
		object->input_sym_info_size +=
		    object->dyst->nextrefsyms * sizeof(struct dylib_reference);
	    }

	    object->output_strings = new_strings;
	    object->output_strings_size = new_strsize;
	    object->output_sym_info_size += new_strsize;
	    object->input_sym_info_size += strsize;
	    object->st->strsize = new_strsize;

	    if(object->code_sig_cmd != NULL){
#ifndef NMEDIT
		if(!cflag)
#endif /* !(NMEDIT) */
		{
		    object->output_code_sig_data = object->object_addr +
			object->code_sig_cmd->dataoff;
		    object->output_code_sig_data_size = 
			object->code_sig_cmd->datasize;
		}
		object->input_sym_info_size =
		    rnd32(object->input_sym_info_size, 16);
		object->input_sym_info_size +=
		    object->code_sig_cmd->datasize;
#ifndef NMEDIT
		if(cflag){
		    strip_LC_CODE_SIGNATURE_commands(arch, member, object);
		}
		else
#endif /* !(NMEDIT) */
		{
		    object->output_sym_info_size =
			rnd32(object->output_sym_info_size, 16);
		    object->output_sym_info_size +=
			object->code_sig_cmd->datasize;
		}
	    }

	    if(object->dyst != NULL){
		object->dyst->ilocalsym = 0;
		object->dyst->nlocalsym = new_nlocalsym;
		object->dyst->iextdefsym = new_nlocalsym;
		object->dyst->nextdefsym = new_nextdefsym;
		object->dyst->iundefsym = new_nlocalsym + new_nextdefsym;
		object->dyst->nundefsym = new_nundefsym;
		if(object->dyst->nindirectsyms != 0){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the indirect symbol table also gets
		     * stripped.
		     */
		    if(cflag){
			object->dyst->nindirectsyms = 0;
		    }
		    else
#endif /* !(NMEDIT) */
		    {
			object->output_indirect_symtab = indirectsyms;
			if(object->object_byte_sex != host_byte_sex)
			    swap_indirect_symbols(indirectsyms, nindirectsyms,
						  object->object_byte_sex);
		    }
		}

		/*
		 * If the -c option is specified the object's filetype will
		 * have been changed from MH_DYLIB to MH_DYLIB_STUB above.
		 */
		if(object->mh_filetype == MH_DYLIB ||
		   object->mh_filetype == MH_DYLIB_STUB){
		    object->output_tocs = new_tocs;
		    object->output_ntoc = new_ntoc;
#ifdef NMEDIT
		    if(object->mh != NULL)
			object->output_mods = new_mods;
		    else
			object->output_mods64 = new_mods64;
		    object->output_nmodtab = new_nmodtab;
#else
		    object->output_mods = mods;
		    object->output_nmodtab = nmodtab;
#endif
		    object->output_refs = new_refs;
		    object->output_nextrefsyms = new_nextrefsyms;
		    if(object->object_byte_sex != host_byte_sex){
			swap_dylib_table_of_contents(new_tocs, new_ntoc,
			    object->object_byte_sex);
#ifdef NMEDIT
			if(object->mh != NULL)
			    swap_dylib_module(new_mods, new_nmodtab,
				object->object_byte_sex);
			else
			    swap_dylib_module_64(new_mods64, new_nmodtab,
				object->object_byte_sex);
#else
			if(object->mh != NULL)
			    swap_dylib_module(mods, nmodtab,
				object->object_byte_sex);
			else
			    swap_dylib_module_64(mods64, nmodtab,
				object->object_byte_sex);
#endif
			swap_dylib_reference(new_refs, new_nextrefsyms,
			    object->object_byte_sex);
		    }
		}
		object->dyst->ntoc = new_ntoc;
		object->dyst->nextrefsyms = new_nextrefsyms;

		offset = get_starting_syminfo_offset(object);

		if(object->dyld_info != 0){
		    if (object->dyld_info->rebase_off != 0){
			object->dyld_info->rebase_off = offset;
			offset += object->dyld_info->rebase_size;
		    }
		    if (object->dyld_info->bind_off != 0){
			object->dyld_info->bind_off = offset;
			offset += object->dyld_info->bind_size;
		    }
		    if (object->dyld_info->weak_bind_off != 0){
			object->dyld_info->weak_bind_off = offset;
			offset += object->dyld_info->weak_bind_size;
		    }
		    if (object->dyld_info->lazy_bind_off != 0){
			object->dyld_info->lazy_bind_off = offset;
			offset += object->dyld_info->lazy_bind_size;
		    }
		    if (object->dyld_info->export_off != 0){
			object->dyld_info->export_off = offset;
			offset += object->dyld_info->export_size;
		    }
		}

		if(object->dyld_chained_fixups != NULL){
#ifndef NMEDIT
		    if(cflag){
			object->dyld_chained_fixups->dataoff = 0;
			object->dyld_chained_fixups->datasize = 0;
		    }
		    else
#endif /* !(NMEDIT) */
		    {
			object->dyld_chained_fixups->dataoff = offset;
			offset += object->dyld_chained_fixups->datasize;
		    }
		}

		if(object->dyld_exports_trie != NULL){
		    object->dyld_exports_trie->dataoff = offset;
		    offset += object->dyld_exports_trie->datasize;
		}
		
		if(object->dyst->nlocrel != 0){
		    object->output_loc_relocs = (struct relocation_info *)
			(object->object_addr + object->dyst->locreloff);
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the relocation info also gets
		     * stripped.
		     */
		    if(cflag){
			object->dyst->nlocrel = 0;
			object->dyst->locreloff = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->dyst->locreloff = offset;
			offset += object->dyst->nlocrel *
				  sizeof(struct relocation_info);
		    }
		}
		else
		    object->dyst->locreloff = 0;

		if(object->split_info_cmd != NULL){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the split info data gets
		     * stripped.
		     * And we it also gets stripped if the -no_split_info
		     * option is specified
		     */
		    if(cflag || no_split_info){
			object->split_info_cmd->dataoff = 0;
			object->split_info_cmd->datasize = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->split_info_cmd->dataoff = offset;
			offset += object->split_info_cmd->datasize;
		    }
		}

		if(object->func_starts_info_cmd != NULL){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the function starts info gets
		     * stripped.
		     */
		    if(cflag){
			object->func_starts_info_cmd->dataoff = 0;
			object->func_starts_info_cmd->datasize = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->func_starts_info_cmd->dataoff = offset;
			offset += object->func_starts_info_cmd->datasize;
		    }
		}

		if(object->data_in_code_cmd != NULL){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the data in code info gets
		     * stripped.
		     */
		    if(cflag){
			object->data_in_code_cmd->dataoff = 0;
			object->data_in_code_cmd->datasize = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->data_in_code_cmd->dataoff = offset;
			offset += object->data_in_code_cmd->datasize;
		    }
		}

		if(object->code_sign_drs_cmd != NULL){
		    object->code_sign_drs_cmd->dataoff = offset;
		    offset += object->code_sign_drs_cmd->datasize;
		}

		if(object->link_opt_hint_cmd != NULL){
		    object->link_opt_hint_cmd->dataoff = offset;
		    offset += object->link_opt_hint_cmd->datasize;
		}

		if(object->st->nsyms != 0){
		    object->st->symoff = offset;
		    if(object->mh != NULL)
			offset += object->st->nsyms * sizeof(struct nlist);
		    else
			offset += object->st->nsyms * sizeof(struct nlist_64);
		}
		else
		    /*
		     * This should be set to zero when nsyms is zero, but dyld
		     * will think it is malformed.  See rdar://34465083
		     */
		    object->st->symoff = offset;

		if(object->hints_cmd != NULL){
		    if(object->hints_cmd->nhints != 0){
			object->output_hints = (struct twolevel_hint *)
			    (object->object_addr + object->hints_cmd->offset);
			object->hints_cmd->offset = offset;
			offset += object->hints_cmd->nhints *
				  sizeof(struct twolevel_hint);
		    }
		    else
			object->hints_cmd->offset = 0;
		}

		if(object->dyst->nextrel != 0){
		    object->output_ext_relocs = (struct relocation_info *)
			(object->object_addr + object->dyst->extreloff);
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the relocation info also gets
		     * stripped.
		     */
		    if(cflag){
			object->dyst->nextrel = 0;
			object->dyst->extreloff = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->dyst->extreloff = offset;
			offset += object->dyst->nextrel *
			    sizeof(struct relocation_info);
		    }
		}
		else
		    object->dyst->extreloff = 0;

		if(object->dyst->nindirectsyms != 0){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub the indirect symbol table also gets
		     * stripped.
		     */
		    if(cflag){
			object->dyst->indirectsymoff = 0;
		    }
		    else
#endif /* defined(NMEDIT) */
		    {
			object->dyst->indirectsymoff = offset;
			offset += object->dyst->nindirectsyms *
				  sizeof(uint32_t) +
				  object->input_indirectsym_pad;
		    }
		}
		else
		    object->dyst->indirectsymoff = 0;;

		if(object->dyst->ntoc != 0){
		    object->dyst->tocoff = offset;
		    offset += object->dyst->ntoc *
			      sizeof(struct dylib_table_of_contents);
		}
		else
		    object->dyst->tocoff = 0;

		if(object->dyst->nmodtab != 0){
#ifndef NMEDIT
		    /*
		     * When stripping out the section contents to create a
		     * dynamic library stub zero out the fields in the module
		     * table for the sections and relocation information and
		     * clear Objective-C address and size from modules.
		     */
		    if(cflag){
			if(object->mh != NULL){
			    for(k = 0; k < object->dyst->nmodtab; k++){
				mods[k].iinit_iterm = 0;
				mods[k].ninit_nterm = 0;
				mods[k].iextrel = 0;
				mods[k].nextrel = 0;
				mods[k].objc_module_info_addr = 0;
				mods[k].objc_module_info_size = 0;
			    }
			}
			else{
			    for(k = 0; k < object->dyst->nmodtab; k++){
				mods64[k].iinit_iterm = 0;
				mods64[k].ninit_nterm = 0;
				mods64[k].iextrel = 0;
				mods64[k].nextrel = 0;
				mods64[k].objc_module_info_addr = 0;
				mods64[k].objc_module_info_size = 0;
			    }
			}
		    }
#endif /* !(NMEDIT) */
		    object->dyst->modtaboff = offset;
		    if(object->mh != NULL)
			offset += object->dyst->nmodtab *
				  sizeof(struct dylib_module);
		    else
			offset += object->dyst->nmodtab *
				  sizeof(struct dylib_module_64);
		}
		else
		    object->dyst->modtaboff = 0;

		if(object->dyst->nextrefsyms != 0){
		    object->dyst->extrefsymoff = offset;
		    offset += object->dyst->nextrefsyms *
			      sizeof(struct dylib_reference);
		}
		else
		    object->dyst->extrefsymoff = 0;

		if(object->st->strsize != 0){
		    object->st->stroff = offset;
		    offset += object->st->strsize;
		}
		else
		    /*
		     * This should be set to zero when strsize is zero, but some
		     * tools will think it is malformed, like machocheck.  See
		     * rdar://34729011
		     */
		    object->st->stroff = offset;

		if(object->code_sig_cmd != NULL){
		    offset = rnd32(offset, 16);
		    object->code_sig_cmd->dataoff = offset;
		    offset += object->code_sig_cmd->datasize;
		}
	    }
	    else{
		if(new_strsize != 0){
		    if(object->mh != NULL)
			object->st->stroff = object->st->symoff +
					 new_nsyms * sizeof(struct nlist);
		    else
			object->st->stroff = object->st->symoff +
					 new_nsyms * sizeof(struct nlist_64);
		}
		else
		    object->st->stroff = 0;
		if(new_nsyms == 0)
		    object->st->symoff = 0;
	    }
	}
#ifndef NMEDIT
	else{
	    /*
	     * Here we are doing a full symbol strip.  In some cases it may
	     * leave the local relocation entries as well as LOCAL indirect
	     * symbol table entries.
	     */
	    if(saves != NULL)
		free(saves);
	    saves = (int32_t *)allocate(object->st->nsyms * sizeof(int32_t));
	    bzero(saves, object->st->nsyms * sizeof(int32_t));

	    /*
	     * Account for the symbolic info in the input file.
	     */
	    if(object->dyst != NULL){
		object->input_sym_info_size +=
		    object->dyst->nlocrel * sizeof(struct relocation_info) +
		    object->dyst->nextrel * sizeof(struct relocation_info) +
		    object->dyst->ntoc * sizeof(struct dylib_table_of_contents)+
		    object->dyst->nextrefsyms * sizeof(struct dylib_reference);
		if(object->mh != NULL){
		    object->input_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module) +
			object->dyst->nindirectsyms * sizeof(uint32_t);
		}
		else{
		    object->input_sym_info_size +=
			object->dyst->nmodtab * sizeof(struct dylib_module_64) +
			object->dyst->nindirectsyms * sizeof(uint32_t) +
			object->input_indirectsym_pad;
		}
	    }

	    /*
	     * Determine the offset where the remaining symbolic info will start
	     * in the output file (if any).
	     */
	    offset = get_starting_syminfo_offset(object);

	    /*
	     * For a full symbol strip all these values in the output file are
	     * set to zero.
	     */
	    object->st->symoff = 0;
	    object->st->nsyms = 0;
	    object->st->stroff = 0;
	    object->st->strsize = 0;
	    if(object->dyst != NULL){
		object->dyst->ilocalsym = 0;
		object->dyst->nlocalsym = 0;
		object->dyst->iextdefsym = 0;
		object->dyst->nextdefsym = 0;
		object->dyst->iundefsym = 0;
		object->dyst->nundefsym = 0;
	    }

	    /*
	     * This will accumulate any remaining symbolic info size in the
	     * output file.
	     */
	    object->output_sym_info_size = 0;

	    /*
	     * We set these so that checking can be done below to report the
	     * symbols that can't be stripped because of relocation entries
	     * or indirect symbol table entries.  Normally if these table have a
	     * non-zero number of entries it will be an error as we are trying
	     * to strip everything.  But it maybe that there are only LOCAL
	     * indirect entries which is odd but will be OK.
	     */
	    if(object->dyst != NULL){
		if(object->dyst->nextrel != 0){
		    object->output_ext_relocs = (struct relocation_info *)
			(object->object_addr + object->dyst->extreloff);
		}
		/*
		 * Since this file has a dynamic symbol table and if this file
		 * has local relocation entries on input make sure they are
		 * there on output.  This is a rare case that it will not have
		 * external relocs or indirect symbols but can happen as is the
		 * case with the dynamic linker itself.
		 */
		if(object->dyst->nlocrel != 0){
		    object->output_loc_relocs = (struct relocation_info *)
			(object->object_addr + object->dyst->locreloff);
		    object->output_sym_info_size +=
			object->dyst->nlocrel * sizeof(struct relocation_info);

		    object->dyst->locreloff = offset;
		    offset += object->dyst->nlocrel *
			      sizeof(struct relocation_info);
		}

		if(object->dyst->nindirectsyms != 0){
		    object->output_indirect_symtab = (uint32_t *)
			(object->object_addr +
			 object->dyst->indirectsymoff);
		    if(object->object_byte_sex != host_byte_sex)
			swap_indirect_symbols(
			    object->output_indirect_symtab,
			    object->dyst->nindirectsyms,
			    object->object_byte_sex);

		    object->output_sym_info_size +=
		    	object->dyst->nindirectsyms * sizeof(uint32_t) +
			object->input_indirectsym_pad;

		    object->dyst->indirectsymoff = offset;
		    offset += object->dyst->nindirectsyms * sizeof(uint32_t) +
			      object->input_indirectsym_pad;
		}
	    }
	    if(no_uuid == TRUE)
		strip_LC_UUID_commands(arch, member, object);
	    if(no_split_info == TRUE)
		strip_LC_SEGMENT_SPLIT_INFO_command(arch, member, object);
	}
#endif /* !defined(NMEDIT) */

	/*
	 * Always clear the prebind checksum if any when creating a new file.
	 */
	if(object->cs != NULL)
	    object->cs->cksum = 0;

	if(object->seg_linkedit != NULL){
	    object->seg_linkedit->filesize += object->output_sym_info_size -
					      object->input_sym_info_size;
	    object->seg_linkedit->vmsize = object->seg_linkedit->filesize;
	}
	else if(object->seg_linkedit64 != NULL){
	    /* Do this in two steps to avoid 32/64-bit casting problems. */
	    object->seg_linkedit64->filesize -= object->input_sym_info_size;
	    object->seg_linkedit64->filesize += object->output_sym_info_size;
	    object->seg_linkedit64->vmsize = object->seg_linkedit64->filesize;
	}

	/*
	 * Check and update the external relocation entries to make sure
	 * referenced symbols are not stripped and refer to the new symbol
	 * table indexes.
	 * 
	 * The external relocation entries can be located in one of two places,
	 * first off of the sections or second off of the dynamic symtab.
	 */
	missing_reloc_symbols = 0;
	lc = object->load_commands;
	if(object->mh != NULL)
	    ncmds = object->mh->ncmds;
	else
	    ncmds = object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT &&
	       object->seg_linkedit != (struct segment_command *)lc){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)sg +
					sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    if(s->nreloc != 0){
			if(s->reloff + s->nreloc *
			   sizeof(struct relocation_info) >
						object->object_size){
			    fatal_arch(arch, member, "truncated or malformed "
				"object (relocation entries for section (%.16s,"
				"%.16s) extends past the end of the file)",
				s->segname, s->sectname);
			}
			relocs = (struct relocation_info *)
					(object->object_addr + s->reloff);
			if(object->object_byte_sex != host_byte_sex)
			    swap_relocation_info(relocs, s->nreloc,
						 host_byte_sex);
			if(s->offset + s->size > object->object_size){
			    fatal_arch(arch, member, "truncated or malformed "
				"object (contents of section (%.16s,"
				"%.16s) extends past the end of the file)",
				s->segname, s->sectname);
			}
			contents = object->object_addr + s->offset;
			check_object_relocs(arch, member, object, s->segname,
    			    s->sectname, s->size, contents, relocs, s->nreloc,
			    symbols, symbols64, nsyms, strings,
			    &missing_reloc_symbols, host_byte_sex);
			if(object->object_byte_sex != host_byte_sex)
			    swap_relocation_info(relocs, s->nreloc,
						 object->object_byte_sex);
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64 &&
	       object->seg_linkedit64 != (struct segment_command_64 *)lc){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)sg64 +
					sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    if(s64->nreloc != 0){
			if(s64->reloff + s64->nreloc *
			   sizeof(struct relocation_info) >
						object->object_size){
			    fatal_arch(arch, member, "truncated or malformed "
				"object (relocation entries for section (%.16s,"
				"%.16s) extends past the end of the file)",
				s64->segname, s64->sectname);
			}
			relocs = (struct relocation_info *)
					(object->object_addr + s64->reloff);
			if(object->object_byte_sex != host_byte_sex)
			    swap_relocation_info(relocs, s64->nreloc,
						 host_byte_sex);
			if(s64->offset + s64->size > object->object_size){
			    fatal_arch(arch, member, "truncated or malformed "
				"object (contents of section (%.16s,"
				"%.16s) extends past the end of the file)",
				s64->segname, s64->sectname);
			}
			contents = object->object_addr + s64->offset;
			check_object_relocs(arch, member, object, s64->segname,
    			    s64->sectname, s64->size, contents, relocs,
			    s64->nreloc, symbols, symbols64, nsyms, strings,
			    &missing_reloc_symbols, host_byte_sex);
			if(object->object_byte_sex != host_byte_sex)
			    swap_relocation_info(relocs, s64->nreloc,
						 object->object_byte_sex);
		    }
		    s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(object->dyst != NULL && object->dyst->nextrel != 0){
	    relocs = object->output_ext_relocs;
	    if(object->object_byte_sex != host_byte_sex)
		swap_relocation_info(relocs, object->dyst->nextrel,
				     host_byte_sex);

	    for(i = 0; i < object->dyst->nextrel; i++){
		if((relocs[i].r_address & R_SCATTERED) == 0 &&
		   relocs[i].r_extern == 1){
		    if(relocs[i].r_symbolnum > nsyms){
			fatal_arch(arch, member, "bad r_symbolnum for external "
			    "relocation entry %d in: ", i);
		    }
		    if(saves[relocs[i].r_symbolnum] == 0){
			if(missing_reloc_symbols == 0){
			    error_arch(arch, member, "error: symbols "
			      "referenced by relocation entries that can't be "
			      "stripped in: ");
			    missing_reloc_symbols = 1;
			}
			if(object->mh != NULL){
			    fprintf(stderr, "%s\n", strings + symbols
			            [relocs[i].r_symbolnum].n_un.n_strx);
			}
			else {
			    fprintf(stderr, "%s\n", strings + symbols64
			            [relocs[i].r_symbolnum].n_un.n_strx);
			}
			saves[relocs[i].r_symbolnum] = -1;
		    }
		    if(saves[relocs[i].r_symbolnum] != -1){
			relocs[i].r_symbolnum =
			    saves[relocs[i].r_symbolnum] - 1;
		    }
		}
		else{
		    fatal_arch(arch, member, "bad external relocation entry "
			"%d (not external) in: ", i);
		}
		if((relocs[i].r_address & R_SCATTERED) == 0){
		    if(reloc_has_pair(object->mh_cputype, relocs[i].r_type))
			i++;
		}
		else{
		    sreloc = (struct scattered_relocation_info *)relocs + i;
		    if(reloc_has_pair(object->mh_cputype, sreloc->r_type))
			i++;
		}
	    }
	    if(object->object_byte_sex != host_byte_sex)
		swap_relocation_info(relocs, object->dyst->nextrel,
				     object->object_byte_sex);
	}

	/*
	 * Check and update the indirect symbol table entries to make sure
	 * referenced symbols are not stripped and refer to the new symbol
	 * table indexes.
	 */
	if(object->dyst != NULL && object->dyst->nindirectsyms != 0){
	    if(object->object_byte_sex != host_byte_sex)
		swap_indirect_symbols(object->output_indirect_symtab,
		    object->dyst->nindirectsyms, host_byte_sex);

	    lc = object->load_commands;
	    if(object->mh != NULL)
		ncmds = object->mh->ncmds;
	    else
		ncmds = object->mh64->ncmds;
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_SEGMENT &&
		   object->seg_linkedit != (struct segment_command *)lc){
		    sg = (struct segment_command *)lc;
		    s = (struct section *)((char *)sg +
					    sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++){
			section_type = s->flags & SECTION_TYPE;
			if(section_type == S_LAZY_SYMBOL_POINTERS ||
			   section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
			   section_type == S_NON_LAZY_SYMBOL_POINTERS)
			  stride = 4;
			else if(section_type == S_SYMBOL_STUBS)
			    stride = s->reserved2;
			else{
			    s++;
			    continue;
			}
			nitems = s->size / stride;
			contents = object->object_addr + s->offset;
			check_indirect_symtab(arch, member, object, nitems,
			    s->reserved1, section_type, contents, symbols,
			    symbols64, nsyms, strings, &missing_reloc_symbols,
			    swift_version, host_byte_sex);
			s++;
		    }
		}
		else if(lc->cmd == LC_SEGMENT_64 &&
		   object->seg_linkedit64 != (struct segment_command_64 *)lc){
		    sg64 = (struct segment_command_64 *)lc;
		    s64 = (struct section_64 *)((char *)sg64 +
					    sizeof(struct segment_command_64));
		    for(j = 0; j < sg64->nsects; j++){
			section_type = s64->flags & SECTION_TYPE;
			if(section_type == S_LAZY_SYMBOL_POINTERS ||
			   section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
			   section_type == S_NON_LAZY_SYMBOL_POINTERS)
			  stride = 8;
			else if(section_type == S_SYMBOL_STUBS)
			    stride = s64->reserved2;
			else{
			    s64++;
			    continue;
			}
			nitems = (uint32_t)(s64->size / stride);
			contents = object->object_addr + s64->offset;
			check_indirect_symtab(arch, member, object, nitems,
			    s64->reserved1, section_type, contents, symbols,
			    symbols64, nsyms, strings, &missing_reloc_symbols,
			    swift_version, host_byte_sex);
			s64++;
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }

	    if(object->object_byte_sex != host_byte_sex)
		swap_indirect_symbols(object->output_indirect_symtab,
		    object->dyst->nindirectsyms, object->object_byte_sex);
	}

	/*
	 * Issue a warning if object file has a code signature that the
	 * operation will invalidate it.
	 */
	if(object->code_sig_cmd != NULL
#ifndef NMEDIT
 	   && !no_code_signature_warning
#endif /* !(NMEDIT) */
	  )
	    warning_arch(arch, member, "changes being made to the file will "
		"invalidate the code signature in: ");

	if(nlist_outofsync_with_dyldinfo == TRUE){
	    if(object->mh != NULL)
		object->mh->flags |= MH_NLIST_OUTOFSYNC_WITH_DYLDINFO;
	    else
		object->mh64->flags |= MH_NLIST_OUTOFSYNC_WITH_DYLDINFO;
	}
}

/*
 * get_starting_syminfo_offset() returns the starting offset of the symbolic
 * info in the object file.
 */
static
uint32_t
get_starting_syminfo_offset(
struct object *object)
{
    uint32_t offset;

	if(object->seg_linkedit != NULL ||
	   object->seg_linkedit64 != NULL){
	    if(object->mh != NULL)
		offset = object->seg_linkedit->fileoff;
	    else
		offset = (uint32_t)object->seg_linkedit64->fileoff;
	}
	else{
	    offset = UINT_MAX;
	    if(object->dyst != NULL &&
	       object->dyst->nlocrel != 0 &&
	       object->dyst->locreloff < offset)
		offset = object->dyst->locreloff;
	    if(object->func_starts_info_cmd != NULL &&
	       object->func_starts_info_cmd->datasize != 0 &&
	       object->func_starts_info_cmd->dataoff < offset)
	        offset = object->func_starts_info_cmd->dataoff;
	    if(object->data_in_code_cmd != NULL &&
	       object->data_in_code_cmd->datasize != 0 &&
	       object->data_in_code_cmd->dataoff < offset)
	        offset = object->data_in_code_cmd->dataoff;
	    if(object->link_opt_hint_cmd != NULL &&
	       object->link_opt_hint_cmd->datasize != 0 &&
	       object->link_opt_hint_cmd->dataoff < offset)
	        offset = object->link_opt_hint_cmd->dataoff;
	    if(object->st->nsyms != 0 &&
	       object->st->symoff < offset)
		offset = object->st->symoff;
	    if(object->dyst != NULL &&
	       object->dyst->nextrel != 0 &&
	       object->dyst->extreloff < offset)
		offset = object->dyst->extreloff;
	    if(object->dyst != NULL &&
	       object->dyst->nindirectsyms != 0 &&
	       object->dyst->indirectsymoff < offset)
		offset = object->dyst->indirectsymoff;
	    if(object->dyst != NULL &&
	       object->dyst->ntoc != 0 &&
	       object->dyst->tocoff < offset)
		offset = object->dyst->tocoff;
	    if(object->dyst != NULL &&
	       object->dyst->nmodtab != 0 &&
	       object->dyst->modtaboff < offset)
		offset = object->dyst->modtaboff;
	    if(object->dyst != NULL &&
	       object->dyst->nextrefsyms != 0 &&
	       object->dyst->extrefsymoff < offset)
		offset = object->dyst->extrefsymoff;
	    if(object->st->strsize != 0 &&
	       object->st->stroff < offset)
		offset = object->st->stroff;
	} 
	return(offset);
}

/*
 * check_object_relocs() is used to check and update the external relocation
 * entries from a section in an object file, to make sure referenced symbols
 * are not stripped and are changed to refer to the new symbol table indexes.
 */
static
void
check_object_relocs(
struct arch *arch,
struct member *member,
struct object *object,
char *segname,
char *sectname,
uint64_t sectsize,
char *contents,
struct relocation_info *relocs,
uint32_t nreloc,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsyms,
char *strings,
int32_t *missing_reloc_symbols,
enum byte_sex host_byte_sex)
{
    uint32_t k, n_strx;
    uint64_t n_value;
#ifdef NMEDIT
    uint32_t value, n_ext;
    uint64_t value64; 
#endif
    struct scattered_relocation_info *sreloc;

	for(k = 0; k < nreloc; k++){
	    if((relocs[k].r_address & R_SCATTERED) == 0 &&
	       relocs[k].r_extern == 1){
		if(relocs[k].r_symbolnum > nsyms){
		    fatal_arch(arch, member, "bad r_symbolnum for relocation "
			"entry %d in section (%.16s,%.16s) in: ", k, segname,
			sectname);
		}
		if(object->mh != NULL){
		    n_strx = symbols[relocs[k].r_symbolnum].n_un.n_strx;
		    n_value = symbols[relocs[k].r_symbolnum].n_value;
		}
		else{
		    n_strx = symbols64[relocs[k].r_symbolnum].n_un.n_strx;
		    n_value = symbols64[relocs[k].r_symbolnum].n_value;
		}
#ifndef NMEDIT
		if(saves[relocs[k].r_symbolnum] == 0){
		    if(*missing_reloc_symbols == 0){
			error_arch(arch, member, "error: symbols referenced by "
			    "relocation entries that can't be stripped in: ");
			*missing_reloc_symbols = 1;
		    }
		    fprintf(stderr, "%s\n", strings + n_strx);
		    saves[relocs[k].r_symbolnum] = -1;
		}
#else /* defined(NMEDIT) */
		/*
		 * We are letting nmedit change global coalesed symbols into
		 * statics in MH_OBJECT file types only. Relocation entries to
		 * global coalesced symbols are external relocs.
		 */
		if(object->mh != NULL)
		    n_ext = new_symbols[saves[relocs[k].r_symbolnum] - 1].
				n_type & N_EXT;
		else
		    n_ext = new_symbols64[saves[relocs[k].r_symbolnum] - 1].
				n_type & N_EXT;
		if(n_ext != N_EXT &&
		   object->mh_cputype != CPU_TYPE_X86_64){
		    /*
		     * We need to do the relocation for this external relocation
		     * entry so the item to be relocated is correct for a local
		     * relocation entry. We don't need to do this for x86-64.
		     */
		    if(relocs[k].r_address + sizeof(int32_t) > sectsize){
			fatal_arch(arch, member, "truncated or malformed "
			    "object (r_address of relocation entry %u of "
			    "section (%.16s,%.16s) extends past the end "
			    "of the section)", k, segname, sectname);
		    }
		    if(object->mh != NULL){
			value = *(uint32_t *)
				 (contents + relocs[k].r_address);
			if(object->object_byte_sex != host_byte_sex)
			    value = SWAP_INT(value);
			/*
			 * We handle a very limited form here.  Only VANILLA
			 * (r_type == 0) long (r_length==2) absolute or pcrel
			 * that won't need a scattered relocation entry.
			 */
			if(relocs[k].r_type != 0 ||
			   relocs[k].r_length != 2){
			    fatal_arch(arch, member, "don't have "
			      "code to convert external relocation "
			      "entry %d in section (%.16s,%.16s) "
			      "for global coalesced symbol: %s "
			      "in: ", k, segname, sectname,
			      strings + n_strx);
			}
			value += n_value;
			if(object->object_byte_sex != host_byte_sex)
			    value = SWAP_INT(value);
			*(uint32_t *)(contents + relocs[k].r_address) =
			    value;
		    }
		    else{
			value64 = *(uint64_t *)(contents + relocs[k].r_address);
			if(object->object_byte_sex != host_byte_sex)
			    value64 = SWAP_LONG_LONG(value64);
			/*
			 * We handle a very limited form here.  Only VANILLA
			 * (r_type == 0) quad (r_length==3) absolute or pcrel
			 * that won't need a scattered relocation entry.
			 */
			if(relocs[k].r_type != 0 ||
			   relocs[k].r_length != 3){
			    fatal_arch(arch, member, "don't have "
			      "code to convert external relocation "
			      "entry %d in section (%.16s,%.16s) "
			      "for global coalesced symbol: %s "
			      "in: ", k, segname, sectname,
			      strings + n_strx);
			}
			value64 += n_value;
			if(object->object_byte_sex != host_byte_sex)
			    value64 = SWAP_LONG_LONG(value64);
			*(uint64_t *)(contents + relocs[k].r_address) = value64;
		    }
		    /*
		     * Turn the extern reloc into a local.
		     */
		    if(object->mh != NULL)
			relocs[k].r_symbolnum =
			 new_symbols[saves[relocs[k].r_symbolnum] - 1].n_sect;
		    else
			relocs[k].r_symbolnum =
			 new_symbols64[saves[relocs[k].r_symbolnum] - 1].n_sect;
		    relocs[k].r_extern = 0;
		}
#endif /* NMEDIT */
		if(relocs[k].r_extern == 1 &&
		   saves[relocs[k].r_symbolnum] != -1){
		    relocs[k].r_symbolnum = saves[relocs[k].r_symbolnum] - 1;
		}
	    }
	    if((relocs[k].r_address & R_SCATTERED) == 0){
		if(reloc_has_pair(object->mh_cputype, relocs[k].r_type) == TRUE)
		    k++;
	    }
	    else{
		sreloc = (struct scattered_relocation_info *)relocs + k;
		if(reloc_has_pair(object->mh_cputype, sreloc->r_type) == TRUE)
		    k++;
	    }
	}
}

/*
 * check_indirect_symtab() checks and updates the indirect symbol table entries
 * to make sure referenced symbols are not stripped and refer to the new symbol
 * table indexes.
 */
static
void
check_indirect_symtab(
struct arch *arch,
struct member *member,
struct object *object,
uint32_t nitems,
uint32_t reserved1,
uint32_t section_type,
char *contents,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsyms,
char *strings,
int32_t *missing_reloc_symbols,
uint32_t swift_version,
enum byte_sex host_byte_sex)
{
    uint32_t k, index;
    uint8_t n_type;
    uint32_t n_strx, value;
    uint64_t value64;
    enum bool made_local;
    uint32_t mh_flags, mh_filetype;

	if(object->mh != NULL) {
	    mh_flags = object->mh->flags;
	    mh_filetype = object->mh->filetype;
	} else {
	    mh_flags = object->mh64->flags;
	    mh_filetype = object->mh64->filetype;
	}

	for(k = 0; k < nitems; k++){
	    made_local = FALSE;
	    index = object->output_indirect_symtab[reserved1 + k];
	    if(index == INDIRECT_SYMBOL_LOCAL ||
	       index == INDIRECT_SYMBOL_ABS ||
	       index == (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS))
		continue;
	    if(index > nsyms)
		fatal_arch(arch, member,"indirect symbol table entry %d (past "
			   "the end of the symbol table) in: ", reserved1 + k);
#ifdef NMEDIT
	    if(pflag == 0 && nmedits[index] == TRUE && saves[index] != -1)
#else
	    if(saves[index] == 0)
#endif
	    {
		/*
		 * Indirect symbol table entries for defined symbols in a
		 * non-lazy pointer section that are not saved are changed to
		 * INDIRECT_SYMBOL_LOCAL which their values just have to be
		 * slid if the are not absolute symbols.
		 */
		if(object->mh != NULL){
		    n_type = symbols[index].n_type;
		    n_strx = symbols[index].n_un.n_strx;
		}
		else{
		    n_type = symbols64[index].n_type;
		    n_strx = symbols64[index].n_un.n_strx;
		}
		if((n_type & N_TYPE) != N_UNDF &&
		   (n_type & N_TYPE) != N_PBUD &&
		   section_type == S_NON_LAZY_SYMBOL_POINTERS){
		    object->output_indirect_symtab[reserved1 + k] =
			    INDIRECT_SYMBOL_LOCAL;
		    if((n_type & N_TYPE) == N_ABS)
			object->output_indirect_symtab[reserved1 + k] |=
				INDIRECT_SYMBOL_ABS;
		    made_local = TRUE;
		    /*
		     * When creating a stub shared library the section contents
		     * are not updated since they will be stripped.
		     */
		    if(object->mh_filetype != MH_DYLIB_STUB){
			if(object->mh != NULL){
			    value = symbols[index].n_value;
			    if (symbols[index].n_desc & N_ARM_THUMB_DEF)
				value |= 1;
			    if(object->object_byte_sex != host_byte_sex)
				value = SWAP_INT(value);
			    *(uint32_t *)(contents + k * 4) = value;
			}
			else{
			    value64 = symbols64[index].n_value;
				if(object->object_byte_sex != host_byte_sex)
				value64 = SWAP_LONG_LONG(value64);
			    *(uint64_t *)(contents + k * 8) = value64;
			}
		    }
		}
#ifdef NMEDIT
		else {
		    object->output_indirect_symtab[reserved1 + k] =
			saves[index] - 1;
		}
#else /* !defined(NMEDIT) */
		else if(strip_all_nlists ||
			(Tflag && swift_version != 0 &&
			(mh_flags & MH_DYLDLINK) == MH_DYLDLINK &&
	                n_strx != 0 &&
			(strncmp(strings + n_strx, "_$S", 3) == 0 ||
			 strncmp(strings + n_strx, "_$s", 3) == 0))){
		    object->output_indirect_symtab[reserved1 + k] =
			    INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS;
		    made_local = TRUE;
		}
		else{
		    if(*missing_reloc_symbols == 0){
			error_arch(arch, member, "error: symbols referenced by "
			    "indirect symbol table entries that can't be "
			    "stripped in: ");
			*missing_reloc_symbols = 1;
		    }
		    fprintf(stderr, "%s\n", strings + n_strx);
		    saves[index] = -1;
		}
#endif /* !defined(NMEDIT) */
	    }
#ifdef NMEDIT
	    else
#else /* !defined(NMEDIT) */
	    if(made_local == FALSE && saves[index] != -1)
#endif /* !defined(NMEDIT) */
	    {
		object->output_indirect_symtab[reserved1+k] = saves[index] - 1;
	    }
	}
}

#ifndef NMEDIT
/*
 * This is called if there is a -d option specified.  It reads the file with
 * the strings in it and places them in the array debug_filenames and sorts
 * them by name.  The file that contains the file names must have names one
 * per line with no white space (except the newlines).
 */
static
void
setup_debug_filenames(
char *dfile)
{
    int fd, i;
    off_t strings_size;
    struct stat stat_buf;
    char *strings, *p;

	if((fd = open(dfile, O_RDONLY)) < 0){
	    system_error("can't open: %s", dfile);
	    return;
	}
	if(fstat(fd, &stat_buf) == -1){
	    system_error("can't stat: %s", dfile);
	    close(fd);
	    return;
	}
	strings_size = stat_buf.st_size;
	strings = (char *)allocate(strings_size + 1);
	strings[strings_size] = '\0';
	if(read(fd, strings, strings_size) != strings_size){
	    system_error("can't read: %s", dfile);
	    close(fd);
	    return;
	}
	p = strings;
	for(i = 0; i < strings_size; i++){
	    if(*p == '\n'){
		*p = '\0';
		ndebug_filenames++;
	    }
	    p++;
	}
	debug_filenames = (char **)allocate(ndebug_filenames * sizeof(char *));
	p = strings;
	for(i = 0; i < ndebug_filenames; i++){
	    debug_filenames[i] = p;
	    p += strlen(p) + 1;
	}
	qsort(debug_filenames, ndebug_filenames, sizeof(char *),
	      (int (*)(const void *, const void *))cmp_qsort_filename);

#ifdef DEBUG
	printf("Debug filenames:\n");
	for(i = 0; i < ndebug_filenames; i++){
	    printf("filename = %s\n", debug_filenames[i]);
	}
#endif /* DEBUG */
}

/*
 * Hack needed to dig out the swift_version from (flags >> 8) & 0xff to see if
 * non-zero to apply the -T hack for trying to removing only swift symbols that
 * start with "_$S" and not other symbols starting with "_$S".
 */
struct objc_image_info {
    uint32_t version;
    uint32_t flags;
};

static
void
swap_objc_image_info(
struct objc_image_info *o,
enum byte_sex target_byte_sex)
{
        o->version = SWAP_INT(o->version);
        o->flags = SWAP_INT(o->flags);
}

/*
 * Strip the symbol table to the level specified by the command line arguments.
 * The new symbol table is built and new_symbols is left pointing to it.  The
 * number of new symbols is left in new_nsyms, the new string table is built
 * and new_stings is left pointing to it and new_strsize is left containing it.
 * This routine returns zero if successfull and non-zero otherwise.
 */
static
enum bool
strip_symtab(
struct arch *arch,
struct member *member,
struct object *object,
struct dylib_table_of_contents *tocs,
uint32_t ntoc,
struct dylib_module *mods,
struct dylib_module_64 *mods64,
uint32_t nmodtab,
struct dylib_reference *refs,
uint32_t nextrefsyms,
uint32_t *p_swift_version,
enum bool *nlist_outofsync_with_dyldinfo)
{
    uint32_t i, j, k, n, inew_syms, save_debug, missing_syms;
    uint32_t missing_symbols;
    char *p, *q, **pp, *basename;
    struct symbol_list *sp;
    uint32_t new_ext_strsize, *changes, inew_undefsyms;
    long len;
    unsigned char nsects;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s, **sections;
    struct section_64 *s64, **sections64;
    uint32_t ncmds, mh_flags, mh_filetype, s_flags, n_strx;
    struct nlist *sym;
    struct undef_map *undef_map;
    struct undef_map64 *undef_map64;
    uint8_t n_type, n_sect;
    uint16_t n_desc;
    uint64_t n_value;
    uint32_t module_name, iextdefsym, nextdefsym, ilocalsym, nlocalsym;
    uint32_t irefsym, nrefsym;
    enum bool has_dwarf, hack_5614542;
    uint32_t swift_version;
    char *p_objc_image_info;
    struct objc_image_info o;
    struct strx_map* strx_map;
    uint32_t strx_count;
    uint32_t strx_uniqcount;

	*nlist_outofsync_with_dyldinfo = FALSE;
	save_debug = 0;
	if(saves != NULL)
	    free(saves);
	changes = NULL;
	for(i = 0; i < nsave_symbols; i++)
	    save_symbols[i].sym = NULL;
	for(i = 0; i < nremove_symbols; i++)
	    remove_symbols[i].sym = NULL;
	if(member == NULL){
	    for(i = 0; i < nsave_symbols; i++)
		save_symbols[i].seen = FALSE;
	    for(i = 0; i < nremove_symbols; i++)
		remove_symbols[i].seen = FALSE;
	}

	new_nsyms = 0;
	new_strsize = sizeof(int32_t);
	new_nlocalsym = 0;
	new_nextdefsym = 0;
	new_nundefsym = 0;
	new_ext_strsize = 0;
        strx_map = NULL;
        strx_count = 0;
        strx_uniqcount = 0;
    
	/*
	 * If this an object file that has DWARF debugging sections to strip
	 * then we have to run ld -r on it.  We also have to do this for
	 * ARM objects because thumb symbols can't be stripped as they are
	 * needed for proper linking in .o files.  And we need to for i386
	 * objects to not mess up compact unwind info.
	 */
	if(object->mh_filetype == MH_OBJECT && (Sflag || xflag)){
	    has_dwarf = FALSE;
	    lc = object->load_commands;
	    if(object->mh != NULL)
		ncmds = object->mh->ncmds;
	    else
		ncmds = object->mh64->ncmds;
	    for(i = 0; i < ncmds && has_dwarf == FALSE; i++){
		if(lc->cmd == LC_SEGMENT){
		    sg = (struct segment_command *)lc;
		    s = (struct section *)((char *)sg +
					    sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++){
			if(s->flags & S_ATTR_DEBUG){
			    has_dwarf = TRUE;
			    break;
			}
			s++;
		    }
		}
		else if(lc->cmd == LC_SEGMENT_64){
		    sg64 = (struct segment_command_64 *)lc;
		    s64 = (struct section_64 *)((char *)sg64 +
					    sizeof(struct segment_command_64));
		    for(j = 0; j < sg64->nsects; j++){
			if(s64->flags & S_ATTR_DEBUG){
			    has_dwarf = TRUE;
			    break;
			}
			s64++;
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	    /*
	     * If the file has dwarf symbols or is an ARM or i386 object then
	     * have ld(1) do the "stripping" and make an ld -r version of the
	     * object.
	     */
	    if(has_dwarf == TRUE ||
	       object->mh_cputype == CPU_TYPE_ARM ||
	       object->mh_cputype == CPU_TYPE_I386)
		make_ld_r_object(arch, member, object);
	}
	/*
	 * Because of the "design" of 64-bit object files and the lack of
	 * local relocation entries it is not possible for strip(1) to do its
	 * job without becoming a static link editor.  The "design" does not
	 * actually strip the symbols it simply renames them to things like
	 * "l1000".  And they become static symbols but still have external
	 * relocation entries.  Thus can never actually be stripped.  Also some
	 * symbols, *.eh, symbols are not even changed to these names if there
	 * corresponding global symbol is not stripped.  So strip(1) only
	 * recourse is to use the unified linker to create an ld -r object then
	 * save all resulting symbols (both static and global) and hope the user
	 * does not notice the stripping is not what they asked for.
	 */
	if(object->mh_filetype == MH_OBJECT &&
	   (object->mh64 != NULL && object->ld_r_ofile == NULL))
	    make_ld_r_object(arch, member, object);

	/*
	 * Since make_ld_r_object() may create an object with more symbols
	 * this has to be done after make_ld_r_object() and nsyms is updated.
	 */
	saves = (int32_t *)allocate(nsyms * sizeof(int32_t));
	bzero(saves, nsyms * sizeof(int32_t));
    
        /*
         * Allocate space for the strx_map. This table will be used unique
         * local symbol strings, reclaiming space from the file.
         */
        strx_map = calloc(nsyms, sizeof(*strx_map));

	/*
	 * Gather an array of section struct pointers so we can later determine
	 * if we run into a global symbol in a coalesced section and not strip
	 * those symbols.
	 * statics.
	 */
	nsects = 0;
	lc = object->load_commands;
	if(object->mh != NULL)
	    ncmds = object->mh->ncmds;
	else
	    ncmds = object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		nsects += sg->nsects;
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		nsects += sg64->nsects;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(object->mh != NULL){
	    sections = allocate(nsects * sizeof(struct section *));
	    sections64 = NULL;
	}
	else{
	    sections = NULL;
	    sections64 = allocate(nsects * sizeof(struct section_64 *));
	}
	nsects = 0;
	lc = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)sg +
					sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++)
		    sections[nsects++] = s++;
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)sg64 +
					sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++)
		    sections64[nsects++] = s64++;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	if(object->mh != NULL) {
	    mh_flags = object->mh->flags;
	    mh_filetype = object->mh->filetype;
	} else {
	    mh_flags = object->mh64->flags;
	    mh_filetype = object->mh64->filetype;
	}

	/*
	 * Get the swift version if any if we are using the -T flag and this is
	 * an linked image for dyld.  This is a hack to try to lessen the
	 * chance of stripping a symbol that starts with "_$S" that is not a
	 * swift symbol.
	 */
	swift_version = 0;
	if(Tflag && (mh_flags & MH_DYLDLINK) == MH_DYLDLINK){
	    p_objc_image_info = NULL;
	    for(i = 0; i < nsects; i++){
		if(object->mh != NULL){
		    if((strcmp(sections[i]->segname, "__DATA") == 0 ||
		        strcmp(sections[i]->segname, "__DATA_CONST") == 0 ||
		        strcmp(sections[i]->segname, "__DATA_DIRTY") == 0) &&
		       strncmp(sections[i]->sectname, "__objc_imageinfo", 16)
									== 0 &&
		       sections[i]->size >= sizeof(struct objc_image_info)) {
			p_objc_image_info = object->object_addr +
					    sections[i]->offset;
			break;
		    }
		}
		else{
		    if((strcmp(sections64[i]->segname, "__DATA") == 0 ||
		        strcmp(sections64[i]->segname, "__DATA_CONST") == 0 ||
		        strcmp(sections64[i]->segname, "__DATA_DIRTY") == 0) &&
		       strncmp(sections64[i]->sectname, "__objc_imageinfo", 16)
									== 0 &&
		       sections64[i]->size >= sizeof(struct objc_image_info)) {
			p_objc_image_info = object->object_addr +
					    sections64[i]->offset;
			break;
		    }
		}
	    }
	    if(p_objc_image_info != NULL){
		memcpy(&o, p_objc_image_info, sizeof(struct objc_image_info));
		if(object->object_byte_sex != get_host_byte_sex())
		    swap_objc_image_info(&o, get_host_byte_sex());
		swift_version = (o.flags >> 8) & 0xff;
	    }
	}
	*p_swift_version = swift_version;

        /*
         * Build the list of symbols to save. Also compute the space required
         * by the external and unknown symbol strings,
         *
         * In order to unique local symbol strings in a reasonable amount of
         * time-complexity, we will build a list of strx string indexes to
         * retain in the final file. This list will be processed and measured
         * outside of this nsyms loop.
         */
	for(i = 0; i < nsyms; i++){
	    s_flags = 0;
	    if(object->mh != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_sect = symbols[i].n_sect;
		if((n_type & N_TYPE) == N_SECT){
		    if(n_sect == 0 || n_sect > nsects){
			error_arch(arch, member, "bad n_sect for symbol "
				   "table entry %d in: ", i);
			return(FALSE);
		    }
		    s_flags = sections[n_sect - 1]->flags;
		}
		n_desc = symbols[i].n_desc;
		n_value = symbols[i].n_value;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_sect = symbols64[i].n_sect;
		if((n_type & N_TYPE) == N_SECT){
		    if(n_sect == 0 || n_sect > nsects){
			error_arch(arch, member, "bad n_sect for symbol "
				   "table entry %d in: ", i);
			return(FALSE);
		    }
		    s_flags = sections64[n_sect - 1]->flags;
		}
		n_desc = symbols64[i].n_desc;
		n_value = symbols64[i].n_value;
	    }
	    if(n_strx != 0){
		if(n_strx > strsize){
		    error_arch(arch, member, "bad string index for symbol "
			       "table entry %d in: ", i);
		    return(FALSE);
		}
	    }
	    if((n_type & N_TYPE) == N_INDR){
		if(n_value != 0){
		    if(n_value > strsize){
			error_arch(arch, member, "bad string index for "
				   "indirect symbol table entry %d in: ", i);
			return(FALSE);
		    }
		}
	    }
	    /*
             * If we use the -N flag to set strip all symbols in binaries used
	     * with the dynamic linker set *nlist_outofsync_with_dyldinfo to
	     * true.
	     */
	    if(strip_all_nlists) {
		*nlist_outofsync_with_dyldinfo = TRUE;
		continue;
	    }
	    /*
	     * If the -T flag is specified then if this is a final linked
	     * binary never save symbols that start with "_$S" if the
	     * swift version is non-zero.
	     */
	    if(Tflag && swift_version != 0 &&
	       (mh_flags & MH_DYLDLINK) == MH_DYLDLINK &&
	       n_strx != 0 &&
               (strncmp(strings + n_strx, "_$S", 3) == 0 ||
                strncmp(strings + n_strx, "_$s", 3) == 0)){
		/* don't save this symbol */
		*nlist_outofsync_with_dyldinfo = TRUE;
		continue;
	    }
	    if((n_type & N_EXT) == 0){ /* local symbol */
		/*
		 * For x86_64, i386 .o or ARM files we have run ld -r on them
		 * we keeping all resulting symbols.
		 */
		if((object->mh_cputype == CPU_TYPE_X86_64 ||
		    object->mh_cputype == CPU_TYPE_I386 ||
                    object->mh_cputype == CPU_TYPE_ARM64 ||
                    object->mh_cputype == CPU_TYPE_ARM64_32 ||
		    object->mh_cputype == CPU_TYPE_ARM) &&
		   object->mh_filetype == MH_OBJECT){
		    if(n_strx != 0)
                        strx_map[strx_count++].old_strx = n_strx;
		    new_nlocalsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		/*
		 * The cases a local symbol might be saved is with -X -S or
		 * with -d filename.
		 */
		else if((!strip_all && (Xflag || Sflag)) || dfile){
		    if(n_type & N_STAB){ /* debug symbol */
			if(dfile && n_type == N_SO){
			    if(n_strx != 0){
				basename = strrchr(strings + n_strx, '/');
				if(basename != NULL)
				    basename++;
				else
				    basename = strings + n_strx;
				pp = bsearch(basename, debug_filenames,
					    ndebug_filenames, sizeof(char *),
			 		    (int (*)(const void *, const void *)
					    )cmp_bsearch_filename);
				/*
				 * Save the bracketing N_SO. For each N_SO that
				 * has a filename there is an N_SO that has a
				 * name of "" which ends the stabs for that file
				 */
				if(*basename != '\0'){
				    if(pp != NULL)
					save_debug = 1;
				    else
					save_debug = 0;
				}
				else{
				    /*
				     * This is a bracketing SO so if we are
				     * currently saving debug symbols save this
				     * last one and turn off saving debug syms.
				     */
				    if(save_debug){
					if(n_strx != 0)
                                            strx_map[strx_count++].old_strx =
                                                n_strx;
					new_nlocalsym++;
					new_nsyms++;
					saves[i] = new_nsyms;
				    }
				    save_debug = 0;
				}
			    }
			    else{
				save_debug = 0;
			    }
			}
			if(saves[i] == 0 && (!Sflag || save_debug)){
			    if(n_strx != 0)
                                strx_map[strx_count++].old_strx = n_strx;
			    new_nlocalsym++;
			    new_nsyms++;
			    saves[i] = new_nsyms;
			}
		    }
		    else{ /* non-debug local symbol */
			if(xflag == 0 && (Sflag || Xflag)){
			    if(Xflag == 0 ||
			       (n_strx != 0 &&
		                strings[n_strx] != 'L')){
				/*
				 * If this file is a for the dynamic linker and
				 * this symbol is in a section marked so that
				 * static symbols are stripped then don't
				 * keep this symbol.
				 */
				if((mh_flags & MH_DYLDLINK) != MH_DYLDLINK ||
				   (n_type & N_TYPE) != N_SECT ||
			   	   (s_flags & S_ATTR_STRIP_STATIC_SYMS) != 
					      S_ATTR_STRIP_STATIC_SYMS){
                                    strx_map[strx_count++].old_strx = n_strx;
				    new_nlocalsym++;
				    new_nsyms++;
				    saves[i] = new_nsyms;
				}
			    }
			}
			/*
			 * Treat a local symbol that was a private extern as if
			 * were global if it is referenced by a module and save
			 * it.
			 */
			if((n_type & N_PEXT) == N_PEXT){
			    if(saves[i] == 0 &&
			       private_extern_reference_by_module(
				i, refs ,nextrefsyms) == TRUE){
				if(n_strx != 0)
				    //new_strsize += strlen(strings + n_strx) + 1;
                                    strx_map[strx_count++].old_strx = n_strx;
				new_nlocalsym++;
				new_nsyms++;
				saves[i] = new_nsyms;
			    }
			    /*
			     * We need to save symbols that were private externs
			     * that are used with indirect symbols.
			     */
			    if(saves[i] == 0 &&
			       symbol_pointer_used(i, indirectsyms,
						   nindirectsyms) == TRUE){
				if(n_strx != 0)
                                    strx_map[strx_count++].old_strx = n_strx;
				new_nlocalsym++;
				new_nsyms++;
				saves[i] = new_nsyms;
			    }
			}
		    }
		}
		/*
		 * Treat a local symbol that was a private extern as if were
		 * global if it is not referenced by a module.
		 */
		else if((n_type & N_PEXT) == N_PEXT){
		    if(saves[i] == 0 && sfile){
			sp = bsearch(strings + n_strx,
				     save_symbols, nsave_symbols,
				     sizeof(struct symbol_list),
				     (int (*)(const void *, const void *))
					symbol_list_bsearch);
			if(sp != NULL){
			    if(sp->sym == NULL){
				if(object->mh != NULL)
				    sp->sym = &(symbols[i]);
				else
				    sp->sym = &(symbols64[i]);
				sp->seen = TRUE;
			    }
			    if(n_strx != 0)
                                strx_map[strx_count++].old_strx = n_strx;
			    new_nlocalsym++;
			    new_nsyms++;
			    saves[i] = new_nsyms;
			}
		    }
		    if(saves[i] == 0 &&
		       private_extern_reference_by_module(
			i, refs ,nextrefsyms) == TRUE){
			if(n_strx != 0)
                            strx_map[strx_count++].old_strx = n_strx;
			new_nlocalsym++;
			new_nsyms++;
			saves[i] = new_nsyms;
		    }
		    /*
		     * We need to save symbols that were private externs that
		     * are used with indirect symbols.
		     */
		    if(saves[i] == 0 &&
		       symbol_pointer_used(i, indirectsyms, nindirectsyms) ==
									TRUE){
			if(n_strx != 0)
                            strx_map[strx_count++].old_strx = n_strx;
			new_nlocalsym++;
			new_nsyms++;
			saves[i] = new_nsyms;
		    }
		}
	    }
	    else{ /* global symbol */
		/*
		 * strip -R on an x86_64 .o file should do nothing.
		 */
		if(Rfile &&
		   (object->mh != NULL ||
		    object->mh64->cputype != CPU_TYPE_X86_64 ||
		    object->mh64->filetype != MH_OBJECT)){
		    sp = bsearch(strings + n_strx,
				 remove_symbols, nremove_symbols,
				 sizeof(struct symbol_list),
				 (int (*)(const void *, const void *))
				    symbol_list_bsearch);
		    if(sp != NULL){
			if((n_type & N_TYPE) == N_UNDF ||
			   (n_type & N_TYPE) == N_PBUD){
			    error_arch(arch, member, "symbol: %s undefined"
				       " and can't be stripped from: ",
				       sp->name);
			}
			else if(sp->sym != NULL){
			    sym = (struct nlist *)sp->sym;
			    if((sym->n_type & N_PEXT) != N_PEXT)
				error_arch(arch, member, "more than one symbol "
					   "for: %s found in: ", sp->name);
			}
			else{
			    if(object->mh != NULL)
				sp->sym = &(symbols[i]);
			    else
				sp->sym = &(symbols64[i]);
			    sp->seen = TRUE;
			}
			if(n_desc & REFERENCED_DYNAMICALLY){
			    error_arch(arch, member, "symbol: %s is dynamically"
				       " referenced and can't be stripped "
				       "from: ", sp->name);
			}
	    		if((n_type & N_TYPE) == N_SECT &&
			   (s_flags & SECTION_TYPE) == S_COALESCED){
			    error_arch(arch, member, "symbol: %s is a global "
				       "coalesced symbol and can't be "
				       "stripped from: ", sp->name);
			}
			/* don't save this symbol */
			continue;
		    }
		}
		if(Aflag && (n_type & N_TYPE) == N_ABS &&
		   (n_value != 0 ||
		   (n_strx != 0 &&
		    strncmp(strings + n_strx,
			    ".objc_class_name_",
			    sizeof(".objc_class_name_") - 1) == 0))){
		    len = strlen(strings + n_strx) + 1;
		    new_ext_strsize += len;
		    new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		if(saves[i] == 0 && (uflag || default_dyld_executable) &&
		   ((((n_type & N_TYPE) == N_UNDF) &&
		     n_value == 0) ||
		    (n_type & N_TYPE) == N_PBUD)){
		    if(n_strx != 0){
			len = strlen(strings + n_strx) + 1;
			new_ext_strsize += len;
		    }
		    new_nundefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		if(saves[i] == 0 && nflag &&
		   (n_type & N_TYPE) == N_SECT){
		    if(n_strx != 0){
			len = strlen(strings + n_strx) + 1;
			new_ext_strsize += len;
		    }
		    new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		if(saves[i] == 0 && sfile){
		    sp = bsearch(strings + n_strx,
				 save_symbols, nsave_symbols,
				 sizeof(struct symbol_list),
				 (int (*)(const void *, const void *))
				    symbol_list_bsearch);
		    if(sp != NULL){
			if(sp->sym != NULL){
			    sym = (struct nlist *)sp->sym;
			    if((sym->n_type & N_PEXT) != N_PEXT)
				error_arch(arch, member, "more than one symbol "
					   "for: %s found in: ", sp->name);
			}
			else{
			    if(object->mh != NULL)
				sp->sym = &(symbols[i]);
			    else
				sp->sym = &(symbols64[i]);
			    sp->seen = TRUE;
			    len = strlen(strings + n_strx) + 1;
			    new_ext_strsize += len;
			    if((n_type & N_TYPE) == N_UNDF ||
			       (n_type & N_TYPE) == N_PBUD)
				new_nundefsym++;
			    else
				new_nextdefsym++;
			    new_nsyms++;
			    saves[i] = new_nsyms;
			}
		    }
		}
		/*
		 * We only need to save coalesced symbols that are used as
		 * indirect symbols in 32-bit applications.
		 *
		 * In 64-bit applications, we only need to save coalesced
		 * symbols that are used as weak definitions.
		 */
		if(object->mh != NULL &&
		   saves[i] == 0 &&
		   (n_type & N_TYPE) == N_SECT &&
		   (s_flags & SECTION_TYPE) == S_COALESCED &&
		   symbol_pointer_used(i, indirectsyms, nindirectsyms) == TRUE){
		    if(n_strx != 0){
			len = strlen(strings + n_strx) + 1;
			new_ext_strsize += len;
		    }
		    new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		if(saves[i] == 0 &&
		   (n_type & N_TYPE) == N_SECT &&
		   (n_desc & N_WEAK_DEF) != 0){
		    if(n_strx != 0){
			len = strlen(strings + n_strx) + 1;
			new_ext_strsize += len;
		    }
		    new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		if(saves[i] == 0 && ((Xflag || Sflag || xflag) ||
		   ((rflag || default_dyld_executable) &&
		    n_desc & REFERENCED_DYNAMICALLY))){
		    len = strlen(strings + n_strx) + 1;
		    new_ext_strsize += len;
		    if((n_type & N_TYPE) == N_INDR){
			len = strlen(strings + n_value) + 1;
			new_ext_strsize += len;
		    }
		    if((n_type & N_TYPE) == N_UNDF ||
		       (n_type & N_TYPE) == N_PBUD)
			new_nundefsym++;
		    else
			new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
		/*
		 * For x86_64 and i386 .o files we have run ld -r on them and
		 * are stuck keeping all resulting symbols.
		 */
		if(saves[i] == 0 &&
		   ((object->mh == NULL && 
		     object->mh64->cputype == CPU_TYPE_X86_64 &&
		     object->mh64->filetype == MH_OBJECT) ||
		    (object->mh64 == NULL && 
		     object->mh->cputype == CPU_TYPE_I386 &&
		     object->mh->filetype == MH_OBJECT))){
		    len = strlen(strings + n_strx) + 1;
		    new_ext_strsize += len;
		    if((n_type & N_TYPE) == N_INDR){
			len = strlen(strings + n_value) + 1;
			new_ext_strsize += len;
		    }
		    if((n_type & N_TYPE) == N_UNDF ||
		       (n_type & N_TYPE) == N_PBUD)
			new_nundefsym++;
		    else
			new_nextdefsym++;
		    new_nsyms++;
		    saves[i] = new_nsyms;
		}
	    }
	}
	/*
	 * The module table's module names are placed with the external strings.
	 * So size them and add this to the external string size.
	 */
	for(i = 0; i < nmodtab; i++){
	    if(object->mh != NULL)
		module_name = mods[i].module_name;
	    else
		module_name = mods64[i].module_name;
	    if(module_name == 0 || module_name > strsize){
		error_arch(arch, member, "bad string index for module_name "
			   "of module table entry %d in: ", i);
		return(FALSE);
	    }
	    len = strlen(strings + module_name) + 1;
	    new_ext_strsize += len;
	}

	/*
	 * Updating the reference table may require a symbol not yet listed as
	 * as saved to be present in the output file.  If a defined external
	 * symbol is removed and there is a undefined reference to it in the
	 * reference table an undefined symbol needs to be created for it in
	 * the output file.  If this happens the number of new symbols and size
	 * of the new strings are adjusted.  And the array changes[] is set to
	 * map the old symbol index to the new symbol index for the symbol that
	 * is changed to an undefined symbol.
	 */
	missing_symbols = 0;
	if(ref_saves != NULL)
	    free(ref_saves);
	ref_saves = (int32_t *)allocate(nextrefsyms * sizeof(int32_t));
	bzero(ref_saves, nextrefsyms * sizeof(int32_t));
	changes = (uint32_t *)allocate(nsyms * sizeof(int32_t));
	bzero(changes, nsyms * sizeof(int32_t));
	new_nextrefsyms = 0;
	for(i = 0; i < nextrefsyms; i++){
	    if(refs[i].isym > nsyms){
		error_arch(arch, member, "bad symbol table index for "
			   "reference table entry %d in: ", i);
		return(FALSE);
	    }
	    if(saves[refs[i].isym]){
		new_nextrefsyms++;
		ref_saves[i] = new_nextrefsyms;
	    }
	    else{
		if(refs[i].flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
		   refs[i].flags == REFERENCE_FLAG_UNDEFINED_LAZY){
		    if(changes[refs[i].isym] == 0){
			if(object->mh != NULL)
			    n_strx = symbols[refs[i].isym].n_un.n_strx;
			else
			    n_strx = symbols64[refs[i].isym].n_un.n_strx;
			len = strlen(strings + n_strx) + 1;
			new_ext_strsize += len;
			new_nundefsym++;
			new_nsyms++;
			changes[refs[i].isym] = new_nsyms;
			new_nextrefsyms++;
			ref_saves[i] = new_nextrefsyms;
		    }
		}
		else{
		    if(refs[i].flags ==
				    REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY ||
		       refs[i].flags == REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY){
			if(missing_symbols == 0){
			    error_arch(arch, member, "private extern symbols "
			      "referenced by modules can't be stripped in: ");
			    missing_symbols = 1;
			}
			if(object->mh != NULL)
			    n_strx = symbols[refs[i].isym].n_un.n_strx;
			else
			    n_strx = symbols64[refs[i].isym].n_un.n_strx;
			fprintf(stderr, "%s\n", strings + n_strx);
			saves[refs[i].isym] = -1;
		    }
		}
	    }
	}
	if(missing_symbols == 1)
	    return(FALSE);

	if(member == NULL){
	    missing_syms = 0;
	    if(iflag == 0){
		for(i = 0; i < nsave_symbols; i++){
		    if(save_symbols[i].sym == NULL){
			if(missing_syms == 0){
			    error_arch(arch, member, "symbols names listed "
				       "in: %s not in: ", sfile);
			    missing_syms = 1;
			}
			fprintf(stderr, "%s\n", save_symbols[i].name);
		    }
		}
	    }
	    missing_syms = 0;
	    /*
	     * strip -R on an x86_64 .o file should do nothing.
	     */
	    if(iflag == 0 &&
	       (object->mh != NULL ||
		object->mh64->cputype != CPU_TYPE_X86_64 ||
		object->mh64->filetype != MH_OBJECT)){
		for(i = 0; i < nremove_symbols; i++){
		    if(remove_symbols[i].sym == NULL){
			if(missing_syms == 0){
			    error_arch(arch, member, "symbols names listed "
				       "in: %s not in: ", Rfile);
			    missing_syms = 1;
			}
			fprintf(stderr, "%s\n", remove_symbols[i].name);
		    }
		}
	    }
	}

        /*
         * preserve uniqued local symbol strings by sorting and uniqing the
         * n_strx values in strx_map. If the linker or some previous tool has
         * uniqued the strings, each n_strx represents a uniqued string.
         *
         * Note that strip currently will not unique local symbol strings
         * itself. It simply preserves the uniqueness when deserializing and
         * reserializing the strings table.
         */
        qsort(strx_map, strx_count, sizeof(*strx_map),
              (int (*)(const void *, const void *))cmp_qsort_strx_map);
        for (j = 0; j < strx_count; ++j) {
            if (strx_map[strx_uniqcount].old_strx != strx_map[j].old_strx) {
	      strx_uniqcount += 1;
                if (strx_uniqcount < j) {
                    strx_map[strx_uniqcount].old_strx = strx_map[j].old_strx;
                }
            }
        }
        if (strx_count > 0) {
            strx_uniqcount += 1;
        }
    
        /*
         * compute the size of the local symbol strings by measuring each
         * remaining string. new_strsize represents the total size of the
         * strings table, so it will include the new_ext_strsize value. From
         * this point forward, new_ext_strsize represents the beginning of the
         * local symbol strings in the strings table.
         */
        for (i = 0; i < strx_uniqcount; ++i) {
            new_strsize += strlen(strings + strx_map[i].old_strx) + 1;
        }
        new_strsize += new_ext_strsize;

	/*
	 * If there is a chance that we could end up with an indirect symbol
	 * with an index of zero we need to avoid that due to a work around
	 * in the dynamic linker for a bug it is working around that was in
	 * the old classic static linker.  See radar bug 5614542 and the
	 * related bugs 3685312 and 3534709.
	 *
	 * A reasonable way to do this to know that local symbols are first in
	 * the symbol table.  So if we have any local symbols this won't happen
	 * and if there are no indirect symbols it will also not happen.  Past
	 * that we'll just add a local symbol so it will end up at symbol index
	 * zero and avoid any indirect symbol having that index.
	 *
	 * If one really wanted they could build up the new symbol table then
	 * look at all the indirect symbol table entries to see if any of them
	 * have an index of zero then in that case throw that new symbol table
	 * away and rebuild the symbol and string table once again after adding 
         * a local symbol.  This seems not all that resonable to save one symbol
	 * table entry and a few bytes in the string table for the complexity it
	 * would add and what it would save.
	 */
	if(!strip_all_nlists &&
	   (new_nlocalsym == 0 && nindirectsyms != 0)){
	    len = strlen("radr://5614542") + 1;
	    new_strsize += len;
	    new_nlocalsym++;
	    new_nsyms++;
	    hack_5614542 = TRUE;
	}
	else{
    	    hack_5614542 = FALSE;
	}
    
	if(object->mh != NULL){
	    new_symbols = (struct nlist *)
			  allocate(new_nsyms * sizeof(struct nlist));
	    new_symbols64 = NULL;
	}
	else{
	    new_symbols = NULL;
	    new_symbols64 = (struct nlist_64 *)
			  allocate(new_nsyms * sizeof(struct nlist_64));
	}
	if(object->mh != NULL)
	    new_strsize = rnd32(new_strsize, sizeof(int32_t));
	else
	    new_strsize = rnd32(new_strsize, sizeof(int64_t));
	new_strings = (char *)allocate(new_strsize);
	if(object->mh != NULL){
	    new_strings[new_strsize - 3] = '\0';
	    new_strings[new_strsize - 2] = '\0';
	    new_strings[new_strsize - 1] = '\0';
	}
	else{
	    new_strings[new_strsize - 7] = '\0';
	    new_strings[new_strsize - 6] = '\0';
	    new_strings[new_strsize - 5] = '\0';
	    new_strings[new_strsize - 4] = '\0';
	    new_strings[new_strsize - 3] = '\0';
	    new_strings[new_strsize - 2] = '\0';
	    new_strings[new_strsize - 1] = '\0';
	}

        /*
         * Zero out the new_strings table, and calculate working pointers:
         *   p is the location where the next external string will go
         *   q is the location where the next local string will go
         */
	memset(new_strings, '\0', sizeof(int32_t));
	p = new_strings + sizeof(int32_t);
	q = p + new_ext_strsize;

	/*
	 * If all strings were stripped set the size to zero but only for 32-bit
	 * because the unified linker seems to set the filesize of empty .o
	 * files to include the string table.
	 */
	if(object->mh != NULL && new_strsize == sizeof(int32_t))
	    new_strsize = 0;

	/*
	 * Now create a symbol table and string table in this order
	 * symbol table
	 *	local symbols
	 *	external defined symbols
	 *	undefined symbols
	 * string table
	 *	external strings
	 *	local strings
	 */
	inew_syms = 0;

	/*
	 * If we are doing the hack for radar bug 5614542 (see above) add the
	 * one local symbol and string.
	 *
	 * We use an N_OPT stab which should be safe to use and not mess any
	 * thing up.  In Mac OS X, gcc(1) writes one N_OPT stab saying the file
	 * is compiled with gcc(1).   Then gdb(1) looks for that stab, but it
	 * also looks at the name.  If the name string is "gcc_compiled" or
	 * "gcc2_compiled" gdb(1) sets its "compiled by gcc flag.  If the N_OPT
	 * is emitted INSIDE an N_SO section, then gdb(1) thinks that object
	 * module was compiled by Sun's compiler, which apparently sticks one
	 * outermost N_LBRAC/N_RBRAC pair, which gdb(1) strips off.  But if the 
	 * N_OPT comes before any N_SO stabs, then gdb(1) will just ignore it.
	 * Since this N_OPT is the first local symbol, it will always come
	 * before any N_SO stabs that might be around and should be fine.
	 */
	if(hack_5614542 == TRUE){
	    if(object->mh != NULL){
		new_symbols[inew_syms].n_type = N_OPT;
		new_symbols[inew_syms].n_sect = NO_SECT;
		new_symbols[inew_syms].n_desc = 0;
		new_symbols[inew_syms].n_value = 0x05614542;
	    }
	    else{
		new_symbols64[inew_syms].n_type = N_OPT;
		new_symbols64[inew_syms].n_sect = NO_SECT;
		new_symbols64[inew_syms].n_desc = 0;
		new_symbols64[inew_syms].n_value = 0x05614542;
	    }
	    strcpy(q, "radr://5614542");
	    if(object->mh != NULL)
		new_symbols[inew_syms].n_un.n_strx = (uint32_t)
		    (q - new_strings);
	    else
		new_symbols64[inew_syms].n_un.n_strx = (uint32_t)
		    (q - new_strings);
	    q += strlen(q) + 1;
	    inew_syms++;
	}

        /*
         * write the local symbol names into the strings table, keeping track
         * of the new strx so we can preserve string uniqueness. Begin by
         * finding the strx_map entry for each symbol's n_strx. If the strx_map
         * entry does not yet have a strx value, copy the string into the
         * strings table and compute the new strx value; if the strx_map does
         * have a new strx value, simply reuse it and move on...
         */
	for(i = 0; i < nsyms; i++){
	    if(saves[i]){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		}
		if((n_type & N_EXT) == 0){
		    if(object->mh != NULL)
			new_symbols[inew_syms] = symbols[i];
		    else
			new_symbols64[inew_syms] = symbols64[i];
		    if(n_strx != 0){
                        struct strx_map* map =
                            bsearch(&n_strx, strx_map, strx_uniqcount,
                                    sizeof(*strx_map),
                                    (int(*)(const void*, const void*))
                                    cmp_bsearch_strx_map);
                        if (map != NULL) {
                            if (map->new_strx == 0) {
                                strcpy(q, strings + n_strx);
                                map->new_strx = (uint32_t)(q - new_strings);
                                q += strlen(q) + 1;
                            }
                            if(object->mh != NULL)
                                new_symbols[inew_syms].n_un.n_strx =
                                map->new_strx;
                            else
                                new_symbols64[inew_syms].n_un.n_strx =
                                map->new_strx;
                        }
                        else {
			    error_arch(arch, member, "n_strx %d is not in the "
				       "local symbol table index: ", n_strx);
			    return(FALSE);
                        }
		    }
		    inew_syms++;
		    saves[i] = inew_syms;
		}
	    }
	}
#ifdef TRIE_SUPPORT
	inew_nextdefsym = inew_syms;
#endif /* TRIE_SUPPORT */
	for(i = 0; i < nsyms; i++){
	    if(saves[i]){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		    n_value = symbols[i].n_value;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		    n_value = symbols64[i].n_value;
		}
		if((n_type & N_EXT) == N_EXT &&
		   ((n_type & N_TYPE) != N_UNDF &&
		    (n_type & N_TYPE) != N_PBUD)){
		    if(object->mh != NULL)
			new_symbols[inew_syms] = symbols[i];
		    else
			new_symbols64[inew_syms] = symbols64[i];
		    if(n_strx != 0){
			strcpy(p, strings + n_strx);
			if(object->mh != NULL)
			    new_symbols[inew_syms].n_un.n_strx =
				(uint32_t)(p - new_strings);
			else
			    new_symbols64[inew_syms].n_un.n_strx =
				(uint32_t)(p - new_strings);
			p += strlen(p) + 1;
		    }
		    if((n_type & N_TYPE) == N_INDR){
			if(n_value != 0){
			    strcpy(p, strings + n_value);
			    if(object->mh != NULL)
				new_symbols[inew_syms].n_value =
				    (uint32_t)(p - new_strings);
			    else
				new_symbols64[inew_syms].n_value =
				    (uint32_t)(p - new_strings);
			    p += strlen(p) + 1;
			}
		    }
		    inew_syms++;
		    saves[i] = inew_syms;
		}
	    }
	}
	/*
	 * Build the new undefined symbols into a map and sort it.
	 */
	inew_undefsyms = 0;
	if(object->mh != NULL){
	    undef_map = (struct undef_map *)allocate(new_nundefsym *
						     sizeof(struct undef_map));
	    undef_map64 = NULL;
	}
	else{
	    undef_map = NULL;
	    undef_map64 = (struct undef_map64 *)allocate(new_nundefsym *
						sizeof(struct undef_map64));
	}
	for(i = 0; i < nsyms; i++){
	    if(saves[i]){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		}
		if((n_type & N_EXT) == N_EXT &&
		   ((n_type & N_TYPE) == N_UNDF ||
		    (n_type & N_TYPE) == N_PBUD)){
		    if(object->mh != NULL)
			undef_map[inew_undefsyms].symbol = symbols[i];
		    else
			undef_map64[inew_undefsyms].symbol64 = symbols64[i];
		    if(n_strx != 0){
			strcpy(p, strings + n_strx);
			if(object->mh != NULL)
			    undef_map[inew_undefsyms].symbol.n_un.n_strx =
				(uint32_t)(p - new_strings);
			else
			    undef_map64[inew_undefsyms].symbol64.n_un.n_strx =
				(uint32_t)(p - new_strings);
			p += strlen(p) + 1;
		    }
		    if(object->mh != NULL)
			undef_map[inew_undefsyms].index = i;
		    else
			undef_map64[inew_undefsyms].index = i;
		    inew_undefsyms++;
		}
	    }
	}
	for(i = 0; i < nsyms; i++){
	    if(changes[i]){
		if(object->mh != NULL)
		    n_strx = symbols[i].n_un.n_strx;
		else
		    n_strx = symbols64[i].n_un.n_strx;
		if(n_strx != 0){
		    strcpy(p, strings + n_strx);
		    if(object->mh != NULL)
			undef_map[inew_undefsyms].symbol.n_un.n_strx =
			    (uint32_t)(p - new_strings);
		    else
			undef_map64[inew_undefsyms].symbol64.n_un.n_strx =
			    (uint32_t)(p - new_strings);
		    p += strlen(p) + 1;
		}
		if(object->mh != NULL){
		    undef_map[inew_undefsyms].symbol.n_type = N_UNDF | N_EXT;
		    undef_map[inew_undefsyms].symbol.n_sect = NO_SECT;
		    undef_map[inew_undefsyms].symbol.n_desc = 0;
		    undef_map[inew_undefsyms].symbol.n_value = 0;
		    undef_map[inew_undefsyms].index = i;
		}
		else{
		    undef_map64[inew_undefsyms].symbol64.n_type = N_UNDF |N_EXT;
		    undef_map64[inew_undefsyms].symbol64.n_sect = NO_SECT;
		    undef_map64[inew_undefsyms].symbol64.n_desc = 0;
		    undef_map64[inew_undefsyms].symbol64.n_value = 0;
		    undef_map64[inew_undefsyms].index = i;
		}
		inew_undefsyms++;
	    }
	}
	/* Sort the undefined symbols by name */
	qsort_strings = new_strings;
	if(object->mh != NULL)
	    qsort(undef_map, new_nundefsym, sizeof(struct undef_map),
		  (int (*)(const void *, const void *))cmp_qsort_undef_map);
	else
	    qsort(undef_map64, new_nundefsym, sizeof(struct undef_map64),
		  (int (*)(const void *, const void *))cmp_qsort_undef_map_64);
	/* Copy the symbols now in sorted order into new_symbols */
	for(i = 0; i < new_nundefsym; i++){
	    if(object->mh != NULL){
		new_symbols[inew_syms] = undef_map[i].symbol;
		inew_syms++;
		saves[undef_map[i].index] = inew_syms;
	    }
	    else{
		new_symbols64[inew_syms] = undef_map64[i].symbol64;
		inew_syms++;
		saves[undef_map64[i].index] = inew_syms;
	    }
	}

	/*
	 * Fixup the module table's module name strings adding them to the
	 * string table.  Also fix the indexes into the symbol table for
	 * external and local symbols.  And fix up the indexes into the
	 * reference table.
	 */
	for(i = 0; i < nmodtab; i++){
	    if(object->mh != NULL){
		strcpy(p, strings + mods[i].module_name);
		mods[i].module_name = (uint32_t)(p - new_strings);
		iextdefsym = mods[i].iextdefsym;
		nextdefsym = mods[i].nextdefsym;
		ilocalsym = mods[i].ilocalsym;
		nlocalsym = mods[i].nlocalsym;
		irefsym = mods[i].irefsym;
		nrefsym = mods[i].nrefsym;
	    }
	    else{
		strcpy(p, strings + mods64[i].module_name);
		mods64[i].module_name = (uint32_t)(p - new_strings);
		iextdefsym = mods64[i].iextdefsym;
		nextdefsym = mods64[i].nextdefsym;
		ilocalsym = mods64[i].ilocalsym;
		nlocalsym = mods64[i].nlocalsym;
		irefsym = mods64[i].irefsym;
		nrefsym = mods64[i].nrefsym;
	    }
	    p += strlen(p) + 1;

	    if(iextdefsym > nsyms){
		error_arch(arch, member, "bad index into externally defined "
		    "symbols of module table entry %d in: ", i);
		return(FALSE);
	    }
	    if(iextdefsym + nextdefsym > nsyms){
		error_arch(arch, member, "bad number of externally defined "
		    "symbols of module table entry %d in: ", i);
		return(FALSE);
	    }
	    for(j = iextdefsym; j < iextdefsym + nextdefsym; j++){
		if(saves[j] != 0 && changes[j] == 0)
		    break;
	    }
	    n = 0;
	    for(k = j; k < iextdefsym + nextdefsym; k++){
		if(saves[k] != 0 && changes[k] == 0)
		    n++;
	    }
	    if(n == 0){
		if(object->mh != NULL){
		    mods[i].iextdefsym = 0;
		    mods[i].nextdefsym = 0;
		}
		else{
		    mods64[i].iextdefsym = 0;
		    mods64[i].nextdefsym = 0;
		}
	    }
	    else{
		if(object->mh != NULL){
		    mods[i].iextdefsym = saves[j] - 1;
		    mods[i].nextdefsym = n;
		}
		else{
		    mods64[i].iextdefsym = saves[j] - 1;
		    mods64[i].nextdefsym = n;
		}
	    }

	    if(ilocalsym > nsyms){
		error_arch(arch, member, "bad index into symbols for local "
		    "symbols of module table entry %d in: ", i);
		return(FALSE);
	    }
	    if(ilocalsym + nlocalsym > nsyms){
		error_arch(arch, member, "bad number of local "
		    "symbols of module table entry %d in: ", i);
		return(FALSE);
	    }
	    for(j = ilocalsym; j < ilocalsym + nlocalsym; j++){
		if(saves[j] != 0)
		    break;
	    }
	    n = 0;
	    for(k = j; k < ilocalsym + nlocalsym; k++){
		if(saves[k] != 0)
		    n++;
	    }
	    if(n == 0){
		if(object->mh != NULL){
		    mods[i].ilocalsym = 0;
		    mods[i].nlocalsym = 0;
		}
		else{
		    mods64[i].ilocalsym = 0;
		    mods64[i].nlocalsym = 0;
		}
	    }
	    else{
		if(object->mh != NULL){
		    mods[i].ilocalsym = saves[j] - 1;
		    mods[i].nlocalsym = n;
		}
		else{
		    mods64[i].ilocalsym = saves[j] - 1;
		    mods64[i].nlocalsym = n;
		}
	    }

	    if(irefsym > nextrefsyms){
		error_arch(arch, member, "bad index into reference table "
		    "of module table entry %d in: ", i);
		return(FALSE);
	    }
	    if(irefsym + nrefsym > nextrefsyms){
		error_arch(arch, member, "bad number of reference table "
		    "entries of module table entry %d in: ", i);
		return(FALSE);
	    }
	    for(j = irefsym; j < irefsym + nrefsym; j++){
		if(ref_saves[j] != 0)
		    break;
	    }
	    n = 0;
	    for(k = j; k < irefsym + nrefsym; k++){
		if(ref_saves[k] != 0)
		    n++;
	    }
	    if(n == 0){
		if(object->mh != NULL){
		    mods[i].irefsym = 0;
		    mods[i].nrefsym = 0;
		}
		else{
		    mods64[i].irefsym = 0;
		    mods64[i].nrefsym = 0;
		}
	    }
	    else{
		if(object->mh != NULL){
		    mods[i].irefsym = ref_saves[j] - 1;
		    mods[i].nrefsym = n;
		}
		else{
		    mods64[i].irefsym = ref_saves[j] - 1;
		    mods64[i].nrefsym = n;
		}
	    }
	}

	/*
	 * Create a new reference table.
	 */
	new_refs = allocate(new_nextrefsyms * sizeof(struct dylib_reference));
	j = 0;
	for(i = 0; i < nextrefsyms; i++){
	    if(ref_saves[i]){
		if(saves[refs[i].isym]){
		    new_refs[j].isym = saves[refs[i].isym] - 1;
		    new_refs[j].flags = refs[i].flags;
		}
		else{
		    if(refs[i].flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
		       refs[i].flags == REFERENCE_FLAG_UNDEFINED_LAZY){
			new_refs[j].isym = changes[refs[i].isym] - 1;
			new_refs[j].flags = refs[i].flags;
		    }
		}
		j++;
	    }
	}

	/*
	 * Create a new dylib table of contents.
	 */
	new_ntoc = 0;
	for(i = 0; i < ntoc; i++){
	    if(tocs[i].symbol_index >= nsyms){
		error_arch(arch, member, "bad symbol index for table of "
		    "contents table entry %d in: ", i);
		return(FALSE);
	    }
	    if(saves[tocs[i].symbol_index] != 0 &&
	       changes[tocs[i].symbol_index] == 0)
		new_ntoc++;
	}
	new_tocs = allocate(new_ntoc * sizeof(struct dylib_table_of_contents));
	j = 0;
	for(i = 0; i < ntoc; i++){
	    if(saves[tocs[i].symbol_index] != 0 &&
	       changes[tocs[i].symbol_index] == 0){
		new_tocs[j].symbol_index = saves[tocs[i].symbol_index] - 1;
		new_tocs[j].module_index = tocs[i].module_index;
		j++;
	    }
	}
#ifdef TRIE_SUPPORT
	/*
	 * Update the export trie if it has one but only call the the
	 * prune_trie() routine when we are removing global symbols as is
	 * done with default stripping of a dyld executable or with the -s
	 * or -R options.  If we are stripping nlist with the -N flag we must
	 * leave the export trie as is.
	 */
	if(!strip_all_nlists && object->dyld_info != NULL &&
	   object->dyld_info->export_size != 0 &&
	   (default_dyld_executable || sfile != NULL || Rfile != NULL)){
	    const char *error_string;
	    uint32_t trie_new_size;

	    error_string = prune_trie((uint8_t *)(object->object_addr +
						 object->dyld_info->export_off),
		       		      object->dyld_info->export_size,
		       		      prune,
				      &trie_new_size);
	    if(error_string != NULL){
		error_arch(arch, member, "%s", error_string);
		return(FALSE);
	    }
	}
#endif /* TRIE_SUPPORT */

	if(undef_map != NULL)
	    free(undef_map);
	if(undef_map64 != NULL)
	    free(undef_map64);
	if(changes != NULL)
	    free(changes);
	if(sections != NULL)
	    free(sections);
	if(sections64 != NULL)
	    free(sections64);
        if (strx_map != NULL)
            free(strx_map);

	if(errors == 0)
	    return(TRUE);
	else
	    return(FALSE);
}

#ifdef TRIE_SUPPORT
/*
 * prune() is called by prune_trie() and passed a name of an external symbol
 * in the trie.  It returns 1 if the symbols is to be pruned out and 0 if the
 * symbol is to be kept.
 *
 * Note that it may seem like a linear search of the new symbols would not be
 * the best approach but in 10.6 the only defined global symbol left in a
 * stripped executable is __mh_execute_header and new_nextdefsym is usually 1
 * so this never actually loops in practice.
 */
static
int
prune(
const char *name)
{
    uint32_t i;

	for(i = 0; i < new_nextdefsym; i++){
	    if(new_symbols != NULL){
		if(strcmp(name, new_strings + new_symbols[inew_nextdefsym + i]
							 .n_un.n_strx) == 0)
		    return(0);
	    }
	    else{
		if(strcmp(name, new_strings + new_symbols64[inew_nextdefsym + i]
							   .n_un.n_strx) == 0)
		    return(0);
	    }
	}
	return(1);
}
#endif /* TRIE_SUPPORT */

/*
 * make_ld_r_object() takes the object file contents referenced by the passed
 * data structures, writes that to a temporary file, runs "ld -r" plus the
 * specified stripping option creating a second temporary file, reads that file
 * in and replaces the object file contents with that and resets the variables
 * pointing to the symbol, string and indirect tables.
 */
static
void
make_ld_r_object(
struct arch *arch,
struct member *member,
struct object *object)
{
    enum byte_sex host_byte_sex;
    char *input_file, *output_file;
    int fd;
    struct ofile *ld_r_ofile;
    struct arch *ld_r_archs;
    uint32_t ld_r_narchs, save_errors;
    char* ld;

	host_byte_sex = get_host_byte_sex();

	ld = getenv("STRIP_LD");

	/*
	 * Swap the object file back into its bytesex before writing it to the
	 * temporary file if needed.
	 */
	if(object->object_byte_sex != host_byte_sex){
	    if(object->mh != NULL){
		if(swap_object_headers(object->mh, object->load_commands) ==
		   FALSE)
		    fatal("internal error: swap_object_headers() failed");
		swap_nlist(symbols, nsyms, object->object_byte_sex);
	    }
	    else{
		if(swap_object_headers(object->mh64, object->load_commands) ==
		   FALSE)
		    fatal("internal error: swap_object_headers() failed");
		swap_nlist_64(symbols64, nsyms, object->object_byte_sex);
	    }
	    swap_indirect_symbols(indirectsyms, nindirectsyms, 
				  object->object_byte_sex);
	}

	/*
	 * Create an input object file for the ld -r command from the bytes
	 * of this arch's object file.
	 */
	input_file = makestr("/tmp/strip.XXXXXX", NULL);
	input_file = mktemp(input_file);

	if((fd = open(input_file, O_WRONLY|O_CREAT, 0600)) < 0)
	    system_fatal("can't open temporary file: %s", input_file);

	if(write64(fd, object->object_addr, object->object_size) !=
	        object->object_size)
	    system_fatal("can't write temporary file: %s", input_file);

	if(close(fd) == -1)
	    system_fatal("can't close temporary file: %s", input_file);

	/*
	 * Create a temporary name for the output file of the ld -r
	 */
	output_file = makestr("/tmp/strip.XXXXXX", NULL);
	output_file = mktemp(output_file);

	/*
	 * Create the ld -r command line and execute it.
	 */
	reset_execute_list();
	if (ld)
	    add_execute_list(ld);
	else
	    add_execute_list_with_prefix("ld");
	add_execute_list("-keep_private_externs");
	add_execute_list("-r");
	if(Sflag)
	    add_execute_list("-S");
	if(xflag)
	    add_execute_list("-x");
	add_execute_list(input_file);
	add_execute_list("-o");
	add_execute_list(output_file);
	if(sfile != NULL){
	    add_execute_list("-x");
	    add_execute_list("-exported_symbols_list");
	    add_execute_list(sfile);
	}
	if(Rfile != NULL){
	    add_execute_list("-unexported_symbols_list");
	    add_execute_list(Rfile);
	}
	if(execute_list(vflag) == 0)
	    fatal("internal link edit command failed");

	save_errors = errors;
	errors = 0;
	/* breakout the output file of the ld -f for processing */
	ld_r_ofile = breakout(output_file, &ld_r_archs, &ld_r_narchs, FALSE);
	if(errors)
	    goto make_ld_r_object_cleanup;

	/* checkout the file for symbol table replacement processing */
	checkout(ld_r_archs, ld_r_narchs);

	/*
	 * Make sure the output of the ld -r is an object file with one arch.
	 */
	if(ld_r_narchs != 1 ||
	   ld_r_archs->type != OFILE_Mach_O ||
	   ld_r_archs->object == NULL ||
	   ld_r_archs->object->mh_filetype != MH_OBJECT)
	    fatal("internal link edit command failed to produce a thin Mach-O "
		  "object file");

	/*
	 * Now reset all the data of the input object with the ld -r output
	 * object file.
	 */
	nsyms = ld_r_archs->object->st->nsyms;
	if(ld_r_archs->object->mh != NULL){
	    symbols = (struct nlist *)
		       (ld_r_archs->object->object_addr +
			ld_r_archs->object->st->symoff);
	    if(ld_r_archs->object->object_byte_sex != host_byte_sex)
		swap_nlist(symbols, nsyms, host_byte_sex);
	    symbols64 = NULL;
	}
	else{
	    symbols = NULL;
	    symbols64 = (struct nlist_64 *)
		         (ld_r_archs->object->object_addr +
			  ld_r_archs->object->st->symoff);
	    if(ld_r_archs->object->object_byte_sex != host_byte_sex)
		swap_nlist_64(symbols64, nsyms, host_byte_sex);
	}
	strings = ld_r_archs->object->object_addr +
		  ld_r_archs->object->st->stroff;
	strsize = ld_r_archs->object->st->strsize;

	if(ld_r_archs->object->dyst != NULL &&
	   ld_r_archs->object->dyst->nindirectsyms != 0){
	    nindirectsyms = ld_r_archs->object->dyst->nindirectsyms;
	    indirectsyms = (uint32_t *)
		(ld_r_archs->object->object_addr +
		 ld_r_archs->object->dyst->indirectsymoff);
	    if(ld_r_archs->object->object_byte_sex != host_byte_sex)
		swap_indirect_symbols(indirectsyms, nindirectsyms,
				      host_byte_sex);
	}
	else{
	    indirectsyms = NULL;
	    nindirectsyms = 0;
	}

	if(ld_r_archs->object->mh != NULL)
	    ld_r_archs->object->input_sym_info_size =
		nsyms * sizeof(struct nlist) +
		strsize;
	else
	    ld_r_archs->object->input_sym_info_size =
		nsyms * sizeof(struct nlist_64) +
		strsize;

	/*
	 * Copy over the object struct from the ld -r object file onto the
	 * input object file.
	 */
	*object = *ld_r_archs->object;

	/*
	 * Save the ofile struct for the ld -r output so it can be umapped when
	 * we are done.  And free up the ld_r_archs now that we are done with
	 * them.
	 */
	object->ld_r_ofile = ld_r_ofile;
	free_archs(ld_r_archs, ld_r_narchs);

make_ld_r_object_cleanup:
	errors += save_errors;
	/*
	 * Remove the input and output files and clean up.
	 */
	if(unlink(input_file) == -1)
	    system_fatal("can't remove temporary file: %s", input_file);
	if(unlink(output_file) == -1)
	    system_fatal("can't remove temporary file: %s", output_file);
	free(input_file);
	free(output_file);
}

/*
 * strip_LC_UUID_commands() is called when -no_uuid is specified to remove any
 * LC_UUID load commands from the object's load commands.
 */
static
void
strip_LC_UUID_commands(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, ncmds, nuuids, mh_sizeofcmds, sizeofcmds;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct segment_command *sg;

	/*
	 * See if there are any LC_UUID load commands.
	 */
	nuuids = 0;
	lc1 = object->load_commands;
        if(object->mh != NULL){
            ncmds = object->mh->ncmds;
	    mh_sizeofcmds = object->mh->sizeofcmds;
	}
	else{
            ncmds = object->mh64->ncmds;
	    mh_sizeofcmds = object->mh64->sizeofcmds;
	}
	for(i = 0; i < ncmds; i++){
	    if(lc1->cmd == LC_UUID){
		nuuids++;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
	/* if no LC_UUID load commands just return */
	if(nuuids == 0)
	    return;

	/*
	 * Allocate space for the new load commands as zero it out so any holes
	 * will be zero bytes.
	 */
	new_load_commands = allocate(mh_sizeofcmds);
	memset(new_load_commands, '\0', mh_sizeofcmds);

	/*
	 * Copy all the load commands except the LC_UUID load commands into the
	 * allocated space for the new load commands.
	 */
	lc1 = object->load_commands;
	lc2 = new_load_commands;
	sizeofcmds = 0;
	for(i = 0; i < ncmds; i++){
	    if(lc1->cmd != LC_UUID){
		memcpy(lc2, lc1, lc1->cmdsize);
		sizeofcmds += lc2->cmdsize;
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands.
	 */
	memcpy(object->load_commands, new_load_commands, sizeofcmds);
	if(mh_sizeofcmds > sizeofcmds){
		memset((char *)object->load_commands + sizeofcmds, '\0', 
			   (mh_sizeofcmds - sizeofcmds));
	}
	ncmds -= nuuids;
        if(object->mh != NULL) {
            object->mh->sizeofcmds = sizeofcmds;
            object->mh->ncmds = ncmds;
        } else {
            object->mh64->sizeofcmds = sizeofcmds;
            object->mh64->ncmds = ncmds;
        }
	free(new_load_commands);

	/* reset the pointers into the load commands */
	lc1 = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    object->seg_linkedit = sg;
		break;
	    case LC_SEGMENT_SPLIT_INFO:
		object->split_info_cmd = (struct linkedit_data_command *)lc1;
		break;
	    case LC_FUNCTION_STARTS:
		object->func_starts_info_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		object->data_in_code_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		object->code_sign_drs_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		object->link_opt_hint_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_CODE_SIGNATURE:
		object->code_sig_cmd = (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_INFO_ONLY:
	    case LC_DYLD_INFO:
		object->dyld_info = (struct dyld_info_command *)lc1;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}

/*
 * strip_LC_SEGMENT_SPLIT_INFO_command() is called when -no_split_info is
 * specified to the LC_SEGMENT_SPLIT_INFO load command from the object's load
 * commands.
 */
static
void
strip_LC_SEGMENT_SPLIT_INFO_command(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, ncmds, mh_sizeofcmds, sizeofcmds;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct segment_command *sg;

	/*
	 * See if there is a LC_SEGMENT_SPLIT_INFO load command.
	 * if no LC_SEGMENT_SPLIT_INFO load command just return.
	 */
	if(object->split_info_cmd == NULL)
	    return;

	/*
	 * Allocate space for the new load commands as zero it out so any holes
	 * will be zero bytes.
	 */
        if(object->mh != NULL){
            ncmds = object->mh->ncmds;
	    mh_sizeofcmds = object->mh->sizeofcmds;
	}
	else{
            ncmds = object->mh64->ncmds;
	    mh_sizeofcmds = object->mh64->sizeofcmds;
	}
	new_load_commands = allocate(mh_sizeofcmds);
	memset(new_load_commands, '\0', mh_sizeofcmds);

	/*
	 * Copy all the load commands except the LC_SEGMENT_SPLIT_INFO load
	 * command into the allocated space for the new load commands.
	 */
	lc1 = object->load_commands;
	lc2 = new_load_commands;
	sizeofcmds = 0;
	for(i = 0; i < ncmds; i++){
	    if(lc1->cmd != LC_SEGMENT_SPLIT_INFO){
		memcpy(lc2, lc1, lc1->cmdsize);
		sizeofcmds += lc2->cmdsize;
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands.
	 */
	memcpy(object->load_commands, new_load_commands, sizeofcmds);
	if(mh_sizeofcmds > sizeofcmds){
		memset((char *)object->load_commands + sizeofcmds, '\0', 
			   (mh_sizeofcmds - sizeofcmds));
	}
	ncmds -= 1;
        if(object->mh != NULL) {
            object->mh->sizeofcmds = sizeofcmds;
            object->mh->ncmds = ncmds;
        } else {
            object->mh64->sizeofcmds = sizeofcmds;
            object->mh64->ncmds = ncmds;
        }
	free(new_load_commands);

	/* reset the pointers into the load commands */
	object->split_info_cmd = NULL;
	lc1 = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    object->seg_linkedit = sg;
		break;
	    case LC_FUNCTION_STARTS:
		object->func_starts_info_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		object->data_in_code_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		object->code_sign_drs_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		object->link_opt_hint_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_CODE_SIGNATURE:
		object->code_sig_cmd = (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_INFO_ONLY:
	    case LC_DYLD_INFO:
		object->dyld_info = (struct dyld_info_command *)lc1;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}

#ifndef NMEDIT
/*
 * strip_LC_CODE_SIGNATURE_commands() is called when -c is specified to remove
 * any LC_CODE_SIGNATURE load commands from the object's load commands.
 */
static
void
strip_LC_CODE_SIGNATURE_commands(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, ncmds, mh_sizeofcmds, sizeofcmds;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct segment_command *sg;

	/*
	 * See if there is an LC_CODE_SIGNATURE load command and if no command
	 * just return.
	 */
	if(object->code_sig_cmd == NULL)
	    return;

	/*
	 * Allocate space for the new load commands and zero it out so any holes
	 * will be zero bytes.
	 */
        if(object->mh != NULL){
            ncmds = object->mh->ncmds;
	    mh_sizeofcmds = object->mh->sizeofcmds;
	}
	else{
            ncmds = object->mh64->ncmds;
	    mh_sizeofcmds = object->mh64->sizeofcmds;
	}
	new_load_commands = allocate(mh_sizeofcmds);
	memset(new_load_commands, '\0', mh_sizeofcmds);

	/*
	 * Copy all the load commands except the LC_CODE_SIGNATURE load commands
	 * into the allocated space for the new load commands.
	 */
	lc1 = object->load_commands;
	lc2 = new_load_commands;
	sizeofcmds = 0;
	for(i = 0; i < ncmds; i++){
	    if(lc1->cmd != LC_CODE_SIGNATURE){
		memcpy(lc2, lc1, lc1->cmdsize);
		sizeofcmds += lc2->cmdsize;
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands.
	 */
	memcpy(object->load_commands, new_load_commands, sizeofcmds);
	if(mh_sizeofcmds > sizeofcmds){
		memset((char *)object->load_commands + sizeofcmds, '\0', 
			   (mh_sizeofcmds - sizeofcmds));
	}
	ncmds -= 1;
        if(object->mh != NULL) {
            object->mh->sizeofcmds = sizeofcmds;
            object->mh->ncmds = ncmds;
        } else {
            object->mh64->sizeofcmds = sizeofcmds;
            object->mh64->ncmds = ncmds;
        }
	free(new_load_commands);

	/* reset the pointers into the load commands */
	object->code_sig_cmd = NULL;
	lc1 = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    object->seg_linkedit = sg;
		break;
	    case LC_SEGMENT_SPLIT_INFO:
		object->split_info_cmd = (struct linkedit_data_command *)lc1;
		break;
	    case LC_FUNCTION_STARTS:
		object->func_starts_info_cmd =
					 (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		object->data_in_code_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		object->code_sign_drs_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		object->link_opt_hint_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * To get the right amount of the file copied out by writeout() for the
	 * case when we are stripping out the section contents we already reduce
	 * the object size by the size of the section contents including the
	 * padding after the load commands.  So here we need to further reduce
	 * it by the load command for the LC_CODE_SIGNATURE (a struct
	 * linkedit_data_command) we are removing.
	 */
	object->object_size -= sizeof(struct linkedit_data_command);
	/*
 	 * Then this size minus the size of the input symbolic information is
	 * what is copied out from the file by writeout().  Which in this case
	 * is just the new headers.
	 */

	/*
	 * Finally for -c the file offset to the link edit information is to be
	 * right after the load commands.  So reset this for the updated size
	 * of the load commands without the LC_CODE_SIGNATURE.
	 */
	if(object->mh != NULL)
	    object->seg_linkedit->fileoff = sizeof(struct mach_header) +
					    sizeofcmds;
	else
	    object->seg_linkedit64->fileoff = sizeof(struct mach_header_64) +
					    sizeofcmds;
}
#endif /* !(NMEDIT) */

/*
 * private_extern_reference_by_module() is passed a symbol_index of a private
 * extern symbol and the module table.  If the symbol_index appears in the
 * module symbol table this returns TRUE else it returns FALSE.
 */
static
enum bool
private_extern_reference_by_module(
uint32_t symbol_index,
struct dylib_reference *refs,
uint32_t nextrefsyms)
{
    uint32_t i;

	for(i = 0; i < nextrefsyms; i++){
	    if(refs[i].isym == symbol_index){
		if(refs[i].flags == REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY ||
		   refs[i].flags == REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY){
		    return(TRUE);
		}
	    }
	}
	return(FALSE);
}

/*
 * symbol_pointer_used() is passed a symbol_index and the indirect table.  If
 * the symbol_index appears in the indirect symbol table this returns TRUE else
 * it returns FALSE.
 */
static
enum bool
symbol_pointer_used(
uint32_t symbol_index,
uint32_t *indirectsyms,
uint32_t nindirectsyms)
{
    uint32_t i;

	for(i = 0; i < nindirectsyms; i++){
	    if(indirectsyms[i] == symbol_index)
		return(TRUE);
	}
	return(FALSE);
}

/*
 * Functions for comparing strx_map entries.
 */
static
int
cmp_qsort_strx_map(
const struct strx_map* a,
const struct strx_map* b)
{
    return a->old_strx - b->old_strx;
}

static
int
cmp_bsearch_strx_map(const uint32_t* old_strx,
                     const struct strx_map *strx_map)
{
    return *old_strx - strx_map->old_strx;
}

/*
 * Function for qsort for comparing undefined map entries.
 */
static
int
cmp_qsort_undef_map(
const struct undef_map *sym1,
const struct undef_map *sym2)
{
	return(strcmp(qsort_strings + sym1->symbol.n_un.n_strx,
		      qsort_strings + sym2->symbol.n_un.n_strx));
}

static
int
cmp_qsort_undef_map_64(
const struct undef_map64 *sym1,
const struct undef_map64 *sym2)
{
	return(strcmp(qsort_strings + sym1->symbol64.n_un.n_strx,
		      qsort_strings + sym2->symbol64.n_un.n_strx));
}
#endif /* !defined(NMEDIT) */

#ifndef NMEDIT
/*
 * Function for qsort for comparing object names.
 */
static
int
cmp_qsort_filename(
const char **name1,
const char **name2)
{
	return(strcmp(*name1, *name2));
}

/*
 * Function for bsearch for finding a object name.
 */
static
int
cmp_bsearch_filename(
const char *name1,
const char **name2)
{
	return(strcmp(name1, *name2));
}
#endif /* !defined(NMEDIT) */

#ifdef NMEDIT
static
enum bool
edit_symtab(
struct arch *arch,
struct member *member,
struct object *object,
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
    uint32_t i, j, k;
    unsigned char data_n_sect, nsects;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s, **sections;
    struct section_64 *s64, **sections64;

    uint32_t missing_syms;
    struct symbol_list *sp;
    struct nlist **global_symbol;
    struct nlist_64 **global_symbol64;
    enum bool global_symbol_found;
    char *global_name, save_char;
    enum bool dwarf_debug_map;
    enum byte_sex host_byte_sex;
    int32_t missing_reloc_symbols;
    enum bool edit_symtab_return;

    char *p, *q;
    uint32_t new_ext_strsize, len, inew_syms;

    struct nlist **changed_globals;
    struct nlist_64 **changed_globals64;
    uint32_t nchanged_globals;
    uint32_t ncmds, s_flags, n_strx, module_name, ilocalsym, nlocalsym;
    uint32_t iextdefsym, nextdefsym;
    uint8_t n_type, n_sect, global_symbol_n_sect;
    uint64_t n_value;
    enum bool warned_about_global_coalesced_symbols;

	edit_symtab_return = TRUE;
	host_byte_sex = get_host_byte_sex();
	missing_reloc_symbols = 0;
	warned_about_global_coalesced_symbols = FALSE;

	if(nmedits != NULL)
	    free(nmedits);
	nmedits = allocate(nsyms * sizeof(enum bool));
	for(i = 0; i < nsyms; i++)
	    nmedits[i] = FALSE;

	/*
	 * If nmedit is operating on a dynamic library then symbols are turned
	 * into private externs with the extern bit off not into static symbols.
	 */
	if(object->mh_filetype == MH_DYLIB && pflag == TRUE){
	    error_arch(arch, member, "can't use -p with dynamic libraries");
	    return(FALSE);
	}

	/*
	 * As part of the MAJOR guess for the second pass to fix stabs for the
	 * globals symbols that get turned into non-global symbols.  We need to
	 * change the stabs.  To do this we to know if a N_GSYM is for a data
	 * symbol or not to know to turn it into an N_STSYM or a N_FUN.
	 * This logic as determined by compiling test cases with and without
	 * the key word 'static' and looking at the difference between the STABS
	 * the compiler generates and trying to match that here.
	 *
	 * We also use this loop and the next to gather an array of section
	 * struct pointers so we can later determine if we run into a global
	 * symbol in a coalesced section and not turn those symbols into
	 * statics.
	 */
	j = 0;
	nsects = 0;
	n_sect = 1;
	data_n_sect = NO_SECT;
	lc = object->load_commands;
	if(object->mh != NULL)
	    ncmds = object->mh->ncmds;
	else
	    ncmds = object->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)sg +
					sizeof(struct segment_command));
		nsects += sg->nsects;
		for(j = 0; j < sg->nsects; j++){
		    if(strcmp(s->segname, SEG_DATA) == 0 &&
		       strcmp(s->sectname, SECT_DATA) == 0 &&
		       data_n_sect == NO_SECT){
			data_n_sect = n_sect;
			break;
		    }
		    n_sect++;
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)sg64 +
					sizeof(struct segment_command_64));
		nsects += sg64->nsects;
		for(j = 0; j < sg64->nsects; j++){
		    if(strcmp(s64->segname, SEG_DATA) == 0 &&
		       strcmp(s64->sectname, SECT_DATA) == 0 &&
		       data_n_sect == NO_SECT){
			data_n_sect = n_sect;
			break;
		    }
		    n_sect++;
		    s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(object->mh != NULL){
	    sections = allocate(nsects * sizeof(struct section *));
	    sections64 = NULL;
	}
	else{
	    sections = NULL;
	    sections64 = allocate(nsects * sizeof(struct section_64 *));
	}
	nsects = 0;
	lc = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)sg +
					sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    sections[nsects++] = s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)sg64 +
					sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    sections64[nsects++] = s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/*
	 * Zero out the saved symbols so they can be recorded for this file.
	 */
	for(i = 0; i < nsave_symbols; i++)
	    save_symbols[i].sym = NULL;
	for(i = 0; i < nremove_symbols; i++)
	    remove_symbols[i].sym = NULL;
	if(member == NULL){
	    for(i = 0; i < nsave_symbols; i++)
		save_symbols[i].seen = FALSE;
	    for(i = 0; i < nremove_symbols; i++)
		remove_symbols[i].seen = FALSE;
	}

	nchanged_globals = 0;
	if(object->mh != NULL){
	    changed_globals = allocate(nsyms * sizeof(struct nlist *));
	    changed_globals64 = NULL;
	    for(i = 0; i < nsyms; i++)
		changed_globals[i] = NULL;
	}
	else{
	    changed_globals = NULL;
	    changed_globals64 = allocate(nsyms * sizeof(struct nlist_64 *));
	    for(i = 0; i < nsyms; i++)
		changed_globals64[i] = NULL;
	}

	/*
	 * These are the variables for the new symbol table and new string
	 * table.  Since this routine only turns globals into non-globals the
	 * number of symbols does not change.  But the count of local, defined
	 * external symbols does change.
	 */
	new_nsyms = nsyms;
	new_nlocalsym = 0;
	new_nextdefsym = 0;
	new_nundefsym = 0;

	new_strsize = sizeof(int32_t);
	new_ext_strsize = 0;

	/*
	 * First pass: turn the globals symbols into non-global symbols.
	 */
	for(i = 0; i < nsyms; i++){
	    len = 0;
	    s_flags = 0;
	    if(object->mh != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_sect = symbols[i].n_sect;
		if((n_type & N_TYPE) == N_SECT)
		    s_flags = sections[n_sect - 1]->flags;
		n_value = symbols[i].n_value;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_sect = symbols64[i].n_sect;
		if((n_type & N_TYPE) == N_SECT)
		    s_flags = sections64[n_sect - 1]->flags;
		n_value = symbols64[i].n_value;
	    }
	    if(n_strx != 0){
		if(n_strx > strsize){
		    error_arch(arch, member, "bad string index for symbol "
			       "table entry %u in: ", i);
		    return((uint32_t)FALSE);
		}
		len = (uint32_t)strlen(strings + n_strx) + 1;
	    }
	    if(n_type & N_EXT){
		if((n_type & N_TYPE) != N_UNDF &&
		   (n_type & N_TYPE) != N_PBUD){
		    if((n_type & N_TYPE) == N_SECT){
			if(n_sect > nsects){
			    error_arch(arch, member, "bad n_sect for symbol "
				       "table entry %u in: ", i);
			    return((uint32_t)FALSE);
			}
			if(((s_flags & SECTION_TYPE) == S_COALESCED) &&
			   pflag == FALSE &&
			   object->mh_filetype != MH_OBJECT){
			    /* this remains a global defined symbol */
			    if(warned_about_global_coalesced_symbols == FALSE){
				warning_arch(arch, member, "can't make global "
				    "coalesced symbols (like %s) into static "
				    "symbols (use ld(1)'s "
				    "-exported_symbols_list option) in a final "
				    "linked image: ", strings + n_strx);
				warned_about_global_coalesced_symbols = TRUE;
			    }
			    new_nextdefsym++;
			    new_ext_strsize += len;
			    new_strsize += len;
			    sp = bsearch(strings + n_strx,
					 remove_symbols, nremove_symbols,
					 sizeof(struct symbol_list),
					 (int (*)(const void *, const void *))
					    symbol_list_bsearch);
			    if(sp != NULL){
				if(sp->sym != NULL){
				    error_arch(arch, member, "more than one "
					"symbol for: %s found in: ", sp->name);
				    return(FALSE);
				}
				else{
				    if(object->mh != NULL)
					sp->sym = &(symbols[i]);
				    else
					sp->sym = &(symbols64[i]);
				    sp->seen = TRUE;
				    warning_arch(arch, member, "can't make "
					"global coalesced symbol: %s into a "
					"static symbol in: ", sp->name);
				}
			    }
			    /*
			     * In case the user has listed this coalesced
			     * symbol in the save list look for it and mark it
			     * as seen so we don't complain about not seeing it.
			     */
			    sp = bsearch(strings + n_strx,
					 save_symbols, nsave_symbols,
					 sizeof(struct symbol_list),
					 (int (*)(const void *, const void *))
					    symbol_list_bsearch);
			    if(sp != NULL){
				if(sp->sym != NULL){
				    error_arch(arch, member, "more than one "
					"symbol for: %s found in: ", sp->name);
				    return(FALSE);
				}
				else{
				    if(object->mh != NULL)
					sp->sym = &(symbols[i]);
				    else
					sp->sym = &(symbols64[i]);
				    sp->seen = TRUE;
				}
			    }
			    continue; /* leave this symbol unchanged */
			}
		    }
		    sp = bsearch(strings + n_strx,
				 remove_symbols, nremove_symbols,
				 sizeof(struct symbol_list),
				 (int (*)(const void *, const void *))
				    symbol_list_bsearch);
		    if(sp != NULL){
			if(sp->sym != NULL){
			    error_arch(arch, member, "more than one symbol "
				       "for: %s found in: ", sp->name);
			    return(FALSE);
			}
			else{
			    if(object->mh != NULL)
				sp->sym = &(symbols[i]);
			    else
				sp->sym = &(symbols64[i]);
			    sp->seen = TRUE;
			    goto change_symbol;
			}
		    }
		    else{
			/*
			 * If there is no list of saved symbols, then all
			 * symbols will be saved unless listed in the remove
			 * list.
			 */
			if(sfile == NULL){
			    /*
			     * There is no save list, so if there is also no
			     * remove list but the -p flag is specified or it is
			     * a dynamic library then change all symbols.
			     */
			    if((pflag || object->mh_filetype == MH_DYLIB)
			        && nremove_symbols == 0)
				goto change_symbol;
			    /* this remains a global defined symbol */
			    new_nextdefsym++;
			    new_ext_strsize += len;
			    new_strsize += len;
			    continue; /* leave this symbol unchanged */
			}
		    }
		    sp = bsearch(strings + n_strx,
				 save_symbols, nsave_symbols,
				 sizeof(struct symbol_list),
				 (int (*)(const void *, const void *))
				    symbol_list_bsearch);
		    if(sp != NULL){
			if(sp->sym != NULL){
			    error_arch(arch, member, "more than one symbol "
				       "for: %s found in: ", sp->name);
			    return(FALSE);
			}
			else{
			    if(object->mh != NULL)
				sp->sym = &(symbols[i]);
			    else
				sp->sym = &(symbols64[i]);
			    sp->seen = TRUE;
			    /* this remains a global defined symbol */
			    new_nextdefsym++;
			    new_ext_strsize += len;
			    new_strsize += len;
			}
		    }
		    else{
			if(Aflag && n_type == (N_EXT | N_ABS) &&
		            (n_value != 0 ||
		            (n_strx != 0 &&
			     strncmp(strings + n_strx,
				".objc_class_name_",
				sizeof(".objc_class_name_") - 1) == 0))){
			    /* this remains a global defined symbol */
			    new_nextdefsym++;
			    new_ext_strsize += len;
			    new_strsize += len;
			}
			else{
change_symbol:
			    if((n_type & N_TYPE) != N_INDR){
				nmedits[i] = TRUE;
				if(object->mh != NULL)
				    changed_globals[nchanged_globals++] =
					symbols + i;
				else
				    changed_globals64[nchanged_globals++] =
					symbols64 + i;
				if(pflag){
				    /* this remains a global defined symbol */
				    new_nextdefsym++;
				    new_ext_strsize += len;
				    new_strsize += len;
				}
				else{
				    /* this will become a non-global symbol */
				    new_nlocalsym++;
				    new_strsize += len;
				}
			    }
			    else{
				/* this remains a global defined symbol */
				new_nextdefsym++;
				new_ext_strsize += len;
				new_strsize += len;
			    }
			}
		    }
		}
		else{
		    /* this is an undefined symbol */
		    new_nundefsym++;
		    new_ext_strsize += len;
		    new_strsize += len;
		}
	    }
	    else{
		/* this is a local symbol */
		new_nlocalsym++;
		new_strsize += len;
	    }
	}

	/*
	 * The module table's module names are placed with the external
	 * strings. So size them and add this to the external string size.
	 */
	for(i = 0; i < nmodtab; i++){
	    if(object->mh != NULL)
		module_name = mods[i].module_name;
	    else
		module_name = mods64[i].module_name;
	    if(module_name == 0 || module_name > strsize){
		error_arch(arch, member, "bad string index for module_name "
			   "of module table entry %d in: ", i);
		return(FALSE);
	    }
	    len = (uint32_t)strlen(strings + module_name) + 1;
	    new_strsize += len;
	    new_ext_strsize += len;
	}

	/*
	 * Warn about symbols to be saved that were missing.
	 */
	if(member == NULL){
	    missing_syms = 0;
	    if(iflag == 0){
		for(i = 0; i < nsave_symbols; i++){
		    if(save_symbols[i].sym == NULL){
			if(missing_syms == 0){
			    error_arch(arch, member, "symbols names listed "
				       "in: %s not in: ", sfile);
			    missing_syms = 1;
			}
			fprintf(stderr, "%s\n", save_symbols[i].name);
		    }
		}
		for(i = 0; i < nremove_symbols; i++){
		    if(remove_symbols[i].sym == NULL){
			if(missing_syms == 0){
			    error_arch(arch, member, "symbols names listed "
				       "in: %s not in: ", Rfile);
			    missing_syms = 1;
			}
			fprintf(stderr, "%s\n", remove_symbols[i].name);
		    }
		}
	    }
	}

	/*
	 * Second pass: fix stabs for the globals symbols that got turned into
	 * non-global symbols.  This is a MAJOR guess.  The specific changes
	 * to do here were determined by compiling test cases with and without
	 * the key word 'static' and looking at the difference between the STABS
	 * the compiler generates and trying to match that here.
	 */
	global_strings = strings;
	if(object->mh != NULL)
	    qsort(changed_globals, nchanged_globals, sizeof(struct nlist *),
		  (int (*)(const void *, const void *))cmp_qsort_global);
	else
	    qsort(changed_globals64, nchanged_globals,sizeof(struct nlist_64 *),
		  (int (*)(const void *, const void *))cmp_qsort_global_64);
	dwarf_debug_map = FALSE;
	for(i = 0; i < nsyms; i++){
	  uint16_t n_desc;
	    if(object->mh != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_desc = symbols[i].n_desc;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_desc = symbols64[i].n_desc;
	    }
	    if(n_type == N_SO)
	      dwarf_debug_map = FALSE;
	    else if (n_type == N_OSO)
	      dwarf_debug_map = n_desc != 0;
	    else if (dwarf_debug_map && n_type == N_GSYM){
	      global_name = strings + n_strx;
	      if(object->mh != NULL){
		global_symbol = bsearch(global_name, changed_globals,
					nchanged_globals,sizeof(struct nlist *),
			     		(int (*)(const void *, const void *))
					cmp_bsearch_global);
		if(global_symbol != NULL){
		  symbols[i].n_type = N_STSYM;
		  symbols[i].n_sect = (*global_symbol)->n_sect;
		  symbols[i].n_value = (*global_symbol)->n_value;
		}
	      }
	      else{
		global_symbol64 = bsearch(global_name, changed_globals64,
					  nchanged_globals,
					  sizeof(struct nlist_64 *),
					  (int (*)(const void *, const void *))
					  cmp_bsearch_global_64);
		if(global_symbol64 != NULL){
		  symbols64[i].n_type = N_STSYM;
		  symbols64[i].n_sect = (*global_symbol64)->n_sect;
		  symbols64[i].n_value = (*global_symbol64)->n_value;
		}
	      }
	    }
	    else if(! dwarf_debug_map &&
		    (n_type == N_GSYM || n_type == N_FUN) &&
		    (n_strx != 0 && strings[n_strx] != '\0')){
		global_name = strings + n_strx;
		if((global_name[0] == '+' || global_name[0] == '-') &&
		   global_name[1] == '['){
		    j = 2;
		    while(j + n_strx < strsize && global_name[j] != ']')
			j++;
		    if(j + n_strx < strsize && global_name[j] == ']')
			j++;
		}
		else
		    j = 0;
		while(j + n_strx < strsize && global_name[j] != ':')
		    j++;
		if(j + n_strx >= strsize){
		    error_arch(arch, member, "bad N_STAB symbol name for entry "
			"%u (does not contain ':' separating name from type) "
			"in: ", i);
		    return(FALSE);
		}
		save_char = global_name[j];
		global_name[j] = '\0';

		global_symbol_found = FALSE;
		global_symbol_n_sect = 0;
		if(object->mh != NULL){
		    global_symbol = bsearch(global_name, changed_globals,
					nchanged_globals,sizeof(struct nlist *),
			     		(int (*)(const void *, const void *))
					cmp_bsearch_global_stab);
		    global_symbol64 = NULL;
		    if(global_symbol != NULL){
			global_symbol_found = TRUE;
			global_symbol_n_sect = (*global_symbol)->n_sect;
		    }
		}
		else{
		    global_symbol64 = bsearch(global_name, changed_globals64,
					nchanged_globals,
					sizeof(struct nlist_64 *),
			     		(int (*)(const void *, const void *))
					cmp_bsearch_global_stab_64);
		    global_symbol = NULL;
		    if(global_symbol64 != NULL){
			global_symbol_found = TRUE;
			global_symbol_n_sect = (*global_symbol64)->n_sect;
		    }
		}
		global_name[j] = save_char;
		if(global_symbol_found == TRUE){
		    if(n_type == N_GSYM){
			if(global_symbol_n_sect == data_n_sect){
			    if(object->mh != NULL)
				symbols[i].n_type = N_STSYM;
			    else
				symbols64[i].n_type = N_STSYM;
			}
			else{
			    if(object->mh != NULL)
				symbols[i].n_type = N_FUN;
			    else
				symbols64[i].n_type = N_FUN;
			}
			if(object->mh != NULL){
			    symbols[i].n_sect = (*global_symbol)->n_sect;
			    symbols[i].n_value = (*global_symbol)->n_value;
			    symbols[i].n_desc = (*global_symbol)->n_desc;
			}
			else{
			    symbols64[i].n_sect = (*global_symbol64)->n_sect;
			    symbols64[i].n_value = (*global_symbol64)->n_value;
			    symbols64[i].n_desc = (*global_symbol64)->n_desc;
			}
			if(j + 1 + n_strx >= strsize ||
			   global_name[j+1] != 'G'){
			    error_arch(arch, member, "bad N_GSYM symbol name "
				"for entry %u (does not have type 'G' after "
				"':' in name) in: ", i);
			    return(FALSE);
			}
		        global_name[j+1] = 'S';
		    }
		    else{ /* n_type == N_FUN */
			if(j + 1 + n_strx >= strsize ||
			   global_name[j+1] == 'F'){
			    global_name[j+1] = 'f';
			}
		    }
		}
	    }
	}
	global_strings = NULL;

	/*
	 * Now what needs to be done is to create the new symbol table moving
	 * those global symbols being changed into non-globals into the areas
	 * in the symbol table for local symbols.  The symbol table and string
	 * table must be in this order:
	 *
	 * symbol table
	 *	local symbols
	 *	external defined symbols
	 *	undefined symbols
	 * string table
	 *	external strings
	 *	local strings
	 */
	if(saves != NULL)
	    free(saves);
	saves = (int32_t *)allocate(nsyms * sizeof(int32_t));
	bzero(saves, nsyms * sizeof(int32_t));

	if(object->mh != NULL){
	    new_symbols = (struct nlist *)
			  allocate(new_nsyms * sizeof(struct nlist));
	    new_symbols64 = NULL;
	}
	else{
	    new_symbols = NULL;
	    new_symbols64 = (struct nlist_64 *)
			    allocate(new_nsyms * sizeof(struct nlist_64));
	}
	new_strsize = rnd32(new_strsize, sizeof(int32_t));
	new_strings = (char *)allocate(new_strsize);
	new_strings[new_strsize - 3] = '\0';
	new_strings[new_strsize - 2] = '\0';
	new_strings[new_strsize - 1] = '\0';

	memset(new_strings, '\0', sizeof(int32_t));
	p = new_strings + sizeof(int32_t);
	q = p + new_ext_strsize;

	/*
	 * If this is a dynamic library the movement of the symbols has to be
	 * done with respect to the modules.  As the local symbols, and external
	 * defined symbols are grouped together for each module.  Then a new
	 * module table needs to be created with the new indexes into the symbol
	 * table for each module.
	 */
	new_nmodtab = nmodtab;
	new_ntoc = ntoc;
	new_nextrefsyms = nextrefsyms;
	if(object->mh_filetype == MH_DYLIB && nmodtab != 0){
	    if(object->mh != NULL){
		new_mods = allocate(nmodtab * sizeof(struct dylib_module));
		new_mods64 = NULL;
	    }
	    else{
		new_mods = NULL;
		new_mods64 = allocate(nmodtab * sizeof(struct dylib_module_64));
	    }

	    inew_syms = 0;
	    /*
	     * This first loop through the module table sets the index and
	     * counts of the local symbols for each module.
	     */
	    for(i = 0; i < nmodtab; i++){
		/*
		 * First put the existing local symbols into the new symbol
		 * table.
		 */
		if(object->mh != NULL){
		    new_mods[i].ilocalsym = inew_syms;
		    new_mods[i].nlocalsym = 0;
		    ilocalsym = mods[i].ilocalsym;
		    nlocalsym = mods[i].nlocalsym;
		}
		else{
		    new_mods64[i].ilocalsym = inew_syms;
		    new_mods64[i].nlocalsym = 0;
		    ilocalsym = mods64[i].ilocalsym;
		    nlocalsym = mods64[i].nlocalsym;
		}
		for(j = ilocalsym; j < ilocalsym + nlocalsym; j++){
		    if(object->mh != NULL){
			n_strx = symbols[j].n_un.n_strx;
			n_type = symbols[j].n_type;
		    }
		    else{
			n_strx = symbols64[j].n_un.n_strx;
			n_type = symbols64[j].n_type;
		    }
		    if((n_type & N_EXT) == 0){
			if(object->mh != NULL)
			    new_symbols[inew_syms] = symbols[j];
			else
			    new_symbols64[inew_syms] = symbols64[j];
			if(n_strx != 0){
			    strcpy(q, strings + n_strx);
			    if(object->mh != NULL)
				new_symbols[inew_syms].n_un.n_strx =
				    (uint32_t)(q - new_strings);
			    else
				new_symbols64[inew_syms].n_un.n_strx =
				    (uint32_t)(q - new_strings);
			    q += strlen(q) + 1;
			}
			inew_syms++;
			saves[j] = inew_syms;
			if(object->mh != NULL)
			    new_mods[i].nlocalsym++;
			else
			    new_mods64[i].nlocalsym++;
		    }
		}
		/*
		 * Next put the global symbols that were changed into
		 * non-global symbols into the new symbol table and moved their
		 * counts to the local symbol counts.
		 */
		if(object->mh != NULL){
		    iextdefsym = mods[i].iextdefsym;
		    nextdefsym = mods[i].nextdefsym;
		}
		else{
		    iextdefsym = mods64[i].iextdefsym;
		    nextdefsym = mods64[i].nextdefsym;
		}
		for(j = iextdefsym; j < iextdefsym + nextdefsym; j++){
		    if(object->mh != NULL){
			n_strx = symbols[j].n_un.n_strx;
			n_type = symbols[j].n_type;
		    }
		    else{
			n_strx = symbols64[j].n_un.n_strx;
			n_type = symbols64[j].n_type;
		    }
		    if((n_type & N_EXT) != 0){
			if(nmedits[j] == TRUE){
			    /*
			     * Change the new symbol to a private extern symbol
			     * with the extern bit off.
			     */
			    if(object->mh != NULL){
				new_symbols[inew_syms] = symbols[j];
				new_symbols[inew_syms].n_type |= N_PEXT;
				new_symbols[inew_syms].n_type &= ~N_EXT;
			    }
			    else{
				new_symbols64[inew_syms] = symbols64[j];
				new_symbols64[inew_syms].n_type |= N_PEXT;
				new_symbols64[inew_syms].n_type &= ~N_EXT;
			    }
			    if(n_strx != 0){
				strcpy(q, strings + n_strx);
				if(object->mh != NULL)
				    new_symbols[inew_syms].n_un.n_strx =
					(uint32_t)(q - new_strings);
				else
				    new_symbols64[inew_syms].n_un.n_strx =
					(uint32_t)(q - new_strings);
				q += strlen(q) + 1;
			    }
			    inew_syms++;
			    saves[j] = inew_syms;
			    if(object->mh != NULL)
				new_mods[i].nlocalsym++;
			    else
				new_mods64[i].nlocalsym++;
			}
		    }
		}
	    }
	    /*
	     * Next put the unchanged defined global symbols into the new
	     * symbol table.
	     */
	    for(i = 0; i < nmodtab; i++){
		if(object->mh != NULL){
		    new_mods[i].iextdefsym = inew_syms;
		    new_mods[i].nextdefsym = 0;
		    iextdefsym = mods[i].iextdefsym;
		    nextdefsym = mods[i].nextdefsym;
		}
		else{
		    new_mods64[i].iextdefsym = inew_syms;
		    new_mods64[i].nextdefsym = 0;
		    iextdefsym = mods64[i].iextdefsym;
		    nextdefsym = mods64[i].nextdefsym;
		}
		for(j = iextdefsym; j < iextdefsym + nextdefsym; j++){
		    if(object->mh != NULL){
			n_strx = symbols[j].n_un.n_strx;
			n_type = symbols[j].n_type;
		    }
		    else{
			n_strx = symbols64[j].n_un.n_strx;
			n_type = symbols64[j].n_type;
		    }
		    if((n_type & N_EXT) != 0){
			if(nmedits[j] == FALSE){
			    if(object->mh != NULL)
				new_symbols[inew_syms] = symbols[j];
			    else
				new_symbols64[inew_syms] = symbols64[j];
			    if(n_strx != 0){
				strcpy(p, strings + n_strx);
				if(object->mh != NULL)
				    new_symbols[inew_syms].n_un.n_strx =
					(uint32_t)(p - new_strings);
				else
				    new_symbols64[inew_syms].n_un.n_strx =
					(uint32_t)(p - new_strings);
				p += strlen(p) + 1;
			    }
			    inew_syms++;
			    saves[j] = inew_syms;
			    if(object->mh != NULL)
				new_mods[i].nextdefsym++;
			    else
				new_mods64[i].nextdefsym++;
			}
		    }
		}
	    }
	    /*
	     * Last put the undefined symbols into the new symbol table.
	     */
	    for(i = 0; i < nsyms; i++){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		}
		if((n_type & N_EXT) != 0 &&
		   ((n_type & N_TYPE) == N_UNDF ||
		    (n_type & N_TYPE) == N_PBUD)){
		    if(object->mh != NULL)
			new_symbols[inew_syms] = symbols[i];
		    else
			new_symbols64[inew_syms] = symbols64[i];
		    if(n_strx != 0){
			strcpy(p, strings + n_strx);
			if(object->mh != NULL)
			    new_symbols[inew_syms].n_un.n_strx =
				(uint32_t)(p - new_strings);
			else
			    new_symbols64[inew_syms].n_un.n_strx =
				(uint32_t)(p - new_strings);
			p += strlen(p) + 1;
		    }
		    inew_syms++;
		    saves[i] = inew_syms;
		}
	    }

	    /*
	     * Place the module table's module names with the external strings
	     * and set the names in the new module table.  And then copy the
	     * other unchanged fields.
	     */
	    for(i = 0; i < nmodtab; i++){
		if(object->mh != NULL){
		    strcpy(p, strings + mods[i].module_name);
		    new_mods[i].module_name = (uint32_t)(p - new_strings);
		    p += strlen(p) + 1;

		    new_mods[i].irefsym = mods[i].irefsym;
		    new_mods[i].nrefsym = mods[i].nrefsym;
		    new_mods[i].iextrel = mods[i].iextrel;
		    new_mods[i].nextrel = mods[i].nextrel;
		    new_mods[i].iinit_iterm = mods[i].iinit_iterm;
		    new_mods[i].ninit_nterm = mods[i].ninit_nterm;
		    new_mods[i].objc_module_info_addr =
			mods[i].objc_module_info_addr;
		    new_mods[i].objc_module_info_size =
			mods[i].objc_module_info_size;
		}
		else{
		    strcpy(p, strings + mods64[i].module_name);
		    new_mods64[i].module_name = (uint32_t)(p - new_strings);
		    p += strlen(p) + 1;

		    new_mods64[i].irefsym = mods64[i].irefsym;
		    new_mods64[i].nrefsym = mods64[i].nrefsym;
		    new_mods64[i].iextrel = mods64[i].iextrel;
		    new_mods64[i].nextrel = mods64[i].nextrel;
		    new_mods64[i].iinit_iterm = mods64[i].iinit_iterm;
		    new_mods64[i].ninit_nterm = mods64[i].ninit_nterm;
		    new_mods64[i].objc_module_info_addr =
			mods64[i].objc_module_info_addr;
		    new_mods64[i].objc_module_info_size =
			mods64[i].objc_module_info_size;
		}
	    }

	    /*
	     * Update the reference table with the new symbol indexes for all
	     * entries and change type of reference (the flags field) for those
	     * symbols that got changed from globals to non-globals.
	     */
	    new_nextrefsyms = nextrefsyms;
	    new_refs = allocate(new_nextrefsyms *
				sizeof(struct dylib_reference));
	    j = 0;
	    for(i = 0; i < nextrefsyms; i++){
		if(nmedits[refs[i].isym] == TRUE){
		    if(refs[i].flags == REFERENCE_FLAG_DEFINED)
			new_refs[i].flags =
		 	    REFERENCE_FLAG_PRIVATE_DEFINED;
		    else if(refs[i].flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY)
			new_refs[i].flags =
			    REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY;
		    else if(refs[i].flags == REFERENCE_FLAG_UNDEFINED_LAZY)
			new_refs[i].flags =
			    REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY;
		    else
			new_refs[i].flags = refs[i].flags;
		}
		else{
		    new_refs[i].flags = refs[i].flags;
		}
		new_refs[i].isym = saves[refs[i].isym] - 1;
	    }

	    /*
	     * Create a new dylib table of contents without the global symbols
	     * that got turned into non-globals.
	     */
	    new_ntoc = ntoc - nchanged_globals;
	    new_tocs = allocate(new_ntoc *
				sizeof(struct dylib_table_of_contents));
	    k = 0;
	    for(i = 0; i < ntoc; i++){
		if(tocs[i].symbol_index >= nsyms){
		    error_arch(arch, member, "bad symbol index for table of "
			"contents table entry %d in: ", i);
		    return(FALSE);
		}
		if(nmedits[tocs[i].symbol_index] == FALSE){
		    new_tocs[k].symbol_index = saves[tocs[i].symbol_index] - 1;
		    new_tocs[k].module_index = tocs[i].module_index;
		    k++;
		}
	    }
	}
	/*
	 * If is not a dynamic library so all global symbols changed into
	 * statics can be moved to the end of the local symbols.  If the pflag
	 * is set then the changed symbols remain global and just get the
	 * private extern bit set.
	 */
	else{
	    /*
	     * First put the existing local symbols into the new symbol table.
	     */
	    inew_syms = 0;
	    for(i = 0; i < nsyms; i++){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		}
		if((n_type & N_EXT) == 0){
		    if(object->mh != NULL)
			new_symbols[inew_syms] = symbols[i];
		    else
			new_symbols64[inew_syms] = symbols64[i];
		    if(n_strx != 0){
			strcpy(q, strings + n_strx);
			if(object->mh != NULL)
			    new_symbols[inew_syms].n_un.n_strx =
				(uint32_t)(q - new_strings);
			else
			    new_symbols64[inew_syms].n_un.n_strx =
				(uint32_t)(q - new_strings);
			q += strlen(q) + 1;
		    }
		    inew_syms++;
		    saves[i] = inew_syms;
		}
	    }
	    /*
	     * Next put the global symbols that were changed into statics
	     * symbols into the new symbol table.
	     */
	    if(pflag == FALSE){
		for(i = 0; i < nsyms; i++){
		    if(object->mh != NULL){
			n_strx = symbols[i].n_un.n_strx;
			n_type = symbols[i].n_type;
		    }
		    else{
			n_strx = symbols64[i].n_un.n_strx;
			n_type = symbols64[i].n_type;
		    }
		    if((n_type & N_EXT) != 0){
			if(nmedits[i] == TRUE){
			    /*
			     * Change the new symbol to not be an extern symbol
			     * by turning off the extern bit.
			     */
			    if(object->mh != NULL){
				new_symbols[inew_syms] = symbols[i];
				new_symbols[inew_syms].n_type &= ~N_EXT;
				new_symbols[inew_syms].n_desc &= ~N_WEAK_DEF;
			    }
			    else{
				new_symbols64[inew_syms] = symbols64[i];
				new_symbols64[inew_syms].n_type &= ~N_EXT;
				new_symbols64[inew_syms].n_desc &= ~N_WEAK_DEF;
			    }
			    if(n_strx != 0){
				strcpy(q, strings + n_strx);
				if(object->mh != NULL)
				    new_symbols[inew_syms].n_un.n_strx =
					(uint32_t)(q - new_strings);
				else
				    new_symbols64[inew_syms].n_un.n_strx =
					(uint32_t)(q - new_strings);
				q += strlen(q) + 1;
			    }
			    inew_syms++;
			    saves[i] = inew_syms;
			}
		    }
		}
	    }
	    /*
	     * Last put the unchanged global symbols into the new symbol table
	     * and symbols changed into private externs.
	     */
	    for(i = 0; i < nsyms; i++){
		if(object->mh != NULL){
		    n_strx = symbols[i].n_un.n_strx;
		    n_type = symbols[i].n_type;
		}
		else{
		    n_strx = symbols64[i].n_un.n_strx;
		    n_type = symbols64[i].n_type;
		}
		if((n_type & N_EXT) != 0){
		    if(nmedits[i] == FALSE || pflag == TRUE){
			if(object->mh != NULL)
			    new_symbols[inew_syms] = symbols[i];
			else
			    new_symbols64[inew_syms] = symbols64[i];
			if(nmedits[i] == TRUE && pflag == TRUE){
			    /*
			     * Change the new symbol to be a private extern
			     * symbol by turning on the private extern bit.
			     */
			    if(object->mh != NULL)
				new_symbols[inew_syms].n_type |= N_PEXT;
			    else
				new_symbols64[inew_syms].n_type |= N_PEXT;
			}
			if(n_strx != 0){
			    strcpy(p, strings + n_strx);
			    if(object->mh != NULL)
				new_symbols[inew_syms].n_un.n_strx =
				    (uint32_t)(p - new_strings);
			    else
				new_symbols64[inew_syms].n_un.n_strx =
				    (uint32_t)(p - new_strings);
			    p += strlen(p) + 1;
			}
			inew_syms++;
			saves[i] = inew_syms;
		    }
		}
	    }
	}

	if(sections != NULL)
	    free(sections);
	if(sections64 != NULL)
	    free(sections64);

	if(errors == 0)
	    return(TRUE);
	else
	    return(FALSE);
}

/*
 * Function for qsort for comparing global symbol names.
 */
static
int
cmp_qsort_global(
const struct nlist **sym1,
const struct nlist **sym2)
{
	return(strcmp(global_strings + (*sym1)->n_un.n_strx,
		      global_strings + (*sym2)->n_un.n_strx));
}

static
int
cmp_qsort_global_64(
const struct nlist_64 **sym1,
const struct nlist_64 **sym2)
{
	return(strcmp(global_strings + (*sym1)->n_un.n_strx,
		      global_strings + (*sym2)->n_un.n_strx));
}

/*
 * Function for bsearch for finding a global symbol that matches a stab name.
 */
static
int
cmp_bsearch_global_stab(
const char *name,
const struct nlist **sym)
{
	/*
	 * The +1 is for the '_' on the global symbol that is not on the
	 * stab string that is trying to be matched.
	 */
	return(strcmp(name, global_strings + (*sym)->n_un.n_strx + 1));
}

static
int
cmp_bsearch_global_stab_64(
const char *name,
const struct nlist_64 **sym)
{
	/*
	 * The +1 is for the '_' on the global symbol that is not on the
	 * stab string that is trying to be matched.
	 */
	return(strcmp(name, global_strings + (*sym)->n_un.n_strx + 1));
}

/*
 * Function for bsearch for finding a global symbol that matches a stab name
 * in the debug map.
 */
static
int
cmp_bsearch_global(
const char *name,
const struct nlist **sym)
{
	return(strcmp(name, global_strings + (*sym)->n_un.n_strx));
}

static
int
cmp_bsearch_global_64(
const char *name,
const struct nlist_64 **sym)
{
	return(strcmp(name, global_strings + (*sym)->n_un.n_strx));
}
#endif /* defined(NMEDIT) */
