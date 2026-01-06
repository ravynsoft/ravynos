/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
/* GDB symbol table format definitions.
   Copyright (C) 1986 Free Software Foundation, Inc.

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

/* Format of GDB symbol table data.
   There is one symbol segment for each source file or
   independant compilation.  These segments are simply concatenated
   to form the GDB symbol table.  A zero word where the beginning
   of a segment is expected indicates there are no more segments.

Format of a symbol segment:

   The symbol segment begins with a word containing 1
   if it is in the format described here.  Other formats may
   be designed, with other code numbers.

   The segment contains many objects which point at each other.
   The pointers are offsets in bytes from the beginning of the segment.
   Thus, each segment can be loaded into core and its pointers relocated
   to make valid in-core pointers.

   All the data objects in the segment can be found indirectly from
   one of them, the root object, of type `struct symbol_root'.
   It appears at the beginning of the segment.

   The total size of the segment, in bytes, appears as the `length'
   field of this object.  This size includes the size of the
   root object.

   All the object data types are defined here to contain pointer types
   appropriate for in-core use on a relocated symbol segment.
   Casts to and from type int are required for working with
   unrelocated symbol segments such as are found in the file.

   The ldsymaddr word is filled in by the loader to contain
   the offset (in bytes) within the ld symbol table
   of the first nonglobal symbol from this compilation.
   This makes it possible to match those symbols
   (which contain line number information) reliably with
   the segment they go with.

   Core addresses within the program that appear in the symbol segment
   are not relocated by the loader.  They are inserted by the assembler
   and apply to addresses as output by the assembler, so GDB must
   relocate them when it loads the symbol segment.  It gets the information
   on how to relocate from the textrel, datarel, bssrel, databeg and bssbeg
   words of the root object.

   The words textrel, datarel and bssrel
   are filled in by ld with the amounts to relocate within-the-file
   text, data and bss addresses by; databeg and bssbeg can be
   used to tell which kind of relocation an address needs.  */

enum language {language_c};

/*
 * All symbol roots must have as their first two fields format and length
 * fields.  The total length of the symbol root must be a multiple of
 * sizeof(uint32_t) and any padding must be zeroed.
 */
struct symbol_root_header
{
  int format;	/* type of symbol segment */
  int length;	/* # bytes in this symbol segment, rounded to sizeof(uint32_t) */
};

/*
 * Constants for symbol root format fields
 */
#define SYMBOL_ROOT_FORMAT	1
#define INDIRECT_ROOT_FORMAT	1002
#define COMMON_ROOT_FORMAT	1003
#define SHLIB_ROOT_FORMAT	1004
#define ALIAS_ROOT_FORMAT	1005
#define MACH_ROOT_FORMAT		2001
#define MACH_INDIRECT_ROOT_FORMAT	2002
#define MACH_SHLIB_ROOT_FORMAT		2004


struct symbol_root
{
  int format;			/* SYMBOL_ROOT_FORMAT */
  int length;			/* # bytes in this symbol segment */
  int ldsymoff;			/* Offset in ld symtab of this file's syms */
  int textrel;			/* Relocation for text addresses */
  int datarel;			/* Relocation for data addresses */
  int bssrel;			/* Relocation for bss addresses */
  char *filename;		/* Name of main source file compiled */
  char *filedir;		/* Name of directory it was reached from */
  struct blockvector *blockvector; /* Vector of all symbol-naming blocks */
  struct typevector *typevector; /* Vector of all data types */
  enum language language;	/* Code identifying the language used */
  char *version;		/* Version info.  Not fully specified */
  char *compilation;		/* Compilation info.  Not fully specified */
  int databeg;			/* Address within the file of data start */
  int bssbeg;			/* Address within the file of bss start */
  struct sourcevector *sourcevector; /* Vector of line-number info */
};

struct mach_root
{
  int format;			/* MACH_ROOT_FORMAT */
  int length;			/* # bytes in this symbol segment */
  int ldsymoff;			/* Offset in ld symtab of this file's syms */
  struct loadmap *loadmap;	/* load map of the relocatable object */
  char *filename;		/* Name of main source file compiled */
  char *filedir;		/* Name of directory it was reached from */
  struct blockvector *blockvector; /* Vector of all symbol-naming blocks */
  struct typevector *typevector; /* Vector of all data types */
  enum language language;	/* Code identifying the language used */
  char *version;		/* Version info.  Not fully specified */
  char *compilation;		/* Compilation info.  Not fully specified */
  struct sourcevector *sourcevector; /* Vector of line-number info */
};

