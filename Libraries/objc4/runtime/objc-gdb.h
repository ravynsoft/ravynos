/*
 * Copyright (c) 2008 Apple Inc.  All Rights Reserved.
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

#ifndef _OBJC_GDB_H
#define _OBJC_GDB_H

/* 
 * WARNING  DANGER  HAZARD  BEWARE  EEK
 * 
 * Everything in this file is for debugger and developer tool use only.
 * These will change in arbitrary OS updates and in unpredictable ways.
 * When your program breaks, you get to keep both pieces.
 */

#ifdef __APPLE_API_PRIVATE

#ifndef _OBJC_PRIVATE_H_
#   define _OBJC_PRIVATE_H_
#endif
#include <stdint.h>
#include <objc/hashtable.h>
#include <objc/maptable.h>

__BEGIN_DECLS


/***********************************************************************
* Class pointer preflighting
**********************************************************************/

// Return cls if it's a valid class, or crash.
OBJC_EXPORT Class _Nonnull
gdb_class_getClass(Class _Nonnull cls)
#if __OBJC2__
    OBJC_AVAILABLE(10.6, 3.1, 9.0, 1.0, 2.0);
#else
    OBJC_AVAILABLE(10.7, 3.1, 9.0, 1.0, 2.0);
#endif

// Same as gdb_class_getClass(object_getClass(cls)).
OBJC_EXPORT Class _Nonnull gdb_object_getClass(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 4.3, 9.0, 1.0, 2.0);

/***********************************************************************
* Class inspection
**********************************************************************/

#if __OBJC2__

// Return the raw, mangled name of cls.
OBJC_EXPORT const char * _Nonnull
objc_debug_class_getNameRaw(Class _Nullable cls)
OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

#endif

/***********************************************************************
* Class lists for heap.
**********************************************************************/

#if __OBJC2__

// Maps class name to Class, for in-use classes only. NXStrValueMapPrototype.
OBJC_EXPORT NXMapTable * _Nullable gdb_objc_realized_classes
    OBJC_AVAILABLE(10.6, 3.1, 9.0, 1.0, 2.0);

// A generation count of realized classes. Increments when new classes
// are realized. This is NOT an exact count of the number of classes.
// It's not guaranteed by how much it increases when classes are
// realized, only that it increases by something. When classes are
// removed (unloading bundles or destroying dynamically allocated
// classes) the number will also increase to signal that there has
// been a change.
OBJC_EXPORT uintptr_t objc_debug_realized_class_generation_count;

#else

// Hashes Classes, for all known classes. Custom prototype.
OBJC_EXPORT NXHashTable * _Nullable _objc_debug_class_hash
    __OSX_AVAILABLE(10.2) 
    __IOS_UNAVAILABLE __TVOS_UNAVAILABLE
    __WATCHOS_UNAVAILABLE __BRIDGEOS_UNAVAILABLE;

#endif


/***********************************************************************
* Non-pointer isa
**********************************************************************/

#if __OBJC2__

// Extract isa pointer from an isa field.
// (Class)(isa & mask) == class pointer
OBJC_EXPORT const uintptr_t objc_debug_isa_class_mask
    OBJC_AVAILABLE(10.10, 7.0, 9.0, 1.0, 2.0);

// Extract magic cookie from an isa field.
// (isa & magic_mask) == magic_value
OBJC_EXPORT const uintptr_t objc_debug_isa_magic_mask
    OBJC_AVAILABLE(10.10, 7.0, 9.0, 1.0, 2.0);
OBJC_EXPORT const uintptr_t objc_debug_isa_magic_value
    OBJC_AVAILABLE(10.10, 7.0, 9.0, 1.0, 2.0);

// Use indexed ISAs for targets which store index of the class in the ISA.
// This index can be used to index the array of classes.
OBJC_EXPORT const uintptr_t objc_debug_indexed_isa_magic_mask;
OBJC_EXPORT const uintptr_t objc_debug_indexed_isa_magic_value;

// Then these are used to extract the index from the ISA.
OBJC_EXPORT const uintptr_t objc_debug_indexed_isa_index_mask;
OBJC_EXPORT const uintptr_t objc_debug_indexed_isa_index_shift;

// And then we can use that index to get the class from this array.  Note
// the size is provided so that clients can ensure the index they get is in
// bounds and not read off the end of the array.
OBJC_EXPORT Class _Nullable objc_indexed_classes[];

// When we don't have enough bits to store a class*, we can instead store an
// index in to this array.  Classes are added here when they are realized.
// Note, an index of 0 is illegal.
OBJC_EXPORT uintptr_t objc_indexed_classes_count;

// Absolute symbols for some of the above values are in objc-abi.h.

#endif


/***********************************************************************
* Class structure decoding
**********************************************************************/
#if __OBJC2__

