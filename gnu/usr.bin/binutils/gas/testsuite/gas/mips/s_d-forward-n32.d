#objdump: -dr --prefix-addresses
#as: -n32 --defsym ts_d=1 --defsym forward=1
#name: MIPS s.d forward n32
#source: ld.s
#dump: s_d-n32.d
