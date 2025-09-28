#ifdef __cplusplus
extern "C" {
#endif
/**
 * Allocates a C++ exception.  This function is part of the Itanium C++ ABI and
 * is provided externally.
 */
/*
 * Note: Recent versions of libsupc++ already provide a prototype for
 * __cxa__allocate_exception(). Since the libsupc++ version is defined with
 * _GLIBCXX_NOTHROW, clang gives a type mismatch error.
 */
#ifndef __cplusplus
#undef CXA_ALLOCATE_EXCEPTION_SPECIFIER
#define CXA_ALLOCATE_EXCEPTION_SPECIFIER
#endif
__attribute__((weak))
void *__cxa_allocate_exception(size_t thrown_size) CXA_ALLOCATE_EXCEPTION_SPECIFIER;

/**
 * Initialises an exception object returned by __cxa_allocate_exception() for
 * storing an Objective-C object.  The return value is the location of the
 * _Unwind_Exception structure within this structure, and should be passed to
 * the C++ personality function.
 */
__attribute__((weak))
struct _Unwind_Exception *objc_init_cxx_exception(id thrown_exception);
/**
 * The GNU C++ exception personality function, provided by libsupc++ (GNU) or
 * libcxxrt (PathScale).
 */
__attribute__((weak)) DECLARE_PERSONALITY_FUNCTION(__gxx_personality_v0);
/**
 * Frees an exception object allocated by __cxa_allocate_exception().  Part of
 * the Itanium C++ ABI.
 */
__attribute__((weak))
void __cxa_free_exception(void *thrown_exception);
/**
 * Tests whether a C++ exception contains an Objective-C object, and returns if
 * if it does.  The second argument is a pointer to a boolean value indicating
 * whether this is a valid object.
 */
__attribute__((weak))
void *objc_object_for_cxx_exception(void *thrown_exception, int *isValid);

/**
 * Prints the type info associated with an exception.  Used only when
 * debugging, not compiled in the normal build.
 */
__attribute__((weak))
void print_type_info(void *thrown_exception);

/**
 * The exception class that we've detected that C++ runtime library uses.
 */
extern uint64_t cxx_exception_class;

/**
 * The exception class that libsupc++ and libcxxrt use.
 */
const uint64_t gnu_cxx_exception_class = EXCEPTION_CLASS('G','N','U','C','C','+','+','\0');

/**
 * The exception class that libc++abi uses.
 */
const uint64_t llvm_cxx_exception_class = EXCEPTION_CLASS('C','L','N','G','C','+','+','\0');

#ifdef __cplusplus
}
#endif
