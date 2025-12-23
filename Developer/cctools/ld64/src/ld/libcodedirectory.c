
#ifdef __linux__
#define __unused __attribute__((unused))
#define htonll htonl
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <arpa/inet.h>

#include <sys/mman.h>
#include <sys/queue.h>

#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha1.h>
#include <corecrypto/ccsha2.h>

#define LIBCD_HAS_PLATFORM_VERSION 1
#include "libcodedirectory.h"

#define _libcd_err(msg, ...) _libcd_err_log("%s: " msg "\n", __func__, ##__VA_ARGS__)

#if LIBCD_PARALLEL
#include <dispatch/dispatch.h>
#endif

#if LIBCD_HAS_PLATFORM_VERSION
#include <mach-o/loader.h>
#endif

#if __has_extension(blocks)
#define _libcd_block __block
#else
#define _libcd_block
#endif

#if TESTING
#include <fcntl.h>
// not POSIX
#include <err.h>
#include <sysexits.h>
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define bl htonl

/* As of the time of writing this, code signing always uses
 * 4096 byte pages, even on devices with higher native page
 * size. */
static const int _cs_page_shift = 12;
static const int _cs_page_bytes = 1 << _cs_page_shift;

typedef struct _libcd libcd;

struct _raw_blob {
    SLIST_ENTRY(_raw_blob) entries;

    uint32_t slot;
    uint8_t const *data;
    size_t len;
};

struct _sslot_data {
    SLIST_ENTRY(_sslot_data) entries;

    uint32_t slot;
    uint8_t const *data;
    size_t len;
};

static const int _default_hash_types[] = { CS_HASHTYPE_SHA256 };
static unsigned int const _default_hash_type_count = 1;

enum _iomethod_t {
    LIBCD_IO_INVALID,
    LIBCD_IO_FD,
    LIBCD_IO_MEM,
    LIBCD_IO_CUSTOM,
#if __has_extension(blocks)
    LIBCD_IO_BLOCK,
#endif
};

typedef struct  {
    int hash_type;
    uint8_t cdhash[CS_CDHASH_LEN];
    bool set;
} _cdhash_for_hash_type_t;

struct _libcd {
    size_t image_size;

    libcd_write *write;
    enum _iomethod_t write_method;

    union {
        int fd;
        struct {
            uint8_t *start;
            uint8_t *end;
        } mem;
        struct {
            libcd_write *write_func;
            void *user_ctx; // Not happy with what we provide? Make your own!
        } func;
#if __has_extension(blocks)
        libcd_write_block block;
#endif
    } write_ctx;
    off_t write_pos;
    off_t write_offset;
    bool parallel_write;

    libcd_read_page *read_page;
    enum _iomethod_t read_page_method;

    union {
        struct {
            int fd;
        } fd;
        struct {
            uint8_t const *start;
        } mem;
#if __has_extension(blocks)
        libcd_read_page_block block;
#endif
        void *user_ctx; // Not happy with what we provide? Make your own!
    } read_ctx;
    bool parallel_read;

    bool parallelization_disabled;

    uint32_t flags;

    char const *signing_id;

    SLIST_HEAD(, _raw_blob) raw_blobs;
    SLIST_HEAD(, _sslot_data) sslot_data;
    uint32_t special_slot_count;

    size_t signature_space;
    enum _iomethod_t signature_method;
    union {
        struct {
            libcd_signature_generator* generator;
            void* ctx;
        } signature_func;
#if __has_extension(blocks)
        libcd_signature_generator_block signature_block;
#endif
    } signature_ctx;

    int *hash_types;
    unsigned int hash_types_count;
    _cdhash_for_hash_type_t* cdhashes;

    uint8_t platform_identifier;

    char const *team_id;

    uint64_t exec_seg_base;
    uint64_t exec_seg_limit;
    uint64_t exec_seg_flags;

    bool linkage_set;
    uint8_t linkage_hash_type;
    uint8_t linkage_hash[CS_CDHASH_LEN];
};

#if LIBCD_LOG_OS_LOG
#include <os/log.h>
#endif

void
libcd_log_os_log (char *stmt)
{
#if LIBCD_LOG_OS_LOG
    static os_log_t __unsafe_unretained my_log_handle;
    if (my_log_handle == nil) {
        my_log_handle = os_log_create("com.apple.libcodedirectory", "libcodedirectory");
    }

    os_log_error(my_log_handle, stmt);
#else
    libcd_log_stderr(stmt);
#endif
}


void
libcd_log_stderr (char *stmt)
{
    fprintf(stderr, "%s", stmt);
}

void
libcd_log_none (char *stmt __unused)
{}

static libcd_log_writer *_configured_log_writer = libcd_log_default;

