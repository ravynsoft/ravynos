#ifndef H_LIBCODEDIRECTORY
#define H_LIBCODEDIRECTORY

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if 0 // __has_include(<Kernel/kern/cs_blobs.h>)
#include <Kernel/kern/cs_blobs.h>
#else
// This will need to be provided for building with non-apple toolchains
#include "cs_blobs.h"
#ifndef __unused
#define __unused
#endif
#endif

__BEGIN_DECLS

typedef struct _libcd libcd;

typedef size_t libcd_write (libcd *s, size_t pos, uint8_t const *data, size_t len);
typedef size_t libcd_read_page (libcd *s, int page_no, size_t pos, size_t page_size,
                                uint8_t * const page_buf);
typedef bool libcd_signature_generator (libcd *s, void *user_ctx, size_t signature_size, uint8_t *signature_buf);
typedef void libcd_log_writer (char *stmt);

void libcd_log_none (char *stmt __unused);
void libcd_log_stderr (char *stmt);
#define libcd_log_default libcd_log_stderr

// For library versions that don't support os_log, libcd_log_os_log is equivalent to libcd_log_stderr
void libcd_log_os_log (char *stmt);

void libcd_set_log_writer (libcd_log_writer *writer);

libcd *libcd_create (size_t image_size);
void libcd_free (libcd *s);

void libcd_set_flags (libcd *s, uint32_t flags);
void libcd_set_signing_id (libcd *s, char const *signing_id);
void libcd_set_team_id (libcd *s, char const *team_id);
void libcd_set_platform_identifier (libcd *s, uint8_t platform_identifier);
void libcd_set_exec_seg (libcd *s, uint64_t base, uint64_t limit, uint64_t flags);

enum libcd_set_hash_type_ret {
    LIBCD_SET_HASH_TYPE_SUCCESS = 0,
    LIBCD_SET_HASH_TYPE_UNKNOWN,
};

enum libcd_set_hash_type_ret
libcd_set_hash_types (libcd *s, int const hash_types[], unsigned int count);

enum libcd_set_hash_type_ret
libcd_set_hash_types_for_platform_version (libcd *s, int platform, int min_version);

void libcd_set_input_fd (libcd *s, int fd);
void libcd_set_input_mem (libcd *s, uint8_t const *mem);
void libcd_set_input_func (libcd *s, libcd_read_page *read_page, void *user_ctx, bool supports_parallel_read);

void* libcd_get_input_func_ctx (libcd *s);

void libcd_set_output_fd (libcd *s, int fd);
void libcd_set_output_mem (libcd *s, uint8_t *mem, size_t limit);
void libcd_set_output_func (libcd *s, libcd_write *write, void *user_ctx, bool supports_parallel_write);
void libcd_set_output_offset(libcd *s, size_t offset);

void* libcd_get_output_func_ctx (libcd *s);

void libcd_set_disable_parallelization (libcd* s, bool disable);

#if __has_extension(blocks)
typedef size_t (^libcd_read_page_block)(libcd *s, int page_no, size_t pos, size_t page_size,
                                        uint8_t * const page_buf);
typedef size_t (^libcd_write_block)(libcd *s, size_t pos, uint8_t const *data, size_t len);
typedef bool (^libcd_signature_generator_block) (libcd *s, size_t signature_size,
                                                 uint8_t *signature_buf);

void libcd_set_input_block (libcd *s, libcd_read_page_block read_page, bool supports_parallel_read);
void libcd_set_output_block (libcd *s, libcd_write_block write, bool supports_parallel_write);
void libcd_set_signature_generator_block (libcd *s, libcd_signature_generator_block generator);
#endif

void libcd_add_raw_blob_no_copy (libcd *s, uint32_t slot, uint8_t const *data, size_t len);
void libcd_add_sslot_data_no_copy (libcd *s, uint32_t slot, uint8_t const *data, size_t len);

void libcd_add_signature_space (libcd *s, uint32_t space);
void libcd_set_signature_generator (libcd *s, libcd_signature_generator *generator, void *user_ctx);

size_t libcd_cd_size (libcd *s, uint32_t hash_type);
size_t libcd_superblob_size (libcd *s);

enum libcd_serialize_ret {
    LIBCD_SERIALIZE_SUCCESS = 0,
    LIBCD_SERIALIZE_WRITE_ERROR,
    LIBCD_SERIALIZE_READ_PAGE_ERROR,
    LIBCD_SERIALIZE_INVALID_THREAD_COUNT,
    LIBCD_SERIALIZE_SIGNATURE_FAILURE,
    LIBCD_SERIALIZE_EMPTY,
    LIBCD_SERIALIZE_NO_MEM,
};

enum libcd_serialize_ret libcd_serialize_as_type (libcd *s, uint32_t type);
enum libcd_serialize_ret libcd_serialize (libcd *s);

enum libcd_cdhash_ret {
    LIBCD_CDHASH_SUCCESS,
    LIBCD_CDHASH_INVALID_BUFFER,
    LIBCD_CDHASH_NOT_SERIALIZED,
    LIBCD_CDHASH_TYPE_NOT_FOUND,
};

enum libcd_cdhash_ret libcd_get_cdhash_for_type(libcd *s, int hash_type, uint8_t *cdhash_buf, size_t cdhash_buf_len);

enum libcd_set_linkage_ret {
    LIBCD_SET_LINKAGE_SUCCESS,
    LIBCD_SET_LINKAGE_UNKNOWN_HASH_TYPE,
};

enum libcd_set_linkage_ret libcd_set_linkage(libcd *s, int linkage_hash_type, uint8_t *linkage_hash);

__END_DECLS

#endif // H_LIBCODEDIRECTORY
