// unload3: contains imageinfo but no other objc metadata
// libobjc must not keep it open

#include <TargetConditionals.h>

int fake[2] __attribute__((section("__DATA,__objc_imageinfo")))
    = { 0, TARGET_OS_SIMULATOR ? (1<<5) : 0 };

// silence "no debug symbols in executable" warning
void fn(void) { }