/*
 * Indirect symbol root format.  Written by ld when -g is used (the default).
 * This is for lazy evaluation of the -gg symbol segments.
 */
struct indirect_root {
  int format;		/* INDIRECT_ROOT_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int ldsymoff;		/* Offset in ld symtab of this file's syms */
  int textrel;		/* Relocation for text addresses */
  int datarel;		/* Relocation for data addresses */
  int bssrel;		/* Relocation for bss addresses */
  int textsize;		/* text size */
  int datasize;		/* data size */
  int bsssize;		/* bss size */
  int mtime;		/* last modified time, as returned by stat(2) */
  int fileoffset;	/* Offset in file that contains symbol_root */
  char filename[1];	/* variable length file name, zero padded */
};

/*
 * Mach indirect symbol root format.  Written by ld when -g is used (the
 * default).  This is for lazy evaluation of the -gg symbol segments.
 */
struct mach_indirect_root {
  int format;		/* MACH_INDIRECT_ROOT_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int ldsymoff;		/* Offset in ld symtab of this file's syms */
  struct loadmap *loadmap; /* load map of the relocatable object */
  int mtime;		/* last modified time, as returned by stat(2) */
  int fileoffset;	/* Offset in relocatable file that contains the 
			   mach_root */
  char filename[1];	/* variable length file name, zero padded */
};

/*
 * common symbol root format.  For each common symbol that the link editor
 * defines the storage for that symbol name is recorded in here.
 */
struct common_root {
  int format;		/* COMMON_SYM_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int nsyms;		/* the number of strings in the data[] field for the
			   common symbols names of this file */
  char data[1];		
  /* Data looks like the following:
    - Null terminated string for the filename.
	- Null terminated stings for syms.
	...
    - zero padded to round to sizeof(uint32_t)
   */
};
 
/*
 * shlib_root: Written by ld for target shared library output.  This has two
 * fields for each of the data segment fields. The data segments of .o files
 * that go into target shared libraries have all their static data first in
 * the data segment followed by all the global data.  When it is loaded into
 * a target shared library the global data from all the .o files is placed
 * first in the data segment then all of the static data.  So this information
 * is reflected in the {global,static}datarel and the {global,static}databeg
 * fields.
 *
 * After one of these has been written for each object in the shared library
 * then the symbol root from each object is written into the shared library.
 */
struct shlib_root {
  int format;		/* SHLIB_ROOT_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int ldsymoff;		/* Offset in ld symtab of this file's syms */
  int textrel;		/* Relocation for text addresses */
  int globaldatarel;	/* Relocation for global data addresses */
  int staticdatarel;	/* Relocation for static data addresses */
  int globaldatabeg;	/* Address of the global data start */
  int staticdatabeg;	/* Address of the static data start */
  int globaldatasize;	/* global data size */
  int staticdatasize;	/* static data size */
  int symreloffset;	/* relitive offset, from the first SYMBOL_ROOT_FORMAT
			   of the symbol root for this file */
  char filename[1];	/* variable length file name, zero padded */
};

struct mach_shlib_root {
  int format;		/* MACH_SHLIB_ROOT_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int ldsymoff;		/* Offset in ld symtab of this file's syms */
  struct loadmap *loadmap; /* load map of the relocatable object */
  int symreloffset;	/* relitive offset, from the first SYMBOL_ROOT_FORMAT
			   of the symbol root for this file */
  char filename[1];	/* variable length file name, zero padded */
};

/*
 * This format is used when the link-editor alias option -a original:alias
 * is used when producing an output file.  This option changes symbols in
 * the .o file from 'original' to 'alias' in the a.out file.
 */
struct alias_root {
  int format;		/* ALIAS_SYM_FORMAT */
  int length;		/* length of this struct, rounded to sizeof(uint32_t) */
  int naliases;		/* number of pairs of aliased symbols */
  char data[1];		
  /* Data looks like the following:
	- Pairs of:
	    - Null terminated string for the original symbol
	    - Null terminated string for the aliased symbol
	- zero padded to round to sizeof(uint32_t)
   */
};

/*
 * The load map describes where the parts the relocatable object have been
 * loaded in the executable.  The enitre address space of the relocatable
 * is to be covered by all the map entries.  There may be multiple map entries
 * for a single section or one map entry for multiple sections.  This allows
 * the link editor to scatter load a section based on information that improves
 * performance by increasing the locality of reference.
 */
