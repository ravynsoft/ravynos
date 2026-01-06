#ifndef _STUFF_LTO_H_
#define _STUFF_LTO_H_

#include "stuff/arch.h"

#ifdef LTO_SUPPORT

__private_extern__ int is_llvm_bitcode_from_memory(
    char *addr,
    uint32_t size,
    struct arch_flag *arch_flag,
    void **mod); /* maybe NULL */

__private_extern__ uint32_t lto_get_nsyms(
    void *mod);

__private_extern__ int lto_toc_symbol(
    void *mod,
    uint32_t symbol_index,
    int commons_in_toc);

__private_extern__ void lto_get_nlist_64(
    struct nlist_64 *nl,
    void *mod,
    uint32_t symbol_index);

__private_extern__ const char * lto_symbol_name(
    void *mod,
    uint32_t symbol_index);

__private_extern__ void lto_free(
    void *mod);

#endif /* LTO_SUPPORT */

#endif /* _STUFF_LTO_H_ */
