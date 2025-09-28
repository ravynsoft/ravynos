#name: C6X SB-relative relocations, local symbols, -r
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: sbr-reloc-local-1.s
#source: sbr-reloc-local-2.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

0+ <[^>]*>:
[ \t]*0:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
[ \t]*0: R_C6000_SBR_U15_W[ \t]+\.data
[ \t]*4:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
[ \t]*4: R_C6000_SBR_U15_W[ \t]+\.data\+0x4
[ \t]*8:[ \t]+0080004e[ \t]+ldh \.D2T2 \*\+b14\(0\),b1
[ \t]*8: R_C6000_SBR_U15_H[ \t]+\.data\+0x4
[ \t]*c:[ \t]+0080004e[ \t]+ldh \.D2T2 \*\+b14\(0\),b1
[ \t]*c: R_C6000_SBR_U15_H[ \t]+\.data\+0x6
[ \t]*10:[ \t]+0080002e[ \t]+ldb \.D2T2 \*\+b14\(0\),b1
[ \t]*10: R_C6000_SBR_U15_B[ \t]+\.data\+0x6
[ \t]*14:[ \t]+0080002e[ \t]+ldb \.D2T2 \*\+b14\(0\),b1
[ \t]*14: R_C6000_SBR_U15_B[ \t]+\.data\+0x7
[ \t]*18:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*18: R_C6000_SBR_S16[ \t]+\.data\+0x7
[ \t]*1c:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*1c: R_C6000_SBR_L16_B[ \t]+\.data\+0x6
[ \t]*20:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*20: R_C6000_SBR_H16_B[ \t]+\.data\+0x7
[ \t]*24:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*24: R_C6000_SBR_L16_H[ \t]+\.data\+0x4
[ \t]*28:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*28: R_C6000_SBR_H16_H[ \t]+\.data\+0x6
[ \t]*2c:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*2c: R_C6000_SBR_L16_W[ \t]+\.data
[ \t]*30:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*30: R_C6000_SBR_H16_W[ \t]+\.data\+0x4
[ \t]*\.\.\.
[ \t]*40:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
[ \t]*40: R_C6000_SBR_U15_W[ \t]+\.data\+0x8
[ \t]*44:[ \t]+0080006e[ \t]+ldw \.D2T2 \*\+b14\(0\),b1
[ \t]*44: R_C6000_SBR_U15_W[ \t]+\.data\+0xc
[ \t]*48:[ \t]+0080004e[ \t]+ldh \.D2T2 \*\+b14\(0\),b1
[ \t]*48: R_C6000_SBR_U15_H[ \t]+\.data\+0xc
[ \t]*4c:[ \t]+0080004e[ \t]+ldh \.D2T2 \*\+b14\(0\),b1
[ \t]*4c: R_C6000_SBR_U15_H[ \t]+\.data\+0xe
[ \t]*50:[ \t]+0080002e[ \t]+ldb \.D2T2 \*\+b14\(0\),b1
[ \t]*50: R_C6000_SBR_U15_B[ \t]+\.data\+0xe
[ \t]*54:[ \t]+0080002e[ \t]+ldb \.D2T2 \*\+b14\(0\),b1
[ \t]*54: R_C6000_SBR_U15_B[ \t]+\.data\+0xf
[ \t]*58:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*58: R_C6000_SBR_S16[ \t]+\.data\+0xf
[ \t]*5c:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*5c: R_C6000_SBR_L16_B[ \t]+\.data\+0xe
[ \t]*60:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*60: R_C6000_SBR_H16_B[ \t]+\.data\+0xf
[ \t]*64:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*64: R_C6000_SBR_L16_H[ \t]+\.data\+0xc
[ \t]*68:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*68: R_C6000_SBR_H16_H[ \t]+\.data\+0xe
[ \t]*6c:[ \t]+00800028[ \t]+mvk \.S1 0,a1
[ \t]*6c: R_C6000_SBR_L16_W[ \t]+\.data\+0x8
[ \t]*70:[ \t]+00800068[ \t]+mvkh \.S1 0,a1
[ \t]*70: R_C6000_SBR_H16_W[ \t]+\.data\+0xc
[ \t]*\.\.\.
