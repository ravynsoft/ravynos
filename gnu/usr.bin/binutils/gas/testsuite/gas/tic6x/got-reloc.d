#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X GOT relocs
#as: -march=c674x -mlittle-endian

.*: *file format elf32-tic6x-le

Disassembly of section \.text:
0+00 <[^>]*> 0000006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b0
[ \t]*0: R_C6000_SBR_GOT_U15_W[ \t]+\.LC1
0+04 <[^>]*> 0080002a[ \t]+mvk \.S2 0,b1
[ \t]*4: R_C6000_SBR_GOT_L16_W[ \t]+\.LC1
0+08 <[^>]*> 0080006a[ \t]+mvkh \.S2 0,b1
[ \t]*8: R_C6000_SBR_GOT_H16_W[ \t]+\.LC1
[ \t]*\.\.\.
