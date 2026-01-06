/*
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
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
 * The NeXT Computer, Inc. libtool(1) program that handles fat files, archives
 * and Mach-O objects files (no 4.3bsd a.out files).  This is also the ranlib(1)
 * program.
 */
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include <libc.h>
#ifndef __OPENSTEP__
#include <time.h>
#include <utime.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ar.h>
#include <mach-o/ranlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h> /* cctools-port: gettimeofday() */
#include <unistd.h>
#include "stuff/args.h"
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/rnd.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/execute.h"
#include "stuff/version_number.h"
#include "stuff/unix_standard_mode.h"
#include "stuff/write64.h"
#include "stuff/port.h" /* cctools-port */
#ifdef LTO_SUPPORT
#include "stuff/lto.h"
#endif /* LTO_SUPPORT */

#include <mach/mach_init.h>
#if defined(__OPENSTEP__) || defined(__GONZO_BUNSEN_BEAKER__)
#include <servers/netname.h>
#else
#include <servers/bootstrap.h>
#endif


/*
 * This is used internally to build the table of contents.
 */
struct toc {
    char *name;		/* symbol defined by */
    int64_t index1;	/* library member at this index plus 1 */
};

/* used by error routines as the name of the program */
char *progname = NULL;

/* the bytesex of the host this program is running on */
static enum byte_sex host_byte_sex = UNKNOWN_BYTE_SEX;

/*
 * toc_time holds the time_t value the archive contents
 *
 * toc_timeval, and toc_timespec hold the time value the archive contents are
 * set to, as well as the modification of the output file. Which value is used
 * depends on the deployment operating system. For modern macOS systems (e.g.)
 * nanosecond-precise toc_timespec will be used, whereas older macOS systems
 * will use microsecond-precise toc_timeval; toc_time will be used for
 * historical OS versions.
 *
 * toc_uid, toc_gid, and toc_mode similarly hold the the uid, gid, and file mode
 * values for the archive contents.
 *
 * all of these values are initialized to reasonable defaults for deterministic
 * archives: archives that are consistent regardless of user, time, or umask
 * differences. Ordinarily, these values and will be set to more specific
 * ones when building new archives, but that can be suppressed using the
 * '-D' option or the ZERO_AR_DATE environment variable.
 */
static time_t toc_time = 0;
#ifndef __OPENSTEP__
static struct timeval toc_timeval;
static struct timespec toc_timespec;
#endif /* !defined(__OPENSTEP__) */
static uid_t toc_uid = 0;
static gid_t toc_gid = 0;
static u_short toc_mode = 0100644;

/*
 * The environment variable ZERO_AR_DATE is used here and other places that
 * write archives to allow testing and comparing things for exact binary
 * equality.
 */
static enum bool zero_ar_date = FALSE;

/* flags set from the command line arguments */
struct cmd_flags {
    char **files;	/* array of file name arguments */
    uint32_t
	nfiles;		/* number of file name arguments */
    char **filelist;	/* filelist argument the file name argument came from */
    enum bool
	no_files_ok;	/* ok to see no files */
    enum bool ranlib;	/* set if this is run as ranlib not libtool */
    enum bool s;	/* sort the table of contents */
    enum bool a;	/* don't sort the table of contents (original form) */
    enum bool c;	/* include commmon symbols in the table of contents */
    enum bool t;	/* just "touch" the archives to get the date right */
    enum bool f;	/* warn if the output archive is fat,used by ar(1) -s */
    enum bool q;	/* only write archive if NOT fat, used by ar(1) */
    enum bool D;	/* write deterministic archive files */
    char *output;	/* the output file specified by -o */
    enum bool final_output_specified; /* if -final_output is specified */
    enum bool dynamic;	/* create a dynamic shared library, static by default */
    char *compatibility;/* compatibility version if specified, NULL otherwise */
    char *current;	/* current version if specified, NULL otherwise */
    char *install_name; /* install name if specified, NULL otherwise */
    char *seg1addr;	/* seg1addr if specified, NULL otherwise */
    char *segs_read_only_addr;	/* segs_read_only_addr if specified, or NULL */
    char *segs_read_write_addr;	/* segs_read_write_addr if specified, or NULL */
    char *seg_addr_table;	/* seg_addr_table if specified, or NULL */
    char *seg_addr_table_filename;
			/* seg_addr_table_filename if specified, or NULL */
    char **Ldirs;	/* array of -Ldir arguments */
    uint32_t
	nLdirs;		/* number of -Ldir arguments */
    char **ldflags;	/* other ld(1) flags to pass */
    uint32_t
	nldflags;	/* number of ld(1) flags for above */
    enum bool verbose;	/* print exec(2) commands run */
    struct arch_flag
	arch_only_flag;	/* the -arch_only flag if specified */
    enum bool		/* set if either -prebind or -noprebind is seen */
	prebinding_flag_specified;
    enum bool		/* set if -prebind is seen or the LD_PREBIND */
	prebinding;	/*  environment variable is set (and -noprebind isn't)*/
    enum bool		/* set if either -all_load or -noall_load is seen */
	all_load_flag_specified;
    enum bool		/* set if -all_load is seen (and -noall_load isn't) */
	all_load;
    enum bool		/* set with -L (the default) off with -T, for -static */
	use_long_names; /* use 4.4bsd extended format 1 for long names */
    enum bool L_or_T_specified;
    enum bool		/* set if the environ var LD_TRACE_ARCHIVES or        */
                        /* RC_TRACE_ARCHIVES is set                           */
	ld_trace_archives;
    enum bool           /* set if the environ var LD_TRACE_DEPENDENTS is set. */
                        /* Note that this value will take precedence over     */
                        /* ld_trace_archives.                                 */
        ld_trace_dependents;
    const char *	/* LD_TRACE_FILE if set and one of LD_TRACE_ARCHIVES, */
                        /* RC_TRACE_ARCHIVES, or LD_TRACE_DEPENDENTS is set,  */
                        /* or NULL.                                           */
	trace_file_path;
    enum bool		/* set if -search_paths_first is specified */
	search_paths_first;
    enum bool noflush;	/* don't use the output_flush routine to flush the
			   static library output file by pages */
    uint32_t debug;	/* debug value to debug output_flush() routine */
    enum bool		/* don't warn if members have no symbols */
	no_warning_for_no_symbols;
    enum bool toc64;	/* force the use of the 64-bit toc */
    enum bool fat64;	/* force the use of 64-bit fat files
			   when a fat is to be created */
};
static struct cmd_flags cmd_flags = { 0 };

/* The value of the environment variable NEXT_ROOT */
static char *next_root = NULL;

/* the standard directories to search for -lx names */
char *standard_dirs[] = {
    "/lib/",
    "/usr/lib/",
    "/usr/local/lib/",
    NULL
};

/*
 * The input files are broken down in to their object files and then placed in
 * these structures.  They are sorted by architecture type and then each object
 * has a member struct created for it in one of the arch structs.  All of these
 * structs hang off of 'archs'.
 */
static struct arch *archs = NULL;
static uint32_t narchs = 0;

struct arch {
    struct arch_flag arch_flag;	/* the identifing info of this architecture */
    uint64_t size;		/* current working size and final size */

    /* the table of contents (toc) stuff for this architecture in the library */
    uint32_t  toc_size;	/* total size of the toc including ar_hdr */
    struct ar_hdr  toc_ar_hdr;	/* the archive header for this member */
    enum bool toc_long_name;    /* use the long name in the output */
    char *toc_name;		/* name of toc member */
    uint32_t toc_name_size;/* size of name of toc member */
    struct toc    *tocs;	/* internal table of contents */
    enum bool using_64toc;	/* TRUE if we are using a 64-bit toc */
    struct ranlib *toc_ranlibs;	/* 32-bit ranlib structs for output */
    struct ranlib_64 *toc_ranlibs64;	/* 64-bit ranlib structs for output */
    uint64_t       toc_nranlibs;/* number of ranlib structs */
    char	  *toc_strings;	/* strings of symbol names for ranlib structs */
    uint64_t       toc_strsize;	/* number of bytes for the strings above */

    /* the members of this architecture in the library */
    struct member *members;	/* the members of the library for this arch */
    uint32_t nmembers;	/* the number of the above members */
};

struct member {
    uint64_t offset;	    	    /* current working offset and final offset*/
    struct ar_hdr ar_hdr;	    /* the archive header for this member */
    char null_byte;		    /* space to write '\0' for ar_hdr */
    char *object_addr;		    /* the address of the object file */
    uint32_t object_size;	    /* the size of the object file */
    enum byte_sex object_byte_sex;  /* the byte sex of the object file */
    struct mach_header *mh;	    /* the mach_header of 32-bit object files */
    struct mach_header_64 *mh64;    /* the mach_header of 64-bit object files */
    struct load_command		    /* the start of the load commands */
	*load_commands;
    struct symtab_command *st;	    /* the symbol table command */
    struct section **sections;	    /* array of section structs for 32-bit */
    struct section_64 **sections64; /* array of section structs for 64-bit */
#ifdef LTO_SUPPORT
    enum bool lto_contents;	    /* TRUE if this member has lto contents */
    uint32_t lto_toc_nsyms;	    /* number of symbols for the toc */
    uint32_t lto_toc_strsize;	    /* the size of the strings for the toc */
    char *lto_toc_strings;	    /* the strings of the symbols for the toc */
#endif /* LTO_SUPPORT */

    /* the name of the member in the output */
    char         *member_name;	    /* the member name */
    uint32_t member_name_size;	    /* the size of the member name */
    enum bool output_long_name;	    /* use the extended format #1 for the
				       member name in the output */

    /* info recorded from the input file this member came from */
    char	  *input_file_name;	/* the input file name */
    char	  *input_base_name;     /* the base name in the input file */
    uint32_t  input_base_name_size;	/* the size of the base name */
    struct ar_hdr *input_ar_hdr;
    uint64_t      input_member_offset;  /* if from a thin archive */
};

/*
 * trace_buffer points to a C string that will be written to the trace file, and
 * trace_buflen records the current length of the trace data string, but without
 * including the ASCII zero terminator. trace data will be written to the trace
 * file only when processing concludes.
 */
static char* trace_buffer = NULL;
static int trace_buflen = 0;

static void usage(
    void);
static void process(
    void);
static char *file_name_from_l_flag(
    char *l_flag);
static char *search_for_file(
    char *base_name);
static char * search_paths_for_lname(
    const char *lname_argument);
static char * search_path_for_lname(
    const char *dir,
    const char *lname_argument);
static void add_member(
    struct ofile *ofile);
static void free_archs(
    void);
static void create_library(
    char *output,
    struct ofile *ofile);
static enum byte_sex get_target_byte_sex(
    struct arch *arch,
    enum byte_sex host_byte_sex);
static char *put_toc_member(
    char *p,
    struct arch *arch,
    enum byte_sex host_byte_sex,
    enum byte_sex target_byte_sex);
static void create_dynamic_shared_library(
    char *output);
static void create_dynamic_shared_library_cleanup(
    int sig);
static void make_table_of_contents(
    struct arch *arch,
    char *output);
#ifdef LTO_SUPPORT
static void save_lto_member_toc_info(
    struct member *member,
    void *mod);
#endif /* LTO_SUPPORT */
static int toc_name_qsort(
    const struct toc *toc1,
    const struct toc *toc2);
static int toc_index1_qsort(
    const struct toc *toc1,
    const struct toc *toc2);
static enum bool toc_symbol(
    struct nlist *symbol,
    struct section **sections);
static enum bool toc_symbol_64(
    struct nlist_64 *symbol64,
    struct section_64 **sections64);
static enum bool toc(
    uint32_t n_strx,
    uint8_t n_type,
    uint64_t n_value,
    enum bool attr_no_toc);
static enum bool check_sort_tocs(
    struct arch *arch,
    char *output,
    enum bool library_warnings);
static void warn_duplicate_member_names(
    void);
static int member_name_qsort(
    const struct member *member1,
    const struct member *member2);
static int member_offset_qsort(
    const struct member *member1,
    const struct member *member2);
static void warn_member(
    struct arch *arch,
    struct member *member,
    const char *format, ...) __attribute__ ((format (printf, 3, 4)));
static void ld_trace_archive(const char* path);
static void ld_trace_close(void);
static void ld_trace_append(
     const char *format, ...) __attribute__ ((format (printf, 1, 2)));

/*
 * This structure is used to describe blocks of the output file that are flushed
 * to the disk file with output_flush.  It is kept in an ordered list starting
 * with output_blocks.
 */
static struct block {
    uint64_t offset;	/* starting offset of this block */
    uint64_t size;		/* size of this block */
    uint64_t written_offset;/* first page offset after starting offset */
    uint64_t written_size;	/* size of written area from written_offset */
    struct block *next; /* next block in the list */
} *output_blocks;

static void output_flush(
    char *library,
    uint64_t library_size,
    int fd,
    uint64_t offset,
    uint64_t size);
static void final_output_flush(
    char *library,
    int fd);
#ifdef DEBUG
static void print_block_list(void);
#endif /* DEBUG */
static struct block *get_block(void);
static void remove_block(
    struct block *block);
static uint64_t trnc64(
    uint64_t v,
    uint64_t r);

/* apple_version is in vers.c which is created by the libstuff/Makefile */
extern char apple_version[];