struct loadmap
{
  /* Number of maps in the list.  */
  int nmaps;
  /* The maps themselves.  */
  struct map *map[1];
};
struct map
{
  /* The starting address in the relocatable object and size of part of the
     object file. */
  int reladdr, size;
  /* The address the loader loaded this part of the object file at */
  int ldaddr;	
};


/* All data types of symbols in the compiled program
   are represented by `struct type' objects.
   All of these objects are pointed to by the typevector.
   The type vector may have empty slots that contain zero.  */

struct typevector
{
  int length;			/* Number of types described */
  struct type *type[1];
};

/* Different kinds of data types are distinguished by the `code' field.  */

enum type_code
{
  TYPE_CODE_UNDEF,		/* Not used; catches errors */
  TYPE_CODE_PTR,		/* Pointer type */
  TYPE_CODE_ARRAY,		/* Array type, lower bound zero */
  TYPE_CODE_STRUCT,		/* C struct or Pascal record */
  TYPE_CODE_UNION,		/* C union or Pascal variant part */
  TYPE_CODE_ENUM,		/* Enumeration type */
  TYPE_CODE_FUNC,		/* Function type */
  TYPE_CODE_INT,		/* Integer type */
  TYPE_CODE_FLT,		/* Floating type */
  TYPE_CODE_VOID,		/* Void type (values zero length) */
  TYPE_CODE_SET,		/* Pascal sets */
  TYPE_CODE_RANGE,		/* Range (integers within spec'd bounds) */
  TYPE_CODE_PASCAL_ARRAY,	/* Array with explicit type of index */
};

/* This appears in a type's flags word for an unsigned integer type.  */
#define TYPE_FLAG_UNSIGNED 1

/* Other flag bits are used with GDB.  */

struct type
{
  /* Code for kind of type */
  enum type_code code;
  /* Name of this type, or zero if none.
     This is used for printing only.
     Type names specified as input are defined by symbols.  */
  char *name;
  /* Length in bytes of storage for a value of this type */
  int length;
  /* For a pointer type, describes the type of object pointed to.
     For an array type, describes the type of the elements.
     For a function type, describes the type of the value.
     Unused otherwise.  */
  struct type *target_type;
  /* Type that is a pointer to this type.
     Zero if no such pointer-to type is known yet.
     The debugger may add the address of such a type
     if it has to construct one later.  */ 
  struct type *pointer_type;
  /* Type that is a function returning this type.
     Zero if no such function type is known here.
     The debugger may add the address of such a type
     if it has to construct one later.  */
  struct type *function_type;
  /* Flags about this type.  */
  short flags;
  /* Number of fields described for this type */
  short nfields;
  /* For structure and union types, a description of each field.
     For set and pascal array types, there is one "field",
     whose type is the domain type of the set or array.
     For range types, there are two "fields",
     the minimum and maximum values (both inclusive).
     For enum types, each possible value is described by one "field".
     For range types, there are two "fields", that record constant values
     (inclusive) for the minimum and maximum.

     Using a pointer to a separate array of fields
     allows all types to have the same size, which is useful
     because we can allocate the space for a type before
     we know what to put in it.  */
  struct field
    {
      /* Position of this field, counting in bits from start of
	 containing structure.  For a function type, this is the
	 position in the argument list of this argument.
	 For a range bound or enum value, this is the value itself.  */
      int bitpos;
      /* Size of this field, in bits, or zero if not packed.
	 For an unpacked field, the field's type's length
	 says how many bytes the field occupies.  */
      int bitsize;
      /* In a struct or enum type, type of this field.
	 In a function type, type of this argument.
	 In an array type, the domain-type of the array.  */
      struct type *type;
      /* Name of field, value or argument.
	 Zero for range bounds and array domains.  */
      char *name;
    } *fields;
};

/* All of the name-scope contours of the program
   are represented by `struct block' objects.
   All of these objects are pointed to by the blockvector.

   Each block represents one name scope.
   Each lexical context has its own block.

   The first two blocks in the blockvector are special.
   The first one contains all the symbols defined in this compilation
   whose scope is the entire program linked together.
   The second one contains all the symbols whose scope is the
   entire compilation excluding other separate compilations.
   In C, these correspond to global symbols and static symbols.

   Each block records a range of core addresses for the code that
   is in the scope of the block.  The first two special blocks
   give, for the range of code, the entire range of code produced
   by the compilation that the symbol segment belongs to.

   The blocks appear in the blockvector
   in order of increasing starting-address,
   and, within that, in order of decreasing ending-address.

   This implies that within the body of one function
   the blocks appear in the order of a depth-first tree walk.  */

