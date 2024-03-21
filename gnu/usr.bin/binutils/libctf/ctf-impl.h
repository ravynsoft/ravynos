/* Implementation header.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_CTF_IMPL_H
#define	_CTF_IMPL_H

#include "config.h"
#include <errno.h>
#include <sys/param.h>
#include "ctf-decls.h"
#include <ctf-api.h>
#include "ctf-sha1.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <elf.h>
#include <bfd.h>
#include "hashtab.h"
#include "ctf-intl.h"

#ifdef	__cplusplus
extern "C"
{
#endif

/* Tuning.  */

/* The proportion of symtypetab entries which must be pads before we consider it
   worthwhile to emit a symtypetab section as an index.  Indexes cost time to
   look up, but save space all told.  Do not set to 1, since this will cause
   indexes to be eschewed completely, even in child dicts, at considerable space
   cost.  */
#define CTF_INDEX_PAD_THRESHOLD .75

/* Compiler attributes.  */

#if defined (__GNUC__)

/* GCC.  We assume that all compilers claiming to be GCC support sufficiently
   many GCC attributes that the code below works.  If some non-GCC compilers
   masquerading as GCC in fact do not implement these attributes, version checks
   may be required.  */

/* We use the _libctf_*_ pattern to avoid clashes with any future attribute
   macros glibc may introduce, which have names of the pattern
   __attribute_blah__.  */

#define _libctf_printflike_(string_index,first_to_check) \
    __attribute__ ((__format__ (__printf__, (string_index), (first_to_check))))
#define _libctf_unlikely_(x) __builtin_expect ((x), 0)
#define _libctf_unused_ __attribute__ ((__unused__))
#define _libctf_malloc_ __attribute__((__malloc__))
#define _libctf_nonnull_(params) __attribute__((__nonnull__ params))

#else

#define _libctf_printflike_(string_index,first_to_check)
#define _libctf_unlikely_(x) (x)
#define _libctf_unused_
#define _libctf_malloc_
#define _libctf_nonnull_(params)
#define __extension__

#endif

