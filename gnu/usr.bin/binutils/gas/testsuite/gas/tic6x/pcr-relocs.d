#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X PCR H16/L16 relocs
#as: -mlittle-endian

.*: *file format elf32-tic6x-le

Disassembly of section \.text:
0+00 <[^>]*> 00800264[ \t]+ldw \.D1T1 \*\+a0\(0\),a1
0+04 <[^>]*> 00800264[ \t]+ldw \.D1T1 \*\+a0\(0\),a1
0+08 <[^>]*> 00800264[ \t]+ldw \.D1T1 \*\+a0\(0\),a1
0+0c <[^>]*> 004003e2[ \t]+mvc \.S2 pce1,b0
0+10 <[^>]*> 01000264[ \t]+ldw \.D1T1 \*\+a0\(0\),a2
0+14 <[^>]*> 0100002a[ \t]+mvk \.S2 0,b2
[ \t]+14: R_C6000_PCR_L16	S0-0xc
0+18 <[^>]*> 0100006a[ \t]+mvkh \.S2 0,b2
[ \t]+18: R_C6000_PCR_H16	S0-0xc
0+1c <[^>]*> 0100002a[ \t]+mvk \.S2 0,b2
[ \t]+1c: R_C6000_PCR_L16	S0-0x38
0+20 <[^>]*> 0100006a[ \t]+mvkh \.S2 0,b2
[ \t]+20: R_C6000_PCR_H16	S0-0x18
0+24 <[^>]*> 0100002a[ \t]+mvk \.S2 0,b2
[ \t]+24: R_C6000_PCR_L16	S1\+0x14
0+28 <[^>]*> 0100006a[ \t]+mvkh \.S2 0,b2
[ \t]+28: R_C6000_PCR_H16	S1\+0x14
0+2c <[^>]*> 0100002a[ \t]+mvk \.S2 0,b2
[ \t]+2c: R_C6000_PCR_L16	S1-0x18
0+30 <[^>]*> 0100006a[ \t]+mvkh \.S2 0,b2
[ \t]+30: R_C6000_PCR_H16	S1-0x18
0+34 <[^>]*> 00800264[ \t]+ldw \.D1T1 \*\+a0\(0\),a1
0+38 <[^>]*> 004003e2[ \t]+mvc \.S2 pce1,b0
0+3c <[^>]*> 00800264[ \t]+ldw \.D1T1 \*\+a0\(0\),a1
