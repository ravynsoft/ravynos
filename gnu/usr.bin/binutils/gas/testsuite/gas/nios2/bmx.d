#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 BMX instructions
#as: -march=r2

# Test the BMX instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> cfe0f820 	extract	ra,zero,31,0
0+0004 <[^>]*> cf0707e0 	extract	zero,ra,24,7
0+0008 <[^>]*> cfe022a0 	extract	r4,r10,31,0
0+000c <[^>]*> ce8fbae0 	extract	r23,r11,20,15
0+0010 <[^>]*> 8fe0f820 	insert	ra,zero,31,0
0+0014 <[^>]*> 8f0707e0 	insert	zero,ra,24,7
0+0018 <[^>]*> 8fe022a0 	insert	r4,r10,31,0
0+001c <[^>]*> 8e8fbae0 	insert	r23,r11,20,15
0+0020 <[^>]*> afe0f820 	merge	ra,zero,31,0
0+0024 <[^>]*> af0707e0 	merge	zero,ra,24,7
0+0028 <[^>]*> afe022a0 	merge	r4,r10,31,0
0+002c <[^>]*> ae8fbae0 	merge	r23,r11,20,15
