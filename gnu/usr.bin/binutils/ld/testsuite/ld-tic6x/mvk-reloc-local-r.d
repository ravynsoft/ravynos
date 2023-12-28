#name: C6X MVK relocations, local symbols, -r
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: mvk-reloc-local-1.s
#source: mvk-reloc-local-2.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

0+ <[^>]*>:
[ \t]*0:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*0: R_C6000_ABS_S16[ \t]+\.data
[ \t]*4:[ \t]+01000028[ \t]+mvk \.S1 0,a2
[ \t]*4: R_C6000_ABS_L16[ \t]+\.data\+0x4
[ \t]*8:[ \t]+01800068[ \t]+mvkh \.S1 0,a3
[ \t]*8: R_C6000_ABS_H16[ \t]+\.data\+0x8
[ \t]*\.\.\.
[ \t]*20:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*20: R_C6000_ABS_S16[ \t]+\.data\+0xc
[ \t]*24:[ \t]+01000028[ \t]+mvk \.S1 0,a2
[ \t]*24: R_C6000_ABS_L16[ \t]+\.data\+0x10
[ \t]*28:[ \t]+01800068[ \t]+mvkh \.S1 0,a3
[ \t]*28: R_C6000_ABS_H16[ \t]+\.data-0xec
[ \t]*\.\.\.
