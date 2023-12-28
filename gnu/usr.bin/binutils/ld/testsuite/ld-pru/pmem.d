#name: PRU R_PRU_BFD_*_PMEM
#source: pmem.s
#source: pmem_symbol.s
#ld:
#objdump: -s

# Note: default linker script should put a guard at DRAM address 0

.*: +file format elf32-pru

Contents of section .text:
 [0-9a-f]+ e0050024 e0070024 05000000 05000600  .*
 [0-9a-f]+ e0e0e012 e0e0e012                    .*
Contents of section .data:
 0000 00000000                             .*
