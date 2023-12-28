#objdump: -dr --prefix-addresses
#as: -32 --defsym tl_d=1 --defsym forward=1
#name: MIPS l.d forward
#source: ld.s
#dump: mips1@l_d.d
