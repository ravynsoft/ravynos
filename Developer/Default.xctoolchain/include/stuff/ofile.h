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
/* ofile.h */
#ifndef _STUFF_OFILE_H_
#define _STUFF_OFILE_H_

#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

#import <ar.h>
#ifndef AR_EFMT1
#define	AR_EFMT1	"#1/"		/* extended format #1 */
#endif
#import <mach-o/loader.h>
#ifdef OFI
#import <mach-o/dyld.h>
#endif
#import "stuff/bytesex.h"
#import "stuff/bool.h"
#import "stuff/arch.h"

enum ofile_type {
    OFILE_UNKNOWN,
    OFILE_FAT,
    OFILE_ARCHIVE,
    OFILE_Mach_O
#ifdef LTO_SUPPORT
    ,
    OFILE_LLVM_BITCODE
#endif /* LTO_SUPPORT */
};

/*
 * The structure used by ofile_*() routines for object files.
 */
struct ofile {
    char *file_name;		    /* pointer to name malloc'ed by ofile_map */
    char *file_addr;		    /* pointer to vm_allocate'ed memory       */
    uint64_t file_size;	    	    /* size of vm_allocate'ed memory	      */
    uint64_t file_mtime;	    /* stat(2)'s mtime                        */
    enum ofile_type file_type;	    /* type of the file			      */

    struct fat_header *fat_header;  /* If a fat file these are filled in and */
    struct fat_arch *fat_archs;     /*  if needed converted to host byte sex */
    struct fat_arch_64 *fat_archs64;

    /* If this is a fat file then these are valid and filled in */
    uint32_t narch;	    	    /* the current architecture */
    enum ofile_type arch_type;	    /* the type of file for this arch. */
    struct arch_flag arch_flag;     /* the arch_flag for this arch, the name */
				    /*  field is pointing at space malloc'ed */
				    /*  by ofile_map. */

    /* If this structure is currently referencing an archive and it has a
       table of contents then these are valid and filled in. */
    char *toc_addr;		    /* pointer to the toc contents */
    uint32_t toc_size;		    /* total size of the toc */
    struct ar_hdr *toc_ar_hdr;	    /* the archive header for the toc */
    char *toc_name;		    /* name of toc member */
    uint32_t toc_name_size;	    /* size of name of toc member */
    enum bool toc_is_32bit;	    /* TRUE if toc is 32-bit */
    struct ranlib *toc_ranlibs;     /* 32-bit ranlib structs */
    struct ranlib_64 *toc_ranlibs64; /* 64-bit ranlib_64 structs */
    uint64_t       toc_nranlibs;    /* number of ranlib structs */
    char          *toc_strings;     /* strings of symbol names (for above) */
    uint64_t       toc_strsize;     /* number of bytes for the strings above */
    enum bool	   toc_bad;	    /* the toc needs to be rebuilt */

    /* If this structure is currently referencing a System V archive and it has
       an archive member named "//" which holds the extended filenames then
       these gets filled in after that member is seen. */
    char *sysv_ar_strtab;
    uint32_t sysv_ar_strtab_size;

    /* If this structure is currently referencing an archive member or an object
       file that is an archive member these are valid and filled in. */
    uint64_t member_offset;         /* logical offset to the member starting */
    char *member_addr;      	    /* pointer to the member contents */
    uint32_t member_size;           /* actual size of the member (not rounded)*/
    struct ar_hdr *member_ar_hdr;   /* pointer to the ar_hdr for this member */
    char *member_name;		    /* name of this member */
    uint32_t member_name_size;      /* size of the member name */
    enum ofile_type member_type;    /* the type of file for this member */
    cpu_type_t archive_cputype;	    /* if the archive contains objects then */
    cpu_subtype_t		    /*  these two fields reflect the objects */
	archive_cpusubtype;	    /*  at are in the archive. */

    /* If this structure is currently referencing a dynamic library module 
       these are valid and filled in. */
    struct dylib_module *modtab;    /* the 32-bit module table */
    struct dylib_module_64 *modtab64;/* the 64-bit module table */
    uint32_t nmodtab;	    	    /* the number of module table entries */
    struct dylib_module		    /* pointer to the 32-bit dylib_module for */
	*dylib_module;		    /*  this module. */
    struct dylib_module_64	    /* pointer to the 64-bit dylib_module for */
	*dylib_module64;	    /*  this module. */
    char *dylib_module_name;	    /* the name of the module */

