/*
 * Blocks Runtime
 */

#ifdef __cplusplus
#define BLOCKS_EXPORT extern "C"
#else
#define BLOCKS_EXPORT extern 
#endif

BLOCKS_EXPORT void *_Block_copy(const void *);
BLOCKS_EXPORT void _Block_release(const void *);
BLOCKS_EXPORT const char *_Block_get_types(const void*);

#define Block_copy(x) ((__typeof__(x))_Block_copy((const void *)(x)))
#define Block_release(x) _Block_release((const void *)(x))
