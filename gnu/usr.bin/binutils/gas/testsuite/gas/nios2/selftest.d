#as: -r
#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 selftest

# Test the assembler self-test mode on some instructions that
# manipulate control registers.  The purpose of this test is to make
# sure the assembler doesn't choke, rather than to match the encodings
# of the particular instructions in the test here.

.*: +file format elf32-littlenios2


Disassembly of section .text:
0+0000 <[^>]*> 1001703a 	wrctl	status,r2
0+0004 <[^>]*> 1001703a 	wrctl	status,r2
0+0008 <[^>]*> 1001707a 	wrctl	estatus,r2
0+000c <[^>]*> 1001707a 	wrctl	estatus,r2
#...
