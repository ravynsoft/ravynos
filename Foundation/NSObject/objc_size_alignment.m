/*
 Most of this comes from PyObjC, http://pyobjc.sourceforge.net/
 Copyright 2002, 2003 - Bill Bumgarner, Ronald Oussoren, Steve Majewski, Lele Gaifax, et.al.
 Copyright 2008 Johannes Fortmann

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef GCC_RUNTIME_3
#include <objc/runtime.h>
#else
#import <objc/objc-class.h>
#endif
#import "objc_size_alignment.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#include <string.h>
#include <ctype.h>

static inline size_t
ROUND(size_t v, size_t a)
{
	if (v % a == 0) {
		return v;
	} else {
		return v + a - (v % a);
	}
}


static inline const char*
objc_ext_skip_type_qualifier (const char* type)
{
	NSCAssert(type != NULL, NULL);

	while (
          *type == _C_CONST ||
          *type == _C_IN ||
          *type == _C_INOUT ||
          *type == _C_OUT ||
          *type == _C_BYCOPY ||
          *type == _C_ONEWAY) {
		type++;
	}
	while (*type && isdigit(*type)) type++;
	return type;
}

const char *
objc_ext_skip_type_specifier (const char *type,BOOL skipDigits)
{
	NSCAssert(type != NULL, NULL);

	type = objc_ext_skip_type_qualifier (type);

	switch (*type) {
			/* The following are one character type codes */
		case _C_UNDEF:
		case _C_CLASS:
		case _C_SEL:
		case _C_CHR:
		case _C_UCHR:
		case _C_CHARPTR:
#ifdef _C_ATOM
		case _C_ATOM:
#endif
#ifdef _C_BOOL
		case _C_BOOL:
#endif
		case _C_SHT:
		case _C_USHT:
		case _C_INT:
		case _C_UINT:
		case _C_LNG:
		case _C_ULNG:
		case _C_FLT:
		case _C_DBL:
		case _C_VOID:
		case _C_LNG_LNG:
		case _C_ULNG_LNG:
			++type;
			break;

		case _C_BFLD:
			while (isdigit (*++type));
			break;

      case _C_ID:
			++type;
			if (*type == '"') {
				/* embedded field name in an ivar_type */
				type=strchr(type+1, '"');
				if (type != NULL) {
					type++;
				}
			}
			break;

      case _C_ARY_B:
			/* skip digits, typespec and closing ']' */

			while (isdigit (*++type));
			type = objc_ext_skip_type_specifier (type,skipDigits);
			NSCAssert (type == NULL || *type == _C_ARY_E, nil);
			if (type) type++;
			break;

      case _C_STRUCT_B:
			/* skip name, and elements until closing '}'  */
			while (*type != _C_STRUCT_E && *type++ != '=');
			while (type && *type != _C_STRUCT_E) {
				if (*type == '"') {
					/* embedded field names */
					type = strchr(type+1, '"');
					if (type != NULL) {
						type++;
					} else {
						return NULL;
					}
				}
				type = objc_ext_skip_type_specifier (type,skipDigits);
			}
			if (type) type++;
			break;

      case _C_UNION_B:
			/* skip name, and elements until closing ')'  */
			while (*type != _C_UNION_E && *type++ != '=');
			while (type && *type != _C_UNION_E) {
				if (*type == '"') {
					/* embedded field names */
					type = strchr(type+1, '"');
					if (type != NULL) {
						type++;
					} else {
						return NULL;
					}
				}
				type = objc_ext_skip_type_specifier (type,skipDigits);
			}
			if (type) type++;
			break;

      case _C_PTR:
      case _C_CONST:
      case _C_IN:
      case _C_INOUT:
      case _C_OUT:
      case _C_BYCOPY:
      case _C_ONEWAY:

			/* Just skip the following typespec */
			type = objc_ext_skip_type_specifier (type+1,skipDigits);
			break;


      default:
			NSLog(@"objc_ext_skip_type_specifier: Unhandled type '%#x' %s", *type, type);
			return NULL;
	}

   if(skipDigits){
	/* The compiler inserts a number after the actual signature,
	 * this number may or may not be usefull depending on the compiler
	 * version. We never use it.
	 */
	 while (type && *type && isdigit(*type)) type++;
    }

	return type;
}


/*
 Return the alignment of an object specified by type
 */

