#name: C6X PC-relative relocations, global symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s7a=0x0fffff00 --defsym s7b=0x100000fc --defsym s10a=0x0ffff800 --defsym s10b=0x100007fc --defsym s12a=0x0fffe000 --defsym s12b=0x10001ffc --defsym s21a=0x0fc00000 --defsym s21b=0x103ffffc
#source: pcrel-reloc-global.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+00c00162[ \t]+addkpc \.S2 fffff00 <[^>]*>,b1,0
10000004:[ \t]+00bf0162[ \t]+addkpc \.S2 100000fc <[^>]*>,b1,0
10000008:[ \t]+08000012[ \t]+b \.S2 fc00000 <[^>]*>
1000000c:[ \t]+07ffff92[ \t]+b \.S2 103ffffc <[^>]*>
10000010:[ \t]+00c01022[ \t]+bdec \.S2 ffff800 <[^>]*>,b1
10000014:[ \t]+00bff022[ \t]+bdec \.S2 100007fc <[^>]*>,b1
10000018:[ \t]+08000122[ \t]+bnop \.S2 fffe000 <[^>]*>,0
1000001c:[ \t]+07ff0122[ \t]+bnop \.S2 10001ffc <[^>]*>,0