#if LIBCD_PARALLEL
static dispatch_queue_t
_libcd_get_log_queue (void)
{
    static dispatch_once_t once = 0;
    static dispatch_queue_t log_queue = NULL;
    dispatch_once(&once, ^{
        log_queue = dispatch_queue_create("libcd_log", DISPATCH_QUEUE_SERIAL);
    });
    return log_queue;
}
#endif

void
libcd_set_log_writer (libcd_log_writer *writer)
{
    if (writer) {
#if LIBCD_PARALLEL
        dispatch_queue_t queue = _libcd_get_log_queue();
        dispatch_sync(queue, ^{
#endif
            _configured_log_writer = writer;
#if LIBCD_PARALLEL
        });
#endif
    }
}

static void
_libcd_err_log (const char* fmt, ...)
{
    char* stmt = NULL;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(&stmt, fmt, ap);
    va_end(ap);
    if (stmt) {
#if LIBCD_PARALLEL
        dispatch_queue_t queue = _libcd_get_log_queue();
        dispatch_sync(queue, ^{
#endif
            _configured_log_writer(stmt);
#if LIBCD_PARALLEL
        });
#endif
        free(stmt);
    }
}

static size_t
_libcd_write_fd (libcd *s, size_t pos, uint8_t const *data, size_t len)
{
    pos += s->write_offset;
    ssize_t written = pwrite(s->write_ctx.fd, data, len, pos);
    if (written == -1) {
        _libcd_err("failed pwrite (pos: %d len: %d): error (%d)", pos, len, errno);
        return 0;
    }
    s->write_pos += written;

    return written;
}

static size_t
_libcd_write_mem (libcd *s, size_t pos, uint8_t const *data, size_t len)
{
    uint8_t * const start = s->write_ctx.mem.start;
    uint8_t * const end = s->write_ctx.mem.end;
    pos += s->write_offset;

    if (pos + len > (size_t)(end-start)) {
        _libcd_err("write beyond end: %zu > %zu", pos+len, end-start);
        return 0;
    }

    memcpy(start+pos, data, len);
    s->write_pos += len;

    return len;
}

static size_t
_libcd_read_page_fd (libcd *s, int page_no, size_t pos, size_t page_size, uint8_t * const page_buf)
{
    size_t bytes_read = 0;
    assert(page_size == _cs_page_bytes);
    memset(page_buf, 0, page_size);

    if (pos >= s->image_size) {
        _libcd_err("pos too big: %zu >= %lu", pos, s->image_size);
        goto lb_out;
    }

    const size_t to_read = (pos + page_size) > s->image_size ? s->image_size-pos : page_size;

    bytes_read = pread(s->read_ctx.fd.fd, page_buf, to_read, pos);

    if (bytes_read == 0) {
        _libcd_err("short read from fd %d: %zu != %zu",
                   s->read_ctx.fd.fd, bytes_read, to_read);
        //fall through
    }
lb_out:
    return bytes_read;
}

static size_t
_libcd_read_page_mem (libcd *s, int page_no, size_t pos, size_t page_size, uint8_t * const page_buf)
{
    uint8_t const * const start = s->read_ctx.mem.start;
    memset(page_buf, 0, page_size);

    const size_t bytes_read = pos + page_size > s->image_size ? s->image_size-pos : page_size;
    memcpy(page_buf, start + pos, bytes_read);

    return bytes_read;
}

libcd *
libcd_create (size_t image_size)
{
    libcd *s = calloc(1, sizeof(libcd));

    s->image_size = image_size;
    SLIST_INIT(&s->raw_blobs);
    SLIST_INIT(&s->sslot_data);
    s->read_page_method = LIBCD_IO_INVALID;
    s->write_method = LIBCD_IO_INVALID;
    s->signature_method = LIBCD_IO_INVALID;

    s->hash_types = calloc(_default_hash_type_count, sizeof(int));
    memcpy(s->hash_types, _default_hash_types, _default_hash_type_count*sizeof(int));
    s->hash_types_count = _default_hash_type_count;
    s->cdhashes = calloc(s->hash_types_count,sizeof(_cdhash_for_hash_type_t));
    s->linkage_set = false;

    return s;
}

static void
libcd_reset_write_method (libcd *s)
{
    switch (s->write_method) {
        case LIBCD_IO_FD:
            s->write_ctx.fd = -1;
            break;
        case LIBCD_IO_MEM:
            s->write_ctx.mem.start = NULL;
            s->write_ctx.mem.end = NULL;
            break;
        case LIBCD_IO_CUSTOM:
            s->write_ctx.func.write_func = NULL;
            s->write_ctx.func.user_ctx = NULL;
            break;
#if __has_extension(blocks)
        case LIBCD_IO_BLOCK:
            s->write_ctx.block = NULL;
            break;
#endif
        default:
            break;
    }
    s->write_method = LIBCD_IO_INVALID;
    s->parallel_write = false;
    s->write = NULL;
}