int
main(
int argc,
char **argv,
char **envp)
{
    char *p, *endp, *filelist, *dirname, *addr;
    int fd, i;
    struct stat stat_buf;
    uint32_t j, nfiles, maxfiles;
    uint32_t temp;
    int oumask, numask;
    enum bool bad_flag_seen, Vflag;

	Vflag = FALSE;
	progname = argv[0];

	host_byte_sex = get_host_byte_sex();

	/* see if this is being run as ranlib */
	/* cctools-port start*/
#if 0 /* old code */
	p = strrchr(argv[0], '/');
	if(p != NULL)
	    p++;
	else
	    p = argv[0];
	if(strncmp(p, "ranlib", sizeof("ranlib") - 1) == 0)
	    cmd_flags.ranlib = TRUE;
#endif
#ifdef RANLIB
    cmd_flags.ranlib = TRUE;
#endif
	/* cctools-port end */

	/* The default is to used long names */
	cmd_flags.use_long_names = TRUE;

	/* expand @file references in the options list */
	if (FALSE == cmd_flags.ranlib)
	    if (args_expand_at(&argc, &argv))
		exit(EXIT_FAILURE);

	/* process the command line arguments and collect the files */
	maxfiles = argc;
        cmd_flags.files = allocate(sizeof(char *) * maxfiles);
        cmd_flags.filelist = allocate(sizeof(char *) * maxfiles);
        memset(cmd_flags.filelist, '\0', sizeof(char *) * maxfiles);
	for(i = 1; i < argc; i++){
	    if(argv[i][0] == '-'){
		if(argv[i][1] == '\0'){
		    for(i += 1 ; i < argc; i++)
			cmd_flags.files[cmd_flags.nfiles++] = argv[i];
		    break;
		}
		if(strcmp(argv[i], "-o") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.output != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    cmd_flags.output = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-arch_only") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.arch_only_flag.name != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    else{
			if(get_arch_from_flag(argv[i+1],
					      &cmd_flags.arch_only_flag) == 0){
			    error("unknown architecture specification flag: "
				  "%s %s", argv[i], argv[i+1]);
			    arch_usage();
			    usage();
			}
		    }
		    i++;
		}
		else if(strcmp(argv[i], "-dynamic") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    cmd_flags.dynamic = TRUE;
		}
		else if(strcmp(argv[i], "-static") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    cmd_flags.dynamic = FALSE;
		}
		else if(strcmp(argv[i], "-filelist") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    filelist = argv[i + 1];
		    dirname = strrchr(filelist, ',');
		    if(dirname != NULL){
			*dirname = '\0';
			dirname++;
		    }
		    else
			dirname = "";
		    if((fd = open(filelist, O_RDONLY, 0)) == -1)
			system_fatal("can't open file list file: %s", filelist);
		    if(fstat(fd, &stat_buf) == -1)
			system_fatal("can't stat file list file: %s", filelist);
		    /*
		     * For some reason mapping files with zero size fails
		     * so it has to be handled specially.
		     */
		    addr = NULL;
		    if(stat_buf.st_size != 0){
			addr = mmap(0, stat_buf.st_size, PROT_READ|PROT_WRITE,
				    MAP_FILE|MAP_PRIVATE, fd, 0);
			if((intptr_t)addr == -1)
			    system_error("can't map file list file: %s",
				         filelist);
		    }
		    else{
			fatal("file list file: %s is empty", filelist);
		    }
		    if(*dirname != '\0')
			dirname[-1] = ',';
		    close(fd);
		    nfiles = 0;
		    for(j = 0; j < stat_buf.st_size; j++){
			if(addr[j] == '\n')
			    nfiles++;
		    }
		    if(addr[stat_buf.st_size - 1] != '\n')
			nfiles++;
		    p = allocate((strlen(dirname) + 1) * nfiles +
				 stat_buf.st_size);
		    cmd_flags.files = reallocate(cmd_flags.files,
					sizeof(char *) * (maxfiles + nfiles));
        	    cmd_flags.filelist = reallocate(cmd_flags.filelist,
					sizeof(char *) * (maxfiles + nfiles));
        	    memset(cmd_flags.filelist + maxfiles, '\0',
			   sizeof(char *) * nfiles);
		    maxfiles += nfiles;

		    cmd_flags.files[cmd_flags.nfiles] = p;
		    cmd_flags.filelist[cmd_flags.nfiles] = filelist;
		    cmd_flags.nfiles++;
		    if(*dirname != '\0'){
			strcpy(p, dirname);
			p += strlen(dirname);
			*p++ = '/';
		    }
		    for(j = 0; j < stat_buf.st_size; j++){
			if(addr[j] != '\n')
			    *p++ = addr[j];
			else{
			    *p++ = '\0';
			    if(j != stat_buf.st_size - 1){
				cmd_flags.files[cmd_flags.nfiles] = p;
				cmd_flags.filelist[cmd_flags.nfiles] =argv[i+1];
				cmd_flags.nfiles++;
				if(*dirname != '\0'){
				    strcpy(p, dirname);
				    p += strlen(dirname);
				    *p++ = '/';
				}
			    }
			}
		    }
		    if(addr[stat_buf.st_size - 1] != '\n')
			*p = '\0';
		    i++;
		}
		else if(strcmp(argv[i], "-compatibility_version") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.compatibility != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    if(get_version_number(argv[i], argv[i+1], &temp) == FALSE){
			usage();
		    }
		    cmd_flags.compatibility = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-current_version") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.current != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    if(get_version_number(argv[i], argv[i+1], &temp) == FALSE){
			usage();
		    }
		    cmd_flags.current = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-install_name") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.install_name != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    cmd_flags.install_name = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-seg1addr") == 0 ||
			strcmp(argv[i], "-image_base") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.seg1addr != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    temp = (uint32_t)strtoul(argv[i + 1], &endp, 16);
		    if(*endp != '\0'){
			error("address for -seg1addr %s not a proper "
			      "hexadecimal number", argv[i+1]);
			usage();
		    }
		    cmd_flags.seg1addr = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-segs_read_only_addr") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.segs_read_only_addr != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    temp = (uint32_t)strtoul(argv[i + 1], &endp, 16);
		    if(*endp != '\0'){
			error("address for -segs_read_only_addr %s not a "
			      "proper hexadecimal number", argv[i+1]);
			usage();
		    }
		    cmd_flags.segs_read_only_addr = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-segs_read_write_addr") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.segs_read_write_addr != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    temp = (uint32_t)strtoul(argv[i + 1], &endp, 16);
		    if(*endp != '\0'){
			error("address for -segs_read_write_addr %s not a "
			      "proper hexadecimal number", argv[i+1]);
			usage();
		    }
		    cmd_flags.segs_read_write_addr = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.seg_addr_table != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    cmd_flags.seg_addr_table = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table_filename") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(cmd_flags.seg_addr_table_filename != NULL){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    cmd_flags.seg_addr_table_filename = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-syslibroot") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 == argc){
			error("missing argument to: %s option", argv[i]);
			usage();
		    }
		    if(next_root != NULL && strcmp(next_root, argv[i+1]) != 0){
			error("more than one: %s option specified", argv[i]);
			usage();
		    }
		    next_root = argv[i+1];
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 2));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i+1];
		    i++;
		}
		else if(strcmp(argv[i], "-sectcreate") == 0 ||
		        strcmp(argv[i], "-segcreate") == 0 ||
		        strcmp(argv[i], "-sectorder") == 0 ||
		        strcmp(argv[i], "-sectalign") == 0 ||
		        strcmp(argv[i], "-segprot") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 3 >= argc){
			error("not enough arguments follow %s", argv[i]);
			usage();
		    }
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 4));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i+1];
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i+2];
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i+3];
		    if(strcmp(argv[i], "-sectcreate") == 0 ||
		       strcmp(argv[i], "-segcreate") == 0)
			cmd_flags.no_files_ok = TRUE;
		    i += 3;
		}
		else if(strcmp(argv[i], "-segalign") == 0 ||
		        strcmp(argv[i], "-undefined") == 0 ||
		        strcmp(argv[i], "-macosx_version_min") == 0 ||
		        strcmp(argv[i], "-ios_version_min") == 0 ||
		        strcmp(argv[i], "-ios_simulator_version_min") == 0 ||
		        strcmp(argv[i], "-watchos_version_min") == 0 ||
		        strcmp(argv[i], "-watchos_simulator_version_min") == 0 ||
		        strcmp(argv[i], "-tvos_version_min") == 0 ||
		        strcmp(argv[i], "-tvos_simulator_version_min") == 0 ||
		        strcmp(argv[i], "-multiply_defined") == 0 ||
		        strcmp(argv[i], "-multiply_defined_unused") == 0 ||
		        strcmp(argv[i], "-umbrella") == 0 ||
			strcmp(argv[i], "-sub_umbrella") == 0 ||
			strcmp(argv[i], "-sub_library") == 0 ||
			strcmp(argv[i], "-allowable_client") == 0 ||
		        strcmp(argv[i], "-read_only_relocs") == 0 ||
		        strcmp(argv[i], "-init") == 0 ||
		        strcmp(argv[i], "-U") == 0 ||
		        strcmp(argv[i], "-Y") == 0 ||
		        strcmp(argv[i], "-dylib_file") == 0 ||
		        strcmp(argv[i], "-final_output") == 0 ||
		        strcmp(argv[i], "-headerpad") == 0 ||
		        strcmp(argv[i], "-weak_reference_mismatches") == 0 ||
		        strcmp(argv[i], "-u") == 0 ||
		        strcmp(argv[i], "-exported_symbols_list") == 0 ||
		        strcmp(argv[i], "-unexported_symbols_list") == 0 ||
		        strcmp(argv[i], "-executable_path") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 >= argc){
			error("not enough arguments follow %s", argv[i]);
			usage();
		    }
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 2));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i+1];
		    if(strcmp(argv[i], "-final_output") == 0)
			cmd_flags.final_output_specified = TRUE;
		    i += 1;
		}
		else if(strcmp(argv[i], "-sectorder_detail") == 0 ||
		        strcmp(argv[i], "-Sn") == 0 ||
		        strcmp(argv[i], "-Si") == 0 ||
		        strcmp(argv[i], "-Sp") == 0 ||
		        strcmp(argv[i], "-S") == 0 ||
		        strcmp(argv[i], "-X") == 0 ||
		        strcmp(argv[i], "-x") == 0 ||
		        strcmp(argv[i], "-whatsloaded") == 0 ||
			strcmp(argv[i], "-whyload") == 0 ||
			strcmp(argv[i], "-arch_errors_fatal") == 0 ||
			strcmp(argv[i], "-run_init_lazily") == 0 ||
			strcmp(argv[i], "-twolevel_namespace") == 0 ||
			strcmp(argv[i], "-twolevel_namespace_hints") == 0 ||
			strcmp(argv[i], "-flat_namespace") == 0 ||
			strcmp(argv[i], "-nomultidefs") == 0 ||
			strcmp(argv[i], "-headerpad_max_install_names") == 0 ||
			strcmp(argv[i], "-prebind_all_twolevel_modules") == 0 ||
			strcmp(argv[i], "-prebind_allow_overlap") == 0 ||
			strcmp(argv[i], "-ObjC") == 0 ||
			strcmp(argv[i], "-M") == 0 ||
			strcmp(argv[i], "-t") == 0 ||
			strcmp(argv[i], "-single_module") == 0 ||
			strcmp(argv[i], "-multi_module") == 0 ||
			strcmp(argv[i], "-m") == 0 ||
			strcmp(argv[i], "-dead_strip") == 0 ||
			strcmp(argv[i], "-no_uuid") == 0 ||
			strcmp(argv[i], "-no_dead_strip_inits_and_terms") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 1));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		}
		else if(strcmp(argv[i], "-no_arch_warnings") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    /* ignore this flag */
		}
		else if(strcmp(argv[i], "-prebind") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(cmd_flags.prebinding_flag_specified == TRUE &&
		       cmd_flags.prebinding == FALSE){
			error("both -prebind and -noprebind can't be "
			      "specified");
			usage();
		    }
		    cmd_flags.prebinding_flag_specified = TRUE;
		    cmd_flags.prebinding = TRUE;
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 1));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		}
		else if(strcmp(argv[i], "-noprebind") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(cmd_flags.prebinding_flag_specified == TRUE &&
		       cmd_flags.prebinding == TRUE){
			error("both -prebind and -noprebind can't be "
			      "specified");
			usage();
		    }
		    cmd_flags.prebinding_flag_specified = TRUE;
		    cmd_flags.prebinding = FALSE;
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 1));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		}
		else if(strcmp(argv[i], "-all_load") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(cmd_flags.all_load_flag_specified == TRUE &&
		       cmd_flags.all_load == FALSE){
			error("both -all_load and -noall_load can't be "
			      "specified");
			usage();
		    }
		    cmd_flags.all_load_flag_specified = TRUE;
		    cmd_flags.all_load = TRUE;
		}
		else if(strcmp(argv[i], "-noall_load") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(cmd_flags.all_load_flag_specified == TRUE &&
		       cmd_flags.all_load == TRUE){
			error("both -all_load and -noall_load can't be "
			      "specified");
			usage();
		    }
		    cmd_flags.all_load_flag_specified = TRUE;
		    cmd_flags.all_load = FALSE;
		}
		else if(strncmp(argv[i], "-y", 2) == 0 ||
		        strncmp(argv[i], "-i", 2) == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(strncmp(argv[i], "-i", 2) == 0)
			cmd_flags.no_files_ok = TRUE;
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 1));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		}
		else if(argv[i][1] == 'l'){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(argv[i][2] == '\0'){
			error("-l: name missing");
			usage();
		    }
		    cmd_flags.files[cmd_flags.nfiles++] = argv[i];
		}
		else if(strncmp(argv[i], "-weak-l", 7) == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(argv[i][7] == '\0'){
			error("-weak-l: name missing");
			usage();
		    }
		    cmd_flags.files[cmd_flags.nfiles++] = argv[i];
		}
		else if(strcmp(argv[i], "-framework") == 0 ||
		        strcmp(argv[i], "-weak_framework") == 0 ||
		        strcmp(argv[i], "-weak_library") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    if(i + 1 >= argc){
			error("not enough arguments follow %s", argv[i]);
			usage();
		    }
		    cmd_flags.files[cmd_flags.nfiles++] = argv[i];
		    cmd_flags.files[cmd_flags.nfiles++] = argv[i+1];
		    i += 1;
		}
		else if(strcmp(argv[i], "-T") == 0){
		    if(cmd_flags.L_or_T_specified == TRUE){
			error("both -T and -L can't be specified");
			usage();
		    }
		    cmd_flags.L_or_T_specified = TRUE;
		    cmd_flags.use_long_names = FALSE;
		}
		else if(argv[i][1] == 'L' || argv[i][1] == 'F'){
		    if(argv[i][1] == 'L' && argv[i][2] == '\0'){
			if(cmd_flags.L_or_T_specified == TRUE){
			    error("both -T and -L can't be specified");
			    usage();
			}
			cmd_flags.L_or_T_specified = TRUE;
			cmd_flags.use_long_names = TRUE;
		    }
		    else{
			if(cmd_flags.ranlib == TRUE){
			    error("unknown option: %s", argv[i]);
			    usage();
			}
			cmd_flags.Ldirs = realloc(cmd_flags.Ldirs,
				    sizeof(char *) * (cmd_flags.nLdirs + 1));
			cmd_flags.Ldirs[cmd_flags.nLdirs++] = argv[i];
		    }
		}
		else if(argv[i][1] == 'g'){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    /* We need to ignore -g[gdb,codeview,stab][number] flags */
			;
		}
		else if(strcmp(argv[i], "-pg") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    /* We need to ignore -pg */
			;
		}
		else if(strcmp(argv[i], "-search_paths_first") == 0){
		    if(cmd_flags.ranlib == TRUE){
			error("unknown option: %s", argv[i]);
			usage();
		    }
		    cmd_flags.search_paths_first = TRUE;
		    cmd_flags.ldflags = reallocate(cmd_flags.ldflags,
				sizeof(char *) * (cmd_flags.nldflags + 1));
		    cmd_flags.ldflags[cmd_flags.nldflags++] = argv[i];
		}
		else if(strcmp(argv[i], "-noflush") == 0){
		    cmd_flags.noflush = TRUE;
		}
		else if(strcmp(argv[i], "-no_warning_for_no_symbols") == 0){
		    cmd_flags.no_warning_for_no_symbols = TRUE;
		}
		else if(strcmp(argv[i], "-toc64") == 0){
		    cmd_flags.toc64 = TRUE;
		}
		else if(strcmp(argv[i], "-fat64") == 0){
		    cmd_flags.fat64 = TRUE;
		}
#ifdef DEBUG
		else if(strcmp(argv[i], "-debug") == 0){
		    if(i + 1 >= argc){
			error("not enough arguments follow %s", argv[i]);
			usage();
		    }
		    i++;
		    cmd_flags.debug |= 1 << strtoul(argv[i], &endp, 10);
		    if(*endp != '\0' || strtoul(argv[i], &endp, 10) > 32)
			fatal("argument for -debug %s not a proper "
			      "decimal number less than 32", argv[i]);
		}
#endif /* DEBUG */
		else{
		    for(j = 1; argv[i][j] != '\0'; j++){
			switch(argv[i][j]){
			case 's':
			    cmd_flags.s = TRUE;
			    break;
			case 'a':
			    cmd_flags.a = TRUE;
			    break;
			case 'c':
			    cmd_flags.c = TRUE;
			    break;
			case 'v':
			    if(cmd_flags.ranlib == TRUE){
				error("unknown option character `%c' in: %s",
				      argv[i][j], argv[i]);
				usage();
			    }
			    cmd_flags.verbose= TRUE;
			    break;
			case 'V':
			    printf("Apple Inc. version %s\n", apple_version);
			    Vflag = TRUE;
			    break;
			case 't':
			    if(cmd_flags.ranlib == TRUE){
				warning("touch option (`%c' in: %s) ignored "
					"(table of contents rebuilt anyway)",
					argv[i][j], argv[i]);
				cmd_flags.t = TRUE;
				break;
			    }
			    else {
				error("unknown option character `%c' in: %s",
				      argv[i][j], argv[i]);
				usage();
			    }
			case 'f':
			    if(cmd_flags.ranlib == TRUE){
				cmd_flags.f = TRUE;
				break;
			    }
			    else {
				error("unknown option character `%c' in: %s",
				      argv[i][j], argv[i]);
				usage();
			    }
			case 'q':
			    if(cmd_flags.ranlib == TRUE){
				cmd_flags.q = TRUE;
				break;
			    }
			    else {
				error("unknown option character `%c' in: %s",
				      argv[i][j], argv[i]);
				usage();
			    }
                        case 'D':
                            cmd_flags.D = TRUE;
                            break;
			default:
			    error("unknown option character `%c' in: %s",
				  argv[i][j], argv[i]);
			    usage();
			}
		    }
		}
	    }
	    else
		cmd_flags.files[cmd_flags.nfiles++] = argv[i];
	}
    
	/*
         * Test to see if one of the following trace environment variables are
         * set:
         *
         *     LC_TRACE_DEPENDENTS
         *     RC_TRACE_ARCHIVES
         *     LC_TRACE_ARCHIVES
         *
         * If so, also get the LD_TRACE_FILE.
         */
        if (getenv("LD_TRACE_DEPENDENTS") != NULL) {
            cmd_flags.ld_trace_dependents = TRUE;
            cmd_flags.ld_trace_archives = TRUE;
            cmd_flags.trace_file_path = getenv("LD_TRACE_FILE");
        }
        else if ((getenv("RC_TRACE_ARCHIVES") != NULL) ||
            (getenv("LD_TRACE_ARCHIVES") != NULL)) {
            cmd_flags.ld_trace_archives = TRUE;
            cmd_flags.trace_file_path = getenv("LD_TRACE_FILE");
        }

        /*
         * The environment variable ZERO_AR_DATE is used here and other
         * places that write archives to allow testing and comparing
         * things for exact binary equality.
         */
        if(getenv("ZERO_AR_DATE") == NULL)
            zero_ar_date = FALSE;
        else
            zero_ar_date = TRUE;

	/*
	 * If either -syslibroot or the environment variable NEXT_ROOT is set
	 * prepend it to the standard paths for library searches.  This was
	 * added to ease cross build environments.
	 */
	if(next_root != NULL){
	    if(getenv("NEXT_ROOT") != NULL)
		warning("NEXT_ROOT environment variable ignored because "
			"-syslibroot specified");
	}
	else{
	    next_root = getenv("NEXT_ROOT");
	}
	if(next_root != NULL){
	    for(i = 0; standard_dirs[i] != NULL; i++){
		p = allocate(strlen(next_root) +
			     strlen(standard_dirs[i]) + 1);
		strcpy(p, next_root);
		strcat(p, standard_dirs[i]);
		standard_dirs[i] = p;
	    }
	    for(i = 0; i < cmd_flags.nLdirs ; i++){
		if(cmd_flags.Ldirs[i][1] != 'L')
		    continue;
		if(cmd_flags.Ldirs[i][2] == '/'){
		    p = makestr(next_root, cmd_flags.Ldirs[i] + 2, NULL);
		    if(access(p, F_OK) != -1){
			free(p);
			p = makestr("-L", next_root, cmd_flags.Ldirs[i] + 2,
				    NULL);
			cmd_flags.Ldirs[i] = p;
		    }
		    else{
			free(p);
		    }
		}
	    }
	}

	/* check the command line arguments for correctness */
	if(cmd_flags.ranlib == FALSE && cmd_flags.dynamic == TRUE){
	    if(cmd_flags.s == TRUE){
		warning("-static not specified, -s invalid");
	    }
	    if(cmd_flags.a == TRUE){
		warning("-static not specified, -a invalid");
	    }
	    if(cmd_flags.c == TRUE){
		warning("-static not specified, -c invalid");
	    }
	    if(cmd_flags.L_or_T_specified == TRUE){
		if(cmd_flags.use_long_names == TRUE)
		    warning("-static not specified, -L invalid");
		else
		    warning("-static not specified, -T invalid");
	    }
	}
	if(cmd_flags.s == TRUE && cmd_flags.a == TRUE){
	    error("only one of -s or -a can be specified");
	    usage();
	}
	if(cmd_flags.ranlib == FALSE && cmd_flags.output == NULL){
	    if(Vflag == TRUE)
		exit(EXIT_SUCCESS);
	    error("no output file specified (specify with -o output)");
	    usage();
	}
	if(cmd_flags.dynamic == FALSE){
	    if(cmd_flags.compatibility != NULL){
		warning("-dynamic not specified, -compatibility_version %s "
		      "invalid", cmd_flags.compatibility);
	    }
	    if(cmd_flags.current != NULL){
		warning("-dynamic not specified, -current_version %s invalid",
		      cmd_flags.current);
	    }
	    if(cmd_flags.install_name != NULL){
		warning("-dynamic not specified, -install_name %s invalid",
		      cmd_flags.install_name);
	    }
	    if(cmd_flags.seg1addr != NULL){
		warning("-dynamic not specified, -seg1addr %s invalid",
		      cmd_flags.seg1addr);
	    }
	    if(cmd_flags.segs_read_only_addr != NULL){
		warning("-dynamic not specified, -segs_read_only_addr %s "
			"invalid", cmd_flags.segs_read_only_addr);
	    }
	    if(cmd_flags.segs_read_write_addr != NULL){
		warning("-dynamic not specified, -segs_read_write_addr %s "
			"invalid", cmd_flags.segs_read_write_addr);
	    }
	    if(cmd_flags.seg_addr_table != NULL){
		warning("-dynamic not specified, -seg_addr_table %s "
			"invalid", cmd_flags.seg_addr_table);
	    }
	    if(cmd_flags.seg_addr_table_filename != NULL){
		warning("-dynamic not specified, -seg_addr_table_filename %s "
			"invalid", cmd_flags.seg_addr_table_filename);
	    }
	    if(cmd_flags.all_load_flag_specified == TRUE){
		if(cmd_flags.all_load == TRUE)
		    warning("-dynamic not specified, -all_load invalid");
		else
		    warning("-dynamic not specified, -noall_load invalid");
	    }
	    if(cmd_flags.nldflags != 0){
		bad_flag_seen = FALSE;
		for(j = 0; j < cmd_flags.nldflags; j++){
		    if(strcmp(cmd_flags.ldflags[j], "-syslibroot") == 0){
			j++;
			continue;
		    }
		    if(bad_flag_seen == FALSE){
			fprintf(stderr, "%s: -dynamic not specified the "
				"following flags are invalid: ", progname);
			bad_flag_seen = TRUE;
		    }
		    fprintf(stderr, "%s ", cmd_flags.ldflags[j]);
		}
		if(bad_flag_seen == TRUE)
		    fprintf(stderr, "\n");
	    }
	    if(cmd_flags.nLdirs != 0){
		/* Note: both -L and -F flags are in cmd_flags.Ldirs to keep the
		   search order right. */
		bad_flag_seen = FALSE;
		for(j = 0; j < cmd_flags.nLdirs; j++){
		    if(strncmp(cmd_flags.Ldirs[j], "-L", 2) == 0)
			continue;
		    if(bad_flag_seen == FALSE){
			fprintf(stderr, "%s: -dynamic not specified the "
				"following flags are invalid: ", progname);
			bad_flag_seen = TRUE;
		    }
		    fprintf(stderr, "%s ", cmd_flags.Ldirs[j]);
		}
		if(bad_flag_seen == TRUE)
		    fprintf(stderr, "\n");
	    }
	}
	else{
	    /*
	     * The -prebind flag can also be specified with the LD_PREBIND
	     * environment variable.
	     */
	    if(getenv("LD_PREBIND") != NULL){
		if(cmd_flags.prebinding_flag_specified == TRUE &&
		   cmd_flags.prebinding == FALSE){
		    warning("LD_PREBIND environment variable ignored because "
			    "-noprebind specified");
		}
		else{
		    cmd_flags.prebinding_flag_specified = TRUE;
		    cmd_flags.prebinding = TRUE;
		}
	    }
	}
	if(cmd_flags.nfiles == 0){
	    if(cmd_flags.ranlib == TRUE){
		error("no archives specified");
		usage();
	    }
	    else{
		if(cmd_flags.dynamic == TRUE && cmd_flags.no_files_ok == TRUE)
		    warning("warning no files specified");
		else{
		    error("no files specified");
		    usage();
		}
	    }
	}

	/* set the defaults if not specified */
	if(cmd_flags.a == FALSE)
	    cmd_flags.s = TRUE; /* sort table of contents by default */

	/* remember common values used in the archive table of contents */
	if (cmd_flags.D == FALSE && zero_ar_date == FALSE) {
#ifndef __OPENSTEP__
		/* cctools-port: replaced __builtin_available */
#if 0
	    if (__builtin_available(macOS 10.12, *)) {
#endif /* 0*/ 
#ifdef HAVE_CLOCK_GETTIME
		if (clock_gettime(CLOCK_REALTIME, &toc_timespec)) {
		    system_fatal("clock_gettime failed");
		    return(EXIT_FAILURE);
		}
		toc_time = toc_timespec.tv_sec;
#endif /* HAVE_CLOCK_GETTIME */
#if 0
	    } else {
#endif /* 0 */
#ifndef HAVE_CLOCK_GETTIME
		if (gettimeofday(&toc_timeval, NULL)) {
		    system_fatal("gettimeofday failed");
		    return(EXIT_FAILURE);
		}
		toc_time = toc_timeval.tv_sec;
#endif /* !HAVE_CLOCK_GETTIME */
#if 0
	    }
#endif /* 0 */
#else
	    toc_time = time(NULL);
#endif /* !defined(__OPENSTEP__) */
	}
	if (cmd_flags.D == FALSE) {
	    toc_uid = getuid();
	    toc_gid = getgid();
	    
	    numask = 0;
	    oumask = umask(numask);
	    toc_mode = S_IFREG | (0666 & ~oumask);
	    (void)umask(oumask);
	}

	process();

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
	if(cmd_flags.ranlib)
	    fprintf(stderr, "Usage: %s [-sactfqLT] [-] archive [...]\n",
		    progname);
	else{
	    fprintf(stderr, "Usage: %s -static [-] file [...] "
		    "[-filelist listfile[,dirname]] [-arch_only arch] "
		    "[-sacLT] [-no_warning_for_no_symbols]\n", progname);
	    fprintf(stderr, "Usage: %s -dynamic [-] file [...] "
		    "[-filelist listfile[,dirname]] [-arch_only arch] "
		    "[-o output] [-install_name name] "
		    "[-compatibility_version #] [-current_version #] "
		    "[-seg1addr 0x#] [-segs_read_only_addr 0x#] "
		    "[-segs_read_write_addr 0x#] [-seg_addr_table <filename>] "
		    "[-seg_addr_table_filename <file_system_path>] "
		    "[-all_load] [-noall_load]\n",
		    progname);
	}
	exit(EXIT_FAILURE);
}

