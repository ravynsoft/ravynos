/* ldlang.h - linker command language support
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef LDLANG_H
#define LDLANG_H

#define DEFAULT_MEMORY_REGION   "*default*"

#define SECTION_NAME_MAP_LENGTH (16)

typedef enum
{
  lang_input_file_is_l_enum,
  lang_input_file_is_symbols_only_enum,
  lang_input_file_is_marker_enum,
  lang_input_file_is_fake_enum,
  lang_input_file_is_search_file_enum,
  lang_input_file_is_file_enum
} lang_input_file_enum_type;

struct _fill_type
{
  size_t size;
  unsigned char data[1];
};

typedef struct statement_list
{
  union lang_statement_union *  head;
  union lang_statement_union ** tail;
} lang_statement_list_type;

typedef struct memory_region_name_struct
{
  const char * name;
  struct memory_region_name_struct * next;
} lang_memory_region_name;

typedef struct memory_region_struct
{
  lang_memory_region_name name_list;
  struct memory_region_struct *next;
  union etree_union *origin_exp;
  bfd_vma origin;
  bfd_size_type length;
  union etree_union *length_exp;
  bfd_vma current;
  union lang_statement_union *last_os;
  flagword flags;
  flagword not_flags;
  bool had_full_message;
} lang_memory_region_type;

enum statement_enum
{
  lang_address_statement_enum,
  lang_assignment_statement_enum,
  lang_data_statement_enum,
  lang_fill_statement_enum,
  lang_group_statement_enum,
  lang_input_section_enum,
  lang_input_matcher_enum,
  lang_input_statement_enum,
  lang_insert_statement_enum,
  lang_output_section_statement_enum,
  lang_output_statement_enum,
  lang_padding_statement_enum,
  lang_reloc_statement_enum,
  lang_target_statement_enum,
  lang_wild_statement_enum,
  lang_constructors_statement_enum,
  lang_object_symbols_statement_enum
};

typedef struct lang_statement_header_struct
{
  /* Next pointer for statement_list statement list.  */
  union lang_statement_union *next;
  enum statement_enum type;
} lang_statement_header_type;

typedef struct
{
  lang_statement_header_type header;
  union etree_union *exp;
} lang_assignment_statement_type;

typedef struct lang_target_statement_struct
{
  lang_statement_header_type header;
  const char *target;
} lang_target_statement_type;

typedef struct lang_output_statement_struct
{
  lang_statement_header_type header;
  const char *name;
} lang_output_statement_type;

/* Section types specified in a linker script.  */

enum section_type
{
  normal_section,
  first_overlay_section,
  overlay_section,
  noload_section,
  noalloc_section,
  type_section,
  readonly_section,
  typed_readonly_section
};

/* This structure holds a list of program headers describing
   segments in which this section should be placed.  */

typedef struct lang_output_section_phdr_list
{
  struct lang_output_section_phdr_list *next;
  const char *name;
  bool used;
} lang_output_section_phdr_list;

typedef struct lang_output_section_statement_struct
{
  lang_statement_header_type header;
  lang_statement_list_type children;
  struct lang_output_section_statement_struct *next;
  struct lang_output_section_statement_struct *prev;
  const char *name;
  asection *bfd_section;
  lang_memory_region_type *region;
  lang_memory_region_type *lma_region;
  fill_type *fill;
  union etree_union *addr_tree;
  union etree_union *load_base;
  union etree_union *section_alignment;
  union etree_union *subsection_alignment;

  /* If non-null, an expression to evaluate after setting the section's
     size.  The expression is evaluated inside REGION (above) with '.'
     set to the end of the section.  Used in the last overlay section
     to move '.' past all the overlaid sections.  */
  union etree_union *update_dot_tree;

  lang_output_section_phdr_list *phdrs;

  /* Used by ELF SHF_LINK_ORDER sorting.  */
  void *data;

  unsigned int block_value;
  int constraint;
  flagword flags;
  enum section_type sectype;
  etree_type *sectype_value;
  unsigned int processed_vma : 1;
  unsigned int processed_lma : 1;
  unsigned int all_input_readonly : 1;
  /* If this section should be ignored.  */
  unsigned int ignored : 1;
  /* If this section should update "dot".  Prevents section being ignored.  */
  unsigned int update_dot : 1;
  /* If this section is after assignment to _end.  */
  unsigned int after_end : 1;
  /* If this section uses the alignment of its input sections.  */
  unsigned int align_lma_with_input : 1;
  /* If script has duplicate output section statements of the same name
     create duplicate output sections.  */
  unsigned int dup_output : 1;
} lang_output_section_statement_type;