// Mask for the pointer from class struct to class rw data.
// Other bits may be used for flags.
// Use 0x00007ffffffffff8UL or 0xfffffffcUL when this variable is unavailable.
OBJC_EXPORT const uintptr_t objc_debug_class_rw_data_mask
    OBJC_AVAILABLE(10.13, 11.0, 11.0, 4.0, 2.0);

// The ABI version for the internal runtime representations
// lldb, CoreSymbolication and debugging tools need to know
OBJC_EXTERN const uint32_t objc_class_abi_version
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

// the maximum ABI version in existence for now
#define OBJC_CLASS_ABI_VERSION_MAX 1

// Used when objc_class_abi_version is absent or 0
struct class_rw_v0_t {
    uint32_t  flags;
    uint32_t  version;
    uintptr_t ro;                   // class_ro_t
    uintptr_t methods;              // method_array_t
    uintptr_t properties;           // property_array_t
    uintptr_t protocols;            // protocol_array_t
    Class _Nullable firstSubclass;
    Class _Nullable nextSiblingClass;
    char *_Nullable demangledName;

    // uint32_t index;              // only when indexed-isa is used
};

// Used when objc_class_abi_version is 1
struct class_rw_v1_t {
    uint32_t  flags;
    uint16_t  witness;
    uint16_t  index;                // only when indexed-isa is used
    uintptr_t ro_or_rw_ext;         // tagged union based on the low bit:
                                    // 0: class_ro_t  1: class_rw_ext_t
    Class _Nullable firstSubclass;
    Class _Nullable nextSiblingClass;
};

struct class_rw_ext_v1_t {
    uintptr_t ro;                   // class_ro_t
    uintptr_t methods;              // method_array_t
    uintptr_t properties;           // property_array_t
    uintptr_t protocols;            // protocol_array_t
    char *_Nullable demangledName;
    uint32_t version;
};

#endif


/***********************************************************************
* Tagged pointer decoding
**********************************************************************/
#if __OBJC2__

// Basic tagged pointers (7 classes, 60-bit payload).

// if (obj & mask) obj is a tagged pointer object
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_mask
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

// tagged pointers are obfuscated by XORing with a random value
// decoded_obj = (obj ^ obfuscator)
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_obfuscator
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);


// tag_slot = (obj >> slot_shift) & slot_mask
OBJC_EXPORT unsigned int objc_debug_taggedpointer_slot_shift
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_slot_mask
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

// class = classes[tag_slot]
OBJC_EXPORT Class _Nullable objc_debug_taggedpointer_classes[]
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

// payload = (decoded_obj << payload_lshift) >> payload_rshift
// Payload signedness is determined by the signedness of the right-shift.
OBJC_EXPORT unsigned int objc_debug_taggedpointer_payload_lshift
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);
OBJC_EXPORT unsigned int objc_debug_taggedpointer_payload_rshift
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);


// Extended tagged pointers (255 classes, 52-bit payload).

// If you interrogate an extended tagged pointer using the basic 
// tagged pointer scheme alone, it will appear to have an isa 
// that is either nil or class __NSUnrecognizedTaggedPointer.

// if (ext_mask != 0  &&  (decoded_obj & ext_mask) == ext_mask)
//   obj is a ext tagged pointer object
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_ext_mask
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

// ext_tag_slot = (obj >> ext_slot_shift) & ext_slot_mask
OBJC_EXPORT unsigned int objc_debug_taggedpointer_ext_slot_shift
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_ext_slot_mask
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

// class = ext_classes[ext_tag_slot]
OBJC_EXPORT Class _Nullable objc_debug_taggedpointer_ext_classes[]
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

// payload = (decoded_obj << ext_payload_lshift) >> ext_payload_rshift
// Payload signedness is determined by the signedness of the right-shift.
OBJC_EXPORT unsigned int objc_debug_taggedpointer_ext_payload_lshift
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);
OBJC_EXPORT unsigned int objc_debug_taggedpointer_ext_payload_rshift
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

#endif


/***********************************************************************
* Swift marker bits
**********************************************************************/
#if __OBJC2__
OBJC_EXPORT const uintptr_t objc_debug_swift_stable_abi_bit
OBJC_AVAILABLE(10.14.4, 12.2, 12.2, 5.2, 3.2);
#endif



/***********************************************************************
* AutoreleasePoolPage
**********************************************************************/
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_magic_offset  OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_next_offset   OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_thread_offset OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_parent_offset OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_child_offset  OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_depth_offset  OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
OBJC_EXTERN const uint32_t objc_debug_autoreleasepoolpage_hiwat_offset  OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

__END_DECLS

// APPLE_API_PRIVATE
#endif

// _OBJC_GDB_H
#endif