struct blockvector
{
  /* Number of blocks in the list.  */
  int nblocks;
  /* The blocks themselves.  */
  struct block *block[1];
};

struct block
{
  /* Addresses in the executable code that are in this block.
     Note: in an unrelocated symbol segment in a file,
     these are always zero.  They can be filled in from the
     N_LBRAC and N_RBRAC symbols in the loader symbol table.  */
  int startaddr, endaddr;
  /* The symbol that names this block,
     if the block is the body of a function;
     otherwise, zero.
     Note: In an unrelocated symbol segment in an object file,
     this field may be zero even when the block has a name.
     That is because the block is output before the name
     (since the name resides in a higher block).
     Since the symbol does point to the block (as its value),
     it is possible to find the block and set its name properly.  */
  struct symbol *function;
  /* The `struct block' for the containing block, or 0 if none.  */
  /* Note that in an unrelocated symbol segment in an object file
     this pointer may be zero when the correct value should be
     the second special block (for symbols whose scope is one compilation).
     This is because the compiler ouptuts the special blocks at the
     very end, after the other blocks.   */
  struct block *superblock;
  /* Number of local symbols.  */
  int nsyms;
  /* The symbols.  */
  struct symbol *sym[1];
};

/* Represent one symbol name; a variable, constant, function or typedef.  */

/* Different name spaces for symbols.  Looking up a symbol specifies
   a namespace and ignores symbol definitions in other name spaces.

   VAR_NAMESPACE is the usual namespace.
   In C, this contains variables, function names, typedef names
   and enum type values.

   STRUCT_NAMESPACE is used in C to hold struct, union and enum type names.
   Thus, if `struct foo' is used in a C program,
   it produces a symbol named `foo' in the STRUCT_NAMESPACE.

   LABEL_NAMESPACE may be used for names of labels (for gotos);
   currently it is not used and labels are not recorded at all.  */

/* For a non-global symbol allocated statically,
   the correct core address cannot be determined by the compiler.
   The compiler puts an index number into the symbol's value field.
   This index number can be matched with the "desc" field of
   an entry in the loader symbol table.  */

enum namespace
{
  UNDEF_NAMESPACE, VAR_NAMESPACE, STRUCT_NAMESPACE, LABEL_NAMESPACE,
};

/* An address-class says where to find the value of the symbol in core.  */

enum address_class
{
  LOC_UNDEF,		/* Not used; catches errors */
  LOC_CONST,		/* Value is constant int */
  LOC_STATIC,		/* Value is at fixed address */
  LOC_REGISTER,		/* Value is in register */
  LOC_ARG,		/* Value is at spec'd position in arglist */
  LOC_LOCAL,		/* Value is at spec'd pos in stack frame */
  LOC_TYPEDEF,		/* Value not used; definition in SYMBOL_TYPE
			   Symbols in the namespace STRUCT_NAMESPACE
			   all have this class.  */
  LOC_LABEL,		/* Value is address in the code */
  LOC_BLOCK,		/* Value is address of a `struct block'.
			   Function names have this class.  */
  LOC_EXTERNAL,		/* Value is at address not in this compilation.
			   This is used for .comm symbols
			   and for extern symbols within functions.
			   Inside GDB, this is changed to LOC_STATIC once the
			   real address is obtained from a loader symbol.  */
  LOC_CONST_BYTES	/* Value is a constant byte-sequence.   */
};

struct symbol
{
  /* Symbol name */
  char *name;
  /* Name space code.  */
  enum namespace namespace;
  /* Address class */
  enum address_class class;
  /* Data type of value */
  struct type *type;
  /* constant value, or address if static, or register number,
     or offset in arguments, or offset in stack frame.  */
  union
    {
      long value;
      struct block *block;      /* for LOC_BLOCK */
      char *bytes;		/* for LOC_CONST_BYTES */
    }
  value;
};

/* Source-file information.
   This describes the relation between source files and line numbers
   and addresses in the program text.  */

struct sourcevector
{
  int length;			/* Number of source files described */
  struct source *source[1];	/* Descriptions of the files */
};

/* Each item is either minus a line number, or a program counter.
   If it represents a line number, that is the line described by the next
   program counter value.  If it is positive, it is the program
   counter at which the code for the next line starts.

   Consecutive lines can be recorded by program counter entries
   with no line number entries between them.  Line number entries
   are used when there are lines to skip with no code on them.
   This is to make the table shorter.  */

struct linetable
  {
    int nitems;
    int item[1];
  };

/* All the information on one source file.  */

struct source
{
  char *name;			/* Name of file */
  struct linetable contents;
};
