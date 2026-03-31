/*
TEST_BUILD_OUTPUT
.*nilAPIArgs.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
.*nilAPIArgs.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
END
*/

#include "test.h"

#import <objc/runtime.h>

int main() {
    // ensure various bits of API don't crash when tossed nil parameters
    class_conformsToProtocol(nil, nil);
    method_setImplementation(nil, NULL);
  
    succeed(__FILE__);
}