#if defined (ENABLE_LIBCTF_HASH_DEBUGGING) && !defined (NDEBUG)
#include <assert.h>
#define ctf_assert(fp, expr) (assert (expr), 1)
#else
#define ctf_assert(fp, expr)						\
  _libctf_unlikely_ (ctf_assert_internal (fp, __FILE__, __LINE__,	\
					  #expr, !!(expr)))
#endif

/* libctf in-memory state.  */

typedef struct ctf_fixed_hash ctf_hash_t; /* Private to ctf-hash.c.  */
typedef struct ctf_dynhash ctf_dynhash_t; /* Private to ctf-hash.c.  */
typedef struct ctf_dynset ctf_dynset_t;   /* Private to ctf-hash.c.  */

typedef struct ctf_strs
{
  const char *cts_strs;		/* Base address of string table.  */
  size_t cts_len;		/* Size of string table in bytes.  */
} ctf_strs_t;

typedef struct ctf_strs_writable
{
  char *cts_strs;		/* Base address of string table.  */
  size_t cts_len;		/* Size of string table in bytes.  */
} ctf_strs_writable_t;

typedef struct ctf_dmodel
{
  const char *ctd_name;		/* Data model name.  */
  int ctd_code;			/* Data model code.  */
  size_t ctd_pointer;		/* Size of void * in bytes.  */
  size_t ctd_char;		/* Size of char in bytes.  */
  size_t ctd_short;		/* Size of short in bytes.  */
  size_t ctd_int;		/* Size of int in bytes.  */
  size_t ctd_long;		/* Size of long in bytes.  */
} ctf_dmodel_t;

typedef struct ctf_names
{
  ctf_hash_t *ctn_readonly;	/* Hash table when readonly.  */
  ctf_dynhash_t *ctn_writable;	/* Hash table when writable.  */
} ctf_names_t;

typedef struct ctf_lookup
{
  const char *ctl_prefix;	/* String prefix for this lookup.  */
  size_t ctl_len;		/* Length of prefix string in bytes.  */
  ctf_names_t *ctl_hash;	/* Pointer to hash table for lookup.  */
} ctf_lookup_t;

typedef struct ctf_dictops
{
  uint32_t (*ctfo_get_kind) (uint32_t);
  uint32_t (*ctfo_get_root) (uint32_t);
  uint32_t (*ctfo_get_vlen) (uint32_t);
  ssize_t (*ctfo_get_ctt_size) (const ctf_dict_t *, const ctf_type_t *,
				ssize_t *, ssize_t *);
  ssize_t (*ctfo_get_vbytes) (ctf_dict_t *, unsigned short, ssize_t, size_t);
} ctf_dictops_t;

typedef struct ctf_list
{
  struct ctf_list *l_prev;	/* Previous pointer or tail pointer.  */
  struct ctf_list *l_next;	/* Next pointer or head pointer.  */
} ctf_list_t;

typedef enum
  {
   CTF_PREC_BASE,
   CTF_PREC_POINTER,
   CTF_PREC_ARRAY,
   CTF_PREC_FUNCTION,
   CTF_PREC_MAX
  } ctf_decl_prec_t;

typedef struct ctf_decl_node
{
  ctf_list_t cd_list;		/* Linked list pointers.  */
  ctf_id_t cd_type;		/* Type identifier.  */
  uint32_t cd_kind;		/* Type kind.  */
  uint32_t cd_n;		/* Type dimension if array.  */
} ctf_decl_node_t;

typedef struct ctf_decl
{
  ctf_list_t cd_nodes[CTF_PREC_MAX]; /* Declaration node stacks.  */
  int cd_order[CTF_PREC_MAX];	     /* Storage order of decls.  */
  ctf_decl_prec_t cd_qualp;	     /* Qualifier precision.  */
  ctf_decl_prec_t cd_ordp;	     /* Ordered precision.  */
  char *cd_buf;			     /* Buffer for output.  */
  int cd_err;			     /* Saved error value.  */
  int cd_enomem;		     /* Nonzero if OOM during printing.  */
} ctf_decl_t;

typedef struct ctf_dtdef
{
  ctf_list_t dtd_list;		/* List forward/back pointers.  */
  ctf_id_t dtd_type;		/* Type identifier for this definition.  */
  ctf_type_t dtd_data;		/* Type node, including name.  */
  size_t dtd_vlen_alloc;	/* Total vlen space allocated.  */
  unsigned char *dtd_vlen;	/* Variable-length data for this type.  */
} ctf_dtdef_t;

typedef struct ctf_dvdef
{
  ctf_list_t dvd_list;		/* List forward/back pointers.  */
  char *dvd_name;		/* Name associated with variable.  */
  ctf_id_t dvd_type;		/* Type of variable.  */
  unsigned long dvd_snapshots;	/* Snapshot count when inserted.  */
} ctf_dvdef_t;

typedef struct ctf_err_warning
{
  ctf_list_t cew_list;		/* List forward/back pointers.  */
  int cew_is_warning;		/* 1 if warning, 0 if error.  */
  char *cew_text;		/* Error/warning text.  */
} ctf_err_warning_t;

/* Atoms associate strings with a list of the CTF items that reference that
   string, so that ctf_update() can instantiate all the strings using the
   ctf_str_atoms and then reassociate them with the real string later.

   Strings can be interned into ctf_str_atom without having refs associated
   with them, for values that are returned to callers, etc.  Items are only
   removed from this table on ctf_close(), but on every ctf_update(), all the
   csa_refs in all entries are purged.  */

typedef struct ctf_str_atom
{
  const char *csa_str;		/* Backpointer to string (hash key).  */
  ctf_list_t csa_refs;		/* This string's refs.  */
  uint32_t csa_offset;		/* Strtab offset, if any.  */
  uint32_t csa_external_offset;	/* External strtab offset, if any.  */
  unsigned long csa_snapshot_id; /* Snapshot ID at time of creation.  */
} ctf_str_atom_t;

/* The refs of a single string in the atoms table.  */

typedef struct ctf_str_atom_ref
{
  ctf_list_t caf_list;		/* List forward/back pointers.  */
  uint32_t *caf_ref;		/* A single ref to this string.  */
} ctf_str_atom_ref_t;

/* A single linker-provided symbol, during symbol addition, possibly before we
   have been given external strtab refs.  */
typedef struct ctf_in_flight_dynsym
{
  ctf_list_t cid_list;		/* List forward/back pointers.  */
  ctf_link_sym_t cid_sym;	/* The linker-known symbol.  */
} ctf_in_flight_dynsym_t;

/* The structure used as the key in a ctf_link_type_mapping.  The value is a
   type index, not a type ID.  */

typedef struct ctf_link_type_key
{
  ctf_dict_t *cltk_fp;
  ctf_id_t cltk_idx;
} ctf_link_type_key_t;

/* The structure used as the key in a cd_id_to_dict_t on 32-bit platforms.  */
typedef struct ctf_type_id_key
{
  int ctii_input_num;
  ctf_id_t ctii_type;
} ctf_type_id_key_t;

/* Deduplicator state.

   The dedup state below uses three terms consistently. A "hash" is a
   ctf_dynhash_t; a "hash value" is the hash value of a type as returned by
   ctf_dedup_hash_type; a "global type ID" or "global ID" is a packed-together
   reference to a single ctf_dict_t (by array index in an array of inputs) and
   ctf_id_t, i.e. a single instance of some hash value in some input.

   The deduplication algorithm takes a bunch of inputs and yields a single
   shared "output" and possibly many outputs corresponding to individual inputs
   that still contain types after sharing of unconflicted types.  Almost all
   deduplicator state is stored in the struct ctf_dedup in the output, though a
   (very) few things are stored in inputs for simplicity's sake, usually if they
   are linking together things within the scope of a single TU.

   Flushed at the end of every ctf_dedup run.  */

typedef struct ctf_dedup
{
  /* The CTF linker flags in force for this dedup run.  */
  int cd_link_flags;

  /* On 32-bit platforms only, a hash of global type IDs, in the form of
     a ctf_link_type_id_key_t.  */
  ctf_dynhash_t *cd_id_to_dict_t;

  /* Atoms tables of decorated names: maps undecorated name to decorated name.
     (The actual allocations are in the CTF dict for the former and the real
     atoms table for the latter).  Uses the same namespaces as ctf_lookups,
     below, but has no need for null-termination.  */
  ctf_dynhash_t *cd_decorated_names[4];

  /* Map type names to a hash from type hash value -> number of times each value
     has appeared.  */
  ctf_dynhash_t *cd_name_counts;

  /* Map global type IDs to type hash values.  Used to determine if types are
     already hashed without having to recompute their hash values again, and to
     link types together at later stages.  Forwards that are peeked through to
     structs and unions are not represented in here, so lookups that might be
     such a type (in practice, all lookups) must go via cd_replaced_types first
     to take this into account.  Discarded before each rehashing.  */
  ctf_dynhash_t *cd_type_hashes;

  /* Maps from the names of structs/unions/enums to a a single GID which is the
     only appearance of that type in any input: if it appears in more than one
     input, a value which is a GID with an input_num of -1 appears.  Used in
     share-duplicated link mode link modes to determine whether structs/unions
     can be cited from multiple TUs.  Only populated in that link mode.  */
  ctf_dynhash_t *cd_struct_origin;

  /* Maps type hash values to a set of hash values of the types that cite them:
     i.e., pointing backwards up the type graph.  Used for recursive conflict
     marking.  Citations from tagged structures, unions, and forwards do not
     appear in this graph.  */
  ctf_dynhash_t *cd_citers;

  /* Maps type hash values to input global type IDs.  The value is a set (a
     hash) of global type IDs.  Discarded before each rehashing.  The result of
     the ctf_dedup function.  */
  ctf_dynhash_t *cd_output_mapping;

  /* A map giving the GID of the first appearance of each type for each type
     hash value.  */
  ctf_dynhash_t *cd_output_first_gid;

  /* Used to ensure that we never try to map a single type ID to more than one
     hash.  */
  ctf_dynhash_t *cd_output_mapping_guard;

  /* Maps the global type IDs of structures in input TUs whose members still
     need emission to the global type ID of the already-emitted target type
     (which has no members yet) in the appropriate target.  Uniquely, the latter
     ID represents a *target* ID (i.e. the cd_output_mapping of some specified
     input): we encode the shared (parent) dict with an ID of -1.  */
  ctf_dynhash_t *cd_emission_struct_members;

  /* A set (a hash) of hash values of conflicting types.  */
  ctf_dynset_t *cd_conflicting_types;

  /* A hash mapping fp *'s of inputs to their input_nums.  Used only by
     functions outside the core ctf_dedup / ctf_dedup_emit machinery which do
     not take an inputs array.  */
  ctf_dynhash_t *cd_input_nums;

  /* Maps type hashes to ctf_id_t's in this dictionary.  Populated only at
     emission time, in the dictionary where emission is taking place.  */
  ctf_dynhash_t *cd_output_emission_hashes;

  /* Maps the decorated names of conflicted cross-TU forwards that were forcibly
     emitted in this TU to their emitted ctf_id_ts.  Populated only at emission
     time, in the dictionary where emission is taking place. */
  ctf_dynhash_t *cd_output_emission_conflicted_forwards;

  /* Points to the output counterpart of this input dictionary, at emission
     time.  */
  ctf_dict_t *cd_output;
} ctf_dedup_t;

/* The ctf_dict is the structure used to represent a CTF dictionary to library
   clients, who see it only as an opaque pointer.  Modifications can therefore
   be made freely to this structure without regard to client versioning.  The
   ctf_dict_t typedef appears in <ctf-api.h> and declares a forward tag.
   (A ctf_file_t typedef also appears there, for historical reasons.)

   NOTE: ctf_serialize requires that everything inside of ctf_dict either be an
   immediate value, a pointer to dynamically allocated data *outside* of the
   ctf_dict itself, a pointer to statically allocated data, or specially handled
   in ctf_serialize.  If you add a pointer to ctf_dict that points to something
   within the ctf_dict itself, you must make corresponding changes to
   ctf_serialize.  */

struct ctf_dict
{
  const ctf_dictops_t *ctf_dictops; /* Version-specific dict operations.  */
  struct ctf_header *ctf_header;    /* The header from this CTF dict.  */
  unsigned char ctf_openflags;	    /* Flags the dict had when opened.  */
  ctf_sect_t ctf_data;		    /* CTF data from object file.  */
  ctf_sect_t ctf_symtab;	    /* Symbol table from object file.  */
  ctf_sect_t ctf_strtab;	    /* String table from object file.  */
  int ctf_symsect_little_endian;    /* Endianness of the ctf_symtab.  */
  ctf_dynhash_t *ctf_symhash;       /* (partial) hash, symsect name -> idx. */
  size_t ctf_symhash_latest;	    /* Amount of symsect scanned so far.  */
  ctf_dynhash_t *ctf_prov_strtab;   /* Maps provisional-strtab offsets
				       to names.  */
  ctf_dynhash_t *ctf_syn_ext_strtab; /* Maps ext-strtab offsets to names.  */
  void *ctf_data_mmapped;	    /* CTF data we mmapped, to free later.  */
  size_t ctf_data_mmapped_len;	    /* Length of CTF data we mmapped.  */
  ctf_names_t ctf_structs;	    /* Hash table of struct types.  */
  ctf_names_t ctf_unions;	    /* Hash table of union types.  */
  ctf_names_t ctf_enums;	    /* Hash table of enum types.  */
  ctf_names_t ctf_names;	    /* Hash table of remaining type names.  */
  ctf_lookup_t ctf_lookups[5];	    /* Pointers to nametabs for name lookup.  */
  ctf_strs_t ctf_str[2];	    /* Array of string table base and bounds.  */
  ctf_dynhash_t *ctf_str_atoms;	    /* Hash table of ctf_str_atoms_t.  */
  ctf_dynset_t *ctf_str_pending_ref; /* Locations awaiting ref addition.  */
  uint64_t ctf_str_num_refs;	  /* Number of refs to cts_str_atoms.  */
  uint32_t ctf_str_prov_offset;	  /* Latest provisional offset assigned so far.  */
  unsigned char *ctf_base;	  /* CTF file pointer.  */
  unsigned char *ctf_dynbase;	  /* Freeable CTF file pointer. */
  unsigned char *ctf_buf;	  /* Uncompressed CTF data buffer.  */
  size_t ctf_size;		  /* Size of CTF header + uncompressed data.  */
  uint32_t *ctf_sxlate;		  /* Translation table for unindexed symtypetab
				     entries.  */
  unsigned long ctf_nsyms;	  /* Number of entries in symtab xlate table.  */
  uint32_t *ctf_txlate;		  /* Translation table for type IDs.  */
  uint32_t *ctf_ptrtab;		  /* Translation table for pointer-to lookups.  */
  size_t ctf_ptrtab_len;	  /* Num types storable in ptrtab currently.  */
  uint32_t *ctf_pptrtab;	  /* Parent types pointed to by child dicts.  */
  size_t ctf_pptrtab_len;	  /* Num types storable in pptrtab currently.  */
  uint32_t ctf_pptrtab_typemax;	  /* Max child type when pptrtab last updated.  */
  uint32_t *ctf_funcidx_names;	  /* Name of each function symbol in symtypetab
				     (if indexed).  */
  uint32_t *ctf_objtidx_names;	  /* Likewise, for object symbols.  */
  size_t ctf_nfuncidx;		  /* Number of funcidx entries.  */
  uint32_t *ctf_funcidx_sxlate;	  /* Offsets into funcinfo for a given funcidx.  */
  uint32_t *ctf_objtidx_sxlate;	  /* Likewise, for ctf_objtidx.  */
  size_t ctf_nobjtidx;		  /* Number of objtidx entries.  */
  ctf_dynhash_t *ctf_objthash;	  /* name -> type ID.  */
  ctf_dynhash_t *ctf_funchash;	  /* name -> CTF_K_FUNCTION type ID.  */

  /* The next three are linker-derived state found in ctf_link targets only.  */

  ctf_dynhash_t *ctf_dynsyms;	  /* Symbol info from ctf_link_shuffle_syms.  */
  ctf_link_sym_t **ctf_dynsymidx;  /* Indexes ctf_dynsyms by symidx.  */
  uint32_t ctf_dynsymmax;	  /* Maximum ctf_dynsym index.  */
  ctf_list_t ctf_in_flight_dynsyms; /* Dynsyms during accumulation.  */
  struct ctf_varent *ctf_vars;	  /* Sorted variable->type mapping.  */
  unsigned long ctf_nvars;	  /* Number of variables in ctf_vars.  */
  unsigned long ctf_typemax;	  /* Maximum valid type ID number.  */
  const ctf_dmodel_t *ctf_dmodel; /* Data model pointer (see above).  */
  const char *ctf_cuname;	  /* Compilation unit name (if any).  */
  char *ctf_dyncuname;		  /* Dynamically allocated name of CU.  */
  struct ctf_dict *ctf_parent;	  /* Parent CTF dict (if any).  */
  int ctf_parent_unreffed;	  /* Parent set by ctf_import_unref?  */
  const char *ctf_parlabel;	  /* Label in parent dict (if any).  */
  const char *ctf_parname;	  /* Basename of parent (if any).  */
  char *ctf_dynparname;		  /* Dynamically allocated name of parent.  */
  uint32_t ctf_parmax;		  /* Highest type ID of a parent type.  */
  uint32_t ctf_refcnt;		  /* Reference count (for parent links).  */
  uint32_t ctf_flags;		  /* Libctf flags (see below).  */
  int ctf_errno;		  /* Error code for most recent error.  */
  int ctf_version;		  /* CTF data version.  */
  ctf_dynhash_t *ctf_dthash;	  /* Hash of dynamic type definitions.  */
  ctf_list_t ctf_dtdefs;	  /* List of dynamic type definitions.  */
  ctf_dynhash_t *ctf_dvhash;	  /* Hash of dynamic variable mappings.  */
  ctf_list_t ctf_dvdefs;	  /* List of dynamic variable definitions.  */
  unsigned long ctf_dtoldid;	  /* Oldest id that has been committed.  */
  unsigned long ctf_snapshots;	  /* ctf_snapshot() plus ctf_update() count.  */
  unsigned long ctf_snapshot_lu;  /* ctf_snapshot() call count at last update.  */
  ctf_archive_t *ctf_archive;	  /* Archive this ctf_dict_t came from.  */
  ctf_list_t ctf_errs_warnings;	  /* CTF errors and warnings.  */
  ctf_dynhash_t *ctf_link_inputs; /* Inputs to this link.  */
  ctf_dynhash_t *ctf_link_outputs; /* Additional outputs from this link.  */

  /* If a link input CU, points at the corresponding per-CU output (if any);
     if an output, points at the input (if any).  */
  ctf_dict_t *ctf_link_in_out;

  /* Map input types to output types for ctf_add_type.  Key is a
     ctf_link_type_key_t: value is a type ID.  */
  ctf_dynhash_t *ctf_link_type_mapping;

  /* Map input CU names to output CTF dict names: populated in the top-level
     output dict.

     Key and value are dynamically-allocated strings.  */
  ctf_dynhash_t *ctf_link_in_cu_mapping;

  /* Map output CTF dict names to input CU names: populated in the top-level
     output dict.  A hash of string to hash (set) of strings.  Key and
     individual value members are shared with ctf_link_in_cu_mapping.  */
  ctf_dynhash_t *ctf_link_out_cu_mapping;

  /* CTF linker flags.  Set on the parent output dict (the one passed to
     ctf_link).  Only respected when LCTF_LINKING set in ctf_flags.  */
  int ctf_link_flags;

  /* Allow the caller to change the name of link archive members.  */
  ctf_link_memb_name_changer_f *ctf_link_memb_name_changer;
  void *ctf_link_memb_name_changer_arg;         /* Argument for it.  */

  /* Allow the caller to filter out variables they don't care about.  */
  ctf_link_variable_filter_f *ctf_link_variable_filter;
  void *ctf_link_variable_filter_arg;           /* Argument for it. */

  ctf_dynhash_t *ctf_add_processing; /* Types ctf_add_type is working on now.  */

  /* Atoms table for dedup string storage.  All strings in the ctf_dedup_t are
     stored here.  Only the _alloc copy is allocated or freed: the
     ctf_dedup_atoms may be pointed to some other CTF dict, to share its atoms.
     We keep the atoms table outside the ctf_dedup so that atoms can be
     preserved across multiple similar links, such as when doing cu-mapped
     links.  */
  ctf_dynset_t *ctf_dedup_atoms;
  ctf_dynset_t *ctf_dedup_atoms_alloc;

  ctf_dedup_t ctf_dedup;	  /* Deduplicator state.  */

  char *ctf_tmp_typeslice;	  /* Storage for slicing up type names.  */
  size_t ctf_tmp_typeslicelen;	  /* Size of the typeslice.  */
  void *ctf_specific;		  /* Data for ctf_get/setspecific().  */
};

/* An abstraction over both a ctf_dict_t and a ctf_archive_t.  */

struct ctf_archive_internal
{
  int ctfi_is_archive;
  int ctfi_unmap_on_close;
  ctf_dict_t *ctfi_dict;
  struct ctf_archive *ctfi_archive;
  ctf_dynhash_t *ctfi_dicts;	  /* Dicts we have opened and cached.  */
  ctf_dict_t *ctfi_crossdict_cache; /* Cross-dict caching.  */
  ctf_dict_t **ctfi_symdicts;	  /* Array of index -> ctf_dict_t *.  */
  ctf_dynhash_t *ctfi_symnamedicts; /* Hash of name -> ctf_dict_t *.  */
  ctf_sect_t ctfi_symsect;
  int ctfi_symsect_little_endian; /* -1 for unknown / do not set.  */
  ctf_sect_t ctfi_strsect;
  int ctfi_free_symsect;
  int ctfi_free_strsect;
  void *ctfi_data;
  bfd *ctfi_abfd;		  /* Optional source of section data.  */
  void (*ctfi_bfd_close) (struct ctf_archive_internal *);
};

/* Iterator state for the *_next() functions.  */

/* A single hash key/value pair.  */
typedef struct ctf_next_hkv
{
  void *hkv_key;
  void *hkv_value;
} ctf_next_hkv_t;

struct ctf_next
{
  void (*ctn_iter_fun) (void);
  ctf_id_t ctn_type;
  ssize_t ctn_size;
  ssize_t ctn_increment;
  const ctf_type_t *ctn_tp;
  uint32_t ctn_n;

  /* Some iterators contain other iterators, in addition to their other
     state.  */
  ctf_next_t *ctn_next;

  /* We can save space on this side of things by noting that a dictionary is
     either dynamic or not, as a whole, and a given iterator can only iterate
     over one kind of thing at once: so we can overlap the DTD and non-DTD
     members, and the structure, variable and enum members, etc.  */
  union
  {
    unsigned char *ctn_vlen;
    const ctf_enum_t *ctn_en;
    const ctf_dvdef_t *ctn_dvd;
    ctf_next_hkv_t *ctn_sorted_hkv;
    void **ctn_hash_slot;
  } u;

  /* This union is of various sorts of dict we can iterate over:
     currently dictionaries and archives, dynhashes, and dynsets.  */
  union
  {
    const ctf_dict_t *ctn_fp;
    const ctf_archive_t *ctn_arc;
    const ctf_dynhash_t *ctn_h;
    const ctf_dynset_t *ctn_s;
  } cu;
};

/* Return x rounded up to an alignment boundary.
   eg, P2ROUNDUP(0x1234, 0x100) == 0x1300 (0x13*align)
   eg, P2ROUNDUP(0x5600, 0x100) == 0x5600 (0x56*align)  */
#define P2ROUNDUP(x, align)		(-(-(x) & -(align)))

/* * If an offs is not aligned already then round it up and align it. */
#define LCTF_ALIGN_OFFS(offs, align) ((offs + (align - 1)) & ~(align - 1))

#define LCTF_TYPE_ISPARENT(fp, id) ((id) <= fp->ctf_parmax)
#define LCTF_TYPE_ISCHILD(fp, id) ((id) > fp->ctf_parmax)
#define LCTF_TYPE_TO_INDEX(fp, id) ((id) & (fp->ctf_parmax))
#define LCTF_INDEX_TO_TYPE(fp, id, child) (child ? ((id) | (fp->ctf_parmax+1)) : \
					   (id))

#define LCTF_INDEX_TO_TYPEPTR(fp, i) \
    ((fp->ctf_flags & LCTF_RDWR) ?					\
     &(ctf_dtd_lookup (fp, LCTF_INDEX_TO_TYPE				\
		       (fp, i, fp->ctf_flags & LCTF_CHILD))->dtd_data) : \
     (ctf_type_t *)((uintptr_t)(fp)->ctf_buf + (fp)->ctf_txlate[(i)]))

#define LCTF_INFO_KIND(fp, info)	((fp)->ctf_dictops->ctfo_get_kind(info))
#define LCTF_INFO_ISROOT(fp, info)	((fp)->ctf_dictops->ctfo_get_root(info))
#define LCTF_INFO_VLEN(fp, info)	((fp)->ctf_dictops->ctfo_get_vlen(info))
#define LCTF_VBYTES(fp, kind, size, vlen) \
  ((fp)->ctf_dictops->ctfo_get_vbytes(fp, kind, size, vlen))

#define LCTF_CHILD	0x0001	/* CTF dict is a child.  */
#define LCTF_RDWR	0x0002	/* CTF dict is writable.  */
#define LCTF_DIRTY	0x0004	/* CTF dict has been modified.  */
#define LCTF_LINKING	0x0008  /* CTF link is underway: respect ctf_link_flags.  */

extern ctf_names_t *ctf_name_table (ctf_dict_t *, int);
extern const ctf_type_t *ctf_lookup_by_id (ctf_dict_t **, ctf_id_t);
extern ctf_id_t ctf_lookup_by_rawname (ctf_dict_t *, int, const char *);
extern ctf_id_t ctf_lookup_by_rawhash (ctf_dict_t *, ctf_names_t *, const char *);
extern void ctf_set_ctl_hashes (ctf_dict_t *);

extern int ctf_symtab_skippable (ctf_link_sym_t *sym);
extern int ctf_add_funcobjt_sym (ctf_dict_t *, int is_function,
				 const char *, ctf_id_t);

extern ctf_dict_t *ctf_get_dict (ctf_dict_t *fp, ctf_id_t type);

typedef unsigned int (*ctf_hash_fun) (const void *ptr);
extern unsigned int ctf_hash_integer (const void *ptr);
extern unsigned int ctf_hash_string (const void *ptr);
extern unsigned int ctf_hash_type_key (const void *ptr);
extern unsigned int ctf_hash_type_id_key (const void *ptr);

typedef int (*ctf_hash_eq_fun) (const void *, const void *);
extern int ctf_hash_eq_integer (const void *, const void *);
extern int ctf_hash_eq_string (const void *, const void *);
extern int ctf_hash_eq_type_key (const void *, const void *);
extern int ctf_hash_eq_type_id_key (const void *, const void *);

typedef void (*ctf_hash_free_fun) (void *);

typedef void (*ctf_hash_iter_f) (void *key, void *value, void *arg);
typedef int (*ctf_hash_iter_remove_f) (void *key, void *value, void *arg);
typedef int (*ctf_hash_iter_find_f) (void *key, void *value, void *arg);
typedef int (*ctf_hash_sort_f) (const ctf_next_hkv_t *, const ctf_next_hkv_t *,
				void *arg);

extern ctf_hash_t *ctf_hash_create (unsigned long, ctf_hash_fun, ctf_hash_eq_fun);
extern int ctf_hash_insert_type (ctf_hash_t *, ctf_dict_t *, uint32_t, uint32_t);
extern int ctf_hash_define_type (ctf_hash_t *, ctf_dict_t *, uint32_t, uint32_t);
extern ctf_id_t ctf_hash_lookup_type (ctf_hash_t *, ctf_dict_t *, const char *);
extern uint32_t ctf_hash_size (const ctf_hash_t *);
extern void ctf_hash_destroy (ctf_hash_t *);

extern ctf_dynhash_t *ctf_dynhash_create (ctf_hash_fun, ctf_hash_eq_fun,
					  ctf_hash_free_fun, ctf_hash_free_fun);
extern int ctf_dynhash_insert (ctf_dynhash_t *, void *, void *);
extern void ctf_dynhash_remove (ctf_dynhash_t *, const void *);
extern size_t ctf_dynhash_elements (ctf_dynhash_t *);
extern void ctf_dynhash_empty (ctf_dynhash_t *);
extern void *ctf_dynhash_lookup (ctf_dynhash_t *, const void *);
extern int ctf_dynhash_lookup_kv (ctf_dynhash_t *, const void *key,
				  const void **orig_key, void **value);
extern void ctf_dynhash_destroy (ctf_dynhash_t *);
extern void ctf_dynhash_iter (ctf_dynhash_t *, ctf_hash_iter_f, void *);
extern void ctf_dynhash_iter_remove (ctf_dynhash_t *, ctf_hash_iter_remove_f,
				     void *);
extern void *ctf_dynhash_iter_find (ctf_dynhash_t *, ctf_hash_iter_find_f,
				    void *);
extern int ctf_dynhash_sort_by_name (const ctf_next_hkv_t *,
				     const ctf_next_hkv_t *,
				     void * _libctf_unused_);
extern int ctf_dynhash_next (ctf_dynhash_t *, ctf_next_t **,
			     void **key, void **value);
extern int ctf_dynhash_next_sorted (ctf_dynhash_t *, ctf_next_t **,
				    void **key, void **value, ctf_hash_sort_f,
				    void *);

extern ctf_dynset_t *ctf_dynset_create (htab_hash, htab_eq, ctf_hash_free_fun);
extern int ctf_dynset_insert (ctf_dynset_t *, void *);
extern void ctf_dynset_remove (ctf_dynset_t *, const void *);
extern void ctf_dynset_destroy (ctf_dynset_t *);
extern void *ctf_dynset_lookup (ctf_dynset_t *, const void *);
extern size_t ctf_dynset_elements (ctf_dynset_t *);
extern int ctf_dynset_exists (ctf_dynset_t *, const void *key,
			      const void **orig_key);
extern int ctf_dynset_next (ctf_dynset_t *, ctf_next_t **, void **key);
extern void *ctf_dynset_lookup_any (ctf_dynset_t *);

extern void ctf_sha1_init (ctf_sha1_t *);
extern void ctf_sha1_add (ctf_sha1_t *, const void *, size_t);
extern char *ctf_sha1_fini (ctf_sha1_t *, char *);

#define	ctf_list_prev(elem)	((void *)(((ctf_list_t *)(elem))->l_prev))
#define	ctf_list_next(elem)	((void *)(((ctf_list_t *)(elem))->l_next))

extern void ctf_list_append (ctf_list_t *, void *);
extern void ctf_list_prepend (ctf_list_t *, void *);
extern void ctf_list_delete (ctf_list_t *, void *);
extern void ctf_list_splice (ctf_list_t *, ctf_list_t *);
extern int ctf_list_empty_p (ctf_list_t *lp);

extern int ctf_dtd_insert (ctf_dict_t *, ctf_dtdef_t *, int flag, int kind);
extern void ctf_dtd_delete (ctf_dict_t *, ctf_dtdef_t *);
extern ctf_dtdef_t *ctf_dtd_lookup (const ctf_dict_t *, ctf_id_t);
extern ctf_dtdef_t *ctf_dynamic_type (const ctf_dict_t *, ctf_id_t);

extern int ctf_dvd_insert (ctf_dict_t *, ctf_dvdef_t *);
extern void ctf_dvd_delete (ctf_dict_t *, ctf_dvdef_t *);
extern ctf_dvdef_t *ctf_dvd_lookup (const ctf_dict_t *, const char *);

extern ctf_id_t ctf_add_encoded (ctf_dict_t *, uint32_t, const char *,
				 const ctf_encoding_t *, uint32_t kind);
extern ctf_id_t ctf_add_reftype (ctf_dict_t *, uint32_t, ctf_id_t,
				 uint32_t kind);

extern int ctf_dedup_atoms_init (ctf_dict_t *);
extern int ctf_dedup (ctf_dict_t *, ctf_dict_t **, uint32_t ninputs,
		      uint32_t *parents, int cu_mapped);
extern void ctf_dedup_fini (ctf_dict_t *, ctf_dict_t **, uint32_t);
extern ctf_dict_t **ctf_dedup_emit (ctf_dict_t *, ctf_dict_t **,
				    uint32_t ninputs, uint32_t *parents,
				    uint32_t *noutputs, int cu_mapped);
extern ctf_id_t ctf_dedup_type_mapping (ctf_dict_t *fp, ctf_dict_t *src_fp,
					ctf_id_t src_type);

extern void ctf_decl_init (ctf_decl_t *);
extern void ctf_decl_fini (ctf_decl_t *);
extern void ctf_decl_push (ctf_decl_t *, ctf_dict_t *, ctf_id_t);

_libctf_printflike_ (2, 3)
extern void ctf_decl_sprintf (ctf_decl_t *, const char *, ...);
extern char *ctf_decl_buf (ctf_decl_t *cd);

extern const char *ctf_strptr (ctf_dict_t *, uint32_t);
extern const char *ctf_strraw (ctf_dict_t *, uint32_t);
extern const char *ctf_strraw_explicit (ctf_dict_t *, uint32_t,
					ctf_strs_t *);
extern int ctf_str_create_atoms (ctf_dict_t *);
extern void ctf_str_free_atoms (ctf_dict_t *);
extern uint32_t ctf_str_add (ctf_dict_t *, const char *);
extern uint32_t ctf_str_add_ref (ctf_dict_t *, const char *, uint32_t *ref);
extern uint32_t ctf_str_add_pending (ctf_dict_t *, const char *, uint32_t *);
extern int ctf_str_move_pending (ctf_dict_t *, uint32_t *, ptrdiff_t);
extern int ctf_str_add_external (ctf_dict_t *, const char *, uint32_t offset);
extern void ctf_str_remove_ref (ctf_dict_t *, const char *, uint32_t *ref);
extern void ctf_str_rollback (ctf_dict_t *, ctf_snapshot_id_t);
extern void ctf_str_purge_refs (ctf_dict_t *);
extern ctf_strs_writable_t ctf_str_write_strtab (ctf_dict_t *);

extern struct ctf_archive_internal *
ctf_new_archive_internal (int is_archive, int unmap_on_close,
			  struct ctf_archive *, ctf_dict_t *,
			  const ctf_sect_t *symsect,
			  const ctf_sect_t *strsect, int *errp);
extern struct ctf_archive *ctf_arc_open_internal (const char *, int *);
extern void ctf_arc_close_internal (struct ctf_archive *);
extern const ctf_preamble_t *ctf_arc_bufpreamble (const ctf_sect_t *);
extern void *ctf_set_open_errno (int *, int);
extern unsigned long ctf_set_errno (ctf_dict_t *, int);
extern void ctf_flip_header (ctf_header_t *);
extern int ctf_flip (ctf_dict_t *, ctf_header_t *, unsigned char *, int);

extern ctf_dict_t *ctf_simple_open_internal (const char *, size_t, const char *,
					     size_t, size_t,
					     const char *, size_t,
					     ctf_dynhash_t *, int, int *);
extern ctf_dict_t *ctf_bufopen_internal (const ctf_sect_t *, const ctf_sect_t *,
					 const ctf_sect_t *, ctf_dynhash_t *,
					 int, int *);
extern int ctf_import_unref (ctf_dict_t *fp, ctf_dict_t *pfp);
extern int ctf_serialize (ctf_dict_t *);

_libctf_malloc_
extern void *ctf_mmap (size_t length, size_t offset, int fd);
extern void ctf_munmap (void *, size_t);
extern ssize_t ctf_pread (int fd, void *buf, ssize_t count, off_t offset);

extern void *ctf_realloc (ctf_dict_t *, void *, size_t);
extern char *ctf_str_append (char *, const char *);
extern char *ctf_str_append_noerr (char *, const char *);

extern ctf_id_t ctf_type_resolve_unsliced (ctf_dict_t *, ctf_id_t);
extern int ctf_type_kind_unsliced (ctf_dict_t *, ctf_id_t);

_libctf_printflike_ (1, 2)
extern void ctf_dprintf (const char *, ...);
extern void libctf_init_debug (void);

_libctf_printflike_ (4, 5)
extern void ctf_err_warn (ctf_dict_t *, int is_warning, int err,
			  const char *, ...);
extern void ctf_err_warn_to_open (ctf_dict_t *);
extern void ctf_assert_fail_internal (ctf_dict_t *, const char *,
				      size_t, const char *);
extern const char *ctf_link_input_name (ctf_dict_t *);

extern ctf_link_sym_t *ctf_elf32_to_link_sym (ctf_dict_t *fp, ctf_link_sym_t *dst,
					      const Elf32_Sym *src, uint32_t symidx);
extern ctf_link_sym_t *ctf_elf64_to_link_sym (ctf_dict_t *fp, ctf_link_sym_t *dst,
					      const Elf64_Sym *src, uint32_t symidx);

/* Variables, all underscore-prepended. */

extern const char _CTF_SECTION[];	/* name of CTF ELF section */
extern const char _CTF_NULLSTR[];	/* empty string */

extern int _libctf_version;	/* library client version */
extern int _libctf_debug;	/* debugging messages enabled */

#include "ctf-inlines.h"

#ifdef	__cplusplus
}
#endif

#endif /* _CTF_IMPL_H */
