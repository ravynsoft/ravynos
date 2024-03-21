#name: C6X SB-relative relocations, global symbols, REL
#as: -mlittle-endian -mgenerate-rel
#ld: -melf32_tic6x_le -Tsbr.ld --defsym sw1=0x80 --defsym sw2=0x2007c --defsym sh1=0x80 --defsym sh2=0x1007e --defsym sb1=0x80 --defsym sb2=0x807f --defsym sb16a=0xffff8080 --defsym sb16b=0x807f --defsym sbw=0x123456f8 --defsym shw=0x2468ad70 --defsym sww=0x48d15a60
#source: sbr-reloc-global-rel.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
10000004:[ \t]+00ffff6e[ \t]+ldw \.D2T2 \*\+b14\(131068\),b1
10000008:[ \t]+0080004e[ \t]+ldh \.D2T2 \*\+b14\(0\),b1
1000000c:[ \t]+00ffff4e[ \t]+ldh \.D2T2 \*\+b14\(65534\),b1
10000010:[ \t]+0080002e[ \t]+ldb \.D2T2 \*\+b14\(0\),b1
10000014:[ \t]+00ffff2e[ \t]+ldb \.D2T2 \*\+b14\(32767\),b1
10000018:[ \t]+00c00028[ \t]+mvk \.S1 -32768,a1
1000001c:[ \t]+00bfffa8[ \t]+mvk \.S1 32767,a1
10000020:[ \t]+00ab3c28[ \t]+mvk \.S1 22136,a1
10000024:[ \t]+00ab3c28[ \t]+mvk \.S1 22136,a1
10000028:[ \t]+00ab3c28[ \t]+mvk \.S1 22136,a1
[ \t]*\.\.\.