static void
libcd_reset_read_method (libcd *s)
{
    switch (s->read_page_method) {
        case LIBCD_IO_FD:
            s->read_ctx.fd.fd = -1;
            break;
        case LIBCD_IO_MEM:
            s->read_ctx.mem.start = NULL;
            break;
        case LIBCD_IO_CUSTOM:
            s->read_ctx.user_ctx = NULL;
            break;
#if __has_extension(blocks)
        case LIBCD_IO_BLOCK:
            s->read_ctx.block = NULL;
            break;
#endif
        default:
            break;
    }
    s->read_page_method = LIBCD_IO_INVALID;
    s->parallel_read = false;
    s->read_page = NULL;
}

void
libcd_free (libcd *s)
{
    if (s) {
        free((void*)s->signing_id);
        free((void*)s->team_id);

        while (!SLIST_EMPTY(&s->raw_blobs)) {
            struct _raw_blob *blob = SLIST_FIRST(&s->raw_blobs);
            SLIST_REMOVE_HEAD(&s->raw_blobs, entries);
            free(blob);
        }
        while (!SLIST_EMPTY(&s->sslot_data)) {
            struct _sslot_data *blob = SLIST_FIRST(&s->sslot_data);
            SLIST_REMOVE_HEAD(&s->sslot_data, entries);
            free(blob);
        }
        free(s->hash_types);
        free(s->cdhashes);

        libcd_reset_write_method(s);
        libcd_reset_read_method(s);

        free(s);
    }
}

void
libcd_set_flags (libcd *s, uint32_t flags)
{
    s->flags = flags;
}

void
libcd_set_signing_id (libcd *s, char const *signing_id)
{
    s->signing_id = strdup(signing_id);
}

void
libcd_set_team_id (libcd *s, char const *team_id)
{
    s->team_id = strdup(team_id);
}

void
libcd_set_platform_identifier (libcd *s, uint8_t platform_identifier)
{
    s->platform_identifier = platform_identifier;
}

void libcd_set_exec_seg (libcd *s, uint64_t base, uint64_t limit, uint64_t flags)
{
    s->exec_seg_base = base;
    s->exec_seg_limit = limit;
    s->exec_seg_flags = flags;
}

struct _hash_info {
    size_t hash_len;
    const struct ccdigest_info *(*di)(void);
};

static const struct _hash_info _known_hash_types[] = {
    { 0, NULL },
    { CS_SHA1_LEN, ccsha1_di }, // CS_HASHTYPE_SHA1
    { CS_SHA256_LEN, ccsha256_di }, // CS_HASHTYPE_SHA256
    // { 0, NULL }, // CS_HASHTYPE_SHA256_TRUNCATED, unsupported
    // { 0, NULL }, // CS_HASHTYPE_SHA384, unsupported
};
static const size_t _max_known_hash_len = CS_SHA256_LEN;
static const int _known_hash_types_count = sizeof(_known_hash_types)/sizeof(_known_hash_types[0]);

static struct _hash_info const *
_libcd_get_hash_info (int hash_type)
{
    if (hash_type >= _known_hash_types_count) {
        _libcd_err("unknown hash type %d (>= %d)", hash_type, _known_hash_types_count);
        return NULL;
    }
    struct _hash_info const * info = &_known_hash_types[hash_type];
    if (info->hash_len > _max_known_hash_len) {
        _libcd_err("internal error, hash len (%d) is larger than max known hash len (%d)", info->hash_len, _max_known_hash_len);
        return NULL;
    }
    return &_known_hash_types[hash_type];
}

enum libcd_set_hash_type_ret
libcd_set_hash_types (libcd *s, int const hash_types[], unsigned int count)
{
    free(s->hash_types);
    s->hash_types = NULL;

    for (unsigned int i = 0; i < count; i++) {
        if (_libcd_get_hash_info(hash_types[i]) == NULL) {
            return LIBCD_SET_HASH_TYPE_UNKNOWN;
        }
    }

    s->hash_types = calloc(count, sizeof(int));
    memcpy(s->hash_types, hash_types, count*sizeof(int));
    s->hash_types_count = count;
    s->cdhashes = realloc(s->cdhashes, count*sizeof(_cdhash_for_hash_type_t));
    memset(s->cdhashes, 0, count*sizeof(_cdhash_for_hash_type_t));

    return LIBCD_SET_HASH_TYPE_SUCCESS;
}