/*
 * process() the input files into libraries based on the command flags.
 */
static
void
process(
void)
{
    uint32_t i, j, k, previous_errors;
    struct ofile *ofiles;
    char *file_name;
    enum bool flag, ld_trace_archive_printed;
    
	/*
	 * For libtool processing put all input files in the specified output
	 * file.  For ranlib processing all input files should be archives or
	 * fat files with archives in them and each is processed by itself and
	 * not combined with anything else.  The format of fat object files in
	 * a thin archive is supported here also.
	 */
	ofiles = allocate(sizeof(struct ofile) * cmd_flags.nfiles);
	for(i = 0; i < cmd_flags.nfiles; i++){
	    if(strncmp(cmd_flags.files[i], "-l", 2) == 0 ||
	       strncmp(cmd_flags.files[i], "-weak-l", 7) == 0){
		if(cmd_flags.dynamic == TRUE)
		    continue;
		file_name = file_name_from_l_flag(cmd_flags.files[i]);
		if(file_name != NULL)
		    if(ofile_map(file_name, NULL, NULL, ofiles + i, TRUE) ==
		       FALSE)
			continue;
	    }
	    else if(strcmp(cmd_flags.files[i], "-framework") == 0 ||
		    strcmp(cmd_flags.files[i], "-weak_framework") == 0 ||
		    strcmp(cmd_flags.files[i], "-weak_library") == 0){
		i++;
		continue;
	    }
	    else{
		if(ofile_map(cmd_flags.files[i], NULL, NULL, ofiles + i,
			     TRUE) == FALSE)
		    continue;
	    }

	    previous_errors = errors;
	    errors = 0;
	    ld_trace_archive_printed = FALSE;

	    if(ofiles[i].file_type == OFILE_FAT){
		(void)ofile_first_arch(ofiles + i);
                do{
                    if(ofiles[i].arch_type == OFILE_ARCHIVE){
                        if (cmd_flags.ld_trace_archives == TRUE &&
                            cmd_flags.dynamic == FALSE &&
                            ld_trace_archive_printed == FALSE){
                            ld_trace_archive(ofiles[i].file_name);
                            ld_trace_archive_printed = TRUE;
			}
			/* loop through archive */
			if((flag = ofile_first_member(ofiles + i)) == TRUE){
			    if(ofiles[i].member_ar_hdr != NULL &&
			       strncmp(ofiles[i].member_name, SYMDEF,
				       sizeof(SYMDEF) - 1) == 0)
				flag = ofile_next_member(ofiles + i);
			    while(flag == TRUE){
				/* No fat members in a fat file */
				if(ofiles[i].mh != NULL ||
				   ofiles[i].mh64 != NULL ||
#ifdef LTO_SUPPORT
				   ofiles[i].lto != NULL ||
#endif /* LTO_SUPPORT */
				   cmd_flags.ranlib == TRUE)
				    add_member(ofiles + i);
				else{
				    error("for architecture: %s file: %s(%.*s) "
					  "is not an object file (not allowed "
					  "in a library)",
					  ofiles[i].arch_flag.name,
					  cmd_flags.files[i],
					  (int)ofiles[i].member_name_size,
					  ofiles[i].member_name);
				}
				flag = ofile_next_member(ofiles + i);
			    }
			}
		    }
		    else if(ofiles[i].arch_type == OFILE_Mach_O
#ifdef LTO_SUPPORT
			    || ofiles[i].arch_type == OFILE_LLVM_BITCODE
#endif
			   ){
			if(cmd_flags.ranlib == TRUE){
			    error("for architecture: %s file: %s is not an "
				  "archive (no processing done on this file)",
				  ofiles[i].arch_flag.name, cmd_flags.files[i]);
			    goto ranlib_fat_error;
			}
			else
			    add_member(ofiles + i);
		    }
		    else if(ofiles[i].arch_type == OFILE_UNKNOWN){
			if(cmd_flags.ranlib == TRUE){
			    error("for architecture: %s file: %s is not an "
				  "archive (no processing done on this file)",
				  ofiles[i].arch_flag.name, cmd_flags.files[i]);
			    goto ranlib_fat_error;
			}
			else{
			    error("for architecture: %s file: %s is not an "
				  "object file (not allowed in a library)",
				  ofiles[i].arch_flag.name, cmd_flags.files[i]);
			}
		    }
		}while(ofile_next_arch(ofiles + i) == TRUE);
	    }
	    else if(ofiles[i].file_type == OFILE_ARCHIVE){
                if (cmd_flags.ld_trace_archives == TRUE &&
                    cmd_flags.dynamic == FALSE &&
                    ld_trace_archive_printed == FALSE){
                    ld_trace_archive(ofiles[i].file_name);
                    ld_trace_archive_printed =
                        TRUE;
                }
		/* loop through archive */
		if((flag = ofile_first_member(ofiles + i)) == TRUE){
		    if(ofiles[i].member_ar_hdr != NULL &&
		       strncmp(ofiles[i].member_name, SYMDEF,
			       sizeof(SYMDEF) - 1) == 0){
			flag = ofile_next_member(ofiles + i);
		    }
		    while(flag == TRUE){
			/* incorrect form: archive with fat object members */
			if(ofiles[i].member_type == OFILE_FAT){
			    (void)ofile_first_arch(ofiles + i);
			    do{
				if(ofiles[i].mh != NULL ||
				   ofiles[i].mh64 != NULL ||
				   ofiles[i].lto != NULL ||
				   cmd_flags.ranlib == TRUE){
				    add_member(ofiles + i);
				}
				else{
				    /*
				     * Can't really get here because ofile_*()
				     * routines will refuse to process this
				     * type of file (but I'll leave it here).
				     */
				    error("file: %s(%.*s) for architecture: %s "
					"is not an object file (not allowed in "
					"a library)", cmd_flags.files[i],
					(int)ofiles[i].member_name_size,
					ofiles[i].member_name,
					ofiles[i].arch_flag.name);
				}

			    }while(ofile_next_arch(ofiles + i) == TRUE);
			}
			else if(ofiles[i].mh != NULL ||
			        ofiles[i].mh64 != NULL ||
#ifdef LTO_SUPPORT
			        ofiles[i].lto != NULL ||
#endif /* LTO_SUPPORT */
				cmd_flags.ranlib == TRUE){
			    add_member(ofiles + i);
			}
			else{
			    error("file: %s(%.*s) is not an object file (not "
				  "allowed in a library)", cmd_flags.files[i],
				  (int)ofiles[i].member_name_size,
				  ofiles[i].member_name);
			}
			flag = ofile_next_member(ofiles + i);
		    }
		}
	    }
	    else if(ofiles[i].file_type == OFILE_Mach_O){
		if(cmd_flags.ranlib == TRUE){
		    error("file: %s is not an archive", cmd_flags.files[i]);
		    continue;
		}
		add_member(ofiles + i);
	    }
#ifdef LTO_SUPPORT
	    else if(ofiles[i].file_type == OFILE_LLVM_BITCODE){
		if(cmd_flags.ranlib == TRUE){
		    error("file: %s is not an archive", cmd_flags.files[i]);
		    continue;
		}
		add_member(ofiles + i);
	    }
#endif /* LTO_SUPPORT */
	    else{ /* ofiles[i].file_type == OFILE_UNKNOWN */
		if(cmd_flags.ranlib == TRUE){
		    error("file: %s is not an archive", cmd_flags.files[i]);
		    continue;
		}
		else{
		    error("file: %s is not an object file (not allowed in a "
			  "library)", cmd_flags.files[i]);
		}
	    }
            
	    if(cmd_flags.ranlib == TRUE){
		/*
		 * In the case where ranlib is being used on an archive that
		 * contains fat object files with multiple members and non-
		 * object members this has to be treated as an error because
		 * it is not known which architecture(s) the non-object file
		 * belong to.
		 */
		if(narchs > 1){
		    for(j = 0; j < narchs; j++){
			for(k = 0; k < archs[j].nmembers; k++){
			    if(archs[j].members[k].mh == NULL &&
#ifdef LTO_SUPPORT
			       archs[j].members[k].lto_contents == FALSE &&
#endif /* LTO_SUPPORT */
			       archs[j].members[k].mh64 == NULL){
				error("library member: %s(%.*s) is not an "
				      "object file (not allowed in a library "
				      "with multiple architectures)",
				      cmd_flags.files[i],
				      (int)archs[j].members[k].
					input_base_name_size,
				      archs[j].members[k].input_base_name);
			    }
			}
		    }
		}
		if(errors == 0)
		    create_library(cmd_flags.files[i], ofiles + i);
		if(cmd_flags.nfiles > 1){
ranlib_fat_error:
		    free_archs();
		    ofile_unmap(ofiles + i);
		}
	    }
	    errors += previous_errors;
	}
	if(cmd_flags.ranlib == FALSE && errors == 0)
	    create_library(cmd_flags.output, NULL);

    	/* Finalize the trace log */
        if (cmd_flags.ld_trace_archives)
            ld_trace_close();

	/*
	 * Clean-up of ofiles[] and archs could be done here but since this
	 * program is now done it is faster to just exit.
	 */
}

/*
 * file_name_from_l_flag() is passed a "-lx" or "-weak-lx" flag and returns a
 * name of a file for this flag.  The flag "-lx" and "-weak-lx" are the same
 * flags as used in the link editor to refer to file names.  If it can't find a
 * file name for the flag it prints an error and returns NULL.
 */
static
char *
file_name_from_l_flag(
char *l_flag)
{
    char *file_name, *p, *start;

	if(strncmp(l_flag, "-weak-l", 7) == 0)
	    start = &l_flag[7];
	else
	    start = &l_flag[2];
	p = strrchr(start, '.');
	if(p != NULL && strcmp(p, ".o") == 0){
	    p = start;
	    file_name = search_for_file(p);
	}
	else{
	    file_name = NULL;
	    if(cmd_flags.dynamic == TRUE){
		if(cmd_flags.search_paths_first == TRUE){
		    file_name = search_paths_for_lname(start);
		}
		else{
		    p = makestr("lib", start, ".dylib", NULL);
		    file_name = search_for_file(p);
		    free(p);
		    if(file_name == NULL){
			p = makestr("lib", start, ".a", NULL);
			file_name = search_for_file(p);
			free(p);
		    }
		}
	    }
	    else{
		p = makestr("lib", start, ".a", NULL);
		file_name = search_for_file(p);
		free(p);
	    }
	}
	if(file_name == NULL)
	    error("can't locate file for: %s", l_flag);
	return(file_name);
}

/*
 * search_for_file() takes base_name and trys to find a file with that base name
 * is the -L search directories and in the standard directories.  If it is
 * sucessful it returns a pointer to the file name else it returns NULL.
 */
static
char *
search_for_file(
char *base_name)
{
    uint32_t i;
    char *file_name;

	for(i = 0; i < cmd_flags.nLdirs ; i++){
	    if(cmd_flags.Ldirs[i][1] != 'L')
		continue;
	    file_name = makestr(cmd_flags.Ldirs[i] + 2, "/", base_name, NULL);
	    if(access(file_name, R_OK) != -1)
		return(file_name);
	    free(file_name);
	}
	for(i = 0; standard_dirs[i] != NULL ; i++){
	    file_name = makestr(standard_dirs[i], base_name, NULL);
	    if(access(file_name, R_OK) != -1)
		return(file_name);
	    free(file_name);
	}
	return(NULL);
}

/*
 * search_paths_for_lname() takes the argument to a -lx option and and trys to
 * find a file with the name libx.dylib or libx.a.  This routine is only used
 * when the -search_paths_first option is specified and -dynamic is in effect.
 * And looks for a file name ending in .dylib then .a in each directory before
 * looking in the next directory.  The list of the -L search directories and in
 * the standard directories are searched in that order.  If this is sucessful
 * it returns a pointer to the file name else NULL.
 */
static
char *
search_paths_for_lname(
const char *lname_argument)
{
    uint32_t i;
    char *file_name, *dir;

	for(i = 0; i < cmd_flags.nLdirs ; i++){
	    if(cmd_flags.Ldirs[i][1] != 'L')
		continue;
	    dir = makestr(cmd_flags.Ldirs[i] + 2, "/", NULL);
	    file_name = search_path_for_lname(dir, lname_argument);
	    free(dir);
	    if(file_name != NULL)
		return(file_name);
	}
	for(i = 0; standard_dirs[i] != NULL ; i++){
	    file_name = search_path_for_lname(standard_dirs[i], lname_argument);
	    if(file_name != NULL)
		return(file_name);
	}
	return(NULL);
}

/*
 * search_path_for_lname() takes the argument to a -lx option and and trys to
 * find a file with the name libx.dylib then libx.a in the specified directory
 * name.  This routine is only used when the -search_paths_first option is
 * specified and -dynamic is in effect.  If this is sucessful it returns a
 * pointer to the file name else NULL.
 */
static
char *
search_path_for_lname(
const char *dir,
const char *lname_argument)
{
    char *file_name;

	file_name = makestr(dir, "/", "lib", lname_argument, ".dylib", NULL);
	if(access(file_name, R_OK) != -1)
	    return(file_name);
	free(file_name);

	file_name = makestr(dir, "/", "lib", lname_argument, ".a", NULL);
	if(access(file_name, R_OK) != -1)
	    return(file_name);
	free(file_name);

	return(NULL);
}

/*
 * add_member() add the specified ofile as a member to the library.  The
 * specified ofile must be either an object file (libtool or ranlib) or an
 * archive member with an unknown file type (ranlib only).
 */
static
void
add_member(
struct ofile *ofile)
{
    uint32_t i, j, size, ar_name_size;
    struct arch *arch;
    struct member *member;
    struct stat stat_buf;
    char *p, c, ar_name_buf[sizeof(ofile->member_ar_hdr->ar_name) + 1];
    char ar_size_buf[sizeof(ofile->member_ar_hdr->ar_size) + 1];
    const struct arch_flag *family_arch_flag;

	/*
	 * If this did not come from an archive get the stat info which is
	 * needed to fill in the archive header for this member.
	 */
	if(ofile->member_ar_hdr == NULL){
	    if(stat(ofile->file_name, &stat_buf) == -1){
		system_error("can't stat file: %s", ofile->file_name);
		return;
	    }
	}

	/*
	 * Determine the size this member will have in the library which
	 * includes the padding as a result of rounding the size of the
	 * member.  To get all members on an 8 byte boundary (so that mapping
	 * in object files can be used directly) the size of the member is
	 * CHANGED to reflect this padding.  In the UNIX definition of archives
	 * the size of the member is never changed but the offset to the next
	 * member is defined to be the offset of the previous member plus
	 * the size of the previous member rounded to 2.  So to get 8 byte
	 * boundaries without breaking the UNIX definition of archives the
	 * size is changed here.  As with the UNIX ar(1) program the padded
	 * bytes are set to the character '\n'.
	 */
	if(ofile->mh != NULL || ofile->mh64 != NULL)
	    size = (uint32_t)rnd(ofile->object_size, 8);
#ifdef LTO_SUPPORT
        else if(ofile->lto != NULL){
            if(ofile->file_type == OFILE_LLVM_BITCODE)
                size = (uint32_t)rnd(ofile->file_size, 8);
            else if(ofile->file_type == OFILE_FAT ||
                    (ofile->file_type == OFILE_ARCHIVE &&
                     ofile->member_type == OFILE_FAT))
                size = (uint32_t)rnd(ofile->object_size, 8);
            else
                size = (uint32_t)rnd(ofile->member_size, 8);
        }
#endif /* LTO_SUPPORT */
	else
	    size = (uint32_t)rnd(ofile->member_size, 8);

