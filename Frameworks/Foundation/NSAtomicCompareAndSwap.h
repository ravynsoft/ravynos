
#if COCOTRON_USE_NONATOMIC_COMPARE_AND_SWAP
#warning __sync_bool_compare_and_swap is nonatomic. Do not use more than one thread!
#define __sync_bool_compare_and_swap(pointer, a, b, ...) \
    ((*pointer == a) ? (*pointer = b), 1 : 0)
#endif