typedef struct
{
  lang_statement_header_type header;
  fill_type *fill;
  int size;
  asection *output_section;
} lang_fill_statement_type;

typedef struct
{
  lang_statement_header_type header;
  unsigned int type;
  union etree_union *exp;
  bfd_vma value;
  asection *output_section;
  bfd_vma output_offset;
} lang_data_statement_type;

/* Generate a reloc in the output file.  */

typedef struct
{
  lang_statement_header_type header;

  /* Reloc to generate.  */
  bfd_reloc_code_real_type reloc;

  /* Reloc howto structure.  */
  reloc_howto_type *howto;

  /* Section to generate reloc against.
     Exactly one of section and name must be NULL.  */
  asection *section;

  /* Name of symbol to generate reloc against.
     Exactly one of section and name must be NULL.  */
  const char *name;

  /* Expression for addend.  */
  union etree_union *addend_exp;

  /* Resolved addend.  */
  bfd_vma addend_value;

  /* Output section where reloc should be performed.  */
  asection *output_section;

  /* Offset within output section.  */
  bfd_vma output_offset;
} lang_reloc_statement_type;

struct lang_input_statement_flags
{
  /* 1 means this file was specified in a -l option.  */
  unsigned int maybe_archive : 1;

  /* 1 means this file was specified in a -l:namespec option.  */
  unsigned int full_name_provided : 1;

  /* 1 means search a set of directories for this file.  */
  unsigned int search_dirs : 1;

  /* 1 means this was found when processing a script in the sysroot.  */
  unsigned int sysrooted : 1;

  /* 1 means this is base file of incremental load.
     Do not load this file's text or data.
     Also default text_start to after this file's bss.  */
  unsigned int just_syms : 1;

  /* Whether to search for this entry as a dynamic archive.  */
  unsigned int dynamic : 1;

  /* Set if a DT_NEEDED tag should be added not just for the dynamic library
     explicitly given by this entry but also for any dynamic libraries in
     this entry's needed list.  */
  unsigned int add_DT_NEEDED_for_dynamic : 1;

  /* Set if this entry should cause a DT_NEEDED tag only when some
     regular file references its symbols (ie. --as-needed is in effect).  */
  unsigned int add_DT_NEEDED_for_regular : 1;

  /* Whether to include the entire contents of an archive.  */
  unsigned int whole_archive : 1;

  /* Set when bfd opening is successful.  */
  unsigned int loaded : 1;

  unsigned int real : 1;

  /* Set if the file does not exist.  */
  unsigned int missing_file : 1;

  /* Set if reloading an archive or --as-needed lib.  */
  unsigned int reload : 1;

#if BFD_SUPPORTS_PLUGINS
  /* Set if the file was claimed by a plugin.  */
  unsigned int claimed : 1;

  /* Set if the file was claimed from an archive.  */
  unsigned int claim_archive : 1;

  /* Set if added by the lto plugin add_input_file callback.  */
  unsigned int lto_output : 1;
#endif /* BFD_SUPPORTS_PLUGINS */

  /* Head of list of pushed flags.  */
  struct lang_input_statement_flags *pushed;
};

typedef struct lang_input_statement_struct
{
  lang_statement_header_type header;
  /* Name of this file.  */
  const char *filename;
  /* Name to use for the symbol giving address of text start.
     Usually the same as filename, but for a file spec'd with
     -l this is the -l switch itself rather than the filename.  */
  const char *local_sym_name;
  /* Name to use when sorting.  */
  const char *sort_key;
  /* Extra search path. Used to find a file relative to the
     directory of the current linker script.  */
  const char *extra_search_path;

  bfd *the_bfd;

  ctf_archive_t *the_ctf;

  struct flag_info *section_flag_list;

  /* Next pointer for file_chain statement list.  */
  struct lang_input_statement_struct *next;

  /* Next pointer for input_file_chain statement list.  */
  struct lang_input_statement_struct *next_real_file;

  const char *target;

  struct lang_input_statement_flags flags;
} lang_input_statement_type;

