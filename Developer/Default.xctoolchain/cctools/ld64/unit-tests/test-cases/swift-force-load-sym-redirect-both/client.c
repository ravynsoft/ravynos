__attribute__((weak)) extern void *_swift_FORCE_LOAD_$_swiftFoundation;
__attribute__((weak)) extern void *foovar;

__attribute__((section("__DATA,__const")))
const void *_swift_FORCE_LOAD_$_swiftFoundation_MINE = &_swift_FORCE_LOAD_$_swiftFoundation;

__attribute__((section("__DATA,__const")))
const void *foovar_MINE = &foovar;

int main() {}
