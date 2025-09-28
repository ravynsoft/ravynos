#name: C6X relocs to weak symbols, big endian
#as: -mbig-endian
#ld: -melf32_tic6x_be -Tsbr.ld
#source: weak.s
#objdump: -dr

.*: *file format elf32-tic6x-be


Disassembly of section \.text:
10000000 <[^>]*>:
10000000:[ \t]+000c0362[ \t]+b \.S2 b3
10000004:[ \t]+a00c0362[ \t]+\[a2\] b \.S2 b3
10000008:[ \t]+000c0363[ \t]+b \.S2 b3
1000000c:[ \t]+00800028[ \t]+\|\| mvk \.S1 0,a1
10000010:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
10000014:[ \t]+00800028[ \t]+mvk \.S1 0,a1
10000018:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
1000001c:[ \t]+00000000[ \t]+nop 1
