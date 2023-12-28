#name: MIPS PIC relocation 0
#ld: -shared -T pic-reloc-ordinary.ld
#target: [check_shared_lib_support]
#source: pic-reloc-lui.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x0\): relocation R_MIPS_HI16 against `bar' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x8\): relocation R_MIPS_HI16 against `bar' cannot be used when making a shared object; recompile with -fPIC\Z
