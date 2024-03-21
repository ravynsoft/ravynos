#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X endian options 3
#as: -mbig-endian -mlittle-endian
#source: dummy.s

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
0+00 <[^>]*> 00002000[ \t]+nop 2
[ \t]*\.\.\.
