#ifndef _EFI_DEBUG_DIRECTORY_H_
#define _EFI_DEBUG_DIRECTORY_H_
/*
 * These data structures are discribed in the pecoff_v8.doc in section
 * "6.1.1. Debug Directory (Image Only)"
 */
#include <stdint.h>

 
/*
 * 6.1.1 Debug Directory (Image Only)
 * Image files contain an optional debug directory that indicates what form of
 * debug information is present and where it is. This directory consists of an
 * array of debug directory entries whose location and size are indicated in
 * the image optional header.
 * 
 * The debug directory can be in a discardable .debug section (if one exists),
 * or it can be included in any other section in the image file, or not be in
 * a section at all.
 * 
 * Each debug directory entry identifies the location and size of a block of
 * debug information. The specified RVA can be zero if the debug information is 
 * not covered by a section header (that is, it resides in the image file and
 * is not mapped into the run-time address space). If it is mapped, the RVA is
 * its address. 
 *
 * A debug directory entry has the following format:
 * Offset	Size	Field		Description
 * 0		4	Characteristics	Reserved, must be zero.
 * 4		4	TimeDateStamp	The time and date that the debug data
 *					was created.
 * 8		2	MajorVersion	The major version number of the debug
 *					data format.
 * 10		2	MinorVersion	The minor version number of the debug
 *					data format.
 * 12		4	Type		The format of debugging information.
 *					This field enables support of multiple
 *					debuggers. For more information, see
 *					section 6.1.2, "Debug Type."
 * 16		4	SizeOfData	The size of the debug data (not
 *					including the debug directory itself).
 * 20		4	AddressOfRawData The address of the debug data when
 *					loaded, relative to the image base.
 * 24		4	PointerToRawData The file pointer to the debug data.
 */
struct debug_directory_entry {
    uint32_t	Characteristics;
    uint32_t	TimeDateStamp;
    uint16_t	MajorVersion;
    uint16_t	MinorVersion;
    uint32_t	Type;
    uint32_t	SizeOfData;
    uint32_t	AddressOfRawData;
    uint32_t	PointerToRawData;
};

/*
 * For mtoc(1)'s -d option it uses a debug directory entry with the Type of
 * IMAGE_DEBUG_TYPE_CODEVIEW pointing to the structure below.
 */
#define IMAGE_DEBUG_TYPE_CODEVIEW 2

/*
 * This structure is directly followed by a null terminated filename of the -d
 * argument file passed to the motc(1) command.
 */
struct mtoc_debug_info {
    uint32_t	Signature;	/* this is be 'MTOC' or 0x434f544d */
    uint8_t     uuid[16];       /* the 128-bit uuid from the LC_UUID command */
    /* argument of the -d mtoc(1) command follows directly after */
};
#define MTOC_SIGNATURE	0x434f544d /* the value of the characters 'MTOC' */

#endif /* _EFI_DEBUG_DIRECTORY_H_ */
