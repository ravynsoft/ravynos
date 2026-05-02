/*
 * _mh_dylib_header is normally synthesized by the linker for MH_DYLIB binaries.
 * dyld is MH_DYLINKER, so synthesize a compatible alias to the dylinker header
 * for objc-os.mm code that expects the dylib header symbol to exist.
 */

.globl __mh_dylib_header
__mh_dylib_header = __mh_dylinker_header