	/* select or create an arch type to put this in */
	i = 0;
	if(ofile->mh != NULL ||
	   ofile->mh64 != NULL){
	    if(ofile->mh_cputype == 0){
		if(ofile->member_ar_hdr != NULL){
		    error("file: %s(%.*s) cputype is zero (a reserved value)",
			  ofile->file_name, (int)ofile->member_name_size,
			  ofile->member_name);
		}
		else
		    error("file: %s cputype is zero (a reserved value)",
			  ofile->file_name);
		return;
	    }
	    /*
	     * If we are building a dynamic library then don't add dynamic
	     * shared libraries to the archs.  This is so that a dependent
	     * dynamic shared library that happens to be fat will not cause the
	     * library to be created fat unless there are object going into
	     * the library that are fat.
	     */
	    if(ofile->mh_filetype == MH_DYLIB ||
	       ofile->mh_filetype == MH_DYLIB_STUB){
		/*
		 * If we are building a static library we should not put a
		 * dynamic library Mach-O file into the static library.  This
		 * can happen if a libx.a file is really a dynamic library and
		 * someone is using -lx when creating a static library.
		 */
		if(cmd_flags.dynamic != TRUE){
		    if(ofile->member_ar_hdr != NULL){
			warning("file: %s(%.*s) is a dynamic library, not "
				"added to the static library",
			        ofile->file_name, (int)ofile->member_name_size,
			        ofile->member_name);
		    }
		    else
			warning("file: %s is a dynamic library, not added to "
				"the static library", ofile->file_name);
		}
		return;
	    }
	    /*
	     * If -arch_only is specified then only add this file if it matches
	     * the architecture specified.
	     */
	    if(cmd_flags.arch_only_flag.name != NULL){
		if(cmd_flags.arch_only_flag.cputype != ofile->mh_cputype)
		    return;
		if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM ||
		   cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64_32 ||
		   cmd_flags.arch_only_flag.cputype == CPU_TYPE_X86_64){
		    if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->mh_cpusubtype)
			return;
		}
		if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64){
		    if(cmd_flags.arch_only_flag.cpusubtype ==
		       CPU_SUBTYPE_ARM64_ALL){
			if(ofile->mh_cpusubtype != CPU_SUBTYPE_ARM64_ALL &&
			   ofile->mh_cpusubtype != CPU_SUBTYPE_ARM64_V8)
			    return;
		    }
		    else if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->mh_cpusubtype)
			return;
		}
	    }

	    for( ; i < narchs; i++){
		if(archs[i].arch_flag.cputype == ofile->mh_cputype){
		    if((archs[i].arch_flag.cputype == CPU_TYPE_ARM ||
		        archs[i].arch_flag.cputype == CPU_TYPE_ARM64 ||
		        archs[i].arch_flag.cputype == CPU_TYPE_ARM64_32 ||
		        archs[i].arch_flag.cputype == CPU_TYPE_X86_64) &&
		       archs[i].arch_flag.cpusubtype != ofile->mh_cpusubtype)
			continue;
		    if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64){
			if(cmd_flags.arch_only_flag.cpusubtype ==
			   CPU_SUBTYPE_ARM64_ALL){
			    if(ofile->mh_cpusubtype != CPU_SUBTYPE_ARM64_ALL &&
			       ofile->mh_cpusubtype != CPU_SUBTYPE_ARM64_V8)
				continue;
			}
			else if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->mh_cpusubtype)
			    continue;
		    }
		    break;
		}
	    }
	}
#ifdef LTO_SUPPORT
	else if(ofile->lto != NULL){
	    /*
	     * If -arch_only is specified then only add this file if it matches
	     * the architecture specified.
	     */
	    if(cmd_flags.arch_only_flag.name != NULL){
		if(cmd_flags.arch_only_flag.cputype != ofile->lto_cputype)
		    return;
		if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM ||
		   cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64_32 ||
		   cmd_flags.arch_only_flag.cputype == CPU_TYPE_X86_64){
		    if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->lto_cpusubtype)
			return;
		}
		if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64){
		    if(cmd_flags.arch_only_flag.cpusubtype ==
		       CPU_SUBTYPE_ARM64_ALL){
			if(ofile->lto_cpusubtype != CPU_SUBTYPE_ARM64_ALL &&
			   ofile->lto_cpusubtype != CPU_SUBTYPE_ARM64_V8)
			    return;
		    }
		    else if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->lto_cpusubtype)
			return;
		}
	    }

	    for( ; i < narchs; i++){
		if(archs[i].arch_flag.cputype == ofile->lto_cputype){
		    if((archs[i].arch_flag.cputype == CPU_TYPE_ARM ||
		        archs[i].arch_flag.cputype == CPU_TYPE_ARM64 ||
		        archs[i].arch_flag.cputype == CPU_TYPE_ARM64_32 ||
		        archs[i].arch_flag.cputype == CPU_TYPE_X86_64) &&
		       archs[i].arch_flag.cpusubtype != ofile->lto_cpusubtype)
			continue;
		    if(cmd_flags.arch_only_flag.cputype == CPU_TYPE_ARM64){
			if(cmd_flags.arch_only_flag.cpusubtype ==
			   CPU_SUBTYPE_ARM64_ALL &&
			   (ofile->lto_cpusubtype != CPU_SUBTYPE_ARM64_ALL &&
			    ofile->lto_cpusubtype != CPU_SUBTYPE_ARM64_V8))
			    continue;
			else if(cmd_flags.arch_only_flag.cpusubtype !=
							ofile->lto_cpusubtype)
			    continue;
		    }
		    break;
		}
	    }
	}
#endif /* LTO_SUPPORT */
	if(narchs == 1 && archs[0].arch_flag.cputype == 0){
	    i = 0;
	}
	else if(i == narchs){
	    archs = reallocate(archs, sizeof(struct arch) * (narchs+1));
	    memset(archs + narchs, '\0', sizeof(struct arch));
	    if(ofile->mh != NULL ||
	       ofile->mh64 != NULL){
		if(ofile->mh_cputype == CPU_TYPE_ARM ||
		   ofile->mh_cputype == CPU_TYPE_ARM64 ||
		   ofile->mh_cputype == CPU_TYPE_ARM64_32 ||
		   ofile->mh_cputype == CPU_TYPE_X86_64){
		    archs[narchs].arch_flag.name = (char *)
			get_arch_name_from_types(
				ofile->mh_cputype, ofile->mh_cpusubtype);
		    archs[narchs].arch_flag.cputype = ofile->mh_cputype;
		    archs[narchs].arch_flag.cpusubtype = ofile->mh_cpusubtype;
		}
		else{
		    family_arch_flag =
			    get_arch_family_from_cputype(ofile->mh_cputype);
		    if(family_arch_flag != NULL)
			archs[narchs].arch_flag = *family_arch_flag;
		}
	    }
#ifdef LTO_SUPPORT
	    else if(ofile->lto != NULL){
		if(ofile->lto_cputype == CPU_TYPE_ARM ||
		   ofile->lto_cputype == CPU_TYPE_ARM64 ||
		   ofile->lto_cputype == CPU_TYPE_ARM64_32 ||
		   ofile->lto_cputype == CPU_TYPE_X86_64){
		    archs[narchs].arch_flag.name = (char *)
			get_arch_name_from_types(
				ofile->lto_cputype, ofile->lto_cpusubtype);
		    archs[narchs].arch_flag.cputype = ofile->lto_cputype;
		    archs[narchs].arch_flag.cpusubtype = ofile->lto_cpusubtype;
		}
		else{
		    family_arch_flag =
			    get_arch_family_from_cputype(ofile->lto_cputype);
		    if(family_arch_flag != NULL)
			archs[narchs].arch_flag = *family_arch_flag;
		}
	    }
#endif /* LTO_SUPPORT */
	    else
		archs[narchs].arch_flag.name = "unknown";
	    narchs++;
	}
	arch = archs + i;

	/*
	 * If the cputype of this arch is not yet known then see if this new
	 * member can determine it.
	 */
	if(arch->arch_flag.cputype == 0 &&
	   (ofile->mh != NULL || ofile->mh64 != NULL)){
	    family_arch_flag = get_arch_family_from_cputype(ofile->mh_cputype);
	    if(family_arch_flag != NULL){
		arch->arch_flag = *family_arch_flag;
	    }
            else{
                arch->arch_flag.name =
                    savestr("cputype 1234567890 cpusubtype 1234567890");
                if(arch->arch_flag.name != NULL)
                    sprintf(arch->arch_flag.name, "cputype %u cpusubtype %u",
                            ofile->mh_cputype, ofile->mh_cpusubtype &
			    ~CPU_SUBTYPE_MASK);
                    arch->arch_flag.cputype = ofile->mh_cputype;
                    arch->arch_flag.cpusubtype = ofile->mh_cpusubtype;
	    }
	}
#ifdef LTO_SUPPORT
	if(arch->arch_flag.cputype == 0 &&
	   (ofile->lto != NULL)){
	    family_arch_flag = get_arch_family_from_cputype(ofile->lto_cputype);
	    if(family_arch_flag != NULL){
		arch->arch_flag = *family_arch_flag;
	    }
            else{
                arch->arch_flag.name =
                    savestr("cputype 1234567890 cpusubtype 1234567890");
                if(arch->arch_flag.name != NULL)
                    sprintf(arch->arch_flag.name, "cputype %u cpusubtype %u",
                            ofile->lto_cputype, ofile->lto_cpusubtype &
			    ~CPU_SUBTYPE_MASK);
                    arch->arch_flag.cputype = ofile->lto_cputype;
                    arch->arch_flag.cpusubtype = ofile->lto_cpusubtype;
	    }
	}
#endif /* LTO_SUPPORT */

	/* create a member in this arch type for this member */
	arch->members = reallocate(arch->members, sizeof(struct member) *
				   (arch->nmembers + 1));
	member = arch->members + arch->nmembers;
	memset(member, '\0', sizeof(struct member));
	arch->nmembers++;

	/* fill in the member for this ofile */
	member->input_file_name = ofile->file_name;

	if(ofile->member_ar_hdr == NULL){
	    /*
	     * We are creating an archive member in the output file from a
	     * file (that is not archive member in an input file).  First get
	     * the base name the file_name for the member name.
	     */
	    p = strrchr(ofile->file_name, '/');
	    if(p != NULL)
		p++;
	    else
		p = ofile->file_name;
	    member->input_base_name = p;
	    member->input_base_name_size = (uint32_t)strlen(p);
	    member->member_name = member->input_base_name;
	    /*
	     * If we can use long names then force using them to allow 64-bit
	     * objects to be aligned on an 8 byte boundary.  This is needed
	     * since the struct ar_hdr is not a multiple of 8.  This is normally
	     * done if the name does not fit in the archive header or contains
	     * a space character then we use the extened format #1.  The size
	     * of the name is rounded up so the object file after the name will
	     * be on an 8 byte boundary (including rounding the size of the
	     * struct ar_hdr).  The name will be padded with '\0's when it is
	     * written out.
	     */
	    if(cmd_flags.use_long_names == TRUE){
		member->output_long_name = TRUE;
		member->member_name_size = member->input_base_name_size;
		ar_name_size = (uint32_t)(rnd(member->input_base_name_size, 8) +
					  (rnd(sizeof(struct ar_hdr), 8) -
					   sizeof(struct ar_hdr)));
		sprintf(ar_name_buf, "%s%-*lu", AR_EFMT1,
			(int)(sizeof(member->ar_hdr.ar_name) -
			      (sizeof(AR_EFMT1) - 1)),
			(long unsigned int)ar_name_size);
		memcpy(member->ar_hdr.ar_name, ar_name_buf,
		      sizeof(member->ar_hdr.ar_name));
	    }
	    else{
		ar_name_size = 0;
		member->output_long_name = FALSE;
		/*
		 * Truncate the file_name if needed and place in archive header.
		 */
		c = '\0';
		if(strlen(p) > sizeof(member->ar_hdr.ar_name)){
		    c = p[sizeof(member->ar_hdr.ar_name)];
		    p[sizeof(member->ar_hdr.ar_name)] = '\0';
		}
		sprintf((char *)(&member->ar_hdr), "%-*s",
			(int)sizeof(member->ar_hdr.ar_name), p);
		if(c != '\0')
		    p[sizeof(member->ar_hdr.ar_name)] = c;
		member->member_name_size = size_ar_name(&member->ar_hdr);
	    }
	    /*
	     * adjust the time, mode, uid, and gid for the incoming archive
	     * member.
	     */
	    if(cmd_flags.D == TRUE || zero_ar_date == TRUE)
		stat_buf.st_mtime = toc_time;
            if (cmd_flags.D == TRUE) {
                stat_buf.st_mode = toc_mode;
                stat_buf.st_uid = toc_uid;
                stat_buf.st_gid = toc_gid;
            }
            
	    /*
	     * Create the rest of the archive header after the name.
	     */
	    sprintf((char *)(&member->ar_hdr) + sizeof(member->ar_hdr.ar_name),
	       "%-*ld%-*u%-*u%-*o%-*ld%-*s",
	       (int)sizeof(member->ar_hdr.ar_date),
		   (long int)stat_buf.st_mtime,
	       (int)sizeof(member->ar_hdr.ar_uid),
		   (unsigned short)stat_buf.st_uid,
	       (int)sizeof(member->ar_hdr.ar_gid),
		   (unsigned short)stat_buf.st_gid,
	       (int)sizeof(member->ar_hdr.ar_mode),
		   (unsigned int)stat_buf.st_mode,
	       (int)sizeof(member->ar_hdr.ar_size),
		   (long)size + ar_name_size,
	       (int)sizeof(member->ar_hdr.ar_fmag),
		   ARFMAG);
	}
	else{
	    /*
	     * We are creating an archive member in the output file from an
	     * archive member in an input file.  There can be some changes to
	     * the contents.  First the size might be changed and the contents
	     * padded with '\n's to round it to a multiple of 8
	     * Second we may take a member using extended format #1
	     * for it's name and truncate it then place the name in the archive
	     * header.  Or we may round the name size to a multiple of 8.
	     */
	    member->input_ar_hdr = ofile->member_ar_hdr;
	    member->input_base_name = ofile->member_name;
	    member->input_base_name_size = ofile->member_name_size;
	    member->input_member_offset = ofile->member_offset -
					  sizeof(struct ar_hdr);
	    if(strncmp(ofile->member_ar_hdr->ar_name, AR_EFMT1,
	       sizeof(AR_EFMT1) - 1) == 0)
		member->input_member_offset -= ofile->member_name_size;

	    member->ar_hdr = *(ofile->member_ar_hdr);
	    member->member_name = ofile->member_name;

	    if(cmd_flags.use_long_names == TRUE){
		/*
		 * We can use long names.  So if the input ofile is using the
		 * extended format #1 we need make sure the size of the name and
		 * the size of struct ar_hdr are rounded to 8 bytes. And write
		 * that size into the ar_name with the AR_EFMT1 string.  To
		 * avoid growing the size of names first trim the long name size
		 * before rounding up.
		 */
		if(ofile->member_name != ofile->member_ar_hdr->ar_name){
		    member->output_long_name = TRUE;
		    member->member_name_size = ofile->member_name_size;
		    for(ar_name_size = member->member_name_size;
			ar_name_size > 1 ;
			ar_name_size--){
			if(ofile->member_name[ar_name_size - 1] != '\0')
			   break;
		    }
		    member->member_name_size = ar_name_size;
		    ar_name_size = (uint32_t)(rnd(ar_name_size, 8) +
					      (rnd(sizeof(struct ar_hdr), 8) -
					       sizeof(struct ar_hdr)));
		    sprintf(ar_name_buf, "%s%-*lu", AR_EFMT1,
			    (int)(sizeof(member->ar_hdr.ar_name) -
				  (sizeof(AR_EFMT1) - 1)),
			    (long unsigned int)ar_name_size);
		    memcpy(member->ar_hdr.ar_name, ar_name_buf,
			  sizeof(member->ar_hdr.ar_name));
		}
		else{
		    /*
		     * Since we can use long names force this to use extended
		     * format #1. And round the name size to 8 plus the size of
		     * struct ar_hdr rounded to 8 bytes.
		     */
		    member->member_name_size = size_ar_name(&member->ar_hdr);
		    ar_name_size = (uint32_t)(rnd(ofile->member_name_size, 8) +
					      (rnd(sizeof(struct ar_hdr), 8) -
					       sizeof(struct ar_hdr)));
		    member->output_long_name = TRUE;
		    sprintf(ar_name_buf, "%s%-*lu", AR_EFMT1,
			    (int)(sizeof(member->ar_hdr.ar_name) -
				  (sizeof(AR_EFMT1) - 1)),
			    (long unsigned int)ar_name_size);
		    memcpy(member->ar_hdr.ar_name, ar_name_buf,
			  sizeof(member->ar_hdr.ar_name));
		}
	    }
	    else{
		/*
		 * We can't use long names.  So if the input ofile is using the
		 * extended format #1 we need to truncate the name and write it
		 * into the ar_name field.  Note the extended format is also
		 * used it the name has a space in it so it may be shorter than
		 * sizeof(ar_hdr.ar_name) .
		 */
		ar_name_size = 0;
		member->output_long_name = FALSE;
		if(ofile->member_name != ofile->member_ar_hdr->ar_name){
		    for(j = 0; j < sizeof(member->ar_hdr.ar_name) &&
			       j < ofile->member_name_size &&
			       ofile->member_name[j] != '\0'; j++)
			member->ar_hdr.ar_name[j] = ofile->member_name[j];
		    for( ; j < sizeof(member->ar_hdr.ar_name); j++)
			member->ar_hdr.ar_name[j] = ' ';
		}
		member->member_name_size = size_ar_name(&member->ar_hdr);
	    }
	    /*
	     * Since sprintf() writes a '\0' at the end of the string the
	     * memcpy is needed to preserve the ARFMAG string that follows.
	     */
	    sprintf(ar_size_buf, "%-*ld",
		    (int)sizeof(member->ar_hdr.ar_size),
		    (long)size + ar_name_size);
	    memcpy(member->ar_hdr.ar_size, ar_size_buf,
		   sizeof(member->ar_hdr.ar_size));
	}

	member->offset = arch->size;
	arch->size += sizeof(struct ar_hdr) + size + ar_name_size;

	if(ofile->mh != NULL ||
	   ofile->mh64 != NULL){
	    member->object_addr = ofile->object_addr;
	    member->object_size = ofile->object_size;
	    member->object_byte_sex = ofile->object_byte_sex;
	    member->mh = ofile->mh;
	    member->mh64 = ofile->mh64;
	    member->load_commands = ofile->load_commands;
	}
#ifdef LTO_SUPPORT
	else if(ofile->file_type == OFILE_LLVM_BITCODE){
	    member->object_addr = ofile->file_addr;
	    member->object_size = (uint32_t)ofile->file_size;
	    member->lto_contents = TRUE;
	    save_lto_member_toc_info(member, ofile->lto);
	    lto_free(ofile->lto);
	    ofile->lto = NULL;
	    member->object_byte_sex = get_byte_sex_from_flag(&arch->arch_flag);
	}
        else if((ofile->file_type == OFILE_FAT &&
                 ofile->arch_type == OFILE_LLVM_BITCODE) ||
                (ofile->file_type == OFILE_ARCHIVE &&
                 ofile->member_type == OFILE_FAT &&
                 ofile->arch_type == OFILE_LLVM_BITCODE)){
            member->object_addr = ofile->object_addr;
            member->object_size = ofile->object_size;
	    member->lto_contents = TRUE;
	    save_lto_member_toc_info(member, ofile->lto);
	    lto_free(ofile->lto);
	    ofile->lto = NULL;
            member->object_byte_sex = get_byte_sex_from_flag(&arch->arch_flag);
        }
#endif /* LTO_SUPPORT */
	else{
	    member->object_addr = ofile->member_addr;
	    member->object_size = ofile->member_size;
#ifdef LTO_SUPPORT
	    if(ofile->lto != NULL){
		member->lto_contents = TRUE;
	        save_lto_member_toc_info(member, ofile->lto);
		lto_free(ofile->lto);
		ofile->lto = NULL;
		member->object_byte_sex = get_byte_sex_from_flag(
							&arch->arch_flag);
	    }
#endif /* LTO_SUPPORT */
	}
}

