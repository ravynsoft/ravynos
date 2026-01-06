const char baz_old __asm("$ld$previous$@rpath/libbaz.dylib$$1$1.0$100.0$_baz");
const char baz_old_ios __asm("$ld$previous$@rpath/libbaz.dylib$$2$1.0$100.0$_baz");
__attribute((visibility("default"))) int baz = 0;
