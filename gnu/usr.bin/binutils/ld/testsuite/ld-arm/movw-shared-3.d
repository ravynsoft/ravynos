#name: MOVW/MOVT shared libraries test 3
#source: movw-shared-3.s
#target: [check_shared_lib_support]
#ld: -shared
#error: .*: relocation R_ARM_THM_MOVW_ABS_NC against `c' can not be used when making a shared object; recompile with -fPIC
