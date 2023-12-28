#name: MIPS PIC relocation 5
#ld: -shared -T pic-reloc-tls.ld
#target: [check_shared_lib_support]
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x0\): relocation R_(MICRO|)MIPS_HI16 against `a local symbol' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x8\): relocation R_(MICRO|)MIPS_HI16 against `bar' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x10\): relocation R_(MICRO|)MIPS_TLS_TPREL_HI16 against `a local symbol' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x14\): relocation R_(MICRO|)MIPS_TLS_TPREL_LO16 against `a local symbol' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x18\): relocation R_(MICRO|)MIPS_TLS_TPREL_HI16 against `global_tls' cannot be used when making a shared object; recompile with -fPIC\n
#error:   \(\.text\+0x1c\): relocation R_(MICRO|)MIPS_TLS_TPREL_LO16 against `global_tls' cannot be used when making a shared object; recompile with -fPIC\Z
