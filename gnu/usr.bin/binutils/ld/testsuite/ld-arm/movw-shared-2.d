#name: MOVW/MOVT shared libraries test 2
#source: movw-shared-2.s
#target: [check_shared_lib_support]
#ld: -shared
#error: .*: relocation R_ARM_MOVT_ABS against `b' can not be used when making a shared object; recompile with -fPIC
