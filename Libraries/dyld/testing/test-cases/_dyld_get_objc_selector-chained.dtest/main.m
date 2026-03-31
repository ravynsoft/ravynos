// BUILD_ONLY: MacOSX

// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_get_objc_selector-chained.exe -lobjc -Wl,-fixup_chains

// RUN:  ./_dyld_get_objc_selector-chained.exe

#include <mach-o/dyld_priv.h>

#import <Foundation/Foundation.h>

#include "test_support.h"

@interface DyldClass : NSObject
@end

@implementation DyldClass
-(void) dyldClassFoo {
	
}
+(void) dyldClassFoo {
	
}
@end

@interface DyldMainClass : NSObject
@end

@implementation DyldMainClass
-(void) dyldMainClassFoo {
	
}
-(void) dyldMainClassFoo2 {
	
}
@end

extern id objc_getClass(const char *name);

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // dyldClassFoo
    const char* sel = _dyld_get_objc_selector("dyldClassFoo");
    if (sel) {
        if ((SEL)sel != @selector(dyldClassFoo)) {
            FAIL("dyldClassFoo is wrong");
        }
    }

    // dyldMainClassFoo
    sel = _dyld_get_objc_selector("dyldMainClassFoo");
    if (sel) {
        if ((SEL)sel != @selector(dyldMainClassFoo)) {
            FAIL("dyldMainClassFoo is wrong");
        }
    }

    // dyldMainClassFoo2
    sel = _dyld_get_objc_selector("dyldMainClassFoo2");
    if (sel) {
        if ((SEL)sel != @selector(dyldMainClassFoo2)) {
            FAIL("dyldMainClassFoo2 is wrong");
        }
    }

    PASS("Success");
}
