#name: MIPS _gp_disp removal from symbol tables
#ld: -shared
#objdump: -tT
#target: [check_shared_lib_support]

#failif
#...
.*_gp_disp
#pass
