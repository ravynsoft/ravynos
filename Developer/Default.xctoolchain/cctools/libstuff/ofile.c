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
#define __darwin_i386_exception_state i386_exception_state
#define __darwin_i386_float_state i386_float_state
#define __darwin_i386_thread_state i386_thread_state

#ifndef RLD
#ifdef SHLIB
#include "shlib.h"
#endif
#include <libc.h>
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <ar.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#import <mach/i386/thread_status.h>
#undef MACHINE_THREAD_STATE
#undef MACHINE_THREAD_STATE_COUNT
#import <mach/arm/thread_status.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include "stuff/bool.h"
#ifdef OFI
#include <mach-o/dyld.h>
#else
#include "stuff/lto.h"
#endif
#include "stuff/bytesex.h"
#include "stuff/arch.h"
#include "stuff/rnd.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/ofile.h"
#include "stuff/print.h"

#ifdef OTOOL
#undef ALIGNMENT_CHECKS
#include "otool.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"
static enum bool otool_first_ofile_map = TRUE;
#else /* !define(OTOOL) */

/* cctools-port: Added    && !defined(__arm64__) && !defined(__aarch64__)  */
#if (!defined(m68k) && !defined(__i386__) && !defined(__x86_64__) && !defined(__ppc__) && !defined(__arm__) && !defined(__arm64__) && !defined(__aarch64__))
#define ALIGNMENT_CHECKS_ARCHIVE_64_BIT
static enum bool archive_64_bit_align_warning = FALSE;
#endif /* (!defined(m68k) && !defined(__i386__) && !defined(__x86_64__) && !defined(__ppc__) && !defined(__arm__) && !defined(__arm64__) && !defined(__aarch64__)) */

#endif /* OTOOL */

/* <mach/loader.h> */
/* The maximum section alignment allowed to be specified, as a power of two */
#define MAXSECTALIGN		15 /* 2**15 or 0x8000 */

enum check_type {
    CHECK_BAD,
    CHECK_GOOD
};

#ifndef OTOOL
struct element {
    uint32_t offset;
    uint32_t size;
    char *name;
    struct element *next;
};
#endif /* !defined(OTOOL) */

static enum bool ofile_specific_arch(
    struct ofile *ofile,
    uint32_t narch);
static enum check_type check_fat(
    struct ofile *ofile);
static enum check_type check_fat_object_in_archive(
    struct ofile *ofile);
static enum check_type check_archive(
    struct ofile *ofile,
    enum bool archives_with_fat_objects);
static enum check_type check_extend_format_1(
    struct ofile *ofile,
    struct ar_hdr *ar_hdr,
    uint32_t size_left,
    uint32_t *member_name_size);
static enum check_type check_archive_toc(
    struct ofile *ofile);
static enum check_type check_Mach_O(
    struct ofile *ofile);
static void swap_back_Mach_O(
    struct ofile *ofile);
#ifndef OTOOL
static enum check_type check_overlaping_element(
    struct ofile *ofile,
    struct element *head,
    uint32_t offset,
    uint32_t size,
    char *name);
static void free_elements(
    struct element *head);
#endif /* !defined(OTOOL) */
static enum check_type check_dylib_module(
    struct ofile *ofile,
    struct symtab_command *st,
    struct dysymtab_command *dyst,
    char *strings,
    uint32_t module_index);

#ifndef OTOOL
#if defined(ALIGNMENT_CHECKS) || defined(ALIGNMENT_CHECKS_ARCHIVE_64_BIT)
static
void
temporary_archive_member_warning(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    print("%s: for architecture %s archive member: %s(%.*s) ",
		  progname, ofile->arch_flag.name, ofile->file_name,
		  (int)ofile->member_name_size, ofile->member_name);
	}
	else{
	    print("%s: archive member: %s(%.*s) ", progname, ofile->file_name,
		  (int)ofile->member_name_size, ofile->member_name);
	}
	vprint(format, ap);
        print("\n");
	va_end(ap);
}
#endif /* defined(ALIGNMENT_CHECKS) */
#endif /* !defined(OTOOL) */

#ifndef OFI
/*
 * ofile_process() processes the specified file name can calls the routine
 * processor on the ofiles in it.  arch_flags is an array of architectures
 * containing narch_flags which are the only architectures to process if
 * narch_flags is non-zero.  If all_archs is TRUE then all architectures of
 * the specified file are processed.  The specified file name can be of the
 * form "archive(member)" which is taken to mean that member in that archive
 * (or that module of a dynamic library if dylib_flat is not FALSE).
 * For each ofile that is to be processed the routine processor is called with
 * the corresponding ofile struct, the arch_name pass to it is either NULL or
 * an architecture name (when it should be printed or show by processor) and
 * cookie is the same value as passed to ofile_process.
 */