/*
 * free_archs() frees the memory allocated that is pointed to by archs.
 */
static
void
free_archs(
void)
{
    uint32_t i;

	for(i = 0 ; i < narchs; i++){
	    /*
	     * Just leak memory on the arch_flag.name in some cases
	     * (unknown archiectures only where the space is malloced and
	     * a sprintf() is done into the memory)
	     */
	    if(archs[i].tocs != NULL)
		free(archs[i].tocs);
	    if(archs[i].toc_ranlibs != NULL)
		free(archs[i].toc_ranlibs);
	    if(archs[i].toc_ranlibs64 != NULL)
		free(archs[i].toc_ranlibs64);
	    if(archs[i].toc_strings != NULL)
		free(archs[i].toc_strings);
	    if(archs[i].members != NULL)
		free(archs[i].members);
	}
	if(archs != NULL)
	    free(archs);
	archs = NULL;
	narchs = 0;
}

/*
 * create_library() creates a library from the data structure pointed to by
 * archs into the specified output file.  Only when more than one architecture
 * is in archs will a fat file be created.
 *
 * In the case of cmd_flags.ranlib == TRUE the ofile may not be NULL if it
 * from a thin archive.  If so and the toc_* fields are set and we may update
 * the table of contents in place if the new one fits where the old table of
 * contents was.
 */
static
void
create_library(
char *output,
struct ofile *ofile)
{
    uint32_t i, j, k, pad;
    uint64_t library_size, offset, *time_offsets;
    enum byte_sex target_byte_sex;
    kern_return_t r;
    struct arch *arch;
    struct fat_header *fat_header;
    struct fat_arch *fat_arch;
    struct fat_arch_64 *fat_arch64;
    int fd;
	/* cctools-port: commented. */
/*
#ifndef __OPENSTEP__
    struct timeval timep[2];
#else
    time_t timep[2];
#endif
*/
    struct timespec times[2]; /* cctools-port */
    struct stat stat_buf;
    struct ar_hdr toc_ar_hdr;
    enum bool some_tocs, same_toc, different_offsets;
    uint32_t toc_mtime;
    enum bool write_in_place;
    const char* suffix = ".XXXXXX";
    char* tempfile;
    size_t templen;
    const char* libtool_force_fail;
    const char* ranlib_force_fail;
    int time_result;

	libtool_force_fail = getenv("LIBTOOL_FORCE_FAIL");
	ranlib_force_fail = getenv("RANLIB_FORCE_FAIL");
	different_offsets = FALSE;

	if(narchs == 0){
	    if(cmd_flags.ranlib == TRUE){
		if(cmd_flags.q == FALSE)
		    warning("empty library: %s (no table of contents added)",
			    output);
		return;
	    }
	    else{
		if(cmd_flags.dynamic == FALSE ||
		   cmd_flags.no_files_ok == FALSE){
		    if(cmd_flags.arch_only_flag.name != NULL)
			error("no library created (no object files in input "
			      "files matching -arch_only %s)",
			      cmd_flags.arch_only_flag.name);
		    else
			error("no library created (no object files in input "
			      "files)");
		    return;
		}
	    }
	}

	if(cmd_flags.dynamic == TRUE){
	    create_dynamic_shared_library(output);
	    return;
	}

	/* if this is libtool warn about duplicate member names */
	if(cmd_flags.ranlib == FALSE)
	    warn_duplicate_member_names();

	/*
	 * Calculate the total size of the library and the final size of each
	 * architecture.
	 */
	if(narchs > 1){
	    library_size = sizeof(struct fat_header);
	    if(cmd_flags.fat64 == TRUE)
		library_size += sizeof(struct fat_arch_64) * narchs;
	    else
		library_size += sizeof(struct fat_arch) * narchs;
	    /*
	     * The ar(1) program uses the -q flag to ranlib(1) to add a table
	     * of contents only of the output is not a fat file.  This is done
	     * by default for UNIX standards conformance when the files are
	     * thin .o files.
	     */
	    if(cmd_flags.q == TRUE)
		exit(EXIT_SUCCESS);
	    /*
	     * The ar(1) program uses the -f to ranlib(1) when it see the 's'
	     * option.  And if we are creating a fat file issue a warning that
	     * ar(1) will not be able to use it.
	     */
	    if(cmd_flags.f == TRUE)
		warning("archive library: %s will be fat and ar(1) will not "
			"be able to operate on it", output);
	}
	else
	    library_size = 0;
	some_tocs = FALSE;
	for(i = 0; i < narchs; i++){
	    if(narchs > 1 && (archs[i].arch_flag.cputype & CPU_ARCH_ABI64))
		library_size = rnd(library_size, 1 << 3);
	    make_table_of_contents(archs + i, output);
	    if(errors != 0)
		return;
	    if(archs[i].toc_nranlibs != 0)
		some_tocs = TRUE;
	    archs[i].size += SARMAG + archs[i].toc_size;
	    library_size += archs[i].size;
	}
	/*
	 * The ar(1) program uses the -q flag to ranlib(1) to add a table of
	 * contents only of the output contains some object files.  This is
	 * done for UNIX standards conformance.
	 */
	if(cmd_flags.q == TRUE && some_tocs == FALSE)
	    exit(EXIT_SUCCESS);
	
 	/* 
	 * If this is ranlib(1) and we are running in UNIX standard mode and
	 * the file is not writeable just print and error message and return.
	 */
	if(cmd_flags.ranlib == TRUE &&
	   get_unix_standard_mode() == TRUE &&
	   access(output, W_OK) == -1){
	    system_error("file: %s is not writable", output);
	    return;
	}

	/*
	 * If this is ranlib(1) and we have a thin archive that has an existing
	 * table of contents see if we have enough room to update it in place.
	 * Actually we check to see that we have the exact same number of
	 * ranlib structs and string size, as this is the most common case that
	 * the defined global symbols have not changed when rebuilding and it
	 * will just be the offset to archive members that will have changed.
	 */
	write_in_place = FALSE;
	if(cmd_flags.ranlib == TRUE && narchs == 1 &&
	   ofile != NULL && ofile->toc_addr != NULL &&
	   ofile->toc_bad == FALSE &&
	   archs[0].using_64toc != ofile->toc_is_32bit &&
	   archs[0].toc_nranlibs == ofile->toc_nranlibs &&
	   archs[0].toc_strsize == ofile->toc_strsize){

            write_in_place = TRUE;

	    /*
	     * If the table of contents in the input does have a long name and
	     * the one we built does not (or vice a versa) then don't update it
	     * in place.
	     */
	    if(strncmp(ofile->toc_ar_hdr->ar_name, AR_EFMT1,
		       sizeof(AR_EFMT1) - 1) == 0){
	       /*
	        * Also if it has a long name and the sizes of the long name
	        * are not the same or the names are not the same don't update
	        * it in place.
	        */
	       if(archs[0].toc_long_name != TRUE ||
		  ofile->toc_name_size != archs[0].toc_name_size ||
		  strcmp(ofile->toc_name, archs[0].toc_name) != 0)
		   write_in_place = FALSE;
	    }
	    else{
	       if(archs[0].toc_long_name == TRUE)
		   write_in_place = FALSE;
	    }

	    /*
	     * The existing thin archive may not be laid out the same way as
	     * libtool(1) would do it.  As ar(1) does not know to pad things
	     * so object files are on their natural alignment.  So check to
	     * see if the offsets are not the same and if the alignment is OK.
	     */
	    for(i = 0; i < archs[0].nmembers; i++){
		if(archs[0].members[i].input_member_offset !=
		   archs[0].members[i].offset){
		    different_offsets = TRUE;
		    /*
		     * For now we will allow alignments of 4 bytes offsets even
		     * though we would produce 8 byte alignments.
		     */
		    if(archs[0].members[i].input_member_offset % 4 != 0){
			write_in_place = FALSE;
			break;
		    }
		}
	    }
	}

	if (write_in_place == TRUE) {
	    char *library, *p;

	    /* write in place, not into a temporary file. */
	    tempfile = NULL;

	    /*
	     * The time_offsets array records the offsets to the table of
	     * contents archive header's ar_date fields.  In this case we just
	     * have one since this is a thin file (non-fat) file.
	     */
	    time_offsets = allocate(1 * sizeof(uint64_t));
	    /*
	     * Calculate the offset to the archive header's time field for the
	     * table of contents.
	     */
	    time_offsets[0] = SARMAG +
			 ((char *)&toc_ar_hdr.ar_date - (char *)&toc_ar_hdr);

	    /*
	     * If we had different member offsets in the input thin archive
	     * we adjust the ranlib structs ran_off to use them.
	     */
	    if(different_offsets == TRUE){
		same_toc = FALSE;
		if(archs[0].using_64toc == FALSE){
		    for(i = 0; i < archs[0].toc_nranlibs; i++){
			for(j = 0; j < archs[0].nmembers; j++){
			    if(archs[0].members[j].offset == 
			       archs[0].toc_ranlibs[i].ran_off){
				archs[0].toc_ranlibs[i].ran_off = (uint32_t)
				    archs[0].members[j].input_member_offset;
				break;
			    }
			}
		    }
		}
		else{
		    for(i = 0; i < archs[0].toc_nranlibs; i++){
			for(j = 0; j < archs[0].nmembers; j++){
			    if(archs[0].members[j].offset == 
			       archs[0].toc_ranlibs64[i].ran_off){
				archs[0].toc_ranlibs64[i].ran_off = 
				    archs[0].members[j].input_member_offset;
				break;
			    }
			}
		    }
		}
	    }
	    else{
		/*
		 * If the new table of contents and the new string table are the
		 * same as the old then the archive only needs to be "touched"
		 * and the time field of the toc needs to be updated.
		 */
		same_toc = TRUE;
		for(i = 0; i < archs[0].toc_nranlibs; i++){
		    if(archs[0].using_64toc == FALSE){
			if(archs[0].toc_ranlibs[i].ran_un.ran_strx != 
			   ofile->toc_ranlibs[i].ran_un.ran_strx ||
			   archs[0].toc_ranlibs[i].ran_off !=
			   ofile->toc_ranlibs[i].ran_off){
			    same_toc = FALSE;
			    break;
			}
		    }
		    else{
			if(archs[0].toc_ranlibs64[i].ran_un.ran_strx != 
			   ofile->toc_ranlibs64[i].ran_un.ran_strx ||
			   archs[0].toc_ranlibs64[i].ran_off !=
			   ofile->toc_ranlibs64[i].ran_off){
			    same_toc = FALSE;
			    break;
			}
		    }
		}
		if(same_toc == TRUE){
		    for(i = 0; i < archs[0].toc_strsize; i++){
			if(archs[0].toc_strings[i] != ofile->toc_strings[i]){
			    same_toc = FALSE;
			    break;
			}
		    }
		}
	    }

	    library_size = SARMAG;
	    if(same_toc == FALSE)
		library_size += archs[0].toc_size;
	    if((r = vm_allocate(mach_task_self(), (vm_address_t *)&library,
				library_size, TRUE)) != KERN_SUCCESS)
		mach_fatal(r, "can't vm_allocate() buffer for output file: %s "
			   "of size %llu", output, library_size);

	    /* put in the archive magic string in the buffer */
	    p = library;
	    memcpy(p, ARMAG, SARMAG);
	    p += SARMAG;

	    /* put the table of contents in the buffer if needed */
	    target_byte_sex = get_target_byte_sex(archs + 0, host_byte_sex);
	    if(same_toc == FALSE)
		p = put_toc_member(p, archs+0, host_byte_sex, target_byte_sex);

	    /*
	     * Because we are writing in place, just open the output file
	     * directly and modify its contents. The output file will be closed
	     * after the toc timestamps have been refreshed.
	     */
	    if((fd = open(output, O_WRONLY, 0)) == -1){
		system_error("can't open output file: %s", output);
		return;
	    }
	    if(write64(fd, library, library_size) != (ssize_t)library_size){
		system_error("can't write output file: %s", output);
		return;
	    }

	    if((r = vm_deallocate(mach_task_self(), (vm_address_t)library,
				  library_size)) != KERN_SUCCESS){
		my_mach_error(r, "can't vm_deallocate() buffer for output file");
		return;
	    }
	} /* write_in_place == TRUE */
	else { /* if (write_in_place != TRUE) */
	    char *library, *p, *flush_start;

	    /* create a temporary file name */
	    templen = strlen(output) + strlen(suffix) + 1;
	    tempfile = calloc(templen, sizeof(char));
	    if (tempfile == NULL) {
		fatal("internal error: calloc() failed");
	    }
	    if (snprintf(tempfile, templen, "%s%s", output, suffix) !=
		(templen - 1)) {
		fatal("internal error: snprintf() failed");
	    }

	    /*
	     * This buffer is vm_allocate'ed to make sure all holes are filled
	     * with zero bytes. The range will be deallocated in pieces via
	     * output_flush, rather than vm_deallocated at the end.
	     */
	    if((r = vm_allocate(mach_task_self(), (vm_address_t *)&library,
				library_size, TRUE)) != KERN_SUCCESS)
		mach_fatal(r,
			   "can't vm_allocate() buffer for output file: %s of "
			   "size %llu", output, library_size);
	    
	    /*
	     * Create the output file.
	     */
	    if ((fd = mkstemp(tempfile)) == -1) {
		system_error("can't create temporary file: %s", tempfile);
		return;
	    }
	    if (fchmod(fd, toc_mode) == -1) {
		system_error("can't update temporary file: %s", tempfile);
		return;
	    }
	    
#ifdef F_NOCACHE
	    /* tell filesystem to NOT cache the file when reading or writing */
	    (void)fcntl(fd, F_NOCACHE, 1);
#endif

	    /*
	     * If there is more than one architecture then fill in the fat file
	     * header and the fat_arch or fat_arch64 structures in the buffer.
	     */
	    if(narchs > 1){
		fat_header = (struct fat_header *)library;
		if(cmd_flags.fat64 == TRUE)
		    fat_header->magic = FAT_MAGIC_64;
		else
		    fat_header->magic = FAT_MAGIC;
		fat_header->nfat_arch = narchs;
		offset = sizeof(struct fat_header);
		if(cmd_flags.fat64 == TRUE){
		    offset += sizeof(struct fat_arch_64) * narchs;
		    fat_arch64 = (struct fat_arch_64 *)
		    (library + sizeof(struct fat_header));
		    fat_arch = NULL;
		}
		else{
		    offset += sizeof(struct fat_arch) * narchs;
		    fat_arch = (struct fat_arch *)
		    (library + sizeof(struct fat_header));
		    fat_arch64 = NULL;
		}
		for(i = 0; i < narchs; i++){
		    if(cmd_flags.fat64 == TRUE){
			fat_arch64[i].cputype = archs[i].arch_flag.cputype;
			fat_arch64[i].cpusubtype =
			    archs[i].arch_flag.cpusubtype;
		    }
		    else{
			fat_arch[i].cputype = archs[i].arch_flag.cputype;
			fat_arch[i].cpusubtype = archs[i].arch_flag.cpusubtype;
		    }
		    if(cmd_flags.fat64 == FALSE && offset > UINT32_MAX)
			error("file too large to create as a fat file because "
			      "offset field in struct fat_arch is only 32-bits "
			      "and offset (%llu) to architecture %s exceeds "
			      "that", offset, archs[i].arch_flag.name);
		    if(archs[i].arch_flag.cputype & CPU_ARCH_ABI64){
			if(cmd_flags.fat64 == TRUE)
			    fat_arch64[i].align = 3;
			else
			    fat_arch[i].align = 3;
		    }
		    else{
			if(cmd_flags.fat64 == TRUE)
			    fat_arch64[i].align = 2;
			else
			    fat_arch[i].align = 2;
		    }
		    if(cmd_flags.fat64 == TRUE)
			offset = rnd(offset, 1 << fat_arch64[i].align);
		    else
			offset = rnd(offset, 1 << fat_arch[i].align);
		    if(cmd_flags.fat64 == TRUE)
			fat_arch64[i].offset = offset;
		    else
			fat_arch[i].offset = (uint32_t)offset;
		    if(cmd_flags.fat64 == FALSE && archs[i].size > UINT32_MAX)
			error("file too large to create as a fat file because "
			      "size field in struct fat_arch is only 32-bits "
			      "and size (%llu) of architecture %s exceeds that",
			      archs[i].size, archs[i].arch_flag.name);
		    if(cmd_flags.fat64 == TRUE)
			fat_arch64[i].size = archs[i].size;
		    else
			fat_arch[i].size = (uint32_t)archs[i].size;
		    offset += archs[i].size;
		}
		if(errors != 0){
		    (void)unlink(tempfile);
		    return;
		}
#ifdef __LITTLE_ENDIAN__
		swap_fat_header(fat_header, BIG_ENDIAN_BYTE_SEX);
		if(cmd_flags.fat64 == TRUE)
		    swap_fat_arch_64(fat_arch64, narchs, BIG_ENDIAN_BYTE_SEX);
		else
		    swap_fat_arch(fat_arch, narchs, BIG_ENDIAN_BYTE_SEX);
#endif /* __LITTLE_ENDIAN__ */
		offset = sizeof(struct fat_header);
		if(cmd_flags.fat64 == TRUE)
		    offset += sizeof(struct fat_arch_64) * narchs;
		else
		    offset += sizeof(struct fat_arch) * narchs;
	    }
	    else
		offset = 0;
	    
	    /* flush out the fat headers if any */
	    output_flush(library, library_size, fd, 0, offset);

	    /*
	     * The time_offsets array records the offsets to the table of
	     * contents archive header's ar_date fields.
	     */
	    time_offsets = allocate(narchs * sizeof(uint64_t));
	    
	    /*
	     * Now put each arch in the buffer.
	     */
	    for(i = 0; i < narchs; i++){
		arch = archs + i;
		if(narchs > 1 && (arch->arch_flag.cputype & CPU_ARCH_ABI64)){
		    pad = (uint32_t)(rnd(offset, 1 << 3) - offset);
		    output_flush(library, library_size, fd, offset, pad);
		    offset = rnd(offset, 1 << 3);
		}
		p = library + offset;
		flush_start = p;
		
		/*
		 * If the input files only contains non-object files then the
		 * byte sex of the output can't be determined which is needed
		 * for the two binary long's of the table of contents.  But
		 * since these will be zero (the same in both byte sexes)
		 * because there are no symbols in the table of contents if
		 * there are no object files.
		 */
		
		/* put in the archive magic string */
		memcpy(p, ARMAG, SARMAG);
		p += SARMAG;
		
		/*
		 * Warn for what really is a bad library that has an empty table
		 * of contents but this is allowed in the original ranlib.
		 */
		if(arch->toc_nranlibs == 0 && cmd_flags.q == FALSE){
		    if(narchs > 1)
			warning("warning for library: %s for architecture: %s "
				"the table of contents is empty (no object "
				"file members in the library define global "
				"symbols)", output, arch->arch_flag.name);
		    else
			warning("warning for library: %s the table of contents "
				"is empty (no object file members in the "
				"library define global symbols)", output);
		}
		
		/*
		 * Pick the byte sex to write the table of contents in.
		 */
		target_byte_sex = get_target_byte_sex(arch, host_byte_sex);
		
		/*
		 * Remember the offset to the archive header's time field for
		 * this arch's table of contents member.
		 */
		time_offsets[i] =
		(p - library) +
		((char *)&toc_ar_hdr.ar_date - (char *)&toc_ar_hdr);
		
		/*
		 * Put in the table of contents member in the output buffer.
		 */
		p = put_toc_member(p, arch, host_byte_sex, target_byte_sex);
		
		output_flush(library, library_size, fd, flush_start - library,
			     p - flush_start);
		
		/*
		 * Put in the archive header and member contents for each
		 * member.
		 */
		for(j = 0; j < arch->nmembers; j++){
		    flush_start = p;
		    memcpy(p, (char *)&(arch->members[j].ar_hdr),
			   sizeof(struct ar_hdr));
		    p += sizeof(struct ar_hdr);
		    
		    /*
		     * If we are using extended format #1 for long names write
		     * out the name.  Note the name is padded with '\0' and the
		     * member_name_size is the unrounded size.
		     */
		    if(arch->members[j].output_long_name == TRUE){
			strncpy(p, arch->members[j].member_name,
				arch->members[j].member_name_size);
			p += rnd(arch->members[j].member_name_size, 8) +
			(rnd(sizeof(struct ar_hdr), 8) -
			 sizeof(struct ar_hdr));
		    }
		    
		    /*
		     * ofile_map swaps the headers to the host_byte_sex if the
		     * object's byte sex is not the same as the host byte sex
		     * so if this is the case swap them back before writing
		     * them out.
		     */
		    if(arch->members[j].mh != NULL &&
		       arch->members[j].object_byte_sex != host_byte_sex){
			if(swap_object_headers(arch->members[j].mh,
					       arch->members[j].load_commands)
					       == FALSE)
			    fatal("internal error: swap_object_headers() "
				  "failed");
		    }
		    else if(arch->members[j].mh64 != NULL &&
			    arch->members[j].object_byte_sex != host_byte_sex){
			if(swap_object_headers(arch->members[j].mh64,
					       arch->members[j].load_commands)
					       == FALSE)
			    fatal("internal error: swap_object_headers() "
				  "failed");
		    }
		    memcpy(p, arch->members[j].object_addr,
			   arch->members[j].object_size);
#ifdef VM_SYNC_DEACTIVATE
		    vm_msync(mach_task_self(),
			     (vm_address_t)arch->members[j].object_addr,
			     (vm_size_t)arch->members[j].object_size,
			     VM_SYNC_DEACTIVATE);
#endif /* VM_SYNC_DEACTIVATE */
		    p += arch->members[j].object_size;
		    pad = rnd32(arch->members[j].object_size, 8) -
		    arch->members[j].object_size;
		    /*
		     * as with the UNIX ar(1) program pad with '\n' characters
		     */
		    for(k = 0; k < pad; k++)
			*p++ = '\n';
		    
		    output_flush(library, library_size, fd,
				 flush_start - library, p - flush_start);
		}
		offset += arch->size;
	    }

	    /*
	     * Write the library to the file or flush the remaining buffer to
	     * the file.
	     */
	    if(cmd_flags.noflush == TRUE){
		if(write64(fd, library, library_size) != (ssize_t)library_size){
		    system_error("can't write temporary file: %s", tempfile);
		    return;
		}
		if((r = vm_deallocate(mach_task_self(), (vm_address_t)library,
				      library_size)) != KERN_SUCCESS){
		    my_mach_error(r, "can't vm_deallocate() buffer for output "
				  "file");
		    return;
		}
	    }
	    else{
		final_output_flush(library, fd);
	    }
	} /* if write_in_place != TRUE */

