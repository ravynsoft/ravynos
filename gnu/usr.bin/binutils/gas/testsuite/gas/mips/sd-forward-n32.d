#objdump: -dr --prefix-addresses
#as: -n32 --defsym tsd=1 --defsym forward=1
#name: MIPS sd forward n32
#source: ld.s
#dump: sd-n32.d