    /* If this structure is currently referencing an object file these are
       valid and filled in.  The mach_header and load commands have been 
       converted to the host byte sex if needed */
    enum bool headers_swapped;	    /* true if the headers have already been
				       swapped to host byte sex */
    char *object_addr;		    /* the address of the object file */
    uint32_t object_size;	    /* the size of the object file */
    enum byte_sex object_byte_sex;  /* the byte sex of the object file */
    struct mach_header *mh;	    /* the mach_header of 32-bit object file */
    struct mach_header_64 *mh64;    /* the mach_header of 64-bit object file */
    struct load_command		    /* the start of the load commands */
	*load_commands;
    /* these copied from the mach header above */
    cpu_type_t mh_cputype;	    /* cpu specifier */
    cpu_subtype_t mh_cpusubtype;    /* machine specifier */
    uint32_t mh_filetype;	    /* type of file */

    /* If this structure is currently referencing an xar member from the xar
       file embedded in the (__LLVM,__bundle) section of the Mach-O file then
       this is valid and filled in. */
    const char *xar_member_name;

    /* If this structure is currently referencing an llvm bitcode file these are
       valid and filled in. */
    void *lto;			    /* really the opaque struct LTOModule * */
    /* these are translated from the lto's target_triple */
    cpu_type_t lto_cputype;	    /* cpu specifier */
    cpu_subtype_t lto_cpusubtype;   /* machine specifier */
};

__private_extern__ void ofile_process(
    char *name,
    struct arch_flag *arch_flags,
    uint32_t narch_flags,
    enum bool all_archs,
    enum bool process_non_objects,
    enum bool dylib_flat,
    enum bool use_member_syntax,
    void (*processor)(struct ofile *ofile, char *arch_name, void *cookie),
    void *cookie);
#ifdef OFI
__private_extern__ NSObjectFileImageReturnCode ofile_map(
#else
__private_extern__ enum bool ofile_map(
#endif
    const char *file_name,
    const struct arch_flag *arch_flag,	/* can be NULL */
    const char *object_name,		/* can be NULL */
    struct ofile *ofile,
    enum bool archives_with_fat_objects);
#ifdef OFI
__private_extern__ NSObjectFileImageReturnCode ofile_map_from_memory(
#else
__private_extern__ enum bool ofile_map_from_memory(
#endif
    char *addr,
    uint64_t size,
    const char *file_name,
    uint64_t mtime,
    const struct arch_flag *arch_flag,	/* can be NULL */
    const char *object_name,		/* can be NULL */
    struct ofile *ofile,
    enum bool archives_with_fat_objects);
__private_extern__ void ofile_unmap(
    struct ofile *ofile);
__private_extern__ enum bool ofile_first_arch(
    struct ofile *ofile);
__private_extern__ enum bool ofile_next_arch(
    struct ofile *ofile);
__private_extern__ enum bool ofile_first_member(
    struct ofile *ofile);
__private_extern__ enum bool ofile_next_member(
    struct ofile *ofile);
__private_extern__ enum bool ofile_specific_member(
    const char *object_name,
    struct ofile *ofile);
__private_extern__ enum bool ofile_first_module(
    struct ofile *ofile);
__private_extern__ enum bool ofile_next_module(
    struct ofile *ofile);
__private_extern__ enum bool ofile_specific_module(
    const char *module_name,
    struct ofile *ofile);
__private_extern__ void ofile_print(
    struct ofile *ofile);
__private_extern__ uint32_t size_ar_name(
    const struct ar_hdr *ar_hdr);
__private_extern__ int32_t ofile_get_word(
    uint64_t addr,
    uint32_t *word,
    void *get_word_data /* struct ofile *ofile */);
__private_extern__ void archive_error(
    struct ofile *ofile,
    const char *format, ...)
#ifndef __MWERKS__
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;
__private_extern__ void archive_member_error(
    struct ofile *ofile,
    const char *format, ...)
#ifndef __MWERKS__
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;
__private_extern__ void Mach_O_error(
    struct ofile *ofile,
    const char *format, ...)
#ifndef __MWERKS__
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;

#ifdef LTO_SUPPORT
__private_extern__ int is_llvm_bitcode(
    struct ofile *ofile,
    char *addr,
    size_t size);
#endif /* LTO_SUPPORT */

#endif /* _STUFF_OFILE_H_ */
