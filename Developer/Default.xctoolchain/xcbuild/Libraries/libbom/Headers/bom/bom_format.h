/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */
/*
 * These structure define part of the NextSTEP/OSX BOM file format
 *
 * Initial code
 * Author: Joseph Coffland
 * Date: October, 2011
 *
 * Additional work on BOMPath & BOMTree
 * Author: Julian Devlin
 * Date: October, 2012
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * This program is in the public domain.
 */

#ifndef _BOM_FORMAT_H
#define _BOM_FORMAT_H

#include <libutil/CompilerSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER
__pragma(warning(push))
__pragma(warning(disable: 4200))
#endif

/**
 * Build of Materials format (BOM)
 *
 * BOM files are a nonstandard archive format originating in NeXTSTEP and
 * since used in OS X package installers and compiled asset archives.
 *
 * The structure of a BOM file is simple, with three major file sections
 * described by a header at the start of the file. Most fields stored in
 * BOM files are big endian.
 *
 * The first section of a BOM file is the index. This contains a list of
 * file offsets into the data section and a length, identified by index.
 * The BOM index is used for both data in the BOM format itself, as well
 * as for contents within the data section to refer to other data. After
 * the index list is the free list, which presumably lists available data
 * segments that can be filled by later index insertions.
 *
 * The next section is the variables. BOM variables have a name and an
 * index into the index section pointing to the variable's data.
 *
 * Finally, the data section contains the data pointed to by the BOM index.
 */

/**
 * BOM Tree format
 *
 * Some BOM variables contain trees, most commonly the "Paths" variable
 * found in standard BOM archives. BOM trees are a header in the variable's
 * data section with an index pointing to the root of the tree. Each item
 * in the tree has indexes pointing to both key and value data for that item.
 */

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_header {
  char magic[8]; // = BOMStore
  uint32_t version; // = 1?
  uint32_t block_count; // = 73 = 0x49?
  uint32_t index_offset; // Length of first part
  uint32_t index_length; // Length of second part
  uint32_t variables_offset;
  uint32_t trailer_len; // FIXME: What does this data at the end mean?
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_index {
  uint32_t address;
  uint32_t length;
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_index_header {
  uint32_t count; // FIXME: What is this? It is not the length of the array...
  struct bom_index index[0];
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_variable {
  uint32_t index;
  uint8_t length;
  char name[0]; // length
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_variables {
  uint32_t count; // Number of entries that follow
  // Followed by bom_variable entries
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_tree { // 21 bytes
  char magic[4]; // = "tree"
  uint32_t version;
  uint32_t child; // FIXME: Not sure about this one...
  uint32_t node_size; // byte count of each entry in the tree (BOMPaths)
  uint32_t path_count; // total number of paths in all leaves combined
  uint8_t unknown3;
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_tree_entry_indexes {
  uint32_t value_index;
  uint32_t key_index;
} LIBUTIL_PACKED_STRUCT_END;

LIBUTIL_PACKED_STRUCT_BEGIN struct bom_tree_entry {
  uint16_t is_leaf; // if 0 then this entry refers to other BOMPaths entries
  uint16_t count;  // for leaf, count of paths. for top level, (# of leafs - 1)
  uint32_t forward;  // next leaf, when there are multiple leafs
  uint32_t backward; // previous leaf, when there are multiple leafs
  struct bom_tree_entry_indexes indexes[0];
} LIBUTIL_PACKED_STRUCT_END;

#if _MSC_VER
__pragma(warning(pop))
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BOM_FORMAT_H */
