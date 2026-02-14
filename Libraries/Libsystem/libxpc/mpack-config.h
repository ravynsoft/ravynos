
/**
 * This is a sample MPack configuration file. Copy it to mpack-config.h somewhere
 * in your project's include tree and, optionally, edit it to suit your setup.
 * In most cases you can leave this file with the default config.
 */

#ifndef MPACK_CONFIG_H
#define MPACK_CONFIG_H 1


/*
 * Features
 */

/** Enables compilation of the base Tag Reader. */
#define MPACK_READER 1

/** Enables compilation of the static Expect API. */
#define MPACK_EXPECT 1

/** Enables compilation of the dynamic Node API. */
#define MPACK_NODE 1

/** Enables compilation of the Writer. */
#define MPACK_WRITER 1


/*
 * Dependencies
 */

/**
 * Enables the use of C stdlib. This allows the library to use malloc
 * for debugging and in allocation helpers.
 */
#define MPACK_STDLIB 1

/**
 * Enables the use of C stdio. This adds helpers for easily
 * reading/writing C files and makes debugging easier.
 */
#define MPACK_STDIO 1

/**
 * \def MPACK_MALLOC
 *
 * Defines the memory allocation function used by mpack. This is used by
 * helpers for automatically allocating data the correct size, and for
 * debugging functions. If this macro is undefined, the allocation helpers
 * will not be compiled.
 *
 * A memory allocator is required for the Node API.
 */
/**
 * \def MPACK_REALLOC
 *
 * Defines the realloc function used by mpack. It is used by growable buffers
 * to resize more quickly.
 *
 * This is optional, even when MPACK_MALLOC is used. If MPACK_MALLOC is
 * set and MPACK_REALLOC is not, MPACK_MALLOC is used with a simple copy
 * to grow buffers.
 */
#if defined(MPACK_STDLIB) && !defined(MPACK_MALLOC)
#define MPACK_MALLOC malloc
#define MPACK_REALLOC realloc
#endif

/**
 * \def MPACK_FREE
 *
 * Defines the memory free function used by mpack. This is used by helpers
 * for automatically allocating data the correct size. If this macro is
 * undefined, the allocation helpers will not be compiled.
 *
 * A memory allocator is required for the Node API.
 */
#if defined(MPACK_STDLIB) && !defined(MPACK_FREE)
#define MPACK_FREE free
#endif

/**
 * Enables the setjmp()/longjmp() error handling option. MPACK_MALLOC is required.
 *
 * Note that you don't have to use it; this just enables the option. It can be
 * disabled to avoid the dependency on setjmp.h .
 */
#if defined(MPACK_MALLOC)
#define MPACK_SETJMP 1
#endif


/*
 * Debugging options
 */

/**
 * \def MPACK_DEBUG
 *
 * Enables debug features. You may want to wrap this around your
 * own debug preprocs. By default, they are enabled if DEBUG or _DEBUG
 * are defined.
 *
 * Note that MPACK_DEBUG cannot be defined differently for different
 * source files because it affects layout of structs defined in header
 * files. Your entire project must be compiled with the same value of
 * MPACK_DEBUG. (This is why NDEBUG is not used.)
 */
#if defined(DEBUG) || defined(_DEBUG)
#define MPACK_DEBUG 1
#else
#define MPACK_DEBUG 0
#endif

/**
 * Set this to 1 to implement a custom mpack_assert_fail() function. This
 * function must not return, and must have the following signature:
 *
 *     void mpack_assert_fail(const char* message)
 *
 * Asserts are only used when MPACK_DEBUG is enabled, and can be triggered
 * by bugs in mpack or bugs due to incorrect usage of mpack.
 */
#define MPACK_CUSTOM_ASSERT 0

/**
 * \def MPACK_READ_TRACKING
 *
 * Enables compound type size tracking for readers. This ensures that the
 * correct number of elements or bytes are read from a compound type.
 *
 * This is enabled by default in debug builds (provided a malloc() is
 * available.)
 */
#if MPACK_DEBUG && MPACK_READER && defined(MPACK_MALLOC)
#define MPACK_READ_TRACKING 1
#endif

/**
 * \def MPACK_WRITE_TRACKING
 *
 * Enables compound type size tracking for writers. This ensures that the
 * correct number of elements or bytes are written in a compound type.
 *
 * Note that without write tracking enabled, it is possible for buggy code
 * to emit invalid MessagePack without flagging an error by writing the wrong
 * number of elements or bytes in a compound type. With tracking enabled,
 * MPACK will catch such errors and break on the offending line of code.
 *
 * This is enabled by default in debug builds (provided a malloc() is
 * available.)
 */
#if MPACK_DEBUG && MPACK_WRITER && defined(MPACK_MALLOC)
#define MPACK_WRITE_TRACKING 1
#endif


/*
 * Miscellaneous
 */

/**
 * Stack space to use when initializing a reader or writer with a
 * stack-allocated buffer.
 */
#define MPACK_STACK_SIZE 4096

/**
 * Buffer size to use for allocated buffers (such as for a file writer.)
 */
#define MPACK_BUFFER_SIZE 65536

/**
 * Number of nodes in each allocated node page.
 *
 * Nodes are 16 bytes when compiled for a 32-bit architecture and
 * 24 bytes when compiled for a 64-bit architecture.
 *
 * Using as many nodes fit in one memory page seems to provide the
 * best performance, and has very little waste when parsing small
 * messages.
 */
#define MPACK_NODE_PAGE_SIZE (4096 / sizeof(mpack_node_t))

/**
 * The initial depth for the node parser. When MPACK_MALLOC is available,
 * the node parser has no practical depth limit, and it is not recursive
 * so there is no risk of overflowing the call stack.
 */
#define MPACK_NODE_INITIAL_DEPTH 8

/**
 * The maximum depth for the node parser if MPACK_MALLOC is not available.
 * The parsing stack is placed on the call stack.
 */
#define MPACK_NODE_MAX_DEPTH_WITHOUT_MALLOC 32


#endif