enum libcd_set_hash_type_ret
libcd_set_hash_types_for_platform_version (libcd *s, int platform, int min_version)
{
#if LIBCD_HAS_PLATFORM_VERSION
    uint32_t limit = UINT32_MAX;

    switch (platform) {
        case 0:
            // If we don't know the platform, we stay agile.
            break;
        case PLATFORM_MACOS:
            // 10.11.4 had first proper sha256 support.
            limit = (10 << 16 | 11 << 8 | 4 << 0);
            break;
        case PLATFORM_TVOS:
        case PLATFORM_IOS:
            // iOS 11 and tvOS 11 had first proper sha256 support.
            limit = (11 << 16 | 0 << 8 | 0 << 0);
            break;
        case PLATFORM_WATCHOS:
            // We stay agile on the watch right now.
            break;
        default:
            // All other platforms are assumed to be new and support SHA256.
            limit = 0;
            break;
    }
    if ((uint32_t)min_version >= limit) {
        // young enough not to need SHA-1 legacy support
        const int sha256_only[1] = {CS_HASHTYPE_SHA256};
        return libcd_set_hash_types(s, sha256_only, 1);
    } else {
        // agile signing
        const int agile[2] = {CS_HASHTYPE_SHA1, CS_HASHTYPE_SHA256};
        return libcd_set_hash_types(s, agile, 2);
    }
#else
    return LIBCD_SET_HASH_TYPE_UNKNOWN;
#endif
}


void
libcd_set_input_fd (libcd *s, int fd)
{
    libcd_reset_read_method(s);
    s->read_page_method = LIBCD_IO_FD;
    s->read_page = _libcd_read_page_fd;
    s->read_ctx.fd.fd = fd;
    s->parallel_read = true;
}

void
libcd_set_input_mem (libcd *s, uint8_t const *mem)
{
    libcd_reset_read_method(s);
    s->read_page_method = LIBCD_IO_MEM;
    s->read_page = _libcd_read_page_mem;
    s->read_ctx.mem.start = mem;
    s->parallel_read = true;
}

void
libcd_set_input_func (libcd *s, libcd_read_page *read_page, void *user_ctx, bool parallel_read)
{
    libcd_reset_read_method(s);
    s->read_page_method = LIBCD_IO_CUSTOM;
    s->read_page = read_page;
    s->read_ctx.user_ctx = user_ctx;
    s->parallel_read = parallel_read;
}

void*
libcd_get_input_func_ctx (libcd* s)
{
    return (s->read_page_method == LIBCD_IO_CUSTOM) ? s->read_ctx.user_ctx : NULL;
}

void
libcd_set_output_fd (libcd *s, int fd)
{
    libcd_reset_write_method(s);
    s->write_method = LIBCD_IO_FD;
    s->write = _libcd_write_fd;
    s->write_ctx.fd = fd;
    s->parallel_write = true;
}

void
libcd_set_output_mem (libcd *s, uint8_t *mem, size_t limit)
{
    libcd_reset_write_method(s);
    s->write_method = LIBCD_IO_MEM;
    s->write = _libcd_write_mem;
    s->write_ctx.mem.start = mem;
    s->write_ctx.mem.end = mem + limit;
    s->parallel_write = true;
}

static size_t
_libcd_write_with_func (libcd *s, size_t pos, uint8_t const *data, size_t len)
{
    pos += s->write_offset;
    size_t written = s->write_ctx.func.write_func(s, pos, data, len);
    s->write_pos += written;
    return written;
}

void
libcd_set_output_func (libcd *s, libcd_write *write, void *user_ctx,
                       bool parallel_write)
{
    libcd_reset_write_method(s);
    s->write_method = LIBCD_IO_CUSTOM;
    s->write = _libcd_write_with_func;
    s->write_ctx.func.write_func = write;
    s->write_ctx.func.user_ctx = user_ctx;
    s->parallel_write = parallel_write;
}

void
libcd_set_output_offset(libcd *s, size_t offset)
{
    s->write_offset = offset;
}

void*
libcd_get_output_func_ctx (libcd *s)
{
    return (s->write_method == LIBCD_IO_CUSTOM) ? s->write_ctx.func.user_ctx : NULL;
}

void
libcd_set_disable_parallelization (libcd* s, bool disable)
{
    s->parallelization_disabled = disable;
}

#if __has_extension(blocks)

static size_t
_libcd_read_page_with_block (libcd *s, int page_no, size_t pos, size_t page_size, uint8_t *page_buf)
{
    return s->read_ctx.block(s, page_no, pos, page_size, page_buf);
}

static size_t
_libcd_write_with_block (libcd *s, size_t pos, uint8_t const *data, size_t len)
{
    pos += s->write_offset;
    size_t written = s->write_ctx.block(s, pos, data, len);
    s->write_pos += written;
    return written;
}

