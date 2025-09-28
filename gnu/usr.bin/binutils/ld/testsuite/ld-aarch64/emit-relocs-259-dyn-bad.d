#name: R_AARCH64_ABS16 shared library test
#source: emit-relocs-259.s
#target: [check_shared_lib_support]
#ld: -shared
#error: .*: relocation R_AARCH64_ABS16 against `dummy' can not be used when making a shared object
