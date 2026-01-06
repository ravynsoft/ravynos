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
#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

#import "stuff/ofile.h"

/*
 * This is used to build the table of contents of an archive.  Each toc_entry
 * Contains a pointer to a symbol name that is defined by a member of the
 * archive.  The member that defines this symbol is referenced by its index in
 * the archive plus one.  This is done so the negative value if the index can
 * be used for marking then later to generate the ran_off field with the byte
 * offset.
 */
struct toc_entry {
    char *symbol_name;
    int64_t member_index;
};

/*
 * The input files are broken out in to their object files and then placed in
 * these structures.  These structures are then used to edit the object files'
 * symbol table.  And then finally used to reassemble the object file for
 * output.
 */
struct arch {
    char *file_name;		/* name of file this arch came from */
    enum ofile_type type;	/* The type of file for this architecture */
				/*  can be OFILE_ARCHIVE, OFILE_Mach_O, */
    				/*  OFILE_LLVM_BITCODE or OFILE_UNKNOWN. */
    struct fat_arch *fat_arch;	/* If this came from fat file one of these */
    struct fat_arch_64		/* is valid and not NULL (needed for the */
		    *fat_arch64;/* align value and to output a fat file if */
				/* only one arch) */
    char *fat_arch_name;	/* If this came from fat file this is valid */
				/*  and is tthe name of this architecture */
				/*  (used for error messages). */

    /* if this is an archive: the members of this archive */
    struct member *members;	/* the members of the library for this arch */
    uint32_t       nmembers;	/* the number of the above members */
    /*
     * The output table of contents (toc) for this arch in the library (this
     * must be recreated, or at least the time of the toc member set, when
     * the output is modified because modifiy time is shared by all libraries
     * in the file).
     */
    uint32_t       toc_size;	/* total size of the toc including ar_hdr */
    struct ar_hdr  toc_ar_hdr;	/* the archive header for this member */
    enum bool      toc_long_name;/* use the long name in the output */
    char	  *toc_name;	 /* name of toc member */
    uint32_t       toc_name_size;/* size of name of toc member */
    uint64_t       ntocs;	/* number of table of contents entries */
    enum bool	   using_64toc; /* TRUE if we are using a 64-bit toc */
    struct toc_entry
		  *toc_entries; /* the table of contents entries */
    struct ranlib *toc_ranlibs;	/* the 32-bit ranlib structs */
    struct ranlib_64 *toc_ranlibs64; /* the 64-bit ranlib structs */
    char	  *toc_strings;	/* strings of symbol names for toc entries */
    uint64_t       toc_strsize;	/* number of bytes for the strings above */
    uint64_t	  library_size;	/* current working size and final output size */
				/*  for this arch when it's a library (used */
				/*  for creating the toc entries). */

    /* if this is an object file: the object file */
    struct object *object;	/* the object file */

#ifdef LTO_SUPPORT
    /* if this member is an llvm bit code file: the lto module */
    void *lto;                  /* lto module */
#endif /* LTO_SUPPORT */

    /* if this is an unknown file: the addr and size of the file */
    char *unknown_addr;
    uint64_t unknown_size;

    /* don't update LC_ID_DYLIB timestamp */
    enum bool dont_update_LC_ID_DYLIB_timestamp;
};

struct member {
    enum ofile_type type;	/* the type of this member can be OFILE_Mach_O*/
				/*  OFILE_LLVM_BITCODE or OFILE_UNKNOWN */
    struct ar_hdr *ar_hdr;	/* the archive header for this member */
    uint64_t offset;		/* current working offset and final offset */
				/*  use in creating the table of contents */

    /* the name of the member in the output */
    char         *member_name;	    /* the member name */
    uint32_t      member_name_size; /* the size of the member name */
    enum bool     member_long_name; /* use the extended format #1 for the
				       member name in the output */

    /* if this member is an object file: the object file */
    struct object *object;	/* the object file */

#ifdef LTO_SUPPORT
    /* if this member is an llvm bit code file: the lto module */
    void *lto;                  /* lto module */
#endif /* LTO_SUPPORT */

    /* if this member is an unknown file: the addr and size of the member */
    char *unknown_addr;
    uint64_t unknown_size;

    /*
     * If this member was created from a file then input_file_name is set else
     * it is NULL and input_ar_hdr is set (these are recorded to allow
     * warn_member() messages to be printed)
     */
    char *input_file_name;
    struct ar_hdr *input_ar_hdr;
};

struct object {
    char *object_addr;		    /* the address of the object file */
    uint32_t object_size;	    /* the size of the object file on input */
    enum byte_sex object_byte_sex;  /* the byte sex of the object file */
    struct mach_header *mh;	    /* the mach_header of 32-bit object file */
    struct mach_header_64 *mh64;    /* the mach_header of 64-bit object file */
    /* these copied from the mach header above */
    cpu_type_t mh_cputype;	    /* cpu specifier */
    cpu_subtype_t mh_cpusubtype;    /* machine specifier */
    uint32_t mh_filetype;	    /* type of file */
    struct load_command		    /* the start of the load commands */
	*load_commands;
    struct symtab_command *st;	    /* the symbol table command */
    struct dysymtab_command *dyst;  /* the dynamic symbol table command */
    struct twolevel_hints_command   /* the two-level namespace hints command */
	*hints_cmd;
    struct prebind_cksum_command *cs;/* the prebind check sum command */
    struct segment_command
	*seg_bitcode;	    	    /* the 32-bit bitcode segment command */
    struct segment_command_64
	*seg_bitcode64;    	    /* the 64-bit bitcode segment command */
    struct segment_command
	*seg_linkedit;	    	    /* the 32-bit link edit segment command */
    struct segment_command_64
	*seg_linkedit64;    	    /* the 64-bit link edit segment command */
    struct linkedit_data_command
	*code_sig_cmd;	    	    /* the code signature load command, if any*/
    struct linkedit_data_command
	*split_info_cmd;    	    /* the split info load command, if any*/
    struct linkedit_data_command
	*func_starts_info_cmd; 	    /* the func starts load command, if any*/
    struct linkedit_data_command
	*data_in_code_cmd;	    /* the data in code load command, if any */
    struct linkedit_data_command
	*code_sign_drs_cmd;	    /* the code signing DRs command, if any */
    struct linkedit_data_command
	*link_opt_hint_cmd;	    /* the linker optimization hint command,
				       if any */
    struct section **sections;	    /* array of 32-bit section structs */
    struct section_64 **sections64; /* array of 64-bit section structs */
    struct dyld_info_command
	*dyld_info;		    /* the LC_DYLD_INFO command,if any */
    struct linkedit_data_command
	*dyld_exports_trie;	    /* the exports trie */
    struct linkedit_data_command
	*dyld_chained_fixups;	    /* the fixups */

