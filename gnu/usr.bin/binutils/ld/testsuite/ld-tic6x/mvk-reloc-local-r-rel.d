#name: C6X MVK relocations, local symbols, -r, REL
#as: -mlittle-endian -mgenerate-rel
#ld: -r -melf32_tic6x_le
#source: mvk-reloc-local-1-rel.s
#source: mvk-reloc-local-2-rel.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

0+ <[^>]*>:
[ \t]*0:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*0: R_C6000_ABS_S16[ \t]+\.data
[ \t]*4:[ \t]+01000228[ \t]+mvk \.S1 4,a2
[ \t]*4: R_C6000_ABS_L16[ \t]+\.data
[ \t]*\.\.\.
[ \t]*20:[ \t]+00800628[ \t]+mvk \.S1 12,a1
[ \t]*20: R_C6000_ABS_S16[ \t]+\.data
[ \t]*24:[ \t]+01000828[ \t]+mvk \.S1 16,a2
[ \t]*24: R_C6000_ABS_L16[ \t]+\.data
[ \t]*\.\.\.
