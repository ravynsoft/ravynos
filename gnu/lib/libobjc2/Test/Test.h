#import "../objc/runtime.h"
#import "../objc/objc-arc.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
@interface Test { id isa; }
+ (Class)class;
+ (id)new;
#if !__has_feature(objc_arc)
- (void)dealloc;
- (id)autorelease;
- (id)retain;
- (void)release;
#endif
@end

#ifdef __OBJC_GNUSTEP_RUNTIME_ABI__
#	if __OBJC_GNUSTEP_RUNTIME_ABI__ >= 20
#		define NEW_ABI
#	endif
#endif

@interface NSConstantString : Test
{
#ifdef NEW_ABI
	uint32_t flags;
	uint32_t length;
	uint32_t size;
	uint32_t hash;
	const char * const str;
#else
	const char * const str;
	const unsigned int length;
#endif
}
@end

@interface NSAutoreleasePool : Test
@end


