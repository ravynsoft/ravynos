#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X endian options 2
#as: -mlittle-endian -mbig-endian
#source: dummy.s

.*: *file format elf32-tic6x-be


Disassembly of section \.text:
0+00 <[^>]*> 00002000[ \t]+nop 2
[ \t]*\.\.\.