void
libcd_set_input_block (libcd *s, libcd_read_page_block read_page_block, bool parallel_read)
{
    libcd_reset_read_method(s);
    s->read_page_method = LIBCD_IO_BLOCK;
    s->read_page = _libcd_read_page_with_block;
    s->read_ctx.block = read_page_block;
    s->parallel_read = parallel_read;
}

void
libcd_set_output_block (libcd *s, libcd_write_block write_block, bool parallel_write)
{
    libcd_reset_write_method(s);
    s->write_method = LIBCD_IO_BLOCK;
    s->write = _libcd_write_with_block;
    s->write_ctx.block = write_block;
    s->parallel_write = parallel_write;
}

void libcd_set_signature_generator_block (libcd *s, libcd_signature_generator_block generator)
{
    s->signature_ctx.signature_func.ctx = NULL;
    s->signature_ctx.signature_func.generator = NULL;
    s->signature_ctx.signature_block = generator;
    s->signature_method = generator != NULL ? LIBCD_IO_BLOCK : LIBCD_IO_INVALID;
}

#endif /* __has_extension(blocks) */

void
libcd_add_raw_blob_no_copy (libcd *s, uint32_t slot, uint8_t const *data, size_t len)
{
    struct _raw_blob *blob = malloc(sizeof(struct _raw_blob));

    blob->slot = slot;
    blob->data = data;
    blob->len = len;

    SLIST_INSERT_HEAD(&s->raw_blobs, blob, entries);
}

void
libcd_add_sslot_data_no_copy (libcd *s, uint32_t slot, uint8_t const *data, size_t len)
{
    struct _sslot_data *sslot = malloc(sizeof(struct _sslot_data));

    sslot->slot = slot;
    sslot->data = data;
    sslot->len = len;

    SLIST_INSERT_HEAD(&s->sslot_data, sslot, entries);
    s->special_slot_count = MAX(s->special_slot_count, slot);
}

void
libcd_add_signature_space (libcd *s, uint32_t space)
{
    s->signature_space = space;
}

void
libcd_set_signature_generator(libcd *s, libcd_signature_generator *generator, void *user_ctx)
{
    s->signature_method = generator != NULL ? LIBCD_IO_CUSTOM : LIBCD_IO_INVALID;
    s->signature_ctx.signature_block = NULL;
    s->signature_ctx.signature_func.ctx = user_ctx;
    s->signature_ctx.signature_func.generator = generator;
}

enum libcd_set_linkage_ret
libcd_set_linkage(libcd *s, int linkage_hash_type, uint8_t *linkage_hash)
{
    struct _hash_info const *lhi = _libcd_get_hash_info(linkage_hash_type);

    if (lhi == NULL) {
        _libcd_err("unknown linkage hash type");
        return LIBCD_SET_LINKAGE_UNKNOWN_HASH_TYPE;
    }
    s->linkage_set = 1;
    s->linkage_hash_type = linkage_hash_type;
    memcpy(s->linkage_hash, linkage_hash, CS_CDHASH_LEN);
    return LIBCD_SET_LINKAGE_SUCCESS;
}

size_t
libcd_cd_size (libcd *s, uint32_t hash_type)
{
    struct _hash_info const *hi = _libcd_get_hash_info(hash_type);

    size_t si = s->linkage_set ? offsetof(CS_CodeDirectory, end_withLinkage) : offsetof(CS_CodeDirectory, end_withExecSeg);

    si += s->signing_id != NULL ? strlen(s->signing_id)+1 : 0;
    si += s->team_id != NULL ? strlen(s->team_id)+1 : 0;

    if (s->linkage_set) {
        si += CS_CDHASH_LEN;
    }

    si += s->special_slot_count * hi->hash_len;

    const uint32_t pages = (uint32_t)((s->image_size + _cs_page_bytes-1) >> _cs_page_shift);
    si += pages * hi->hash_len;

    return si;
}

static enum libcd_serialize_ret
_libcd_hash_page(libcd *s,
                 size_t page_idx,
                 size_t page_count,
                 struct _hash_info const *hi,
                 uint8_t* hash_destination)
{
    uint8_t page_hash[_max_known_hash_len] = {0};
    const unsigned int page_no = (unsigned int)page_idx;

    struct ccdigest_info const *di = hi->di();
    ccdigest_di_decl(di, ctx);

    const size_t pos = page_idx * _cs_page_bytes;
    uint8_t page[_cs_page_bytes] = {0};
    size_t read_bytes = s->read_page(s, page_no, pos, _cs_page_bytes, page);

    if (read_bytes == 0) {
        _libcd_err("read page %d at pos %zu failed (pages: %d)", page_no, pos,
                  page_count);
        return LIBCD_SERIALIZE_READ_PAGE_ERROR;
    }

    ccdigest_init(di, ctx);
    ccdigest_update(di, ctx, read_bytes, page);
    ccdigest_final(di, ctx, page_hash);

    memcpy(hash_destination, page_hash, hi->hash_len);

    return LIBCD_SERIALIZE_SUCCESS;
}

