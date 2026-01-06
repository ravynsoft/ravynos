#import <Foundation/Foundation.h>

@interface Foo: NSObject 
@end 

@implementation Foo
@end

extern const Foo *const globalFoo;


asm (".pushsection __DATA,__marker\n"
    ".p2align 3\n"
    ".global _globalFoo\n"
    "_globalFoo:\n"
#ifdef __LP64__
    "  .quad _OBJC_CLASS_$_Foo\n"
#else
    "  .long _OBJC_CLASS_$_Foo\n"
#endif
    ".popsection\n");