__private_extern__
void
ofile_process(
char *name,
struct arch_flag *arch_flags,
uint32_t narch_flags,
enum bool all_archs,
enum bool process_non_objects,
enum bool dylib_flat,
enum bool use_member_syntax,
void (*processor)(struct ofile *ofile, char *arch_name, void *cookie),
void *cookie)
{
    char *member_name, *p, *arch_name;
    uint32_t i;
    size_t len;
    struct ofile ofile;
    enum bool flag, hostflag, arch_found, family;
    struct arch_flag host_arch_flag, specific_arch_flag;
    const struct arch_flag *family_arch_flag;

	/*
	 * If use_member_syntax is TRUE look for a name of the form
	 * "archive(member)" which is to mean a member in that archive (the
	 * member name must be at least one character long to be recognized as
	 * this form).
	 */
	member_name = NULL;
	if(use_member_syntax == TRUE){
	    len = strlen(name);
	    if(len >= 4 && name[len-1] == ')'){
		p = strrchr(name, '(');
		if(p != NULL && p != name){
		    member_name = p+1;
		    *p = '\0';
		    name[len-1] = '\0';
		}
	    }
	}

#ifdef OTOOL
	otool_first_ofile_map = TRUE;
#endif /* OTOOL */
	if(ofile_map(name, NULL, NULL, &ofile, FALSE) == FALSE)
	    return;
#ifdef OTOOL
	otool_first_ofile_map = FALSE;
#endif /* OTOOL */

	if(ofile.file_type == OFILE_FAT){
	    /*
	     * This is a fat file so see if a list of architecture is
	     * specified and process only those.
	     */
	    if(all_archs == FALSE && narch_flags != 0){
		for(i = 0; i < narch_flags; i++){
		    if(ofile_first_arch(&ofile) == FALSE){
			ofile_unmap(&ofile);
			return;
		    }
		    arch_found = FALSE;
		    family = FALSE;
		    family_arch_flag =
			get_arch_family_from_cputype(arch_flags[i].cputype);
		    if(family_arch_flag != NULL)
			family = (enum bool)
			  ((family_arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			   (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK));
		    if(narch_flags != 1)
			arch_name = ofile.arch_flag.name;
		    else
			arch_name = NULL;
		    do{
			if(ofile.arch_flag.cputype ==
				arch_flags[i].cputype &&
			   ((ofile.arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			    family == TRUE)){
			    arch_found = TRUE;
			    if(ofile.arch_type == OFILE_ARCHIVE){
				if(member_name != NULL){
				    if(ofile_specific_member(member_name,
							     &ofile) == TRUE){
					processor(&ofile, arch_name, cookie);
					if(ofile.headers_swapped == TRUE)
					    swap_back_Mach_O(&ofile);
				    }
				}
				else{
				    /* loop through archive */
#ifdef OTOOL
				    printf("Archive : %s", ofile.file_name);
				    if(arch_name != NULL)
					printf(" (architecture %s)",
					       arch_name);
				    printf("\n");
#endif /* OTOOL */
				    if(ofile_first_member(&ofile) == TRUE){
					flag = FALSE;
					do{
					    if(process_non_objects == TRUE ||
					       ofile.member_type ==
								OFILE_Mach_O){
						processor(&ofile, arch_name,
							  cookie);
						if(ofile.headers_swapped ==TRUE)
						    swap_back_Mach_O(&ofile);
						flag = TRUE;
					    }
					}while(ofile_next_member(&ofile) ==
						TRUE);
					if(flag == FALSE){
					    error("for architecture: %s "
						  "archive: %s contains no "
						  "members that are object "
						  "files", ofile.arch_flag.name,
						  ofile.file_name);
					}
				    }
				    else{
					error("for architecture: %s archive: "
					      "%s contains no members",
					      ofile.arch_flag.name,
					      ofile.file_name);
				    }
				}
			    }
			    else if(process_non_objects == TRUE ||
				    ofile.arch_type == OFILE_Mach_O){
				if(ofile.arch_type == OFILE_Mach_O &&
				   (ofile.mh_filetype == MH_DYLIB ||
				    ofile.mh_filetype == MH_DYLIB_STUB)){
				    if(dylib_flat == TRUE){
					processor(&ofile, arch_name, cookie);
					if(ofile.headers_swapped == TRUE)
					    swap_back_Mach_O(&ofile);
				    }
				    else{
					if(member_name != NULL){
					    if(ofile_specific_module(
						member_name, &ofile) == TRUE){
						processor(&ofile, arch_name,
							  cookie);
						if(ofile.headers_swapped ==TRUE)
						    swap_back_Mach_O(&ofile);
					    }
					}
					else{
					    /*loop through the dynamic library*/
					    if(ofile_first_module(&ofile)){
						do{
						    processor(&ofile, arch_name,
							cookie);
						}while(ofile_next_module(
							&ofile));
					    }
					    else{
						processor(&ofile, arch_name,
							  cookie);
					    }
					}
				    }
				    if(ofile.headers_swapped == TRUE)
					swap_back_Mach_O(&ofile);
				}
				else{
				    if(member_name != NULL)
					error("for architecture: %s file: %s "
					      "is not an archive and thus does "
					      "not contain member: %s",
					      ofile.arch_flag.name,
					      ofile.file_name,
					      member_name);
				    else{
					processor(&ofile, arch_name, cookie);
					if(ofile.headers_swapped == TRUE)
					    swap_back_Mach_O(&ofile);
				     }
				}
			    }
			    else if(ofile.arch_type == OFILE_UNKNOWN){
				error("for architecture: %s file: %s is "
				      "not an object file",
				      ofile.arch_flag.name,ofile.file_name);
			    }
			    if(ofile.headers_swapped == TRUE)
				swap_back_Mach_O(&ofile);
			    break;
			}
			else{
			    if(ofile.headers_swapped == TRUE)
				swap_back_Mach_O(&ofile);
			}
		    }while(ofile_next_arch(&ofile) == TRUE);
		    if(arch_found == FALSE)
			error("file: %s does not contain architecture: %s",
			      ofile.file_name, arch_flags[i].name);
		}
		ofile_unmap(&ofile);
		return;
	    }

	    /*
	     * This is a fat file and no architectures has been specified
	     * so if it contains the host architecture process only that
	     * architecture but if not process all architectures
	     * specified.
	     */
	    if(all_archs == FALSE){
		(void)get_arch_from_host(&host_arch_flag, &specific_arch_flag);
#if __LP64__
		/*
		 * If runing as a 64-bit binary and on an Intel x86 host
		 * default to 64-bit.
		 */
		if(host_arch_flag.cputype == CPU_TYPE_I386)
		    host_arch_flag =
			*get_arch_family_from_cputype(CPU_TYPE_X86_64);
#endif /* __LP64__ */
		hostflag = FALSE;

		family = FALSE;
		family_arch_flag =
		    get_arch_family_from_cputype(host_arch_flag.cputype);
#ifndef __arm__
		if(family_arch_flag != NULL)
		    family = (enum bool)
			((family_arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (host_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK));
#endif /* __arm__ */

		ofile_unmap(&ofile);
		if(ofile_map(name, NULL, NULL, &ofile, FALSE) == FALSE)
		    return;
		if(ofile_first_arch(&ofile) == FALSE){
		    ofile_unmap(&ofile);
		    return;
		}
		do{
		    if(ofile.arch_flag.cputype ==
			    host_arch_flag.cputype &&
		       ((ofile.arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
#ifdef __arm__
			(specific_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ||
#else
			(host_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ||
#endif /* __arm__ */
			family == TRUE)){
			hostflag = TRUE;
			if(ofile.arch_type == OFILE_ARCHIVE){
			    if(member_name != NULL){
				if(ofile_specific_member(member_name,
							 &ofile) == TRUE){
				    processor(&ofile, NULL, cookie);
				    if(ofile.headers_swapped == TRUE)
					swap_back_Mach_O(&ofile);
				}
			    }
			    else{
				/* loop through archive */
#ifdef OTOOL
				printf("Archive : %s\n", ofile.file_name);
#endif /* OTOOL */
				if(ofile_first_member(&ofile) == TRUE){
				    flag = FALSE;
				    do{
					if(process_non_objects == TRUE ||
				           ofile.member_type == OFILE_Mach_O){
					    processor(&ofile, NULL, cookie);
					    if(ofile.headers_swapped == TRUE)
						swap_back_Mach_O(&ofile);
					    flag = TRUE;
					}
				    }while(ofile_next_member(&ofile) ==
					   TRUE);
				    if(flag == FALSE){
					error("archive: %s contains no "
					      "members that are object "
					      "files", ofile.file_name);
				    }
				}
				else{
				    error("archive: %s contains no "
					  "members", ofile.file_name);
				}
			    }
			}
			else if(process_non_objects == TRUE ||
				ofile.arch_type == OFILE_Mach_O){
			    if(ofile.arch_type == OFILE_Mach_O &&
			       (ofile.mh_filetype == MH_DYLIB ||
				ofile.mh_filetype == MH_DYLIB_STUB)){
				if(dylib_flat == TRUE){
				    processor(&ofile, NULL, cookie);
				}
				else{
				    if(member_name != NULL){
					if(ofile_specific_module(member_name,
						&ofile) == TRUE)
					    processor(&ofile, NULL, cookie);
				    }
				    else{
					/* loop through the dynamic library */
					if(ofile_first_module(&ofile) == TRUE){
					    do{
						processor(&ofile, NULL, cookie);
					    }while(ofile_next_module(&ofile));
					}
					else{
					    processor(&ofile, NULL, cookie);
					}
				    }
				}
				if(ofile.headers_swapped == TRUE)
				    swap_back_Mach_O(&ofile);
			    }
			    else{
				if(member_name != NULL)
				    error("for architecture: %s file: %s is "
					  "not an archive and thus does not "
					  "contain member: %s",
					  ofile.arch_flag.name, ofile.file_name,
					  member_name);
				else{
				    processor(&ofile, NULL, cookie);
				    if(ofile.headers_swapped == TRUE)
					swap_back_Mach_O(&ofile);
				}
			    }
			}
			else if(ofile.arch_type == OFILE_UNKNOWN){
			    error("file: %s is not an object file",
				  ofile.file_name);
			}
		    }
		    else{
			if(ofile.headers_swapped == TRUE)
			    swap_back_Mach_O(&ofile);
		    }
		}while(hostflag == FALSE && ofile_next_arch(&ofile) == TRUE);
		if(hostflag == TRUE){
		    ofile_unmap(&ofile);
		    return;
		}
	    }

	    /*
	     * Either all architectures have been specified or none have
	     * been specified and it does not contain the host architecture
	     * so do all the architectures in the fat file
	     */
	    ofile_unmap(&ofile);
	    if(ofile_map(name, NULL, NULL, &ofile, FALSE) == FALSE)
		return;
	    if(ofile_first_arch(&ofile) == FALSE){
		ofile_unmap(&ofile);
		return;
	    }
	    do{
		if(ofile.arch_type == OFILE_ARCHIVE){
		    if(member_name != NULL){
			if(ofile_specific_member(member_name, &ofile) == TRUE)
			    processor(&ofile, ofile.arch_flag.name, cookie);
		    }
		    else{
			/* loop through archive */
#ifdef OTOOL
			printf("Archive : %s (architecture %s)\n",
			       ofile.file_name, ofile.arch_flag.name);
#endif /* OTOOL */
			if(ofile_first_member(&ofile) == TRUE){
			    flag = FALSE;
			    do{
				if(process_non_objects == TRUE ||
				   ofile.member_type == OFILE_Mach_O){
				    processor(&ofile, ofile.arch_flag.name,
					      cookie);
				    flag = TRUE;
				}
			    }while(ofile_next_member(&ofile) == TRUE);
			    if(flag == FALSE){
				error("for architecture: %s archive: %s "
				      "contains no members that are object "
				      "files", ofile.arch_flag.name,
				      ofile.file_name);
			    }
			}
			else{
			    error("for architecture: %s archive: %s "
				  "contains no members",
				  ofile.arch_flag.name, ofile.file_name);
			}
		    }
		}
		else if(process_non_objects == TRUE ||
			ofile.arch_type == OFILE_Mach_O){
		    if(ofile.arch_type == OFILE_Mach_O &&
		       (ofile.mh_filetype == MH_DYLIB ||
			ofile.mh_filetype == MH_DYLIB_STUB)){
			if(dylib_flat == TRUE){
			    processor(&ofile, ofile.arch_flag.name, cookie);
			}
			else{
			    if(member_name != NULL){
				if(ofile_specific_module(member_name, &ofile)
				   == TRUE)
				    processor(&ofile, ofile.arch_flag.name,
					      cookie);
			    }
			    else{
				/* loop through the dynamic library */
				if(ofile_first_module(&ofile) == TRUE){
				    do{
					processor(&ofile, ofile.arch_flag.name,
						  cookie);
				    }while(ofile_next_module(&ofile) == TRUE);
				}
				else{
				    processor(&ofile, ofile.arch_flag.name,
					      cookie);
				}
			    }
			}
		    }
		    else{
			if(member_name != NULL)
			    error("for architecture: %s file: %s is not an "
				  "archive and thus does not contain member: "
				  "%s", ofile.arch_flag.name, ofile.file_name,
				  member_name);
			else
			    processor(&ofile, ofile.arch_flag.name, cookie);
		    }
		}
		else if(ofile.arch_type == OFILE_UNKNOWN){
		    error("for architecture: %s file: %s is not an "
			  "object file", ofile.arch_flag.name,
			  ofile.file_name);
		}
	    }while(ofile_next_arch(&ofile) == TRUE);
	}
	else if(ofile.file_type == OFILE_ARCHIVE){
	    if(narch_flags != 0){
		arch_found = FALSE;
		for(i = 0; i < narch_flags; i++){
		    family = FALSE;
		    if(narch_flags == 1){
			family_arch_flag =
			    get_arch_family_from_cputype(arch_flags[0].cputype);
			if(family_arch_flag != NULL)
			    family = (enum bool)
				((family_arch_flag->cpusubtype &
				  ~CPU_SUBTYPE_MASK) ==
				 (arch_flags[0].cpusubtype &
				  ~CPU_SUBTYPE_MASK));
		    }
		    if(ofile.archive_cputype == arch_flags[i].cputype &&
		       ((ofile.archive_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			(arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			family == TRUE)){
			arch_found = TRUE;
		    }
		    else{
			error("file: %s does not contain architecture: %s",
			      ofile.file_name, arch_flags[i].name);
		    }
		}
		if(arch_found == FALSE){
		    ofile_unmap(&ofile);
		    return;
		}
	    }
	    if(member_name != NULL){
		if(ofile_specific_member(member_name, &ofile) == TRUE)
		    processor(&ofile, NULL, cookie);
	    }
	    else{
		/* loop through archive */
#ifdef OTOOL
		printf("Archive : %s\n", ofile.file_name);
#endif /* OTOOL */
		if(ofile_first_member(&ofile) == TRUE){
		    flag = FALSE;
		    do{
			if(process_non_objects == TRUE ||
			    ofile.member_type == OFILE_Mach_O){
			    processor(&ofile, NULL, cookie);
			    flag = TRUE;
			}
		    }while(ofile_next_member(&ofile) == TRUE);
		    if(flag == FALSE){
			error("archive: %s contains no members that are "
			      "object files", ofile.file_name);
		    }
		}
		else{
		    error("archive: %s contains no members",
			  ofile.file_name);
		}
	    }
	}
	else if(ofile.file_type == OFILE_Mach_O){
	    if(narch_flags != 0){
		arch_found = FALSE;
		for(i = 0; i < narch_flags; i++){
		    family = FALSE;
		    if(narch_flags == 1){
			family_arch_flag =
			    get_arch_family_from_cputype(arch_flags[0].cputype);
			if(family_arch_flag != NULL)
			    family = (enum bool)
				((family_arch_flag->cpusubtype &
				  ~CPU_SUBTYPE_MASK) ==
				 (arch_flags[0].cpusubtype &
				  ~CPU_SUBTYPE_MASK));
		    }
#ifdef OTOOL
		    if(ofile.mh != NULL){
		        if(ofile.mh->magic == MH_MAGIC &&
			   ofile.mh->cputype == arch_flags[i].cputype &&
			   ((ofile.mh->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			    family == TRUE)){
			    arch_found = TRUE;
			}
		        if(ofile.mh->magic == SWAP_INT(MH_MAGIC) &&
			   (cpu_type_t)SWAP_INT(ofile.mh->cputype) ==
				arch_flags[i].cputype &&
			   ((cpu_subtype_t)SWAP_INT(ofile.mh->cpusubtype &
						    ~CPU_SUBTYPE_MASK) ==
				(arch_flags[i].cpusubtype &
				 ~CPU_SUBTYPE_MASK) ||
			    family == TRUE)){
			    arch_found = TRUE;
			}
		    }
		    else if(ofile.mh64 != NULL){
		        if(ofile.mh64->magic == MH_MAGIC_64 &&
			   ofile.mh64->cputype == arch_flags[i].cputype &&
			   ((ofile.mh64->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			    family == TRUE)){
			    arch_found = TRUE;
			}
		        if(ofile.mh64->magic == SWAP_INT(MH_MAGIC_64) &&
			   (cpu_type_t)SWAP_INT(ofile.mh64->cputype) ==
				arch_flags[i].cputype &&
			   ((cpu_subtype_t)SWAP_INT((ofile.mh64->cpusubtype &
						     ~CPU_SUBTYPE_MASK)) ==
			    (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			    family == TRUE)){
			    arch_found = TRUE;
			}
		    }
		    else
#endif /* OTOOL */
		    if(ofile.mh_cputype == arch_flags[i].cputype &&
		       ((ofile.mh_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			(arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ||
			family == TRUE)){
			arch_found = TRUE;
		    }
		    else{
			error("file: %s does not contain architecture: %s",
			      ofile.file_name, arch_flags[i].name);
		    }
		}
		if(arch_found == FALSE){
		    ofile_unmap(&ofile);
		    return;
		}
	    }
	    if(ofile.mh_filetype == MH_DYLIB ||
	       ofile.mh_filetype == MH_DYLIB_STUB){
		if(dylib_flat == TRUE){
		    processor(&ofile, NULL, cookie);
		}
		else{
		    if(member_name != NULL){
			if(ofile_specific_module(member_name, &ofile) == TRUE)
			    processor(&ofile, NULL, cookie);
		    }
		    else{
			/* loop through the dynamic library */
			if(ofile_first_module(&ofile) == TRUE){
			    do{
				processor(&ofile, NULL, cookie);
			    }while(ofile_next_module(&ofile) == TRUE);
			}
			else{
			    processor(&ofile, NULL, cookie);
			}
		    }
		}
	    }
	    else{
		if(member_name != NULL)
		    error("file: %s is not an archive and thus does not contain"
			  " member: %s", ofile.file_name, member_name);
		else
		    processor(&ofile, NULL, cookie);
	    }
	}
	else{
	    if(process_non_objects == TRUE)
		processor(&ofile, NULL, cookie);
	    else if(member_name != NULL)
		error("file: %s(%s) is not an object file", name,
		      member_name);
	    else
		error("file: %s is not an object file", name);
	}
	ofile_unmap(&ofile);
}
#endif /* !defined(OFI) */

/*
 * ofile_map maps in the object file specified by file_name, arch_flag and
 * object_name and fills in the ofile struct pointed to by ofile for it.
 * When arch_flag and object_name are both NULL, the file is just set up into
 * ofile (if the file can be opened and mapped in, if not this call fails
 * are error routnes are called).  If arch_flag is not NULL and object_file is
 * NULL, then the file must be a Mach-O file or a fat file with the architecture
 * specified in the arch_flag, if not this call fails and error routines are
 * called.  When arch_flag and object_name are both not NULL, then the file must
 * be an archive or a fat file containing archives for the specified architec-
 * ture and contain an archive member object file with the name object_name,
 * otherwise this call fails and error routines are called.  If arch_flag is
 * NULL and object_file is not NULL, then the file name must be an archive (not
 * a fat file containing archives) and contain an archive member object file
 * with the name object_name, otherwise this call fails and calls error
 * routines.  If this call suceeds then it returns non-zero and the ofile
 * structure pointed to by ofile is filled in.  If this call fails it returns 0
 * and calls error routines to print error messages and clears the
 * ofile structure pointed to by ofile.
 */
__private_extern__
#ifdef OFI
NSObjectFileImageReturnCode
#else
enum bool
#endif
ofile_map(
const char *file_name,
const struct arch_flag *arch_flag,	/* can be NULL */
const char *object_name,		/* can be NULL */
struct ofile *ofile,
enum bool archives_with_fat_objects)
{
    int fd;
    struct stat stat_buf;
    uint64_t size;
    uint32_t magic;
    char *addr;

	magic = 0; /* to shut up the compiler warning message */
	memset(ofile, '\0', sizeof(struct ofile));

	/* Open the file and map it in */
	if((fd = open(file_name, O_RDONLY)) == -1){
#ifdef OFI
	    return(NSObjectFileImageAccess);
#else
	    system_error("can't open file: %s", file_name);
	    return(FALSE);
#endif
	}
	if(fstat(fd, &stat_buf) == -1){
	    close(fd);
#ifdef OFI
	    return(NSObjectFileImageAccess);
#else
	    system_error("can't stat file: %s", file_name);
	    return(FALSE);
#endif
	}
	size = stat_buf.st_size;
	
	addr = NULL;
	if(size != 0){
	    addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd,
		        0);
	    if((intptr_t)addr == -1){
		system_error("can't map file: %s", file_name);
		close(fd);
		return(FALSE);
	    }
	}
	close(fd);
#ifdef OTOOL
	if(otool_first_ofile_map && Wflag)
	    printf("Modification time = %ld\n", (long int)stat_buf.st_mtime);
#endif /* OTOOL */

	return(ofile_map_from_memory(addr, size, file_name, stat_buf.st_mtime,
		  arch_flag, object_name, ofile, archives_with_fat_objects));
}

/*
 * ofile_map_from_memory() is the guts of ofile_map() but with an interface
 * to pass the address and size of the file already mapped in.
 */
__private_extern__
#ifdef OFI
NSObjectFileImageReturnCode
#else
enum bool
#endif
ofile_map_from_memory(
char *addr,
uint64_t size,
const char *file_name,
uint64_t mtime,
const struct arch_flag *arch_flag,	/* can be NULL */
const char *object_name,		/* can be NULL */
struct ofile *ofile,
enum bool archives_with_fat_objects)
{
    uint32_t i;
    uint32_t magic;
    enum byte_sex host_byte_sex;
    struct arch_flag host_arch_flag;
    enum bool family;
    const struct arch_flag *family_arch_flag;
    uint64_t big_size;
#ifdef OTOOL
    uint32_t small_nfat_arch;
#endif /* OTOOL */
    uint64_t offset;
    uint32_t align;

	/* silence clang warning */
	magic = 0;

	/* fill in the start of the ofile structure */
	ofile->file_name = savestr(file_name);
	if(ofile->file_name == NULL)
	    return(FALSE);
	ofile->file_addr = addr;
	ofile->file_size = size;
	ofile->file_mtime = mtime;

	/* Try to figure out what kind of file this is */

	if(size >= sizeof(uint32_t)){
	   magic = *((uint32_t *)addr);
	}
	host_byte_sex = get_host_byte_sex();

	/* see if this file is a fat file (always in big endian byte sex) */
#ifdef __BIG_ENDIAN__
	if(size >= sizeof(struct fat_header) &&
	   (magic == FAT_MAGIC || magic == FAT_MAGIC_64))
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	if(size >= sizeof(struct fat_header) &&
	   (SWAP_INT(magic) == FAT_MAGIC || SWAP_INT(magic) == FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
	{
	    ofile->file_type = OFILE_FAT;
	    ofile->fat_header = (struct fat_header *)addr;
#ifdef __LITTLE_ENDIAN__
	    swap_fat_header(ofile->fat_header, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
#ifdef OTOOL
	    if(otool_first_ofile_map && fflag)
		printf("Fat headers\n");
#endif /* OTOOL */
	    big_size = ofile->fat_header->nfat_arch;
	    if(ofile->fat_header->magic == FAT_MAGIC_64)
		big_size *= sizeof(struct fat_arch_64);
	    else
		big_size *= sizeof(struct fat_arch);
	    big_size += sizeof(struct fat_header);
	    if(big_size > size){
#ifdef OTOOL
		error("fat file: %s truncated or malformed (fat_arch%s structs "
		      "would extend past the end of the file)", file_name,
		      ofile->fat_header->magic == FAT_MAGIC_64 ? "_64" : "");
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    ofile->fat_archs64 = allocate(size -
						   sizeof(struct fat_header));
		    memset(ofile->fat_archs64, '\0',
			   size - sizeof(struct fat_header));
		    memcpy(ofile->fat_archs64,
			   addr + sizeof(struct fat_header),
			   size - sizeof(struct fat_header));
		    small_nfat_arch = (uint32_t)
			    ((size - sizeof(struct fat_header)) /
			     sizeof(struct fat_arch_64));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch_64(ofile->fat_archs64, small_nfat_arch,
				     host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		else{
		    ofile->fat_archs = allocate(size -
						sizeof(struct fat_header));
		    memset(ofile->fat_archs, '\0',
			   size - sizeof(struct fat_header));
		    memcpy(ofile->fat_archs,
			   addr + sizeof(struct fat_header),
			   size - sizeof(struct fat_header));
		    small_nfat_arch = (uint32_t)
			    ((size - sizeof(struct fat_header)) /
			    sizeof(struct fat_arch));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch(ofile->fat_archs, small_nfat_arch,
				  host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		if(otool_first_ofile_map && fflag)
		    print_fat_headers(ofile->fat_header, ofile->fat_archs,
				      ofile->fat_archs64, size, vflag);
		if(ofile->fat_header->magic == FAT_MAGIC_64)
		    free(ofile->fat_archs64);
		else
		    free(ofile->fat_archs);
		ofile_unmap(ofile);
		return(FALSE);
#else /* !defined(OTOOL) */
		goto unknown;
#endif /* OTOOL */
	    }
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		ofile->fat_archs64 = (struct fat_arch_64 *)
				     (addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		swap_fat_arch_64(ofile->fat_archs64,
				 ofile->fat_header->nfat_arch, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
	    }
	    else{
		ofile->fat_archs = (struct fat_arch *)
				   (addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		swap_fat_arch(ofile->fat_archs, ofile->fat_header->nfat_arch,
			      host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
	    }
#ifdef OTOOL
	    if(otool_first_ofile_map && fflag)
		print_fat_headers(ofile->fat_header, ofile->fat_archs,
				  ofile->fat_archs64, size, vflag);
#endif /* OTOOL */
	    if(check_fat(ofile) == CHECK_BAD){
		ofile_unmap(ofile);
#ifdef OFI
		return(NSObjectFileImageFormat);
#else
		return(FALSE);
#endif
	    }
	    /*
	     * Now that the fat file is mapped fill in the ofile to the level
	     * the caller wants based on the arch_flag and object_name passed.
	     * If the caller did not specify an arch_flag or an object_name
	     * then everything the caller wants is done.
	     */
	    if(arch_flag == NULL && object_name == NULL)
		goto success;
	    if(arch_flag == NULL){
		if(get_arch_from_host(&host_arch_flag, NULL) == 0){
		    error("can't determine the host architecture (specify an "
			  "arch_flag or fix get_arch_from_host() )");
		    goto cleanup;
		}
		ofile->arch_flag.name = savestr(host_arch_flag.name);
		if(ofile->arch_flag.name == NULL)
		    goto cleanup;
		ofile->arch_flag.cputype = host_arch_flag.cputype;
		ofile->arch_flag.cpusubtype = host_arch_flag.cpusubtype;
	    }
	    else{
		ofile->arch_flag.name = savestr(arch_flag->name);
		if(ofile->arch_flag.name == NULL)
		    goto cleanup;
		ofile->arch_flag.cputype = arch_flag->cputype;
		ofile->arch_flag.cpusubtype = arch_flag->cpusubtype;
	    }

	    ofile->narch = UINT_MAX;
	    for(i = 0; i < ofile->fat_header->nfat_arch; i++){
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    if(ofile->fat_archs64[i].cputype ==
			    ofile->arch_flag.cputype &&
		       (ofile->fat_archs64[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (ofile->arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK)){
			ofile->narch = i;
			break;
		    }
		}
		else{
		    if(ofile->fat_archs[i].cputype ==
			    ofile->arch_flag.cputype &&
		       (ofile->fat_archs[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (ofile->arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK)){
			ofile->narch = i;
			break;
		    }
		}
	    }
	    if(ofile->narch == UINT_MAX){
		family = FALSE;
		family_arch_flag =
		    get_arch_family_from_cputype(ofile->arch_flag.cputype);
		if(family_arch_flag != NULL)
		    family = (enum bool)
			((family_arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (ofile->arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK));
		ofile->narch = UINT_MAX;
		for(i = 0; i < ofile->fat_header->nfat_arch; i++){
		    if(ofile->fat_header->magic == FAT_MAGIC_64){
			if(ofile->fat_archs64[i].cputype ==
				ofile->arch_flag.cputype &&
			   (family == TRUE ||
			    (ofile->fat_archs64[i].cpusubtype &
			     ~CPU_SUBTYPE_MASK) ==
			    (ofile->arch_flag.cpusubtype &
			     ~CPU_SUBTYPE_MASK))){
			    ofile->arch_flag.cpusubtype =
				ofile->fat_archs64[i].cpusubtype;
			    ofile->narch = i;
			    break;
			}
		    }
		    else{
			if(ofile->fat_archs[i].cputype ==
				ofile->arch_flag.cputype &&
			   (family == TRUE ||
			    (ofile->fat_archs[i].cpusubtype &
			     ~CPU_SUBTYPE_MASK) ==
			    (ofile->arch_flag.cpusubtype &
			     ~CPU_SUBTYPE_MASK))){
			    ofile->arch_flag.cpusubtype =
				ofile->fat_archs[i].cpusubtype;
			    ofile->narch = i;
			    break;
			}
		    }
		}
	    }
	    if(ofile->narch == UINT_MAX){
#ifdef OFI
		ofile_unmap(ofile);
		return(NSObjectFileImageArch);
#else
		error("fat file: %s does not contain architecture %s",
		      ofile->file_name, arch_flag->name);
		ofile_unmap(ofile);
		return(FALSE);
#endif
	    }
	    /* Now determine the file type for this specific architecture */
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		size = ofile->fat_archs64[i].size;
		addr = addr + ofile->fat_archs64[i].offset;
		offset = ofile->fat_archs64[i].offset;
		align = ofile->fat_archs64[i].align;
	    }
	    else{
		size = ofile->fat_archs[i].size;
		addr = addr + ofile->fat_archs[i].offset;
		offset = ofile->fat_archs[i].offset;
		align = ofile->fat_archs[i].align;
	    }
	    if(size >= sizeof(struct mach_header))
		memcpy(&magic, addr, sizeof(uint32_t));
	    /* see if this file is a 32-bit Mach-O file */
	    if(size >= sizeof(struct mach_header) &&
	       (magic == MH_MAGIC ||
		magic == SWAP_INT(MH_MAGIC))){
#ifdef ALIGNMENT_CHECKS
		if(offset % 4 != 0){
		    error("fat file: %s architecture %s malformed for a 32-bit "
			  "object file (offset is not a multiple of 4)",
			  ofile->file_name, arch_flag->name);
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageFormat);
#else
		    return(FALSE);
#endif
		}
#endif /* ALIGNMENT_CHECKS */
		ofile->arch_type = OFILE_Mach_O;
		ofile->object_addr = addr;
		ofile->object_size = (uint32_t)size;
		if(magic == MH_MAGIC)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh = (struct mach_header *)addr;
		ofile->load_commands = (struct load_command *)(addr +
					    sizeof(struct mach_header));
		if(check_Mach_O(ofile) == CHECK_BAD){
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageFormat);
#else
		    return(FALSE);
#endif
		}
		if(object_name != NULL){
		    error("fat file: %s architecture %s is not an archive "
			  "(object_name to ofile_map() can't be specified to "
			  "be other than NULL)", ofile->file_name,
			  arch_flag->name);
		    goto cleanup;
		}
	    }
	    /* see if this file is a 64-bit Mach-O file */
	    else if(size >= sizeof(struct mach_header_64) &&
	            (magic == MH_MAGIC_64 ||
		     magic == SWAP_INT(MH_MAGIC_64))){
#ifdef ALIGNMENT_CHECKS
		if(offset % 8 != 0){
		    error("fat file: %s architecture %s malformed for a 64-bit "
			  "object file (offset is not a multiple of 8)",
			  ofile->file_name, arch_flag->name);
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageFormat);
#else
		    return(FALSE);
#endif
		}
#endif /* ALIGNMENT_CHECKS */
		ofile->arch_type = OFILE_Mach_O;
		ofile->object_addr = addr;
		ofile->object_size = (uint32_t)size;
		if(magic == MH_MAGIC_64)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh64 = (struct mach_header_64 *)addr;
		ofile->load_commands = (struct load_command *)(addr +
					sizeof(struct mach_header_64));
		if(check_Mach_O(ofile) == CHECK_BAD){
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageFormat);
#else
		    return(FALSE);
#endif
		}
		if(object_name != NULL){
		    error("fat file: %s architecture %s is not an archive "
			  "(object_name to ofile_map() can't be specified to "
			  "be other than NULL)", ofile->file_name,
			  arch_flag->name);
		    goto cleanup;
		}
	    }
	    /* see if this file is an archive file */
	    else if(size >= SARMAG && strncmp(addr, ARMAG, SARMAG) == 0){
		ofile->arch_type = OFILE_ARCHIVE;
		if(check_archive(ofile, FALSE) == CHECK_BAD){
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageInappropriateFile);
#else
		    return(FALSE);
#endif
		}
#ifdef ALIGNMENT_CHECKS
		if(ofile->archive_cputype != 0 &&
		   offset % sizeof(uint32_t) != 0){
		    error("fat file: %s architecture %s malformed archive that "
			  "contains object files (offset to archive is not a "
			  "multiple of sizeof(uint32_t))",
			  ofile->file_name, arch_flag->name);
		    ofile_unmap(ofile);
#ifdef OFI
		    return(NSObjectFileImageInappropriateFile);
#else
		    return(FALSE);
#endif
		}
#endif /* ALIGNMENT_CHECKS */
		if(object_name != NULL){
		    if(ofile_specific_member(object_name, ofile) == FALSE)
			goto cleanup;
		}
	    }
	    /* this file type is now known to be unknown to this program */
	    else{
		ofile->file_type = OFILE_UNKNOWN;
		if(object_name != NULL){
		    error("fat file: %s architecture %s is not an archive "
			  "(object_name to ofile_map() can't be specified to "
			  "be other than NULL)", ofile->file_name,
			  arch_flag->name);
		    goto cleanup;
		}
	    }
	}
	/* see if this file is a 32-bit Mach-O file */
	else if(size >= sizeof(struct mach_header) &&
		(magic == MH_MAGIC ||
		 magic == SWAP_INT(MH_MAGIC))){
	    ofile->file_type = OFILE_Mach_O;
	    ofile->object_addr = addr;
	    ofile->object_size = (uint32_t)size;
	    if(magic == MH_MAGIC)
		ofile->object_byte_sex = host_byte_sex;
	    else
		ofile->object_byte_sex = host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				 LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
	    ofile->mh = (struct mach_header *)addr;
	    ofile->load_commands = (struct load_command *)(addr +
				    sizeof(struct mach_header));
	    if(check_Mach_O(ofile) == CHECK_BAD){
		ofile_unmap(ofile);
#ifdef OFI
		return(NSObjectFileImageFormat);
#else
		return(FALSE);
#endif
	    }
	    if(object_name != NULL){
		error("file: %s is not an archive (object_name to ofile_map() "
		      "can't be specified to be other than NULL)",
		      ofile->file_name);
		goto cleanup;
	    }
	    if(arch_flag != NULL){
		if(arch_flag->cputype != ofile->mh_cputype &&
		   (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		   (ofile->mh_cpusubtype & ~CPU_SUBTYPE_MASK)){
#ifdef OFI
		    ofile_unmap(ofile);
		    return(NSObjectFileImageArch);
#else
		    error("object file: %s does not match specified arch_flag: "
			  "%s passed to ofile_map()", ofile->file_name,
			  arch_flag->name);
		    goto cleanup;
#endif
		}
	    }
	}
	/* see if this file is a 64-bit Mach-O file */
	else if(size >= sizeof(struct mach_header_64) &&
		(magic == MH_MAGIC_64 ||
		 magic == SWAP_INT(MH_MAGIC_64))){
	    ofile->file_type = OFILE_Mach_O;
	    ofile->object_addr = addr;
	    ofile->object_size = (uint32_t)size;
	    if(magic == MH_MAGIC_64)
		ofile->object_byte_sex = host_byte_sex;
	    else
		ofile->object_byte_sex = host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				 LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
	    ofile->mh64 = (struct mach_header_64 *)addr;
	    ofile->load_commands = (struct load_command *)(addr +
				    sizeof(struct mach_header_64));
	    if(check_Mach_O(ofile) == CHECK_BAD){
		ofile_unmap(ofile);
#ifdef OFI
		return(NSObjectFileImageFormat);
#else
		return(FALSE);
#endif
	    }
	    if(object_name != NULL){
		error("file: %s is not an archive (object_name to ofile_map() "
		      "can't be specified to be other than NULL)",
		      ofile->file_name);
		goto cleanup;
	    }
	    if(arch_flag != NULL){
		if(arch_flag->cputype != ofile->mh_cputype &&
		   (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		   (ofile->mh_cpusubtype & ~CPU_SUBTYPE_MASK)){
#ifdef OFI
		    ofile_unmap(ofile);
		    return(NSObjectFileImageArch);
#else
		    error("object file: %s does not match specified arch_flag: "
			  "%s passed to ofile_map()", ofile->file_name,
			  arch_flag->name);
		    goto cleanup;
#endif
		}
	    }
	}
	/* see if this file is an archive file */
	else if(size >= SARMAG && strncmp(addr, ARMAG, SARMAG) == 0){
	    ofile->file_type = OFILE_ARCHIVE;
	    if(check_archive(ofile, archives_with_fat_objects) == CHECK_BAD)
		goto cleanup;
	    if(object_name != NULL){
		if(ofile_specific_member(object_name, ofile) == FALSE)
		    goto cleanup;
		if(arch_flag != NULL){
		    if(arch_flag->cputype != ofile->mh_cputype &&
		       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		       (ofile->mh_cpusubtype & ~CPU_SUBTYPE_MASK)){
			error("object file: %s(%.*s) does not match specified "
			    "arch_flag: %s passed to ofile_map()",
			    ofile->file_name, (int)ofile->member_name_size,
			    ofile->member_name, arch_flag->name);
			goto cleanup;
		    }
		}
	    }
	    else{
		if(arch_flag != NULL){
		    if(arch_flag->cputype != ofile->archive_cputype &&
		       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		       (ofile->archive_cpusubtype & ~CPU_SUBTYPE_MASK)){
			error("archive file: %s objects do not match specified "
			      "arch_flag: %s passed to ofile_map()",
			      ofile->file_name, arch_flag->name);
			goto cleanup;
		    }
		}
	    }
	}
	/* this file type is now known to be unknown to this program */
	else{
#ifndef OTOOL
unknown:
#endif
#ifndef OFI
#ifdef LTO_SUPPORT
	    if(is_llvm_bitcode(ofile, ofile->file_addr, ofile->file_size) ==
	       TRUE){
		ofile->file_type = OFILE_LLVM_BITCODE;
		if(arch_flag != NULL){
		    if(arch_flag->cputype != ofile->lto_cputype &&
		       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) !=
		       (ofile->lto_cpusubtype & ~CPU_SUBTYPE_MASK)){
			error("llvm bitcode file: %s does not match specified "
			      "arch_flag: %s passed to ofile_map()",
			      ofile->file_name, arch_flag->name);
			goto cleanup;
		    }
		}
		if(object_name != NULL){
		    error("file: %s is not an archive (object_name to "
			  "ofile_map() can't be specified to be other than "
			  "NULL)", ofile->file_name);
		    goto cleanup;
		}
		goto success;
	    }
	    else
#endif /* LTO_SUPPORT */
#endif /* OFI */
		ofile->file_type = OFILE_UNKNOWN;
	    if(arch_flag != NULL){
#ifdef OFI
		ofile_unmap(ofile);
		return(NSObjectFileImageInappropriateFile);
#else
		error("file: %s is unknown type (arch_flag to ofile_map() "
		      "can't be specified to be other than NULL)",
		      ofile->file_name);
		ofile_unmap(ofile);
		return(FALSE);
#endif
	    }
	    if(object_name != NULL){
		error("file: %s is not an archive (object_name to ofile_map() "
		      "can't be specified to be other than NULL)",
		      ofile->file_name);
		goto cleanup;
	    }
	}
success:
	return(TRUE);

cleanup:
	ofile_unmap(ofile);
	return(FALSE);
}

/*
 * ofile_unmap() deallocates the memory associated with the specified ofile
 * struct.
 */
__private_extern__
void
ofile_unmap(
struct ofile *ofile)
{
    kern_return_t r;

	if(ofile->file_addr != NULL){
	    if((r = vm_deallocate(mach_task_self(),
				 (vm_address_t)ofile->file_addr,
				 (vm_size_t)ofile->file_size)) != KERN_SUCCESS){
		my_mach_error(r, "Can't vm_deallocate mapped memory for file: "
			      "%s", ofile->file_name);
	    }
	}
	if(ofile->file_name != NULL)
	    free(ofile->file_name);
	if(ofile->arch_flag.name != NULL)
	    free(ofile->arch_flag.name);
	memset(ofile, '\0', sizeof(struct ofile));
}

/*
 * ofile_first_arch() sets up the ofile struct for a fat file to the first arch
 * in it.
 */
__private_extern__
enum bool
ofile_first_arch(
struct ofile *ofile)
{
	if(ofile->file_type == OFILE_FAT ||
	   (ofile->file_type == OFILE_ARCHIVE &&
	    ofile->member_type == OFILE_FAT) )
	    return(ofile_specific_arch(ofile, 0));
	else{
	    error("ofile_first_arch() called and file type of: %s is not a fat "
		  "file\n", ofile->file_name);
	    return(FALSE);
	}
}

/*
 * ofile_next_arch() sets up the ofile struct for a fat file to the next arch
 * in it.
 */
__private_extern__
enum bool
ofile_next_arch(
struct ofile *ofile)
{
	if(ofile->file_type == OFILE_FAT ||
	   (ofile->file_type == OFILE_ARCHIVE &&
	    ofile->member_type == OFILE_FAT) ){
	    if(ofile->narch + 1 < ofile->fat_header->nfat_arch)
		return(ofile_specific_arch(ofile, ofile->narch + 1));
	    else
		return(FALSE);
	}
	else{
	    error("ofile_next_arch() called and file type of: %s is not a fat "
		  "file\n", ofile->file_name);
	    return(FALSE);
	}
}

/*
 * ofile_specific_arch() sets up the ofile struct for the fat file for the
 * specified narch.
 */
static
enum bool
ofile_specific_arch(
struct ofile *ofile,
uint32_t narch)
{
    char *addr;
    uint64_t size, offset;
    uint32_t magic;
    enum byte_sex host_byte_sex;

	ofile->narch = narch;
	ofile->arch_type = OFILE_UNKNOWN;
	if(ofile->arch_flag.name != NULL)
	    free(ofile->arch_flag.name);
	ofile->arch_flag.name = NULL;
	ofile->arch_flag.cputype = 0;
	ofile->arch_flag.cpusubtype = 0;
	ofile->archive_cputype = 0;
	ofile->archive_cpusubtype = 0;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;

	/* silence clang warning */
	magic = 0;

	if(ofile->fat_header->magic == FAT_MAGIC_64){
	    ofile->arch_flag.cputype =
		ofile->fat_archs64[ofile->narch].cputype;
	    ofile->arch_flag.cpusubtype =
		ofile->fat_archs64[ofile->narch].cpusubtype;
	}
	else{
	    ofile->arch_flag.cputype =
		ofile->fat_archs[ofile->narch].cputype;
	    ofile->arch_flag.cpusubtype =
		ofile->fat_archs[ofile->narch].cpusubtype;
	}
	set_arch_flag_name(&(ofile->arch_flag));

	/* Now determine the file type for this specific architecture */
	if(ofile->file_type == OFILE_FAT){
	    ofile->member_offset = 0;
	    ofile->member_addr = NULL;
	    ofile->member_size = 0;
	    ofile->member_ar_hdr = NULL;
	    ofile->member_type = OFILE_UNKNOWN;

	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		size = ofile->fat_archs64[ofile->narch].size;
		addr = ofile->file_addr +
		       ofile->fat_archs64[ofile->narch].offset;
	    }
	    else{
		size = ofile->fat_archs[ofile->narch].size;
		addr = ofile->file_addr +
		       ofile->fat_archs[ofile->narch].offset;
	    }
	}
	else{
	    if(ofile->file_type != OFILE_ARCHIVE ||
	       ofile->member_type != OFILE_FAT){
		error("internal error. ofile_specific_arch() called but file "
		      "is not a fat file or an archive with a fat member ");
	    }
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		size = ofile->fat_archs64[ofile->narch].size;
		addr = ofile->file_addr +
		       ofile->member_offset +
		       ofile->fat_archs64[ofile->narch].offset;
	    }
	    else{
		size = ofile->fat_archs[ofile->narch].size;
		addr = ofile->file_addr +
		       ofile->member_offset +
		       ofile->fat_archs[ofile->narch].offset;
	    }
	}

#ifdef OTOOL
	if(addr - ofile->file_addr > (ptrdiff_t)ofile->file_size){
	    error("fat file: %s offset to architecture %s extends past end "
		  "of file", ofile->file_name, ofile->arch_flag.name);
	    return(FALSE);
	}
	if(addr + size > ofile->file_addr + ofile->file_size)
	    size = (ofile->file_addr + ofile->file_size) - addr;
#endif /* OTOOL */
	if(ofile->fat_header->magic == FAT_MAGIC_64)
	    offset = ofile->fat_archs64[ofile->narch].offset;
	else
	    offset = ofile->fat_archs[ofile->narch].offset;

	if(size >= sizeof(struct mach_header))
	    memcpy(&magic, addr, sizeof(uint32_t));
	/* see if this file is a 32-bit Mach-O file */
	if(size >= sizeof(struct mach_header) &&
	   (magic == MH_MAGIC || magic == SWAP_INT(MH_MAGIC))){
#ifdef ALIGNMENT_CHECKS
	    if(offset % 4 != 0){
		if(ofile->file_type == OFILE_ARCHIVE){
		    error("fat file: %s(%.*s) architecture %s malformed for a "
			  "32-bit object file (offset is not a multiple of 4)",
			  ofile->file_name, (int)ofile->member_name_size,
			  ofile->member_name, ofile->arch_flag.name);
		}
		else
		    error("fat file: %s architecture %s malformed for a 32-bit "
			  "object file (offset is not a multiple of 4)",
			  ofile->file_name, ofile->arch_flag.name);
		goto cleanup;
	    }
#endif /* ALIGNMENT_CHECKS */
	    ofile->arch_type = OFILE_Mach_O;
	    ofile->object_addr = addr;
	    ofile->object_size = (uint32_t)size;
	    host_byte_sex = get_host_byte_sex();
	    if(magic == MH_MAGIC)
		ofile->object_byte_sex = host_byte_sex;
	    else
		ofile->object_byte_sex =
		    host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
		    LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
	    ofile->mh = (struct mach_header *)addr;
	    ofile->load_commands = (struct load_command *)(addr +
				    sizeof(struct mach_header));
	    if(check_Mach_O(ofile) == CHECK_BAD)
		goto cleanup;
	}
	/* see if this file is a 64-bit Mach-O file */
	else if(size >= sizeof(struct mach_header_64) &&
	   (magic == MH_MAGIC_64 || magic == SWAP_INT(MH_MAGIC_64))){
#ifdef ALIGNMENT_CHECKS
	    if(offset % 8 != 0){
		if(ofile->file_type == OFILE_ARCHIVE){
		    error("fat file: %s(%.*s) architecture %s malformed for an "
			  "object file (offset is not a multiple of 8)",
			  ofile->file_name, (int)ofile->member_name_size,
			  ofile->member_name, ofile->arch_flag.name);
		}
		else
		    error("fat file: %s architecture %s malformed for a 64-bit "
			  "object file (offset is not a multiple of 8",
			  ofile->file_name, ofile->arch_flag.name);
		goto cleanup;
	    }
#endif /* ALIGNMENT_CHECKS */
	    ofile->arch_type = OFILE_Mach_O;
	    ofile->object_addr = addr;
	    ofile->object_size = (uint32_t)size;
	    host_byte_sex = get_host_byte_sex();
	    if(magic == MH_MAGIC_64)
		ofile->object_byte_sex = host_byte_sex;
	    else
		ofile->object_byte_sex =
		    host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
		    LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
	    ofile->mh64 = (struct mach_header_64 *)addr;
	    ofile->load_commands = (struct load_command *)(addr +
				    sizeof(struct mach_header_64));
	    if(check_Mach_O(ofile) == CHECK_BAD)
		goto cleanup;
	}
	/* see if this file is an archive file */
	else if(size >= SARMAG && strncmp(addr, ARMAG, SARMAG) == 0){
	    ofile->arch_type = OFILE_ARCHIVE;
	    if(check_archive(ofile, FALSE) == CHECK_BAD)
		goto cleanup;
#ifdef ALIGNMENT_CHECKS
	    if(ofile->archive_cputype != 0 && offset % sizeof(uint32_t) != 0){
		error("fat file: %s architecture %s malformed archive that "
		      "contains object files (offset to archive is not a "
		      "multiple of sizeof(uint32_t))",
		      ofile->file_name, ofile->arch_flag.name);
		goto cleanup;
	    }
#endif /* ALIGNMENT_CHECKS */
	}
	/*
	 * This type for this architecture is now known to be unknown to this
	 * program.
	 */
	else{
#ifdef LTO_SUPPORT
	    if(is_llvm_bitcode(ofile, addr, size) == TRUE){
		ofile->arch_type = OFILE_LLVM_BITCODE;
		ofile->object_addr = addr;
		ofile->object_size = (uint32_t)size;
	    }
	    else
#endif /* LTO_SUPPORT */
	        ofile->arch_type = OFILE_UNKNOWN;
	}
	return(TRUE);
cleanup:
	ofile->narch = 0;;
	ofile->arch_type = OFILE_UNKNOWN;
	if(ofile->arch_flag.name != NULL)
	    free(ofile->arch_flag.name);
	ofile->arch_flag.name = NULL;
	ofile->arch_flag.cputype = 0;
	ofile->arch_flag.cpusubtype = 0;
	if(ofile->file_type != OFILE_ARCHIVE){
	    ofile->member_offset = 0;
	    ofile->member_addr = NULL;
	    ofile->member_size = 0;
	    ofile->member_ar_hdr = NULL;
	    ofile->member_type = OFILE_UNKNOWN;
	}
	ofile->archive_cputype = 0;
	ofile->archive_cpusubtype = 0;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
	return(FALSE);
}

/*
 * ofile_first_member() set up the ofile structure (the member_* fields and
 * the object file fields if the first member is an object file) for the first
 * member.
 */
__private_extern__
enum bool
ofile_first_member(
struct ofile *ofile)
{
    char *addr;
    uint64_t size, offset;
    uint32_t magic;
    enum byte_sex host_byte_sex;
    struct ar_hdr *ar_hdr;
    uint32_t ar_name_size;
    uint32_t sizeof_fat_archs;

	/* These fields are to be filled in by this routine, clear them first */
	ofile->member_offset = 0;
	ofile->member_addr = NULL;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	/*
	 * Note: it is up to the caller if they want to call free_lto() on this
	 * when iterating through the members of an archive. 
	 */
	ofile->lto = NULL;
#endif /* LTO_SUPPORT */

	/*
	 * Get the address and size of the archive.
	 */
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_ARCHIVE){
		error("ofile_first_member() called on fat file: %s with a "
		      "non-archive architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		addr = ofile->file_addr +
		       ofile->fat_archs64[ofile->narch].offset;
		size = ofile->fat_archs64[ofile->narch].size;
	    }
	    else{
		addr = ofile->file_addr +
		       ofile->fat_archs[ofile->narch].offset;
		size = ofile->fat_archs[ofile->narch].size;
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    addr = ofile->file_addr;
	    size = ofile->file_size;
	}
	else{
	    error("ofile_first_member() called and file type of %s is "
		  "OFILE_UNKNOWN\n", ofile->file_name);
	    return(FALSE);
	}
#ifdef OTOOL
	if((addr + SARMAG) - ofile->file_addr > (ptrdiff_t)ofile->file_size){
	    archive_error(ofile, "offset to first member extends past the end "
			  "of the file");
	    return(FALSE);
	}
	if(addr + size > ofile->file_addr + ofile->file_size)
	    size = (ofile->file_addr + ofile->file_size) - addr;
#endif /* OTOOL */
	if(size < SARMAG || strncmp(addr, ARMAG, SARMAG) != 0){
	    archive_error(ofile, "internal error. ofile_first_member() "
			  "called but file does not have an archive magic "
			  "string");
	    return(FALSE);
	}

	offset = SARMAG;
	if(offset != size && offset + sizeof(struct ar_hdr) > size){
	    archive_error(ofile, "truncated or malformed (archive header of "
			  "first member extends past the end of the file)");
	    return(FALSE);
	}

	/* check for empty archive */
	if(size == offset)
	    return(FALSE);

	/* now we know there is a first member so set it up */
	ar_hdr = (struct ar_hdr *)(addr + offset);
	offset += sizeof(struct ar_hdr);
	ofile->member_offset = offset;
	ofile->member_addr = addr + offset;
	ofile->member_size = (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10);
	if(ofile->member_size > size - sizeof(struct ar_hdr)){
	    archive_error(ofile, "size of first archive member extends past "
		          "the end of the archive");
	    ofile->member_size = (uint32_t)(size - sizeof(struct ar_hdr));
	}
	ofile->member_ar_hdr = ar_hdr;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->member_name = ar_hdr->ar_name;
	if(strncmp(ofile->member_name, AR_EFMT1, sizeof(AR_EFMT1) - 1) == 0){
	    ofile->member_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
	    ar_name_size = (uint32_t)
		strtoul(ar_hdr->ar_name + sizeof(AR_EFMT1) - 1, NULL, 10);
	    if(ar_name_size > ofile->member_size){
		archive_error(ofile, "size of first archive member name "
			      "extends past the end of the archive");
		ar_name_size = ofile->member_size;
	    }
	    ofile->member_name_size = ar_name_size;
	    ofile->member_offset += ar_name_size;
	    ofile->member_addr += ar_name_size;
	    ofile->member_size -= ar_name_size;
	}
	else{
	    ofile->member_name_size = size_ar_name(ar_hdr);
	    ar_name_size = 0;
	}
	/* Clear these in case there is no table of contents */
	ofile->toc_addr = NULL;
	ofile->toc_size = 0;
	ofile->toc_ar_hdr = NULL;
	ofile->toc_name = NULL;
	ofile->toc_name_size = 0;
	ofile->toc_ranlibs = NULL;
	ofile->toc_ranlibs64 = NULL;
	ofile->toc_nranlibs = 0;
	ofile->toc_strings = NULL;
	ofile->toc_strsize = 0;
	ofile->toc_bad = FALSE;

	/* Clear these until we find a System V "//" named archive member */
	ofile->sysv_ar_strtab = NULL;
	ofile->sysv_ar_strtab_size = 0;

	host_byte_sex = get_host_byte_sex();

	if(ofile->member_size > sizeof(uint32_t)){
	    memcpy(&magic, ofile->member_addr, sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
	    if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	    if(magic == SWAP_INT(FAT_MAGIC) || magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
	    {
		ofile->member_type = OFILE_FAT;
		ofile->fat_header =
			(struct fat_header *)(ofile->member_addr);
#ifdef __LITTLE_ENDIAN__
		swap_fat_header(ofile->fat_header, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		if(ofile->fat_header->magic == FAT_MAGIC_64)
		    sizeof_fat_archs = ofile->fat_header->nfat_arch *
				       sizeof(struct fat_arch_64);
		else
		    sizeof_fat_archs = ofile->fat_header->nfat_arch *
				       sizeof(struct fat_arch);
		if(sizeof(struct fat_header) + sizeof_fat_archs >
		   ofile->member_size){
		    archive_member_error(ofile, "fat file truncated or "
			    "malformed (fat_arch%s structs would extend past "
			    "the end of the archive member)",
			    ofile->fat_header->magic == FAT_MAGIC_64 ?
			    "_64" : "");
		    goto fatcleanup;
		}
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    ofile->fat_archs64 = (struct fat_arch_64 *)
			(ofile->member_addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch_64(ofile->fat_archs64,
				     ofile->fat_header->nfat_arch,
				     host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		else{
		    ofile->fat_archs = (struct fat_arch *)
			(ofile->member_addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch(ofile->fat_archs,
				  ofile->fat_header->nfat_arch, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		if(check_fat_object_in_archive(ofile) == FALSE)
		    goto fatcleanup;
	    }
	    else if(size - (offset + ar_name_size) >=
		    sizeof(struct mach_header) &&
	       (magic == MH_MAGIC || magic == SWAP_INT(MH_MAGIC))){
#ifdef ALIGNMENT_CHECKS
		if((offset + ar_name_size) % 4 != 0){
		    archive_member_error(ofile, "offset in archive not a "
			"multiple of 4 (must be since member is a 32-bit "
			"object file)");
		    goto cleanup;
		}
#endif /* ALIGNMENT_CHECKS */
		ofile->member_type = OFILE_Mach_O;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
		if(magic == MH_MAGIC)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh = (struct mach_header *)(ofile->object_addr);
		ofile->load_commands = (struct load_command *)
		    (ofile->object_addr + sizeof(struct mach_header));
		if(check_Mach_O(ofile) == CHECK_BAD)
		    goto cleanup;
	    }
	    else if(size - (offset + ar_name_size) >=
		    sizeof(struct mach_header_64) &&
	       (magic == MH_MAGIC_64 || magic == SWAP_INT(MH_MAGIC_64))){
#ifdef ALIGNMENT_CHECKS_ARCHIVE_64_BIT
		if(archive_64_bit_align_warning == FALSE &&
		   (offset + ar_name_size) % 8 != 0){
		    temporary_archive_member_warning(ofile, "offset in archive "
			"not a multiple of 8 (must be since member is an "
			"64-bit object file)");
		    archive_64_bit_align_warning = TRUE;
		    /* goto cleanup; */
		}
#endif /* ALIGNMENT_CHECKS_ARCHIVE_64_BIT */
		ofile->member_type = OFILE_Mach_O;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
		if(magic == MH_MAGIC_64)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh64 = (struct mach_header_64 *)(ofile->object_addr);
		ofile->load_commands = (struct load_command *)
		    (ofile->object_addr + sizeof(struct mach_header_64));
		if(check_Mach_O(ofile) == CHECK_BAD)
		    goto cleanup;
	    }
	    if(ofile->member_type == OFILE_UNKNOWN &&
	       (strncmp(ofile->member_name, SYMDEF_SORTED,
		        sizeof(SYMDEF_SORTED) - 1) == 0 ||
	        strncmp(ofile->member_name, SYMDEF,
		        sizeof(SYMDEF) - 1) == 0 ||
	        strncmp(ofile->member_name, SYMDEF_64_SORTED,
		        sizeof(SYMDEF_64_SORTED) - 1) == 0 ||
	        strncmp(ofile->member_name, SYMDEF_64,
		        sizeof(SYMDEF_64) - 1) == 0)){
		ofile->toc_addr = ofile->member_addr;
		ofile->toc_size = ofile->member_size;
		ofile->toc_ar_hdr = ofile->member_ar_hdr;
		ofile->toc_name = ofile->member_name;
		ofile->toc_name_size = ofile->member_name_size;
		if(check_archive_toc(ofile) == CHECK_BAD)
		    goto cleanup;
	    }
	    else if(ofile->member_type == OFILE_UNKNOWN &&
	            strcmp(ofile->member_name, "// ") == 0){
		ofile->sysv_ar_strtab = ofile->member_addr;
		ofile->sysv_ar_strtab_size = ofile->member_size;
	    }
#ifdef LTO_SUPPORT
	    if(ofile->member_type == OFILE_UNKNOWN &&
	       strncmp(ofile->member_name, SYMDEF_SORTED,
		       sizeof(SYMDEF_SORTED) - 1) != 0 &&
	       strncmp(ofile->member_name, SYMDEF,
		       sizeof(SYMDEF) - 1) != 0 &&
	       is_llvm_bitcode(ofile, ofile->member_addr, ofile->member_size) ==
	       TRUE){
		ofile->member_type = OFILE_LLVM_BITCODE;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
	    }
#endif /* LTO_SUPPORT */
	}
	return(TRUE);

fatcleanup:
	ofile->fat_header = NULL;
	ofile->fat_archs = NULL;
	ofile->fat_archs64 = NULL;
cleanup:
	ofile->member_offset = 0;
	ofile->member_addr = 0;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	ofile->lto = NULL;
	ofile->lto_cputype = 0;
	ofile->lto_cpusubtype = 0;
#endif /* LTO_SUPPORT */
	return(FALSE);
}

/*
 * ofile_next_member() set up the ofile structure (the member_* fields and
 * the object file fields if the next member is an object file) for the next
 * member.
 */
__private_extern__
enum bool
ofile_next_member(
struct ofile *ofile)
{
    char *addr;
    uint64_t size, offset;
    uint32_t magic;
    enum byte_sex host_byte_sex;
    struct ar_hdr *ar_hdr;
    uint32_t ar_name_size, member_name_offset, n;
    uint32_t sizeof_fat_archs;

	/* silence clang warning */
	ar_name_size = 0;

	/*
	 * Get the address and size of the archive.
	 */
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_ARCHIVE){
		error("ofile_next_member() called on fat file: %s with a "
		      "non-archive architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		addr = ofile->file_addr +
		       ofile->fat_archs64[ofile->narch].offset;
		size = ofile->fat_archs64[ofile->narch].size;
	    }
	    else{
		addr = ofile->file_addr +
		       ofile->fat_archs[ofile->narch].offset;
		size = ofile->fat_archs[ofile->narch].size;
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    addr = ofile->file_addr;
	    size = ofile->file_size;
	}
	else{
	    error("ofile_next_member() called and file type of %s is "
		  "OFILE_UNKNOWN\n", ofile->file_name);
	    return(FALSE);
	}
	if(size < SARMAG || strncmp(addr, ARMAG, SARMAG) != 0){
	    archive_error(ofile, "internal error. ofile_next_member() "
			  "called but file does not have an archive magic "
			  "string");
	    return(FALSE);
	}
	if(ofile->member_ar_hdr == NULL){
	    archive_error(ofile, "internal error. ofile_next_member() called "
			  "but the ofile struct does not have an archive "
			  "member selected");
	    return(FALSE);
	}

	/* figure out the offset to the next member */
	offset = ofile->member_offset + rnd(ofile->member_size,sizeof(short));
#ifdef OTOOL
	if((addr - ofile->file_addr) + offset > ofile->file_size){
	    archive_error(ofile, "offset to next member extends past the end "
			  "of the file");
	    return(FALSE);
	}
#endif /* OTOOL */
	/* if now at the end of the file then no more members */
	if(offset == size)
	     goto cleanup;
	if(offset > size){
	    archive_error(ofile, "truncated or malformed (archive header of "
			  "next member extends past the end of the file)");
	    return(FALSE);
	}

	/* now we know there is a next member so set it up */
	ar_hdr = (struct ar_hdr *)(addr + offset);
	offset += sizeof(struct ar_hdr);
	ofile->member_offset = offset;
	ofile->member_addr = addr + offset;
	ofile->member_size = (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10);
	if(ofile->member_size > size - sizeof(struct ar_hdr)){
	    archive_error(ofile, "size of archive member extends past "
		          "the end of the archive");
	    ofile->member_size = (uint32_t)(size - sizeof(struct ar_hdr));
	}
	ofile->member_ar_hdr = ar_hdr;
	ofile->member_name = ar_hdr->ar_name;
	if(ofile->sysv_ar_strtab == NULL && ofile->sysv_ar_strtab_size == 0 &&
	   strncmp(ofile->member_name, "// ", sizeof("// ") - 1) == 0){
	    ofile->sysv_ar_strtab = ofile->member_addr;
	    ofile->sysv_ar_strtab_size = ofile->member_size;
	}
	if(ofile->member_name[0] == '/' &&
		(ofile->member_name[1] != ' ' && ofile->member_name[1] != '/')){
	    member_name_offset =
		(uint32_t)strtoul(ar_hdr->ar_name + 1, NULL, 10);
	    if(member_name_offset < ofile->sysv_ar_strtab_size){
		ofile->member_name = ofile->sysv_ar_strtab + member_name_offset;
		ofile->member_name_size = 0;
		for(n = member_name_offset;
		    n < ofile->sysv_ar_strtab_size; n++){
		    if(ofile->sysv_ar_strtab[n] == '/')
			break;
		    ofile->member_name_size++;
		}
	    }
	    else
		ofile->member_name_size = size_ar_name(ar_hdr);
	}
	else if(strncmp(ofile->member_name, AR_EFMT1,
			sizeof(AR_EFMT1) - 1) == 0){
	    ofile->member_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
	    ar_name_size = (uint32_t)
		strtoul(ar_hdr->ar_name + sizeof(AR_EFMT1) - 1, NULL, 10);
	    if(ar_name_size > ofile->member_size){
		archive_error(ofile, "size of archive member name "
			      "extends past the end of the archive");
		ar_name_size = ofile->member_size;
	    }
	    ofile->member_name_size = ar_name_size;
	    ofile->member_offset += ar_name_size;
	    ofile->member_addr += ar_name_size;
	    ofile->member_size -= ar_name_size;
	}
	else{
	    ofile->member_name_size = size_ar_name(ar_hdr);
	    ar_name_size = 0;
	}
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	ofile->lto = NULL;
	ofile->lto_cputype = 0;
	ofile->lto_cpusubtype = 0;
#endif /* LTO_SUPPORT */

	host_byte_sex = get_host_byte_sex();

	if(ofile->member_size > sizeof(uint32_t)){
	    memcpy(&magic, ofile->member_addr, sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
	    if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	    if(magic == SWAP_INT(FAT_MAGIC) || magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
	    {
		ofile->member_type = OFILE_FAT;
		ofile->fat_header = (struct fat_header *)(ofile->member_addr);
#ifdef __LITTLE_ENDIAN__
		swap_fat_header(ofile->fat_header, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		if(ofile->fat_header->magic == FAT_MAGIC_64)
		    sizeof_fat_archs = ofile->fat_header->nfat_arch *
				       sizeof(struct fat_arch_64);
		else
		    sizeof_fat_archs = ofile->fat_header->nfat_arch *
				       sizeof(struct fat_arch);
		if(sizeof(struct fat_header) + sizeof_fat_archs >
		   ofile->member_size){
		    archive_member_error(ofile, "fat file truncated or "
			    "malformed (fat_arch%s structs would extend past "
			    "the end of the archive member)",
			    ofile->fat_header->magic == FAT_MAGIC_64 ?
			    "_64" : "");
		    goto cleanup;
		}
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    ofile->fat_archs64 = (struct fat_arch_64 *)
			(ofile->member_addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch_64(ofile->fat_archs64,
				     ofile->fat_header->nfat_arch,
				     host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		else{
		    ofile->fat_archs = (struct fat_arch *)
			(ofile->member_addr + sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
		    swap_fat_arch(ofile->fat_archs,
				  ofile->fat_header->nfat_arch, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
		}
		if(check_fat_object_in_archive(ofile) == FALSE)
		    goto cleanup;
	    }
	    else if(size - (offset + ar_name_size) >=
		    sizeof(struct mach_header) &&
		    (magic == MH_MAGIC ||
		     magic == SWAP_INT(MH_MAGIC))){
#ifdef ALIGNMENT_CHECKS
		if((offset + ar_name_size) % 4 != 0){
		    archive_member_error(ofile, "offset in archive not "
			"a multiple of 4 (must be since member is an 32-bit "
			"object file)");
		    goto cleanup;
		}
#endif /* ALIGNMENT_CHECKS */
		ofile->member_type = OFILE_Mach_O;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
		if(magic == MH_MAGIC)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh = (struct mach_header *)ofile->object_addr;
		ofile->load_commands = (struct load_command *)
			   (ofile->object_addr + sizeof(struct mach_header));
		if(check_Mach_O(ofile) == CHECK_BAD)
		    goto cleanup;
	    }
	    else if(size - (offset + ar_name_size) >=
		    sizeof(struct mach_header_64) &&
		    (magic == MH_MAGIC_64 ||
		     magic == SWAP_INT(MH_MAGIC_64))){
#ifdef ALIGNMENT_CHECKS_ARCHIVE_64_BIT
		if(archive_64_bit_align_warning == FALSE &&
		   (offset + ar_name_size) % 8 != 0){
		    temporary_archive_member_warning(ofile, "offset in archive "
			"not a multiple of 8 (must be since member is an "
			"64-bit object file)");
		    archive_64_bit_align_warning = TRUE;
		    /* goto cleanup; */
		}
#endif /* ALIGNMENT_CHECKS_ARCHIVE_64_BIT */
		ofile->member_type = OFILE_Mach_O;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
		if(magic == MH_MAGIC_64)
		    ofile->object_byte_sex = host_byte_sex;
		else
		    ofile->object_byte_sex =
			   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
			   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		ofile->mh64 = (struct mach_header_64 *)ofile->object_addr;
		ofile->load_commands = (struct load_command *)
			   (ofile->object_addr + sizeof(struct mach_header_64));
		if(check_Mach_O(ofile) == CHECK_BAD)
		    goto cleanup;
	    }
#ifdef LTO_SUPPORT
	    if(ofile->member_type == OFILE_UNKNOWN &&
	       is_llvm_bitcode(ofile, ofile->member_addr, ofile->member_size) ==
	       TRUE){
		ofile->member_type = OFILE_LLVM_BITCODE;
		ofile->object_addr = ofile->member_addr;
		ofile->object_size = ofile->member_size;
	    }
#endif /* LTO_SUPPORT */
	}
	return(TRUE);

cleanup:
	if(ofile->member_type == OFILE_FAT){
	    ofile->fat_header = NULL;
	    ofile->fat_archs = NULL;
	    ofile->fat_archs64 = NULL;
	}
	ofile->member_offset = 0;
	ofile->member_addr = NULL;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	ofile->lto = NULL;
	ofile->lto_cputype = 0;
	ofile->lto_cpusubtype = 0;
#endif /* LTO_SUPPORT */
	return(FALSE);
}

/*
 * ofile_specific_member() set up the ofile structure (the member_* fields and
 * the object file fields if the member is an object file) for the specified
 * member member_name.
 */
__private_extern__
enum bool
ofile_specific_member(
const char *member_name,
struct ofile *ofile)
{
    int32_t i, n;
    char *addr;
    uint64_t size, offset;
    uint32_t magic;
    enum byte_sex host_byte_sex;
    char *ar_name;
    uint32_t ar_name_size, member_name_offset;
    struct ar_hdr *ar_hdr;
    uint32_t sizeof_fat_archs;

	/* These fields are to be filled in by this routine, clear them first */
	ofile->member_offset = 0;
	ofile->member_addr = NULL;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	ofile->lto = NULL;
	ofile->lto_cputype = 0;
	ofile->lto_cpusubtype = 0;
#endif /* LTO_SUPPORT */

	/*
	 * Get the address and size of the archive.
	 */
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_ARCHIVE){
		error("ofile_specific_member() called on fat file: %s with a "
		      "non-archive architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		addr = ofile->file_addr +
		       ofile->fat_archs64[ofile->narch].offset;
		size = ofile->fat_archs64[ofile->narch].size;
	    }
	    else{
		addr = ofile->file_addr +
		       ofile->fat_archs[ofile->narch].offset;
		size = ofile->fat_archs[ofile->narch].size;
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    addr = ofile->file_addr;
	    size = ofile->file_size;
	}
	else{
	    error("ofile_specific_member() called and file type of %s is "
		  "OFILE_UNKNOWN\n", ofile->file_name);
	    return(FALSE);
	}
	if(size < SARMAG || strncmp(addr, ARMAG, SARMAG) != 0){
	    archive_error(ofile, "internal error. ofile_specific_member() "
			  "called but file does not have an archive magic "
			  "string");
	    return(FALSE);
	}

	/* Clear these until we find a System V "//" named archive member */
	ofile->sysv_ar_strtab = NULL;
	ofile->sysv_ar_strtab_size = 0;

	offset = SARMAG;
	if(offset != size && offset + sizeof(struct ar_hdr) > size){
	    archive_error(ofile, "truncated or malformed (archive header of "
			  "first member extends past the end of the file)");
	    return(FALSE);
	}
	while(size > offset){
	    ar_hdr = (struct ar_hdr *)(addr + offset);
	    offset += sizeof(struct ar_hdr);

	    if(ofile->sysv_ar_strtab == NULL &&
	       ofile->sysv_ar_strtab_size == 0 &&
	       strncmp(ar_hdr->ar_name, "// ", sizeof("// ") - 1) == 0){
		ofile->sysv_ar_strtab = addr + offset;
		ofile->sysv_ar_strtab_size =
		    (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10);
	    }

	    if(ar_hdr->ar_name[0] == '/' &&
	       (ar_hdr->ar_name[1] != ' ' && ar_hdr->ar_name[1] != '/')){
		member_name_offset =
		    (uint32_t)strtoul(ar_hdr->ar_name + 1, NULL, 10);
		if(member_name_offset < ofile->sysv_ar_strtab_size){
		    ar_name = ofile->sysv_ar_strtab + member_name_offset;
		    i = 0;
		    for(n = member_name_offset;
			n < ofile->sysv_ar_strtab_size; n++){
			if(ofile->sysv_ar_strtab[n] == '/')
			    break;
			i++;
		    }
		    ar_name_size = 0;
		}
		else{
		    i = size_ar_name(ar_hdr);
		    ar_name = ar_hdr->ar_name;
		    ar_name_size = 0;
		}
	    }
	    else if(strncmp(ar_hdr->ar_name, AR_EFMT1,
			    sizeof(AR_EFMT1) - 1) == 0){
#ifdef OTOOL
		if(check_extend_format_1(ofile, ar_hdr, (uint32_t)(size-offset),
				&ar_name_size) == CHECK_BAD){
		    i = size_ar_name(ar_hdr);
		    ar_name = ar_hdr->ar_name;
		    ar_name_size = 0;
		}
		else
#endif /* OTOOL */
		{
		    i = (uint32_t)
			strtoul(ar_hdr->ar_name + sizeof(AR_EFMT1) - 1,NULL,10);
		    ar_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
		    ar_name_size = i;
		}
	    }
	    else{
		i = size_ar_name(ar_hdr);
		ar_name = ar_hdr->ar_name;
		ar_name_size = 0;
	    }
	    if(i > 0 && strncmp(ar_name, member_name, i) == 0){

		ofile->member_name = ar_name;
		ofile->member_name_size = i;
		ofile->member_offset = offset + ar_name_size;
		ofile->member_addr = addr + offset + ar_name_size;
		ofile->member_size =
		    (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10) - ar_name_size;
		ofile->member_ar_hdr = ar_hdr;
		ofile->member_type = OFILE_UNKNOWN;

		host_byte_sex = get_host_byte_sex();

		if(ofile->member_size > sizeof(uint32_t)){
		    memcpy(&magic, addr + offset + ar_name_size,
			   sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
		    if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
		    if(magic == SWAP_INT(FAT_MAGIC) ||
		       magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
		    {
			ofile->member_type = OFILE_FAT;
			ofile->fat_header =
			    (struct fat_header *)(addr + offset + ar_name_size);
#ifdef __LITTLE_ENDIAN__
			swap_fat_header(ofile->fat_header, host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
			if(ofile->fat_header->magic == FAT_MAGIC_64)
			    sizeof_fat_archs = ofile->fat_header->nfat_arch *
					       sizeof(struct fat_arch_64);
			else
			    sizeof_fat_archs = ofile->fat_header->nfat_arch *
					       sizeof(struct fat_arch);
			if(sizeof(struct fat_header) + sizeof_fat_archs >
			   ofile->member_size){
			    archive_member_error(ofile, "fat file truncated or "
				    "malformed (fat_arch%s structs would extend"
				    " past the end of the archive member)",
				    ofile->fat_header->magic == FAT_MAGIC_64 ?
				    "_64" : "");
			    goto fatcleanup;
			}
			if(ofile->fat_header->magic == FAT_MAGIC_64){
			    ofile->fat_archs64 =
				(struct fat_arch_64 *)
				(addr + offset + ar_name_size +
					        sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
			    swap_fat_arch_64(ofile->fat_archs64,
					     ofile->fat_header->nfat_arch,
				             host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
			}
			else{
			    ofile->fat_archs =
				(struct fat_arch *)
				(addr + offset + ar_name_size +
					        sizeof(struct fat_header));
#ifdef __LITTLE_ENDIAN__
			    swap_fat_arch(ofile->fat_archs,
					  ofile->fat_header->nfat_arch,
				          host_byte_sex);
#endif /* __LITTLE_ENDIAN__ */
			}
			if(check_fat_object_in_archive(ofile) == FALSE)
			    goto fatcleanup;
		    }
		    else if(size - (offset + ar_name_size) >=
			    sizeof(struct mach_header) &&
			   (magic == MH_MAGIC ||
			    magic == SWAP_INT(MH_MAGIC))){
#ifdef ALIGNMENT_CHECKS
			if((offset + ar_name_size) % 4 != 0){
			    archive_member_error(ofile, "offset in archive not "
				"a multiple of 4) (must be since member is a "
				"32-bit object file)");
			    goto cleanup;
			}
#endif /* ALIGNMENT_CHECKS */
			ofile->member_type = OFILE_Mach_O;
			ofile->object_addr = ofile->member_addr;
			ofile->object_size = ofile->member_size;
			if(magic == MH_MAGIC)
			    ofile->object_byte_sex = host_byte_sex;
			else
			    ofile->object_byte_sex =
				   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
			ofile->mh = (struct mach_header *)ofile->object_addr;
			ofile->load_commands = (struct load_command *)
			    (ofile->object_addr + sizeof(struct mach_header));
			if(check_Mach_O(ofile) == CHECK_BAD)
			    goto cleanup;
		    }
		    else if(size - (offset + ar_name_size) >=
			    sizeof(struct mach_header_64) &&
			   (magic == MH_MAGIC_64 ||
			    magic == SWAP_INT(MH_MAGIC_64))){
#ifdef ALIGNMENT_CHECKS_ARCHIVE_64_BIT
			if(archive_64_bit_align_warning == FALSE &&
			   (offset + ar_name_size) % 8 != 0){
			    temporary_archive_member_warning(ofile, "offset in "
				"archive not a multiple of 8) (must be since "
				"member is a 64-bit object file)");
			    archive_64_bit_align_warning = TRUE;
			    /* goto cleanup; */
			}
#endif /* ALIGNMENT_CHECKS_ARCHIVE_64_BIT */
			ofile->member_type = OFILE_Mach_O;
			ofile->object_addr = ofile->member_addr;
			ofile->object_size = ofile->member_size;
			if(magic == MH_MAGIC_64)
			    ofile->object_byte_sex = host_byte_sex;
			else
			    ofile->object_byte_sex =
				   host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				   LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
			ofile->mh64 = (struct mach_header_64 *)
				      ofile->object_addr;
			ofile->load_commands = (struct load_command *)
			    (ofile->object_addr +sizeof(struct mach_header_64));
			if(check_Mach_O(ofile) == CHECK_BAD)
			    goto cleanup;
		    }
		}
#ifdef LTO_SUPPORT
		if(ofile->member_type == OFILE_UNKNOWN &&
		   is_llvm_bitcode(ofile, ofile->member_addr,
				   ofile->member_size) == TRUE){
		    ofile->member_type = OFILE_LLVM_BITCODE;
		    ofile->object_addr = ofile->member_addr;
		    ofile->object_size = ofile->member_size;
		}
#endif /* LTO_SUPPORT */
		return(TRUE);
	    }
	    offset += rnd(strtoul(ar_hdr->ar_size, NULL, 10),
			    sizeof(short));
	}
	archive_error(ofile, "does not contain a member named: %s",
		      member_name);
fatcleanup:
	ofile->fat_header = NULL;
	ofile->fat_archs = NULL;
	ofile->fat_archs64 = NULL;
cleanup:
	ofile->member_offset = 0;
	ofile->member_addr = NULL;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	ofile->member_type = OFILE_UNKNOWN;
	ofile->object_addr = NULL;
	ofile->object_size = 0;
	ofile->object_byte_sex = UNKNOWN_BYTE_SEX;
	ofile->mh = NULL;
	ofile->mh64 = NULL;
	ofile->load_commands = NULL;
#ifdef LTO_SUPPORT
	ofile->lto = NULL;
	ofile->lto_cputype = 0;
	ofile->lto_cpusubtype = 0;
#endif /* LTO_SUPPORT */
	return(FALSE);
}

/*
 * ofile_first_module() set up the ofile structure (the dylib_module field)
 * for the first module of an MH_DYLIB or MH_DYLIB_STUB file.
 */
__private_extern__
enum bool
ofile_first_module(
struct ofile *ofile)
{
    uint32_t i, ncmds;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct load_command *lc;
    enum bool swapped;
    enum byte_sex host_byte_sex;
    struct dylib_module m;
    struct dylib_module_64 m64;
    char *strings;

	/* These fields are to be filled in by this routine, clear them first */
	ofile->modtab = NULL;
	ofile->modtab64 = NULL;
	ofile->nmodtab = 0;
	ofile->dylib_module = NULL;
	ofile->dylib_module64 = NULL;
	ofile->dylib_module_name = NULL;

	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_Mach_O &&
	       (ofile->mh_filetype != MH_DYLIB &&
	        ofile->mh_filetype != MH_DYLIB_STUB)){
		error("ofile_first_module() called on fat file: %s with a "
		      "non-MH_DYLIB architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	}
	else if(ofile->arch_type != OFILE_Mach_O &&
	        (ofile->mh_filetype != MH_DYLIB &&
	         ofile->mh_filetype != MH_DYLIB_STUB)){
	    error("ofile_first_module() called and file type of %s is "
		  "non-MH_DYLIB\n", ofile->file_name);
	    return(FALSE);
	}

	st = NULL;
	dyst = NULL;
	lc = ofile->load_commands;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st == NULL || dyst == NULL){
#ifndef OTOOL
	    Mach_O_error(ofile, "MH_DYLIB format error (does not have a symbol "
		"table and/or a dynamic symbol table)");
#endif
	    return(FALSE);
	}
	if(dyst->nmodtab == 0)
	    return(FALSE);

	ofile->nmodtab = dyst->nmodtab;
	host_byte_sex = get_host_byte_sex();
	swapped = (enum bool)(host_byte_sex != ofile->object_byte_sex);
	strings = (char *)(ofile->object_addr + st->stroff);

	if(ofile->mh != NULL){
	    ofile->modtab = (struct dylib_module *)(ofile->object_addr +
						    dyst->modtaboff);
	    ofile->dylib_module = ofile->modtab;
	    m = *ofile->dylib_module;
	    if(swapped)
		swap_dylib_module(&m, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m.module_name;
	}
	else{
	    ofile->modtab64 = (struct dylib_module_64 *)(ofile->object_addr +
						         dyst->modtaboff);
	    ofile->dylib_module64 = ofile->modtab64;
	    m64 = *ofile->dylib_module64;
	    if(swapped)
		swap_dylib_module_64(&m64, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m64.module_name;
	}

	if(check_dylib_module(ofile, st, dyst, strings, 0) == CHECK_BAD)
	    return(FALSE);
	return(TRUE);
}

/*
 * ofile_next_module() set up the ofile structure (the dylib_module field)
 * for the next module of an MH_DYLIB or MH_DYLIB_STUB file.
 */
__private_extern__
enum bool
ofile_next_module(
struct ofile *ofile)
{
    uint32_t i, module_index, ncmds;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct load_command *lc;
    enum bool swapped;
    enum byte_sex host_byte_sex;
    struct dylib_module m;
    struct dylib_module_64 m64;
    char *strings;

	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_Mach_O &&
	       (ofile->mh_filetype != MH_DYLIB &&
	        ofile->mh_filetype != MH_DYLIB_STUB)){
		error("ofile_next_module() called on fat file: %s with a "
		      "non-MH_DYLIB architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	}
	else if(ofile->arch_type != OFILE_Mach_O &&
	        (ofile->mh_filetype != MH_DYLIB &&
	         ofile->mh_filetype != MH_DYLIB_STUB)){
	    error("ofile_next_module() called and file type of %s is "
		  "non-MH_DYLIB\n", ofile->file_name);
	    return(FALSE);
	}
	st = NULL;
	dyst = NULL;
	lc = ofile->load_commands;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st == NULL || dyst == NULL){
#ifndef OTOOL
	    Mach_O_error(ofile, "MH_DYLIB format error (does not have a symbol "
		"table and/or a dynamic symbol table)");
#endif
	    return(FALSE);
	}

	if(ofile->mh != NULL)
	    module_index =
		(uint32_t)((ofile->dylib_module + 1) - ofile->modtab);
	else
	    module_index =
		(uint32_t)((ofile->dylib_module64 + 1) - ofile->modtab64);
	if(module_index >= ofile->nmodtab)
	    return(FALSE);

	host_byte_sex = get_host_byte_sex();
	swapped = (enum bool)(host_byte_sex != ofile->object_byte_sex);
	strings = (char *)(ofile->object_addr + st->stroff);

	if(ofile->mh != NULL){
	    ofile->dylib_module++;
	    m = *ofile->dylib_module;
	    if(swapped)
		swap_dylib_module(&m, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m.module_name;
	}
	else{
	    ofile->dylib_module64++;
	    m64 = *ofile->dylib_module64;
	    if(swapped)
		swap_dylib_module_64(&m64, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m64.module_name;
	}
	if(check_dylib_module(ofile, st, dyst, strings, module_index) ==
	   CHECK_BAD)
	    return(FALSE);
	return(TRUE);
}

/*
 * ofile_specific_module() set up the ofile structure (the dylib_module fields)
 * for the specified module, module_name, of an MH_DYLIB or an MH_DYLIB_STUB
 * file.
 */
__private_extern__
enum bool
ofile_specific_module(
const char *module_name,
struct ofile *ofile)
{
    uint32_t i, ncmds;
    enum bool swapped;
    enum byte_sex host_byte_sex;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct load_command *lc;
    struct dylib_module *p, m;
    struct dylib_module_64 *p64, m64;
    char *strings;

	/* These fields are to be filled in by this routine, clear them first */
	ofile->modtab = NULL;
	ofile->modtab64 = NULL;
	ofile->nmodtab = 0;
	ofile->dylib_module = NULL;
	ofile->dylib_module64 = NULL;
	ofile->dylib_module_name = NULL;

	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_Mach_O &&
	       (ofile->mh_filetype != MH_DYLIB &&
	        ofile->mh_filetype != MH_DYLIB_STUB)){
		error("ofile_specific_module() called on fat file: %s with a "
		      "non-MH_DYLIB architecture or no architecture selected\n",
		      ofile->file_name);
		return(FALSE);
	    }
	}
	else if(ofile->arch_type != OFILE_Mach_O &&
	        (ofile->mh_filetype != MH_DYLIB &&
	         ofile->mh_filetype != MH_DYLIB_STUB)){
	    error("ofile_specific_module() called and file type of %s is "
		  "non-MH_DYLIB\n", ofile->file_name);
	    return(FALSE);
	}

	st = NULL;
	dyst = NULL;
	lc = ofile->load_commands;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st == NULL || dyst == NULL){
#ifndef OTOOL
	    Mach_O_error(ofile, "MH_DYLIB format error (does not have a symbol "
		"table and/or a dynamic symbol table)");
#endif
	    return(FALSE);
	}
	if(dyst->nmodtab == 0)
	    return(FALSE);

	host_byte_sex = get_host_byte_sex();
	swapped = (enum bool)(host_byte_sex != ofile->object_byte_sex);
	strings = (char *)(ofile->object_addr + st->stroff);

	if(ofile->mh != NULL){
	    ofile->nmodtab = dyst->nmodtab;
	    ofile->modtab = (struct dylib_module *)(ofile->object_addr +
						    dyst->modtaboff);
	    p = ofile->modtab;
	    for(i = 0; i < dyst->nmodtab; i++){
		m = *p;
		if(swapped)
		    swap_dylib_module(&m, 1, host_byte_sex);
		ofile->dylib_module = p;
		if(check_dylib_module(ofile, st, dyst, strings, i) == CHECK_BAD)
		    return(FALSE);
		if(strcmp(module_name, strings + m.module_name) == 0){
		    ofile->dylib_module_name = strings + m.module_name;
		    return(TRUE);
		}
		p++;
	    }
	    m = *ofile->dylib_module;
	    if(swapped)
		swap_dylib_module(&m, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m.module_name;
	}
	else{
	    ofile->nmodtab = dyst->nmodtab;
	    ofile->modtab64 = (struct dylib_module_64 *)(ofile->object_addr +
						         dyst->modtaboff);
	    p64 = ofile->modtab64;
	    for(i = 0; i < dyst->nmodtab; i++){
		m64 = *p64;
		if(swapped)
		    swap_dylib_module_64(&m64, 1, host_byte_sex);
		ofile->dylib_module64 = p64;
		if(check_dylib_module(ofile, st, dyst, strings, i) == CHECK_BAD)
		    return(FALSE);
		if(strcmp(module_name, strings + m64.module_name) == 0){
		    ofile->dylib_module_name = strings + m64.module_name;
		    return(TRUE);
		}
		p64++;
	    }
	    m64 = *ofile->dylib_module64;
	    if(swapped)
		swap_dylib_module_64(&m64, 1, host_byte_sex);
	    ofile->dylib_module_name = strings + m64.module_name;
	}
#ifndef OTOOL
	Mach_O_error(ofile, "does not contain a module named: %s", module_name);
#endif
	ofile->modtab = NULL;
	ofile->nmodtab = 0;
	ofile->dylib_module = NULL;
	ofile->dylib_module_name = NULL;
	return(FALSE);
}

#ifdef DEBUG
__private_extern__
void
ofile_print(
struct ofile *ofile)
{
	printf("file_name = %s\n", ofile->file_name);
	printf("file_addr = 0x%x\n", (unsigned int)ofile->file_addr);
	printf("file_size = 0x%x\n", (unsigned int)ofile->file_size);
	printf("file_type = 0x%x\n", (unsigned int)ofile->file_type);
	printf("fat_header = 0x%x\n", (unsigned int)ofile->fat_header);
	printf("fat_archs = 0x%x\n", (unsigned int)ofile->fat_archs);
	printf("fat_archs64 = 0x%x\n", (unsigned int)ofile->fat_archs64);
	printf("narch = 0x%x\n", (unsigned int)ofile->narch);
	printf("arch_type = 0x%x\n", (unsigned int)ofile->arch_type);
	printf("arch_flag.name = %s\n", ofile->arch_flag.name);
	printf("arch_flag.cputype = 0x%x\n",
		(unsigned int)ofile->arch_flag.cputype);
	printf("arch_flag.cpusubtype = 0x%x\n",
		(unsigned int)ofile->arch_flag.cpusubtype);
	printf("member_offset = 0x%x\n", (unsigned int)ofile->member_offset);
	printf("member_addr = 0x%x\n", (unsigned int)ofile->member_addr);
	printf("member_size = 0x%x\n", (unsigned int)ofile->member_size);
	printf("member_ar_hdr = 0x%x\n", (unsigned int)ofile->member_ar_hdr);
	printf("member_type = 0x%x\n", (unsigned int)ofile->member_type);
	printf("archive_cputype = 0x%x\n",
		(unsigned int)ofile->archive_cputype);
	printf("archive_cpusubtype = 0x%x\n",
		(unsigned int)ofile->archive_cpusubtype);
	printf("object_addr = 0x%x\n", (unsigned int)ofile->object_addr);
	printf("object_size = 0x%x\n", (unsigned int)ofile->object_size);
	printf("object_byte_sex = 0x%x\n",
		(unsigned int)ofile->object_byte_sex);
	printf("mh = 0x%x\n", (unsigned int)ofile->mh);
	printf("mh64 = 0x%x\n", (unsigned int)ofile->mh64);
	printf("load_commands = 0x%x\n", (unsigned int)ofile->load_commands);
}
#endif /* DEBUG */

/*
 * check_fat() checks the fat ofile for correctness (the fat_header and
 * fat_archs or fat_archs64 are assumed to be in the host byte sex).
 */
static
enum check_type
check_fat(
struct ofile *ofile)
{
#ifdef OTOOL
	return(CHECK_GOOD);
#else /* !defined OTOOL */

    uint32_t i, j;
    uint64_t big_size;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint64_t offset;
    uint64_t size;
    uint32_t align;

	if(ofile->file_type != OFILE_FAT){
	    error("internal error. check_fat() call and file type of: %s is "
		  "not OFILE_FAT\n", ofile->file_name);
	    return(CHECK_BAD);
	}
	if(ofile->fat_header->nfat_arch == 0){
	    error("fat file: %s malformed (contains zero architecture types)",
		  ofile->file_name);
	    return(CHECK_BAD);
	}
	for(i = 0; i < ofile->fat_header->nfat_arch; i++){
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		cputype = ofile->fat_archs64[i].cputype;
		cpusubtype = ofile->fat_archs64[i].cpusubtype;
		offset = ofile->fat_archs64[i].offset;
		size = ofile->fat_archs64[i].size;
		align = ofile->fat_archs64[i].align;
	    }
	    else{
		cputype = ofile->fat_archs[i].cputype;
		cpusubtype = ofile->fat_archs[i].cpusubtype;
		offset = ofile->fat_archs[i].offset;
		size = ofile->fat_archs[i].size;
		align = ofile->fat_archs[i].align;
	    }
	    big_size = offset;
	    big_size += size;
	    if(big_size > ofile->file_size){
		error("fat file: %s truncated or malformed (offset plus size "
		      "of cputype (%d) cpusubtype (%d) extends past the "
		      "end of the file)", ofile->file_name,
		      cputype, cpusubtype & ~CPU_SUBTYPE_MASK);
		return(CHECK_BAD);
	    }
	    if(align > MAXSECTALIGN){
		error("fat file: %s align (2^%u) too large for cputype (%d) "
		      "cpusubtype (%d) (maximum 2^%d)", ofile->file_name,
		      align, cputype, cpusubtype & ~CPU_SUBTYPE_MASK,
		      MAXSECTALIGN);
		return(CHECK_BAD);
	    }
	    if(offset %
	       (1 << align) != 0){
		error("fat file: %s offset: %llu for cputype (%d) cpusubtype "
		      "(%d)) not aligned on it's alignment (2^%u)",
		      ofile->file_name, offset, cputype,
		      cpusubtype & ~CPU_SUBTYPE_MASK, align);
		return(CHECK_BAD);
	    }
	}
	for(i = 0; i < ofile->fat_header->nfat_arch; i++){
	    for(j = i + 1; j < ofile->fat_header->nfat_arch; j++){
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    if(ofile->fat_archs64[i].cputype ==
			 ofile->fat_archs64[j].cputype &&
		       (ofile->fat_archs64[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (ofile->fat_archs64[j].cpusubtype &
				~CPU_SUBTYPE_MASK)){
			error("fat file: %s contains two of the same "
			      "architecture (cputype (%d) cpusubtype (%d))",
			      ofile->file_name, ofile->fat_archs64[i].cputype,
			      ofile->fat_archs64[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
			return(CHECK_BAD);
		    }
		}
		else{
		    if(ofile->fat_archs[i].cputype ==
			 ofile->fat_archs[j].cputype &&
		       (ofile->fat_archs[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (ofile->fat_archs[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			error("fat file: %s contains two of the same "
			      "architecture (cputype (%d) cpusubtype (%d))",
			      ofile->file_name, ofile->fat_archs[i].cputype,
			      ofile->fat_archs[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
			return(CHECK_BAD);
		    }
		}
	    }
	}
	return(CHECK_GOOD);
#endif /* OTOOL */
}

/*
 * check_fat_object_in_archive() checks the fat object file which is a member
 * of a thin archive for correctness (the fat_header and fat_archs or
 * fat_archs64 are assumed to be in the host byte sex).  This is not a legal
 * form but allowed when archives_with_fat_objects is TRUE when ofile_map() is
 * called.
 */
static
enum check_type
check_fat_object_in_archive(
struct ofile *ofile)
{
    uint32_t i, j;
    uint32_t magic;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint64_t offset;
    uint64_t size;
    uint32_t align;

	if(ofile->file_type != OFILE_ARCHIVE){
	    error("internal error. check_fat_object_in_archive() called and "
		  "file type of: %s is not OFILE_ARCHIVE\n", ofile->file_name);
	    return(CHECK_BAD);
	}
	if(ofile->fat_header->nfat_arch == 0){
	    archive_member_error(ofile, "fat file malformed (contains zero "
				 "architecture types)");
	    return(CHECK_BAD);
	}
	for(i = 0; i < ofile->fat_header->nfat_arch; i++){
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		cputype = ofile->fat_archs64[i].cputype;
		cpusubtype = ofile->fat_archs64[i].cpusubtype;
		offset = ofile->fat_archs64[i].offset;
		size = ofile->fat_archs64[i].size;
		align = ofile->fat_archs64[i].align;
	    }
	    else{
		cputype = ofile->fat_archs[i].cputype;
		cpusubtype = ofile->fat_archs[i].cpusubtype;
		offset = ofile->fat_archs[i].offset;
		size = ofile->fat_archs[i].size;
		align = ofile->fat_archs[i].align;
	    }
	    if(offset + size > ofile->member_size){
		archive_member_error(ofile, "fat file truncated or malformed "
			"(offset plus size of cputype (%d) cpusubtype (%d) "
			"extends past the end of the file)", 
		        cputype, cpusubtype & ~CPU_SUBTYPE_MASK);
		return(CHECK_BAD);
	    }
	    if(align > MAXSECTALIGN){
		archive_member_error(ofile, "fat file's align (2^%u) too "
			"large for cputype (%d) cpusubtype (%d) (maximum 2^%d)",
			align, cputype, cpusubtype & ~CPU_SUBTYPE_MASK,
			MAXSECTALIGN);
		return(CHECK_BAD);
	    }
	    if(offset % (1 << align) != 0){
		archive_member_error(ofile, "fat file's offset: %llu for "
			"cputype (%d) cpusubtype (%d) not aligned on it's "
			"alignment (2^%u)", offset, cputype,
			cpusubtype & ~CPU_SUBTYPE_MASK, align);
		return(CHECK_BAD);
	    }

	    /*
	     * The only supported format where fat files are allowed to appear
	     * in archives is when the fat file contains only object files.
	     */
	    if(size < sizeof(struct mach_header)){
		archive_member_error(ofile, "fat file for cputype (%d) "
			"cpusubtype (%d) is not an object file (size too small "
			"to be an object file)", cputype,
			cpusubtype & ~CPU_SUBTYPE_MASK);
		return(CHECK_BAD);
	    }
	    memcpy(&magic,
		   ofile->file_addr + ofile->member_offset +
			offset,
		   sizeof(uint32_t));
	    if(magic == MH_MAGIC || magic == SWAP_INT(MH_MAGIC)){
#ifdef ALIGNMENT_CHECKS
		if((ofile->member_offset + offset) %
		   4 != 0){
		    archive_member_error(ofile, "fat object file's offset in "
			    "archive not a multiple of 4) (must be since "
			    "member is a 32-bit object file)");
		    return(CHECK_BAD);
		}
#endif /* ALIGNMENT_CHECKS */
	    }
	    else if(magic == MH_MAGIC_64 || magic == SWAP_INT(MH_MAGIC_64)){
#ifdef ALIGNMENT_CHECKS_ARCHIVE_64_BIT
		if(archive_64_bit_align_warning == FALSE &&
		   (ofile->member_offset + offset) %
		   8 != 0){
		    temporary_archive_member_warning(ofile, "fat object file's "
			"offset in archive not a multiple of 8) (must be since "
			"member is a 64-bit object file)");
		    archive_64_bit_align_warning = TRUE;
		    /* return(CHECK_BAD); */
		}
#endif /* ALIGNMENT_CHECKS_ARCHIVE_64_BIT */
	    }
	    else{
#ifdef LTO_SUPPORT
	        if(is_llvm_bitcode(ofile, ofile->file_addr +
		   ofile->member_offset + offset,
		   size) == TRUE){
#ifdef ALIGNMENT_CHECKS_ARCHIVE_64_BIT
		    if(archive_64_bit_align_warning == FALSE &&
		       (ofile->member_offset + offset) %
		       8 != 0){
			temporary_archive_member_warning(ofile, "fat object "
			    "file's offset in archive not a multiple of 8) "
			    "(must be since member is a 64-bit object file)");
			archive_64_bit_align_warning = TRUE;
			/* return(CHECK_BAD); */
		    }
#endif
	        }
		else
#endif /* LTO_SUPPORT */
		{
		    archive_member_error(ofile, "fat file for cputype (%d) "
			    "cpusubtype (%d) is not an object file (bad magic "
			    "number)", cputype,
			    cpusubtype & ~CPU_SUBTYPE_MASK);
		    return(CHECK_BAD);
		}
	    }
	}
	for(i = 0; i < ofile->fat_header->nfat_arch; i++){
	    for(j = i + 1; j < ofile->fat_header->nfat_arch; j++){
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    if(ofile->fat_archs64[i].cputype ==
			 ofile->fat_archs64[j].cputype &&
		       (ofile->fat_archs64[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (ofile->fat_archs64[j].cpusubtype &
				~CPU_SUBTYPE_MASK)){
			error("fat file: %s contains two of the same "
			      "architecture (cputype (%d) cpusubtype (%d))",
			      ofile->file_name, ofile->fat_archs64[i].cputype,
			      ofile->fat_archs64[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
			return(CHECK_BAD);
		    }
		}
		else{
		    if(ofile->fat_archs[i].cputype ==
			 ofile->fat_archs[j].cputype &&
		       (ofile->fat_archs[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			 (ofile->fat_archs[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			error("fat file: %s contains two of the same "
			      "architecture (cputype (%d) cpusubtype (%d))",
			      ofile->file_name, ofile->fat_archs[i].cputype,
			      ofile->fat_archs[i].cpusubtype &
				~CPU_SUBTYPE_MASK);
			return(CHECK_BAD);
		    }
		}
	    }
	}
	return(CHECK_GOOD);
}

/*
 * check_archive() checks the archive referenced in the ofile for correctness.
 */
static
enum check_type
check_archive(
struct ofile *ofile,
enum bool archives_with_fat_objects)
{
    char *addr;
    uint64_t size, offset;
    uint64_t big_size;
    uint32_t magic;
    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct mach_header mh;
    struct mach_header_64 mh64;
    struct ar_hdr *ar_hdr;
    uint32_t ar_name_size;

	/*
	 * Get the address and size of the archive (as well as the cputype and
	 * cpusubtype if known) and make sure it is an archive.
	 */
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->fat_header->magic == FAT_MAGIC_64){
		addr = ofile->file_addr +
		       ofile->fat_archs64[ofile->narch].offset;
		size = ofile->fat_archs64[ofile->narch].size;
		ofile->archive_cputype =
				ofile->fat_archs64[ofile->narch].cputype;
		ofile->archive_cpusubtype =
				ofile->fat_archs64[ofile->narch].cpusubtype;
	    }
	    else{
		addr = ofile->file_addr + ofile->fat_archs[ofile->narch].offset;
		size = ofile->fat_archs[ofile->narch].size;
		ofile->archive_cputype = ofile->fat_archs[ofile->narch].cputype;
		ofile->archive_cpusubtype =
				ofile->fat_archs[ofile->narch].cpusubtype;
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    addr = ofile->file_addr;
	    size = ofile->file_size;
	    ofile->archive_cputype = 0;
	    ofile->archive_cpusubtype = 0;
	}
	else{
	    error("internal error. check_archive() call and file type of %s is "
		  "OFILE_UNKNOWN\n", ofile->file_name);
	    return(CHECK_BAD);
	}
	if(size < SARMAG || strncmp(addr, ARMAG, SARMAG) != 0){
	    error("internal error. check_archive() call for file %s which does "
		  "not have an archive magic string", ofile->file_name);
	    return(CHECK_BAD);
	}

	host_byte_sex = get_host_byte_sex();
	/*
	 * Check this archive out to make sure that it does not contain
	 * any fat files and that all object files it contains have the
	 * same cputype and subsubtype.
	 */
	offset = SARMAG;
	if(offset == size)
	    return(CHECK_GOOD);
	if(offset != size && offset + sizeof(struct ar_hdr) > size){
	    archive_error(ofile, "truncated or malformed (archive header of "
			  "first member extends past the end of the file)");
	    return(CHECK_BAD);
	}
	while(size > offset){
	    ar_hdr = (struct ar_hdr *)(addr + offset);
	    ofile->member_offset = offset;
	    ofile->member_addr = addr + offset;
	    ofile->member_size = (uint32_t)strtoul(ar_hdr->ar_size, NULL, 10);
	    ofile->member_ar_hdr = ar_hdr;
	    ofile->member_name = ar_hdr->ar_name;
	    ofile->member_name_size = size_ar_name(ofile->member_ar_hdr);
	    offset += sizeof(struct ar_hdr);
	    /*
	     * See if this archive member is using extend format #1 where
	     * the size of the name is in ar_name and the name follows the
	     * archive header.
	     */
	    ar_name_size = 0;
	    if(strncmp(ofile->member_name,AR_EFMT1, sizeof(AR_EFMT1) - 1) == 0){
		if(check_extend_format_1(ofile, ar_hdr,
					 (uint32_t)(size - offset),
					 &ar_name_size) == CHECK_BAD)
		    return(CHECK_BAD);
		ofile->member_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
		ofile->member_name_size = ar_name_size;
		offset += ar_name_size;
		ofile->member_offset += ar_name_size;
		ofile->member_addr += ar_name_size;
		ofile->member_size -= ar_name_size;
	    }
#ifndef OTOOL
	    big_size = rnd(ofile->member_size, sizeof(short));
	    big_size += offset;
	    if(big_size > size){
		archive_member_error(ofile, "size too large (archive "
			      "member extends past the end of the file)");
		return(CHECK_BAD);
	    }
#endif /* !defined(OTOOL) */
	    if(size - offset > sizeof(uint32_t)){
		memcpy(&magic, addr + offset, sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
		if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
		if(magic == SWAP_INT(FAT_MAGIC) ||
		   magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
		{
		    if(archives_with_fat_objects == FALSE ||
		       ofile->file_type != OFILE_ARCHIVE){
			archive_member_error(ofile, "is a fat file (not "
					     "allowed in an archive)");
			return(CHECK_BAD);
		    }
		}
		else{
		    if(size - offset >= sizeof(struct mach_header) &&
		       (magic == MH_MAGIC || magic == SWAP_INT(MH_MAGIC))){
			memcpy(&mh, addr + offset, sizeof(struct mach_header));
			if(magic == SWAP_INT(MH_MAGIC)){
			    magic = MH_MAGIC;
			    swapped = TRUE;
			    swap_mach_header(&mh, host_byte_sex);
			}
			swapped = FALSE;
		    }
		    else if(size - offset >= sizeof(struct mach_header_64) &&
		       (magic == MH_MAGIC_64 ||
			magic == SWAP_INT(MH_MAGIC_64))){
			memcpy(&mh64, addr + offset,
			       sizeof(struct mach_header_64));
			if(magic == SWAP_INT(MH_MAGIC_64)){
			    magic = MH_MAGIC_64;
			    swapped = TRUE;
			    swap_mach_header_64(&mh64, host_byte_sex);
			}
			swapped = FALSE;
		    }
		    if(magic == MH_MAGIC){
			if(ofile->archive_cputype == 0){
			    ofile->archive_cputype = mh.cputype;
			    ofile->archive_cpusubtype = mh.cpusubtype;
			}
#ifndef OTOOL
			else if(ofile->archive_cputype != mh.cputype){
			    archive_member_error(ofile, "cputype (%d) does not "
				"match previous archive members cputype (%d) "
				"(all members must match)", mh.cputype,
				ofile->archive_cputype);
			}
#endif /* !defined(OTOOL) */
		    }
		    else if(magic == MH_MAGIC_64){
			if(ofile->archive_cputype == 0){
			    ofile->archive_cputype = mh64.cputype;
			    ofile->archive_cpusubtype = mh64.cpusubtype;
			}
#ifndef OTOOL
			else if(ofile->archive_cputype != mh64.cputype){
			    archive_member_error(ofile, "cputype (%d) does not "
				"match previous archive members cputype (%d) "
				"(all members must match)", mh64.cputype,
				ofile->archive_cputype);
			}
#endif /* !defined(OTOOL) */
		    }
		}
	    }
#ifdef OTOOL
	    big_size = rnd(ofile->member_size, sizeof(short));
	    big_size += offset;
	    if(big_size > size)
		offset = size;
	    else
                offset += rnd(ofile->member_size, sizeof(short));
#else
	    offset += rnd(ofile->member_size, sizeof(short));
#endif /* !defined(OTOOL) */
	}
	ofile->member_offset = 0;
	ofile->member_addr = NULL;
	ofile->member_size = 0;
	ofile->member_ar_hdr = NULL;;
	ofile->member_name = NULL;
	ofile->member_name_size = 0;
	return(CHECK_GOOD);
}

/*
 * check_extend_format_1() checks the archive header for extended format #1.
 */
static
enum check_type
check_extend_format_1(
struct ofile *ofile,
struct ar_hdr *ar_hdr,
uint32_t size_left,
uint32_t *member_name_size)
{
    char *p, *endp, buf[sizeof(ar_hdr->ar_name)+1];
    uint32_t ar_name_size;

	*member_name_size = 0;

	buf[sizeof(ar_hdr->ar_name)] = '\0';
	memcpy(buf, ar_hdr->ar_name, sizeof(ar_hdr->ar_name));
	p = buf + sizeof(AR_EFMT1) - 1;
	if(isdigit(*p) == 0){
	    archive_error(ofile, "malformed (ar_name: %.*s for archive "
		"extend format #1 starts with non-digit)",
		(int)sizeof(ar_hdr->ar_name), ar_hdr->ar_name);
	    return(CHECK_BAD);
	}
	ar_name_size = (uint32_t)strtoul(p, &endp, 10);
	if(ar_name_size == UINT_MAX && errno == ERANGE){
	    archive_error(ofile, "malformed (size in ar_name: %.*s for "
		"archive extend format #1 overflows uint32_t)",
		(int)sizeof(ar_hdr->ar_name), ar_hdr->ar_name);
	    return(CHECK_BAD);
	}
	while(*endp == ' ' && *endp != '\0')
	    endp++;
	if(*endp != '\0'){
	    archive_error(ofile, "malformed (size in ar_name: %.*s for "
		"archive extend format #1 contains non-digit and "
		"non-space characters)", (int)sizeof(ar_hdr->ar_name),
		ar_hdr->ar_name);
	    return(CHECK_BAD);
	}
	if(ar_name_size > size_left){
	    archive_error(ofile, "truncated or malformed (archive name "
		"of member extends past the end of the file)");
	    return(CHECK_BAD);
	}
	*member_name_size = ar_name_size;
	return(CHECK_GOOD);
}

/*
 * check_archive_toc() checks the archive table of contents referenced in the
 * thin archive via the ofile for correctness and if bad sets the bad_toc field
 * in the ofile struct to TRUE.   If not it sets the other toc_* fields that
 * ranlib(1) uses to know it can't update the table of contents and doesn't
 * have to totally rebuild it.  And by this always returning CHECK_GOOD it
 * allows otool(1) to print messed up tables of contents for debugging.
 */
static
enum check_type
check_archive_toc(
struct ofile *ofile)
{
    uint32_t symdef_length, nranlibs, strsize;
    uint64_t i, n, offset, ran_off, nranlibs64, strsize64;
    enum byte_sex host_byte_sex, toc_byte_sex;
    struct ranlib *ranlibs;
    struct ranlib_64 *ranlibs64;
    char *strings;
    enum bool toc_is_32bit;

	/* cctools-port start */
	ranlibs = NULL;
	ranlibs64 = NULL;
	/* cctools-port end */

	/* silence clang warnings */
	nranlibs = 0;
	nranlibs64 = 0;
	strsize = 0;
	strsize64 = 0;
	ranlibs = NULL;
	ranlibs64 = NULL;
    
	ofile->toc_is_32bit = TRUE;
	ofile->toc_ranlibs = NULL;
	ofile->toc_ranlibs64 = NULL;
	ofile->toc_nranlibs = 0;
	ofile->toc_strings = NULL;
	ofile->toc_strsize = 0;

	/*
	 * Note this can only be called when the whole file is a thin archive.
	 */
	if(ofile->file_type != OFILE_ARCHIVE)
	    return(CHECK_GOOD);

	symdef_length = ofile->toc_size;
	if(strncmp(ofile->member_name, SYMDEF_64_SORTED,
		   ofile->member_name_size) == 0 ||
	   strncmp(ofile->member_name, SYMDEF_64,
		   ofile->member_name_size) == 0)
	    toc_is_32bit = FALSE;
	else
	    toc_is_32bit = TRUE;
	if(toc_is_32bit == TRUE){
	    /*
	     * The contents of a __.SYMDEF file is begins with a 32-bit word
	     * giving the size in bytes of ranlib structures which immediately
	     * follow, and then continues with a string table consisting of a
	     * 32-bit word giving the number of bytes of strings which follow
	     * and then the strings themselves.  So the smallest valid size is
	     * two 32-bit words long.
	     */
	    if(symdef_length < 2 * sizeof(uint32_t)){
		/*
		 * Size of table of contents for archive too small to be a valid
		 * table of contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }
	} else {
	    /*
	     * The contents of a __.SYMDEF_64 file is begins with a 64-bit word
	     * giving the size in bytes of ranlib structures which immediately
	     * follow, and then continues with a string table consisting of a
	     * 64-bit word giving the number of bytes of strings which follow
	     * and then the strings themselves.  So the smallest valid size is
	     * two 64-bit words long.
	     */
	    if(symdef_length < 2 * sizeof(uint64_t)){
		/*
		 * Size of table of contents for archive too small to be a valid
		 * table of contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }
        }
	host_byte_sex = get_host_byte_sex();
	toc_byte_sex = get_toc_byte_sex(ofile->file_addr,
					(uint32_t)ofile->file_size);
	if(toc_byte_sex == UNKNOWN_BYTE_SEX){
	    /*
	     * Can't determine the byte order of table of contents as it
	     * contains no Mach-O files.
	     */
	    ofile->toc_bad = TRUE;
	    return(CHECK_GOOD);
	}
	offset = 0;
	if(toc_is_32bit == TRUE){
	    nranlibs = *((uint32_t *)(ofile->toc_addr + offset));
	    if(toc_byte_sex != host_byte_sex)
		nranlibs = SWAP_INT(nranlibs);
	    nranlibs = nranlibs / sizeof(struct ranlib);
	    n = nranlibs;
	    offset += sizeof(uint32_t);

	    ranlibs = (struct ranlib *)(ofile->toc_addr + offset);
	    offset += sizeof(struct ranlib) * nranlibs;
	    if(nranlibs == 0)
		return(CHECK_GOOD);
	    if(offset - (2 * sizeof(uint32_t)) > symdef_length){
		/*
		 * Truncated or malformed archive.  The ranlib structures in
		 * table of contents extends past the end of the table of
		 * contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }

	    strsize = *((uint32_t *)(ofile->toc_addr + offset));
	    if(toc_byte_sex != host_byte_sex)
		strsize = SWAP_INT(strsize);
	    offset += sizeof(uint32_t);

	    strings = ofile->toc_addr + offset;
	    offset += strsize;
	    if(offset - (2 * sizeof(uint32_t)) > symdef_length){
		/*
		 * Truncated or malformed archive.  The ranlib strings in table
		 * of contents extends past the end of the table of contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }
	    if(symdef_length == 2 * sizeof(uint32_t))
		return(CHECK_GOOD);
	} else {
	    nranlibs64 = *((uint64_t *)(ofile->toc_addr + offset));
	    if(toc_byte_sex != host_byte_sex)
		nranlibs64 = SWAP_LONG_LONG(nranlibs64);
	    nranlibs64 = nranlibs64 / sizeof(struct ranlib_64);
	    n = nranlibs64;
	    offset += sizeof(uint64_t);

	    ranlibs64 = (struct ranlib_64 *)(ofile->toc_addr + offset);
	    offset += sizeof(struct ranlib_64) * nranlibs64;
	    if(nranlibs64 == 0)
		return(CHECK_GOOD);
	    if(offset - (2 * sizeof(uint64_t)) > symdef_length){
		/*
		 * Truncated or malformed archive.  The ranlib structures in
		 * table of contents extends past the end of the table of
		 * contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }

	    strsize64 = *((uint64_t *)(ofile->toc_addr + offset));
	    if(toc_byte_sex != host_byte_sex)
		strsize64 = SWAP_LONG_LONG(strsize64);
	    offset += sizeof(uint64_t);

	    strings = ofile->toc_addr + offset;
	    offset += strsize64;
	    if(offset - (2 * sizeof(uint64_t)) > symdef_length){
		/*
		 * Truncated or malformed archive.  The ranlib strings in table
		 * of contents extends past the end of the table of contents.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }
	    if(symdef_length == 2 * sizeof(uint64_t))
		return(CHECK_GOOD);
	}

	/*
	 * Check the string offset and the member offsets of the ranlib structs.
	 */
	if(toc_byte_sex != host_byte_sex){
	    if(toc_is_32bit == TRUE)
	        swap_ranlib(ranlibs, nranlibs, host_byte_sex);
	    else
	        swap_ranlib_64(ranlibs64, nranlibs64, host_byte_sex);
	}
	for(i = 0; i < n; i++){
	    if(toc_is_32bit == TRUE){
		if(ranlibs[i].ran_un.ran_strx >= strsize){
		    /*
		     * Malformed table of contents.  The ranlib struct at this
		     * index has a bad string index field.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
		if(ranlibs[i].ran_off >= ofile->file_size){
		    /*
		     * Malformed table of contents.  The ranlib struct at this
		     * index has a bad library member offset field.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
		ran_off = ranlibs[i].ran_off;
	    } else {
		if(ranlibs64[i].ran_un.ran_strx >= strsize64){
		    /*
		     * Malformed table of contents.  The ranlib struct at this
		     * index has a bad string index field.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
		if(ranlibs64[i].ran_off >= ofile->file_size){
		    /*
		     * Malformed table of contents.  The ranlib struct at this
		     * index has a bad library member offset field.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
		ran_off = ranlibs64[i].ran_off;
	    }

	    /*
	     * These should be on 4 byte boundaries because the maximum
	     * alignment of the header structures and relocation are 4 bytes.
	     * But this is has to be 2 bytes because that's the way ar(1) has
	     * worked historicly in the past.  Fortunately this works on the
	     * 68k machines but will have to change when this is on a real
	     * machine.
	     */
#if defined(mc68000) || defined(__i386__)
	    if(ran_off % sizeof(short) != 0){
		/*
		 * Malformed table of contents.  This ranlib struct library
		 * member offset not a multiple 2 bytes.
		 */
		ofile->toc_bad = TRUE;
		return(CHECK_GOOD);
	    }
#else
	    if(toc_is_32bit == TRUE){
		if(ran_off % sizeof(uint32_t) != 0){
		    /*
		     * Malformed table of contents.  This ranlib struct library
		     * member offset not a multiple of 4 bytes.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
	    } else {
		if(ran_off % sizeof(uint64_t) != 0){
		    /*
		     * Malformed table of contents.  This ranlib struct library
		     * member offset not a multiple of 8 bytes.
		     */
		    ofile->toc_bad = TRUE;
		    return(CHECK_GOOD);
		}
	    }
#endif
	}
	ofile->toc_is_32bit = toc_is_32bit;
	ofile->toc_ranlibs = ranlibs;
	ofile->toc_ranlibs64 = ranlibs64;
	ofile->toc_strings = strings;
	if(toc_is_32bit == TRUE){
	    ofile->toc_nranlibs = nranlibs;
	    ofile->toc_strsize = strsize;
	} else {
	    ofile->toc_nranlibs = nranlibs64;
	    ofile->toc_strsize = strsize64;
	}
	return(CHECK_GOOD);
}

/*
 * check_Mach_O() checks the object file's mach header and load commands
 * referenced in the ofile for correctness (this also swaps the mach header
 * and load commands into the host byte sex if needed).
 */
static
enum check_type
check_Mach_O(
struct ofile *ofile)
{
#ifdef OTOOL
	return(CHECK_GOOD);
#else /* !defined OTOOL */
    uint32_t size, i, j, ncmds, sizeofcmds, load_command_multiple, sizeofhdrs;
    cpu_type_t cputype;
    char *addr, *cmd_name, *element_name;
    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct mach_header *mh;
    struct mach_header_64 *mh64;
    uint32_t mh_flags;
    struct load_command *load_commands, *lc, l;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct symseg_command *ss;
    struct fvmlib_command *fl;
    struct dylib_command *dl;
    struct sub_framework_command *sub;
    struct sub_umbrella_command *usub;
    struct sub_library_command *lsub;
    struct sub_client_command *csub;
    struct prebound_dylib_command *pbdylib;
    struct dylinker_command *dyld;
    struct thread_command *ut;
    struct ident_command *id;
    struct routines_command *rc;
    struct routines_command_64 *rc64;
    struct twolevel_hints_command *hints;
    struct linkedit_data_command *code_sig, *split_info, *func_starts,
			     *data_in_code, *code_sign_drs, *linkedit_data,
			     *exports_trie, *chained_fixups;
    struct linkedit_data_command *link_opt_hint;
    struct version_min_command *vers;
    struct build_version_command *bv, *bv1, *bv2;
    struct build_tool_version *btv;
    struct prebind_cksum_command *cs;
    struct encryption_info_command *encrypt_info;
    struct encryption_info_command_64 *encrypt_info64;
    struct linker_option_command *lo;
    struct dyld_info_command *dyld_info;
    struct uuid_command *uuid;
    struct rpath_command *rpath;
    struct entry_point_command *ep;
    struct source_version_command *sv;
    struct note_command *nc;
    uint32_t flavor, count, nflavor;
    char *p, *state;
    uint32_t sizeof_nlist, sizeof_dylib_module;
    char *struct_dylib_module_name, *struct_nlist_name;
    uint64_t big_size, big_end, big_load_end;
    struct element elements;
    cpu_type_t fat_cputype;

	elements.offset = 0;
	elements.size = 0;
	elements.name = NULL;
	elements.next = NULL;

	addr = ofile->object_addr;
	size = ofile->object_size;
	mh = ofile->mh;
	mh64 = ofile->mh64;
	load_commands = ofile->load_commands;
	host_byte_sex = get_host_byte_sex();
	swapped = (enum bool)(host_byte_sex != ofile->object_byte_sex);

	if(ofile->mh != NULL){
	    if(swapped)
		swap_mach_header(mh, host_byte_sex);
	    big_size = mh->sizeofcmds;
	    big_size += sizeof(struct mach_header);
	    if(big_size > size){
		Mach_O_error(ofile, "truncated or malformed object (load "
			     "commands extend past the end of the file)");
		return(CHECK_BAD);
	    }
	    sizeofhdrs = (uint32_t)big_size;
	    ofile->mh_cputype = mh->cputype;
	    ofile->mh_cpusubtype = mh->cpusubtype;
	    ofile->mh_filetype = mh->filetype;
	    ncmds = mh->ncmds;
	    sizeofcmds = mh->sizeofcmds;
	    cputype = mh->cputype;
	    mh_flags = mh->flags;
	    load_command_multiple = 4;
	    sizeof_nlist = sizeof(struct nlist);
	    struct_nlist_name = "struct nlist";
	    sizeof_dylib_module = sizeof(struct dylib_module);
	    struct_dylib_module_name = "struct dylib_module";
	}
	else{
	    if(swapped)
		swap_mach_header_64(mh64, host_byte_sex);
	    big_size = mh64->sizeofcmds;
	    big_size += sizeof(struct mach_header_64);
	    if(big_size > size){
		Mach_O_error(ofile, "truncated or malformed object (load "
			     "commands extend past the end of the file)");
		return(CHECK_BAD);
	    }
	    sizeofhdrs = (uint32_t)big_size;
	    ofile->mh_cputype = mh64->cputype;
	    ofile->mh_cpusubtype = mh64->cpusubtype;
	    ofile->mh_filetype = mh64->filetype;
	    ncmds = mh64->ncmds;
	    sizeofcmds = mh64->sizeofcmds;
	    cputype = mh64->cputype;
	    mh_flags = mh64->flags;
	    load_command_multiple = 8;
	    sizeof_nlist = sizeof(struct nlist_64);
	    struct_nlist_name = "struct nlist_64";
	    sizeof_dylib_module = sizeof(struct dylib_module_64);
	    struct_dylib_module_name = "struct dylib_module_64";
	}
	if(check_overlaping_element(ofile, &elements, 0, sizeofhdrs,
		"Mach-O headers") == CHECK_BAD)
	    goto return_bad;
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->fat_header->magic == FAT_MAGIC_64)
	        fat_cputype = ofile->fat_archs64[ofile->narch].cputype;
	    else
	        fat_cputype = ofile->fat_archs[ofile->narch].cputype;
	    if(fat_cputype != ofile->mh_cputype){
		Mach_O_error(ofile, "malformed fat file (fat header "
		    "architecture: %u's cputype does not match "
		    "object file's mach header)", ofile->narch);
		goto return_bad;
	    }
	}
	/*
	 * Make a pass through the load commands checking them to the level
	 * that they can be parsed and all fields with offsets and sizes do
	 * not extend past the end of the file.
	 */
	st = NULL;
	dyst = NULL;
	rc = NULL;
	rc64 = NULL;
	hints = NULL;
	code_sig = NULL;
	func_starts = NULL;
	data_in_code = NULL;
	code_sign_drs = NULL;
	link_opt_hint = NULL;
	exports_trie = NULL;
	chained_fixups = NULL;
	split_info = NULL;
	cs = NULL;
	uuid = NULL;
	encrypt_info = NULL;
	dyld_info = NULL;
	vers = NULL;
	bv = NULL;
	bv1 = NULL;
	bv2 = NULL;
	big_load_end = 0;
	for(i = 0, lc = load_commands; i < ncmds; i++){
	    if(big_load_end + sizeof(struct load_command) > sizeofcmds){
		Mach_O_error(ofile, "truncated or malformed object (load "
			     "command %u extends past the end all load "
			     "commands in the file)", i);
		goto return_bad;
	    }
	    l = *lc;
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    /*
	     * Check load command size for a multiple of load_command_multiple.
	     */
	    if(l.cmdsize % load_command_multiple != 0){
		/*
		 * We have a hack here to allow 64-bit Mach-O core files to
		 * have LC_THREAD commands that are only a multiple of 4 and
		 * not 8 to be allowed since the kernel produces them.
		 */
		if(ofile->mh64 == NULL ||
		   ofile->mh64->filetype != MH_CORE ||
		   l.cmd != LC_THREAD ||
		   l.cmdsize % 4 != 0){
		    Mach_O_error(ofile, "malformed object (load command %u "
				 "cmdsize not a multiple of %u)", i,
				 load_command_multiple);
		    goto return_bad;
		}
	    }
	    /* check that load command does not extends past end of commands */
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds){
		Mach_O_error(ofile, "truncated or malformed object (load "
			     "command %u extends past the end of the file)",i);
		goto return_bad;
	    }
	    /* check that the load command size is not zero */
	    if(l.cmdsize == 0){
		Mach_O_error(ofile, "malformed object (load command %u cmdsize"
			     " is zero)", i);
		goto return_bad;
	    }
	    switch(l.cmd){
	    case LC_SEGMENT:
		if(l.cmdsize < sizeof(struct segment_command)){
		    Mach_O_error(ofile, "malformed object (LC_SEGMENT cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		sg = (struct segment_command *)lc;
		if(swapped)
		    swap_segment_command(sg, host_byte_sex);
		big_size = sg->nsects;
		big_size *= sizeof(struct section);
		big_size += sizeof(struct segment_command);
		if(sg->cmdsize != big_size){
		    Mach_O_error(ofile, "malformed object (inconsistent "
				 "cmdsize in LC_SEGMENT command %u for the "
				 "number of sections)", i);
		    goto return_bad;
		}
		if(sg->fileoff > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_SEGMENT command %u fileoff field "
				 "extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = sg->fileoff;
		big_size += sg->filesize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_SEGMENT command %u fileoff field "
				 "plus filesize field extends past the end of "
				 "the file)", i);
		    goto return_bad;
		}
		if(sg->vmsize != 0 && sg->filesize > sg->vmsize){
		    Mach_O_error(ofile, "malformed object (LC_SEGMENT command "
				 "%u filesize field greater than vmsize field)",
				 i);
		    goto return_bad;
		}
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		if(swapped)
		    swap_section(s, sg->nsects, host_byte_sex);
		for(j = 0 ; j < sg->nsects ; j++){
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->flags != S_ZEROFILL &&
		       s->flags != S_THREAD_LOCAL_ZEROFILL && s->offset > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(offset field of section %u in LC_SEGMENT "
				"command %u extends past the end of the file)",
				j, i);
			goto return_bad;
		    }
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->flags != S_ZEROFILL &&
		       s->flags != S_THREAD_LOCAL_ZEROFILL &&
		       sg->fileoff == 0 && s->offset < sizeofhdrs &&
		       s->size != 0){
			Mach_O_error(ofile, "malformed object (offset field of "
				"section %u in LC_SEGMENT command %u not "
				"past the headers of the file)", j, i);
			goto return_bad;
		    }
		    big_size = s->offset;
		    big_size += s->size;
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->flags != S_ZEROFILL &&
		       s->flags != S_THREAD_LOCAL_ZEROFILL && big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(offset field plus size field of section %u "
				"in LC_SEGMENT command %u extends "
				"past the end of the file)", j, i);
			goto return_bad;
		    }
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->flags != S_ZEROFILL &&
		       s->flags != S_THREAD_LOCAL_ZEROFILL &&
		       s->size > sg->filesize){
			Mach_O_error(ofile, "malformed object (size field of "
				"section %u in LC_SEGMENT command %u greater "
				"than the segment)", j, i);
			goto return_bad;
		    }
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->size != 0 && s->addr < sg->vmaddr){
			Mach_O_error(ofile, "malformed object (addr field of "
				"section %u in LC_SEGMENT command %u less than "
				"the segment's vmaddr)", j, i);
			goto return_bad;
		    }
		    big_size = s->addr;
		    big_size += s->size;
		    big_end = sg->vmaddr;
		    big_end += sg->vmsize;
		    if(sg->vmsize != 0 && s->size != 0 && big_size > big_end){
			Mach_O_error(ofile, "malformed object (addr field plus "
				"size of section %u in LC_SEGMENT command %u "
				"greater than than the segment's vmaddr plus "
				"vmsize)", j, i);
			goto return_bad;
		    }
		    if(mh->filetype != MH_DYLIB_STUB &&
		       mh->filetype != MH_DSYM &&
		       s->flags != S_ZEROFILL &&
		       s->flags != S_THREAD_LOCAL_ZEROFILL &&
		       check_overlaping_element(ofile, &elements, s->offset,
			    s->size, "section contents") == CHECK_BAD)
			goto return_bad;
		    if(s->reloff > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(reloff field of section %u in LC_SEGMENT "
				"command %u extends past the end of the file)",
				j, i);
			goto return_bad;
		    }
		    big_size = s->nreloc;
		    big_size *= sizeof(struct relocation_info);
		    big_size += s->reloff;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(reloff field plus nreloc field times sizeof("
				"struct relocation_info) of section %u in "
				"LC_SEGMENT command %u extends past the "
				"end of the file)", j, i);
			goto return_bad;
		    }
		    if(check_overlaping_element(ofile, &elements, s->reloff,
			    s->nreloc * sizeof(struct relocation_info),
			    "section relocation entries") == CHECK_BAD)
			goto return_bad;
		    s++;
		}
		break;

	    case LC_SEGMENT_64:
		if(l.cmdsize < sizeof(struct segment_command_64)){
		    Mach_O_error(ofile, "malformed object (LC_SEGMENT_64 "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		sg64 = (struct segment_command_64 *)lc;
		if(swapped)
		    swap_segment_command_64(sg64, host_byte_sex);
		big_size = sg64->nsects;
		big_size *= sizeof(struct section_64);
		big_size += sizeof(struct segment_command_64);
		if(sg64->cmdsize != big_size){
		    Mach_O_error(ofile, "malformed object (inconsistent "
				 "cmdsize in LC_SEGMENT_64 command %u for "
				 "the number of sections)", i);
		    goto return_bad;
		}
		if(sg64->fileoff > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_SEGMENT_64 command %u fileoff field "
				 "extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = sg64->fileoff;
		big_size += sg64->filesize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_SEGMENT_64 command %u fileoff field "
				 "plus filesize field extends past the end of "
				 "the file)", i);
		    goto return_bad;
		}
		s64 = (struct section_64 *)
		    ((char *)sg64 + sizeof(struct segment_command_64));
		if(swapped)
		    swap_section_64(s64, sg64->nsects, host_byte_sex);
		for(j = 0 ; j < sg64->nsects ; j++){
		    if(mh64->filetype != MH_DYLIB_STUB &&
		       mh64->filetype != MH_DSYM &&
		       s64->flags != S_ZEROFILL &&
		       s64->flags != S_THREAD_LOCAL_ZEROFILL &&
		       s64->offset > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(offset field of section %u in LC_SEGMENT_64 "
				"command %u extends past the end of the file)",
				j, i);
			goto return_bad;
		    }
		    if(mh64->filetype != MH_DYLIB_STUB &&
		       mh64->filetype != MH_DSYM &&
		       s64->flags != S_ZEROFILL &&
		       s64->flags != S_THREAD_LOCAL_ZEROFILL &&
		       sg64->fileoff == 0 && s64->offset < sizeofhdrs &&
		       s64->size != 0){
			Mach_O_error(ofile, "malformed object (offset field of "
				"section %u in LC_SEGMENT command %u not "
				"past the headers of the file)", j, i);
			goto return_bad;
		    }
		    big_size = s64->offset;
		    big_size += s64->size;
		    if(mh64->filetype != MH_DYLIB_STUB &&
		       mh64->filetype != MH_DSYM &&
		       s64->flags != S_ZEROFILL &&
		       s64->flags != S_THREAD_LOCAL_ZEROFILL &&
		       big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(offset field plus size field of section %u "
				"in LC_SEGMENT_64 command %u extends "
				"past the end of the file)", j, i);
			goto return_bad;
		    }
		    if(mh64->filetype != MH_DYLIB_STUB &&
		       mh64->filetype != MH_DSYM &&
		       s64->flags != S_ZEROFILL &&
		       s64->flags != S_THREAD_LOCAL_ZEROFILL &&
		       check_overlaping_element(ofile, &elements, s64->offset,
			    (uint32_t)s64->size, "section contents")==CHECK_BAD)
			goto return_bad;
		    if(s64->reloff > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(reloff field of section %u in LC_SEGMENT_64 "
				"command %u extends past the end of the file)",
				j, i);
			goto return_bad;
		    }
		    big_size = s64->nreloc;
		    big_size *= sizeof(struct relocation_info);
		    big_size += s64->reloff;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
				"(reloff field plus nreloc field times sizeof("
				"struct relocation_info) of section %u in "
				"LC_SEGMENT_64 command %u extends past the "
				"end of the file)", j, i);
			goto return_bad;
		    }
		    if(check_overlaping_element(ofile, &elements, s64->reloff,
			    s64->nreloc * sizeof(struct relocation_info),
			    "section relocation entries") == CHECK_BAD)
			goto return_bad;
		    s64++;
		}
		break;

	    case LC_SYMTAB:
		if(l.cmdsize < sizeof(struct symtab_command)){
		    Mach_O_error(ofile, "malformed object (LC_SYMTAB cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		if(st != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_SYMTAB command)");
		    goto return_bad;
		}
		st = (struct symtab_command *)lc;
		if(swapped)
		    swap_symtab_command(st, host_byte_sex);
		if(st->cmdsize != sizeof(struct symtab_command)){
		    Mach_O_error(ofile, "malformed object (LC_SYMTAB command "
			"%u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(st->symoff > size){
		    Mach_O_error(ofile, "truncated or malformed object (symoff "
			"field of LC_SYMTAB command %u extends past the end "
			"of the file)", i);
		    goto return_bad;
		}
		big_size = st->nsyms;
		big_size *= sizeof_nlist;
		big_size += st->symoff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object (symoff "
			"field plus nsyms field times sizeof(%s) of LC_SYMTAB "
			"command %u extends past the end of the file)",
			struct_nlist_name, i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, st->symoff,
			st->nsyms * sizeof_nlist, "symbol table") == CHECK_BAD)
		    goto return_bad;
		if(st->stroff > size){
		    Mach_O_error(ofile, "truncated or malformed object (stroff "
			"field of LC_SYMTAB command %u extends past the end "
			"of the file)", i);
		    goto return_bad;
		}
		big_size = st->stroff;
		big_size += st->strsize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object (stroff "
			"field plus strsize field of LC_SYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, st->stroff,
			st->strsize, "string table") == CHECK_BAD)
		    goto return_bad;
		break;

	    case LC_DYSYMTAB:
		if(l.cmdsize < sizeof(struct dysymtab_command)){
		    Mach_O_error(ofile, "malformed object (LC_DYSYMTAB cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		if(dyst != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_DYSYMTAB command)");
		    goto return_bad;
		}
		dyst = (struct dysymtab_command *)lc;
		if(swapped)
		    swap_dysymtab_command(dyst, host_byte_sex);
		if(dyst->cmdsize != sizeof(struct dysymtab_command)){
		    Mach_O_error(ofile, "malformed object (LC_DYSYMTAB command "
			"%u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(dyst->tocoff > size){
		    Mach_O_error(ofile, "truncated or malformed object (tocoff "
			"field of LC_DYSYMTAB command %u extends past the end "
			"of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->ntoc;
		big_size *= sizeof(struct dylib_table_of_contents);
		big_size += dyst->tocoff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object (tocoff "
			"field plus ntoc field times sizeof(struct dylib_table"
			"_of_contents) of LC_DYSYMTAB command %u extends past "
			"the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, dyst->tocoff,
			dyst->ntoc * sizeof(struct dylib_table_of_contents),
			"table of contents") == CHECK_BAD)
		    goto return_bad;
		if(dyst->modtaboff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(modtaboff field of LC_DYSYMTAB command %u extends "
			"past the end of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->nmodtab;
		big_size *= sizeof_dylib_module;
		big_size += dyst->modtaboff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(modtaboff field plus nmodtab field times sizeof(%s) "
			"of LC_DYSYMTAB command %u extends past the end of "
			"the file)", struct_dylib_module_name, i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, dyst->modtaboff,
			dyst->nmodtab * sizeof_dylib_module, "module table") == 
			CHECK_BAD)
		    goto return_bad;
		if(dyst->extrefsymoff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(extrefsymoff field of LC_DYSYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->nextrefsyms;
		big_size *= sizeof(struct dylib_reference);
		big_size += dyst->extrefsymoff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(extrefsymoff field plus nextrefsyms field times "
			"sizeof(struct dylib_reference) of LC_DYSYMTAB command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements,dyst->extrefsymoff,
			dyst->nextrefsyms * sizeof(struct dylib_reference),
			"reference table") == CHECK_BAD)
		    goto return_bad;
		if(dyst->indirectsymoff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(indirectsymoff field of LC_DYSYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->nindirectsyms;
		big_size *= sizeof(uint32_t);
		big_size += dyst->indirectsymoff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(indirectsymoff field plus nindirectsyms field times "
			"sizeof(uint32_t) of LC_DYSYMTAB command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements,
			dyst->indirectsymoff, dyst->nindirectsyms *
			sizeof(uint32_t), "indirect table") == CHECK_BAD)
		    goto return_bad;
		if(dyst->extreloff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(extreloff field of LC_DYSYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->nextrel;
		big_size *= sizeof(struct relocation_info);
		big_size += dyst->extreloff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(extreloff field plus nextrel field times "
			"sizeof(struct relocation_info) of LC_DYSYMTAB command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, dyst->extreloff,
			dyst->nextrel * sizeof(struct relocation_info),
			"external relocation table") == CHECK_BAD)
		    goto return_bad;
		if(dyst->locreloff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(locreloff field of LC_DYSYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = dyst->nlocrel;
		big_size *= sizeof(struct relocation_info);
		big_size += dyst->locreloff;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(locreloff field plus nlocrel field times "
			"sizeof(struct relocation_info) of LC_DYSYMTAB command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, dyst->locreloff,
			dyst->nlocrel * sizeof(struct relocation_info),
			"local relocation table") == CHECK_BAD)
		    goto return_bad;
		break;

	    case LC_ROUTINES:
		if(l.cmdsize < sizeof(struct routines_command)){
		    Mach_O_error(ofile, "malformed object (LC_ROUTINES cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		if(rc != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_ROUTINES command)");
		    goto return_bad;
		}
		rc = (struct routines_command *)lc;
		if(swapped)
		    swap_routines_command(rc, host_byte_sex);
		if(rc->cmdsize != sizeof(struct routines_command)){
		    Mach_O_error(ofile, "malformed object (LC_ROUTINES "
			"command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		break;

	    case LC_ROUTINES_64:
		if(l.cmdsize < sizeof(struct routines_command_64)){
		    Mach_O_error(ofile, "malformed object (LC_ROUTINES_64 "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		if(rc64 != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_ROUTINES_64 command)");
		    goto return_bad;
		}
		rc64 = (struct routines_command_64 *)lc;
		if(swapped)
		    swap_routines_command_64(rc64, host_byte_sex);
		if(rc64->cmdsize != sizeof(struct routines_command_64)){
		    Mach_O_error(ofile, "malformed object (LC_ROUTINES_64 "
			"command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		break;

	    case LC_TWOLEVEL_HINTS:
		if(l.cmdsize < sizeof(struct twolevel_hints_command)){
		    Mach_O_error(ofile, "malformed object (LC_TWOLEVEL_HINTS "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		if(hints != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_TWOLEVEL_HINTS command)");
		    goto return_bad;
		}
		hints = (struct twolevel_hints_command *)lc;
		if(swapped)
		    swap_twolevel_hints_command(hints, host_byte_sex);
		if(hints->cmdsize != sizeof(struct twolevel_hints_command)){
		    Mach_O_error(ofile, "malformed object (LC_TWOLEVEL_HINTS "
			         "command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(hints->offset > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(offset field of LC_TWOLEVEL_HINTS command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = hints->nhints;
		big_size *= sizeof(struct twolevel_hint);
		big_size += hints->offset;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(offset field plus nhints field times "
			"sizeof(struct twolevel_hint) of LC_TWOLEVEL_HINTS "
			" command %u extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, hints->offset,
			hints->nhints * sizeof(struct twolevel_hint),
			"two level hints") == CHECK_BAD)
		    goto return_bad;
		break;

	    case LC_SEGMENT_SPLIT_INFO:
		cmd_name = "LC_SEGMENT_SPLIT_INFO";
		element_name = "split info data";
		if(split_info != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		split_info = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_CODE_SIGNATURE:
		cmd_name = "LC_CODE_SIGNATURE";
		element_name = "code signature data";
		if(code_sig != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		code_sig = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_FUNCTION_STARTS:
		cmd_name = "LC_FUNCTION_STARTS";
		element_name = "function starts data";
		if(func_starts != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		func_starts = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_DATA_IN_CODE:
		cmd_name = "LC_DATA_IN_CODE";
		element_name = "data in code info";
		if(data_in_code != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		data_in_code = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_DYLIB_CODE_SIGN_DRS:
		cmd_name = "LC_DYLIB_CODE_SIGN_DRS";
		element_name = "dylib codesign designated requirements data";
		if(code_sign_drs != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		code_sign_drs = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_LINKER_OPTIMIZATION_HINT:
		cmd_name = "LC_LINKER_OPTIMIZATION_HINT";
		element_name = "linker optimization hint";
		if(link_opt_hint != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"%s command)", cmd_name);
		    goto return_bad;
		}
		link_opt_hint = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

	    case LC_DYLD_EXPORTS_TRIE:
		cmd_name = "LC_DYLD_EXPORTS_TRIE";
		element_name = "exports trie";
		if(exports_trie != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
				 "%s command)", cmd_name);
		    goto return_bad;
		}
		exports_trie = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;
		
	    case LC_DYLD_CHAINED_FIXUPS:
		cmd_name = "LC_DYLD_CHAINED_FIXUPS";
		element_name = "chained fixups";
		if(chained_fixups != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
				 "%s command)", cmd_name);
		    goto return_bad;
		}
		chained_fixups = (struct linkedit_data_command *)lc;
		goto check_linkedit_data_command;

check_linkedit_data_command:
		if(l.cmdsize < sizeof(struct linkedit_data_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize too "
				 "small) in command %u", cmd_name, i);
		    goto return_bad;
		}
		linkedit_data = (struct linkedit_data_command *)lc;
		if(swapped)
		    swap_linkedit_data_command(linkedit_data, host_byte_sex);
		if(linkedit_data->cmdsize !=
		   sizeof(struct linkedit_data_command)){
		    Mach_O_error(ofile, "malformed object (%s command %u has "
				 "incorrect cmdsize)", cmd_name, i);
		    goto return_bad;
		}
		if(linkedit_data->dataoff > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(dataoff field of %s command %u extends past the end "
			"of the file)", cmd_name, i);
		    goto return_bad;
		}
		big_size = linkedit_data->dataoff;
		big_size += linkedit_data->datasize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(dataoff field plus datasize field of "
			"%s command %u extends past the end of "
			"the file)", cmd_name, i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements,
			linkedit_data->dataoff, linkedit_data->datasize,
			element_name) == CHECK_BAD)
		    goto return_bad;
		break;

	    case LC_VERSION_MIN_MACOSX:
		if(l.cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
				 "MACOSX cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		if(vers != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_VERSION_MIN_MACOSX, LC_VERSION_MIN_IPHONEOS, "
			"LC_VERSION_MIN_TVOS or LC_VERSION_MIN_WATCHOS "
			"load command)");
		    goto return_bad;
		}
		vers = (struct version_min_command *)lc;
		if(swapped)
		    swap_version_min_command(vers, host_byte_sex);
		if(vers->cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
			"MACOSX command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
                if (bv1 != NULL) {
                    if (bv1->platform == PLATFORM_MACOS) {
                        Mach_O_error(ofile, "malformed object (the "
                                     "LC_VERSION_MIN_MACOSX command %u "
                                     "is not allowed when an LC_BUILD_VERSION "
                                     "command for MACOS is present)", i);
                        goto return_bad;
                    }
                }
		break;

	    case LC_VERSION_MIN_IPHONEOS:
		if(l.cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
				 "IPHONEOS cmdsize too small) in command %u",i);
		    goto return_bad;
		}
		if(vers != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_VERSION_MIN_MACOSX, LC_VERSION_MIN_IPHONEOS, "
			"LC_VERSION_MIN_TVOS or LC_VERSION_MIN_WATCHOS "
			"load command)");
		    goto return_bad;
		}
                if (bv1) {
                    Mach_O_error(ofile, "malformed object "
                                 "(LC_VERSION_MIN_IPHONEOS and some "
                                 "LC_BUILD_VERSION load command also found)");
                    goto return_bad;
                }
		vers = (struct version_min_command *)lc;
		if(swapped)
		    swap_version_min_command(vers, host_byte_sex);
		if(vers->cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
			"IPHONEOS command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		break;

	    case LC_VERSION_MIN_TVOS:
		if(l.cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_TVOS"
				 " cmdsize too small) in command %u",i);
		    goto return_bad;
		}
		if(vers != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_VERSION_MIN_MACOSX, LC_VERSION_MIN_IPHONEOS, "
			"LC_VERSION_MIN_TVOS or LC_VERSION_MIN_WATCHOS "
			"load command)");
		    goto return_bad;
		}
                if (bv1) {
                    Mach_O_error(ofile, "malformed object "
                                 "(LC_VERSION_MIN_TVOS and some "
                                 "LC_BUILD_VERSION load command also found)");
                    goto return_bad;
                }
		vers = (struct version_min_command *)lc;
		if(swapped)
		    swap_version_min_command(vers, host_byte_sex);
		if(vers->cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_TVOS"
			" command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		break;

	    case LC_VERSION_MIN_WATCHOS:
		if(l.cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
				 "WATCHOS cmdsize too small) in command %u",i);
		    goto return_bad;
		}
		if(vers != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_VERSION_MIN_MACOSX, LC_VERSION_MIN_IPHONEOS, "
			"LC_VERSION_MIN_TVOS or LC_VERSION_MIN_WATCHOS "
			"command)");
		    goto return_bad;
		}
                if (bv1) {
                    Mach_O_error(ofile, "malformed object "
                                 "(LC_VERSION_MIN_WATCHOS and some "
                                 "LC_BUILD_VERSION load command also found)");
                    goto return_bad;
                }
		vers = (struct version_min_command *)lc;
		if(swapped)
		    swap_version_min_command(vers, host_byte_sex);
		if(vers->cmdsize < sizeof(struct version_min_command)){
		    Mach_O_error(ofile, "malformed object (LC_VERSION_MIN_"
			"WATCHOS command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		break;

	    case LC_BUILD_VERSION:
                /* check load command size */
		if(l.cmdsize < sizeof(struct build_version_command)){
		    Mach_O_error(ofile, "malformed object (LC_BUILD_VERSION"
				 "cmdsize too small) in command %u",i);
		    goto return_bad;
		}
		if(vers != NULL && (vers->cmd != LC_VERSION_MIN_MACOSX)){
		    Mach_O_error(ofile, "malformed object (LC_BUILD_VERSION "
			"and some LC_VERSION_MIN load command also found)");
		    goto return_bad;
		}
		if(bv1 != NULL && bv2 != NULL &&
		   (mh_flags & MH_SIM_SUPPORT) == 0){
		    Mach_O_error(ofile, "malformed object (more than two "
			"LC_BUILD_VERSION load commands)");
		}
		bv = (struct build_version_command *)lc;
		if(swapped)
		    swap_build_version_command(bv, host_byte_sex);
		if(bv->cmdsize < sizeof(struct build_version_command) +
				bv->ntools * sizeof(struct build_tool_version)){
		    Mach_O_error(ofile, "malformed object (LC_BUILD_VERSION"
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		btv = (struct build_tool_version *)
		      ((char *)bv + sizeof(struct build_version_command));
		if(swapped)
		    swap_build_tool_version(btv, bv->ntools, host_byte_sex);
                if (vers != NULL) {
                    if (vers->cmd == LC_VERSION_MIN_MACOSX &&
                        bv->platform != PLATFORM_MACCATALYST) {
                        Mach_O_error(ofile, "malformed object "
                                     "(LC_VERSION_MIN_MACOSX is set but the "
                                     "LC_BUILD_VERSION load command is not for "
                                     "MACCATALYST)");
                        goto return_bad;
                    }
                }
		if(bv1 == NULL) {
		    bv1 = bv;
		    if (((mh_flags & MH_SIM_SUPPORT) != 0) &&
			(bv1->platform != PLATFORM_MACOS &&
			 bv1->platform != PLATFORM_MACCATALYST &&
			 bv1->platform != PLATFORM_IOSSIMULATOR &&
			 bv1->platform != PLATFORM_TVOSSIMULATOR &&
			 bv1->platform != PLATFORM_WATCHOSSIMULATOR))
			Mach_O_error(ofile, "malformed object (the "
			    "LC_BUILD_VERSION, command %u, platform value is "
			    "not allowed when the mach header flag "
			    "MH_SIM_SUPPORT is set)", i);
		}
		else {
		    bv2 = bv;
		    if ((bv1->platform != PLATFORM_MACOS &&
			 bv1->platform != PLATFORM_MACCATALYST) ||
                        ((bv1->platform == PLATFORM_MACOS &&
			  bv2->platform != PLATFORM_MACCATALYST) ||
			 (bv1->platform == PLATFORM_MACCATALYST &&
			  bv2->platform != PLATFORM_MACOS))) {
			if ((mh_flags & MH_SIM_SUPPORT) == 0) {
			    Mach_O_error(ofile, "malformed object (the two "
				"LC_BUILD_VERSION load commands are not for "
				"MACOS and MACCATALYST)");
			}
			else{
			    if(bv2->platform != PLATFORM_MACOS &&
			       bv2->platform != PLATFORM_MACCATALYST &&
			       bv2->platform != PLATFORM_IOSSIMULATOR &&
			       bv2->platform != PLATFORM_TVOSSIMULATOR &&
			       bv2->platform != PLATFORM_WATCHOSSIMULATOR)
			    Mach_O_error(ofile, "malformed object (the "
			        "LC_BUILD_VERSION, command %u, platform value "
			        "is not allowed when the mach header flag "
			        "MH_SIM_SUPPORT is set)", i);
			}
		    }
		}
		break;

	    case LC_ENCRYPTION_INFO:
		if(l.cmdsize < sizeof(struct encryption_info_command)){
		    Mach_O_error(ofile, "malformed object (LC_ENCRYPTION_INFO "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		encrypt_info = (struct encryption_info_command *)lc;
		if(swapped) 
		    swap_encryption_command(encrypt_info, host_byte_sex);
		if(encrypt_info->cmdsize !=
		   sizeof(struct encryption_info_command)){
		    Mach_O_error(ofile, "malformed object (LC_ENCRYPTION_INFO"
				 "command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(encrypt_info->cryptoff > size){
		Mach_O_error(ofile, "truncated or malformed object (cryptoff "
			     "field of LC_ENCRYPTION_INFO command %u extends "
			     "past the end of the file)", i);
		    goto return_bad;
		}
		big_size = encrypt_info->cryptoff;
		big_size += encrypt_info->cryptsize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
				 "(cryptoff field plus cryptsize field of "
				 "LC_ENCRYPTION_INFO command %u extends past "
				 "the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_ENCRYPTION_INFO_64:
		if(l.cmdsize < sizeof(struct encryption_info_command_64)){
		    Mach_O_error(ofile, "malformed object (LC_ENCRYPTION_INFO"
			         "_64 cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		encrypt_info64 = (struct encryption_info_command_64 *)lc;
		if(swapped) 
		    swap_encryption_command_64(encrypt_info64, host_byte_sex);
		if(encrypt_info64->cmdsize !=
		   sizeof(struct encryption_info_command_64)){
		    Mach_O_error(ofile, "malformed object (LC_ENCRYPTION_INFO"
				 "_64 command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(encrypt_info64->cryptoff > size){
		Mach_O_error(ofile, "truncated or malformed object (cryptoff "
			     "field of LC_ENCRYPTION_INFO_64 command %u extends"
			     " past the end of the file)", i);
		    goto return_bad;
		}
		big_size = encrypt_info64->cryptoff;
		big_size += encrypt_info64->cryptsize;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object "
				 "(cryptoff field plus cryptsize field of "
				 "LC_ENCRYPTION_INFO_64 command %u extends past"
				 " the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_LINKER_OPTION:
		if(l.cmdsize < sizeof(struct linker_option_command)){
		    Mach_O_error(ofile, "malformed object (LC_LINKER_OPTION "
			         "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		lo = (struct linker_option_command *)lc;
		if(swapped) 
		    swap_linker_option_command(lo, host_byte_sex);
		if(lo->cmdsize <
		   sizeof(struct linker_option_command)){
		    Mach_O_error(ofile, "malformed object (LC_LINKER_OPTION "
				 " command %u cmdsize too small)", i);
		    goto return_bad;
		}
		break;

	    case LC_DYLD_INFO:
	    case LC_DYLD_INFO_ONLY:
		if(l.cmdsize < sizeof(struct dyld_info_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize "
			         "too small) in command %u", l.cmd ==
				 LC_DYLD_INFO ? "LC_DYLD_INFO" :
				 "LC_DYLD_INFO_ONLY", i);
		    goto return_bad;
		}
		dyld_info = (struct dyld_info_command *)lc;
		if(swapped) 
		    swap_dyld_info_command(dyld_info, host_byte_sex);
		if(dyld_info->cmdsize !=
		   sizeof(struct dyld_info_command)){
		    Mach_O_error(ofile, "malformed object (LC_DYLD_INFO "
				 "command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(dyld_info->rebase_off != 0 && dyld_info->rebase_off > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(rebase_off field of LC_DYLD_INFO command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(dyld_info->rebase_off != 0){
		    big_size = dyld_info->rebase_off;
		    big_size += dyld_info->rebase_size;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
			    "(rebase_off plus rebase_size of LC_DYLD_INFO "
			    "command %u extends past the end of the file)", i);
			goto return_bad;
		    }
		}
		if(check_overlaping_element(ofile, &elements,
			dyld_info->rebase_off, dyld_info->rebase_size,
			"dyld rebase info") == CHECK_BAD)
		    goto return_bad;
		if(dyld_info->bind_off != 0 && dyld_info->bind_off > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(bind_off field of LC_DYLD_INFO command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(dyld_info->bind_off != 0){
		    big_size = dyld_info->bind_off;
		    big_size += dyld_info->bind_size;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
			    "(bind_off plus bind_size of LC_DYLD_INFO command "
			    "%u extends past the end of the file)", i);
			goto return_bad;
		    }
		}
		if(check_overlaping_element(ofile, &elements,
			dyld_info->bind_off, dyld_info->bind_size,
			"dyld bind info") == CHECK_BAD)
		    goto return_bad;
		if(dyld_info->weak_bind_off != 0 &&
		   dyld_info->weak_bind_off > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(weak_bind_off field of LC_DYLD_INFO command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(dyld_info->weak_bind_off != 0){
		    big_size = dyld_info->weak_bind_off;
		    big_size += dyld_info->weak_bind_size;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
			    "(weak_bind_off plus weak_bind_size of LC_DYLD_INFO"
			    " command %u extends past the end of the file)", i);
			goto return_bad;
		    }
		}
		if(check_overlaping_element(ofile, &elements,
			dyld_info->weak_bind_off, dyld_info->weak_bind_size,
			"dyld bind info") == CHECK_BAD)
		    goto return_bad;
		if(dyld_info->lazy_bind_off != 0 &&
		   dyld_info->lazy_bind_off > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(lazy_bind_off field of LC_DYLD_INFO command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(dyld_info->lazy_bind_off != 0){
		    big_size = dyld_info->lazy_bind_off;
		    big_size += dyld_info->lazy_bind_size;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
			    "(lazy_bind_off plus lazy_bind_size of LC_DYLD_INFO"
			    " command %u extends past the end of the file)", i);
			goto return_bad;
		    }
		}
		if(check_overlaping_element(ofile, &elements,
			dyld_info->lazy_bind_off, dyld_info->lazy_bind_size,
			"dyld lazy bind info") == CHECK_BAD)
		    goto return_bad;
		if(dyld_info->export_off != 0 && dyld_info->export_off > size){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(export_off field of LC_DYLD_INFO command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(dyld_info->export_off != 0){
		    big_size = dyld_info->export_off;
		    big_size += dyld_info->export_size;
		    if(big_size > size){
			Mach_O_error(ofile, "truncated or malformed object "
			    "(export_off plus export_size of LC_DYLD_INFO "
			    "command %u extends past the end of the file)", i);
			goto return_bad;
		    }
		}
		if(check_overlaping_element(ofile, &elements,
			dyld_info->export_off, dyld_info->export_size,
			"dyld export info") == CHECK_BAD)
		    goto return_bad;
		break;
		


	    case LC_PREBIND_CKSUM:
		if(l.cmdsize < sizeof(struct prebind_cksum_command)){
		    Mach_O_error(ofile, "malformed object (LC_PREBIND_CKSUM "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		if(cs != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_PREBIND_CKSUM command)");
		    goto return_bad;
		}
		cs = (struct prebind_cksum_command *)lc;
		if(swapped)
		    swap_prebind_cksum_command(cs, host_byte_sex);
		if(cs->cmdsize != sizeof(struct prebind_cksum_command)){
		    Mach_O_error(ofile, "malformed object (LC_PREBIND_CKSUM "
			"command %u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		break;

	    case LC_UUID:
		if(l.cmdsize < sizeof(struct uuid_command)){
		    Mach_O_error(ofile, "malformed object (LC_UUID cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		if(uuid != NULL){
		    Mach_O_error(ofile, "malformed object (more than one "
			"LC_UUID command)");
		    goto return_bad;
		}
		uuid = (struct uuid_command *)lc;
		if(swapped)
		    swap_uuid_command(uuid, host_byte_sex);
		if(uuid->cmdsize != sizeof(struct uuid_command)){
		    Mach_O_error(ofile, "malformed object (LC_UUID command %u "			"has incorrect cmdsize)", i);
		    goto return_bad;
		}
		break;

	    case LC_SYMSEG:
		if(l.cmdsize < sizeof(struct symseg_command)){
		    Mach_O_error(ofile, "malformed object (LC_SYMSEG cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		ss = (struct symseg_command *)lc;
		if(swapped)
		    swap_symseg_command(ss, host_byte_sex);
		if(ss->cmdsize != sizeof(struct symseg_command)){
		    Mach_O_error(ofile, "malformed object (LC_SYMSEG command "
			"%u has incorrect cmdsize)", i);
		    goto return_bad;
		}
		if(ss->offset > size){
		    Mach_O_error(ofile, "truncated or malformed object (offset "
			"field of LC_SYMSEG command %u extends past the end "
			"of the file)", i);
		    goto return_bad;
		}
		big_size = ss->offset;
		big_size += ss->size;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object (offset "
			"field plus size field of LC_SYMTAB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		if(check_overlaping_element(ofile, &elements, ss->offset,
			ss->size, "symseg info") == CHECK_BAD)
		    goto return_bad;
		break;

	    case LC_IDFVMLIB:
	    case LC_LOADFVMLIB:
		if(l.cmdsize < sizeof(struct fvmlib_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize "
			         "too small) in command %u", l.cmd ==
				 LC_IDFVMLIB ? "LC_IDFVMLIB" :
				 "LC_LOADFVMLIB", i);
		    goto return_bad;
		}
		fl = (struct fvmlib_command *)lc;
		if(swapped)
		    swap_fvmlib_command(fl, host_byte_sex);
		if(fl->cmdsize < sizeof(struct fvmlib_command)){
		    Mach_O_error(ofile, "malformed object (%s command %u has "
			"too small cmdsize field)", fl->cmd == LC_IDFVMLIB ? 
			"LC_IDFVMLIB" : "LC_LOADFVMLIB", i);
		    goto return_bad;
		}
		if(fl->fvmlib.name.offset >= fl->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (name."
			"offset field of %s command %u extends past the end "
			"of the file)", fl->cmd == LC_IDFVMLIB ? "LC_IDFVMLIB"
			: "LC_LOADFVMLIB", i);
		    goto return_bad;
		}
		break;

	    case LC_ID_DYLIB:
		cmd_name = "LC_ID_DYLIB";
		goto check_dylib_command;
	    case LC_LOAD_DYLIB:
		cmd_name = "LC_LOAD_DYLIB";
		goto check_dylib_command;
	    case LC_LOAD_WEAK_DYLIB:
		cmd_name = "LC_LOAD_WEAK_DYLIB";
		goto check_dylib_command;
	    case LC_REEXPORT_DYLIB:
		cmd_name = "LC_REEXPORT_DYLIB";
		goto check_dylib_command;
	    case LC_LOAD_UPWARD_DYLIB:
		cmd_name = "LC_LOAD_UPWARD_DYLIB";
		goto check_dylib_command;
	    case LC_LAZY_LOAD_DYLIB:
		cmd_name = "LC_LAZY_LOAD_DYLIB";
		goto check_dylib_command;
check_dylib_command:
		if(l.cmdsize < sizeof(struct dylib_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize too "
				 "small) in command %u", cmd_name, i);
		    goto return_bad;
		}
		dl = (struct dylib_command *)lc;
		if(swapped)
		    swap_dylib_command(dl, host_byte_sex);
		if(dl->cmdsize < sizeof(struct dylib_command)){
		    Mach_O_error(ofile, "malformed object (%s command %u has "
			"too small cmdsize field)", cmd_name, i);
		    goto return_bad;
		}
		if(dl->dylib.name.offset >= dl->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (name."
			"offset field of %s command %u extends past the end "
			"of the file)", cmd_name, i);
		    goto return_bad;
		}
		break;

	    case LC_SUB_FRAMEWORK:
		if(l.cmdsize < sizeof(struct sub_framework_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_FRAMEWORK "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		sub = (struct sub_framework_command *)lc;
		if(swapped)
		    swap_sub_framework_command(sub, host_byte_sex);
		if(sub->cmdsize < sizeof(struct sub_framework_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_FRAMEWORK "
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(sub->umbrella.offset >= sub->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(umbrella.offset field of LC_SUB_FRAMEWORK command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_SUB_UMBRELLA:
		if(l.cmdsize < sizeof(struct sub_umbrella_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_UMBRELLA "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		usub = (struct sub_umbrella_command *)lc;
		if(swapped)
		    swap_sub_umbrella_command(usub, host_byte_sex);
		if(usub->cmdsize < sizeof(struct sub_umbrella_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_UMBRELLA "
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(usub->sub_umbrella.offset >= usub->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(sub_umbrella.offset field of LC_SUB_UMBRELLA command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_SUB_LIBRARY:
		if(l.cmdsize < sizeof(struct sub_library_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_LIBRARY "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		lsub = (struct sub_library_command *)lc;
		if(swapped)
		    swap_sub_library_command(lsub, host_byte_sex);
		if(lsub->cmdsize < sizeof(struct sub_library_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_LIBRARY "
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(lsub->sub_library.offset >= lsub->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(sub_library.offset field of LC_SUB_LIBRARY command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_SUB_CLIENT:
		if(l.cmdsize < sizeof(struct sub_client_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_CLIENT "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		csub = (struct sub_client_command *)lc;
		if(swapped)
		    swap_sub_client_command(csub, host_byte_sex);
		if(csub->cmdsize < sizeof(struct sub_client_command)){
		    Mach_O_error(ofile, "malformed object (LC_SUB_CLIENT "
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(csub->client.offset >= csub->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(cleient.offset field of LC_SUB_CLIENT command "
			"%u extends past the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_PREBOUND_DYLIB:
		if(l.cmdsize < sizeof(struct prebound_dylib_command)){
		    Mach_O_error(ofile, "malformed object (LC_PREBOUND_DYLIB "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		pbdylib = (struct prebound_dylib_command *)lc;
		if(swapped)
		    swap_prebound_dylib_command(pbdylib, host_byte_sex);
		if(pbdylib->cmdsize < sizeof(struct dylib_command)){
		    Mach_O_error(ofile, "malformed object (LC_PREBIND_DYLIB "
			"command %u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(pbdylib->name.offset >= pbdylib->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (name."
			"offset field of LC_PREBIND_DYLIB command %u extends "
			"past the end of the file)", i);
		    goto return_bad;
		}
		if(pbdylib->linked_modules.offset >= pbdylib->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (linked_"
			"modules.offset field of LC_PREBIND_DYLIB command %u "
			"extends past the end of the file)", i);
		    goto return_bad;
		}
		break;

	    case LC_ID_DYLINKER:
		cmd_name = "LC_ID_DYLINKER";
		goto check_dylinker_command;
	    case LC_LOAD_DYLINKER:
		cmd_name = "LC_LOAD_DYLINKER";
		goto check_dylinker_command;
	    case LC_DYLD_ENVIRONMENT:
		cmd_name = "LC_DYLD_ENVIRONMENT";
		goto check_dylinker_command;
check_dylinker_command:
		if(l.cmdsize < sizeof(struct dylinker_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize "
			         "too small) in command %u", cmd_name, i);
		    goto return_bad;
		}
		dyld = (struct dylinker_command *)lc;
		if(swapped)
		    swap_dylinker_command(dyld, host_byte_sex);
		if(dyld->cmdsize < sizeof(struct dylinker_command)){
		    Mach_O_error(ofile, "malformed object (%s command %u has "
			"too small cmdsize field)", cmd_name, i);
		    goto return_bad;
		}
		if(dyld->name.offset >= dyld->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (name."
			"offset field of %s command %u extends past the end "
			"of the file)", cmd_name, i);
		    goto return_bad;
		}
		break;

	    case LC_UNIXTHREAD:
	    case LC_THREAD:
		if(l.cmdsize < sizeof(struct thread_command)){
		    Mach_O_error(ofile, "malformed object (%s cmdsize "
			         "too small) in command %u", l.cmd ==
				 LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				 "LC_THREAD", i);
		    goto return_bad;
		}
		ut = (struct thread_command *)lc;
		if(swapped)
		    swap_thread_command(ut, host_byte_sex);
		state = (char *)ut + sizeof(struct thread_command);

	    	if(cputype == CPU_TYPE_I386){
		    i386_thread_state_t *cpu;
/* current i386 thread states */
#if i386_THREAD_STATE == 1
		    struct i386_float_state *fpu;
		    i386_exception_state_t *exc;
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
		    i386_thread_fpstate_t *fpu;
		    i386_thread_exceptstate_t *exc;
		    i386_thread_cthreadstate_t *user;
#endif /* i386_THREAD_STATE == -1 */

		    nflavor = 0;
		    p = (char *)ut + ut->cmdsize;
		    while(state < p){
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (flavor in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			flavor = *((uint32_t *)state);
			if(swapped){
			    flavor = SWAP_INT(flavor);
			    *((uint32_t *)state) = flavor;
			}
			state += sizeof(uint32_t);
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (count in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			count = *((uint32_t *)state);
			if(swapped){
			    count = SWAP_INT(count);
			    *((uint32_t *)state) = count;
			}
			state += sizeof(uint32_t);
			switch((int)flavor){
			case i386_THREAD_STATE:
#if i386_THREAD_STATE == 1
			case -1:
#endif /* i386_THREAD_STATE == 1 */
/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
			case 1:
#endif /* i386_THREAD_STATE == -1 */
			    if(count != i386_THREAD_STATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not i386_THREAD_STATE_COUNT for flavor "
				    "number %u which is a i386_THREAD_STATE "
				    "flavor in %s command %u)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    cpu = (i386_thread_state_t *)state;
			    if(state + sizeof(i386_thread_state_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_THREAD_STATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_thread_state(cpu, host_byte_sex);
			    state += sizeof(i386_thread_state_t);
			    break;
/* current i386 thread states */
#if i386_THREAD_STATE == 1
			case i386_FLOAT_STATE:
			    if(count != i386_FLOAT_STATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not i386_FLOAT_STATE_COUNT for flavor "
				    "number %u which is a i386_FLOAT_STATE "
				    "flavor in %s command %u)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    fpu = (struct i386_float_state *)state;
			    if(state + sizeof(struct i386_float_state) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_FLOAT_STATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_float_state(fpu, host_byte_sex);
			    state += sizeof(struct i386_float_state);
			    break;
			case i386_EXCEPTION_STATE:
			    if(count != I386_EXCEPTION_STATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not I386_EXCEPTION_STATE_COUNT for "
				    "flavor number %u which is a i386_"
				    "EXCEPTION_STATE flavor in %s command %u)",
				    nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    exc = (i386_exception_state_t *)state;
			    if(state + sizeof(i386_exception_state_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_EXCEPTION_STATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_exception_state(exc,host_byte_sex);
			    state += sizeof(i386_exception_state_t);
			    break;
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
			case i386_THREAD_FPSTATE:
			    if(count != i386_THREAD_FPSTATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not i386_THREAD_FPSTATE_COUNT for flavor "
				    "number %u which is a i386_THREAD_FPSTATE "
				    "flavor in %s command %u)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    fpu = (i386_thread_fpstate_t *)state;
			    if(state + sizeof(i386_thread_fpstate_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_THREAD_FPSTATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_thread_fpstate(fpu, host_byte_sex);
			    state += sizeof(i386_thread_fpstate_t);
			    break;
			case i386_THREAD_EXCEPTSTATE:
			    if(count != i386_THREAD_EXCEPTSTATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not i386_THREAD_EXCEPTSTATE_COUNT for "
				    "flavor number %u which is a i386_THREAD_"
				    "EXCEPTSTATE flavor in %s command %u)",
				    nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    exc = (i386_thread_exceptstate_t *)state;
			    if(state + sizeof(i386_thread_exceptstate_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_THREAD_EXCEPTSTATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_thread_exceptstate(exc,host_byte_sex);
			    state += sizeof(i386_thread_exceptstate_t);
			    break;
			case i386_THREAD_CTHREADSTATE:
			    if(count != i386_THREAD_CTHREADSTATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not i386_THREAD_CTHREADSTATE_COUNT for "
				    "flavor number %u which is a i386_THREAD_"
				    "CTHREADSTATE flavor in %s command %u)",
				    nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    user = (i386_thread_cthreadstate_t *)state;
			    if(state + sizeof(i386_thread_cthreadstate_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "i386_THREAD_CTHREADSTATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_i386_thread_cthreadstate(user,
							      host_byte_sex);
			    state += sizeof(i386_thread_cthreadstate_t);
			    break;
#endif /* i386_THREAD_STATE == -1 */
			default:
			    if(swapped){
				Mach_O_error(ofile, "malformed object (unknown "
				    "flavor for flavor number %u in %s command"
				    " %u can't byte swap it)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    state += count * sizeof(uint32_t);
			    break;
			}
			nflavor++;
		    }
		    break;
		}
#ifdef x86_THREAD_STATE64_COUNT
	    	if(cputype == CPU_TYPE_X86_64){
		    x86_thread_state64_t *cpu;

		    nflavor = 0;
		    p = (char *)ut + ut->cmdsize;
		    while(state < p){
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (flavor in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			flavor = *((uint32_t *)state);
			if(swapped){
			    flavor = SWAP_INT(flavor);
			    *((uint32_t *)state) = flavor;
			}
			state += sizeof(uint32_t);
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (count in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			count = *((uint32_t *)state);
			if(swapped){
			    count = SWAP_INT(count);
			    *((uint32_t *)state) = count;
			}
			state += sizeof(uint32_t);
			switch(flavor){
			case x86_THREAD_STATE64:
			    if(count != x86_THREAD_STATE64_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not x86_THREAD_STATE64_COUNT for "
				    "flavor number %u which is a x86_THREAD_"
				    "STATE64 flavor in %s command %u)",
				    nflavor, ut->cmd == LC_UNIXTHREAD ? 
				    "LC_UNIXTHREAD" : "LC_THREAD", i);
				goto return_bad;
			    }
			    cpu = (x86_thread_state64_t *)state;
			    if(state + sizeof(x86_thread_state64_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "x86_THREAD_STATE64 in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_x86_thread_state64(cpu, host_byte_sex);
			    state += sizeof(x86_thread_state64_t);
			    break;
			default:
			    if(swapped){
				Mach_O_error(ofile, "malformed object (unknown "
				    "flavor for flavor number %u in %s command"
				    " %u can't byte swap it)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    state += count * sizeof(uint32_t);
			    break;
			}
			nflavor++;
		    }
		    break;
		}
#endif /* x86_THREAD_STATE64_COUNT */
	    	if(cputype == CPU_TYPE_ARM){
		    arm_thread_state_t *cpu;

		    nflavor = 0;
		    p = (char *)ut + ut->cmdsize;
		    while(state < p){
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (flavor in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			flavor = *((uint32_t *)state);
			if(swapped){
			    flavor = SWAP_INT(flavor);
			    *((uint32_t *)state) = flavor;
			}
			state += sizeof(uint32_t);
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (count in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			count = *((uint32_t *)state);
			if(swapped){
			    count = SWAP_INT(count);
			    *((uint32_t *)state) = count;
			}
			state += sizeof(uint32_t);
			switch(flavor){
			case ARM_THREAD_STATE:
			    if(count != ARM_THREAD_STATE_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not ARM_THREAD_STATE_COUNT for "
				    "flavor number %u which is a ARM_THREAD_"
				    "STATE flavor in %s command %u)",
				    nflavor, ut->cmd == LC_UNIXTHREAD ? 
				    "LC_UNIXTHREAD" : "LC_THREAD", i);
				goto return_bad;
			    }
			    cpu = (arm_thread_state_t *)state;
			    if(state + sizeof(arm_thread_state_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "ARM_THREAD_STATE in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_arm_thread_state_t(cpu, host_byte_sex);
			    state += sizeof(arm_thread_state_t);
			    break;
			default:
			    if(swapped){
				Mach_O_error(ofile, "malformed object (unknown "
				    "flavor for flavor number %u in %s command"
				    " %u can't byte swap it)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    state += count * sizeof(uint32_t);
			    break;
			}
			nflavor++;
		    }
		    break;
		}
	    	if(cputype == CPU_TYPE_ARM64 || cputype == CPU_TYPE_ARM64_32){
		    arm_thread_state64_t *cpu;

		    nflavor = 0;
		    p = (char *)ut + ut->cmdsize;
		    while(state < p){
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (flavor in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			flavor = *((uint32_t *)state);
			if(swapped){
			    flavor = SWAP_INT(flavor);
			    *((uint32_t *)state) = flavor;
			}
			state += sizeof(uint32_t);
			if(state +  sizeof(uint32_t) >
			   (char *)ut + ut->cmdsize){
			    Mach_O_error(ofile, "malformed object (count in "
				"%s command %u extends past end of command)",
				ut->cmd == LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				"LC_THREAD", i);
			    goto return_bad;
			}
			count = *((uint32_t *)state);
			if(swapped){
			    count = SWAP_INT(count);
			    *((uint32_t *)state) = count;
			}
			state += sizeof(uint32_t);
			switch(flavor){
			case ARM_THREAD_STATE64:
			    if(count != ARM_THREAD_STATE64_COUNT){
				Mach_O_error(ofile, "malformed object (count "
				    "not ARM_THREAD_STATE64_COUNT for "
				    "flavor number %u which is a ARM_THREAD_"
				    "STATE64 flavor in %s command %u)",
				    nflavor, ut->cmd == LC_UNIXTHREAD ? 
				    "LC_UNIXTHREAD" : "LC_THREAD", i);
				goto return_bad;
			    }
			    cpu = (arm_thread_state64_t *)state;
			    if(state + sizeof(arm_thread_state64_t) >
			       (char *)ut + ut->cmdsize){
				Mach_O_error(ofile, "malformed object ("
				    "ARM_THREAD_STATE64 in %s command %u "
				    "extends past end of command)", ut->cmd ==
				    LC_UNIXTHREAD ?  "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    if(swapped)
				swap_arm_thread_state64_t(cpu, host_byte_sex);
			    state += sizeof(arm_thread_state64_t);
			    break;
			default:
			    if(swapped){
				Mach_O_error(ofile, "malformed object (unknown "
				    "flavor for flavor number %u in %s command"
				    " %u can't byte swap it)", nflavor,
				    ut->cmd == LC_UNIXTHREAD ? "LC_UNIXTHREAD" :
				    "LC_THREAD", i);
				goto return_bad;
			    }
			    state += count * sizeof(uint32_t);
			    break;
			}
			nflavor++;
		    }
		    break;
		}
		if(swapped){
		    Mach_O_error(ofile, "malformed object (unknown cputype and "
			"cpusubtype of object and can't byte swap and check %s "
			"command %u)", ut->cmd == LC_UNIXTHREAD ?
			"LC_UNIXTHREAD" : "LC_THREAD", i);
		    goto return_bad;
		}
		break;
	    case LC_MAIN:
		if(l.cmdsize < sizeof(struct entry_point_command)){
		    Mach_O_error(ofile, "malformed object (LC_MAIN cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		ep = (struct entry_point_command *)lc;
		if(swapped)
		    swap_entry_point_command(ep, host_byte_sex);
		/*
		 * If we really wanted we could check that the entryoff field
		 * really is an offset into the __TEXT segment.  But since it
		 * is not used here, we won't needlessly check it.
		 */
		break;
	    case LC_SOURCE_VERSION:
		if(l.cmdsize < sizeof(struct source_version_command)){
		    Mach_O_error(ofile, "malformed object (LC_SOURCE_VERSION "
				 "cmdsize too small) in command %u", i);
		    goto return_bad;
		}
		sv = (struct source_version_command *)lc;
		if(swapped)
		    swap_source_version_command(sv, host_byte_sex);
	    case LC_IDENT:
		if(l.cmdsize < sizeof(struct ident_command)){
		    Mach_O_error(ofile, "malformed object (LC_IDENT cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		id = (struct ident_command *)lc;
		if(swapped)
		    swap_ident_command(id, host_byte_sex);
		/*
		 * Note the cmdsize field if the LC_IDENT command was checked
		 * as part of checking all load commands cmdsize field before
		 * the switch statement on the cmd field of the load command.
		 */
		break;
	    case LC_RPATH:
		if(l.cmdsize < sizeof(struct rpath_command)){
		    Mach_O_error(ofile, "malformed object (LC_RPATH: cmdsize "
			         "too small) in command %u", i);
		    goto return_bad;
		}
		rpath = (struct rpath_command *)lc;
		if(swapped)
		    swap_rpath_command(rpath, host_byte_sex);
		if(rpath->cmdsize < sizeof(struct rpath_command)){
		    Mach_O_error(ofile, "malformed object (LC_RPATH command "
			"%u has too small cmdsize field)", i);
		    goto return_bad;
		}
		if(rpath->path.offset >= rpath->cmdsize){
		    Mach_O_error(ofile, "truncated or malformed object (path."
			"offset field of LC_RPATH command %u extends past the "
			"end of the file)", i);
		    goto return_bad;
		}
		break;
	    case LC_NOTE:
		if(l.cmdsize != sizeof(struct note_command)){
		    Mach_O_error(ofile, "malformed object (LC_NOTE: cmdsize "
			         "incorrect) in command %u", i);
		    goto return_bad;
		}
		nc = (struct note_command *)lc;
		if(swapped)
		    swap_note_command(nc, host_byte_sex);
		if(nc->offset > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_NOTE command %u offset field "
				 "extends past the end of the file)", i);
		    goto return_bad;
		}
		big_size = nc->offset;
		big_size += nc->size;
		if(big_size > size){
		    Mach_O_error(ofile, "truncated or malformed object ("
				 "LC_NOTE command %u offset field "
				 "plus size field extends past the end of "
				 "the file)", i);
		    goto return_bad;
		}
		break;

#ifndef OFI
	    default:
		Mach_O_error(ofile, "malformed object (unknown load command "
			     "%u)", i);
		goto return_bad;
#endif /* !defined(OFI) */
	    }

	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    /* check that next load command does not extends past the end */
	    if((char *)lc > (char *)load_commands + sizeofcmds){
		Mach_O_error(ofile, "truncated or malformed object (load "
			     "command %u extends past the end of the file)",
			     i + 1);
		goto return_bad;
	    }
	}
	if(st == NULL){
	    if(dyst != NULL){
		Mach_O_error(ofile, "truncated or malformed object (contains "
		  "LC_DYSYMTAB load command without a LC_SYMTAB load command)");
		goto return_bad;
	    }
	}
	else{
	    if(dyst != NULL){
		if(dyst->nlocalsym != 0 &&
		   dyst->ilocalsym > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(ilocalsym in LC_DYSYMTAB load command extends past "
			"the end of the symbol table)");
		    goto return_bad;
		}
		big_size = dyst->ilocalsym;
		big_size += dyst->nlocalsym;
		if(dyst->nlocalsym != 0 && big_size > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(ilocalsym plus nlocalsym in LC_DYSYMTAB load command "
			"extends past the end of the symbol table)");
		    goto return_bad;
		}

		if(dyst->nextdefsym != 0 &&
		   dyst->iextdefsym > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(iextdefsym in LC_DYSYMTAB load command extends past "
			"the end of the symbol table)");
		    goto return_bad;
		}
		big_size = dyst->iextdefsym;
		big_size += dyst->nextdefsym;
		if(dyst->nextdefsym != 0 && big_size > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(iextdefsym plus nextdefsym in LC_DYSYMTAB load "
			"command extends past the end of the symbol table)");
		    goto return_bad;
		}

		if(dyst->nundefsym != 0 &&
		   dyst->iundefsym > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(iundefsym in LC_DYSYMTAB load command extends past "
			"the end of the symbol table)");
		    goto return_bad;
		}
		big_size = dyst->iundefsym;
		big_size += dyst->nundefsym;
		if(dyst->nundefsym != 0 && big_size > st->nsyms){
		    Mach_O_error(ofile, "truncated or malformed object "
			"(iundefsym plus nundefsym in LC_DYSYMTAB load command "
			"extends past the end of the symbol table)");
		    goto return_bad;
		}
		if(rc != NULL){
		    if(rc->init_module > dyst->nmodtab){
			Mach_O_error(ofile, "malformed object (init_module in "
			    "LC_ROUTINES load command extends past the "
			    "end of the module table)");
			goto return_bad;
		    }
		}
		if(rc64 != NULL){
		    if(rc64->init_module > dyst->nmodtab){
			Mach_O_error(ofile, "malformed object (init_module in "
			    "LC_ROUTINES_64 load command extends past the "
			    "end of the module table)");
			goto return_bad;
		    }
		}
		if(hints != NULL){
		    if(hints->nhints != dyst->nundefsym){
			Mach_O_error(ofile, "malformed object (nhints in "
			    "LC_TWOLEVEL_HINTS load command not the same as "
			    "nundefsym in LC_DYSYMTAB load command)");
			goto return_bad;
		    }
		}
	    }
	}
	/* check for an inconsistent size of the load commands */
	if((char *)load_commands + sizeofcmds != (char *)lc){
	    Mach_O_error(ofile, "malformed object (inconsistent sizeofcmds "
			 "field in mach header)");
	    goto return_bad;
	}

	/*
	 * Mark this ofile so we know its headers have been swapped.  We do this
	 * in case we don't process it the first time so we can swap them back
	 * in case we loop back to it in a fat file to process it later.
	 */
	if(swapped == TRUE)
	    ofile->headers_swapped = TRUE;

	/* looks good return ok */
	free_elements(&elements);
	return(CHECK_GOOD);

return_bad:
	free_elements(&elements);
	return(CHECK_BAD);
#endif /* OTOOL */
}

/*
 * swap_back_Mach_O() is called after the ofile has been processed to swap back
 * the mach header and load commands if check_Mach_O() above swapped them.
 */
static
void
swap_back_Mach_O(
struct ofile *ofile)
{
	if(ofile->headers_swapped == TRUE){
	    ofile->headers_swapped = FALSE;
	    if(ofile->mh != NULL)
		swap_object_headers(ofile->mh, ofile->load_commands);
	    else if(ofile->mh64 != NULL)
		swap_object_headers(ofile->mh64, ofile->load_commands);
	}
}

#ifndef OTOOL
/*
 * check_overlaping_element() checks that the element in the ofile described by
 * offset, size and name does not overlap in list of elements in head.  If it
 * does CHECK_BAD is returned and an error message is generated.  If it doesn't
 * then an element is added in the ordered list and CHECK_GOOD is returned
 */
static
enum check_type
check_overlaping_element(
struct ofile *ofile,
struct element *head,
uint32_t offset,
uint32_t size,
char *name)
{
    struct element *e, *p, *n;

	if(size == 0)
	    return(CHECK_GOOD);

	if(head->next == NULL){
	    n = allocate(sizeof(struct element));
	    n->offset = offset;
	    n->size = size;
	    n->name = name;
	    n->next = NULL;
	    head->next = n;
	    return(CHECK_GOOD);
	}

	p = NULL;
	e = head;
	while(e->next != NULL){
	    p = e;
	    e = e->next;
	    if((offset >= e->offset &&
		offset < e->offset + e->size) ||
	       (offset + size > e->offset &&
		offset + size < e->offset + e->size) ||
	       (offset <= e->offset &&
		offset + size >= e->offset + e->size)){
		Mach_O_error(ofile, "malformed object (%s at offset %u with a "
		    "size of %u, overlaps %s at offset %u with a size of %u)",
		    name, offset, size, e->name, e->offset, e->size);
		return(CHECK_BAD);
	    }
	    if(e->next != NULL && offset + size <= e->next->offset){
		n = allocate(sizeof(struct element));
		n->offset = offset;
		n->size = size;
		n->name = name;
		n->next = e;
		p->next = n;
		return(CHECK_GOOD);
	    }
	}
	n = allocate(sizeof(struct element));
	n->offset = offset;
	n->size = size;
	n->name = name;
	n->next = NULL;
	e->next = n;
	return(CHECK_GOOD);
}

/*
 * free_elements() frees the list of elements on the list head.
 */
static
void
free_elements(
struct element *head)
{
    struct element *e, *e_next;

	e = head->next;
	while(e != NULL){
	    e_next = e->next;
	    free(e);
	    e = e_next;
	}
}
#endif /* !defined(OTOOL) */

/*
 * check_dylib_module() checks the object file's dylib_module as referenced
 * by the dylib_module field in the ofile for correctness.
 */
static
enum check_type
check_dylib_module(
struct ofile *ofile,
struct symtab_command *st,
struct dysymtab_command *dyst,
char *strings,
uint32_t module_index)
{
#ifdef OTOOL
	return(CHECK_GOOD);
#else /* !defined OTOOL */
    uint32_t i;
    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct dylib_module m;
    struct dylib_module_64 m64;
    uint32_t module_name, nextdefsym, iextdefsym, nlocalsym, ilocalsym, nrefsym;
    uint32_t irefsym, nextrel, iextrel;

	host_byte_sex = get_host_byte_sex();
	swapped = (enum bool)(host_byte_sex != ofile->object_byte_sex);
	if(ofile->mh != NULL){
	    m = *ofile->dylib_module;
	    if(swapped)
		swap_dylib_module(&m, 1, host_byte_sex);
	    module_name = m.module_name;
	    nextdefsym = m.nextdefsym;
	    iextdefsym = m.iextdefsym;
	    nlocalsym = m.nlocalsym;
	    ilocalsym = m.ilocalsym;
	    nrefsym = m.nrefsym;
	    irefsym = m.irefsym;
	    nextrel = m.nextrel;
	    iextrel = m.iextrel;
	}
	else{
	    m64 = *ofile->dylib_module64;
	    if(swapped)
		swap_dylib_module_64(&m64, 1, host_byte_sex);
	    module_name = m64.module_name;
	    nextdefsym = m64.nextdefsym;
	    iextdefsym = m64.iextdefsym;
	    nlocalsym = m64.nlocalsym;
	    ilocalsym = m64.ilocalsym;
	    nrefsym = m64.nrefsym;
	    irefsym = m64.irefsym;
	    nextrel = m64.nextrel;
	    iextrel = m64.iextrel;
	}

	if(module_name > st->strsize){
	    Mach_O_error(ofile, "truncated or malformed object (module_name "
		"of module table entry %u past the end of the string table)",
		module_index);
	    return(CHECK_BAD);
	}
	for(i = module_name; i < st->strsize && strings[i] != '\0'; i++)
		;
	if(i >= st->strsize){
	    Mach_O_error(ofile, "truncated or malformed object (module_name "
		"of module table entry %u extends past the end of the string "
		"table)", module_index);
	    return(CHECK_BAD);
	}

	if(nextdefsym != 0){
	    if(iextdefsym > st->nsyms){
		Mach_O_error(ofile, "truncated or malformed object (iextdefsym "
		    "field of module table entry %u past the end of the "
		    "symbol table", module_index);
		return(CHECK_BAD);
	    }
	    if(iextdefsym + nextdefsym > st->nsyms){
		Mach_O_error(ofile, "truncated or malformed object (iextdefsym "
		    "field of module table entry %u plus nextdefsym field "
		    "extends past the end of the symbol table", module_index);
		return(CHECK_BAD);
	    }
	}
	if(nlocalsym != 0){
	    if(ilocalsym > st->nsyms){
		Mach_O_error(ofile, "truncated or malformed object (ilocalsym "
		    "field of module table entry %u past the end of the "
		    "symbol table", module_index);
		return(CHECK_BAD);
	    }
	    if(ilocalsym + nlocalsym > st->nsyms){
		Mach_O_error(ofile, "truncated or malformed object (ilocalsym "
		    "field of module table entry %u plus nlocalsym field "
		    "extends past the end of the symbol table", module_index);
		return(CHECK_BAD);
	    }
	}
	if(nrefsym != 0){
	    if(irefsym > dyst->nextrefsyms){
		Mach_O_error(ofile, "truncated or malformed object (irefsym "
		    "field of module table entry %u past the end of the "
		    "reference table", module_index);
		return(CHECK_BAD);
	    }
	    if(irefsym + nrefsym > dyst->nextrefsyms){
		Mach_O_error(ofile, "truncated or malformed object (irefsym "
		    "field of module table entry %u plus nrefsym field "
		    "extends past the end of the reference table",module_index);
		return(CHECK_BAD);
	    }
	}
	if(nextrel != 0){
	    if(iextrel > dyst->extreloff){
		Mach_O_error(ofile, "truncated or malformed object (iextrel "
		    "field of module table entry %u past the end of the "
		    "external relocation enrties", module_index);
		return(CHECK_BAD);
	    }
	    if(iextrel + nextrel > dyst->extreloff){
		Mach_O_error(ofile, "truncated or malformed object (iextrel "
		    "field of module table entry %u plus nextrel field "
		    "extends past the end of the external relocation enrties",
		    module_index);
		return(CHECK_BAD);
	    }
	}
	return(CHECK_GOOD);
#endif /* OTOOL */
}

__private_extern__
uint32_t
size_ar_name(
const struct ar_hdr *ar_hdr)
{
    int32_t i;

	i = sizeof(ar_hdr->ar_name) - 1;
	if(ar_hdr->ar_name[i] == ' '){
	    do{
		if(ar_hdr->ar_name[i] != ' ')
		    break;
		i--;
	    }while(i > 0);
	}
	/*
	 * For System V archives names ends in a '/' which are not part of the
	 * name. Except for the table of contents member named "/" and archive
	 * string table member name which has the name "//".
	 */
	if(i > 1 && ar_hdr->ar_name[i] == '/')
	    i--;
	return(i + 1);
}
#endif /* !defined(RLD) */
