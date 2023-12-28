#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU extern function call dump

# Test dumping of U16_PMEMIMM relocation

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 230000c3 	call	00000000 <myextfunc>
[\t ]*0: R_PRU_U16_PMEMIMM[\t ]*myextfunc
