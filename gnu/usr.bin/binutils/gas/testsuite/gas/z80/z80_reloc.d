#objdump: -r
#name: Z80 relocations

.*:[     ]+file format (coff)|(elf32)\-z80

RELOCATION RECORDS FOR \[\.text\]:
OFFSET[   ]+TYPE[              ]+VALUE\s*
00000001[ ]+r_byte0[           ]+\.text
00000004[ ]+r_byte1[           ]+\.text
00000007[ ]+r_byte0[           ]+glb_proc
0000000a[ ]+r_byte1[           ]+glb_proc
0000000d[ ]+r_byte2[           ]+glb_proc
00000010[ ]+r_byte3[           ]+glb_proc
00000012[ ]+r_imm16[           ]+\.text(\+0x0000001f)?
00000015[ ]+r_word0[           ]+glb_proc
00000018[ ]+r_word1[           ]+glb_proc
0000001b[ ]+r_jr[              ]+start(\+0xffffffff)|(\-0x00000001)
0000001d[ ]+r_imm8[            ]+data8
0000001f[ ]+r_imm8[            ]+data8
00000020[ ]+r_imm16[           ]+data16
00000022[ ]+r_imm24[           ]+data24
00000025[ ]+r_imm32[           ]+data32
00000029[ ]+r_byte0[           ]+data16
0000002a[ ]+r_byte1[           ]+data16
0000002b[ ]+r_word0[           ]+data32
0000002d[ ]+r_word1[           ]+data32
0000002f[ ]+r_byte3[           ]+data32
00000031[ ]+r_byte1[           ]+data24
00000032[ ]+r_byte2[           ]+data24
00000033[ ]+r_byte1[           ]+data32
00000034[ ]+r_byte2[           ]+data32
#pass
