#ifndef _EFI_BASE_RELOCS_H_
#define _EFI_BASE_RELOCS_H_
/*
 * These data structures are discribed in the pecoff_v8.doc in section
 * "6.6. The .reloc Section (Image Only)".
 */
#include <stdint.h>

 
/*
 * 6.6. The .reloc Section (Image Only)
 * The base relocation table contains entries for all base relocations in the
 * image.  (For PECOFF) The Base Relocation Table field in the optional header
 * data directories gives the number of bytes in the base relocation table. For 
 * more information, see section 3.4.3, "Optional Header Data Directories
 * (Image Only)."
 *
 * For Mach-O files being conveteted to PECOFF files the relocation table
 * entries are placed in the (__RELOC,__reloc) section after they are created
 * by makerelocs(1) and offset and size are found in the Mach-O section header
 * for this section.
 *
 * The base relocation table is divided into blocks. Each block represents the
 * base relocations for a 4K page. Each block must start on a 32-bit boundary.
 * The loader is not required to process base relocations that are resolved by
 * the linker, unless the load image cannot be loaded at the image base that is
 * specified in the PE header.
 * 
 * 6.6.1. Base Relocation Block
 * Each base relocation block starts with the following structure:
 * Offset	Size	Field	Description
 * 0		4	Page RVA	The image base plus the page RVA is
 *					added to each offset to create the VA
 *					where the base relocation must be
 *					applied.
 * 4		4	Block Size	The total number of bytes in the base
 *					relocation block, including the Page
 *					RVA and Block Size fields and the
 *					Type/Offset fields that follow.
 */
struct base_relocation_block_header {
        uint32_t page_rva;
        uint32_t block_size;
};

/*
 * From "6.6.1. Base Relocation Block""
 * The Block Size field is then followed by any number of Type or Offset field
 * entries. Each entry is a WORD (2 bytes) and has the following structure:
 * Offset	Size	Field	Description
 * 0		4 bits	Type	Stored in the high 4 bits of the WORD, a value
 *				that indicates the type of base relocation to
 *				be applied. For more information, see section
 *				6.6.2, "Base Relocation Types."
 * 0		12 bits	Offset	Stored in the remaining 12 bits of the WORD, an
 *				offset from the starting address that was
 *				specified in the Page RVA field for the block.
 *				This offset specifies where the base relocation
 *				is to be applied.
 */
struct base_relocation_entry {
#if __BIG_ENDIAN__
        uint16_t type:4,
		 offset:12;
#else
        uint16_t offset:12,
		 type:4;
#endif
};

#define	IMAGE_REL_BASED_ABSOLUTE	0 /* The base relocation is skipped.
					     This type can be used to pad a
					     block. */
#define IMAGE_REL_BASED_HIGHLOW		3 /* The base relocation applies all
					     32 bits of the difference to the
					     32-bit field at offset. */
#define IMAGE_REL_BASED_DIR64	       10 /* The base relocation applies the
					     difference to the 64-bit field at
					     offset. */
#endif /* _EFI_BASE_RELOCS_H_ */
