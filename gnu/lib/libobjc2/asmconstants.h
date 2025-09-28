#ifdef __LP64__
#define DTABLE_OFFSET  64
#define SMALLOBJ_BITS  3
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    0
#elif defined(_WIN64)
// long is 32 bits on Win64, so struct objc_class is smaller.  All other offsets are the same.
#define DTABLE_OFFSET  56
#define SMALLOBJ_BITS  3
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    0
#else
#define DTABLE_OFFSET  32
#define SMALLOBJ_BITS  1
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    0
#endif
#define SMALLOBJ_MASK  ((1<<SMALLOBJ_BITS) - 1)