	/*
	 * Now that the library is created on the file system, stat the file
	 * to get the time it was last modified from the file system.
	 */
	if (fstat(fd, &stat_buf) == -1) {
	    system_fatal("can't stat output file: %s",
			 tempfile ? tempfile : output);
	    return;
	}
	if(cmd_flags.D == TRUE || zero_ar_date == TRUE)
	    toc_mtime = 0;
	else
	    toc_mtime = (uint32_t)stat_buf.st_mtime + 5;
	/*
         * With the time from the file system the library is on set the ar_date
	 * using the modification time returned by stat.  Then write this into
	 * all the ar_date's in the file.
	 */
	sprintf((char *)(&toc_ar_hdr), "%-*s%-*ld",
	   (int)sizeof(toc_ar_hdr.ar_name),
	       SYMDEF,
	   (int)sizeof(toc_ar_hdr.ar_date),
	       (long int)toc_mtime);
	for(i = 0; i < narchs; i++){
	    if(lseek(fd, time_offsets[i], L_SET) == -1){
		system_error("can't lseek in output file: %s",
			     tempfile ? tempfile : output);
		return;
	    }
            /* MDT: write(2) is OK here, size is less than 2^31-1 */
	    if(write(fd, &toc_ar_hdr.ar_date, sizeof(toc_ar_hdr.ar_date)) !=
		     sizeof(toc_ar_hdr.ar_date)){
		system_error("can't write to output file: %s",
			     tempfile ? tempfile : output);
		return;
	    }
	}
	if(close(fd) == -1){
	    system_fatal("can't close output file: %s",
			 tempfile ? tempfile : output);
	    return;
	}
	/*
	 * Now set the modtime of the created library back to it's stat time
	 * when we first closed it.
	 */

	/*
		cctools-port:
		  Original code (949.0.1)
		  Replaced for portability.
	*/

#if 0
#ifndef __OPENSTEP__
	if (__builtin_available(macOS 10.12, *)) {
	    struct timespec times[2];
	    memcpy(&times[0], &stat_buf.st_atimespec, sizeof(struct timespec));
	    memcpy(&times[1], &stat_buf.st_mtimespec, sizeof(struct timespec));
	    time_result = utimensat(AT_FDCWD, tempfile, times, 0);
	}
	else {
	    TIMESPEC_TO_TIMEVAL(&timep[0], &stat_buf.st_atimespec);
	    TIMESPEC_TO_TIMEVAL(&timep[1], &stat_buf.st_mtimespec);
	    time_result = utimes(tempfile, timep);
	}
#else
	timep[0] = stat_buf.st_mtime;
	timep[1] = stat_buf.st_mtime;
	time_result = utime(tempfile, timep);
#endif
	if (time_result == -1) {
	    system_fatal("can't set the modifiy times in output file: %s",
			 tempfile ? tempfile : output);
	    return;
	}
#endif /* 0 */


#ifdef HAVE_STAT_ST_MTIMESPEC
	times[0] = stat_buf.st_atimespec;
	times[1] = stat_buf.st_mtimespec;
#elif HAVE_STAT_ST_MTIM
	times[0] = stat_buf.st_atim;
	times[1] = stat_buf.st_mtim;
#else
	times[0].tv_sec = stat_buf.st_atime;
	times[0].tv_nsec = 0;
	times[1].tv_sec = stat_buf.st_mtime;
	times[0].tv_nsec = 0;
#endif
	if(utimens(tempfile, times) == -1)
	{
	    system_fatal("can't set the modifiy times in output file: %s",
			 tempfile ? tempfile : output);
	    return;
	}

	/*
	 * Error out before writing the final file, if requested. This is only
	 * useful for testing.
	 */
	if (libtool_force_fail != NULL || ranlib_force_fail != NULL) {
	    system_fatal("%s set, simulating a system-fatal error: %s",
			 libtool_force_fail != NULL ? "LIBTOOL_FORCE_FAIL" :
			 "RANLIB_FORCE_FAIL", tempfile ? tempfile : output);
	    return;
	}

	/*
	 * Move the temporary file into its final location
	 */
	if (tempfile) {
	    if (rename(tempfile, output)) {
		system_fatal("can't move the output file to its final location: %s",
			     output);
		return;
	    }
	}
}

/*
 * get_target_byte_sex() pick the byte sex to write the table of contents in
 * for the arch.
 */
static
enum byte_sex
get_target_byte_sex(
struct arch *arch,
enum byte_sex host_byte_sex)
{
    uint32_t i;
    enum byte_sex target_byte_sex;

	target_byte_sex = UNKNOWN_BYTE_SEX;
	for(i = 0;
	    i < arch->nmembers && target_byte_sex == UNKNOWN_BYTE_SEX;
	    i++){
	    target_byte_sex = arch->members[i].object_byte_sex;
	}
	if(target_byte_sex == UNKNOWN_BYTE_SEX)
	    target_byte_sex = host_byte_sex;
	return(target_byte_sex);
}

/*
 * put_toc_member() put the contents member for arch into the buffer p and 
 * returns the pointer to the buffer after the table of contents.
 * The table of contents member uses either a 32-bit toc or a 64-bit toc.
 * Both forms start with:
 *  the archive header
 *  the archive member name (if using a long name)
 * then for a 32-bit toc the rest is this:
 *  a uint32_t for the number of bytes of the ranlib structs
 *  the ranlib structs
 *  a uint32_t for the number of bytes of the strings for the ranlibs
 *  the strings for the ranlib structs
 * and for a 64-bit toc the rest is this:
 *  a uint64_t for the number of bytes of the ranlib structs
 *  the ranlib_64 structs
 *  a uint64_t for the number of bytes of the strings for the ranlibs
 *  the strings for the ranlib structs
 */
static
char *
put_toc_member(
char *p,
struct arch *arch,
enum byte_sex host_byte_sex,
enum byte_sex target_byte_sex)
{
    uint32_t l;
    uint64_t l64;

	memcpy(p, (char *)&arch->toc_ar_hdr, sizeof(struct ar_hdr));
	p += sizeof(struct ar_hdr);

	if(arch->toc_long_name == TRUE){
	    memcpy(p, arch->toc_name, arch->toc_name_size);
	    p += arch->toc_name_size +
		 (rnd(sizeof(struct ar_hdr), 8) -
		  sizeof(struct ar_hdr));
	}

	if(arch->using_64toc == FALSE){
	    l = (uint32_t)(arch->toc_nranlibs * sizeof(struct ranlib));
	    if(target_byte_sex != host_byte_sex)
		l = SWAP_INT(l);
	    memcpy(p, (char *)&l, sizeof(uint32_t));
	    p += sizeof(uint32_t);

	    if(target_byte_sex != host_byte_sex)
		swap_ranlib(arch->toc_ranlibs, (uint32_t)arch->toc_nranlibs,
			    target_byte_sex);
	    memcpy(p, (char *)arch->toc_ranlibs,
		   arch->toc_nranlibs * sizeof(struct ranlib));
	    p += arch->toc_nranlibs * sizeof(struct ranlib);

	    l = (uint32_t)arch->toc_strsize;
	    if(target_byte_sex != host_byte_sex)
		l = SWAP_INT(l);
	    memcpy(p, (char *)&l, sizeof(uint32_t));
	    p += sizeof(uint32_t);

	    memcpy(p, (char *)arch->toc_strings, arch->toc_strsize);
	    p += arch->toc_strsize;
        }
	else{
	    l64 = arch->toc_nranlibs * sizeof(struct ranlib_64);
	    if(target_byte_sex != host_byte_sex)
		l64 = SWAP_LONG_LONG(l64);
	    memcpy(p, (char *)&l64, sizeof(uint64_t));
	    p += sizeof(uint64_t);

	    if(target_byte_sex != host_byte_sex)
		swap_ranlib_64(arch->toc_ranlibs64, arch->toc_nranlibs,
			       target_byte_sex);
	    memcpy(p, (char *)arch->toc_ranlibs64,
		   arch->toc_nranlibs * sizeof(struct ranlib_64));
	    p += arch->toc_nranlibs * sizeof(struct ranlib_64);

	    l64 = arch->toc_strsize;
	    if(target_byte_sex != host_byte_sex)
		l64 = SWAP_LONG_LONG(l64);
	    memcpy(p, (char *)&l64, sizeof(uint64_t));
	    p += sizeof(uint64_t);

	    memcpy(p, (char *)arch->toc_strings, arch->toc_strsize);
	    p += arch->toc_strsize;
	}

	return(p);
}

/*
 * output_flush() takes an offset and a size of part of the output library,
 * known in the comments as the new area, and causes any fully flushed pages to
 * be written to the library file the new area in combination with previous
 * areas created.  The data structure output_blocks has ordered blocks of areas
 * that have been flushed which are maintained by this routine.  Any area can
 * only be flushed once and an error will result is the new area overlaps with a
 * previously flushed area.
 */
static
void
output_flush(
char *library,
uint64_t library_size,
int fd,
uint64_t offset,
uint64_t size)
{ 
    uint64_t write_offset, write_size, host_pagesize;
    struct block **p, *block, *before, *after;
    kern_return_t r;

	host_pagesize = getpagesize();
	write_offset = 0;

	if(cmd_flags.noflush == TRUE)
	    return;

	if(offset + size > library_size)
	    fatal("internal error: output_flush(offset = %llu, size = %llu) "
		  "out of range for library_size = %llu", offset, size,
		  library_size);

#ifdef DEBUG
	if(cmd_flags.debug & (1 << 2))
	    print_block_list();
	if(cmd_flags.debug & (1 << 1))
	    printf("output_flush(offset = %llu, size %llu)", offset, size);
#endif /* DEBUG */

	if(size == 0){
#ifdef DEBUG
	if(cmd_flags.debug & (1 << 1))
	    printf("\n");
#endif /* DEBUG */
	    return;
	}

	/*
	 * Search through the ordered output blocks to find the block before the
	 * new area and after the new area if any exist.
	 */
	before = NULL;
	after = NULL;
	p = &(output_blocks);
	while(*p){
	    block = *p;
	    if(offset < block->offset){
		after = block;
		break;
	    }
	    else{
		before = block;
	    }
	    p = &(block->next);
	}

	/*
	 * Check for overlap of the new area with the block before and after the
	 * new area if there are such blocks.
	 */
	if(before != NULL){
	    if(before->offset + before->size > offset){
		warning("internal error: output_flush(offset = %llu, size = "
			"%llu) overlaps with flushed block(offset = %llu, "
			"size = %llu)", offset, size, before->offset,
			before->size);
		printf("calling abort()\n");	
		abort();
	    }
	}
	if(after != NULL){
	    if(offset + size > after->offset){
		warning("internal error: output_flush(offset = %llu, size = "
			"%llu) overlaps with flushed block(offset = %llu, "
			"size = %llu)", offset, size, after->offset,
			after->size);
		printf("calling abort()\n");	
		abort();
	    }
	}

	/*
	 * Now see how the new area fits in with the blocks before and after it
	 * (that is does it touch both, one or the other or neither blocks).
	 * For each case first the offset and size to write (write_offset and
	 * write_size) are set for the area of full pages that can now be
	 * written from the block.  Then the area written in the block
	 * (->written_offset and ->written_size) are set to reflect the total
	 * area in the block now written.  Then offset and size the block
	 * refers to (->offset and ->size) are set to total area of the block.
	 * Finally the links to others blocks in the list are adjusted if a
	 * block is added or removed.
	 *
	 * See if there is a block before the new area and the new area
	 * starts at the end of that block.
	 */
	if(before != NULL && before->offset + before->size == offset){
	    /*
	     * See if there is also a block after the new area and the new area
	     * ends at the start of that block.
	     */
	    if(after != NULL && offset + size == after->offset){
		/*
		 * This is the case where the new area exactly fill the area
		 * between two existing blocks.  The total area is folded into
		 * the block before the new area and the block after the new
		 * area is removed from the list.
		 */
		if(before->offset == 0 && before->written_size == 0){
		    write_offset = 0;
		    before->written_offset = 0;
		}
		else
		    write_offset =before->written_offset + before->written_size;
		if(after->written_size == 0)
		    write_size = trnc64(after->offset + after->size -
					write_offset, host_pagesize);
		else
		    write_size = trnc64(after->written_offset - write_offset,
					host_pagesize);
		if(write_size != 0){
		    before->written_size += write_size;
		}
		if(after->written_size != 0)
		    before->written_size += after->written_size;
		before->size += size + after->size;

		/* remove the block after the new area */
		before->next = after->next;
		remove_block(after);
	    }
	    else{
		/*
		 * This is the case where the new area starts at the end of the
		 * block just before it but does not end where the block after
		 * it (if any) starts.  The new area is folded into the block
		 * before the new area.
		 */
		write_offset = before->written_offset + before->written_size;
		write_size = trnc64(offset + size - write_offset,host_pagesize);
		if(write_size != 0)
		    before->written_size += write_size;
		before->size += size;
	    }
	}
	/*
	 * See if the new area and the new area ends at the start of the block
	 * after it (if any).
	 */
	else if(after != NULL && offset + size == after->offset){
	    /*
	     * This is the case where the new area ends at the begining of the
	     * block just after it but does not start where the block before it.
	     * (if any) ends.  The new area is folded into this block after the
	     * new area.
	     */
	    write_offset = rnd(offset, host_pagesize);
	    if(after->written_size == 0)
		write_size = trnc64(after->offset + after->size - write_offset,
				    host_pagesize);
	    else
		write_size = trnc64(after->written_offset - write_offset,
				    host_pagesize);
	    if(write_size != 0){
		after->written_offset = write_offset;
		after->written_size += write_size;
	    }
	    else if(write_offset != after->written_offset){
		after->written_offset = write_offset;
	    }
	    after->offset = offset;
	    after->size += size;
	}
	else{
	    /*
	     * This is the case where the new area neither starts at the end of
	     * the block just before it (if any) or ends where the block after
	     * it (if any) starts.  A new block is created and the new area is
	     * is placed in it.
	     */
	    write_offset = rnd64(offset, host_pagesize);
	    write_size = trnc64(offset + size - write_offset, host_pagesize);
	    block = get_block();
	    block->offset = offset;
	    block->size = size;
	    block->written_offset = write_offset;
	    block->written_size = write_size;
	    /*
	     * Insert this block in the ordered list in the correct place.
	     */
	    if(before != NULL){
		block->next = before->next;
		before->next = block;
	    }
	    else{
		block->next = output_blocks;
		output_blocks = block;
	    }
	}

	/*
	 * Now if there are full pages to write write them to the output file.
	 */
	if(write_size != 0){
#ifdef DEBUG
	if((cmd_flags.debug & (1 << 1)) || (cmd_flags.debug & (1 << 0)))
	    printf(" writing (write_offset = %llu write_size = %llu)\n",
		   write_offset, write_size);
#endif /* DEBUG */
	    lseek(fd, write_offset, L_SET);
	    if(write64(fd, library + write_offset, write_size) !=
	       (ssize_t)write_size)
		system_fatal("can't write to output file");
	    if((r = vm_deallocate(mach_task_self(), (vm_address_t)(library +
				  write_offset), write_size)) != KERN_SUCCESS)
		mach_fatal(r, "can't vm_deallocate() buffer for output file");
	}
#ifdef DEBUG
	else{
	    if(cmd_flags.debug & (1 << 1))
		printf(" no write\n");
	}
#endif /* DEBUG */
}

/*
 * final_output_flush() flushes the last part of the last page of the object
 * file if it does not round out to exactly a page.
 */
static
void
final_output_flush(
char *library,
int fd)
{ 
    struct block *block;
    uint64_t write_offset, write_size;
    kern_return_t r;

	write_offset = 0;

#ifdef DEBUG
	if((cmd_flags.debug & (1 << 1)) || (cmd_flags.debug & (1 << 0))){
	    printf("final_output_flush block_list:\n");
	    print_block_list();
	}
#endif /* DEBUG */

	write_size = 0;
	block = output_blocks;
	if(block != NULL){
	    if(block->offset != 0)
		fatal("internal error: first block not at offset 0");
	    if(block->written_size != 0){
		if(block->written_offset != 0)
		    fatal("internal error: first block written_offset not 0");
		write_offset = block->written_size;
		write_size = block->size - block->written_size;
	    }
	    else{
		write_offset = block->offset;
		write_size = block->size;
	    }
	    if(block->next != NULL)
		fatal("internal error: more than one block in final list");
	}
	if(write_size != 0){
#ifdef DEBUG
	    if((cmd_flags.debug & (1 << 1)) || (cmd_flags.debug & (1 << 1)))
		printf(" writing (write_offset = %llu write_size = %llu)\n",
		       write_offset, write_size);
#endif /* DEBUG */
	    lseek(fd, write_offset, L_SET);
	    if(write64(fd, library + write_offset, write_size) !=
	       (ssize_t)write_size)
		system_fatal("can't write to output file");
	    if((r = vm_deallocate(mach_task_self(), (vm_address_t)(library +
				  write_offset), write_size)) != KERN_SUCCESS)
		mach_fatal(r, "can't vm_deallocate() buffer for output file");
	}
	output_blocks = NULL;
}