/*
 *  On MacOS X, the elements of a struct are aligned differently inside the
 *  struct than outside. That is, the maximum alignment of any struct field
 *  (except the first) is 4, doubles outside of a struct have an alignment of
 *  8.
 *
 *  Other platform don't seem to have this inconsistency.
 *
 *  XXX: sizeof_struct, alignof_struct and {de,}pythonify_c_struct should
 *  probably be moved to platform dependend files. As long as this is the
 *  only platform dependent code this isn't worth the effort.
 */

static inline size_t
PyObjC_EmbeddedAlignOfType (const char*  type)
{
	NSCAssert(type != NULL, nil);

	size_t align = objc_ext_alignof_type(type);

#if (defined(__i386__) || defined(__x86_64__)) && !defined(LINUX)
	return align;

#else
	if (align < 4 || align == 16) {
		return align;
	} else {
		return 4;
	}
#endif
}

size_t
objc_ext_alignof_type (const char *type)
{
	NSCAssert(type != NULL, nil);

	switch (*type) {
		case _C_VOID:  return __alignof__(char);
		case _C_ID:    return __alignof__ (id);
		case _C_CLASS: return __alignof__ (Class);
		case _C_SEL:   return __alignof__ (SEL);
		case _C_CHR:   return __alignof__ (char);
		case _C_UCHR:  return __alignof__ (unsigned char);
		case _C_SHT:   return __alignof__ (short);
		case _C_USHT:  return __alignof__ (unsigned short);
#ifdef _C_BOOL
		case _C_BOOL:   return __alignof__ (BOOL);
#endif
		case _C_INT:   return __alignof__ (int);
		case _C_UINT:  return __alignof__ (unsigned int);
		case _C_LNG:   return __alignof__ (long);
		case _C_ULNG:  return __alignof__ (unsigned long);
		case _C_FLT:   return __alignof__ (float);
		case _C_DBL:
#if defined(__APPLE__) && defined(__i386__)
			/* The ABI says natural alignment is 4 bytes, but
			 * GCC's __alignof__ says 8. The latter is wrong.
			 */
			return 4;
#else
			return __alignof__ (double);
#endif

		case _C_CHARPTR: return __alignof__ (char *);
#ifdef _C_ATOM
		case _C_ATOM: return __alignof__ (char *);
#endif
		case _C_PTR:   return __alignof__ (void *);
#if defined(__APPLE__) && defined(__i386__)
			/* The ABI says natural alignment is 4 bytes, but
			 * GCC's __alignof__ says 8. The latter is wrong.
			 */
		case _C_LNG_LNG: return 4;
		case _C_ULNG_LNG: return 4;
#else
		case _C_LNG_LNG: return __alignof__(long long);
		case _C_ULNG_LNG: return __alignof__(unsigned long long);
#endif

		case _C_ARY_B:
			while (isdigit(*++type)) /* do nothing */;
			return objc_ext_alignof_type (type);

      case _C_STRUCT_B:
		{
			struct { int x; double y; } fooalign;
			while(*type != _C_STRUCT_E && *type++ != '=') /* do nothing */;
			if (*type != _C_STRUCT_E) {
				int have_align = 0;
				size_t align = 0;

				while (type != NULL && *type != _C_STRUCT_E) {
					if (*type == '"') {
						type = strchr(type+1, '"');
						if (type) type++;
					}
					if (have_align) {
						align = MAX(align,
                              PyObjC_EmbeddedAlignOfType(type));
					} else {
						align = objc_ext_alignof_type(type);
						have_align = 1;
					}
					type = objc_ext_skip_type_specifier(type,YES);
				}
				if (type == NULL) return -1;
				return align;
			} else {
				return __alignof__ (fooalign);
			}
		}

      case _C_UNION_B:
		{
			size_t maxalign = 0;
			type++;
         while (*type != _C_UNION_E && *type++ != '=')
            ; /* skip "<name>=" */
			while (*type != _C_UNION_E) {
            if (*type == '"') {
					type = strchr(type+1, '"');
					if (type) type++;
				}
				size_t item_align = objc_ext_alignof_type(type);
				if (item_align == -1) return -1;
				maxalign = MAX (maxalign, item_align);
				type = objc_ext_skip_type_specifier (type,YES);
			}
			return maxalign;
		}

      case _C_CONST:
      case _C_IN:
      case _C_INOUT:
      case _C_OUT:
      case _C_BYCOPY:
      case _C_ONEWAY:
			return objc_ext_alignof_type(type+1);

      case _C_BFLD:
			return 1;

      default:
			NSLog(@"objc_ext_alignof_type: Unhandled type '%c' %s", *type, type);
			return -1;
	}
}

