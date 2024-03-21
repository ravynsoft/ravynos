#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU loop

# Test the loop instructions

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 304a0000 	loop	00000000 <[^>]*>, r10.b2
[\t ]*0: R_PRU_U8_PCREL[\t ]*.text\+0x14
0+0004 <[^>]*> 30eb8000 	iloop	00000004 <[^>]*>, r11
[\t ]*4: R_PRU_U8_PCREL[\t ]*.text\+0x14
0+0008 <[^>]*> 00e0e0e0 	add	r0, r0, r0
0+000c <[^>]*> 00e0e0e0 	add	r0, r0, r0
0+0010 <[^>]*> 00e0e0e0 	add	r0, r0, r0
