#objdump: -dr --prefix-addresses
#as: -64 --defsym tl_d=1 --defsym forward=1
#name: MIPS l.d forward n64
#source: ld.s
#dump: l_d-n64.d
