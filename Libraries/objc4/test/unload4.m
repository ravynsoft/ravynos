// unload4: contains some objc metadata other than imageinfo
// libobjc must keep it open

int fake2 __attribute__((section("__DATA,__objc_foo"))) = 0;

// getsectiondata() falls over if __TEXT has no contents
const char *unload4 = "unload4";
