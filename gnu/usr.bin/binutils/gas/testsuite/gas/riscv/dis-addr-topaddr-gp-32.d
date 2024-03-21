#as: -march=rv32i
#source: dis-addr-topaddr-gp.s
#objdump: -d

.*:     file format elf32-(little|big)riscv


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+0051a283[ 	]+lw[   	]+t0,5\(gp\) # 4 <addr_rel_gp_pos>
[ 	]+[0-9a-f]+:[ 	]+ffd1a303[ 	]+lw[   	]+t1,-3\(gp\) # fffffffc <addr_rel_gp_neg>
