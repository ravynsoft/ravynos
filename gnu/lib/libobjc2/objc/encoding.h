#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif
#include "objc-visibility.h"

#ifndef __LIBOBJC_ENCODING_H_INCLUDED__
#define __LIBOBJC_ENCODING_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

OBJC_PUBLIC
const char *objc_skip_type_qualifiers (const char *type);

OBJC_PUBLIC
const char *objc_skip_typespec(const char *type);

OBJC_PUBLIC
const char *objc_skip_argspec(const char *type);


OBJC_PUBLIC
size_t objc_sizeof_type(const char *type);

OBJC_PUBLIC
size_t objc_alignof_type(const char *type);

OBJC_PUBLIC
size_t objc_aligned_size(const char *type);

OBJC_PUBLIC
size_t objc_promoted_size(const char *type);

OBJC_PUBLIC
void method_getReturnType(Method method, char *dst, size_t dst_len);

OBJC_PUBLIC
const char *method_getTypeEncoding(Method method);

OBJC_PUBLIC
void method_getArgumentType(Method method,
                            unsigned int index,
                            char *dst,
                            size_t dst_len);

OBJC_PUBLIC
unsigned method_getNumberOfArguments(Method method);

OBJC_PUBLIC
unsigned method_get_number_of_arguments(struct objc_method *method);

OBJC_PUBLIC
char * method_copyArgumentType(Method method, unsigned int index);

OBJC_PUBLIC
char * method_copyReturnType(Method method);

////////////////////////////////////////////////////////////////////////////////
// Deprecated functions - do not use functions below this line in new code.
////////////////////////////////////////////////////////////////////////////////
OBJC_PUBLIC
unsigned objc_get_type_qualifiers (const char *type);

struct objc_struct_layout
{
	const char *original_type;
	const char *type;
	const char *prev_type;
	unsigned int record_size;
	unsigned int record_align;
};

// Note: The implementations of these functions is horrible.
OBJC_PUBLIC
void objc_layout_structure (const char *type,
                            struct objc_struct_layout *layout);

OBJC_PUBLIC
BOOL objc_layout_structure_next_member(struct objc_struct_layout *layout);

OBJC_PUBLIC
void objc_layout_structure_get_info (struct objc_struct_layout *layout,
                                     unsigned int *offset,
                                     unsigned int *align,
                                     const char **type);

#define _F_CONST       0x01
#define _F_IN          0x01
#define _F_OUT         0x02
#define _F_INOUT       0x03
#define _F_BYCOPY      0x04
#define _F_BYREF       0x08
#define _F_ONEWAY      0x10
#define _F_GCINVISIBLE 0x20

#ifdef __cplusplus
}
#endif

#endif // __LIBOBJC_ENCODING_H_INCLUDED__