static enum libcd_serialize_ret
_libcd_serialize_cd (libcd *s, uint32_t hash_type)
{
    struct _hash_info const *hi = _libcd_get_hash_info(hash_type);
    size_t cd_size = libcd_cd_size(s, hash_type);
    uint8_t* cd_mem = calloc(1, cd_size);

    if (cd_mem == NULL) {
        _libcd_err("Failed to allocate temporary memory for code directory");
        return LIBCD_SERIALIZE_NO_MEM;
    }

    uint8_t* cursor = cd_mem;
    //// code directory
    {
        CS_CodeDirectory* cd = (CS_CodeDirectory*) cursor;

        cd->magic = bl(CSMAGIC_CODEDIRECTORY);

        cd->length = bl(libcd_cd_size(s, hash_type));
        cd->version = s->linkage_set ? bl(CS_SUPPORTSLINKAGE) : bl(CS_SUPPORTSEXECSEG);

        cd->flags = bl(s->flags);

        uint32_t end = s->linkage_set ? offsetof(CS_CodeDirectory, end_withLinkage) : offsetof(CS_CodeDirectory, end_withExecSeg);

        if (s->signing_id != NULL) {
            cd->identOffset = bl(end);
            end += strlen(s->signing_id)+1;
        }

        cd->nSpecialSlots = bl(s->special_slot_count);
        cd->nCodeSlots = bl((s->image_size + (_cs_page_bytes-1)) >> _cs_page_shift);

        if (s->image_size > UINT32_MAX) {
            cd->codeLimit64 = htonll(s->image_size);
        } else {
            cd->codeLimit = bl(s->image_size);
        }

        cd->hashSize = hi->hash_len;
        cd->hashType = hash_type;

        cd->platform = s->platform_identifier;

        cd->pageSize = _cs_page_shift;

        if (s->team_id != NULL) {
            cd->teamOffset = bl(end);
            end += strlen(s->team_id)+1;
        }

        cd->execSegBase = bl(s->exec_seg_base);
        cd->execSegLimit = bl(s->exec_seg_limit);
        cd->execSegFlags = bl(s->exec_seg_flags);

        if (s->linkage_set) {
            cd->linkageHashType = s->linkage_hash_type;
            cd->linkageTruncated = 1;
            cd->linkageOffset = bl(end);
            cd->linkageSize = bl(CS_CDHASH_LEN);
            end += CS_CDHASH_LEN;
        }

        cd->hashOffset = bl(end+s->special_slot_count*hi->hash_len);
    }
    cursor += s->linkage_set ? offsetof(CS_CodeDirectory, end_withLinkage) : offsetof(CS_CodeDirectory, end_withExecSeg);

    //// code directory data
    {
        if (s->signing_id != NULL) {
            size_t signing_id_len = strlen(s->signing_id)+1;
            memcpy(cursor, (uint8_t*)s->signing_id, signing_id_len);
            cursor += signing_id_len;
        }

        if (s->team_id != NULL) {
            size_t team_id_len = strlen(s->team_id)+1;
            memcpy(cursor, (uint8_t*)s->team_id, team_id_len);
            cursor += team_id_len;
        }

        if (s->linkage_set) {
            memcpy(cursor, s->linkage_hash, CS_CDHASH_LEN);
            cursor += CS_CDHASH_LEN;
        }
    }

    //// code directory hashes
    {
        if (s->special_slot_count > 0) {
            struct ccdigest_info const *di = hi->di();
            ccdigest_di_decl(di, ctx);

            uint8_t *special_slot_buf = calloc(s->special_slot_count, hi->hash_len);

            struct _sslot_data *sslot = NULL;
            SLIST_FOREACH(sslot, &s->sslot_data, entries) {
                ccdigest_init(di, ctx);
                ccdigest_update(di, ctx, sslot->len, sslot->data);
                ccdigest_final(di, ctx, special_slot_buf + (s->special_slot_count-sslot->slot)*hi->hash_len);
            }
            memcpy(cursor, special_slot_buf, s->special_slot_count*hi->hash_len);
            cursor += s->special_slot_count*hi->hash_len;

            free(special_slot_buf);
        }

        unsigned int const page_count = (unsigned int)((s->image_size + _cs_page_bytes-1) >> _cs_page_shift);

        volatile enum libcd_serialize_ret _libcd_block ret = LIBCD_SERIALIZE_SUCCESS;

#if LIBCD_PARALLEL
        if(s->parallel_read && s->parallel_write && !s->parallelization_disabled) {
            dispatch_apply(page_count, DISPATCH_APPLY_AUTO, ^(size_t page_no) {
                uint8_t* destination = cursor + page_no * hi->hash_len;
                enum libcd_serialize_ret local_ret = _libcd_hash_page(s, page_no, page_count, hi, destination);
                ret = (ret == LIBCD_SERIALIZE_SUCCESS) ? local_ret : ret;
            });
        } else {
#endif
            for (size_t page_no = 0; page_no < page_count; page_no++) {
                uint8_t* destination = cursor + page_no * hi->hash_len;
                ret = _libcd_hash_page(s, page_no, page_count, hi, destination);
                if (ret != LIBCD_SERIALIZE_SUCCESS) {
                    break;
                }
            }
#if LIBCD_PARALLEL
        }
#endif

        if (ret != LIBCD_SERIALIZE_SUCCESS) {
            _libcd_err("serialize page hashes failed");
            free(cd_mem);
            return ret;
        }
        if (!s->write(s, s->write_pos, cd_mem, cd_size)) {
            _libcd_err("failed to write directory");
            free(cd_mem);
            return LIBCD_SERIALIZE_WRITE_ERROR;
        }
    }

    //Record the cdhash for this codedirectory
    {
        struct ccdigest_info const *di = hi->di();
        ccdigest_di_decl(di, ctx);
        uint8_t *cdhash_buf = calloc(1, hi->hash_len);
        if (cdhash_buf == NULL) {
            _libcd_err("Failed to allocated memory for cdhash");
            free(cd_mem);
            return LIBCD_SERIALIZE_NO_MEM;
        }
        ccdigest_init(di, ctx);
        ccdigest_update(di, ctx, cd_size, cd_mem);
        ccdigest_final(di, ctx, cdhash_buf);

        for (size_t i = 0; i < s->hash_types_count; i++) {
            if (s->cdhashes[i].set) {
                if (s->cdhashes[i].hash_type == (int)hash_type) {
                    memcpy(s->cdhashes[i].cdhash, cdhash_buf, CS_CDHASH_LEN);
                    break;
                }
            } else {
                s->cdhashes[i].hash_type = hash_type;
                s->cdhashes[i].set = true;
                memcpy(s->cdhashes[i].cdhash, cdhash_buf, CS_CDHASH_LEN);
                break;
            }
        }
        free(cdhash_buf);
    }

    free(cd_mem);
    return LIBCD_SERIALIZE_SUCCESS;
}

