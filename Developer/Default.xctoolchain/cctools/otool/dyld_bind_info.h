/*
 * enum chain_format_t matches the dyld pointer format uint32_t values used
 * by LC_DYLD_CHAINED_FIXUPS. Note that the CHAIN_FORMAT_ARM64E format is also
 * used by LC_DYLD_INFO for arm64e.
 */
enum chain_format_t {
    CHAIN_FORMAT_NONE            = 0,
    CHAIN_FORMAT_ARM64E          = 1,
    CHAIN_FORMAT_PTR_64          = 2,
    CHAIN_FORMAT_PTR_32          = 3,
    CHAIN_FORMAT_PTR_32_CACHE    = 4,
    CHAIN_FORMAT_PTR_32_FIRMWARE = 5,
};

/*
 * dyld_bind_info is the internal structure for the broken out bind info.
 *
 * When printing arm64e data from DYLD_INFO:
 *   bind_type will be set to the bind type. rebase_type will be ignored.
 *   bind_name will be NULL.
 *   pointer_format will be set to 0 (default to the 64-bit behavior, even
 *     though not strictly a legal value)
 *
 * When printing info from DYLD_CHAINED_FIXUPS:
 *   bind_type will be 0.
 *   bind_name will be set to a human-readable value.
 *   pointer_format will be set to one of the following chained_fixup pointer
 *     format values: 0 (not set, default to 64-bit), 3 (32-bit)
 */
struct dyld_bind_info {
    const char *segname;
    const char *sectname;
    uint64_t address;
    int bind_type;
    const char *bind_name;
    uint64_t addend;
    const char *dylibname;
    const char *symbolname;
    enum bool weak_import;
    uint64_t pointer_value;
    uint32_t pointer_format;/* dyld_chained_starts_in_segment.pointer_format */
};

extern void get_dyld_bind_info(
    const uint8_t *start, /* inputs */
    const uint8_t *end,
    const char **dylibs,
    uint32_t ndylibs,
    struct segment_command **segs,
    uint32_t nsegs,
    struct segment_command_64 **segs64,
    uint32_t nsegs64,
    enum bool swapped,
    char *object_addr,
    uint64_t object_size,
    struct dyld_bind_info **dbi, /* outputs */
    uint64_t *ndbi,
    enum chain_format_t *chain_format,
    enum bool print_errors);

extern void print_dyld_bind_info(
    struct dyld_bind_info *dbi,
    uint64_t ndbi);

extern void print_dyld_rebase_opcodes(
    const uint8_t* object_addr,
    uint64_t object_size,
    uint32_t offset,
    uint32_t size);

extern void print_dyld_bind_opcodes(
    const uint8_t* object_addr,
    uint64_t object_size,
    const char* type,
    uint32_t offset,
    uint32_t size);

extern const char * get_dyld_bind_info_symbolname(
    uint64_t address,
    struct dyld_bind_info *dbi,
    uint64_t ndbi,
    enum chain_format_t chain_format,
    int64_t *addend);

extern void get_dyld_chained_fixups(
    const uint8_t *start, /* inputs */
    const uint8_t *end,
    const char **dylibs,
    uint32_t ndylibs,
    struct segment_command **segs,
    uint32_t nsegs,
    struct segment_command_64 **segs64,
    uint32_t nsegs64,
    enum bool swapped,
    char *object_addr,
    uint64_t object_size,
    struct dyld_bind_info **dbi, /* outputs */
    uint64_t *ndbi,
    enum chain_format_t *chain_format,
    enum bool print_errors);

extern uint64_t get_chained_rebase_value(
    uint64_t chain_value,
    enum chain_format_t chain_format,
    enum bool *has_auth);