typedef struct
{
  lang_statement_header_type header;
  asection *section;
  void *pattern;
} lang_input_section_type;

typedef struct
{
  lang_statement_header_type header;
  asection *section;
  void *pattern;
  lang_input_statement_type *input_stmt;
} lang_input_matcher_type;

struct map_symbol_def {
  struct bfd_link_hash_entry *entry;
  struct map_symbol_def *next;
};

/* For input sections, when writing a map file: head / tail of a linked
   list of hash table entries for symbols defined in this section.  */
typedef struct input_section_userdata_struct
{
  struct map_symbol_def *map_symbol_def_head;
  struct map_symbol_def **map_symbol_def_tail;
  unsigned long map_symbol_def_count;
} input_section_userdata_type;

static inline bool
bfd_input_just_syms (const bfd *abfd)
{
  lang_input_statement_type *is = bfd_usrdata (abfd);
  return is != NULL && is->flags.just_syms;
}

typedef struct lang_wild_statement_struct lang_wild_statement_type;

typedef void (*callback_t) (lang_wild_statement_type *, struct wildcard_list *,
			    asection *, lang_input_statement_type *, void *);

typedef void (*walk_wild_section_handler_t) (lang_wild_statement_type *,
					     lang_input_statement_type *,
					     callback_t callback,
					     void *data);

typedef bool (*lang_match_sec_type_func) (bfd *, const asection *,
					  bfd *, const asection *);

/* Binary search tree structure to efficiently sort sections by
   name.  */
typedef struct lang_section_bst
{
  asection *section;
  void *pattern;
  struct lang_section_bst *left;
  struct lang_section_bst *right;
} lang_section_bst_type;

struct lang_wild_statement_struct
{
  lang_statement_header_type header;
  const char *filename;
  bool filenames_sorted;
  bool any_specs_sorted;
  struct wildcard_list *section_list;
  bool keep_sections;
  lang_statement_list_type children;
  struct name_list *exclude_name_list;
  lang_statement_list_type matching_sections;

  lang_section_bst_type *tree, **rightmost;
  struct flag_info *section_flag_list;
};

typedef struct lang_address_statement_struct
{
  lang_statement_header_type header;
  const char *section_name;
  union etree_union *address;
  const segment_type *segment;
} lang_address_statement_type;

typedef struct
{
  lang_statement_header_type header;
  bfd_vma output_offset;
  bfd_size_type size;
  asection *output_section;
  fill_type *fill;
} lang_padding_statement_type;

/* A group statement collects a set of libraries together.  The
   libraries are searched multiple times, until no new undefined
   symbols are found.  The effect is to search a group of libraries as
   though they were a single library.  */

typedef struct
{
  lang_statement_header_type header;
  lang_statement_list_type children;
} lang_group_statement_type;

typedef struct
{
  lang_statement_header_type header;
  const char *where;
  bool is_before;
} lang_insert_statement_type;

typedef union lang_statement_union
{
  lang_statement_header_type header;
  lang_address_statement_type address_statement;
  lang_assignment_statement_type assignment_statement;
  lang_data_statement_type data_statement;
  lang_fill_statement_type fill_statement;
  lang_group_statement_type group_statement;
  lang_input_section_type input_section;
  lang_input_matcher_type input_matcher;
  lang_input_statement_type input_statement;
  lang_insert_statement_type insert_statement;
  lang_output_section_statement_type output_section_statement;
  lang_output_statement_type output_statement;
  lang_padding_statement_type padding_statement;
  lang_reloc_statement_type reloc_statement;
  lang_target_statement_type target_statement;
  lang_wild_statement_type wild_statement;
} lang_statement_union_type;

/* This structure holds information about a program header, from the
   PHDRS command in the linker script.  */

struct lang_phdr
{
  struct lang_phdr *next;
  const char *name;
  unsigned long type;
  bool filehdr;
  bool phdrs;
  etree_type *at;
  etree_type *flags;
};

/* This structure is used to hold a list of sections which may not
   cross reference each other.  */

typedef struct lang_nocrossref
{
  struct lang_nocrossref *next;
  const char *name;
} lang_nocrossref_type;

