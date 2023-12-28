#name: C6X SB-relative relocations, local symbols, -r, REL
#as: -mlittle-endian -mgenerate-rel
#ld: -r -melf32_tic6x_le
#source: sbr-reloc-local-1-rel.s
#source: sbr-reloc-local-2-rel.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

0+ <[^>]*>:
[ \t]*0:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
[ \t]*0: R_C6000_SBR_U15_W[ \t]+\.data
[ \t]*4:[ \t]+0080016e[ \t]+ldw \.D2T2 \*\+b14\(4\),b1
[ \t]*4: R_C6000_SBR_U15_W[ \t]+\.data
[ \t]*8:[ \t]+0080024e[ \t]+ldh \.D2T2 \*\+b14\(4\),b1
[ \t]*8: R_C6000_SBR_U15_H[ \t]+\.data
[ \t]*c:[ \t]+0080034e[ \t]+ldh \.D2T2 \*\+b14\(6\),b1
[ \t]*c: R_C6000_SBR_U15_H[ \t]+\.data
[ \t]*10:[ \t]+0080062e[ \t]+ldb \.D2T2 \*\+b14\(6\),b1
[ \t]*10: R_C6000_SBR_U15_B[ \t]+\.data
[ \t]*14:[ \t]+0080072e[ \t]+ldb \.D2T2 \*\+b14\(7\),b1
[ \t]*14: R_C6000_SBR_U15_B[ \t]+\.data
[ \t]*18:[ \t]+008003a8[ \t]+mvk \.S1 7,a1
[ \t]*18: R_C6000_SBR_S16[ \t]+\.data
[ \t]*1c:[ \t]+00800328[ \t]+mvk \.S1 6,a1
[ \t]*1c: R_C6000_SBR_L16_B[ \t]+\.data
[ \t]*20:[ \t]+00800128[ \t]+mvk \.S1 2,a1
[ \t]*20: R_C6000_SBR_L16_H[ \t]+\.data
[ \t]*24:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*24: R_C6000_SBR_L16_W[ \t]+\.data
[ \t]*\.\.\.
[ \t]*40:[ \t]+0080026e[ \t]+ldw \.D2T2 \*\+b14\(8\),b1
[ \t]*40: R_C6000_SBR_U15_W[ \t]+\.data
[ \t]*44:[ \t]+0080036e[ \t]+ldw \.D2T2 \*\+b14\(12\),b1
[ \t]*44: R_C6000_SBR_U15_W[ \t]+\.data
[ \t]*48:[ \t]+0080064e[ \t]+ldh \.D2T2 \*\+b14\(12\),b1
[ \t]*48: R_C6000_SBR_U15_H[ \t]+\.data
[ \t]*4c:[ \t]+0080074e[ \t]+ldh \.D2T2 \*\+b14\(14\),b1
[ \t]*4c: R_C6000_SBR_U15_H[ \t]+\.data
[ \t]*50:[ \t]+00800e2e[ \t]+ldb \.D2T2 \*\+b14\(14\),b1
[ \t]*50: R_C6000_SBR_U15_B[ \t]+\.data
[ \t]*54:[ \t]+00800f2e[ \t]+ldb \.D2T2 \*\+b14\(15\),b1
[ \t]*54: R_C6000_SBR_U15_B[ \t]+\.data
[ \t]*58:[ \t]+008007a8[ \t]+mvk \.S1 15,a1
[ \t]*58: R_C6000_SBR_S16[ \t]+\.data
[ \t]*5c:[ \t]+00800728[ \t]+mvk \.S1 14,a1
[ \t]*5c: R_C6000_SBR_L16_B[ \t]+\.data
[ \t]*60:[ \t]+00800328[ \t]+mvk \.S1 6,a1
[ \t]*60: R_C6000_SBR_L16_H[ \t]+\.data
[ \t]*64:[ \t]+00800128[ \t]+mvk \.S1 2,a1
[ \t]*64: R_C6000_SBR_L16_W[ \t]+\.data
[ \t]*\.\.\.
