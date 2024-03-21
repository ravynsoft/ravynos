#name: PRU R_PRU_BFD_RELOC_XX
#source: reloc.s
#source: reloc_symbol.s
#ld:
#objdump: -s

# Note: default linker script should put a guard at DRAM address 0

.*: +file format elf32-pru

Contents of section .text:
 [0-9a-f]+ fa00cefa efbeadde facefaef beadde00  .*
Contents of section .data:
 0000 00000000                             .*
