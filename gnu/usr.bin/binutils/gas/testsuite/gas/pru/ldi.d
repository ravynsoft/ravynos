#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU ldi

# Test the load/store operations

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 240000d0 	ldi	r16.w2, 0
[\t ]*0: R_PRU_LDI32	\*ABS\*\+0x12345678
0+0004 <[^>]*> 24000090 	ldi	r16.w0, 0
0+0008 <[^>]*> 241234f0 	ldi	r16, 4660
0+000c <[^>]*> 240000f0 	ldi	r16, 0
[\t ]*c: R_PRU_U16_PMEMIMM	.text
0+0010 <[^>]*> 240000d0 	ldi	r16.w2, 0
[\t ]*10: R_PRU_LDI32	var1
0+0014 <[^>]*> 24000090 	ldi	r16.w0, 0
