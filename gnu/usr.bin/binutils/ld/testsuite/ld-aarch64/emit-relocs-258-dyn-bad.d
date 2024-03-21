#name: LP64 R_AARCH64_ABS32 shared library test
#source: emit-relocs-258.s
#target: [check_shared_lib_support]
#ld: -shared
#error: .*: relocation R_AARCH64_ABS32 against `dummy' can not be used when making a shared object
