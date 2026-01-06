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
 * The lipo(1) program.  This program creates, thins and operates on fat files.
 * This program takes the following options:
 *   <input_file>
 *   -output <filename>
 *   -create
 *   -info
 *   -detailed_info
 *   -arch <arch_type> <input_file>
 *   -arch_blank <arch_type>
 *   -thin <arch_type>
 *   -extract <arch_type>
 *   -remove <arch_type>
 *   -replace <arch_type> <file_name>
 *   -segalign <arch_type> <value>
 *   -verify_arch <arch_type> ...
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ar.h>
#ifndef AR_EFMT1
#define	AR_EFMT1	"#1/"		/* extended format #1 */
#endif
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <libc.h>
#ifndef __OPENSTEP__
#include <time.h>
#include <utime.h>
#endif
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include "stuff/arch.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/lto.h"
#include "stuff/write64.h"
#include "stuff/rnd.h"
#include <math.h>

/* cctools-port start */
#ifndef HAVE_UTIMENS
int utimens(const char *path, const struct timespec times[2]);
#endif
/* cctools-port end */

/* The maximum section alignment allowed to be specified, as a power of two */
#define MAXSECTALIGN		15 /* 2**15 or 0x8000 */

/* These #undef's are because of the #define's in <mach.h> */
#undef TRUE
#undef FALSE

/* name of the program for error messages (argv[0]) */
char *progname = NULL;

/* names and types (if any) of input file specified on the commmand line */
struct input_file {
    char *name;
    struct arch_flag arch_flag;
    struct fat_header *fat_header;
    struct fat_arch *fat_arches;
    struct fat_arch_64 *fat_arches64;
    enum bool is_thin;
    uint32_t raw_nfat_arch;
};
static struct input_file *input_files = NULL;
static uint32_t ninput_files = 0;

/* Thin files from the input files to operate on */
struct thin_file {
    char *name;
    char *addr;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint64_t offset;
    uint64_t size;
    uint32_t align;
    enum bool from_fat;
    enum bool extract;
    enum bool remove;
    enum bool replace;
};
static struct thin_file *thin_files = NULL;
static uint32_t nthin_files = 0;

/* The specified output file */
static char *output_file = NULL;
static uint32_t output_filemode = 0;
#ifndef __OPENSTEP__
static struct timespec output_times[2] = { 0 };
static struct timeval output_timev[2] = { 0 };
#else
static time_t output_timep[2] = { 0 };
#endif
static enum bool archives_in_input = FALSE;

/* flags set from command line arguments to specify the operation */
static enum bool create_flag = FALSE;
static enum bool info_flag = FALSE;
static enum bool detailed_info_flag = FALSE;
static enum bool brief_info_flag = FALSE; // -archs

static enum bool thin_flag = FALSE;
static struct arch_flag thin_arch_flag = { 0 };

static enum bool remove_flag = FALSE;
static struct arch_flag *remove_arch_flags = NULL;
static uint32_t nremove_arch_flags = 0;

static enum bool extract_flag = FALSE;
static struct arch_flag *extract_arch_flags = NULL;
static uint32_t nextract_arch_flags = 0;
static enum bool extract_family_flag = FALSE;

static enum bool replace_flag = FALSE;
struct replace {
    struct arch_flag arch_flag;
    struct thin_file thin_file;
};
static struct replace *replaces = NULL;
static uint32_t nreplaces = 0;

struct segalign {
    struct arch_flag arch_flag;
    uint32_t align;
};
static struct segalign *segaligns = NULL;
static uint32_t nsegaligns = 0;

static enum bool arch_blank_flag = FALSE;

static struct fat_header fat_header = { 0 };

static enum bool verify_flag = FALSE;
static struct arch_flag *verify_archs = NULL;
static uint32_t nverify_archs = 0;

static enum bool fat64_flag = FALSE;

static enum bool hideARM64_flag = FALSE;

static void create_fat(
    void);
static void process_input_file(
    struct input_file *input);
static void process_replace_file(
    struct replace *replace);
static void check_archive(
    char *name,
    char *addr,
    uint64_t size,
    cpu_type_t *cputype,
    cpu_subtype_t *cpusubtype);
static void check_extend_format_1(
    char *name,
    struct ar_hdr *ar_hdr,
    uint64_t size_left,
    uint32_t *member_name_size);
static uint32_t get_mh_filetype(
    char* addr,
    uint64_t size);
static uint32_t get_align(
    struct mach_header *mhp,
    struct load_command *load_commands,
    uint64_t size,
    char *name,
    enum bool swapped);
static uint32_t get_align_64(
    struct mach_header_64 *mhp64,
    struct load_command *load_commands,
    uint64_t size,
    char *name,
    enum bool swapped);
static uint32_t guess_align(
    uint64_t vmaddr);
static void print_arch(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);
static void print_cputype(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);
static int size_ar_name(
    char *ar_name);
static struct input_file *new_input(
    void);
static struct thin_file *new_thin(
    void);
static struct arch_flag *new_arch_flag(
    struct arch_flag **arch_flags,
    uint32_t *narch_flags);
static struct replace *new_replace(
    void);
static struct segalign *new_segalign(
    void);
static int cmp_qsort(
    const struct thin_file *thin1,
    const struct thin_file *thin2);
static enum bool ispoweroftwo(
    uint32_t x);
static void check_arch(
    struct input_file *input,
    struct thin_file *thin);
static void usage(
    void);
static struct thin_file *new_blank_dylib(
    struct arch_flag *arch);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int
