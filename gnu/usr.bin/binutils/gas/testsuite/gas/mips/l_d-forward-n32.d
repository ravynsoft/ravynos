#objdump: -dr --prefix-addresses
#as: -n32 --defsym tl_d=1 --defsym forward=1
#name: MIPS l.d forward n32
#source: ld.s
#dump: l_d-n32.d
