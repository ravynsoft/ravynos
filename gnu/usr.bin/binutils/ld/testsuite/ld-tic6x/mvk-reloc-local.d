#name: C6X MVK relocations, local symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld
#source: mvk-reloc-local-1.s
#source: mvk-reloc-local-2.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+00804028[ \t]+mvk \.S1 128,a1
10000004:[ \t]+01004228[ \t]+mvk \.S1 132,a2
10000008:[ \t]+01800068[ \t]+mvkh \.S1 0,a3
[ \t]*\.\.\.
10000020:[ \t]+00804628[ \t]+mvk \.S1 140,a1
10000024:[ \t]+01004828[ \t]+mvk \.S1 144,a2
10000028:[ \t]+01ffffe8[ \t]+mvkh \.S1 4294901760,a3
[ \t]*\.\.\.