    /*
     * This is only used for redo_prebinding and is calculated by breakout()
     * if the calculate_input_prebind_cksum parameter is TRUE and there is an
     * LC_PREBIND_CKSUM load command that has a zero value for the cksum field
     * (if so this will be value of the cksum field on output).
     */
    uint32_t calculated_input_prebind_cksum;

    /*
     * New content to be added to the output file just after where the input
     * sym info was.
     */
    char *output_new_content;
    uint32_t output_new_content_size;

    uint32_t input_sym_info_size;
    uint32_t output_sym_info_size;

    /*
     * For 64-bit Mach-O files they may have an odd number of indirect symbol
     * table entries so the next offset MAYBE or MAY NOT be rounded to a
     * multiple of 8. input_indirectsym_pad contains the amount of padding in
     * that was in the input.
     */
    uint32_t input_indirectsym_pad;

    char *output_dyld_info;
    uint32_t      output_dyld_info_size;
    struct nlist *output_symbols;
    struct nlist_64 *output_symbols64;
    uint32_t      output_nsymbols;
    char	 *output_strings;
    uint32_t      output_strings_size;
    /*
     * To get the code signature data on a page alignment and be compatible with
     * existing tools we have to actually change the string table and pad it.
     */
    uint32_t      output_strings_size_pad;
    char *output_code_sig_data;
    uint32_t      output_code_sig_data_size;
    char *output_split_info_data;
    uint32_t      output_split_info_data_size;
    char *output_func_start_info_data;
    uint32_t      output_func_start_info_data_size;
    char *output_data_in_code_info_data;
    uint32_t      output_data_in_code_info_data_size;
    char *output_code_sign_drs_info_data;
    uint32_t      output_code_sign_drs_info_data_size;
    char *output_link_opt_hint_info_data;
    uint32_t      output_link_opt_hint_info_data_size;
    char *output_dyld_chained_fixups_data;
    uint32_t      output_dyld_chained_fixups_data_size;
    char *output_dyld_exports_trie_data;
    uint32_t      output_dyld_exports_trie_data_size;

    uint32_t      output_ilocalsym;
    uint32_t      output_nlocalsym;
    uint32_t      output_iextdefsym;
    uint32_t      output_nextdefsym;
    uint32_t      output_iundefsym;
    uint32_t      output_nundefsym;

    struct twolevel_hint *output_hints;

    struct relocation_info *output_loc_relocs;
    struct relocation_info *output_ext_relocs;
    uint32_t *output_indirect_symtab;

    struct dylib_table_of_contents *output_tocs;
    uint32_t      output_ntoc;
    struct dylib_module *output_mods;
    struct dylib_module_64 *output_mods64;
    uint32_t      output_nmodtab;
    struct dylib_reference *output_refs;
    uint32_t      output_nextrefsyms;

    /*
     * For strip(1) to strip DWARF debug info it must run ld -r on the original
     * object contents and overwrite it with that output.  That output is mapped
     * by this ofile struct and is cleaned up when strip is done with the arch
     * that contains this object.
     */
    struct ofile *ld_r_ofile;
};

__private_extern__ struct ofile * breakout(
    char *filename,
    struct arch **archs,
    uint32_t *narchs,
    enum bool calculate_input_prebind_cksum);

__private_extern__ struct ofile * breakout_mem(
    void *membuf,
    uint32_t length,
    char *filename,
    struct arch **archs,
    uint32_t *narchs,
    enum bool calculate_input_prebind_cksum);

__private_extern__ void free_archs(
    struct arch *archs,
    uint32_t narchs);

__private_extern__ void writeout(
    struct arch *archs,
    uint32_t narchs,
    char *output,
    unsigned short mode,
    enum bool sort_toc,
    enum bool commons_in_toc,
    enum bool force_64bit_toc,
    enum bool library_warnings,
    uint32_t *throttle);

__private_extern__ void writeout_to_mem(
    struct arch *archs,
    uint32_t narchs,
    char *filename,
    void **outputbuf,
    uint64_t *length,
    enum bool sort_toc,
    enum bool commons_in_toc,
    enum bool force_64bit_toc,
    enum bool library_warning,
    enum bool *seen_archive);

__private_extern__ void checkout(
    struct arch *archs,
    uint32_t narchs);

void warning_arch(
    struct arch *arch,
    struct member *member,
    char *format, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
    ;

void error_arch(
    struct arch *arch,
    struct member *member,
    char *format, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
    ;

void fatal_arch(
    struct arch *arch,
    struct member *member,
    char *format, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
    ;