#ifdef DEBUG
/*
 * print_block_list() prints the list of blocks.  Used for debugging.
 */
static
void
print_block_list(void)
{
    struct block **p, *block;

	p = &(output_blocks);
	if(*p == NULL)
	    printf("Empty block list\n");
	while(*p){
	    block = *p;
	    printf("block 0x%x\n", (unsigned int)block);
	    printf("    offset %llu\n", block->offset);
	    printf("    size %llu\n", block->size);
	    printf("    written_offset %llu\n", block->written_offset);
	    printf("    written_size %llu\n", block->written_size);
	    printf("    next 0x%x\n", (unsigned int)(block->next));
	    p = &(block->next);
	}
}
#endif /* DEBUG */

/*
 * get_block() returns a pointer to a new block.  This could be done by
 * allocating block of these placing them on a free list and and handing them
 * out.  For the initial release of this code this number is typicly low and not
 * a big win so each block just allocated and free'ed.
 */
static
struct block *
get_block(void)
{
    struct block *block;

	block = allocate(sizeof(struct block));
	return(block);
}

/*
 * remove_block() throws away the block specified.  See comments in get_block().
 */
static
void
remove_block(
struct block *block)
{
	free(block);
}

/*
 * trnc64() truncates the value 'v' to the power of two value 'r'.
 * If v is less than zero it returns zero.
 */

static
uint64_t
trnc64(
uint64_t v,
uint64_t r)
{
    if(((int64_t)v) < 0)
	return(0);
    return(v & ~(r - 1));
}

/*
 * create_dynamic_shared_library() creates a dynamic shared library from the
 * data structure pointed to by archs into the specified output file.  Only
 * when more than one architecture is in archs will a fat file be created.
 */
