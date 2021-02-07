#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#include <unicode/utext.h>

/*
 * Define TRUE/FALSE to be used with UBool parameters, as these are no longer
 * defined in ICU as of ICU 68.
 */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/**
 * Initialises a UText structure with an NSString.  If txt is NULL, then this
 * allocates a new structure on the heap, otherwise it fills in the existing
 * one.
 *
 * The returned UText object holds a reference to the NSString and accesses its
 * contents directly.  
 */
UText* UTextInitWithNSString(UText *txt, NSString *str);

/**
 * Initialises a UText structure with an NSMutableString.  If txt is NULL, then
 * this allocates a new structure on the heap, otherwise it fills in the
 * existing one.
 *
 * The returned UText object holds a reference to the NSMutableString and
 * accesses its contents directly.  
 *
 * This function returns a mutable UText, and changes made to it will be
 * reflected in the underlying NSMutableString.
 */
UText* UTextInitWithNSMutableString(UText *txt, NSMutableString *str);

/**
 * GSUTextString is an NSString subclass that is backed by a libicu UText
 * structure.  This class is intended to be used when returning UText created
 * by libicu functions to Objective-C code.
 */
@interface GSUTextString : NSString
{
  @public
  /** The UText structure containing the libicu string interface. */
  UText txt;
}
@end

/**
 * GSUTextString is an NSMutableString subclass that is backed by a libicu
 * UText structure.  This class is intended to be used when returning UText
 * created by libicu functions to Objective-C code.
 */
@interface GSUTextMutableString : NSMutableString
{
  @public
  /** The UText structure containing the libicu string interface. */
  UText txt;
}
@end

/**
 * Cleanup function used to fee a unichar buffer.
 */
static inline void free_string(unichar **buf)
{
  if (0 != *buf)
    {
      free(*buf);
    }
}

/**
 * Allocates a temporary buffer of the requested size.  This allocates buffers
 * of up to 64 bytes on the stack or more than 64 bytes on the heap.  The
 * buffer is automatically destroyed when it goes out of scope in either case.
 *
 * Buffers created in this way are exception safe when using native exceptions.
 */
#define TEMP_BUFFER(name, size)\
  __attribute__((cleanup(free_string))) unichar *name ##_onheap = 0;\
  unichar name ## _onstack[64 / sizeof(unichar)];\
  unichar *name = name ## _onstack;\
  if (size > 64)\
    {\
      name ## _onheap = malloc(size);\
      name = name ## _onheap;\
    }

