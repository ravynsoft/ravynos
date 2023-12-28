#name: MIPS PIC relocation 7
#ld: -shared -T pic-reloc-ordinary.ld
#target: [check_shared_lib_support]
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x0\): relocation R_(MICRO|)MIPS_HIGHEST against `a local symbol' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x4\): relocation R_(MICRO|)MIPS_HIGHER against `a local symbol' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x8\): relocation R_(MICRO|)MIPS_HIGHEST against `bar' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0xc\): relocation R_(MICRO|)MIPS_HIGHER against `bar' cannot be used when making a shared object; recompile with -fPIC\Z
