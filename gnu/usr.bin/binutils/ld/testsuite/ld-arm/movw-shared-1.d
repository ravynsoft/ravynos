#name: MOVW/MOVT shared libraries test 1
#source: movw-shared-1.s
#target: [check_shared_lib_support]
#ld: -shared
#error: .*: relocation R_ARM_MOVW_ABS_NC against `a' can not be used when making a shared object; recompile with -fPIC