main(
int argc,
char *argv[],
char *envp[])
{
    int fd, a;
    uint32_t i, j, k, value;
    char *p, *endp;
    struct input_file *input;
    struct arch_flag *arch_flag;
    struct replace *replace;
    struct segalign *segalign;
    const struct arch_flag *arch_flags;
    enum bool found;
    struct arch_flag blank_arch;
    int time_result;

	input = NULL;
	/*
	 * Process the command line arguments.
	 */
	progname = argv[0];
	for(a = 1; a < argc; a++){
	    if(argv[a][0] == '-'){
		p = &(argv[a][1]);
		switch(*p){
		case 'a':
		    if(strcmp(p, "arch") == 0 || strcmp(p, "a") == 0){
			if(a + 2 >= argc){
			    error("missing argument(s) to %s option", argv[a]);
			    usage();
			}
			input = new_input();
			if(get_arch_from_flag(argv[a+1],
					      &(input->arch_flag)) == 0){
			    error("unknown architecture specification flag: %s "
				  "in specifying input file %s %s %s", argv[a+1],
				  argv[a], argv[a+1], argv[a+2]);
			    arch_usage();
			    usage();
			}
			input->name = argv[a+2];
			a += 2;
		    }
		    else if(strcmp(p, "arch_blank") == 0){
			arch_blank_flag = TRUE;
			if(a + 1 >= argc){
			    error("missing argument(s) to %s option", argv[a]);
			    usage();
			}
			if(get_arch_from_flag(argv[a+1], &blank_arch) == 0){
			    error("unknown architecture specification flag: %s "
				  "in specifying input file %s %s", argv[a+1],
				  argv[a], argv[a+1]);
			    arch_usage();
			    usage();
			}
			new_blank_dylib(&blank_arch);
			a += 1;
		    }
		    else if(strcmp(p, "archs") == 0){
			brief_info_flag = TRUE;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'c':
		    if(strcmp(p, "create") == 0 || strcmp(p, "c") == 0){
			create_flag = TRUE;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'd':
		    if(strcmp(p, "detailed_info") == 0 || strcmp(p, "d") == 0){
			detailed_info_flag = TRUE;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'e':
		    if(strcmp(p, "extract") == 0 ||
		       strcmp(p, "extract_family") == 0 ||
		       strcmp(p, "e") == 0){
			extract_flag = TRUE;
			if(strcmp(p, "extract_family") == 0)
			    extract_family_flag = TRUE;
			if(a + 1 >= argc){
			    error("missing argument to %s option", argv[a]);
			    usage();
			}
			arch_flag = new_arch_flag(&extract_arch_flags,
						&nextract_arch_flags);
			if(get_arch_from_flag(argv[a+1], arch_flag) == 0){
			    error("unknown architecture specification flag: "
				  "%s in specifying extract operation: %s %s",
				  argv[a+1], argv[a], argv[a+1]);
			    arch_usage();
			    usage();
			}
			a++;
		    }
		    else
			goto unknown_flag;
		    break;
                case 'h':
                    if (strcmp(p, "hideARM64") == 0) {
                        hideARM64_flag = TRUE;
                    }
                    else
                        goto unknown_flag;
                    break;
		case 'i':
		    if(strcmp(p, "info") == 0 || strcmp(p, "i") == 0){
			info_flag = TRUE;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'o':
		    if(strcmp(p, "output") == 0 || strcmp(p, "o") == 0){
			if(a + 1 >= argc){
			    error("missing argument to %s option", argv[a]);
			    usage();
			}
			if(output_file != NULL)
			    fatal("more than one %s option specified", argv[a]);
			output_file = argv[a + 1];
			a++;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'r':
		    if(strcmp(p, "remove") == 0 || strcmp(p, "rem") == 0){
			remove_flag = TRUE;
			if(a + 1 >= argc){
			    error("missing argument to %s option", argv[a]);
			    usage();
			}
			arch_flag = new_arch_flag(&remove_arch_flags,
						&nremove_arch_flags);
			if(get_arch_from_flag(argv[a+1], arch_flag) == 0){
			    error("unknown architecture specification flag: "
				  "%s in specifying remove operation: %s %s",
				  argv[a+1], argv[a], argv[a+1]);
			    arch_usage();
			    usage();
			}
			a++;
		    }
		    else if(strcmp(p, "replace") == 0 || strcmp(p, "rep") == 0){
			replace_flag = TRUE;
			if(a + 2 >= argc){
			    error("missing argument(s) to %s option", argv[a]);
			    usage();
			}
			replace = new_replace();
			if(get_arch_from_flag(argv[a+1],
					      &(replace->arch_flag)) == 0){
			    error("unknown architecture specification flag: "
				  "%s in specifying replace operation: %s %s %s",
				  argv[a+1], argv[a], argv[a+1], argv[a+2]);
			    arch_usage();
			    usage();
			}
			replace->thin_file.name = argv[a+2];
			a += 2;
		    }
		    else
			goto unknown_flag;
		    break;
		case 's':
		    if(strcmp(p, "segalign") == 0 || strcmp(p, "s") == 0){
			if(a + 2 >= argc){
			    error("missing argument(s) to %s option", argv[a]);
			    usage();
			}
			segalign = new_segalign();
			if(get_arch_from_flag(argv[a+1],
					      &(segalign->arch_flag)) == 0){
			    error("unknown architecture specification flag: "
				  "%s in specifying segment alignment: %s %s %s",
				  argv[a+1], argv[a], argv[a+1], argv[a+2]);
			    arch_usage();
			    usage();
			}
			value = (uint32_t)strtoul(argv[a+2], &endp, 16);
			if(*endp != '\0')
			    fatal("argument for -segalign <arch_type> %s not a "
				  "proper hexadecimal number", argv[a+2]);
			if(!ispoweroftwo(value) || value == 0)
			    fatal("argument to -segalign <arch_type> %x (hex) "
				  "must be a non-zero power of two", value);
			if(value > (1 << MAXSECTALIGN))
			    fatal("argument to -segalign <arch_type> %x (hex) "
				  "must equal to or less than %x (hex)",
				  value, (unsigned int)(1 << MAXSECTALIGN));
			segalign->align = 0;
			while((value & 0x1) != 1){
			    value >>= 1;
			    segalign->align++;
			}
			a += 2;
		    }
		    else
			goto unknown_flag;
		    break;
		case 't':
		    if(strcmp(p, "thin") == 0 || strcmp(p, "t") == 0){
			if(thin_flag == TRUE)
			    fatal("more than one %s option specified", argv[a]);
			thin_flag = TRUE;
			if(a + 1 >= argc){
			    error("missing argument to %s option", argv[a]);
			    usage();
			}
			if(get_arch_from_flag(argv[a+1], &thin_arch_flag) == 0){
			    error("unknown architecture specification flag: "
				  "%s in specifying thin operation: %s %s",
				  argv[a+1], argv[a], argv[a+1]);
			    arch_usage();
			    usage();
			}
			a++;
		    }
		    else
			goto unknown_flag;
		    break;
		case 'v':
		    if(strcmp(p, "verify_arch") == 0){
			verify_flag = TRUE;
			if(a + 1 >= argc){
			    error("missing argument(s) to %s option", argv[a]);
			    usage();
			}
			a++;
			nverify_archs = argc - a;
			verify_archs = (struct arch_flag *)allocate(
			    sizeof(struct arch_flag) * nverify_archs);
			for(i = 0; a < argc; a++, i++){
			    if(get_arch_from_flag(argv[a],
			       verify_archs + i) == 0){
				error("unknown architecture specification "
				      "flag: %s in specifying -verify_arch "
				      "operation", argv[a]);
				arch_usage();
				usage();
			    }
			}
		    }
		    else
			goto unknown_flag;
		    break;
		case 'f':
		    if(strcmp(p, "fat64") == 0)
			fat64_flag = TRUE;
		    else
			goto unknown_flag;
		    break;
		default:
unknown_flag:
		    fatal("unknown flag: %s", argv[a]);
		}
	    }
	    else{
		input = new_input();
		input->name = argv[a];
	    }
	}

	/*
	 * Check to see the specified arguments are valid.
	 */
	if(info_flag == FALSE && detailed_info_flag == FALSE &&
	   brief_info_flag == FALSE &&
	   create_flag == FALSE && thin_flag == FALSE &&
	   extract_flag == FALSE && remove_flag == FALSE &&
	   replace_flag == FALSE && verify_flag == FALSE){
	    error("one of -create, -thin <arch_type>, -extract <arch_type>, "
		  "-remove <arch_type>, -replace <arch_type> <file_name>, "
		  "-verify_arch <arch_type> ... , "
		  "-archs, -info, or -detailed_info must be specified");
	    usage();
	}
	if((create_flag == TRUE || thin_flag == TRUE || extract_flag == TRUE ||
	    remove_flag == TRUE || replace_flag == TRUE) &&
	   output_file == NULL){
	    error("no output file specified");
	    usage();
	}
	if(ninput_files == 0){
	    error("no input files specified");
	    usage();
	}
	if(verify_flag == TRUE && ninput_files != 1){
	    error("only one input file allowed with -verify_arch");
	    usage();
	}
	if(brief_info_flag == TRUE && ninput_files != 1){
	    error("only one input file allowed with -archs");
	    usage();
	}
	if(create_flag + thin_flag + extract_flag + remove_flag + replace_flag +
	   info_flag + detailed_info_flag + brief_info_flag + verify_flag > 1){
	    error("only one of -create, -thin <arch_type>, -extract <arch_type>"
		  ", -remove <arch_type>, -replace <arch_type> <file_name>, "
		  "-verify_arch <arch_type> ..., "
		  "-info, -archs, or -detailed_info can be specified");
	    usage();
	}
	if(arch_blank_flag == TRUE && create_flag == FALSE){
		error("-arch_blank may only be used with -create");
		usage();
	}
        if (hideARM64_flag == TRUE && create_flag == FALSE &&
            replace_flag == FALSE && remove_flag == FALSE) {
            error("-hideARM64 may only be used with -create, -remove, or "
                  "-replace");
            usage();
        }

	/*
	 * Determine the types of the input files.
	 */
	for(i = 0; i < ninput_files; i++)
	    process_input_file(input_files + i);

	/*
	 * Do the specified operation.
	 */

	if(create_flag){
	    /* check to make sure no two files have the same architectures */
	    for(i = 0; i < nthin_files; i++)
		for(j = i + 1; j < nthin_files; j++)
		    if(thin_files[i].cputype == 
		       thin_files[j].cputype && 
		       (thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK)==
		       (thin_files[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			arch_flags = get_arch_flags();
			for(k = 0; arch_flags[k].name != NULL; k++){
			    if(arch_flags[k].cputype ==
			       thin_files[j].cputype && 
			       (arch_flags[k].cpusubtype &
				~CPU_SUBTYPE_MASK) ==
			       (thin_files[j].cpusubtype &
				~CPU_SUBTYPE_MASK))
			    fatal("%s and %s have the same architectures (%s) "
			      "and can't be in the same fat output file",
			      thin_files[i].name, thin_files[j].name,
			      arch_flags[k].name);
			}
			fatal("%s and %s have the same architectures (cputype "
			      "(%d) and cpusubtype (%d)) and can't be in the "
			      "same fat output file", thin_files[i].name,
			      thin_files[j].name,thin_files[i].cputype,
			      thin_files[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
		    }
	    create_fat();
	}

	if(thin_flag){
	    if(ninput_files != 1)
		fatal("only one input file can be specified with the -thin "
		      "option");
	    if(input_files[0].fat_header == NULL)
		fatal("input file (%s) must be a fat file when the -thin "
		      "option is specified", input_files[0].name);
	    for(i = 0; i < nthin_files; i++){
		if(thin_files[i].cputype == thin_arch_flag.cputype &&
		   (thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   (thin_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK)){
		    (void)unlink(output_file);
		    if((fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC,
				  output_filemode)) == -1)
			system_fatal("can't create output file: %s",
				     output_file);

                    if (write64(fd, thin_files[i].addr, thin_files[i].size) !=
                        thin_files[i].size)
                        system_fatal("can't write thin file to output "
                                     "file: %s", output_file);
		    if(close(fd) == -1)
			system_fatal("can't close output file: %s",output_file);
#ifndef __OPENSTEP__
			/* cctools-port: Replaced for portability. */
#if 0
		    if (__builtin_available(macOS 10.12, *)) {
			time_result = utimensat(AT_FDCWD, output_file,
						output_times, 0);
		    }
		    else {
			time_result = utimes(output_file, output_timev);
		    }
#endif
			time_result = utimens(output_file, output_times);
#else
		    time_result = utime(output_file, output_timep);
#endif
		    if (time_result == -1)
			system_fatal("can't set the modify times for "
				     "output file: %s", output_file);
		    break;
		}
	    }
	    if(i == nthin_files)
		fatal("fat input file (%s) does not contain the specified "
		      "architecture (%s) to thin it to", input->name,
		      thin_arch_flag.name);
	}

	if(extract_flag){
	    if(ninput_files != 1)
		fatal("only one input file can be specified with the -extract "
		      "option");
	    if(input_files[0].fat_header == NULL)
		fatal("input file (%s) must be a fat file when the -extract "
		      "option is specified", input_files[0].name);
	    if(input_files[0].fat_header->magic == FAT_MAGIC_64)
		fat64_flag = TRUE;

	    for(i = 0; i < nextract_arch_flags; i++){
		for(j = i + 1; j < nextract_arch_flags; j++){
		if(extract_arch_flags[i].cputype ==
		       extract_arch_flags[j].cputype &&
		   (extract_arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (extract_arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK))
		    fatal("-extract %s specified multiple times", 
			  extract_arch_flags[i].name);
		}
	    }
	    /* mark those thin files for extraction */
	    for(i = 0; i < nextract_arch_flags; i++){
		found = FALSE;
		for(j = 0; j < nthin_files; j++){
		    if(extract_arch_flags[i].cputype ==
			   thin_files[j].cputype &&
		       ((extract_arch_flags[i].cpusubtype &
			 ~CPU_SUBTYPE_MASK)==
		        (thin_files[j].cpusubtype &
			 ~CPU_SUBTYPE_MASK) ||
			extract_family_flag == TRUE)){
			thin_files[j].extract = TRUE;
			found = TRUE;
		    }
		}
		if(found == FALSE)
		    fatal("-extract %s specified but fat file: %s does not "
			  "contain that architecture",
			  extract_arch_flags[i].name, input_files[0].name);
	    }
	    /* remove those thin files not marked for extraction */
	    for(i = 0; i < nthin_files; ){
		if(thin_files[i].extract == FALSE){
		    for(j = i; j < nthin_files - 1; j++)
			thin_files[j] = thin_files[j + 1];
		    nthin_files--;
		}
		else
		    i++;
	    }
	    create_fat();
	}

	if(remove_flag){
	    if(ninput_files != 1)
		fatal("only one input file can be specified with the -remove "
		      "option");
	    if(input_files[0].fat_header == NULL)
		fatal("input file (%s) must be a fat file when the -remove "
		      "option is specified", input_files[0].name);
	    if(input_files[0].fat_header->magic == FAT_MAGIC_64)
		fat64_flag = TRUE;
	    for(i = 0; i < nremove_arch_flags; i++){
		for(j = i + 1; j < nremove_arch_flags; j++){
		if(remove_arch_flags[i].cputype ==
		       remove_arch_flags[j].cputype &&
		   (remove_arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (remove_arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK))
		    fatal("-remove %s specified multiple times", 
			  remove_arch_flags[i].name);
		}
	    }
	    /* mark those thin files for removal */
	    for(i = 0; i < nremove_arch_flags; i++){
		for(j = 0; j < nthin_files; j++){
		    if(remove_arch_flags[i].cputype ==
			   thin_files[j].cputype &&
		       (remove_arch_flags[i].cpusubtype &
			~CPU_SUBTYPE_MASK) ==
		       (thin_files[j].cpusubtype &
			~CPU_SUBTYPE_MASK)){
			thin_files[j].remove = TRUE;
			break;
		    }
		}
		if(j == nthin_files)
		    fatal("-remove %s specified but fat file: %s does not "
			  "contain that architecture",
			  remove_arch_flags[i].name, input_files[0].name);
	    }
	    /* remove those thin files marked for removal */
	    for(i = 0; i < nthin_files; ){
		if(thin_files[i].remove == TRUE){
		    for(j = i; j < nthin_files; j++)
			thin_files[j] = thin_files[j + 1];
		    nthin_files--;
		}
		else
		    i++;
	    }
	    if(nthin_files == 0)
		fatal("-remove's specified would result in an empty fat file");
	    create_fat();
	}

	if(replace_flag){
	    if(ninput_files != 1)
		fatal("only one input file can be specified with the -replace "
		      "option");
	    if(input_files[0].fat_header == NULL)
		fatal("input file (%s) must be a fat file when the -replace "
		      "option is specified", input_files[0].name);
	    if(input_files[0].fat_header->magic == FAT_MAGIC_64)
		fat64_flag = TRUE;
	    for(i = 0; i < nreplaces; i++){
		for(j = i + 1; j < nreplaces; j++){
		if(replaces[i].arch_flag.cputype ==
		       replaces[j].arch_flag.cputype &&
		   (replaces[i].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (replaces[j].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK))
		    fatal("-replace %s <file_name> specified multiple times", 
			  replaces[j].arch_flag.name);
		}
	    }
	    for(i = 0; i < nreplaces; i++){
		process_replace_file(replaces + i);
		for(j = 0; j < nthin_files; j++){
		    if(replaces[i].arch_flag.cputype ==
			   thin_files[j].cputype &&
		       (replaces[i].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (thin_files[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			thin_files[j] = replaces[i].thin_file;
			break;
		    }
		}
		if(j == nthin_files)
		    fatal("-replace %s <file_name> specified but fat file: %s "
			  "does not contain that architecture",
			  replaces[i].arch_flag.name, input_files[0].name);
	    }
	    create_fat();
	}

	if (brief_info_flag) {
	    for (i = 0; i < nthin_files; i++) {
		const char* s = get_arch_name_if_known(
				    thin_files[i].cputype,
				    thin_files[i].cpusubtype);
		if (i) {
		    printf(" ");
		}
		if (s) {
		    printf("%s", s);
		}
		else {
		    printf("unknown(%u,%u)", thin_files[i].cputype,
			   thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK);
		}
	    }
	    printf("\n");
	}

	if(info_flag){
	    for(i = 0; i < ninput_files; i++){
		if(input_files[i].fat_header != NULL){
		    printf("Architectures in the fat file: %s are: ",
			   input_files[i].name);
		    if(input_files[i].fat_arches != NULL){
			for(j = 0; j < input_files[i].fat_header->nfat_arch;
			    j++){
			    print_arch(input_files[i].fat_arches[j].cputype,
				input_files[i].fat_arches[j].cpusubtype);
			    printf(" ");
			}
		    }
		    else{
			for(j = 0; j < input_files[i].fat_header->nfat_arch;
			    j++){
			    print_arch(input_files[i].fat_arches64[j].cputype,
				input_files[i].fat_arches64[j].cpusubtype);
			    printf(" ");
			}
		    }
		    printf("\n");
		}
		else{
		    if(input_files[i].is_thin == FALSE)
			printf("input file %s is not a fat file\n",
			       input_files[i].name);
		}
	    }
	    for(i = 0; i < nthin_files; i++){
		if(thin_files[i].from_fat == TRUE)
		    continue;
		printf("Non-fat file: %s is architecture: %s\n",
		       thin_files[i].name,
		       get_arch_name_from_types(thin_files[i].cputype,
			thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK));
	    }
	}

	if(detailed_info_flag){
	    for(i = 0; i < ninput_files; i++){
		if(input_files[i].fat_header != NULL){
		    printf("Fat header in: %s\n", input_files[i].name);
		    printf("fat_magic 0x%x\n",
			  (unsigned int)(input_files[i].fat_header->magic));
		    printf("nfat_arch %u",
			   input_files[i].raw_nfat_arch);
                    if (input_files[i].fat_header->nfat_arch -
                        input_files[i].raw_nfat_arch) {
                        printf(" (+%u hidden)",
                               input_files[i].fat_header->nfat_arch -
                               input_files[i].raw_nfat_arch);
                    }
                    printf("\n");
		    for(j = 0; j < input_files[i].fat_header->nfat_arch; j++){
			printf("architecture ");
			if(input_files[i].fat_arches != NULL){
			    print_arch(input_files[i].fat_arches[j].cputype,
				       input_files[i].fat_arches[j].cpusubtype);
                            if (j >= input_files[i].raw_nfat_arch) {
                                printf(" (hidden)");
                            }
			    printf("\n");
			    print_cputype(input_files[i].fat_arches[j].cputype,
				      input_files[i].fat_arches[j].cpusubtype &
					  ~CPU_SUBTYPE_MASK);
			    printf("    offset %u\n",
				   input_files[i].fat_arches[j].offset);
			    printf("    size %u\n",
				   input_files[i].fat_arches[j].size);
			    printf("    align 2^%u (%d)\n",
				   input_files[i].fat_arches[j].align,
				   1 << input_files[i].fat_arches[j].align);
			}
			else{
			    print_arch(input_files[i].fat_arches64[j].cputype,
				   input_files[i].fat_arches64[j].cpusubtype);
                            if (j >= input_files[i].raw_nfat_arch) {
                                printf(" (hidden)");
                            }
			    printf("\n");
			    print_cputype(
				input_files[i].fat_arches64[j].cputype,
				input_files[i].fat_arches64[j].cpusubtype &
					  ~CPU_SUBTYPE_MASK);
			    printf("    offset %llu\n",
				   input_files[i].fat_arches64[j].offset);
			    printf("    size %llu\n",
				   input_files[i].fat_arches64[j].size);
			    printf("    align 2^%u (%d)\n",
				   input_files[i].fat_arches64[j].align,
				   1 << input_files[i].fat_arches64[j].align);
			}
		    }
		}
		else{
		    printf("input file %s is not a fat file\n",
			   input_files[i].name);
		}
	    }
	    for(i = 0; i < nthin_files; i++){
		if(thin_files[i].from_fat == TRUE)
		    continue;
		printf("Non-fat file: %s is architecture: %s\n",
		       thin_files[i].name,
		       get_arch_name_from_types(thin_files[i].cputype,
			thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK));
	    }
	}

	if(verify_flag == TRUE){
	    for(i = 0; i < nverify_archs; i++){
		found = FALSE;
		for(j = 0; j < nthin_files; j++){
		    if(verify_archs[i].cputype == 
		       thin_files[j].cputype && 
		       (verify_archs[i].cpusubtype & ~CPU_SUBTYPE_MASK) == 
		       (thin_files[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			found = TRUE;
			break;
		    }
		}
		if(found == FALSE)
		    exit(1);
	    }
	}

	return(0);
}

/*
 * create_fat() creates a fat output file from the thin files.
 */
static
void
create_fat(void)
{
    uint32_t i, j;
    uint64_t offset;
    char *rename_file;
    int fd;
    struct fat_arch fat_arch;
    struct fat_arch_64 fat_arch64;

	/* fold in specified segment alignments */
	for(i = 0; i < nsegaligns; i++){
	    for(j = i + 1; j < nsegaligns; j++){
	    if(segaligns[i].arch_flag.cputype ==
		   segaligns[j].arch_flag.cputype &&
	       (segaligns[i].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   (segaligns[j].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK))
		fatal("-segalign %s <value> specified multiple times", 
		      segaligns[j].arch_flag.name);
	    }
	}
	for(i = 0; i < nsegaligns; i++){
	    for(j = 0; j < nthin_files; j++){
		if(segaligns[i].arch_flag.cputype ==
		       thin_files[j].cputype &&
		   (segaligns[i].arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (thin_files[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
/*
 Since this program has to guess at alignments and guesses high when unsure this
 check shouldn't be used so the the correct alignment can be specified by the
 the user.
		    if(thin_files[j].align > segaligns[i].align)
			fatal("specified segment alignment: %d for "
			      "architecture %s is less than the alignment: %d "
			      "required from the input file",
			      1 << segaligns[i].align,
			      segaligns[i].arch_flag.name,
			      1 << thin_files[j].align);
*/
		    thin_files[j].align = segaligns[i].align;
		    break;
		}
	    }
	    if(j == nthin_files)
		fatal("-segalign %s <value> specified but resulting fat "
		      "file does not contain that architecture",
		      segaligns[i].arch_flag.name);
	}

	/* sort the files by alignment to save space in the output file */
	qsort(thin_files, nthin_files, sizeof(struct thin_file),
	      (int (*)(const void *, const void *))cmp_qsort);
    
	/* Fill in the fat header and the fat_arch's offsets. */
	if(fat64_flag == TRUE)
	    fat_header.magic = FAT_MAGIC_64;
	else
	    fat_header.magic = FAT_MAGIC;
	fat_header.nfat_arch = nthin_files;
        /* begin change for 15002326: write a hidden arm64 arch */
        if (hideARM64_flag) {
            enum bool has_arm32 = FALSE;
            enum bool has_arm64 = FALSE;
            uint32_t num_archs = 0;
            uint32_t num_archs_arm64 = 0;
            
            for(i = 0; i < nthin_files; i++){
                if (MH_EXECUTE != get_mh_filetype(thin_files[i].addr,
                                                  thin_files[i].size)) {
                    fatal("-hideARM64 specified but thin file %s is not of "
                          "type MH_EXECUTE", thin_files[i].name);
                }
                if (has_arm64 && thin_files[i].cputype != CPU_TYPE_ARM64) {
                    fatal("-hideARM64 specified but thin files are not in "
                          "correct order");
                }
                if (thin_files[i].cputype == CPU_TYPE_ARM) {
                    has_arm32 = TRUE;
                }
                if (thin_files[i].cputype == CPU_TYPE_ARM64) {
                    has_arm64 = TRUE;
                    num_archs_arm64 += 1;
                }
                num_archs += 1;
            }
            if (has_arm32 && has_arm64) {
                fat_header.nfat_arch = num_archs - num_archs_arm64;
            }
        }
        /* end change for 15002326 */
	offset = sizeof(struct fat_header);
	if(fat64_flag == TRUE)
	    offset += nthin_files * sizeof(struct fat_arch_64);
	else
	    offset += nthin_files * sizeof(struct fat_arch);
	for(i = 0; i < nthin_files; i++){
	    offset = rnd(offset, 1 << thin_files[i].align);
	    if(fat64_flag == FALSE && offset > UINT32_MAX)
		fatal("fat file too large to be created because the offset "
		      "field in struct fat_arch is only 32-bits and the offset "
		      "(%llu) for %s for architecture %s exceeds that",
		      offset, thin_files[i].name,
		      get_arch_name_from_types(thin_files[i].cputype,
			thin_files[i].cpusubtype & ~CPU_SUBTYPE_MASK));
	    thin_files[i].offset = offset;
	    offset += thin_files[i].size;
	}

	rename_file = makestr(output_file, ".lipo", NULL);
	if((fd = open(rename_file, O_WRONLY | O_CREAT | O_TRUNC,
		      output_filemode)) == -1)
	    system_fatal("can't create temporary output file: %s", rename_file);

	/*
	 * If this is an extract_family_flag operation and the is just one
	 * thin file on the list don't create a fat file.
	 */
	if(extract_family_flag != TRUE || nthin_files != 1){
#ifdef __LITTLE_ENDIAN__
	    swap_fat_header(&fat_header, BIG_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    if(write64(fd, &fat_header, sizeof(struct fat_header)) !=
	       sizeof(struct fat_header))
		system_fatal("can't write fat header to output file: %s",
			     rename_file);
#ifdef __LITTLE_ENDIAN__
	    swap_fat_header(&fat_header, LITTLE_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    for(i = 0; i < nthin_files; i++){
		if(fat64_flag == TRUE){
		    fat_arch64.cputype = thin_files[i].cputype;
		    fat_arch64.cpusubtype = thin_files[i].cpusubtype;
		    fat_arch64.offset = thin_files[i].offset;
		    fat_arch64.size = thin_files[i].size;
		    fat_arch64.align = thin_files[i].align;
		}
		else{
		    fat_arch.cputype = thin_files[i].cputype;
		    fat_arch.cpusubtype = thin_files[i].cpusubtype;
		    fat_arch.offset = (uint32_t)thin_files[i].offset;
		    fat_arch.size = (uint32_t)thin_files[i].size;
		    fat_arch.align = thin_files[i].align;
		}
		if(fat64_flag == TRUE){
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch_64(&fat_arch64, 1, BIG_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
		    if(write64(fd, &fat_arch64, sizeof(struct fat_arch_64)) !=
		       sizeof(struct fat_arch_64))
			system_fatal("can't write fat arch to output file: %s",
				     rename_file);
		}
		else{
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch(&fat_arch, 1, BIG_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
		    if(write64(fd, &fat_arch, sizeof(struct fat_arch)) !=
		       sizeof(struct fat_arch))
			system_fatal("can't write fat arch to output file: %s",
				     rename_file);
		}
	    }
	}

	for(i = 0; i < nthin_files; i++){
	    if(extract_family_flag == FALSE || nthin_files > 1)
		if(lseek(fd, thin_files[i].offset, L_SET) == -1)
		    system_fatal("can't lseek in output file: %s", rename_file);
            if(write64(fd, thin_files[i].addr, thin_files[i].size) !=
               thin_files[i].size)
                system_fatal("can't write to output file: %s", rename_file);
	}
	if(close(fd) == -1)
	    system_fatal("can't close output file: %s", rename_file);
	if(rename(rename_file, output_file) == -1)
	    system_error("can't move temporary file: %s to file: %s",
			 output_file, rename_file);
	free(rename_file);
}

/*
 * process_input_file() checks input file and breaks it down into thin files
 * for later operations.
 */
static
void
process_input_file(
struct input_file *input)
{
    int fd;
    struct stat stat_buf, stat_buf2;
    uint32_t i, j;
    uint64_t size;
    char *addr;
    struct thin_file *thin;
    struct mach_header *mhp, mh;
    struct mach_header_64 *mhp64, mh64;
    struct load_command *lcp;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    enum bool swapped;
    uint64_t big_size;
    uint32_t offset, first_offset;

	/* Open the input file and map it in */
	if((fd = open(input->name, O_RDONLY)) == -1)
	    system_fatal("can't open input file: %s", input->name);
	if(fstat(fd, &stat_buf) == -1)
	    system_fatal("can't stat input file: %s", input->name);
	size = stat_buf.st_size;
	/* pick up set uid, set gid and sticky text bits */
	output_filemode = stat_buf.st_mode & 07777;
#ifndef __OPENSTEP__
	/*
	 * Select the first modify time
	 */
	/* cctools-port: Replaced for portability. */
#if 0
	if (__builtin_available(macOS 10.12, *)) {
	    if (output_times[1].tv_sec == 0) {
		memcpy(&output_times[0], &stat_buf.st_atimespec,
		       sizeof(struct timespec));
		memcpy(&output_times[1], &stat_buf.st_mtimespec,
		       sizeof(struct timespec));
	    }
	} else {
	    if (output_timev[1].tv_sec == 0) {
		TIMESPEC_TO_TIMEVAL(&output_timev[0], &stat_buf.st_atimespec);
		TIMESPEC_TO_TIMEVAL(&output_timev[1], &stat_buf.st_mtimespec);
	    }
	}
#endif 
	/* cctools-port start */
#ifdef HAVE_STAT_ST_MTIMESPEC
	output_times[0] = stat_buf.st_atimespec;
	output_times[1] = stat_buf.st_mtimespec;
#elif HAVE_STAT_ST_MTIM
	output_times[0] = stat_buf.st_atim;
	output_times[1] = stat_buf.st_mtim;
#else
	output_times[0].tv_sec = stat_buf.st_atime;
	output_times[0].tv_nsec = 0;
	output_times[1].tv_sec = stat_buf.st_mtime;
	output_times[0].tv_nsec = 0;
#endif
	/* cctools-port end */
#else
	/*
	 * Select the eariliest modify time so that if the output file
	 * contains archives with table of contents lipo will not make them
	 * out of date.  This logic however could make an out of date table of
	 * contents appear up todate if another file is combined with it that
	 * has a date early enough.
	 */
	if(output_timep[1] == 0 || output_timep[1] > stat_buf.st_mtime){
	    output_timep[0] = stat_buf.st_atime;
	    output_timep[1] = stat_buf.st_mtime;
	}
#endif
	/*
	 * mmap() can't handle mapping regular files with zero size.  So this
	 * is handled separately.
	 */
	if((stat_buf.st_mode & S_IFREG) == S_IFREG && size == 0)
	    addr = NULL;
	else
	    addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE,
			fd, 0);
	if((intptr_t)addr == -1)
	    system_fatal("can't map input file: %s", input->name);

	/*
	 * Because of rdar://8087586 we do a second stat to see if the file
	 * is still there and the same file.
	 */
	if(fstat(fd, &stat_buf2) == -1)
	    system_fatal("can't stat input file: %s", input->name);
	if(stat_buf2.st_size != size ||
	   stat_buf2.st_mtime != stat_buf.st_mtime)
	    system_fatal("Input file: %s changed since opened", input->name);

	close(fd);

	/* Try to figure out what kind of file this is */

	/* see if this file is a 32-bit fat file */
	if(size >= sizeof(struct fat_header) &&
#ifdef __BIG_ENDIAN__
	   *((uint32_t *)addr) == FAT_MAGIC)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	   *((uint32_t *)addr) == SWAP_INT(FAT_MAGIC))
#endif /* __LITTLE_ENDIAN__ */
	{

	    if(input->arch_flag.name != NULL)
		fatal("architecture specifed for fat input file: %s "
		      "(architectures can't be specifed for fat input files)",
		      input->name);

	    input->fat_header = (struct fat_header *)addr;
#ifdef __LITTLE_ENDIAN__
	    swap_fat_header(input->fat_header, LITTLE_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    big_size = input->fat_header->nfat_arch;
	    big_size *= sizeof(struct fat_arch);
	    big_size += sizeof(struct fat_header);
	    if(big_size > size)
		fatal("truncated or malformed fat file (fat_arch structs would "
		      "extend past the end of the file) %s", input->name);
	    input->fat_arches = (struct fat_arch *)
				(addr + sizeof(struct fat_header));
            /*
	     * begin change for 15002326: look for a hidden arm64 arch
	     *
	     * the hidden arch(es) must reside between the existing fat_arch
	     * list and the start of the first file. Also, the hidden arm64
	     * arch(es) must be CPU_TYPE_ARM64, natch.
	     */
	    first_offset = 0xFFFFFFFF;
	    input->raw_nfat_arch = input->fat_header->nfat_arch;
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		offset = input->fat_arches[i].offset;
#ifdef __LITTLE_ENDIAN__
		cputype = OSSwapInt32(offset);
#endif
		if (offset < first_offset)
		    first_offset = offset;
	    }
            if (big_size + sizeof(struct fat_arch) <= size &&
		big_size + sizeof(struct fat_arch) <= first_offset) {
                i = input->fat_header->nfat_arch;
                cputype = input->fat_arches[i].cputype;
#ifdef __LITTLE_ENDIAN__
                cputype = OSSwapInt32(cputype);
#endif
                if (cputype == CPU_TYPE_ARM64)
                    input->fat_header->nfat_arch += 1;
            }
            /* end change for 15002326 */
#ifdef __LITTLE_ENDIAN__
	    swap_fat_arch(input->fat_arches, input->fat_header->nfat_arch,
			  LITTLE_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		if(input->fat_arches[i].offset + input->fat_arches[i].size >
		   size)
		    fatal("truncated or malformed fat file (offset plus size "
			  "of cputype (%d) cpusubtype (%d) extends past the "
			  "end of the file) %s", input->fat_arches[i].cputype,
			  input->fat_arches[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  input->name);
		if(input->fat_arches[i].align > MAXSECTALIGN)
		    fatal("align (2^%u) too large of fat file %s (cputype (%d)"
			  " cpusubtype (%d)) (maximum 2^%d)",
			  input->fat_arches[i].align, input->name,
			  input->fat_arches[i].cputype,
			  input->fat_arches[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  MAXSECTALIGN);
		if(input->fat_arches[i].offset %
		   (1 << input->fat_arches[i].align) != 0)
		    fatal("offset %u of fat file %s (cputype (%d) cpusubtype "
			  "(%d)) not aligned on its alignment (2^%u)",
			  input->fat_arches[i].offset, input->name,
			  input->fat_arches[i].cputype,
			  input->fat_arches[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  input->fat_arches[i].align);
	    }
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		for(j = i + 1; j < input->fat_header->nfat_arch; j++){
		    if(input->fat_arches[i].cputype ==
		         input->fat_arches[j].cputype &&
		       (input->fat_arches[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		         (input->fat_arches[j].cpusubtype & ~CPU_SUBTYPE_MASK))
		    fatal("fat file %s contains two of the same architecture "
			  "(cputype (%d) cpusubtype (%d))", input->name,
			  input->fat_arches[i].cputype,
			  input->fat_arches[i].cpusubtype & ~CPU_SUBTYPE_MASK);
		}
	    }

	    /* create a thin file struct for each arch in the fat file */
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		thin = new_thin();
		thin->name = input->name;
		thin->addr = addr + input->fat_arches[i].offset;
		thin->cputype = input->fat_arches[i].cputype;
		thin->cpusubtype = input->fat_arches[i].cpusubtype;
		thin->offset = input->fat_arches[i].offset;
		thin->size = input->fat_arches[i].size;
		thin->align = input->fat_arches[i].align;
		thin->from_fat = TRUE;
		if(input->fat_arches[i].size >= SARMAG &&
		   strncmp(thin->addr, ARMAG, SARMAG) == 0)
		    archives_in_input = TRUE;
	    }
	}
	/* see if this file is a 64-bit fat file */
	else if(size >= sizeof(struct fat_header) &&
#ifdef __BIG_ENDIAN__
	   *((uint32_t *)addr) == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	   *((uint32_t *)addr) == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
	{

	    if(input->arch_flag.name != NULL)
		fatal("architecture specifed for fat input file: %s "
		      "(architectures can't be specifed for fat input files)",
		      input->name);

	    input->fat_header = (struct fat_header *)addr;
#ifdef __LITTLE_ENDIAN__
	    swap_fat_header(input->fat_header, LITTLE_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    big_size = input->fat_header->nfat_arch;
	    big_size *= sizeof(struct fat_arch_64);
	    big_size += sizeof(struct fat_header);
	    if(big_size > size)
		fatal("truncated or malformed fat file (fat_arch_64 structs "
		    "would extend past the end of the file) %s", input->name);
	    input->fat_arches64 = (struct fat_arch_64 *)
				(addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
	    swap_fat_arch_64(input->fat_arches64, input->fat_header->nfat_arch,
			     LITTLE_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		if(input->fat_arches64[i].offset + input->fat_arches64[i].size >
		   size)
		    fatal("truncated or malformed fat file (offset plus size "
			  "of cputype (%d) cpusubtype (%d) extends past the "
			  "end of the file) %s", input->fat_arches64[i].cputype,
			  input->fat_arches64[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  input->name);
		if(input->fat_arches64[i].align > MAXSECTALIGN)
		    fatal("align (2^%u) too large of fat file %s (cputype (%d)"
			  " cpusubtype (%d)) (maximum 2^%d)",
			  input->fat_arches64[i].align, input->name,
			  input->fat_arches64[i].cputype,
			  input->fat_arches64[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  MAXSECTALIGN);
		if(input->fat_arches64[i].offset %
		   (1 << input->fat_arches64[i].align) != 0)
		    fatal("offset %llu of fat file %s (cputype (%d) cpusubtype "
			  "(%d)) not aligned on its alignment (2^%u)",
			  input->fat_arches64[i].offset, input->name,
			  input->fat_arches64[i].cputype,
			  input->fat_arches64[i].cpusubtype & ~CPU_SUBTYPE_MASK,
			  input->fat_arches64[i].align);
	    }
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		for(j = i + 1; j < input->fat_header->nfat_arch; j++){
		    if(input->fat_arches64[i].cputype ==
		         input->fat_arches64[j].cputype &&
		       (input->fat_arches64[i].cpusubtype &
			~CPU_SUBTYPE_MASK) ==
		         (input->fat_arches64[j].cpusubtype &
			  ~CPU_SUBTYPE_MASK))
		    fatal("fat file %s contains two of the same architecture "
			  "(cputype (%d) cpusubtype (%d))", input->name,
			  input->fat_arches64[i].cputype,
			  input->fat_arches64[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
		}
	    }

	    /* create a thin file struct for each arch in the fat file */
	    for(i = 0; i < input->fat_header->nfat_arch; i++){
		thin = new_thin();
		thin->name = input->name;
		thin->addr = addr + input->fat_arches64[i].offset;
		thin->cputype = input->fat_arches64[i].cputype;
		thin->cpusubtype = input->fat_arches64[i].cpusubtype;
		thin->offset = input->fat_arches64[i].offset;
		thin->size = input->fat_arches64[i].size;
		thin->align = input->fat_arches64[i].align;
		thin->from_fat = TRUE;
		if(input->fat_arches64[i].size >= SARMAG &&
		   strncmp(thin->addr, ARMAG, SARMAG) == 0)
		    archives_in_input = TRUE;
	    }
	}
	/* see if this file is Mach-O file for 32-bit architectures */
	else if(size >= sizeof(struct mach_header) &&
	        (*((uint32_t *)addr) == MH_MAGIC ||
	         *((uint32_t *)addr) == SWAP_INT(MH_MAGIC))){

	    /* this is a Mach-O file so create a thin file struct for it */
	    thin = new_thin();
	    input->is_thin = TRUE;
	    thin->name = input->name;
	    thin->addr = addr;
	    mhp = (struct mach_header *)addr;
	    lcp = (struct load_command *)((char *)mhp +
					  sizeof(struct mach_header));
	    if(mhp->magic == SWAP_INT(MH_MAGIC)){
		swapped = TRUE;
		mh = *mhp;
		swap_mach_header(&mh, get_host_byte_sex());
		mhp = &mh;
	    }
	    else
		swapped = FALSE;
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the size "
		      "field in struct fat_arch is only 32-bits and the size "
		      "(%llu) of %s exceeds that", size, input->name);
	    thin->cputype = mhp->cputype;
	    thin->cpusubtype = mhp->cpusubtype;
	    thin->offset = 0;
	    thin->size = size;
	    thin->align = get_align(mhp, lcp, size, input->name, swapped);

	    /* if the arch type is specified make sure it matches the object */
	    if(input->arch_flag.name != NULL)
		check_arch(input, thin);
	}
	/* see if this file is Mach-O file for 64-bit architectures */
	else if(size >= sizeof(struct mach_header_64) &&
	        (*((uint32_t *)addr) == MH_MAGIC_64 ||
	         *((uint32_t *)addr) == SWAP_INT(MH_MAGIC_64))){

	    /* this is a Mach-O file so create a thin file struct for it */
	    thin = new_thin();
	    input->is_thin = TRUE;
	    thin->name = input->name;
	    thin->addr = addr;
	    mhp64 = (struct mach_header_64 *)addr;
	    lcp = (struct load_command *)((char *)mhp64 +
					  sizeof(struct mach_header_64));
	    if(mhp64->magic == SWAP_INT(MH_MAGIC_64)){
		swapped = TRUE;
		mh64 = *mhp64;
		swap_mach_header_64(&mh64, get_host_byte_sex());
		mhp64 = &mh64;
	    }
	    else
		swapped = FALSE;
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the size "
		      "field in struct fat_arch is only 32-bits and the size "
		      "(%llu) of %s exceeds that", size, input->name);
	    thin->cputype = mhp64->cputype;
	    thin->cpusubtype = mhp64->cpusubtype;
	    thin->offset = 0;
	    thin->size = size;
	    thin->align = get_align_64(mhp64, lcp, size, input->name, swapped);

	    /* if the arch type is specified make sure it matches the object */
	    if(input->arch_flag.name != NULL)
		check_arch(input, thin);
	}
	/* see if this file is an archive file */
	else if(size >= SARMAG && strncmp(addr, ARMAG, SARMAG) == 0){

	    check_archive(input->name, addr, size, &cputype, &cpusubtype);
	    archives_in_input = TRUE;

	    /* create a thin file struct for this archive */
	    thin = new_thin();
	    input->is_thin = TRUE;
	    thin->name = input->name;
	    thin->addr = addr;
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the size "
		      "field in struct fat_arch is only 32-bits and the size "
		      "(%llu) of %s exceeds that", size, input->name);
	    thin->cputype = cputype;
	    thin->cpusubtype = cpusubtype;
	    thin->offset = 0;
	    thin->size = size;
	    if(thin->cputype & CPU_ARCH_ABI64)
		thin->align = 3; /* 2^3, alignof(uint64_t) */
	    else
		thin->align = 2; /* 2^2, alignof(uint32_t) */

	    /* if the arch type is specified make sure it matches the object */
	    if(input->arch_flag.name != NULL){
		if(cputype == 0){
		    thin->cputype = input->arch_flag.cputype;
		    thin->cpusubtype = input->arch_flag.cpusubtype;
		}
		else
		    check_arch(input, thin);
	    }
	    else{
		if(cputype == 0)
		    fatal("archive with no architecture specification: %s "
			  "(can't determine architecture for it)", input->name);
	    }
	}
	/* this file type is now known to be unknown to this program */
	else{

	    if(input->arch_flag.name != NULL){
		/* create a thin file struct for it */
		thin = new_thin();
		thin->name = input->name;
		thin->addr = addr;
		if(fat64_flag == FALSE && size > UINT32_MAX)
		    fatal("file too large to be in a fat file because the size "
			  "field in struct fat_arch is only 32-bits and the "
			  "size (%llu) of %s exceeds that", size, input->name);
		thin->cputype = input->arch_flag.cputype;
		thin->cpusubtype = input->arch_flag.cpusubtype;
		thin->offset = 0;
		thin->size = size;
		thin->align = 0;
	    }
	    else{
#ifdef LTO_SUPPORT
		if(is_llvm_bitcode_from_memory(addr, (uint32_t)size,
					       &input->arch_flag, NULL) != 0){
		    /* create a thin file struct for it */
		    thin = new_thin();
		    thin->name = input->name;
		    thin->addr = addr;
		    if(fat64_flag == FALSE && size > UINT32_MAX)
			fatal("file too large to be in a fat file because the "
			      "size field in struct fat_arch is only 32-bits "
			      "and the size (%llu) of %s exceeds that", size,
			      input->name);
		    thin->cputype = input->arch_flag.cputype;
		    thin->cpusubtype = input->arch_flag.cpusubtype;
		    thin->offset = 0;
		    thin->size = size;
		    thin->align = 0;
		}
		else
#endif /* LTO_SUPPORT */
		    fatal("can't figure out the architecture type of: %s",
			  input->name);
	    }
	}
}

/*
 * process_replace_file() checks the replacement file and maps it in for later
 * processing.
 */
static
void
process_replace_file(
struct replace *replace)
{
    int fd;
    struct stat stat_buf;
    uint64_t size;
    char *addr;
    struct mach_header *mhp, mh;
    struct mach_header_64 *mhp64, mh64;
    struct load_command *lcp;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    enum bool swapped;

	/* Open the replacement file and map it in */
	if((fd = open(replace->thin_file.name, O_RDONLY)) == -1)
	    system_fatal("can't open replacement file: %s",
			 replace->thin_file.name);
	if(fstat(fd, &stat_buf) == -1)
	    system_fatal("can't stat replacement file: %s",
			 replace->thin_file.name);
	size = stat_buf.st_size;
	/*
	 * mmap() can't handle mapping regular files with zero size.  So this
	 * is handled separately.
	 */
	if((stat_buf.st_mode & S_IFREG) == S_IFREG && size == 0)
	    addr = NULL;
	else
	    addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE,
			fd, 0);
	if((intptr_t)addr == -1)
	    system_error("can't map replacement file: %s",
			 replace->thin_file.name);
	close(fd);

	/* Try to figure out what kind of file this is */

	/* see if this file is a fat file */
	if(size >= sizeof(struct fat_header) &&
#ifdef __BIG_ENDIAN__
	   (*((uint32_t *)addr) == FAT_MAGIC ||
            *((uint32_t *)addr) == FAT_MAGIC_64))
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	   (*((uint32_t *)addr) == SWAP_INT(FAT_MAGIC) ||
            *((uint32_t *)addr) == SWAP_INT(FAT_MAGIC_64)))
#endif /* __LITTLE_ENDIAN__ */
	{

	    fatal("replacement file: %s is a fat file (must be a thin file)",
		  replace->thin_file.name);
	}
	/* see if this file is Mach-O file for 32-bit architectures */
	else if(size >= sizeof(struct mach_header) &&
	        (*((uint32_t *)addr) == MH_MAGIC ||
	         *((uint32_t *)addr) == SWAP_INT(MH_MAGIC))){

	    /* this is a Mach-O file so fill in the thin file struct for it */
	    replace->thin_file.addr = addr;
	    mhp = (struct mach_header *)addr;
	    lcp = (struct load_command *)((char *)mhp +
					  sizeof(struct mach_header));
	    if(mhp->magic == SWAP_INT(MH_MAGIC)){
		swapped = TRUE;
		mh = *mhp;
		swap_mach_header(&mh, get_host_byte_sex());
		mhp = &mh;
	    }
	    else
		swapped = FALSE;
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the "
		      "size field in struct fat_arch is only 32-bits "
		      "and the size (%llu) of %s exceeds that", size,
		      replace->thin_file.name);
	    replace->thin_file.cputype = mhp->cputype;
	    replace->thin_file.cpusubtype = mhp->cpusubtype;
	    replace->thin_file.offset = 0;
	    replace->thin_file.size = size;
	    replace->thin_file.align =
		    get_align(mhp, lcp, size, replace->thin_file.name, swapped);
	}
	/* see if this file is Mach-O file for 64-bit architectures */
	else if(size >= sizeof(struct mach_header_64) &&
	        (*((uint32_t *)addr) == MH_MAGIC_64 ||
	         *((uint32_t *)addr) == SWAP_INT(MH_MAGIC_64))){

	    /* this is a Mach-O file so fill in the thin file struct for it */
	    replace->thin_file.addr = addr;
	    mhp64 = (struct mach_header_64 *)addr;
	    lcp = (struct load_command *)((char *)mhp64 +
					  sizeof(struct mach_header_64));
	    if(mhp64->magic == SWAP_INT(MH_MAGIC_64)){
		swapped = TRUE;
		mh64 = *mhp64;
		swap_mach_header_64(&mh64, get_host_byte_sex());
		mhp64 = &mh64;
	    }
	    else
		swapped = FALSE;
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the "
		      "size field in struct fat_arch is only 32-bits "
		      "and the size (%llu) of %s exceeds that", size,
		      replace->thin_file.name);
	    replace->thin_file.cputype = mhp64->cputype;
	    replace->thin_file.cpusubtype = mhp64->cpusubtype;
	    replace->thin_file.offset = 0;
	    replace->thin_file.size = size;
	    replace->thin_file.align =
	       get_align_64(mhp64, lcp, size, replace->thin_file.name, swapped);
	}
	/* see if this file is an archive file */
	else if(size >= SARMAG && strncmp(addr, ARMAG, SARMAG) == 0){

	    check_archive(replace->thin_file.name, addr, size,
			  &cputype, &cpusubtype);

	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the "
		      "size field in struct fat_arch is only 32-bits "
		      "and the size (%llu) of %s exceeds that", size,
		      replace->thin_file.name);
	    /* fill in the thin file struct for this archive */
	    replace->thin_file.addr = addr;
	    replace->thin_file.cputype = cputype;
	    replace->thin_file.cpusubtype = cpusubtype;
	    replace->thin_file.offset = 0;
	    replace->thin_file.size = size;
	    replace->thin_file.align = 2; /* 2^2, sizeof(uint32_t) */
	}
	else{
	    if(fat64_flag == FALSE && size > UINT32_MAX)
		fatal("file too large to be in a fat file because the "
		      "size field in struct fat_arch is only 32-bits "
		      "and the size (%llu) of %s exceeds that", size,
		      replace->thin_file.name);
	    /* fill in the thin file struct for it */
	    replace->thin_file.addr = addr;
	    replace->thin_file.cputype = replace->arch_flag.cputype;
	    replace->thin_file.cpusubtype = replace->arch_flag.cpusubtype;
	    replace->thin_file.offset = 0;
	    replace->thin_file.size = size;
	    replace->thin_file.align = 0;
	}

	if(replace->thin_file.cputype != replace->arch_flag.cputype ||
	   (replace->thin_file.cpusubtype & ~CPU_SUBTYPE_MASK) !=
	   (replace->arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK))
	    fatal("specified architecture: %s for replacement file: %s does "
		  "not match the file's architecture", replace->arch_flag.name,
		  replace->thin_file.name);
}

/*
 * check_archive() checks an archive (mapped in at 'addr' of size 'size') to
 * make sure it can be included in a fat file (all members are the same
 * architecture, no fat files, etc).  It returns the cputype and cpusubtype.
 */
static
void
check_archive(
char *name,
char *addr,
uint64_t size,
cpu_type_t *cputype,
cpu_subtype_t *cpusubtype)
{
    uint64_t offset;
    uint32_t magic, i, ar_name_size, ar_size;
    struct mach_header mh;
    struct mach_header_64 mh64;
    struct ar_hdr *ar_hdr;
    char *ar_name, *ar_addr;
    struct arch_flag arch_flag;

	/*
	 * Check this archive out to make sure that it does not contain
	 * any fat files and that all object files it contains have the
	 * same cputype and subsubtype.
	 */
	*cputype = 0;
	*cpusubtype = 0;
	offset = SARMAG;
	if(offset == size)
	    fatal("empty archive with no architecture specification: %s "
		  "(can't determine architecture for it)", name);
	if(offset != size && offset + sizeof(struct ar_hdr) > size)
	    fatal("truncated or malformed archive: %s (archive header of "
		  "first member extends past the end of the file)", name);
	while(size > offset){
	    ar_hdr = (struct ar_hdr *)(addr + offset);
	    offset += sizeof(struct ar_hdr);
	    if(strncmp(ar_hdr->ar_name, AR_EFMT1, sizeof(AR_EFMT1) - 1) == 0){
		check_extend_format_1(name, ar_hdr, size-offset, &ar_name_size);
		i = ar_name_size;
		ar_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
	    }
	    else{
		i = size_ar_name(ar_hdr->ar_name);
		ar_name = ar_hdr->ar_name;
		ar_name_size = 0;
	    }
	    if(size + ar_name_size - offset > sizeof(uint32_t)){
		memcpy(&magic, addr + offset + ar_name_size,
		       sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
		if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
		if(magic == SWAP_INT(FAT_MAGIC) ||
		   magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
		    fatal("archive member %s(%.*s) is a fat file (not "
			  "allowed in an archive)", name, (int)i, ar_name);
		if((size - ar_name_size) - offset >=
		    sizeof(struct mach_header) &&
		   (magic == MH_MAGIC || magic == SWAP_INT(MH_MAGIC))){
		    memcpy(&mh, addr + offset + ar_name_size,
			   sizeof(struct mach_header));
		    if(mh.magic == SWAP_INT(MH_MAGIC))
			swap_mach_header(&mh, get_host_byte_sex());
		    if(*cputype == 0){
			*cputype = mh.cputype;
			*cpusubtype = mh.cpusubtype;
		    }
		    else if(*cputype != mh.cputype){
			fatal("archive member %s(%.*s) cputype (%d) and "
			      "cpusubtype (%d) does not match previous "
			      "archive members cputype (%d) and cpusubtype"
			      " (%d) (all members must match)", name,
			      (int)i, ar_name, mh.cputype, mh.cpusubtype &
			      ~CPU_SUBTYPE_MASK, *cputype, (*cpusubtype) & 
			      ~CPU_SUBTYPE_MASK);
		    }
		    else {
			if (mh.cputype == CPU_TYPE_ARM &&
			    *cpusubtype != mh.cpusubtype)
			    fatal("archive member %s(%.*s) cputype (%d) and "
				  "cpusubtype (%d) does not match previous "
				  "archive members cputype (%d) and cpusubtype"
				  " (%d) (all members must match)", name,
				  (int)i, ar_name, mh.cputype, mh.cpusubtype &
				  ~CPU_SUBTYPE_MASK, *cputype, (*cpusubtype) &
				  ~CPU_SUBTYPE_MASK);
		    }
		}
		else if((size - ar_name_size) - offset >=
		    sizeof(struct mach_header_64) &&
		   (magic == MH_MAGIC_64 || magic == SWAP_INT(MH_MAGIC_64))){
		    memcpy(&mh64, addr + offset + ar_name_size,
			   sizeof(struct mach_header_64));
		    if(mh64.magic == SWAP_INT(MH_MAGIC_64))
			swap_mach_header_64(&mh64, get_host_byte_sex());
		    if(*cputype == 0){
			*cputype = mh64.cputype;
			*cpusubtype = mh64.cpusubtype;
		    }
		    else if(*cputype != mh64.cputype){
			fatal("archive member %s(%.*s) cputype (%d) and "
			      "cpusubtype (%d) does not match previous "
			      "archive members cputype (%d) and cpusubtype"
			      " (%d) (all members must match)", name,
			      (int)i, ar_name, mh64.cputype, mh64.cpusubtype &
			      ~CPU_SUBTYPE_MASK, *cputype, (*cpusubtype) &
			      ~CPU_SUBTYPE_MASK);
		    }
		    else {
			if ((mh64.cputype == CPU_TYPE_X86_64 ||
			     mh64.cputype == CPU_TYPE_ARM64) &&
			    *cpusubtype != mh64.cpusubtype)
			    fatal("archive member %s(%.*s) cputype (%d) and "
				  "cpusubtype (%d) does not match previous "
				  "archive members cputype (%d) and cpusubtype"
				  " (%d) (all members must match)", name,
				  (int)i, ar_name, mh64.cputype, mh64.cpusubtype &
				  ~CPU_SUBTYPE_MASK, *cputype, (*cpusubtype) &
				  ~CPU_SUBTYPE_MASK);
		    }
		}
		else{
		    if(strncmp(ar_name, SYMDEF, sizeof(SYMDEF) - 1) != 0){
			ar_addr = addr + offset + ar_name_size;
			ar_size = (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10);
#ifdef LTO_SUPPORT
			if(is_llvm_bitcode_from_memory(ar_addr, ar_size,
						       &arch_flag, NULL) != 0){
			    if(*cputype == 0){
				*cputype = arch_flag.cputype;
				*cpusubtype = arch_flag.cpusubtype;
			    }
			    else if(*cputype != arch_flag.cputype){
				fatal("archive member %s(%.*s) cputype (%d) "
				      "and cpusubtype (%d) does not match "
				      "previous archive members cputype (%d) "
				      "and cpusubtype (%d) (all members must "
				      "match)", name, (int)i, ar_name,
				      arch_flag.cputype, arch_flag.cpusubtype &
				      ~CPU_SUBTYPE_MASK, *cputype,
				      (*cpusubtype) & ~CPU_SUBTYPE_MASK);
			    }
			}
#endif /* LTO_SUPPORT */
		    }
		}
	    }
	    offset += rnd64(strtoul(ar_hdr->ar_size, NULL, 10),
			    sizeof(short));
	}
}
/*
 * check_extend_format_1() checks the archive header for extened format1.
 */
static
void
check_extend_format_1(
char *name,
struct ar_hdr *ar_hdr,
uint64_t size_left,
uint32_t *member_name_size)
{
    char *p, *endp, buf[sizeof(ar_hdr->ar_name)+1];
    uint32_t ar_name_size;

	*member_name_size = 0;

	buf[sizeof(ar_hdr->ar_name)] = '\0';
	memcpy(buf, ar_hdr->ar_name, sizeof(ar_hdr->ar_name));
	p = buf + sizeof(AR_EFMT1) - 1;
	if(isdigit(*p) == 0)
	    fatal("archive: %s malformed (ar_name: %.*s for archive extend "
		  "format #1 starts with non-digit)", name,
		  (int)sizeof(ar_hdr->ar_name), ar_hdr->ar_name);
	ar_name_size = (uint32_t)strtoul(p, &endp, 10);
	if(ar_name_size == UINT_MAX && errno == ERANGE)
	    fatal("archive: %s malformed (size in ar_name: %.*s for archive "
		  "extend format #1 overflows uint32_t)", name,
		  (int)sizeof(ar_hdr->ar_name), ar_hdr->ar_name);
	while(*endp == ' ' && *endp != '\0')
	    endp++;
	if(*endp != '\0')
	    fatal("archive: %s malformed (size in ar_name: %.*s for archive "
		  "extend format #1 contains non-digit and non-space "
		  "characters)", name, (int)sizeof(ar_hdr->ar_name),
		  ar_hdr->ar_name);
	if(ar_name_size > size_left)
	    fatal("archive: %s truncated or malformed (archive name "
		"of member extends past the end of the file)", name);
	*member_name_size = ar_name_size;
}

/*
 * get_default_align() returns the default segment alignment for the specified
 * cputype and cpusubtype, as an exponent of a power of 2; e.g., a segment
 * alignment of 0x4000 will be described as 14. If the default alignment is not
 * known it will return 0.
 */
static
uint32_t
get_default_align(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
    const char* arch_name = get_arch_name_from_types(cputype, cpusubtype);
    if (arch_name != NULL) {
	struct arch_flag arch_flag;
	if (get_arch_from_flag((char*)arch_name, &arch_flag)) {
	    uint32_t pagesize = get_segalign_from_flag(&arch_flag);
	    return (uint32_t)(log2(pagesize));
	}
    }
    
    return 0;
}

/*
 * get_mh_filetype() gets the filetype from the mach-o pointed to by addr.
 * will return 0 if addr does not point to a struct mach_header or struct
 * mach_header_64.
 */
static uint32_t get_mh_filetype(
    char* addr,
    uint64_t size)
{
    uint32_t filetype;
    uint32_t magic;
    struct mach_header mh32;
    struct mach_header_64 mh64;
    
    filetype = 0;
    
    if (size >= sizeof(magic)) {
        magic = *(uint32_t*)(addr);
        if (magic == MH_MAGIC || magic == MH_CIGAM) {
            if (size >= sizeof(mh32)) {
                memcpy(&mh32, addr, sizeof(mh32));
                if (magic == MH_CIGAM)
                    swap_mach_header(&mh32, get_host_byte_sex());
                filetype = mh32.filetype;
            }
        }
        else if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
            if (size >= sizeof(mh64)) {
                memcpy(&mh64, addr, sizeof(mh64));
                if (magic == MH_CIGAM)
                    swap_mach_header_64(&mh64, get_host_byte_sex());
                filetype = mh64.filetype;
            }
        }
    }
    
    return filetype;
}

/*
 * get_align() returns the segment alignment for this object, as an exponent of
 * a power of 2; e.g., a segment alignment of 0x4000 will be described as 14.
 * Since the actual segment alignment used by the linker is not recorded in the
 * Mach-O file, get_align() will choose an alignment based on the file contents.
 *
 * If the mach_header points to a well-known cputype, get_align() will return
 * the default segment alignment for that cputype. No attempt will be made to
 * guess the "-segalign" flag passed into ld(1).
 *
 * If the cputype isn't recognized, get_align() will issue a warning (this is
 * potentially a serious configuration error) and fall back to historical
 * methods:
 *
 *   If the Mach-O is an MH_OBJECT (.o) file, get_align() will return the
 *   largest section alignment within the first-and-only segment.
 *
 *   If the Mach-O is any other file type, get_align() will guess the alignment
 *   for each segment from vmaddr, and then return the smallest such value.
 *   Since all well-formed segments are required to be page aligned, the
 *   resulting alignment will be legal, but there is a risk that unlucky
 *   binaries will choose an alignment value that is larger than necessary.
 *
 * In either fall back method, the result of get_align() will be bounded by
 * 2 (which is log2(sizeof(uint32_t))) and MAXSECTALIGN.
 */
static
uint32_t
get_align(
struct mach_header *mhp,
struct load_command *load_commands,
uint64_t size,
char *name,
enum bool swapped)
{
    uint32_t i, j, cur_align, align;
    struct load_command *lcp, l;
    struct segment_command *sgp, sg;
    struct section *sp, s;
    enum byte_sex host_byte_sex;
    
	/*
	 * Special case well-known architectures. We know that for these
	 * architectures that the Darwin kernel and mmap require file offsets
	 * to be page aligned.
	 */
	align = get_default_align(mhp->cputype, mhp->cpusubtype);
	if (align != 0)
	    return align;
    
	warning("unknown cputype (%u) cpusubtype (%u), computing the segment "
		"alignment from file contents.", mhp->cputype,
		mhp->cpusubtype & ~CPU_SUBTYPE_MASK);

	host_byte_sex = get_host_byte_sex();

	/* set worst case the link editor uses first */
	cur_align = MAXSECTALIGN;
	if(mhp->sizeofcmds + sizeof(struct mach_header) > size)
	    fatal("truncated or malformed object (load commands would "
		  "extend past the end of the file) in: %s", name);
	lcp = load_commands;
	for(i = 0; i < mhp->ncmds; i++){
	    l = *lcp;
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(uint32_t) != 0)
		error("load command %u size not a multiple of "
		      "sizeof(uint32_t) in: %s", i, name);
	    if(l.cmdsize <= 0)
		fatal("load command %u size is less than or equal to zero "
		      "in: %s", i, name);
	    if((char *)lcp + l.cmdsize >
	       (char *)load_commands + mhp->sizeofcmds)
		fatal("load command %u extends past end of all load "
		      "commands in: %s", i, name);
	    if(l.cmd == LC_SEGMENT){
		sgp = (struct segment_command *)lcp;
		sg = *sgp;
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);
		if(mhp->filetype == MH_OBJECT){
		    /* this is the minimum alignment, then take largest */
		    align = 2; /* 2^2 sizeof(uint32_t) */
		    sp = (struct section *)((char *)sgp +
					    sizeof(struct segment_command));
		    for(j = 0; j < sg.nsects; j++){
			s = *sp;
			if(swapped)
			    swap_section(&s, 1, host_byte_sex);
			if(s.align > align)
			    align = s.align;
			sp++;
		    }
		    if(align < cur_align)
			cur_align = align;
		}
		else{
		    /* guess the smallest alignment and use that */
		    align = guess_align(sg.vmaddr);
		    if(align < cur_align)
			cur_align = align;
		}
	    }
	    lcp = (struct load_command *)((char *)lcp + l.cmdsize);
	}
	return(cur_align);
}

/*
 * get_align_64() returns the segment alignment for this object, as an exponent
 * of a power of 2; e.g., a segment alignment of 0x4000 will be described as 14.
 * The method of determining the segment alignment is the same as get_align()
 * above.
 *
 * In either fall back method, the result of get_align_64() will be bounded by
 * 3 (which is log2(sizeof(uint64_t))) and MAXSECTALIGN.
 */
static
uint32_t
get_align_64(
struct mach_header_64 *mhp64,
struct load_command *load_commands,
uint64_t size,
char *name,
enum bool swapped)
{
    uint32_t i, j, cur_align, align;
    struct load_command *lcp, l;
    struct segment_command_64 *sgp, sg;
    struct section_64 *sp, s;
    enum byte_sex host_byte_sex;

	/*
	 * Special case well-known architectures. We know that for these
	 * architectures that the Darwin kernel and mmap require file offsets
	 * to be page aligned.
	 */
	align = get_default_align(mhp64->cputype, mhp64->cpusubtype);
	if (align != 0)
	    return align;

	warning("unknown cputype (%u) cpusubtype (%u), computing the segment "
		"alignment from file contents.", mhp64->cputype,
		mhp64->cpusubtype & ~CPU_SUBTYPE_MASK);
	
	host_byte_sex = get_host_byte_sex();

	/* set worst case the link editor uses first */
	cur_align = MAXSECTALIGN;
	if(mhp64->sizeofcmds + sizeof(struct mach_header_64) > size)
	    fatal("truncated or malformed object (load commands would "
		  "extend past the end of the file) in: %s", name);
	lcp = load_commands;
	for(i = 0; i < mhp64->ncmds; i++){
	    l = *lcp;
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(long long) != 0)
		error("load command %u size not a multiple of "
		      "sizeof(long long) in: %s", i, name);
	    if(l.cmdsize <= 0)
		fatal("load command %u size is less than or equal to zero "
		      "in: %s", i, name);
	    if((char *)lcp + l.cmdsize >
	       (char *)load_commands + mhp64->sizeofcmds)
		fatal("load command %u extends past end of all load "
		      "commands in: %s", i, name);
	    if(l.cmd == LC_SEGMENT_64){
		sgp = (struct segment_command_64 *)lcp;
		sg = *sgp;
		if(swapped)
		    swap_segment_command_64(&sg, host_byte_sex);
		if(mhp64->filetype == MH_OBJECT){
		    /* this is the minimum alignment, then take largest */
		    align = 3; /* 2^3 sizeof(long long) */
		    sp = (struct section_64 *)((char *)sgp +
					    sizeof(struct segment_command_64));
		    for(j = 0; j < sg.nsects; j++){
			s = *sp;
			if(swapped)
			    swap_section_64(&s, 1, host_byte_sex);
			if(s.align > align)
			    align = s.align;
			sp++;
		    }
		    if(align < cur_align)
			cur_align = align;
		}
		else{
		    /* guess the smallest alignment and use that */
		    align = guess_align(sg.vmaddr);
		    if(align < cur_align)
			cur_align = align;
		}
	    }
	    lcp = (struct load_command *)((char *)lcp + l.cmdsize);
	}
	return(cur_align);
}

/*
 * guess_align is passed a vmaddr of a segment and guesses what the segment
 * alignment was.  It uses the most conservative guess up to the maximum
 * alignment that the link editor uses.
 */
static
uint32_t
guess_align(
uint64_t vmaddr)
{
    uint32_t align;
    uint64_t segalign;

	if(vmaddr == 0)
	    return(MAXSECTALIGN);

	align = 0;
	segalign = 1;
	while((segalign & vmaddr) == 0){
	    segalign = segalign << 1;
	    align++;
	}
	
	if(align < 2)
	    return(2);
	if(align > MAXSECTALIGN)
	    return(MAXSECTALIGN);

	return(align);
}

/*
 * print_arch() helps implement -info and -detailed_info by printing the
 * architecture name for the cputype and cpusubtype.
 */
static
void
print_arch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC680x0_ALL:
		printf("m68k");
		break;
	    case CPU_SUBTYPE_MC68030_ONLY:
		printf("m68030");
		break;
	    case CPU_SUBTYPE_MC68040:
		printf("m68040");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("ppc");
		break;
	    case CPU_SUBTYPE_POWERPC_601:
		printf("ppc601");
		break;
	    case CPU_SUBTYPE_POWERPC_603:
		printf("ppc603");
		break;
	    case CPU_SUBTYPE_POWERPC_603e:
		printf("ppc603e");
		break;
	    case CPU_SUBTYPE_POWERPC_603ev:
		printf("ppc603ev");
		break;
	    case CPU_SUBTYPE_POWERPC_604:
		printf("ppc604");
		break;
	    case CPU_SUBTYPE_POWERPC_604e:
		printf("ppc604e");
		break;
	    case CPU_SUBTYPE_POWERPC_750:
		printf("ppc750");
		break;
	    case CPU_SUBTYPE_POWERPC_7400:
		printf("ppc7400");
		break;
	    case CPU_SUBTYPE_POWERPC_7450:
		printf("ppc7450");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("ppc970");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("ppc64");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("ppc970-64");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_VEO:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_VEO_1:
		printf("veo1");
		break;
	    case CPU_SUBTYPE_VEO_2:
		printf("veo2");
		break;
	    case CPU_SUBTYPE_VEO_3:
		printf("veo3");
		break;
	    case CPU_SUBTYPE_VEO_4:
		printf("veo4");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_MC88000:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC88000_ALL:
	    case CPU_SUBTYPE_MC88110:
		printf("m88k");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_386: same as above */
		printf("i386");
		break;
	    case CPU_SUBTYPE_486:
		printf("i486");
		break;
	    case CPU_SUBTYPE_486SX:
		printf("i486SX");
		break;
	    case CPU_SUBTYPE_PENT: /* same as 586 */
		printf("pentium");
		break;
	    case CPU_SUBTYPE_PENTPRO:
		printf("pentpro");
		break;
	    case CPU_SUBTYPE_PENTII_M3:
		printf("pentIIm3");
		break;
	    case CPU_SUBTYPE_PENTII_M5:
		printf("pentIIm5");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_X86_64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_X86_64_ALL:
		printf("x86_64");
		break;
	    case CPU_SUBTYPE_X86_64_H:
		printf("x86_64h");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I860:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I860_ALL:
	    case CPU_SUBTYPE_I860_860:
		printf("i860");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_HPPA:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_HPPA_ALL:
	    case CPU_SUBTYPE_HPPA_7100LC:
		printf("hppa");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_SPARC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_SPARC_ALL:
		printf("sparc");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM:
	    switch(cpusubtype){
	    case CPU_SUBTYPE_ARM_ALL:
		printf("arm");
		break;
	    case CPU_SUBTYPE_ARM_V4T:
		printf("armv4t");
		break;
	    case CPU_SUBTYPE_ARM_V5TEJ:
		printf("armv5");
		break;
	    case CPU_SUBTYPE_ARM_XSCALE:
		printf("xscale");
		break;
	    case CPU_SUBTYPE_ARM_V6:
		printf("armv6");
		break;
	    case CPU_SUBTYPE_ARM_V6M:
		printf("armv6m");
		break;
	    case CPU_SUBTYPE_ARM_V7:
		printf("armv7");
		break;
	    case CPU_SUBTYPE_ARM_V7F:
		printf("armv7f");
		break;
	    case CPU_SUBTYPE_ARM_V7S:
		printf("armv7s");
		break;
	    case CPU_SUBTYPE_ARM_V7K:
		printf("armv7k");
		break;
	    case CPU_SUBTYPE_ARM_V7M:
		printf("armv7m");
		break;
	    case CPU_SUBTYPE_ARM_V7EM:
		printf("armv7em");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_ALL:
		printf("arm64");
		break;
	    case CPU_SUBTYPE_ARM64_V8:
		printf("arm64v8");
		break;
	    case CPU_SUBTYPE_ARM64E:
		printf("arm64e");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64_32:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_32_V8:
		printf("arm64_32");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ANY:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MULTIPLE:
		printf("any");
		break;
	    case CPU_SUBTYPE_LITTLE_ENDIAN:
		printf("little");
		break;
	    case CPU_SUBTYPE_BIG_ENDIAN:
		printf("big");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
print_arch_unknown:
	default:
	    printf("(cputype (%d) cpusubtype (%d))", cputype,
		   cpusubtype & ~CPU_SUBTYPE_MASK);
	    break;
	}
}

/*
 * print_cputype() helps implement -detailed_info by printing the cputype and
 * cpusubtype (symbolicly for the one's it knows about).
 */
static
void
print_cputype(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC680x0_ALL:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC680x0_ALL\n");
		break;
	    case CPU_SUBTYPE_MC68030_ONLY:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC68030_ONLY\n");
		break;
	    case CPU_SUBTYPE_MC68040:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC68040\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_ALL\n");
		break;
	    case CPU_SUBTYPE_POWERPC_601:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_601\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603e:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603ev:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603ev\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_604\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604e:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_604e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_750:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_750\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7400:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_7400\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7450:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_7450\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_970\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("    cputype CPU_TYPE_POWERPC64\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_ALL\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("    cputype CPU_TYPE_POWERPC64\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_970\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_VEO:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_VEO_1:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_1\n");
		break;
	    case CPU_SUBTYPE_VEO_2:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_2\n");
		break;
	    case CPU_SUBTYPE_VEO_3:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_3\n");
		break;
	    case CPU_SUBTYPE_VEO_4:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_4\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_MC88000:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC88000_ALL:
		printf("    cputype CPU_TYPE_MC88000\n"
		       "    cpusubtype CPU_SUBTYPE_MC88000_ALL\n");
		break;
	    case CPU_SUBTYPE_MC88110:
		printf("    cputype CPU_TYPE_MC88000\n"
		       "    cpusubtype CPU_SUBTYPE_MC88110\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_386: same as above */
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_I386_ALL\n");
		break;
	    case CPU_SUBTYPE_486:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_486\n");
		break;
	    case CPU_SUBTYPE_486SX:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_486SX\n");
		break;
	    case CPU_SUBTYPE_PENT: /* same as 586 */
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENT\n");
		break;
	    case CPU_SUBTYPE_PENTPRO:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTPRO\n");
		break;
	    case CPU_SUBTYPE_PENTII_M3:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTII_M3\n");
		break;
	    case CPU_SUBTYPE_PENTII_M5:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTII_M5\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_X86_64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_X86_64_ALL:
		printf("    cputype CPU_TYPE_X86_64\n"
		       "    cpusubtype CPU_SUBTYPE_X86_64_ALL\n");
		break;
	    case CPU_SUBTYPE_X86_64_H:
		printf("    cputype CPU_TYPE_X86_64\n"
		       "    cpusubtype CPU_SUBTYPE_X86_64_H\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I860:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I860_ALL:
		printf("    cputype CPU_TYPE_I860\n"
		       "    cpusubtype CPU_SUBTYPE_I860_ALL\n");
		break;
	    case CPU_SUBTYPE_I860_860:
		printf("    cputype CPU_TYPE_I860\n"
		       "    cpusubtype CPU_SUBTYPE_I860_860\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_HPPA:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_HPPA_ALL:
		printf("    cputype CPU_TYPE_HPPA\n"
		       "    cpusubtype CPU_SUBTYPE_HPPA_ALL\n");
		break;
	    case CPU_SUBTYPE_HPPA_7100LC:
		printf("    cputype CPU_TYPE_HPPA\n"
		       "    cpusubtype CPU_SUBTYPE_HPPA_7100LC\n");
	    	break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_SPARC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_SPARC_ALL:
		printf("    cputype CPU_TYPE_SPARC\n"
		       "    cpusubtype CPU_SUBTYPE_SPARC_ALL\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM:
	    switch(cpusubtype){
	    case CPU_SUBTYPE_ARM_V4T:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V4T\n");
		break;
	    case CPU_SUBTYPE_ARM_V5TEJ:
		printf("     cputype CPU_TYPE_ARM\n"
		       "     cpusubtype CPU_SUBTYPE_ARM_V5TEJ\n");
		break;
	    case CPU_SUBTYPE_ARM_XSCALE:
		printf("     cputype CPU_TYPE_ARM\n"
		       "     cpusubtype CPU_SUBTYPE_ARM_XSCALE\n");
		break;
	    case CPU_SUBTYPE_ARM_V6:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V6\n");
		break;
	    case CPU_SUBTYPE_ARM_V6M:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V6M\n");
		break;
	    case CPU_SUBTYPE_ARM_V7:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7\n");
		break;
	    case CPU_SUBTYPE_ARM_V7F:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7F\n");
		break;
	    case CPU_SUBTYPE_ARM_V7S:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7S\n");
		break;
	    case CPU_SUBTYPE_ARM_V7K:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7K\n");
		break;
	    case CPU_SUBTYPE_ARM_V7M:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7M\n");
		break;
	    case CPU_SUBTYPE_ARM_V7EM:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7EM\n");
		break;
	    case CPU_SUBTYPE_ARM_ALL:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_ALL\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_ALL:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_ALL\n");
		break;
	    case CPU_SUBTYPE_ARM64_V8:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_V8\n");
		break;
	    case CPU_SUBTYPE_ARM64E:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64E\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64_32:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_32_V8:
		printf("    cputype CPU_TYPE_ARM64_32\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_32_V8\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ANY:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MULTIPLE:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_MULTIPLE\n");
		break;
	    case CPU_SUBTYPE_LITTLE_ENDIAN:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_LITTLE_ENDIAN\n");
		break;
	    case CPU_SUBTYPE_BIG_ENDIAN:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_BIG_ENDIAN\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
print_arch_unknown:
	default:
	    printf("    cputype (%d)\n"
		   "    cpusubtype cpusubtype (%d)\n", cputype,
		   cpusubtype & ~CPU_SUBTYPE_MASK);
	    break;
	}
}

/*
 * size_ar_name is used to return the size of the name of an archive member
 * for printing it without blanks (with printf string "%.*s").
 */
static
int
size_ar_name(
char *ar_name)
{
    uint32_t j;
    struct ar_hdr ar_hdr;

	for(j = 0; j < sizeof(ar_hdr.ar_name); j++){
	    if(ar_name[j] == ' ')
		break;
	}
	return(j);
}

/*
 * Create a new input file struct, clear it and return it.
 */
static
struct input_file *
new_input(void)
{
    struct input_file *input;

	input_files = reallocate(input_files,
		      (ninput_files + 1) * sizeof(struct input_file));
	input = input_files + ninput_files;
	ninput_files++;
	memset(input, '\0', sizeof(struct input_file));
	return(input);
}

/*
 * Create a new thin file struct, clear it and return it.
 */
static
struct thin_file *
new_thin(void)
{
    struct thin_file *thin;

	thin_files = reallocate(thin_files,
		      (nthin_files + 1) * sizeof(struct thin_file));
	thin = thin_files + nthin_files;
	nthin_files++;
	memset(thin, '\0', sizeof(struct thin_file));
	return(thin);
}

/*
 * Create a new arch_flag struct on the specified list, clear it and return it.
 */
static
struct arch_flag *
new_arch_flag(
struct arch_flag **arch_flags,
uint32_t *narch_flags)
{
    struct arch_flag *arch_flag;

	*arch_flags = reallocate(*arch_flags,
		      (*narch_flags + 1) * sizeof(struct arch_flag));
	arch_flag = *arch_flags + *narch_flags;
	*narch_flags = *narch_flags + 1;
	memset(arch_flag, '\0', sizeof(struct arch_flag));
	return(arch_flag);
}

/*
 * Create a new replace struct, clear it and return it.
 */
static
struct replace *
new_replace(void)
{
    struct replace *replace;

	replaces = reallocate(replaces,
		      (nreplaces + 1) * sizeof(struct replace));
	replace = replaces + nreplaces;
	nreplaces++;
	memset(replace, '\0', sizeof(struct replace));
	return(replace);
}

/*
 * Create a new segalign struct, clear it and return it.
 */
static
struct segalign *
new_segalign(void)
{
    struct segalign *segalign;

	segaligns = reallocate(segaligns,
		      (nsegaligns + 1) * sizeof(struct segalign));
	segalign = segaligns + nsegaligns;
	nsegaligns++;
	memset(segalign, '\0', sizeof(struct segalign));
	return(segalign);
}

/*
 * Function for qsort for comparing thin file's alignment
 */
static
int
cmp_qsort(
const struct thin_file *thin1,
const struct thin_file *thin2)
{
	/* if cpu types match, sort by cpu subtype */
	if (thin1->cputype == thin2->cputype)
	    return thin1->cpusubtype - thin2->cpusubtype;

	/* force arm64-family to follow after all other slices */
	if (thin1->cputype == CPU_TYPE_ARM64)
	    return 1;
	if (thin2->cputype == CPU_TYPE_ARM64)
	    return -1;

	/* sort all other cpu types by alignment */
	return thin1->align - thin2->align;
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
 * check_arch is called when an input file is specified with a -arch flag input
 * and that the architecture can be determined from the input to check that
 * both architectures match.
 */
static
void
check_arch(
struct input_file *input,
struct thin_file *thin)
{
	if(input->arch_flag.cputype != thin->cputype)
	    fatal("specifed architecture type (%s) for file (%s) does "
		  "not match its cputype (%d) and cpusubtype (%d) "
		  "(should be cputype (%d) and cpusubtype (%d))",
		  input->arch_flag.name, input->name,
		  thin->cputype, thin->cpusubtype &
		  ~CPU_SUBTYPE_MASK, input->arch_flag.cputype,
		  input->arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK);
}

/*
 * Create a blank dylib.  This is a stub dylib with no load commands.
 * It is a target page size block of bytes of zero except for the mach_header.
 */
static
struct thin_file *
new_blank_dylib(
struct arch_flag *arch)
{
    uint32_t target_page_size, align, onebit;
    struct thin_file *file;
    enum byte_sex host_byte_sex, target_byte_sex;
    struct mach_header *mh;
    struct mach_header_64 *mh64;

	file = new_thin();
	file->name = "blank dylib";
	target_page_size = get_segalign_from_flag(arch);
	file->addr = allocate(target_page_size);
	memset(file->addr, '\0', target_page_size);
	file->cputype = arch->cputype;
	file->cpusubtype = arch->cpusubtype;
	file->offset = 0;
	file->size = target_page_size;
	onebit = 1;
	for(align = 1; (target_page_size & onebit) != onebit; align++)
	   onebit = onebit << 1;
	file->align = align;
    
	host_byte_sex = get_host_byte_sex();
	target_byte_sex = get_byte_sex_from_flag(arch);

	if((arch->cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
	    mh64 = (struct mach_header_64 *)file->addr;
	    mh64->magic = MH_MAGIC_64;
	    mh64->cputype = arch->cputype;
	    mh64->cpusubtype = arch->cpusubtype;
	    mh64->filetype = MH_DYLIB_STUB;
	    if(target_byte_sex != host_byte_sex)
		swap_mach_header_64(mh64, target_byte_sex);
	}
	else{
	    mh = (struct mach_header *)file->addr;
	    mh->magic = MH_MAGIC;
	    mh->cputype = arch->cputype;
	    mh->cpusubtype = arch->cpusubtype;
	    mh->filetype = MH_DYLIB_STUB;
	    if(target_byte_sex != host_byte_sex)
		swap_mach_header(mh, target_byte_sex);
	}
	return(file);
}

/*
 * Print the current usage line and exit.
 */
static
void
usage(void)
{
    fprintf(stderr,
"usage: lipo <input_file> <command> [<options> ...]\n"
"  command is one of:\n"
"    -archs\n"
"    -create [-arch_blank <arch_type>]\n"
"    -detailed_info\n"
"    -extract <arch_type> [-extract <arch_type> ...]\n"
"    -extract_family <arch_type> [-extract_family <arch_type> ...]\n"
"    -info\n"
"    -remove <arch_type> [-remove <arch_type> ...]\n"
"    -replace <arch_type> <file_name> [-replace <arch_type> <file_name> ...]\n"
"    -thin <arch_type>\n"
"    -verify_arch <arch_type> ...\n"
"  options are one or more of:\n"
"    -arch <arch_type> <input_file>\n"
"    -hideARM64\n"
"    -output <output_file>\n"
"    -segalign <arch_type> <alignment>\n"
            );
    exit(EXIT_FAILURE);
}
