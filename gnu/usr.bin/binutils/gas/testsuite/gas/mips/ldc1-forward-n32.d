#objdump: -dr --prefix-addresses
#as: -n32 --defsym tldc1=1 --defsym forward=1
#name: MIPS ldc1 forward n32
#source: ld.s
#dump: l_d-n32.d