/* The list of nocrossref lists.  */

struct lang_nocrossrefs
{
  struct lang_nocrossrefs *next;
  lang_nocrossref_type *list;
  bool onlyfirst;
};

/* This structure is used to hold a list of input section names which
   will not match an output section in the linker script.  */

struct unique_sections
{
  struct unique_sections *next;
  const char *name;
};

/* Used by place_orphan to keep track of orphan sections and statements.  */

struct orphan_save
{
  const char *name;
  flagword flags;
  lang_output_section_statement_type *os;
  asection **section;
  lang_statement_union_type **stmt;
  lang_output_section_statement_type **os_tail;
};

struct asneeded_minfo
{
  struct asneeded_minfo *next;
  const char *soname;
  bfd *ref;
  const char *name;
};

extern struct lang_phdr *lang_phdr_list;
extern struct lang_nocrossrefs *nocrossref_list;
extern const char *output_target;
extern lang_output_section_statement_type *abs_output_section;
extern lang_statement_list_type lang_os_list;
extern struct lang_input_statement_flags input_flags;
extern bool lang_has_input_file;
extern lang_statement_list_type statement_list;
extern lang_statement_list_type *stat_ptr;
extern bool delete_output_file_on_failure;
extern bool enable_linker_version;

extern struct bfd_sym_chain entry_symbol;
extern const char *entry_section;
extern bool entry_from_cmdline;
extern lang_statement_list_type file_chain;
extern lang_statement_list_type input_file_chain;

extern struct bfd_elf_dynamic_list **current_dynamic_list_p;

extern int lang_statement_iteration;
extern struct asneeded_minfo **asneeded_list_tail;

extern void (*output_bfd_hash_table_free_fn) (struct bfd_link_hash_table *);

extern void lang_init
  (void);
extern void lang_finish
  (void);
extern lang_memory_region_type * lang_memory_region_lookup
  (const char * const, bool);
extern void lang_memory_region_alias
  (const char *, const char *);
extern void lang_map
  (void);
extern void lang_set_flags
  (lang_memory_region_type *, const char *, int);
extern void lang_add_output
  (const char *, int from_script);
extern lang_output_section_statement_type *lang_enter_output_section_statement
  (const char *, etree_type *, enum section_type, etree_type *, etree_type *,
   etree_type *, etree_type *, int, int);
extern void lang_final
  (void);
extern void lang_relax_sections
  (bool);
extern void lang_process
  (void);
extern void lang_section_start
  (const char *, union etree_union *, const segment_type *);
extern void lang_add_entry
  (const char *, bool);
extern void lang_default_entry
  (const char *);
extern void lang_add_target
  (const char *);
extern void lang_add_wild
  (struct wildcard_spec *, struct wildcard_list *, bool);
extern void lang_add_map
  (const char *);
extern void lang_add_fill
  (fill_type *);
extern lang_assignment_statement_type *lang_add_assignment
  (union etree_union *);
extern void lang_add_attribute
  (enum statement_enum);
extern void lang_startup
  (const char *);
extern void lang_float
  (bool);
extern void lang_leave_output_section_statement
  (fill_type *, const char *, lang_output_section_phdr_list *,
   const char *);
extern void lang_for_each_input_file
  (void (*dothis) (lang_input_statement_type *));
extern void lang_for_each_file
  (void (*dothis) (lang_input_statement_type *));
extern void lang_reset_memory_regions
  (void);
extern void lang_do_assignments
  (lang_phase_type);
extern asection *section_for_dot
  (void);

#define LANG_FOR_EACH_INPUT_STATEMENT(statement)			\
  lang_input_statement_type *statement;					\
  for (statement = (lang_input_statement_type *) file_chain.head;	\
       statement != NULL;						\
       statement = statement->next)

#define lang_output_section_find(NAME) \
  lang_output_section_statement_lookup (NAME, 0, 0)

extern void lang_process
  (void);
extern void ldlang_add_file
  (lang_input_statement_type *);
extern lang_output_section_statement_type *lang_output_section_find_by_flags
  (const asection *, flagword, lang_output_section_statement_type **,
   lang_match_sec_type_func);
extern lang_output_section_statement_type *lang_insert_orphan
  (asection *, const char *, int, lang_output_section_statement_type *,
   struct orphan_save *, etree_type *, lang_statement_list_type *);
