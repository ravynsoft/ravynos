#objdump: -dr --prefix-addresses
#as: -64 --defsym tldc1=1 --defsym forward=1
#name: MIPS ldc1 forward n64
#source: ld.s
#dump: l_d-n64.d