static
void
create_dynamic_shared_library(
char *output)
{
    uint32_t i, j;
    char *p, *filelist;
    struct stat stat_buf;
    enum bool use_force_cpusubtype_ALL;
    const struct arch_flag *family_arch_flag;

	/*
	 * If there is more than one architecture setup a signal handler to
	 * clean up the temporary files in case we get a signal.
	 */
	if(narchs > 1)
	    signal(SIGINT, create_dynamic_shared_library_cleanup);

	/*
	 * If -arch_only is specified with a specific cpusubtype other than the
	 * family cpusubtype do not use -force_cpusubtype_ALL as the user wants
	 * the output to be tagged with that cpusubtype.
	 */
	use_force_cpusubtype_ALL = TRUE;
	if(cmd_flags.arch_only_flag.name != NULL){
	    family_arch_flag = get_arch_family_from_cputype(
				    cmd_flags.arch_only_flag.cputype);
	    if(family_arch_flag != NULL){
		if((family_arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		   (cmd_flags.arch_only_flag.cpusubtype & ~CPU_SUBTYPE_MASK))
		    use_force_cpusubtype_ALL = FALSE;
	    }
	}

	/*
	 * For each architecture run ld(1) -dylib to create the dynamic shared
	 * library.
	 */
	for(i = 0; i < narchs || (i == 0 && narchs == 0); i++){
	    reset_execute_list();
	    add_execute_list_with_prefix("ld");
	    if(narchs != 0 && cmd_flags.arch_only_flag.name == NULL)
		add_execute_list("-arch_multiple");
	    if(archs != NULL){
		add_execute_list("-arch");
		if(use_force_cpusubtype_ALL == TRUE)
		    add_execute_list(archs[i].arch_flag.name);
		else
		    add_execute_list(cmd_flags.arch_only_flag.name);
	    }
	    add_execute_list("-dylib");
	    add_execute_list("-dynamic");
	    if(cmd_flags.all_load_flag_specified == FALSE ||
	       cmd_flags.all_load == TRUE)
		add_execute_list("-all_load");
	    if(use_force_cpusubtype_ALL == TRUE)
		add_execute_list("-force_cpusubtype_ALL");
	    add_execute_list("-no_arch_warnings");
	    if(cmd_flags.seg1addr != NULL){
		add_execute_list("-seg1addr");
		add_execute_list(cmd_flags.seg1addr);
	    }
	    if(cmd_flags.segs_read_only_addr != NULL){
		add_execute_list("-segs_read_only_addr");
		add_execute_list(cmd_flags.segs_read_only_addr);
	    }
	    if(cmd_flags.segs_read_write_addr != NULL){
		add_execute_list("-segs_read_write_addr");
		add_execute_list(cmd_flags.segs_read_write_addr);
	    }
	    if(cmd_flags.seg_addr_table != NULL){
		add_execute_list("-seg_addr_table");
		add_execute_list(cmd_flags.seg_addr_table);
	    }
	    if(cmd_flags.seg_addr_table_filename != NULL){
		add_execute_list("-seg_addr_table_filename");
		add_execute_list(cmd_flags.seg_addr_table_filename);
	    }
	    if(cmd_flags.compatibility != NULL){
		add_execute_list("-dylib_compatibility_version");
		add_execute_list(cmd_flags.compatibility);
	    }
	    if(cmd_flags.current != NULL){
		add_execute_list("-dylib_current_version");
		add_execute_list(cmd_flags.current);
	    }
	    if(cmd_flags.install_name != NULL){
		add_execute_list("-dylib_install_name");
		add_execute_list(cmd_flags.install_name);
	    }
	    else{
		if(narchs > 1){
		    add_execute_list("-dylib_install_name");
		    add_execute_list(cmd_flags.output);
		}
	    }
	    for(j = 0; j < cmd_flags.nldflags; j++)
		add_execute_list(cmd_flags.ldflags[j]);
	    for(j = 0; j < cmd_flags.nLdirs; j++)
		add_execute_list(cmd_flags.Ldirs[j]);

	    // Support using libtool on a systems without the SDK in '/'. This
	    // works because the shims that are included in 10.9 and forwards
	    // automatically inject SDKROOT into the environment of the actual
	    // tools. See <rdar://problem/14264125>.
	    const char *sdkroot = getenv("SDKROOT");

	    // If the SDKROOT environment variable is set and is an absolute
	    // path, then see if we can find dylib1.o inside it and use that if
	    // so.
	    enum bool use_dashl_dylib1o = TRUE;
	    if (sdkroot && sdkroot[0] == '/') {
		// Construct the path to the object file.
		char *sdk_dylib1o_path;
		int res = asprintf(&sdk_dylib1o_path, "%s/usr/lib/dylib1.o",
				   sdkroot);
		if (res > 0 && sdk_dylib1o_path) {
		    struct stat s;
		    // Add the full path if it exists.
		    if (stat(sdk_dylib1o_path, &s) == 0) {
			add_execute_list(sdk_dylib1o_path);
			use_dashl_dylib1o = FALSE;
		    }
		}
	    }

	    filelist = NULL;
	    for(j = 0; j < cmd_flags.nfiles; j++){
		if(cmd_flags.filelist[j] == NULL){
		    add_execute_list(cmd_flags.files[j]);
		}
		else{
		    if(cmd_flags.filelist[j] != filelist){
			add_execute_list("-filelist");
			add_execute_list(cmd_flags.filelist[j]);
			filelist = cmd_flags.filelist[j];
		    }
		}
	    }
	    if(narchs <= 1){
		add_execute_list("-o");
		add_execute_list(cmd_flags.output);
	    }
	    else{
		add_execute_list("-o");
		add_execute_list(makestr(cmd_flags.output, ".libtool.",
					 archs[i].arch_flag.name, NULL));
		if(cmd_flags.final_output_specified == FALSE){
		    add_execute_list("-final_output");
		    add_execute_list(cmd_flags.output);
		}
	    }
	    if(execute_list(cmd_flags.verbose) == 0)
		fatal("internal link edit command failed");
	}
	/*
	 * If there is more than one architecture then run lipo to put them
	 * in a fat file.
	 */
	if(narchs > 1){
	    reset_execute_list();
	    add_execute_list_with_prefix("lipo");
	    add_execute_list("-create");
	    add_execute_list("-output");
	    add_execute_list(cmd_flags.output);
	    for(i = 0; i < narchs; i++){
		add_execute_list(makestr(cmd_flags.output, ".libtool.",
					 archs[i].arch_flag.name, NULL));
	    }
	    if(execute_list(cmd_flags.verbose) == 0)
		fatal("internal lipo command failed");
	    for(i = 0; i < narchs; i++){
		p = makestr(cmd_flags.output, ".libtool.",
			    archs[i].arch_flag.name, NULL);
		if(unlink(p) == -1){
		    error("can't remove temporary file: %s", p);
		}
	    }
	}
	/*
	 * If we are doing prebinding then run objcunique on the
	 * output.
	 */
	if(cmd_flags.prebinding == TRUE){
	    if(stat("/usr/bin/objcunique", &stat_buf) != -1){
		reset_execute_list();
		add_execute_list_with_prefix("objcunique");
		add_execute_list(cmd_flags.output);
		add_execute_list("-prebind");
		for(j = 0; j < cmd_flags.nLdirs; j++)
		    add_execute_list(cmd_flags.Ldirs[j]);
		if(execute_list(cmd_flags.verbose) == 0)
		    fatal("internal objcunique command failed");
	    }
	}
}

/*
 * create_dynamic_shared_library_cleanup() is the signal handler to remove the
 * temporary files if more than one arch is being used.
 */
static
void
create_dynamic_shared_library_cleanup(
int sig)
{
    uint32_t i;

	for(i = 0; i < narchs; i++){
	    (void)unlink(makestr(cmd_flags.output, ".libtool.",
				 archs[i].arch_flag.name, NULL));
	}
	exit(EXIT_FAILURE);
}

/*
 * make_table_of_contents() make the table of contents for the specified arch
 * and fills in the toc_* fields in the arch.  Output is the name of the output
 * file for error messages.
 */
static
void
make_table_of_contents(
struct arch *arch,
char *output)
{
    uint32_t i, j, k, r, s, nsects, ncmds, n_strx;
    struct member *member;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
    char *strings;
    enum bool sorted, is_toc_symbol;
    char *ar_name;
    struct section *section;
    struct section_64 *section64;
    uint8_t n_type, n_sect;
#ifdef LTO_SUPPORT
    char *lto_toc_string;
#endif /* LTO_SUPPORT */

	symbols = NULL;
	symbols64 = NULL;
	/*
	 * First pass over the members to count how many ranlib structs are
	 * needed and the size of the strings in the toc that are needed.
	 */
	for(i = 0; i < arch->nmembers; i++){
	    member = arch->members + i;
	    if(member->mh != NULL || member->mh64 != NULL){
		nsects = 0;
		lc = member->load_commands;
		if(member->mh != NULL)
		    ncmds = member->mh->ncmds;
		else
		    ncmds = member->mh64->ncmds;
		for(j = 0; j < ncmds; j++){
		    if(lc->cmd == LC_SYMTAB){
			if(member->st == NULL)
			    member->st = (struct symtab_command *)lc;
		    }
		    else if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			nsects += sg->nsects;
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			nsects += sg64->nsects;
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		if(member->mh != NULL)
		    member->sections = allocate(nsects *
						sizeof(struct section *));
		else
		    member->sections64 = allocate(nsects *
						  sizeof(struct section_64 *));
		nsects = 0;
		lc = member->load_commands;
		for(j = 0; j < ncmds; j++){
		    if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			section = (struct section *)
				  ((char *)sg + sizeof(struct segment_command));
			for(k = 0; k < sg->nsects; k++){
			    member->sections[nsects++] = section++;
			}
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			section64 = (struct section_64 *)
			    ((char *)sg64 + sizeof(struct segment_command_64));
			for(k = 0; k < sg64->nsects; k++){
			    member->sections64[nsects++] = section64++;
			}
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		if(member->st != NULL && member->st->nsyms != 0){
		    if(member->mh != NULL){
			symbols = (struct nlist *)(member->object_addr +
					           member->st->symoff);
			if(member->object_byte_sex != get_host_byte_sex())
			    swap_nlist(symbols, member->st->nsyms,
				       get_host_byte_sex());
		    }
		    else{
			symbols64 = (struct nlist_64 *)(member->object_addr +
					                member->st->symoff);
			if(member->object_byte_sex != get_host_byte_sex())
			    swap_nlist_64(symbols64, member->st->nsyms,
				          get_host_byte_sex());
		    }
		    strings = member->object_addr + member->st->stroff;
		    for(j = 0; j < member->st->nsyms; j++){
			if(member->mh != NULL){
			    n_strx = symbols[j].n_un.n_strx;
			    n_type = symbols[j].n_type;
			    n_sect = symbols[j].n_sect;
			}
			else{
			    n_strx = symbols64[j].n_un.n_strx;
			    n_type = symbols64[j].n_type;
			    n_sect = symbols64[j].n_sect;
			}
			if(n_strx > member->st->strsize){
			    warn_member(arch, member, "malformed object "
				"(symbol %u n_strx field extends past the "
				"end of the string table)", j);
			    errors++;
			    continue;
			}
			if((n_type & N_TYPE) == N_SECT){
			    if(n_sect == NO_SECT){
				warn_member(arch, member, "malformed object "
				    "(symbol %u must not have NO_SECT for its "
				    "n_sect field given its type (N_SECT))", j);
				errors++;
				continue;
			    }
			    if(n_sect > nsects){
				warn_member(arch, member, "malformed object "
				    "(symbol %u n_sect field greater than the "
				    "number of sections in the file)", j);
				errors++;
				continue;
			    }
			}
			if(member->mh != NULL)
			    is_toc_symbol = toc_symbol(symbols + j,
						       member->sections);
			else
			    is_toc_symbol = toc_symbol_64(symbols64 + j,
						          member->sections64);
			if(is_toc_symbol == TRUE){
			    arch->toc_nranlibs++;
			    arch->toc_strsize += strlen(strings + n_strx) + 1;
			}
		    }
		}
		else{
		    if(cmd_flags.no_warning_for_no_symbols == FALSE)
			warn_member(arch, member, "has no symbols");
		}
	    }
#ifdef LTO_SUPPORT
	    else if(member->lto_contents == TRUE){
		arch->toc_nranlibs += member->lto_toc_nsyms;
		arch->toc_strsize += member->lto_toc_strsize;
	    }
#endif /* LTO_SUPPORT */
	    else{
		if(cmd_flags.ranlib == FALSE){
		    warn_member(arch, member, "is not an object file");
		    errors++;
		}
	    }
	}
	if(errors != 0)
	    return;

	/*
	 * Allocate the space for the ranlib structs and strings for the
	 * table of contents.
	 */
	arch->toc_ranlibs = allocate(sizeof(struct ranlib) *arch->toc_nranlibs);
	arch->tocs = allocate(sizeof(struct toc) * arch->toc_nranlibs);
	arch->toc_strsize = rnd(arch->toc_strsize, 8);
	arch->toc_strings = allocate(arch->toc_strsize);
	if(arch->toc_strsize >= 8)
	    memset(arch->toc_strings + arch->toc_strsize - 7, '\0', 7);

	/*
	 * Second pass over the members to fill in the toc structs and
	 * the strings for the table of contents.  The toc name field is
	 * filled in with a pointer to a string contained in arch->toc_strings
	 * for easy sorting and conversion to an index.  The toc index1 field is
	 * filled in with the member index plus one to allow marking with it's
	 * negative value by check_sort_tocs() and easy conversion to the
	 * real offset.
	 */
	r = 0;
	s = 0;
	for(i = 0; i < arch->nmembers; i++){
	    member = arch->members + i;
	    if(member->mh != NULL || member->mh64 != NULL){
		if(member->st != NULL && member->st->nsyms != 0){
		    if(member->mh != NULL)
			symbols = (struct nlist *)(member->object_addr +
					           member->st->symoff);
		    else
			symbols64 = (struct nlist_64 *)(member->object_addr +
					                member->st->symoff);
		    strings = member->object_addr + member->st->stroff;
		    for(j = 0; j < member->st->nsyms; j++){
			if(member->mh != NULL)
			    n_strx = symbols[j].n_un.n_strx;
			else
			    n_strx = symbols64[j].n_un.n_strx;
			if(n_strx > member->st->strsize)
			    continue;
			if(member->mh != NULL)
			    is_toc_symbol = toc_symbol(symbols + j,
						       member->sections);
			else
			    is_toc_symbol = toc_symbol_64(symbols64 + j,
						          member->sections64);
			if(is_toc_symbol == TRUE){
			    strcpy(arch->toc_strings + s,
				   strings + n_strx);
			    arch->tocs[r].name = arch->toc_strings + s;
			    arch->tocs[r].index1 = i + 1;
			    r++;
			    s += strlen(strings + n_strx) + 1;
			}
		    }
		    if(member->object_byte_sex != get_host_byte_sex()){
			if(member->mh != NULL)
			    swap_nlist(symbols, member->st->nsyms,
				       member->object_byte_sex);
			else
			    swap_nlist_64(symbols64, member->st->nsyms,
				          member->object_byte_sex);
		    }
		}
	    }
#ifdef LTO_SUPPORT
	    else if(member->lto_contents == TRUE){
		lto_toc_string = member->lto_toc_strings;
		for(j = 0; j < member->lto_toc_nsyms; j++){
		    strcpy(arch->toc_strings + s, lto_toc_string);
		    arch->tocs[r].name = arch->toc_strings + s;
		    arch->tocs[r].index1 = i + 1;
		    r++;
		    s += strlen(lto_toc_string) + 1;
		    lto_toc_string += strlen(lto_toc_string) + 1;
		}
	    }
#endif /* LTO_SUPPORT */
	}

	/*
	 * If the table of contents is to be sorted by symbol name then try to
	 * sort it and leave it sorted if no duplicates.
	 */
	if(cmd_flags.s == TRUE){
	    qsort(arch->tocs, arch->toc_nranlibs, sizeof(struct toc),
		  (int (*)(const void *, const void *))toc_name_qsort);
	    sorted = check_sort_tocs(arch, output, FALSE);
	    if(sorted == FALSE){
		qsort(arch->tocs, arch->toc_nranlibs, sizeof(struct toc),
		      (int (*)(const void *, const void *))toc_index1_qsort);
		arch->toc_name = SYMDEF;
		arch->toc_name_size = sizeof(SYMDEF) - 1;
		if(cmd_flags.use_long_names == TRUE){
		    arch->toc_long_name = TRUE;
		    /*
		     * This  assumes that "__.SYMDEF\0\0\0\0\0\0\0" is 16 bytes
		     * and
		     * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		     * is 4 bytes.
		     */
		    ar_name = AR_EFMT1 "20";
		    arch->toc_name_size = 16;
		    arch->toc_name = SYMDEF "\0\0\0\0\0\0\0";
		}
		else{
		    arch->toc_long_name = FALSE;
		    ar_name = arch->toc_name;
		}
	    }
	    else{
		/*
		 * Since the SYMDEF_SORTED is "__.SYMDEF SORTED" which contains
		 * a space, it should use extended format #1 if we can use long
		 * names.
		 */
		arch->toc_name = SYMDEF_SORTED;
		arch->toc_name_size = sizeof(SYMDEF_SORTED) - 1;
		if(cmd_flags.use_long_names == TRUE){
		    arch->toc_long_name = TRUE;
		    /*
		     * This assumes that "__.SYMDEF SORTED" is 16 bytes and
		     * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		     * is 4 bytes.
		     */
		    ar_name = AR_EFMT1 "20";
		}
		else{
		    arch->toc_long_name = FALSE;
		    ar_name = arch->toc_name;
		}
	    }
	}
	else{
	    sorted = FALSE;
	    arch->toc_name = SYMDEF;
	    arch->toc_name_size = sizeof(SYMDEF) - 1;
	    if(cmd_flags.use_long_names == TRUE){
		arch->toc_long_name = TRUE;
		/*
		 * This  assumes that "__.SYMDEF\0\0\0\0\0\0\0" is 16 bytes and
		 * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		 * is 4 bytes.
		 */
		ar_name = AR_EFMT1 "20";
		arch->toc_name_size = 16;
		arch->toc_name = SYMDEF "\0\0\0\0\0\0\0";
	    }
	    else{
		arch->toc_long_name = FALSE;
		ar_name = arch->toc_name;
	    }
	}

	/*
	 * Now set the ran_off and ran_un.ran_strx fields of the ranlib structs.
	 * To do this the size of the toc member must be know because it comes
	 * first in the library.  The size of the toc member is made up of the
	 * sizeof an archive header struct (the size of the name if a long name
	 * is used) then the toc which is (as defined in ranlib.h):
	 *	a uint32_t for the number of bytes of the ranlib structs
	 *	the ranlib structures
	 *	a uint32_t for the number of bytes of the strings
	 *	the strings
	 */
	arch->toc_size = (uint32_t)(sizeof(struct ar_hdr) +
			 sizeof(uint32_t) +
			 arch->toc_nranlibs * sizeof(struct ranlib) +
			 sizeof(uint32_t) +
			 arch->toc_strsize);
	/* add the size of the name is a long name is used */
	if(arch->toc_long_name == TRUE)
	    arch->toc_size += arch->toc_name_size +
			      (rnd(sizeof(struct ar_hdr), 8) -
			       sizeof(struct ar_hdr));

	/*
	 * Now with the size of the 32-bit toc known we can now see if it will
	 * work or if we have offsets to members that are more than 32-bits and
	 * we need to switch to the 64-bit toc, or switch to that if we are
	 * forcing a 64-bit toc via the command line option.
	 */
	if(cmd_flags.toc64 == TRUE)
	    arch->using_64toc = TRUE;
	else{
	    arch->using_64toc = FALSE;
	    for(i = 0; i < arch->nmembers; i++){
		if(arch->members[i].offset + SARMAG + arch->toc_size >
		   UINT32_MAX){
		    arch->using_64toc = TRUE;
		    break;
		}
	    }
	}
	if(arch->using_64toc){
	    if(cmd_flags.use_long_names == FALSE &&
	       cmd_flags.L_or_T_specified == TRUE)
		fatal("archive requires a 64-bit toc that must be aligned so "
		      "-T can't be specified");
	    arch->toc_long_name = TRUE;
	    if(sorted == FALSE){
		/*
		 * This  assumes that "__.SYMDEF_64\0\0\0\0" is 16 bytes
		 * and
		 * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		 * is 4 bytes.
		 */
		ar_name = AR_EFMT1 "20";
		arch->toc_name_size = 16;
		arch->toc_name = SYMDEF_64 "\0\0\0\0";
	    }
	    else{
		arch->toc_name = SYMDEF_64_SORTED;
		arch->toc_name_size = sizeof(SYMDEF_64_SORTED) - 1;
		/*
		 * This assumes that "__.SYMDEF_64 SORTED\0\0\0\0\0" is 24 bytes
		 * and
		 * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		 * is 4 bytes.
		 */
		ar_name = AR_EFMT1 "28";
		arch->toc_name_size = 24;
		arch->toc_name = SYMDEF_64_SORTED "\0\0\0\0\0";
	    }
	    /*
	     * Free the space for the 32-bit ranlib structs and allocate space
	     * for the 64-bit ranlib structs.
	     */
	    free(arch->toc_ranlibs);
	    arch->toc_ranlibs = NULL;
	    arch->toc_ranlibs64 = allocate(sizeof(struct ranlib_64) *
				           arch->toc_nranlibs);
	    /*
	     * Now the size of the toc member when it is a 64-bit toc can be
	     * set.  It is made up of the sizeof an archive header struct (the
	     * size of the name which is always a long name to get 8-byte
	     * alignment then the toc which is (as defined in ranlib.h):
	     *   a uint64_t for the number of bytes of the ranlib_64 structs
	     *   the ranlib_64 structures
	     *   a uint64_t for the number of bytes of the strings
	     *   the strings
	     */
	    arch->toc_size = (uint32_t)(sizeof(struct ar_hdr) +
			     sizeof(uint64_t) +
			     arch->toc_nranlibs * sizeof(struct ranlib_64) +
			     sizeof(uint64_t) +
			     arch->toc_strsize);
	    /* add the size of the name as a long name is always used */
	    arch->toc_size += arch->toc_name_size +
			      (rnd(sizeof(struct ar_hdr), 8) -
			       sizeof(struct ar_hdr));
	}

	for(i = 0; i < arch->nmembers; i++)
	    arch->members[i].offset += SARMAG + arch->toc_size;

	for(i = 0; i < arch->toc_nranlibs; i++){
	    if(arch->using_64toc){
		arch->toc_ranlibs64[i].ran_un.ran_strx =
		    arch->tocs[i].name - arch->toc_strings;
		arch->toc_ranlibs64[i].ran_off =
		    arch->members[arch->tocs[i].index1 - 1].offset;
	    }
	    else{
		arch->toc_ranlibs[i].ran_un.ran_strx =
		    (uint32_t)(arch->tocs[i].name - arch->toc_strings);
		arch->toc_ranlibs[i].ran_off =
		    (uint32_t)(arch->members[arch->tocs[i].index1 - 1].offset);
	    }
	}

	sprintf((char *)(&arch->toc_ar_hdr), "%-*s%-*ld%-*u%-*u%-*o%-*ld",
	   (int)sizeof(arch->toc_ar_hdr.ar_name),
	       ar_name,
	   (int)sizeof(arch->toc_ar_hdr.ar_date),
	       toc_time,
	   (int)sizeof(arch->toc_ar_hdr.ar_uid),
	       (unsigned short)toc_uid,
	   (int)sizeof(arch->toc_ar_hdr.ar_gid),
	       (unsigned short)toc_gid,
	   (int)sizeof(arch->toc_ar_hdr.ar_mode),
	       (unsigned int)toc_mode,
	   (int)sizeof(arch->toc_ar_hdr.ar_size),
	       (long)(arch->toc_size - sizeof(struct ar_hdr)));
	/*
	 * This has to be done by hand because sprintf puts a null
	 * at the end of the buffer.
	 */
	memcpy(arch->toc_ar_hdr.ar_fmag, ARFMAG,
	       (int)sizeof(arch->toc_ar_hdr.ar_fmag));
}

#ifdef LTO_SUPPORT
/*
 * save_lto_member_toc_info() saves away the table of contents info for a
 * member that has lto_content.  This allows the lto module to be disposed of
 * after reading to keep only on in memory at a time.  As these turn out to
 * use a lot of memory.
 */
static
void
save_lto_member_toc_info(
struct member *member,
void *mod)
{
    uint32_t i, nsyms;
    char *s;

        member->lto_toc_nsyms = 0;
	nsyms = lto_get_nsyms(mod);
	for(i = 0; i < nsyms; i++){
	    if(lto_toc_symbol(mod, i, cmd_flags.c) == TRUE){
		member->lto_toc_nsyms++;
		member->lto_toc_strsize += strlen(lto_symbol_name(mod, i)) + 1;
	    }
	}
	member->lto_toc_strings = allocate(member->lto_toc_strsize);
        s = member->lto_toc_strings;
	for(i = 0; i < nsyms; i++){
	    if(lto_toc_symbol(mod, i, cmd_flags.c) == TRUE){
		strcpy(s, lto_symbol_name(mod, i));
		s += strlen(lto_symbol_name(mod, i)) + 1;
	    }
	}
}
#endif /* LTO_SUPPORT */

/*
 * Function for qsort() for comparing toc structures by name.
 */
static
int
toc_name_qsort(
const struct toc *toc1,
const struct toc *toc2)
{
	return(strcmp(toc1->name, toc2->name));
}

/*
 * Function for qsort() for comparing toc structures by index1.
 */
static
int
toc_index1_qsort(
const struct toc *toc1,
const struct toc *toc2)
{
	if(toc1->index1 < toc2->index1)
	    return(-1);
	if(toc1->index1 > toc2->index1)
	    return(1);
	/* toc1->index1 == toc2->index1 */
	    return(0);
}

/*
 * toc_symbol() returns TRUE if the symbol is to be included in the table of
 * contents otherwise it returns FALSE.
 */
static
enum bool
toc_symbol(
struct nlist *symbol,
struct section **sections)
{
	return(toc(symbol->n_un.n_strx,
		   symbol->n_type,
		   symbol->n_value,
		   (symbol->n_type & N_TYPE) == N_SECT &&
		       sections[symbol->n_sect - 1]->flags & S_ATTR_NO_TOC));
}

static
enum bool
toc_symbol_64(
struct nlist_64 *symbol64,
struct section_64 **sections64)
{
	return(toc(symbol64->n_un.n_strx,
		   symbol64->n_type,
		   symbol64->n_value,
		   (symbol64->n_type & N_TYPE) == N_SECT &&
		       sections64[symbol64->n_sect-1]->flags & S_ATTR_NO_TOC));
}

static
enum bool
toc(
uint32_t n_strx,
uint8_t n_type,
uint64_t n_value,
enum bool attr_no_toc)
{
	/* if the name is NULL then it won't be in the table of contents */
	if(n_strx == 0)
	    return(FALSE);
	/* if symbol is not external then it won't be in the toc */
	if((n_type & N_EXT) == 0)
	    return(FALSE);
	/* if symbol is undefined then it won't be in the toc */
	if((n_type & N_TYPE) == N_UNDF && n_value == 0)
	    return(FALSE);
	/* if symbol is common and the -c flag is not specified then ... */
	if((n_type & N_TYPE) == N_UNDF && n_value != 0 &&
	   cmd_flags.c == FALSE)
	    return(FALSE);
	/* if the symbols is in a section marked NO_TOC then ... */
	if(attr_no_toc != 0)
	    return(FALSE);

	return(TRUE);
}

/*
 * check_sort_tocs() checks the table of contents for the specified arch
 * which is sorted by name for more then one object defining the same symbol.
 * It this is the case it prints each symbol that is defined in more than one
 * object along with the object it is defined in.  It returns TRUE if there are
 * no multiple definitions and FALSE otherwise.
 */
static
enum bool
check_sort_tocs(
struct arch *arch,
char *output,
enum bool library_warnings)
{
    uint32_t i;
    enum bool multiple_defs;
    struct member *member;

	if(arch->toc_nranlibs == 0)
	    return(TRUE);
	/*
	 * Since the symbol table is sorted by name look to any two adjcent
	 * entries with the same name.  If such entries are found print them
	 * only once (marked by changing the sign of their ran_off).
	 */
	multiple_defs = FALSE;
	for(i = 0; i < arch->toc_nranlibs - 1; i++){
	    if(strcmp(arch->tocs[i].name, arch->tocs[i+1].name) == 0){
		if(multiple_defs == FALSE){
		    if(library_warnings == FALSE)
			return(FALSE);
		    fprintf(stderr, "%s: same symbol defined in more than one "
			    "member ", progname);
		    if(narchs > 1)
			fprintf(stderr, "for architecture: %s ",
				arch->arch_flag.name);
		    fprintf(stderr, "in: %s (table of contents will not be "
			    "sorted)\n", output);
		    multiple_defs = TRUE;
		}
		if((int)(arch->tocs[i].index1) > 0){
		    member = arch->members + arch->tocs[i].index1 - 1;
		    warn_member(arch, member, "defines symbol: %s",
				arch->tocs[i].name);
		    arch->tocs[i].index1 =
				-(arch->tocs[i].index1);
		}
		if((int)(arch->tocs[i+1].index1) > 0){
		    member = arch->members + arch->tocs[i+1].index1 - 1;
		    warn_member(arch, member, "defines symbol: %s",
				arch->tocs[i+1].name);
		    arch->tocs[i+1].index1 =
				-(arch->tocs[i+1].index1);
		}
	    }
	}

	if(multiple_defs == FALSE)
	    return(TRUE);
	else{
	    for(i = 0; i < arch->toc_nranlibs; i++)
		if(((int)arch->tocs[i].index1) < 0)
		    arch->tocs[i].index1 =
			-(arch->tocs[i].index1);
	    return(FALSE);
	}
}

/*
 * warn_duplicate_member_names() generates a warning if two members end up with
 * the same ar_name.  This is only a warning because ld(1) and this program
 * has no problems with it.  Only if ar(1) were used to extract the files
 * would this be a problem (even the 4.4bsd ar(1) using long names can
 * get hosed by base names,  the 4.3bsd ar(1) can't handle full 16 character
 * ar_names).
 */
static
void
warn_duplicate_member_names(
void)
{
    uint32_t i, j, len, len1, len2;

	for(i = 0; i < narchs; i++){
	    /* sort in order of ar_names */
	    qsort(archs[i].members, archs[i].nmembers, sizeof(struct member),
		  (int (*)(const void *, const void *))member_name_qsort);

	    /* check for duplicate names */
	    for(j = 0; j < archs[i].nmembers - 1; j++){
		len1 = archs[i].members[j].member_name_size;
		len2 = archs[i].members[j+1].member_name_size;
		len = len1 > len2 ? len1 : len2;
		if(strncmp(archs[i].members[j].member_name,
			   archs[i].members[j+1].member_name,
			   len) == 0){
		    fprintf(stderr, "%s: warning ", progname);
		    if(narchs > 1)
			fprintf(stderr, "for architecture: %s ",
				archs[i].arch_flag.name);
		    fprintf(stderr, "same member name (%.*s) in output file "
			    "used for input files: ", (int)len1,
			    archs[i].members[j].member_name);

		    if(archs[i].members[j].input_ar_hdr != NULL){
			len = archs[i].members[j].input_base_name_size;
			fprintf(stderr, "%s(%.*s) and: ",
				archs[i].members[j].input_file_name, (int)len,
				archs[i].members[j].input_base_name);
		    }
		    else
			fprintf(stderr, "%s and: ",
				archs[i].members[j].input_file_name);

		    if(archs[i].members[j+1].input_ar_hdr != NULL){
			len = archs[i].members[j+1].input_base_name_size;
			fprintf(stderr, "%s(%.*s) due to use of basename, "
				"truncation and blank padding\n",
				archs[i].members[j+1].input_file_name, (int)len,
				archs[i].members[j+1].input_base_name);
		    }
		    else
			fprintf(stderr, "%s (due to use of basename, truncation"
				", blank padding or duplicate input files)\n",
				archs[i].members[j+1].input_file_name);
		}
	    }

	    /* sort back in order of offset */
	    qsort(archs[i].members, archs[i].nmembers, sizeof(struct member),
		  (int (*)(const void *, const void *))member_offset_qsort);
	}
}

/*
 * Function for qsort() for comparing member structures by ar_hdr.ar_name.
 */
static
int
member_name_qsort(
const struct member *member1,
const struct member *member2)
{
    uint32_t len, len1, len2;

	len1 = member1->member_name_size;
	len2 = member2->member_name_size;
	len = len1 > len2 ? len1 : len2;
	return(strncmp(member1->member_name, member2->member_name, len));
}

/*
 * Function for qsort() for comparing member structures by offset.
 */
static
int
member_offset_qsort(
const struct member *member1,
const struct member *member2)
{
	if(member1->offset < member2->offset)
	    return(-1);
	if(member1->offset > member2->offset)
	    return(1);
	/* member1->offset == member2->offset */
	    return(0);
}

/*
 * warn_member() is like the error routines it prints the program name the
 * member name specified and message specified.
 */
static
void
warn_member(
struct arch *arch,
struct member *member,
const char *format, ...)
{
    va_list ap;

	fprintf(stderr, "%s: ", progname);
	if(narchs > 1)
	    fprintf(stderr, "for architecture: %s ", arch->arch_flag.name);

	if(member->input_ar_hdr != NULL){
	    fprintf(stderr, "file: %s(%.*s) ", member->input_file_name,
		    (int)member->input_base_name_size, member->input_base_name);
	}
	else
	    fprintf(stderr, "file: %s ", member->input_file_name);

	va_start(ap, format);
	vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
	va_end(ap);
}

/*
 * Prints a message for the archive file specified by archive to
 * cmd_flags.trace_file_path, or stderr if that isn't set.
 */
static void ld_trace_archive(const char* archive)
{
    char resolvedname[MAXPATHLEN];
    const char* path = realpath(archive, resolvedname);

    if (path == NULL)
        path = archive;
    
    if (cmd_flags.ld_trace_dependents) {
        if (trace_buffer == NULL) {
            ld_trace_append("{\"archives\":[");
        } else {
            ld_trace_append(",");
        }
        ld_trace_append("\"%s\"", path);
    }
    else if (cmd_flags.ld_trace_archives) {
        ld_trace_append("[Logging for XBS] Used static archive: "
                        "%s\n", path);
    }
}

/*
 * ld_trace_close completes the trace logging process and writes the contents
 * of the trace buffer.
 *
 *   If logging to a JSON object, the object will be closed.
 *
 *   If LD_TRACE_FILE is present in the environment, the trace buffer will be
 *   written to the path so specified. The file will be created if missing, and
 *   appended to if present. An exclusive lock with flock(2) semantics will
 *   be held to prevent problems caused by concurrent writers.
 *
 *   If LD_TRACE_FILE is not present, the contents of the trace buffer will be
 *   written to stderr.
 */
static void ld_trace_close(void)
{
    int trace_file;

    if (cmd_flags.ld_trace_dependents && trace_buffer) {
        ld_trace_append("]}\n");
    }
    
#ifndef O_EXLOCK // cctools-port
#define O_EXLOCK 0
#endif

    if (trace_buffer) {
        if (cmd_flags.trace_file_path != NULL) {
            trace_file = open(cmd_flags.trace_file_path,
                              O_WRONLY | O_APPEND | O_CREAT | O_EXLOCK, 0666);
            if (trace_file == -1)
                error("Could not open or create trace file: %s\n",
                      cmd_flags.trace_file_path);
        }
        else {
            trace_file = fileno(stderr);
        }
        
        (void)write64(trace_file, trace_buffer, trace_buflen);
        /* Failure to write shouldn't fail the build. */
        
        close(trace_file);
    }
}

/*
 * ld_trace_append appends the message to trace_buffer.
 */
static
void
ld_trace_append(
         const char *format, ...)
{
    va_list ap;
    int length;

    va_start(ap, format);
    length = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    trace_buffer = realloc(trace_buffer, trace_buflen + length + 1);
    
    va_start(ap, format);
    vsnprintf(&trace_buffer[trace_buflen], length  + 1, format, ap);
    va_end(ap);
    
    trace_buflen += length;
}
