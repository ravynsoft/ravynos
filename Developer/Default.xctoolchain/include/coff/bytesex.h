#ifndef _COFF_BYTESEX_H
#define _COFF_BYTESEX_H

#include "stuff/bytesex.h"
#include "coff/base_relocs.h"
#include "coff/ms_dos_stub.h"
#include "coff/filehdr.h"
#include "coff/aouthdr.h"
#include "coff/scnhdr.h"
#include "coff/syment.h"
#include "coff/debug_directory.h"

__private_extern__ void swap_base_relocation_block_header(
    struct base_relocation_block_header *h,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_base_relocation_entry(
    struct base_relocation_entry *b,
    uint32_t n,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_ms_dos_stub(
    struct ms_dos_stub *m,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_filehdr(
    struct filehdr *f,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_aouthdr(
    struct aouthdr *a,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_aouthdr_64(
    struct aouthdr_64 *a,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_scnhdr(
    struct scnhdr *s,
    uint32_t n,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_syment(
    struct syment *s,
    uint32_t n,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_debug_directory_entry(
    struct debug_directory_entry *d,
    enum byte_sex target_byte_sex);

__private_extern__ void swap_mtoc_debug_info(
    struct mtoc_debug_info *m,
    enum byte_sex target_byte_sex);

#endif /* _COFF_BYTESEX_H */