/*
 The aligned size if the size rounded up to the nearest alignment.
 */

static inline size_t
PyObjCRT_AlignedSize (const char *type)
{
	NSCAssert(type != NULL, nil);

	size_t size = objc_ext_sizeof_type (type);
	size_t align = objc_ext_alignof_type (type);

	if (size == -1 || align == -1) return -1;
	return ROUND(size, align);
}

/*
 return the size of an object specified by type
 */

size_t
objc_ext_sizeof_type (const char *type)
{
	NSCAssert(type != NULL, nil);

	size_t itemSize;
	switch (*type) {
		case _C_VOID:    return 1; // More convenient than the correct value.
		case _C_ID:      return sizeof(id);
		case _C_CLASS:   return sizeof(Class);
		case _C_SEL:     return sizeof(SEL);
		case _C_CHR:     return sizeof(char);
		case _C_UCHR:    return sizeof(unsigned char);
		case _C_SHT:     return sizeof(short);
		case _C_USHT:    return sizeof(unsigned short);
#ifdef _C_BOOL
		case _C_BOOL:    return sizeof(BOOL);
#endif
		case _C_INT:     return sizeof(int);
		case _C_UINT:    return sizeof(unsigned int);
		case _C_LNG:     return sizeof(long);
		case _C_ULNG:    return sizeof(unsigned long);
		case _C_FLT:     return sizeof(float);
		case _C_DBL:     return sizeof(double);
		case _C_LNG_LNG:  return sizeof(long long);
		case _C_ULNG_LNG: return sizeof(unsigned long long);

		case _C_PTR:
		case _C_CHARPTR:
#ifdef _C_ATOM
		case _C_ATOM:
#endif
			return sizeof(char*);

		case _C_ARY_B:
		{
			size_t len = atoi(type+1);
			size_t item_align;
			while (isdigit(*++type))
				;
			item_align = PyObjCRT_AlignedSize(type);
			if (item_align == -1) return -1;
			return len*item_align;
		}
			break;

		case _C_STRUCT_B:
		{
			size_t acc_size = 0;
			int have_align =  0;
			size_t align;
			size_t max_align = 0;

			while (*type != _C_STRUCT_E && *type++ != '=')
				; /* skip "<name>=" */
			while (*type != _C_STRUCT_E) {
				if (*type == '"') {
					type = strchr(type+1, '"');
					if (type) type++;
				}
				if (have_align) {
					align = PyObjC_EmbeddedAlignOfType(type);
					if (align == -1) return -1;
				} else {
					align = objc_ext_alignof_type(type);
					if (align == -1) return -1;
					have_align = 1;
				}
				max_align = MAX(align, max_align);
				acc_size = ROUND (acc_size, align);

				itemSize = objc_ext_sizeof_type (type);
				if (itemSize == -1) return -1;
				acc_size += itemSize;
				type = objc_ext_skip_type_specifier (type,YES);
			}
			if (max_align) {
				acc_size = ROUND(acc_size, max_align);
			}
			return acc_size;
		}

		case _C_UNION_B:
		{
			size_t max_size = 0;
         while (*type != _C_UNION_E && *type++ != '=')
            ; /* skip "<name>=" */

			while (*type != _C_UNION_E) {
            if (*type == '"') {
					type = strchr(type+1, '"');
					if (type) type++;
				}
				itemSize = objc_ext_sizeof_type (type);
				if (itemSize == -1) return -1;
				max_size = MAX (max_size, itemSize);
				type = objc_ext_skip_type_specifier (type,YES);
			}
			return max_size;
		}

		case _C_CONST:
		case _C_IN:
		case _C_INOUT:
		case _C_OUT:
		case _C_BYCOPY:
		case _C_ONEWAY:
			return objc_ext_sizeof_type(type+1);

		case _C_BFLD:
		{
			long i = strtol(type+1, NULL, 10);
			return (i+7)/8;
		}
			break;

		default:
			NSLog(@"objc_ext_sizeof_type: Unhandled type '%#x', %s", *type, type);
			return -1;
	}
}
