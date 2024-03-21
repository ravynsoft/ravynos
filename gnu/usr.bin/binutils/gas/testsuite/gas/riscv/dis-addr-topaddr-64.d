#as: -march=rv64ic -defsym rv64=1
#source: dis-addr-topaddr.s
#objdump: -d

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+fff00283[ 	]+lb[   	]+t0,-1\(zero\) # ffffffffffffffff <addr_top>