size_t
libcd_superblob_size (libcd *s)
{
    size_t length = sizeof(CS_SuperBlob);
    int blobs = 0;

    // code directories
    for (unsigned int i = 0; i < s->hash_types_count; i++) {
        length += libcd_cd_size(s, s->hash_types[i]);
        blobs++;
    }

    // raw blobs
    struct _raw_blob *rb;
    SLIST_FOREACH(rb, &s->raw_blobs, entries) {
        length += rb->len;
        blobs++;
    }

    // signature
    if (s->signature_space > 0) {
        length += s->signature_space;
        blobs++;
    }

    // indices
    length += blobs * sizeof(CS_BlobIndex);

    return length;
}

enum libcd_serialize_ret
libcd_serialize_as_type (libcd *s, uint32_t type)
{
    if (s->read_page == NULL || s->read_page_method == LIBCD_IO_INVALID) {
        _libcd_err("No read page method set");
        return LIBCD_SERIALIZE_READ_PAGE_ERROR;
    }
    if (s->write == NULL || s->write_method == LIBCD_IO_INVALID) {
        _libcd_err("No write method set");
        return LIBCD_SERIALIZE_WRITE_ERROR;
    }

    s->write_pos = 0;
    //// superblob header
    int count = 0;
    {
        // code directories
        count += s->hash_types_count;

        // raw blobs
        struct _raw_blob *rb = NULL;
        SLIST_FOREACH(rb, &s->raw_blobs, entries) {
            count++;
        }

        // signature space
        if (s->signature_space > 0) {
            count++;
        }
    }
    if (count == 0) {
        _libcd_err("nothing to serialize");
        return LIBCD_SERIALIZE_EMPTY;
    }

    //// serialize superblob header
    {
        CS_SuperBlob sb = {
            .magic = bl(type),
            .length = bl(libcd_superblob_size(s)),
            .count = bl(count)
        };

        if (!s->write(s, s->write_pos, (uint8_t*)&sb, sizeof(CS_SuperBlob))) {
            _libcd_err("serialize superblob header failed");
            return LIBCD_SERIALIZE_WRITE_ERROR;
        }

        uint32_t next_blob = sizeof(CS_SuperBlob) + count*sizeof(CS_BlobIndex);

        CS_BlobIndex *indices = calloc(count, sizeof(CS_BlobIndex));
        int ci = 0;

        // code directories
        assert(s->hash_types_count <= CSSLOT_ALTERNATE_CODEDIRECTORY_MAX);
        for (unsigned int i = 0; i < s->hash_types_count; i++) {
            indices[ci].type = bl(i == 0 ?
                                  CSSLOT_CODEDIRECTORY :
                                  CSSLOT_ALTERNATE_CODEDIRECTORIES + i-1);
            indices[ci++].offset = bl(next_blob);
            next_blob += libcd_cd_size(s, s->hash_types[i]);
        }

        // raw blobs
        struct _raw_blob *rb = NULL;
        SLIST_FOREACH(rb, &s->raw_blobs, entries) {
            indices[ci].type = bl(rb->slot);
            indices[ci++].offset = bl(next_blob);
            next_blob += rb->len;
        }

        // signature space
        if (s->signature_space > 0) {
            indices[ci].type = bl(CSSLOT_SIGNATURESLOT);
            indices[ci++].offset =  bl(next_blob);
            next_blob += s->signature_space;
        }

        if (!s->write(s, s->write_pos, (uint8_t const *)indices, count*sizeof(CS_BlobIndex))) {
            _libcd_err("serialize superblob indices failed");
            free(indices);
            return LIBCD_SERIALIZE_WRITE_ERROR;
        }

        free(indices);
    }

    //// serialize blobs
    {
        enum libcd_serialize_ret ret;
        // code directories
        for (unsigned int i = 0; i < s->hash_types_count; i++) {
            if ((ret = _libcd_serialize_cd(s, s->hash_types[i])) != LIBCD_SERIALIZE_SUCCESS) {
                _libcd_err("serialize code directory type %d failed, error %d", s->hash_types[i], ret);
                return ret;
            }
        }

        // raw blobs
        struct _raw_blob *rb = NULL;
        SLIST_FOREACH(rb, &s->raw_blobs, entries) {
            if (!s->write(s, s->write_pos, (uint8_t const *)rb->data, rb->len)) {
                _libcd_err("serialize raw blob data, slot %#x, failed", rb->slot);
                return LIBCD_SERIALIZE_WRITE_ERROR;
            }
        }

        // signature space
        if (s->signature_space > 0) {
            uint8_t *signature_space_mem = calloc(1, s->signature_space);
            if (signature_space_mem == NULL) {
                _libcd_err("serialize signature space(%zu) failed allocating space (%d)", s->signature_space, errno);
                return LIBCD_SERIALIZE_NO_MEM;
            }
            switch (s->signature_method) {
                case LIBCD_IO_CUSTOM:
                    if (!s->signature_ctx.signature_func.generator(s, s->signature_ctx.signature_func.ctx,
                                                                   s->signature_space, signature_space_mem)) {
                        _libcd_err("Failed to generate signature");
                        free(signature_space_mem);
                        return LIBCD_SERIALIZE_SIGNATURE_FAILURE;
                    }
                    break;
#if __has_extension(blocks)
                case LIBCD_IO_BLOCK:
                    if (!s->signature_ctx.signature_block(s, s->signature_space, signature_space_mem)) {
                        _libcd_err("Failed to generate signature");
                        free(signature_space_mem);
                        return LIBCD_SERIALIZE_SIGNATURE_FAILURE;
                    }
                    break;
#endif
                default:
                    break;
            }
            if (!s->write(s, s->write_pos, signature_space_mem, s->signature_space)) {
                _libcd_err("serialize signature space (%zu) failed", s->signature_space);
                free(signature_space_mem);
                return LIBCD_SERIALIZE_WRITE_ERROR;
            }
            free(signature_space_mem);
        }
    }

    return LIBCD_SERIALIZE_SUCCESS;
}

enum libcd_serialize_ret
libcd_serialize (libcd *s)
{
    return libcd_serialize_as_type(s, CSMAGIC_EMBEDDED_SIGNATURE);
}

enum libcd_cdhash_ret
libcd_get_cdhash_for_type(libcd *s, int hash_type, uint8_t* cdhash_buf, size_t cdhash_buf_len)
{
    if (cdhash_buf == NULL || cdhash_buf_len < CS_CDHASH_LEN) {
        return LIBCD_CDHASH_INVALID_BUFFER;
    }
    for (size_t i = 0; i < s->hash_types_count; i++) {
        if (i == 0 && !s->cdhashes[i].set) {
            return LIBCD_CDHASH_NOT_SERIALIZED;
        } else if (s->cdhashes[i].set && hash_type == s->cdhashes[i].hash_type) {
            memcpy(cdhash_buf, s->cdhashes[i].cdhash, CS_CDHASH_LEN);
            return LIBCD_CDHASH_SUCCESS;
        }
    }
    return LIBCD_CDHASH_TYPE_NOT_FOUND;
}