extern lang_input_statement_type *lang_add_input_file
  (const char *, lang_input_file_enum_type, const char *);
extern void lang_add_keepsyms_file
  (const char *);
extern lang_output_section_statement_type *lang_output_section_get
  (const asection *);
extern lang_output_section_statement_type *lang_output_section_statement_lookup
  (const char *, int, int);
extern lang_output_section_statement_type *next_matching_output_section_statement
  (lang_output_section_statement_type *, int);
extern void ldlang_add_undef
  (const char *const, bool);
extern void ldlang_add_require_defined
  (const char *const);
extern void lang_add_output_format
  (const char *, const char *, const char *, int);
extern void lang_list_init
  (lang_statement_list_type *);
extern void push_stat_ptr
  (lang_statement_list_type *);
extern void pop_stat_ptr
  (void);
extern void lang_add_data
  (int, union etree_union *);
extern void lang_add_string
  (const char *);
extern void lang_add_reloc
  (bfd_reloc_code_real_type, reloc_howto_type *, asection *, const char *,
   union etree_union *);
extern void lang_for_each_statement
  (void (*) (lang_statement_union_type *));
extern void lang_for_each_statement_worker
  (void (*) (lang_statement_union_type *), lang_statement_union_type *);
extern void *stat_alloc
  (size_t);
extern void strip_excluded_output_sections
  (void);
extern void lang_clear_os_map
  (void);
extern void dprint_statement
  (lang_statement_union_type *, int);
extern void lang_size_sections
  (bool *, bool);
extern void one_lang_size_sections_pass
  (bool *, bool);
extern void lang_add_insert
  (const char *, int);
extern void lang_enter_group
  (void);
extern void lang_leave_group
  (void);
extern void lang_add_section
  (lang_statement_list_type *, asection *, struct wildcard_list *,
   struct flag_info *, lang_output_section_statement_type *);
extern void lang_new_phdr
  (const char *, etree_type *, bool, bool, etree_type *,
   etree_type *);
extern void lang_add_nocrossref
  (lang_nocrossref_type *);
extern void lang_add_nocrossref_to
  (lang_nocrossref_type *);
extern void lang_enter_overlay
  (etree_type *, etree_type *);
extern void lang_enter_overlay_section
  (const char *);
extern void lang_leave_overlay_section
  (fill_type *, lang_output_section_phdr_list *);
extern void lang_leave_overlay
  (etree_type *, int, fill_type *, const char *,
   lang_output_section_phdr_list *, const char *);

extern struct bfd_elf_version_expr *lang_new_vers_pattern
  (struct bfd_elf_version_expr *, const char *, const char *, bool);
extern struct bfd_elf_version_tree *lang_new_vers_node
  (struct bfd_elf_version_expr *, struct bfd_elf_version_expr *);
extern struct bfd_elf_version_deps *lang_add_vers_depend
  (struct bfd_elf_version_deps *, const char *);
extern void lang_register_vers_node
  (const char *, struct bfd_elf_version_tree *, struct bfd_elf_version_deps *);
extern void lang_append_dynamic_list (struct bfd_elf_dynamic_list **,
				      struct bfd_elf_version_expr *);
extern void lang_append_dynamic_list_cpp_typeinfo (void);
extern void lang_append_dynamic_list_cpp_new (void);
extern void lang_add_unique
  (const char *);
extern const char *lang_get_output_target
  (void);
extern void add_excluded_libs (const char *);
extern bool load_symbols
  (lang_input_statement_type *, lang_statement_list_type *);

struct elf_sym_strtab;
struct elf_strtab_hash;
extern void ldlang_ctf_acquire_strings
  (struct elf_strtab_hash *);
extern void ldlang_ctf_new_dynsym
  (int symidx, struct elf_internal_sym *);
extern void ldlang_write_ctf_late
  (void);
extern bool
ldlang_override_segment_assignment
  (struct bfd_link_info *, bfd *, asection *, asection *, bool);

extern void
lang_ld_feature (char *);

extern void
lang_print_memory_usage (void);

extern void
lang_add_gc_name (const char *);

extern bool
print_one_symbol (struct bfd_link_hash_entry *, void *);

extern void lang_add_version_string
  (void);
#endif
